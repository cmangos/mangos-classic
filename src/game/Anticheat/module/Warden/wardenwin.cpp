/*
 * Copyright (C) 2017-2020 namreeb (legal@namreeb.org)
 *
 * This is private software and may not be shared under any circumstances,
 * absent permission of namreeb.
 */

#include "warden.hpp"
#include "WardenWin.hpp"
#include "WardenModuleMgr.hpp"
#include "WardenScanMgr.hpp"
#include "WardenScan.hpp"
#include "../libanticheat.hpp"
#include "World/World.h"
#include "../config.hpp"

#include "Entities/Unit.h"
#include "Chat/Chat.h"
#include "Server/WorldSession.h"
#include "Auth/BigNumber.h"
#include "Auth/CryptoHash.h"
#include "Auth/HMACSHA1.h"
#include "Util/ByteBuffer.h"
#include "Database/DatabaseEnv.h"
#include "Entities/Player.h"
#include "Log/Log.h"

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>

namespace
{
    // fixed offsets for classic client(s):
    static constexpr struct ClientOffsets
    {
        uint32 Build;

        // LuaScan
        uint32 GetText;

        // FileHashScan
        uint32 Open;
        uint32 Size;
        uint32 Read;
        uint32 Close;

        // TimeScan
        uint32 TickCount;

        // CWorld::enables memory scan
        uint32 WorldEnables;

        // LastHardwareAction memory scan (CSimpleTop::m_eventTime)
        uint32 LastHardwareAction;

        // EndScene memory scan
        uint32 g_theGxDevicePtr;
        uint32 OfsDevice2;
        uint32 OfsDevice3;
        uint32 OfsDevice4;

        // Warden memory scan
        uint32 WardenModule;
        uint32 OfsWardenSysInfo;
        uint32 OfsWardenWinSysInfo;
    } Offsets[] = {
        {
            5875,
            0x303BF0,
            0x2477A0, 0x2487F0, 0x248460, 0x253900,
            0x2C010,
            0xC7B2A4,
            0xCF0BC8,
            0xC0ED38, 0x38A8, 0x0, 0xA8,
            0xCE897C, 0x228, 0x08
        }
    };

// TODO: Identify drivers for other hypervisors and add detections for them too
constexpr struct
{
    const char *Name;
    const char *Driver;
    const char *DeviceName;
} Hypervisors[] =
{
    { "VirtualBox", "VBoxGuest", "\\Device\\VBoxGuest"},
    { "ESXi", "vmmemctl", "\\Device\\vmmemctl"},
};

auto constexpr HypervisorCount = sizeof(Hypervisors) / sizeof(Hypervisors[0]);

enum WorldEnables
{
    TerrainDoodads                  = 0x1,              // default, toggled by sub at 0x673130, which is never called.  should always be set
    Terrain                         = 0x2,              // default, toggled by sub at 0x6730F0, which is never called.  should always be set
    TerrainLOD                      = 0x4,              // lod console var
    Unk10                           = 0x10,             // default
    TerrainCulling                  = 0x20,             // default, showCull console command
    TerrainShadows                  = 0x40,             // default, mapShadows console var, showShadow console command
    TerrainDoodadCollisionVisuals   = 0x80,             // toggled by sub at 0x6731C0, which is never called.  should never be set
    MapObjects                      = 0x100,            // default, toggled by sub at 0x673430, which is never called.  should always be set
    MapObjectLighting               = 0x200,            // default, toggled by sub at 0x673360, which is never called.  should always be set
    FootPrints                      = 0x400,            // showfootprints console var
    MapObjectTextures               = 0x800,            // default, toggled by sub at 0x6733A0, which is never called.  should always be set
    PortalDisplay                   = 0x1000,           // toggled by sub at 0x673470, which is never called.  should never be set
    PortalVisual                    = 0x2000,           // toggled by sub at 0x6734B0, which is never called.  should never be set
    DisableDoodadFullAlpha          = 0x4000,           // fullAlpha console var
    DoodadAnimation                 = 0x8000,           // doodadAnim console var
    TriangleStrips                  = 0x10000,          // triangleStrips console var
    CrappyBatches                   = 0x20000,          // toggled by sub at 0x6733E0, which is never called.  should never be set
    ZoneBoundaryVisuals             = 0x40000,          // zoneBoundary disabled console command (should never be set, also sends CMSG_ZONE_MAP, sub at 0x673850)
    BSPRender                       = 0x80000,          // toggled by sub at 0x6730A0, which is never called.  should never be set
    DetailDoodads                   = 0x100000,         // default, showDetailDoodads console command
    ShowQuery                       = 0x200000,         // showQuery disabled console command (should never be set)
    TerrainDoodadAABoxVisuals       = 0x400000,         // toggled by sub at 0x673170, which is never called.  should never be set
    TrilinearFiltering              = 0x800000,         // trilinear console var
    Water                           = 0x1000000,        // default, toggled by sub at 0x673670, which is never called.  should always be set
    WaterParticulates               = 0x2000000,        // default, waterParticulates console command
    TerrainLowDetail                = 0x4000000,        // default, showLowDetail console command
    Specular                        = 0x8000000,        // specular console var
    PixelShaders                    = 0x10000000,       // pixelShaders console var
    Unknown6737F9                   = 0x20000000,       // unknown, set by sub at 0x6737F0, should never be set
    Unknown673820                   = 0x40000000,       // unknown, set by sub at 0x673820, should never be set
    Anisotropic                     = 0x80000000,       // anisotropic console var

