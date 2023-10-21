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

#include "Server/WorldSocket.h"
#include "Common.h"

#include "Util/Util.h"
#include "World/World.h"
#include "Server/WorldPacket.h"
#include "Globals/SharedDefines.h"
#include "Util/ByteBuffer.h"
#include "Addons/AddonHandler.h"
#include "Server/Opcodes.h"
#include "Server/PacketLog.h"
#include "Database/DatabaseEnv.h"
#include "Auth/CryptoHash.h"
#include "Server/WorldSession.h"
#include "Log.h"
#include "Server/DBCStores.h"
#include "Util/CommonDefines.h"
#include "Anticheat/Anticheat.hpp"

#include <chrono>
#include <functional>
#include <memory>

#include <boost/asio.hpp>
#include <utility>

#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif
struct ServerPktHeader
{
    uint16 size;
    uint16 cmd;
};
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

std::vector<uint32> InitOpcodeCooldowns()
{
    std::vector<uint32> data(NUM_MSG_TYPES, 0);

    data[CMSG_WHO] = 5000;
    data[CMSG_WHOIS] = 5000;
    data[CMSG_INSPECT] = 5000;

    return data;
}

std::vector<uint32> WorldSocket::m_packetCooldowns = InitOpcodeCooldowns();

std::deque<uint32> WorldSocket::GetOutOpcodeHistory()
{
    std::lock_guard<std::mutex> guard(m_worldSocketMutex);
    return m_opcodeHistoryOut;
}

std::deque<uint32> WorldSocket::GetIncOpcodeHistory()
{
    std::lock_guard<std::mutex> guard(m_worldSocketMutex);
    return m_opcodeHistoryInc;
}

WorldSocket::WorldSocket(boost::asio::io_service& service, std::function<void (Socket*)> closeHandler) : Socket(service, std::move(closeHandler)), m_lastPingTime(std::chrono::system_clock::time_point::min()), m_overSpeedPings(0), m_existingHeader(),
    m_useExistingHeader(false), m_session(nullptr), m_seed(urand()), m_loggingPackets(false)
{
}

void WorldSocket::SendPacket(const WorldPacket& pct, bool immediate)
{
    if (IsClosed())
        return;

    if (sPacketLog->CanLogPacket() && IsLoggingPackets())
        sPacketLog->LogPacket(pct, SERVER_TO_CLIENT, GetRemoteIpAddress(), GetRemotePort());

    // Dump outgoing packet.
    sLog.outWorldPacketDump(GetRemoteEndpoint().c_str(), pct.GetOpcode(), pct.GetOpcodeName(), pct, false);

    // encrypt thread unsafe due to being executed from map contexts frequently - TODO: move to post service context in future
    std::lock_guard<std::mutex> guard(m_worldSocketMutex);

    ServerPktHeader header;

    header.cmd = pct.GetOpcode();
    EndianConvert(header.cmd);

    header.size = static_cast<uint16>(pct.size() + 2);
    EndianConvertReverse(header.size);

    m_crypt.EncryptSend(reinterpret_cast<uint8*>(&header), sizeof(header));

    if (pct.size() > 0)
        Write(reinterpret_cast<const char*>(&header), sizeof(header), reinterpret_cast<const char*>(pct.contents()), pct.size());
    else
        Write(reinterpret_cast<const char*>(&header), sizeof(header));

    if (immediate)
        ForceFlushOut();

    m_opcodeHistoryOut.push_front(uint32(pct.GetOpcode()));
    if (m_opcodeHistoryOut.size() > 50)
        m_opcodeHistoryOut.resize(30);
}

bool WorldSocket::Open()
{
    if (!Socket::Open())
        return false;

    // Send startup packet.
    WorldPacket packet(SMSG_AUTH_CHALLENGE, 40);
    packet << m_seed;

    BigNumber seed1;
    seed1.SetRand(16 * 8);
    packet.append(seed1.AsByteArray(16).data(), 16);               // new encryption seeds

    BigNumber seed2;
    seed2.SetRand(16 * 8);
    packet.append(seed2.AsByteArray(16).data(), 16);               // new encryption seeds

    SendPacket(packet);

    return true;
}

