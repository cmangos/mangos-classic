/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "Auth/HMACSHA1.h"
#include "Auth/WardenKeyGen.h"
#include "Common.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "Log.h"
#include "Server/Opcodes.h"
#include "ByteBuffer.h"
#include <openssl/md5.h>
#include "Database/DatabaseEnv.h"
#include "World/World.h"
#include "Entities/Player.h"
#include "Util.h"
#include "WardenWin.h"
#include "Modules/WardenModuleWin.h"
#include "WardenCheckMgr.h"

WardenWin::WardenWin() : Warden(), _serverTicks(0) {}

WardenWin::~WardenWin() { }

void WardenWin::Init(WorldSession* session, BigNumber* k)
{
    _session = session;
    // Generate Warden Key
    SHA1Randx WK(k->AsByteArray(), k->GetNumBytes());
    WK.Generate(_inputKey, 16);
    WK.Generate(_outputKey, 16);

    memcpy(_seed, Module.Seed, 16);

    _inputCrypto.Init(_inputKey);
    _outputCrypto.Init(_outputKey);
    sLog.outWardenLog("Server side warden for client %u (build %u) initializing...", session->GetAccountId(), _session->GetClientBuild());
    sLog.outWardenLog("C->S Key: %s", ByteArrayToHexStr(_inputKey, 16).c_str());
    sLog.outWardenLog("S->C Key: %s", ByteArrayToHexStr(_outputKey, 16).c_str());
    sLog.outWardenLog("  Seed: %s", ByteArrayToHexStr(_seed, 16).c_str());
    sLog.outWardenLog("Loading Module...");

    _module = GetModuleForClient();

    sLog.outWardenLog("Module Key: %s", ByteArrayToHexStr(_module->Key, 16).c_str());
    sLog.outWardenLog("Module ID: %s", ByteArrayToHexStr(_module->Id, 16).c_str());
    RequestModule();
}

ClientWardenModule* WardenWin::GetModuleForClient()
{
    ClientWardenModule *mod = new ClientWardenModule;

    uint32 length = sizeof(Module.Module);

    // data assign
    mod->CompressedSize = length;
    mod->CompressedData = new uint8[length];
    memcpy(mod->CompressedData, Module.Module, length);
    memcpy(mod->Key, Module.ModuleKey, 16);

    // md5 hash
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, mod->CompressedData, length);
    MD5_Final((uint8*)&mod->Id, &ctx);

    return mod;
}

void WardenWin::InitializeModule()
{
    sLog.outWardenLog("Initialize module");

    // Create packet structure
    WardenInitModuleRequest Request;
    Request.Command1 = WARDEN_SMSG_MODULE_INITIALIZE;
    Request.Size1 = 20;
    Request.CheckSumm1 = BuildChecksum(&Request.Unk1, 20);
    Request.Unk1 = 1;
    Request.Unk2 = 0;
    Request.Type = 1;
    Request.String_library1 = 0;
    Request.Function1[0] = 0x002485F0;                      // 0x00400000 + 0x002485F0 SFileOpenFile
    Request.Function1[1] = 0x002487F0;                      // 0x00400000 + 0x002487F0 SFileGetFileSize
    Request.Function1[2] = 0x00248460;                      // 0x00400000 + 0x00248460 SFileReadFile
    Request.Function1[3] = 0x00248730;                      // 0x00400000 + 0x00248730 SFileCloseFile

    Request.Command2 = WARDEN_SMSG_MODULE_INITIALIZE;
    Request.Size2 = 8;
    Request.CheckSumm2 = BuildChecksum(&Request.Unk2, 8);
    Request.Unk3 = 4;
    Request.Unk4 = 0;
    Request.String_library2 = 0;
    Request.Function2 = 0x00419D40;                         // 0x00400000 + 0x00419D40 FrameScript::GetText
    Request.Function2_set = 1;

    Request.Command3 = WARDEN_SMSG_MODULE_INITIALIZE;
    Request.Size3 = 8;
    Request.CheckSumm3 = BuildChecksum(&Request.Unk5, 8);
    Request.Unk5 = 1;
    Request.Unk6 = 1;
    Request.String_library3 = 0;
    Request.Function3 = 0x0046AE20;                         // 0x00400000 + 0x0046AE20 PerformanceCounter
    Request.Function3_set = 1;

    // Encrypt with warden RC4 key.
    EncryptData((uint8*)&Request, sizeof(WardenInitModuleRequest));

    WorldPacket pkt(SMSG_WARDEN_DATA, sizeof(WardenInitModuleRequest));
    pkt.append((uint8*)&Request, sizeof(WardenInitModuleRequest));
    _session->SendPacket(pkt);

    Warden::InitializeModule();
}