    Required = (TerrainDoodads|Terrain| MapObjects| MapObjectLighting| MapObjectTextures| Water),
    Prohibited = (TerrainDoodadCollisionVisuals|CrappyBatches|ZoneBoundaryVisuals|BSPRender|ShowQuery|TerrainDoodadAABoxVisuals|Unknown6737F9|Unknown673820),
};

const ClientOffsets* GetClientOffsets(uint32 build)
{
    static auto constexpr offset_count = sizeof(Offsets) / sizeof(Offsets[0]);

    for (auto i = 0; i < offset_count; ++i)
        if (Offsets[i].Build == build)
            return &Offsets[i];

    return nullptr;
}

// returns ScanFlag mask for those builds which we have offsets
ScanFlags GetScanFlagsByAvailableOffsets()
{
    uint32 result = None;

    auto offset_count = sizeof(Offsets) / sizeof(Offsets[0]);

    for (auto i = 0; i < offset_count; ++i)
    {
        switch (Offsets[i].Build)
        {
            case 5875:
                result |= WinBuild5875 | WinBuild6005 | WinBuild6141;
                break;
            default: break;
        }
    }

    return static_cast<ScanFlags>(result);
}

std::string ArchitectureString(uint16 arch)
{
    switch (arch)
    {
        case 0:
            return "x86";
        case 5:
            return "ARM";
        case 6:
            return "IA64";
        case 9:
            return "x64";
        case 0xFFFF:
            return "Unknown";
        default:
            return "INVALID";
    }
}

std::string CPUTypeAndRevision(uint32 cpuType, uint16 revision)
{
    std::stringstream str;

    switch (cpuType)
    {
        case 386:
        {
            str << "i386";

            const uint8 firstByte = revision >> 8;
            const uint8 y = 0xF & (revision >> 4);
            const uint8 z = 0xF & (revision);

            if (firstByte == 0xFF)
                str << " Model: " << static_cast<uint32>(y) << " Stepping: " << z;
            else
                str << " Stepping: " << static_cast<char>('A' + firstByte) << static_cast<uint32>(firstByte & 0xFF);

            break;
        }
        case 486:
        {
            str << "i486";

            const uint8 firstByte = revision >> 8;
            const uint8 y = 0xF & (revision >> 4);
            const uint8 z = 0xF & (revision);

            if (firstByte == 0xFF)
                str << " Model: " << static_cast<uint32>(y) << " Stepping: " << z;
            else
                str << " Stepping: " << static_cast<char>('A' + firstByte) << static_cast<uint32>(firstByte & 0xFF);

            break;
        }
        case 586:
        {
            str << "Pentium (i586)";

            const uint8 model = 0xFF & (revision >> 8);
            const uint8 stepping = 0xFF & (revision);

            str << " Model: " << static_cast<uint32>(model) << " Stepping: " << static_cast<uint32>(stepping);

            break;
        }
        case 2200:
            str << "IA64";
            break;
        case 8664:
            str << "X8664";
            break;
        default:
            str << "ARM/Unknown";
            break;
    }

    return str.str();
}

// this function assumes that the given code begins with a valid instruction.  in other words, that
// it does not begin in random data or in the middle of an instruction.
void DeobfuscateAsm(std::vector<std::uint8_t> &code)
{
#define LSTRIP(c, l) do { if (c.size() <= l) { c.clear(); return; } else { c.erase(c.begin(), c.begin()+l); } } while(false)

    do
    {
        if (code.empty())
            return;

        // At the start of this loop we re-assume the entry condition that the code begins at a valid instruction.

        // remove NOP
        if (code[0] == 0x90)
        {
            LSTRIP(code, 1);
            continue;
        }

        // xchg statements which are either semantically equivalent to NOP or
        // which will be inverted later on.
        if (code[0] == 0x87)
        {
            // {"eax", "ebx", "ecx", "edx", "edi", "esi"};
            if (code[1] == 0xC9 ||  // xchg ecx, ecx
                code[1] == 0xCA ||  // xchg ecx, edx
                code[1] == 0xCB ||  // xchg ebx, ecx
                code[1] == 0xCE ||  // xchg esi, ecx
                code[1] == 0xCF ||  // xchg ecx, edi
                code[1] == 0xE4 ||  // xchg esp, esp
                code[1] == 0xED ||  // xchg ebp, ebp
                code[1] == 0xD1 ||  // xchg ecx, edx
                code[1] == 0xD2 ||  // xchg edx, edx
                code[1] == 0xD3 ||  // xchg ebx, edx
                code[1] == 0xD6 ||  // xchg esi, edx
                code[1] == 0xD7 ||  // xchg edx, edi
                code[1] == 0xD9 ||  // xchg ecx, ebx
                code[1] == 0xDA ||  // xchg ebx, edx
                code[1] == 0xDB ||  // xchg ebx, ebx
                code[1] == 0xDE ||  // xchg esi, ebx
                code[1] == 0xDF ||  // xchg ebx, edi
                code[1] == 0xF1 ||  // xchg ecx, esi
                code[1] == 0xF2 ||  // xchg edx, esi
                code[1] == 0xF3 ||  // xchg ebx, esi
                code[1] == 0xF7 ||  // xchg edi, esi
                code[1] == 0xF9 ||  // xchg ecx, edi
                code[1] == 0xFA ||  // xchg edx, edi
                code[1] == 0xFB ||  // xchg edi, esi
                code[1] == 0xFE)    // xchg ebx, edi
            {
                LSTRIP(code, 2);
                continue;
            }
        }
        // xchg one register with another.  in wrobot this is always repeated/inverted later
        else if (code[0] >= 0x91 && code[0] <= 0x97)
        {
            LSTRIP(code, 1);
            continue;
        }
        // simple NOP-equivalent mov statements
        else if (code[0] == 0x89)
        {
            if (code[1] == 0xC9 ||  // mov ecx, ecx
                code[1] == 0xD2 ||  // mov edx, edx
                code[1] == 0xDB ||  // mov ebx, ebx
                code[1] == 0xE4 ||  // mov esp, esp
                code[1] == 0xFF)    // mov edi, edi
            {
                LSTRIP(code, 2);
                continue;
            }
        }
        // near JMP past random junk
        else if (code[0] == 0xEB)   // JMP +1-0xFF
        {
            // not enough room? clear out anything left and finish
            if (code.size() < 2)
            {
                code.clear();
                return;
            }

            const unsigned int len = static_cast<unsigned int>(code[1]) + 2;

            LSTRIP(code, len);
            continue;
        }
        // far JMP past random junk
        else if (code[0] == 0xE9)
        {
            if (code.size() < 5)
            {
                code.clear();
                return;
            }

            const unsigned int len = *reinterpret_cast<unsigned int *>(&code[1]) + 5;

            LSTRIP(code, len);
            continue;
        }
        // short conditional jumps that always are followed by far jumps to the same place:
        // "JE", "JZ", "JNE", "JNZ", "JG", "JNLE", "JGE", "JNL", "JNGE", "JLE", "JNG", "JO", "JNO", "JP", "JPE", "JNP", "JPO", "JS"
        else if (code[0] == 0x70 ||
            code[0] == 0x71 ||
            code[0] == 0x74 ||
            code[0] == 0x75 ||
            code[0] == 0x78 ||
            code[0] == 0x7A ||
            code[0] == 0x7B ||
            code[0] == 0x7C ||
            code[0] == 0x7D ||
            code[0] == 0x7E ||
            code[0] == 0x7F)
        {
            const unsigned int target = static_cast<unsigned int>(code[1]) + 2;

            unsigned int secondJumpTarget = 0;

            // currently the far jump will come within 12 bytes
            for (auto i = 2u; i < 14; ++i)
            {
                if (code[i] == 0xE9)
                {
                    secondJumpTarget = *reinterpret_cast<unsigned int *>(&code[i + 1]) + 5;
                    break;
                }
                else if (code[i] == 0xEB)
                {
                    secondJumpTarget = static_cast<unsigned int>(code[i + 1]) + 2 + i;
                    break;
                }
            }

            if (secondJumpTarget && secondJumpTarget == target)
            {
                LSTRIP(code, target);
                continue;
            }
        }
        // these are instructions that are not part of the obfuscation.  so for now we will skip them and
        // deobfuscate whatever comes after them.  this will let us build up deobfuscated code.
        // pushfd.  this is currently not part of the obfuscation of wrobot but rather it appears to always be
        // the first non-obfuscated opcode.  therefore, lets skip it and debofuscate what comes after it.
        else if (code[0] == 0x9C || // pushfd
            code[0] == 0x60)   // pushad
        {
            std::vector<std::uint8_t> copy(code.begin() + 1, code.end());
            DeobfuscateAsm(copy);

            code.resize(1);
            code.insert(code.end(), copy.begin(), copy.end());

            // do nothing after this
            return;
        }

        // if we reach here, we haven't made any change.  stop
        break;
    } while (true);
#undef LSTRIP
}

// returns true when the given hook code is suspicious
bool ValidateEndSceneHook(const std::vector<uint8> &code)
{
    auto copy = code;

    // attempt asm deobfuscation to detect wrobot
    DeobfuscateAsm(copy);

    std::stringstream str;

    for (auto i = 0u; i < code.size(); ++i)
    {
        str << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(code[i]);

        if (i < code.size() - 1)
            str << " ";
    }

    sLog.outBasic("WARDEN: Deobfuscate debug.  Original code size: %lu deobfuscated size: %lu Code:\n%s",
        uint64(code.size()), uint64(copy.size()), str.str().c_str());

    // wrobot's deobfuscated endscene hook begins with pushfd, pushad.  if thats what this starts with,
    // assume it is wrobot, regardless of what comes after it.
    if (copy.size() >= 2 && copy[0] == 0x9C && copy[1] == 0x60)
        return true;

    return code.size() == 200 && copy.size() < 15;
}
}