bool WorldSocket::ProcessIncomingData()
{
    ClientPktHeader header;

    if (m_useExistingHeader)
    {
        m_useExistingHeader = false;
        header = m_existingHeader;

        ReadSkip(sizeof(ClientPktHeader));
    }
    else
    {
        if (!Read((char*)&header, sizeof(ClientPktHeader)))
        {
            errno = EBADMSG;
            return false;
        }

        // thread safe due to always being called from service context
        m_crypt.DecryptRecv((uint8*)&header, sizeof(ClientPktHeader));

        EndianConvertReverse(header.size);
        EndianConvert(header.cmd);
    }

    // there must always be at least four bytes for the opcode,
    // and 0x2800 is the largest supported buffer in the client
    if ((header.size < 4) || (header.size > 0x2800) || (header.cmd >= NUM_MSG_TYPES))
    {
        sLog.outError("WorldSocket::ProcessIncomingData: client sent malformed packet size = %u , cmd = %u", header.size, header.cmd);

        errno = EINVAL;
        return false;
    }

    // the minus four is because we've already read the four byte opcode value
    const uint16 validBytesRemaining = header.size - 4;

    // check if the client has told us that there is more data than there is
    if (validBytesRemaining > ReadLengthRemaining())
    {
        // we must preserve the decrypted header so as not to corrupt the crypto state, and to prevent duplicating work
        m_useExistingHeader = true;
        m_existingHeader = header;

        // we move the read pointer backward because it will be skipped again later.  this is a slight kludge, but to solve
        // it more elegantly would require introducing protocol awareness into the socket library, which we want to avoid
        ReadSkip(-static_cast<int>(sizeof(ClientPktHeader)));

        errno = EBADMSG;
        return false;
    }

    const Opcodes opcode = static_cast<Opcodes>(header.cmd);

    if (IsClosed())
        return false;

    std::unique_ptr<WorldPacket> pct(new WorldPacket(opcode, validBytesRemaining));

    if (validBytesRemaining)
    {
        pct->append(InPeak(), validBytesRemaining);
        ReadSkip(validBytesRemaining);
    }

    if (sPacketLog->CanLogPacket() && IsLoggingPackets())
        sPacketLog->LogPacket(*pct, CLIENT_TO_SERVER, GetRemoteIpAddress(), GetRemotePort());

    sLog.outWorldPacketDump(GetRemoteEndpoint().c_str(), pct->GetOpcode(), pct->GetOpcodeName(), *pct, true);

    if (WorldSocket::m_packetCooldowns[opcode])
    {
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(Clock::now());
        if (now < m_lastPacket[opcode]) // packet on cooldown
            return true;
        else // start cooldown and allow execution
            m_lastPacket[opcode] = now + std::chrono::milliseconds(WorldSocket::m_packetCooldowns[opcode]);
    }

    try
    {
        switch (opcode)
        {
            case CMSG_AUTH_SESSION:
                if (m_session)
                {
                    sLog.outError("WorldSocket::ProcessIncomingData: Player send CMSG_AUTH_SESSION again");
                    return false;
                }

                return HandleAuthSession(*pct);

            case CMSG_PING:
                return HandlePing(*pct);

            case CMSG_KEEP_ALIVE:
                DEBUG_LOG("CMSG_KEEP_ALIVE ,size: " SIZEFMTD " ", pct->size());

                return true;

            default:
            {
                m_opcodeHistoryInc.push_front(uint32(pct->GetOpcode()));
                if (m_opcodeHistoryInc.size() > 50)
                    m_opcodeHistoryInc.resize(30);

                if (!m_session)
                {
                    sLog.outError("WorldSocket::ProcessIncomingData: Client not authed opcode = %u", uint32(opcode));
                    return false;
                }

                m_session->QueuePacket(std::move(pct));

                return true;
            }
        }
    }
    catch (ByteBufferException&)
    {
        sLog.outError("WorldSocket::ProcessIncomingData ByteBufferException occured while parsing an instant handled packet (opcode: %u) from client %s, accountid=%i.",
                      opcode, GetRemoteAddress().c_str(), m_session ? m_session->GetAccountId() : -1);

        if (sLog.HasLogLevelOrHigher(LOG_LVL_DEBUG))
        {
            DEBUG_LOG("Dumping error-causing packet:");
            pct->hexlike();
        }

        if (sWorld.getConfig(CONFIG_BOOL_KICK_PLAYER_ON_BAD_PACKET))
        {
            DETAIL_LOG("Disconnecting session [account id %i / address %s] for badly formatted packet.",
                       m_session ? m_session->GetAccountId() : -1, GetRemoteAddress().c_str());
            return false;
        }
    }

    return true;
}