void WardenWin::HandleHashResult(ByteBuffer &buff)
{
    buff.rpos(buff.wpos());

    // Verify key
    if (memcmp(buff.contents() + 1, Module.ClientKeySeedHash, sizeof(Module.ClientKeySeedHash)) != 0)
    {
        sLog.outWardenLog("%s failed hash reply. Action: %s", _session->GetPlayerName(), Penalty().c_str());
        return;
    }

    sLog.outWardenLog("Request hash reply: succeed");

    // Change keys here
    memcpy(_inputKey, Module.ClientKeySeed, 16);
    memcpy(_outputKey, Module.ServerKeySeed, 16);

    _inputCrypto.Init(_inputKey);
    _outputCrypto.Init(_outputKey);

    _previousTimestamp = WorldTimer::getMSTime();
}

void WardenWin::RequestData()
{
    sLog.outWardenLog("Request data");

    uint16 build = _session->GetClientBuild();
    uint16 id = 0;
    uint8 type = 0;
    WardenCheck* wd = NULL;

    // If all checks were done, fill the todo list again
    if (_memChecksTodo.empty())
        sWardenCheckMgr->GetWardenCheckIds(true, build, _memChecksTodo);

    if (_otherChecksTodo.empty())
        sWardenCheckMgr->GetWardenCheckIds(false, build, _otherChecksTodo);

    _serverTicks = WorldTimer::getMSTime();

    _currentChecks.clear();

    // Build check request
    for (uint16 i = 0; i < sWorld.getConfig(CONFIG_UINT32_WARDEN_NUM_MEM_CHECKS); ++i)
    {
        // If todo list is done break loop (will be filled on next Update() run)
        if (_memChecksTodo.empty())
            break;

        // Get check id from the end and remove it from todo
        id = _memChecksTodo.back();
        _memChecksTodo.pop_back();

        // Add the id to the list sent in this cycle
        _currentChecks.push_back(id);
    }

    ByteBuffer buff;
    buff << uint8(WARDEN_SMSG_CHEAT_CHECKS_REQUEST);

    for (uint16 i = 0; i < sWorld.getConfig(CONFIG_UINT32_WARDEN_NUM_OTHER_CHECKS); ++i)
    {
        // If todo list is done break loop (will be filled on next Update() run)
        if (_otherChecksTodo.empty())
            break;

        // Get check id from the end and remove it from todo
        id = _otherChecksTodo.back();
        _otherChecksTodo.pop_back();

        // Add the id to the list sent in this cycle
        _currentChecks.push_back(id);

        // if we are here, the function is guaranteed to not return NULL
        // but ... who knows
        wd = sWardenCheckMgr->GetWardenDataById(build, id);
        if (wd)
        {
            switch (wd->Type)
            {
                case MPQ_CHECK:
                case LUA_STR_CHECK:
                case DRIVER_CHECK:
                    buff << uint8(wd->Str.size());
                    buff.append(wd->Str.c_str(), wd->Str.size());
                    break;
                default:
                    break;
            }
        }
    }

    uint8 xorByte = _inputKey[0];

    // Add TIMING_CHECK
    buff << uint8(0x00);
    buff << uint8(TIMING_CHECK ^ xorByte);

    uint8 index = 1;

    for (std::list<uint16>::iterator itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
    {
        wd = sWardenCheckMgr->GetWardenDataById(build, *itr);

        type = wd->Type;
        buff << uint8(type ^ xorByte);
        switch (type)
        {
            case MEM_CHECK:
            {
                buff << uint8(0x00);
                buff << uint32(wd->Address);
                buff << uint8(wd->Length);
                break;
            }
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            {
                buff.append(wd->Data.AsByteArray(0, false), wd->Data.GetNumBytes());
                buff << uint32(wd->Address);
                buff << uint8(wd->Length);
                break;
            }
            case MPQ_CHECK:
            case LUA_STR_CHECK:
            {
                buff << uint8(index++);
                break;
            }
            case DRIVER_CHECK:
            {
                buff.append(wd->Data.AsByteArray(0, false), wd->Data.GetNumBytes());
                buff << uint8(index++);
                break;
            }
            case MODULE_CHECK:
            {
                uint32 seed = urand();
                buff << uint32(seed);
                HMACSHA1 hmac(4, (uint8*)&seed);
                hmac.UpdateData(wd->Str);
                hmac.Finalize();
                buff.append(hmac.GetDigest(), hmac.GetLength());
                break;
            }
            /*case PROC_CHECK:
            {
                buff.append(wd->i.AsByteArray(0, false).get(), wd->i.GetNumBytes());
                buff << uint8(index++);
                buff << uint8(index++);
                buff << uint32(wd->Address);
                buff << uint8(wd->Length);
                break;
            }*/
            default:
                break;                                      // Should never happen
        }
    }
    buff << uint8(xorByte);
    buff.hexlike();

    // Encrypt with warden RC4 key
    EncryptData((uint8*)buff.contents(), buff.size());

    WorldPacket pkt(SMSG_WARDEN_DATA, buff.size());
    pkt.append(buff);
    _session->SendPacket(pkt);

    std::stringstream stream;
    stream << "Sent check id's: ";
    for (std::list<uint16>::iterator itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
        stream << *itr << " ";

    sLog.outWardenLog("%s", stream.str().c_str());

    Warden::RequestData();
}

void WardenWin::HandleData(ByteBuffer &buff)
{
    sLog.outWardenLog("Handle data");

    uint16 Length;
    buff >> Length;
    uint32 Checksum;
    buff >> Checksum;

    if (!IsValidCheckSum(Checksum, buff.contents() + buff.rpos(), Length))
    {
        buff.rpos(buff.wpos());
        sLog.outWardenLog("%s failed checksum. Action: %s", _session->GetPlayerName(), Penalty().c_str());
        return;
    }

    // TIMING_CHECK
    {
        uint8 result;
        buff >> result;
        /// @todo test it.
        if (result == 0x00)
        {
            sLog.outWardenLog("%s failed timing check. Action: %s", _session->GetPlayerName(), Penalty().c_str());
            return;
        }

        uint32 newClientTicks;
        buff >> newClientTicks;

        uint32 ticksNow = WorldTimer::getMSTime();
        uint32 ourTicks = newClientTicks + (ticksNow - _serverTicks);

        sLog.outWardenLog("ServerTicks %u, RequestTicks %u, ClientTicks %u", ticksNow, _serverTicks, newClientTicks);  // Now, At request, At response
        sLog.outWardenLog("Waittime %u", ourTicks - newClientTicks);

    }

    WardenCheckResult* rs;
    WardenCheck *rd;
    uint8 type;
    uint16 checkFailed = 0;

    for (std::list<uint16>::iterator itr = _currentChecks.begin(); itr != _currentChecks.end(); ++itr)
    {
        rd = sWardenCheckMgr->GetWardenDataById(_session->GetClientBuild(), *itr);
        rs = sWardenCheckMgr->GetWardenResultById(_session->GetClientBuild(), *itr);

        type = rd->Type;
        switch (type)
        {
            case MEM_CHECK:
            {
                uint8 Mem_Result;
                buff >> Mem_Result;

                if (Mem_Result != 0)
                {
                    sLog.outWardenLog("RESULT MEM_CHECK not 0x00, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    checkFailed = *itr;
                    continue;
                }
                if (memcmp(buff.contents() + buff.rpos(), rs->Result.AsByteArray(0, false), rd->Length) != 0)
                {
                    sLog.outWardenLog("RESULT MEM_CHECK fail CheckId %u account Id %u", *itr, _session->GetAccountId());
                    checkFailed = *itr;
                    buff.rpos(buff.rpos() + rd->Length);
                    continue;
                }

                buff.rpos(buff.rpos() + rd->Length);
                sLog.outWardenLog("RESULT MEM_CHECK passed CheckId %u account Id %u", *itr, _session->GetAccountId());
                break;
            }
            case PAGE_CHECK_A:
            case PAGE_CHECK_B:
            case DRIVER_CHECK:
            case MODULE_CHECK:
            {
                const uint8 byte = 0xE9;
                if (memcmp(buff.contents() + buff.rpos(), &byte, sizeof(uint8)) != 0)
                {
                    if (type == PAGE_CHECK_A || type == PAGE_CHECK_B)
                        sLog.outWardenLog("RESULT PAGE_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    if (type == MODULE_CHECK)
                        sLog.outWardenLog("RESULT MODULE_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    if (type == DRIVER_CHECK)
                        sLog.outWardenLog("RESULT DRIVER_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    checkFailed = *itr;
                    buff.rpos(buff.rpos() + 1);
                    continue;
                }

                buff.rpos(buff.rpos() + 1);
                if (type == PAGE_CHECK_A || type == PAGE_CHECK_B)
                    sLog.outWardenLog("RESULT PAGE_CHECK passed CheckId %u account Id %u", *itr, _session->GetAccountId());
                else if (type == MODULE_CHECK)
                    sLog.outWardenLog("RESULT MODULE_CHECK passed CheckId %u account Id %u", *itr, _session->GetAccountId());
                else if (type == DRIVER_CHECK)
                    sLog.outWardenLog("RESULT DRIVER_CHECK passed CheckId %u account Id %u", *itr, _session->GetAccountId());
                break;
            }
            case LUA_STR_CHECK:
            {
                uint8 Lua_Result;
                buff >> Lua_Result;

                if (Lua_Result != 0)
                {
                    sLog.outWardenLog("RESULT LUA_STR_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    checkFailed = *itr;
                    continue;
                }

                uint8 luaStrLen;
                buff >> luaStrLen;

                if (luaStrLen != 0)
                {
                    char *str = new char[luaStrLen + 1];
                    memcpy(str, buff.contents() + buff.rpos(), luaStrLen);
                    str[luaStrLen] = '\0'; // null terminator
                    sLog.outWardenLog("Lua string: %s", str);
                    delete[] str;
                }
                buff.rpos(buff.rpos() + luaStrLen);         // Skip string
                sLog.outWardenLog("RESULT LUA_STR_CHECK passed, CheckId %u account Id %u", *itr, _session->GetAccountId());
                break;
            }
            case MPQ_CHECK:
            {
                uint8 Mpq_Result;
                buff >> Mpq_Result;

                if (Mpq_Result != 0)
                {
                    sLog.outWardenLog("RESULT MPQ_CHECK not 0x00 account id %u", _session->GetAccountId());
                    checkFailed = *itr;
                    continue;
                }

                if (memcmp(buff.contents() + buff.rpos(), rs->Result.AsByteArray(0, false), 20) != 0) // SHA1
                {
                    sLog.outWardenLog("RESULT MPQ_CHECK fail, CheckId %u account Id %u", *itr, _session->GetAccountId());
                    checkFailed = *itr;
                    buff.rpos(buff.rpos() + 20);            // 20 bytes SHA1
                    continue;
                }

                buff.rpos(buff.rpos() + 20);                // 20 bytes SHA1
                sLog.outWardenLog("RESULT MPQ_CHECK passed, CheckId %u account Id %u", *itr, _session->GetAccountId());
                break;
            }
            default:                                        // Should never happen
                break;
        }
    }

    if (checkFailed > 0)
    {
        WardenCheck* check = sWardenCheckMgr->GetWardenDataById(_session->GetClientBuild(), checkFailed);   //note it IS NOT NULL here
        sLog.outWardenLog("%s failed Warden check %u. Action: %s", _session->GetPlayerName(), checkFailed, Penalty(check).c_str());
        LogPositiveToDB(check);
    }

    Warden::HandleData(buff);
}