void WardenWin::LoadScriptedScans()
{
    auto offset_flags = GetScanFlagsByAvailableOffsets();

    // sys info locate phase 2
    auto const wardenSysInfo2 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_sysInfo.dwOemId + offsets->OfsWardenWinSysInfo
            << static_cast<uint8>(sizeof(wardenWin->_sysInfo));
    },
        // checker
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read SYSTEM_INFO from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        buff.read(reinterpret_cast<uint8*>(&wardenWin->_sysInfo), sizeof(wardenWin->_sysInfo));

        // for classic, tbc, and wotlk, the architecute should never be anything other than x86 (0)
        if (!!wardenWin->_sysInfo.wProcessorArchitecture)
        {
            sLog.outBasic("Incorrect architecture reported (%u) for account %u ip %s",
                wardenWin->_sysInfo.wProcessorArchitecture, wardenWin->_session->GetAccountId(),
                wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        // for classic, tbc, and wotlk, the cpu type should never be anything other than i386, i486, or pentium (i586)
        if (wardenWin->_sysInfo.dwProcessorType != 386 &&
            wardenWin->_sysInfo.dwProcessorType != 486 &&
            wardenWin->_sysInfo.dwProcessorType != 586)
        {
            sLog.outBasic("Incorrect processor type: %u for account %u ip %s",
                wardenWin->_sysInfo.dwProcessorType, wardenWin->_session->GetAccountId(),
                wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(WIN_SYSTEM_INFO),
        "Sysinfo locate", None);

    // sys info locate phase 1
    auto const wardenSysInfo1 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_wardenAddress + offsets->OfsWardenSysInfo
            << static_cast<uint8>(sizeof(wardenWin->_sysInfo.dwOemId));
    },
        // checker
        [wardenSysInfo2](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read warden->SysInfo from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        // borrow this memory temporarily
        wardenWin->_sysInfo.dwOemId = buff.read<uint32>();

        // immediately third second stage
        wardenWin->EnqueueScans({ wardenSysInfo2 });

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(uint32),
        "Intermediate sysinfo locate", None);

    // find warden module
    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << offsets->WardenModule
            << static_cast<uint8>(sizeof(wardenWin->_wardenAddress));
    },
        // checker
        [wardenSysInfo1](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read s_moduleInterface from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        wardenWin->_wardenAddress = buff.read<uint32>();

        // immediately enqueue second stage
        wardenWin->EnqueueScans({ wardenSysInfo1 });

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(uint32),
        "Warden locate", InitialLogin | offset_flags));

    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << offsets->WorldEnables
            << static_cast<uint8>(sizeof(wardenWin->_wardenAddress));
    },
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read CWorld::enables from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        auto const val = buff.read<uint32>();

        // if any required flags are missing, or prohibited flags are present
        if ((val & Required) != Required || !!(val & Prohibited))
        {
            sLog.outBasic("CWorld::enables expected 0x%ux prohibited 0x%ux received 0x%ux",
                Required, Prohibited, val);

            return true;
        }

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8),
        sizeof(uint32), "CWorld::enables hack", offset_flags));

    // read game time and last hardware action time together
    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        // NOTE: the order of these two scans is important because the client will not perform the checks
        // at the same time, and we want to safely assume that the last hardware action time is not
        // greater than the current time

        // last hardware action time
        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << offsets->LastHardwareAction
            << static_cast<uint8>(sizeof(uint32));

        // game time
        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[CHECK_TIMING_VALUES] ^ wardenWin->GetXor());
    },
        // checker
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));
        auto const session = wardenWin->_session;

        if (!!buff.read<uint8>())
        {
            sLog.outDebug("Account %u timing check failed to read CSimpleTop::m_eventTime",
                session->GetLatency());
            return true;
        }

        auto const lastHardwareAction = buff.read<uint32>();

        auto const timeResult = buff.read<uint8>();
        auto const currentTime = buff.read<uint32>();

        if (!timeResult)
        {
            // the timing check can fail for innocuous reasons like running under wine, within a virtual machine,
            // or in XP compatibility mode.  there are probably other causes too.  therefore let us ignore this
            // failure, since the clock desync check will catch this same case if the clock is moving at an
            // unfair speed.
            sLog.outDebug("Account %u timing check failed", session->GetAccountId());
            return false;
        }

        // last hardware action cannot legitimately be past the current time
        if (lastHardwareAction > currentTime)
        {
            sLog.outBasic("Account: %u Current time: %u Last hardware action: %u (last hardware action in the future)",
                session->GetAccountId(), currentTime, lastHardwareAction);
            return true;
        }

        wardenWin->_lastClientTime = currentTime;
        wardenWin->_lastHardwareActionTime = lastHardwareAction;
        wardenWin->_lastTimeCheckServer = WorldTimer::getMSTime();

        return false;
    }, 11, 10, "Anti-AFK hack", offset_flags));

    // check for hypervisors
    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>& strings, ByteBuffer& scan)
    {
        MANGOS_ASSERT(strings.size() + HypervisorCount < 0xFF);

        auto const winWarden = reinterpret_cast<const WardenWin*>(warden);
        auto const opcode = static_cast<uint8>(winWarden->GetModule()->opcodes[FIND_DRIVER_BY_NAME] ^ winWarden->GetXor());

        for (auto i = 0u; i < HypervisorCount; ++i)
        {
            auto const& hypervisor = Hypervisors[i];

            strings.emplace_back(hypervisor.Driver);

            auto const seed = urand();

            scan << opcode << seed;

            HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
            hash.UpdateData(hypervisor.DeviceName);
            hash.Finalize();

            scan.append(hash.GetDigest(), hash.GetLength());
            scan << static_cast<uint8>(strings.size());
        }
    },
        // checker
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const winWarden = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        winWarden->_hypervisors = "";

        for (auto i = 0u; i < HypervisorCount; ++i)
        {
            auto const found = buff.read<uint8>() == 0x4A;

            if (!found)
                continue;

            winWarden->_hypervisors += Hypervisors[i].Name;
        }

        // always return false because there is nothing necessary wrong with using a hypervisor
        return false;
    },
        // TODO: Replace the magic number below with combined driver string lengths
        (sizeof(uint8) + sizeof(uint32) + Sha1Hash::GetLength() + sizeof(uint8)) * HypervisorCount + 21,
        sizeof(uint8) * HypervisorCount,
        "Hypervisor check",
        WinAllBuild | InitialLogin));

    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>& strings, ByteBuffer& scan)
    {
        auto const seed = urand();

        scan << static_cast<uint8>(warden->GetModule()->opcodes[FIND_CODE_BY_HASH] ^ warden->GetXor())
            << seed;

        static constexpr std::uint8_t pattern[] =
        {
            0x56, 0x57, 0xFC, 0x8B, 0x54, 0x24, 0x14, 0x8B,
            0x74, 0x24, 0x10, 0x8B, 0x44, 0x24, 0x0C, 0x8B,
            0xCA, 0x8B, 0xF8, 0xC1, 0xE9, 0x02, 0x74, 0x02,
            0xF3, 0xA5, 0xB1, 0x03, 0x23, 0xCA, 0x74, 0x02,
            0xF3, 0xA4, 0x5F, 0x5E, 0xC3
        };

        static_assert(sizeof(pattern) <= 0xFF, "pattern length must fit into 8 bits");

        HMACSHA1 hash(sizeof(seed), reinterpret_cast<const uint8*>(&seed));
        hash.UpdateData(&pattern[0], sizeof(pattern));
        hash.Finalize();

        scan.append(hash.GetDigest(), hash.GetLength());

        scan << warden->GetModule()->memoryRead << static_cast<uint8>(sizeof(pattern));
    },
        // checker
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const found = buff.read<uint8>() == WindowsCodeScan::PatternFound;

        // if this is not found, it means someone has tampered with the function
        return !found;
    }, sizeof(uint8) + sizeof(uint32) + Sha1Hash::GetLength() + sizeof(uint32) + sizeof(uint8), sizeof(uint8),
        "Warden Memory Read check",
        WinAllBuild));

    // end scene hook check 1
    static constexpr uint8 endSceneReadSize = 16u;
    auto const endSceneHookCheck1 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);

        // if we have not found EndScene, do nothing
        if (!wardenWin->_endSceneFound)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_endSceneAddress
            << endSceneReadSize;
    },
        // checker
        [](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read EndScene (hook check stage 1) from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        std::vector<uint8> code(endSceneReadSize);
        buff.read(&code[0], code.size());

        wardenWin->ValidateEndScene(code);

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + endSceneReadSize,
        "EndScene hook check stage 1", WinAllBuild);

    sWardenScanMgr.AddWindowsScan(endSceneHookCheck1);

    // end scene locate phase 4
    auto const endSceneLocate4 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_endSceneAddress + offsets->OfsDevice4
            << static_cast<uint8>(sizeof(uint32));
    },
        // checker
        [endSceneHookCheck1](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read EndScene (stage 4) from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        wardenWin->_endSceneAddress = buff.read<uint32>();
        wardenWin->_endSceneFound = true;

        // immediately request hook check
        wardenWin->EnqueueScans({ endSceneHookCheck1 });

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(uint32),
        "EndScene locate stage 4", None);

    // end scene locate phase 3
    auto const endSceneLocate3 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_endSceneAddress + offsets->OfsDevice3
            << static_cast<uint8>(sizeof(uint32));
    },
        // checker
        [endSceneLocate4](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read EndScene (stage 3) from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        wardenWin->_endSceneAddress = buff.read<uint32>();

        // immediately request fourth stage
        wardenWin->EnqueueScans({ endSceneLocate4 });

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(uint32),
        "EndScene locate stage 3", None);

    // end scene locate phase 2
    auto const endSceneLocate2 = std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << wardenWin->_endSceneAddress + offsets->OfsDevice2
            << static_cast<uint8>(sizeof(uint32));
    },
        // checker
        [endSceneLocate3](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read EndScene (stage 2) from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        wardenWin->_endSceneAddress = buff.read<uint32>();

        // immediately request third stage
        wardenWin->EnqueueScans({ endSceneLocate3 });

        return false;
    }, sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8), sizeof(uint8) + sizeof(uint32),
        "EndScene locate stage 2", None);

    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsScan>(
        // builder
        [](const Warden* warden, std::vector<std::string>&, ByteBuffer& scan)
    {
        auto const wardenWin = reinterpret_cast<const WardenWin*>(warden);
        auto const offsets = GetClientOffsets(wardenWin->_session->GetGameBuild());

        if (!offsets)
            return;

        scan << static_cast<uint8>(wardenWin->GetModule()->opcodes[READ_MEMORY] ^ wardenWin->GetXor())
            << static_cast<uint8>(0)
            << offsets->g_theGxDevicePtr
            << static_cast<uint8>(sizeof(uint32));
    },
        // checker
        [endSceneLocate2](const Warden* warden, ByteBuffer& buff)
    {
        auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));

        auto const result = buff.read<uint8>();

        if (!!result)
        {
            sLog.outBasic("Failed to read g_theGxDevicePtr from account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

            return true;
        }

        buff.read(reinterpret_cast<uint8*>(&wardenWin->_endSceneAddress), sizeof(wardenWin->_endSceneAddress));

        // if for some reason we get nullptr, abort
        if (!wardenWin->_endSceneAddress)
        {
            sLog.outBasic("g_theGxDevicePtr is nullptr for account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());
            return true;
        }

        // immediately request second stage
        wardenWin->EnqueueScans({ endSceneLocate2 });

        return false;
    },
        sizeof(uint8) + sizeof(uint8) + sizeof(uint32) + sizeof(uint8),
        sizeof(uint8) + sizeof(uint32),
        "EndScene locate stage 1", InitialLogin | offset_flags));

    sWardenScanMgr.AddWindowsScan(std::make_shared<WindowsModuleScan>("prxdrvpe.dll",
        // checker
        Scan::CheckT([](const Warden* warden, ByteBuffer& buff)
    {
        if (buff.read<uint8>() == WindowsModuleScan::ModuleFound)
        {
            auto const wardenWin = const_cast<WardenWin*>(reinterpret_cast<const WardenWin*>(warden));
            wardenWin->_proxifierFound = true;

            sLog.outBasic("Proxifier found on account %u ip %s",
                wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());
        }

        return false;
    }), "Proxifier check", WinAllBuild | InitialLogin));
}