bool WorldSocket::HandleAuthSession(WorldPacket& recvPacket)
{
    // NOTE: ATM the socket is singlethread, have this in mind ...
    uint8 digest[20];
    uint32 clientSeed;
    uint32 ClientBuild;
    uint32 unk2;
    LocaleConstant locale;
    std::string account, os;
    Sha1Hash sha1;
    BigNumber v, s, g, N, K;
    WorldPacket packet, SendAddonPacked;

    // Read the content of the packet
    recvPacket >> ClientBuild;
    recvPacket >> unk2;
    recvPacket >> account;
    recvPacket >> clientSeed;
    recvPacket.read(digest, 20);

    DEBUG_LOG("WorldSocket::HandleAuthSession: client build %u, account %s, clientseed %X",
              ClientBuild,
              account.c_str(),
              clientSeed);

    // Check the version of client trying to connect
    if (!IsAcceptableClientBuild(ClientBuild))
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_VERSION_MISMATCH);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (version mismatch).");
        return false;
    }

    // Get the account information from the realmd database
    std::string safe_account = account; // Duplicate, else will screw the SHA hash verification below
    LoginDatabase.escape_string(safe_account);
    // No SQL injection, username escaped.

    auto queryResult =
        LoginDatabase.PQuery("SELECT "
                             "a.id, "                    //0
                             "gmlevel, "                 //1
                             "sessionkey, "              //2
                             "lockedIp, "                //3
                             "locked, "                  //4
                             "v, "                       //5
                             "s, "                       //6
                             "mutetime, "                //7
                             "locale, "                  //8
                             "os, "                      //9
                             "flags, "                   //10
                             "platform "                 //11
                             "FROM account a "
                             "WHERE username = '%s'",
                             safe_account.c_str());

    // Stop if the account is not found
    if (!queryResult)
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_UNKNOWN_ACCOUNT);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (unknown account).");
        return false;
    }

    Field* fields = queryResult->Fetch();

    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);

    v.SetHexStr(fields[5].GetString());
    s.SetHexStr(fields[6].GetString());
    m_s = s;

    const char* sStr = s.AsHexStr();                        // Must be freed by OPENSSL_free()
    const char* vStr = v.AsHexStr();                        // Must be freed by OPENSSL_free()

    DEBUG_LOG("WorldSocket::HandleAuthSession: (s,v) check s: %s v: %s",
              sStr,
              vStr);

    OPENSSL_free((void*) sStr);
    OPENSSL_free((void*) vStr);

    ///- Re-check ip locking (same check as in realmd).
    if (fields[4].GetUInt8() == 1)  // if ip is locked
    {
        if (strcmp(fields[3].GetString(), GetRemoteAddress().c_str()))
        {
            packet.Initialize(SMSG_AUTH_RESPONSE, 1);
            packet << uint8(AUTH_FAILED);
            SendPacket(packet);

            BASIC_LOG("WorldSocket::HandleAuthSession: Sent Auth Response (Account IP differs).");
            return false;
        }
    }

    uint32 id = fields[0].GetUInt32();
    uint32 security = fields[1].GetUInt16();
    if (security > SEC_ADMINISTRATOR)                       // prevent invalid security settings in DB
        security = SEC_ADMINISTRATOR;

    K.SetHexStr(fields[2].GetString());

    time_t mutetime = time_t (fields[7].GetUInt64());

    locale = GetLocaleByName(fields[8].GetString());

    os = fields[9].GetString();

    uint32 accountFlags = fields[10].GetUInt32();
    std::string platform = fields[11].GetString();

    // Re-check account ban (same check as in realmd)
    auto banresult =
        LoginDatabase.PQuery("SELECT 1 FROM account_banned WHERE account_id = %u AND active = 1 AND (expires_at > UNIX_TIMESTAMP() OR expires_at = banned_at)"
                             "UNION "
                             "SELECT 1 FROM ip_banned WHERE (expires_at = banned_at OR expires_at > UNIX_TIMESTAMP()) AND ip = '%s'",
                             id, GetRemoteAddress().c_str());

    if (banresult) // if account banned
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_BANNED);
        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (Account banned).");
        return false;
    }

    // Check locked state for server
    AccountTypes allowedAccountType = sWorld.GetPlayerSecurityLimit();

    if (allowedAccountType > SEC_PLAYER && AccountTypes(security) < allowedAccountType)
    {
        WorldPacket Packet(SMSG_AUTH_RESPONSE, 1);
        Packet << uint8(AUTH_UNAVAILABLE);

        SendPacket(packet);

        BASIC_LOG("WorldSocket::HandleAuthSession: User tries to login but his security level is not enough");
        return false;
    }

    // Check that Key and account name are the same on client and server
    Sha1Hash sha;

    uint32 t = 0;
    uint32 seed = m_seed;

    sha.UpdateData(account);
    sha.UpdateData((uint8*) & t, 4);
    sha.UpdateData((uint8*) & clientSeed, 4);
    sha.UpdateData((uint8*) & seed, 4);
    sha.UpdateBigNumbers(&K, nullptr);
    sha.Finalize();

    if (memcmp(sha.GetDigest(), digest, 20))
    {
        packet.Initialize(SMSG_AUTH_RESPONSE, 1);
        packet << uint8(AUTH_FAILED);

        SendPacket(packet);

        sLog.outError("WorldSocket::HandleAuthSession: Sent Auth Response (authentification failed), account ID: %u.", id);
        return false;
    }

    const std::string& address = GetRemoteAddress();

    DEBUG_LOG("WorldSocket::HandleAuthSession: Client '%s' authenticated successfully from %s.",
              account.c_str(),
              address.c_str());

    ClientOSType clientOS;
    if (os == "Win")
        clientOS = CLIENT_OS_WIN;
    else if (os == "OSX")
        clientOS = CLIENT_OS_MAC;
    else
    {
        sLog.outError("WorldSocket::HandleAuthSession: Unrecognized OS '%s' for account '%s' from %s", os.c_str(), account.c_str(), address.c_str());
        return false;
    }

    // Update the last_ip in the database
    // No SQL injection, username escaped.
    static SqlStatementID updAccount;

    SqlStatement stmt = LoginDatabase.CreateStatement(updAccount, "INSERT INTO account_logons(accountId,ip,loginTime,loginSource) VALUES(?,?,NOW(),?)");
    stmt.PExecute(id, address.c_str(), std::to_string(LOGIN_TYPE_MANGOSD).c_str());

    m_crypt.Init(&K);

    m_session = sWorld.FindSession(id);

    ClientPlatformType clientPlatform;
    if (platform == "x86")
        clientPlatform = CLIENT_PLATFORM_X86;
    else if (platform == "PPC" && clientOS == CLIENT_OS_MAC)
        clientPlatform = CLIENT_PLATFORM_PPC;
    else
    {
        sLog.outError("WorldSocket::HandleAuthSession: Unrecognized platform '%s' for account '%s' from %s", platform.c_str(), account.c_str(), address.c_str());
        return false;
    }

    if (m_session)
    {
        // Session exist so player is reconnecting
        // check if we can request a new socket
        if (!m_session->RequestNewSocket(this))
            return false;

        uint32 counter = 0;

        // wait session going to be ready
        while (m_session->GetState() != WORLD_SESSION_STATE_CHAR_SELECTION)
        {
            ++counter;
            // just wait
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (IsClosed() || counter > 20)
                return false;
        }

        m_session->SetGameBuild(ClientBuild);
        m_session->SetOS(clientOS);
        m_session->SetPlatform(clientPlatform);

        std::unique_ptr<SessionAnticheatInterface> anticheat = sAnticheatLib->NewSession(m_session, K);

        // when false, the client sent invalid addon data.  kick!
        WorldPacket addonPacket; // yes its copypasted atm cos of reconnect
        if (!anticheat->ReadAddonInfo(&recvPacket, addonPacket))
        {
            sLog.outBasic("WorldSocket::HandleAuthSession: Account %s (id %u) IP %s sent bad addon info.  Kicking.",
                account.c_str(), id, address.c_str());
            return false;
        }
        else
            SendPacket(addonPacket);

        m_session->SetDelayedAnticheat(std::move(anticheat));
        m_session->GetMessager().AddMessage([](WorldSession* session)
        {
            session->AssignAnticheat();
        });
    }
    else
    {
        // new session
        if (!(m_session = new WorldSession(id, this, AccountTypes(security), mutetime, locale, account, accountFlags)))
            return false;

        m_session->LoadGlobalAccountData();
        m_session->LoadTutorialsData();
        m_session->SetGameBuild(ClientBuild);
        m_session->SetOS(clientOS);
        m_session->SetPlatform(clientPlatform);
        m_session->InitializeAnticheat(K);

        // when false, the client sent invalid addon data.  kick!
        WorldPacket addonPacket;
        if (!m_session->GetAnticheat()->ReadAddonInfo(&recvPacket, addonPacket))
        {
            sLog.outBasic("WorldSocket::HandleAuthSession: Account %s (id %u) IP %s sent bad addon info.  Kicking.",
                account.c_str(), id, address.c_str());
            return false;
        }
        else
            SendPacket(addonPacket);

        sWorld.AddSession(m_session);
    }

    return true;
}