void WardenWin::BuildLuaInit(const std::string &module, bool fastcall, uint32 offset, ByteBuffer &out) const
{
    const uint16 len = 1 + 1 + 1 + module.length() + 4 + 1;

    out = ByteBuffer(1 + 2 + 4 + len);

    out << static_cast<uint8>(WARDEN_SMSG_MODULE_INITIALIZE)
        << static_cast<uint16>(len)
        << static_cast<uint32>(0)   // checksum placeholder
        << static_cast<uint8>(4)
        << static_cast<uint8>(0)
        << static_cast<uint8>(module.length());

    // avoid writing null terminator
    out.append(module.c_str(), module.length());

    out << offset
        << static_cast<uint8>(fastcall ? 0 : 1);

    // write checksum
    auto const oldwpos = out.wpos();
    out.wpos(3);
    out << BuildChecksum(out.contents() + 7, len);
    out.wpos(oldwpos);
}

void WardenWin::BuildFileHashInit(const std::string &module, bool asyncparam, uint32 openOffset,
    uint32 sizeOffset, uint32 readOffset, uint32 closeOffset, ByteBuffer &out) const
{
    const uint16 len = 1 + 1 + 1 + 1 + module.length() + 4 + 4 + 4 + 4;

    out = ByteBuffer(1 + 2 + 4 + len);

    out << static_cast<uint8>(WARDEN_SMSG_MODULE_INITIALIZE)
        << static_cast<uint16>(len)
        << static_cast<uint32>(0)   // checksum placeholder
        << static_cast<uint8>(1)
        << static_cast<uint8>(0)
        << static_cast<uint8>(asyncparam ? 2 : 1)
        << static_cast<uint8>(module.length());

    // avoid writing null terminator
    out.append(module.c_str(), module.length());

    out << openOffset
        << sizeOffset
        << readOffset
        << closeOffset;

    // write checksum
    auto const oldwpos = out.wpos();
    out.wpos(3);
    out << BuildChecksum(out.contents() + 7, len);
    out.wpos(oldwpos);
}