bool WorldSocket::HandlePing(WorldPacket& recvPacket)
{
    uint32 ping;
    uint32 latency;

    // Get the ping packet content
    recvPacket >> ping;
    recvPacket >> latency;

    if (m_lastPingTime == std::chrono::system_clock::time_point::min())
        m_lastPingTime = std::chrono::system_clock::now();              // for 1st ping
    else
    {
        auto now = std::chrono::system_clock::now();
        std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastPingTime);
        m_lastPingTime = now;

        if (seconds.count() < 27)
        {
            ++m_overSpeedPings;

            const uint32 max_count = sWorld.getConfig(CONFIG_UINT32_MAX_OVERSPEED_PINGS);

            if (max_count && m_overSpeedPings > max_count)
            {
                if (m_session && m_session->GetSecurity() == SEC_PLAYER)
                {
                    sLog.outError("WorldSocket::HandlePing: Player kicked for "
                                  "overspeeded pings address = %s",
                                  GetRemoteAddress().c_str());
                    return false;
                }
            }
        }
        else
            m_overSpeedPings = 0;
    }

    // critical section
    {
        if (m_session)
        {
            m_session->SetLatency(latency);
            m_session->ResetClientTimeDelay();
        }
        else
        {
            sLog.outError("WorldSocket::HandlePing: peer sent CMSG_PING, "
                          "but is not authenticated or got recently kicked,"
                          " address = %s",
                          GetRemoteAddress().c_str());
            return false;
        }
    }

    WorldPacket packet(SMSG_PONG, 4);
    packet << ping;
    SendPacket(packet, true);

    return true;
}