void WardenWin::BuildTimingInit(const std::string &module, uint32 offset, bool set, ByteBuffer &out) const
{
    const uint16 len = 1 + 1 + 1 + module.length() + 4 + 1;

    out = ByteBuffer(1 + 2 + 4 + len);

    out << static_cast<uint8>(WARDEN_SMSG_MODULE_INITIALIZE)
        << static_cast<uint16>(len)
        << static_cast<uint32>(0)   // checksum placeholder
        << static_cast<uint8>(1)
        << static_cast<uint8>(1)
        << static_cast<uint8>(module.length());

    // avoid writing null terminator
    out.append(module.c_str(), module.length());

    out << offset << static_cast<uint8>(set ? 1 : 0);

    // write checksum
    auto const oldwpos = out.wpos();
    out.wpos(3);
    out << BuildChecksum(out.contents() + 7, len);
    out.wpos(oldwpos);
}

WardenWin::WardenWin(WorldSession *session, const BigNumber &K, SessionAnticheatInterface *anticheat) :
    Warden(session, sWardenModuleMgr.GetWindowsModule(), K, anticheat), _wardenAddress(0),
    _sysInfoSaved(false), _proxifierFound(false), _hypervisors(""), _lastClientTime(0),
    _lastHardwareActionTime(0), _lastTimeCheckServer(0), _endSceneFound(false), _endSceneAddress(0)
{
    memset(&_sysInfo, 0, sizeof(_sysInfo));
}

// read the dx9 EndScene binary code to look for bad stuff
void WardenWin::ValidateEndScene(const std::vector<uint8> &code)
{
    auto p = &code[0];

    // skip any NOPs
    while (*p == 0x90) ++p;

    auto const nopCount = static_cast<int>(p - &code[0]);

    static constexpr size_t codeRequestLength = 200u;

    // int3 breakpoint
    if (*p == 0xCC)
    {
        sLog.outBasic("WARDEN: Detected INT3 EndScene hook for account %u IP %s (NOP count = %d)",
            _session->GetAccountId(), _session->GetRemoteAddress().c_str(), nopCount);
    }
    // JMP hook
    else if (*p == 0xE9)
    {
        auto const dest = *reinterpret_cast<const uint32 *>(p + 1);

        auto const absoluteDest = _endSceneAddress + nopCount + dest + 5;
        sLog.outBasic("WARDEN: Detected JMP EndScene hook for account %u IP %s (NOP count = %d)",
            _session->GetAccountId(), _session->GetRemoteAddress().c_str(), nopCount);

        // request a custom scan just to check the JMP destination
        EnqueueScans({ std::make_shared<WindowsMemoryScan>(absoluteDest, codeRequestLength,
        // checker
        [](const Warden *warden, ByteBuffer &buff)
        {
            auto const wardenWin = const_cast<WardenWin *>(reinterpret_cast<const WardenWin *>(warden));

            auto const result = buff.read<uint8>();

            if (!!result)
            {
                sLog.outBasic("WARDEN: Failed to read EndScene hook code from account %u ip %s",
                    wardenWin->_session->GetAccountId(), wardenWin->_session->GetRemoteAddress().c_str());

                return true;
            }

            std::vector<uint8> code(codeRequestLength);

            buff.read(&code[0], code.size());

            if (ValidateEndSceneHook(code) && !!sAnticheatConfig.GetWardenSuspiciousEndSceneHookAction())
                wardenWin->_anticheat->RecordCheat(sAnticheatConfig.GetWardenSuspiciousEndSceneHookAction(),
                    "Warden", "Suspicious client heuristic results.  Probable bot.");

            return false;
        }, "EndScene hook validate scan", None) });
    }
}

uint32 WardenWin::GetScanFlags() const
{
    uint32 result = _session->GetAccountMaxLevel() >= sAnticheatConfig.GetWardenMinimumAdvancedLevel()
        ? ScanFlags::AdvancedScan : ScanFlags::None;

    switch (_session->GetGameBuild())
    {
        case 5875:
            return ScanFlags::WinBuild5875;
        case 6005:
            return ScanFlags::WinBuild6005;
        case 6141:
            return ScanFlags::WinBuild6141;
        default:
            sLog.outBasic("WARDEN: Invalid client build %u for account %u", _session->GetGameBuild(), _session->GetAccountId());
            _session->KickPlayer();
            return ScanFlags::None;
    }

    return result;
}

void WardenWin::InitializeClient()
{
    if (auto const offsets = GetClientOffsets(_session->GetGameBuild()))
    {
        // initialize lua
        ByteBuffer lua;
        BuildLuaInit("", true, offsets->GetText, lua);

        // initialize SFile*
        ByteBuffer file;
        BuildFileHashInit("", true, offsets->Open, offsets->Size, offsets->Read, offsets->Close, file);

        // initialize timing check
        ByteBuffer timing;
        BuildTimingInit("", offsets->TickCount, true, timing);

        ByteBuffer pkt(lua.wpos() + file.wpos() + timing.wpos());

        pkt.append(lua);
        pkt.append(file);
        pkt.append(timing);

        SendPacket(pkt);
    }

    _initialized = true;
}

void WardenWin::Update(uint32 diff)
{
    Warden::Update(diff);

    if (!_initialized)
        return;

    // 'lpMaximumApplicationAddress' should never be zero if the structure has been read
    if (!_sysInfoSaved && !!_sysInfo.lpMaximumApplicationAddress)
    {
        auto activeProcCount = 0;
        for (auto i = 0; i < 8 * sizeof(_sysInfo.dwActiveProcessorMask); ++i)
            if (!!(_sysInfo.dwActiveProcessorMask & (1 << i)))
                ++activeProcCount;

        LoginDatabase.BeginTransaction();

        static SqlStatementID fingerprintUpdate;

        auto stmt = LoginDatabase.CreateStatement(fingerprintUpdate,
            "INSERT INTO system_fingerprint_usage (fingerprint, account, ip, realm, architecture, cputype, activecpus, totalcpus, pagesize) "
            "VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");

        stmt.addUInt32(_anticheat->GetFingerprint());
        stmt.addUInt32(_session->GetAccountId());
        stmt.addString(_session->GetRemoteAddress());
        stmt.addUInt32(realmID);
        stmt.addString(ArchitectureString(_sysInfo.wProcessorArchitecture));
        stmt.addString(CPUTypeAndRevision(_sysInfo.dwProcessorType, _sysInfo.wProcessorRevision));
        stmt.addUInt32(activeProcCount);
        stmt.addUInt32(_sysInfo.dwNumberOfProcessors);
        stmt.addUInt32(_sysInfo.dwPageSize);
        stmt.Execute();

        LoginDatabase.CommitTransaction();

        _anticheat->CleanupFingerprintHistory();

        _sysInfoSaved = true;

        // at this point if we have the character enum packet, it is okay to send
        if (!_charEnum.empty())
        {
            _session->SendPacket(_charEnum, true);
            _charEnum.clear();
        }
    }
}

void WardenWin::SetCharEnumPacket(WorldPacket &&packet)
{
    // if we have already recorded system information, send the packet immediately.  otherwise delay
    if (_sysInfoSaved)
        _session->SendPacket(packet, true);
    else
        _charEnum = std::move(packet);
}

void WardenWin::SendPlayerInfo(ChatHandler *handler, bool includeFingerprint) const
{
    if (!!_lastTimeCheckServer)
    {
        handler->PSendSysMessage("Last hardware action: %u client time: %u idle time: %u seconds info age: %u seconds",
            _lastHardwareActionTime, _lastClientTime,
            (_lastClientTime - _lastHardwareActionTime) / 1000,
            (WorldTimer::getMSTime() - _lastTimeCheckServer) / 1000);
    }

    // 'lpMaximumApplicationAddress' should never be zero if the structure has been read
    if (includeFingerprint && !!_sysInfo.lpMaximumApplicationAddress)
    {
        std::stringstream str;

        str << "Architecture: " << ArchitectureString(_sysInfo.wProcessorArchitecture)
            << " CPU Type: " << CPUTypeAndRevision(_sysInfo.dwProcessorType, _sysInfo.wProcessorRevision)
            << " Page Size: 0x" << std::hex << std::uppercase << _sysInfo.dwPageSize << std::dec;

        auto activeProcCount = 0;
        for (auto i = 0; i < 8*sizeof(_sysInfo.dwActiveProcessorMask); ++i)
            if (!!(_sysInfo.dwActiveProcessorMask & (1 << i)))
                ++activeProcCount;

        str << " Active CPUs: " << activeProcCount;
        str << " Total CPUs: " << _sysInfo.dwNumberOfProcessors;

        handler->SendSysMessage(str.str().c_str());
    }

    if (_hypervisors.length() > 0)
        handler->PSendSysMessage("Hypervisor(s) found: %s", _hypervisors.c_str());

    if (_endSceneFound)
        handler->PSendSysMessage("EndScene: 0x%lx", uint64(_endSceneAddress));

    if (_proxifierFound)
        handler->SendSysMessage("Proxifier is running");
}
