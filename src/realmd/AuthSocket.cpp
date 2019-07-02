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

/** \file
    \ingroup realmd
*/

#include "Common.h"
#include "Auth/HMACSHA1.h"
#include "Auth/base32.h"
#include "Database/DatabaseEnv.h"
#include "Config/Config.h"
#include "Log.h"
#include "RealmList.h"
#include "AuthSocket.h"
#include "AuthCodes.h"

#include <openssl/md5.h>
#include <ctime>
#include <utility>

//#include "Util.h" -- for commented utf8ToUpperOnlyLatin

extern DatabaseType LoginDatabase;

enum AccountFlags
{
    ACCOUNT_FLAG_GM         = 0x00000001,
    ACCOUNT_FLAG_TRIAL      = 0x00000008,
    ACCOUNT_FLAG_PROPASS    = 0x00800000,
};

enum SecurityFlags
{
    SECURITY_FLAG_NONE          = 0x00,
    SECURITY_FLAG_PIN           = 0x01,
    SECURITY_FLAG_UNK           = 0x02,
    SECURITY_FLAG_AUTHENTICATOR = 0x04
};

// GCC have alternative #pragma pack(N) syntax and old gcc version not support pack(push,N), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack(1)
#else
#pragma pack(push,1)
#endif

typedef struct AUTH_LOGON_CHALLENGE_C
{
    uint8   cmd;
    uint8   error;
    uint16  size;
    uint8   gamename[4];
    uint8   version1;
    uint8   version2;
    uint8   version3;
    uint16  build;
    uint8   platform[4];
    uint8   os[4];
    uint8   country[4];
    uint32  timezone_bias;
    uint32  ip;
    uint8   I_len;
    uint8   I[1];
} sAuthLogonChallenge_C;

typedef struct AUTH_LOGON_PIN_DATA_C
{
    uint8 salt[16];
    uint8 hash[20];
} sAuthLogonPinData_C;

typedef struct AUTH_LOGON_AUTHENTICATOR_DATA_C
{
    uint8 unk; // Has to be 0x01
    uint8 keys[6]; // Valid code must be 6 digits
} sAuthLogonAuthenticatorData_C;

// typedef sAuthLogonChallenge_C sAuthReconnectChallenge_C;
/*
typedef struct
{
    uint8   cmd;
    uint8   error;
    uint8   unk2;
    uint8   B[32];
    uint8   g_len;
    uint8   g[1];
    uint8   N_len;
    uint8   N[32];
    uint8   s[32];
    uint8   unk3[16];
} sAuthLogonChallenge_S;
*/

typedef struct AUTH_LOGON_PROOF_C
{
    uint8   cmd;
    uint8   A[32];
    uint8   M1[20];
    uint8   crc_hash[20];
    uint8   number_of_keys;
    uint8   securityFlags;                                  // 0x00-0x04
} sAuthLogonProof_C;
/*
typedef struct
{
    uint16  unk1;
    uint32  unk2;
    uint8   unk3[4];
    uint16  unk4[20];
}  sAuthLogonProofKey_C;
*/
typedef struct AUTH_LOGON_PROOF_S
{
    uint8   cmd;
    uint8   error;
    uint8   M2[20];
    uint32  accountFlags;                                   // see enum AccountFlags
    uint32  surveyId;                                       // SurveyId
    uint16  unkFlags;                                       // some flags (AccountMsgAvailable = 0x01)
} sAuthLogonProof_S;

typedef struct AUTH_LOGON_PROOF_S_BUILD_6005
{
    uint8   cmd;
    uint8   error;
    uint8   M2[20];
    // uint32  unk1;
    uint32  LoginFlags;
    // uint16  unk3;
} sAuthLogonProof_S_BUILD_6005;

typedef struct AUTH_RECONNECT_PROOF_C
{
    uint8   cmd;
    uint8   R1[16];
    uint8   R2[20];
    uint8   R3[20];
    uint8   number_of_keys;
} sAuthReconnectProof_C;

typedef struct XFER_INIT
{
    uint8 cmd;                                              // XFER_INITIATE
    uint8 fileNameLen;                                      // strlen(fileName);
    uint8 fileName[5];                                      // fileName[fileNameLen]
    uint64 file_size;                                       // file size (bytes)
    uint8 md5[MD5_DIGEST_LENGTH];                           // MD5
} XFER_INIT;

typedef struct AuthHandler
{
    eAuthCmd cmd;
    uint32 status;
    bool (AuthSocket::*handler)(void);
} AuthHandler;

// GCC have alternative #pragma pack() syntax and old gcc version not support pack(pop), also any gcc version not support it at some paltform
#if defined( __GNUC__ )
#pragma pack()
#else
#pragma pack(pop)
#endif

std::array<uint8, 16> VersionChallenge = { { 0xBA, 0xA3, 0x1E, 0x99, 0xA0, 0x0B, 0x21, 0x57, 0xFC, 0x37, 0x3F, 0xB3, 0x69, 0xCD, 0xD2, 0xF1 } };

/// Constructor - set the N and g values for SRP6
AuthSocket::AuthSocket(boost::asio::io_service& service, std::function<void (Socket*)> closeHandler)
    : Socket(service, std::move(closeHandler)), _status(STATUS_CHALLENGE), _build(0), _accountSecurityLevel(SEC_PLAYER)
{
    N.SetHexStr("894B645E89E1535BBDAD5B8B290650530801B18EBFBF5E8FAB3C82872A3E9BB7");
    g.SetDword(7);
}

/// Read the packet from the client
bool AuthSocket::ProcessIncomingData()
{
    // benchmarking has demonstrated that this lookup method is faster than std::map
    const static AuthHandler table[] =
    {
        { CMD_AUTH_LOGON_CHALLENGE,     STATUS_CHALLENGE,   &AuthSocket::_HandleLogonChallenge      },
        { CMD_AUTH_LOGON_PROOF,         STATUS_LOGON_PROOF, &AuthSocket::_HandleLogonProof          },
        { CMD_AUTH_RECONNECT_CHALLENGE, STATUS_CHALLENGE,   &AuthSocket::_HandleReconnectChallenge  },
        { CMD_AUTH_RECONNECT_PROOF,     STATUS_RECON_PROOF, &AuthSocket::_HandleReconnectProof      },
        { CMD_REALM_LIST,               STATUS_AUTHED,      &AuthSocket::_HandleRealmList           },
        { CMD_XFER_ACCEPT,              STATUS_PATCH,       &AuthSocket::_HandleXferAccept          },
        { CMD_XFER_RESUME,              STATUS_PATCH,       &AuthSocket::_HandleXferResume          },
        { CMD_XFER_CANCEL,              STATUS_PATCH,       &AuthSocket::_HandleXferCancel          }
    };

    const int tableLength = sizeof(table) / sizeof(AuthHandler);

    // the purpose of this loop is to handle multiple opcodes in the same tcp packet,
    // which presumably the client will never do, but lets support it anyway! \o/
    while (ReadLengthRemaining() > 0)
    {
        const eAuthCmd cmd = static_cast<eAuthCmd>(*InPeak());
        int i;

        ///- Circle through known commands and call the correct command handler
        for (i = 0; i < tableLength; ++i)
        {
            if (table[i].cmd != cmd)
                continue;

            // unauthorized
            DEBUG_LOG("[Auth] Status %u, table status %u", _status, table[i].status);

            if (table[i].status != _status)
            {
                DEBUG_LOG("[Auth] Received unauthorized command %u length %u", cmd, ReadLengthRemaining());
                return false;
            }

            DEBUG_LOG("[Auth] Got data for cmd %u recv length %u", cmd, ReadLengthRemaining());

            if (!(*this.*table[i].handler)())
            {
                DEBUG_LOG("[Auth] Command handler failed for cmd %u recv length %u", cmd, ReadLengthRemaining());
                return false;
            }

            break;
        }

        // did we iterate over the entire command table, finding nothing? if so, punt!
        if (i == tableLength)
        {
            DEBUG_LOG("[Auth] Got unknown packet %u", cmd);
            return false;
        }

        // if we reach here, it means that a valid opcode was found and the handler completed successfully
    }

    return true;
}

/// Make the SRP6 calculation from hash in dB
void AuthSocket::_SetVSFields(const std::string& rI)
{
    s.SetRand(s_BYTE_SIZE * 8);

    BigNumber I;
    I.SetHexStr(rI.c_str());

    // In case of leading zeros in the rI hash, restore them
    uint8 mDigest[SHA_DIGEST_LENGTH];
    memset(mDigest, 0, SHA_DIGEST_LENGTH);
    if (I.GetNumBytes() <= SHA_DIGEST_LENGTH)
        memcpy(mDigest, I.AsByteArray(), I.GetNumBytes());

    std::reverse(mDigest, mDigest + SHA_DIGEST_LENGTH);

    Sha1Hash sha;
    sha.UpdateData(s.AsByteArray(), s.GetNumBytes());
    sha.UpdateData(mDigest, SHA_DIGEST_LENGTH);
    sha.Finalize();
    BigNumber x;
    x.SetBinary(sha.GetDigest(), Sha1Hash::GetLength());
    v = g.ModExp(x, N);
    const char* v_hex = v.AsHexStr();
    const char* s_hex = s.AsHexStr();
    LoginDatabase.PExecute("UPDATE account SET v = '%s', s = '%s' WHERE username = '%s'", v_hex, s_hex, _safelogin.c_str());
    OPENSSL_free((void*)v_hex);
    OPENSSL_free((void*)s_hex);
}

void AuthSocket::SendProof(Sha1Hash sha)
{
    switch (_build)
    {
        case 5875:                                          // 1.12.1
        case 6005:                                          // 1.12.2
        case 6141:                                          // 1.12.3
        {
            sAuthLogonProof_S_BUILD_6005 proof{};
            memcpy(proof.M2, sha.GetDigest(), 20);
            proof.cmd = CMD_AUTH_LOGON_PROOF;
            proof.error = 0;
            proof.LoginFlags = 0x00;

            Write((const char*)&proof, sizeof(proof));
            break;
        }
        case 8606:                                          // 2.4.3
        case 10505:                                         // 3.2.2a
        case 11159:                                         // 3.3.0a
        case 11403:                                         // 3.3.2
        case 11723:                                         // 3.3.3a
        case 12340:                                         // 3.3.5a
        default:                                            // or later
        {
            sAuthLogonProof_S proof{};
            memcpy(proof.M2, sha.GetDigest(), 20);
            proof.cmd = CMD_AUTH_LOGON_PROOF;
            proof.error = 0;
            proof.accountFlags = ACCOUNT_FLAG_PROPASS;
            proof.surveyId = 0x00000000;
            proof.unkFlags = 0x0000;

            Write((const char*)&proof, sizeof(proof));
            break;
        }
    }
}

/// Logon Challenge command handler
bool AuthSocket::_HandleLogonChallenge()
{
    DEBUG_LOG("Entering _HandleLogonChallenge");
    if (ReadLengthRemaining() < sizeof(sAuthLogonChallenge_C))
        return false;

    ///- Read the first 4 bytes (header) to get the length of the remaining of the packet
    std::vector<uint8> buf;
    buf.resize(4);

    Read((char*)&buf[0], 4);
    void* pVoid = static_cast<void*>(&buf[0]);
    uint16* pUint16 = static_cast<uint16*>(pVoid);
    EndianConvert(*pUint16);
    uint16 remaining = ((sAuthLogonChallenge_C*)&buf[0])->size;
    DEBUG_LOG("[AuthChallenge] got header, body is %#04x bytes", remaining);

    if ((remaining < sizeof(sAuthLogonChallenge_C) - buf.size()) || (ReadLengthRemaining() < remaining))
        return false;

    ///- Session is closed unless overriden
    _status = STATUS_CLOSED;

    // No big fear of memory outage (size is int16, i.e. < 65536)
    buf.resize(remaining + buf.size() + 1);
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C* ch = (sAuthLogonChallenge_C*)&buf[0];

    ///- Read the remaining of the packet
    Read((char*)&buf[4], remaining);
    DEBUG_LOG("[AuthChallenge] got full packet, %#04x bytes", ch->size);
    DEBUG_LOG("[AuthChallenge] name(%d): '%s'", ch->I_len, ch->I);

    // BigEndian code, nop in little endian case
    // size already converted
    EndianConvert(*((uint32*)(&ch->gamename[0])));
    EndianConvert(ch->build);
    EndianConvert(*((uint32*)(&ch->platform[0])));
    EndianConvert(*((uint32*)(&ch->os[0])));
    EndianConvert(*((uint32*)(&ch->country[0])));
    EndianConvert(ch->timezone_bias);
    EndianConvert(ch->ip);

    ByteBuffer pkt;

    _login = (const char*)ch->I;
    _build = ch->build;

    // convert uint8[4] to string and storing to m_os
    std::array<char, 5> os;
    os.fill('\0');
    memcpy(os.data(), ch->os, sizeof(ch->os));
    m_os = os.data();

    // Restore string order as its byte order is reversed
    std::reverse(m_os.begin(), m_os.end());

    ///- Normalize account name
    // utf8ToUpperOnlyLatin(_login); -- client already send account in expected form

    // Escape the user login to avoid further SQL injection
    // Memory will be freed on AuthSocket object destruction
    _safelogin = _login;
    LoginDatabase.escape_string(_safelogin);

    pkt << (uint8) CMD_AUTH_LOGON_CHALLENGE;
    pkt << (uint8) 0x00;

    ///- Verify that this IP is not in the ip_banned table
    // No SQL injection possible (paste the IP address as passed by the socket)
    std::unique_ptr<QueryResult> ip_banned_result(LoginDatabase.PQuery("SELECT unbandate FROM ip_banned "
            "WHERE (unbandate = bandate OR unbandate > UNIX_TIMESTAMP()) AND ip = '%s'", m_address.c_str()));

    std::unique_ptr<QueryResult> account_banned_result(LoginDatabase.PQuery(
                "SELECT ab.unbandate FROM account_banned ab LEFT JOIN account a ON a.id = ab.id "
                "WHERE active = 1 AND a.username = '%s' AND (ab.unbandate = ab.bandate OR ab.unbandate > UNIX_TIMESTAMP())", _safelogin.c_str()));

    if (ip_banned_result || account_banned_result)
    {
        pkt << (uint8)WOW_FAIL_BANNED;
        BASIC_LOG("[AuthChallenge] Banned ip %s tries to login!", m_address.c_str());
    }
    else
    {
        ///- Get the account details from the account table
        // No SQL injection (escaped user name)

        QueryResult* result = LoginDatabase.PQuery("SELECT sha_pass_hash,id,locked,last_ip,gmlevel,v,s,token FROM account WHERE username = '%s'", _safelogin.c_str());
        if (result)
        {
            Field* fields = result->Fetch();

            ///- If the IP is 'locked', check that the player comes indeed from the correct IP address
            bool locked = false;
            if (fields[2].GetUInt8() == 1)               // if ip is locked
            {
                DEBUG_LOG("[AuthChallenge] Account '%s' is locked to IP - '%s'", _login.c_str(), fields[3].GetString());
                DEBUG_LOG("[AuthChallenge] Player address is '%s'", m_address.c_str());
                if (strcmp(fields[3].GetString(), m_address.c_str()))
                {
                    DEBUG_LOG("[AuthChallenge] Account IP differs");
                    pkt << (uint8) WOW_FAIL_SUSPENDED;
                    locked = true;
                }
                else
                {
                    DEBUG_LOG("[AuthChallenge] Account IP matches");
                }
            }
            else
            {
                DEBUG_LOG("[AuthChallenge] Account '%s' is not locked to ip", _login.c_str());
            }

            if (!locked)
            {
                ///- If the account is banned, reject the logon attempt
                QueryResult* banresult = LoginDatabase.PQuery("SELECT bandate,unbandate FROM account_banned WHERE "
                                         "id = %u AND active = 1 AND (unbandate > UNIX_TIMESTAMP() OR unbandate = bandate)", fields[1].GetUInt32());
                if (banresult)
                {
                    if ((*banresult)[0].GetUInt64() == (*banresult)[1].GetUInt64())
                    {
                        pkt << (uint8) WOW_FAIL_BANNED;
                        BASIC_LOG("[AuthChallenge] Banned account %s tries to login!", _login.c_str());
                    }
                    else
                    {
                        pkt << (uint8) WOW_FAIL_SUSPENDED;
                        BASIC_LOG("[AuthChallenge] Temporarily banned account %s tries to login!", _login.c_str());
                    }

                    delete banresult;
                }
                else
                {
                    ///- Get the password from the account table, upper it, and make the SRP6 calculation
                    std::string rI = fields[0].GetCppString();

                    ///- Don't calculate (v, s) if there are already some in the database
                    std::string databaseV = fields[5].GetCppString();
                    std::string databaseS = fields[6].GetCppString();

                    DEBUG_LOG("database authentication values: v='%s' s='%s'", databaseV.c_str(), databaseS.c_str());

                    // multiply with 2, bytes are stored as hexstring
                    if (databaseV.size() != s_BYTE_SIZE * 2 || databaseS.size() != s_BYTE_SIZE * 2)
                        _SetVSFields(rI);
                    else
                    {
                        s.SetHexStr(databaseS.c_str());
                        v.SetHexStr(databaseV.c_str());
                    }

                    b.SetRand(19 * 8);
                    BigNumber gmod = g.ModExp(b, N);
                    B = ((v * 3) + gmod) % N;

                    MANGOS_ASSERT(gmod.GetNumBytes() <= 32);

                    ///- Fill the response packet with the result
                    pkt << uint8(WOW_SUCCESS);

                    // B may be calculated < 32B so we force minimal length to 32B
                    pkt.append(B.AsByteArray(32), 32);      // 32 bytes
                    pkt << uint8(1);
                    pkt.append(g.AsByteArray(), 1);
                    pkt << uint8(32);
                    pkt.append(N.AsByteArray(32), 32);
                    pkt.append(s.AsByteArray(), s.GetNumBytes());// 32 bytes
                    pkt.append(VersionChallenge.data(), VersionChallenge.size());
                    uint8 securityFlags = 0;

                    _token = fields[7].GetCppString();
                    if (!_token.empty() && _build >= 8606) // authenticator was added in 2.4.3
                        securityFlags = SECURITY_FLAG_AUTHENTICATOR;

                    pkt << uint8(securityFlags);                    // security flags (0x0...0x04)

                    if (securityFlags & SECURITY_FLAG_PIN)          // PIN input
                    {
                        pkt << uint32(0);
                        pkt << uint64(0);
                        pkt << uint64(0);
                    }

                    if (securityFlags & SECURITY_FLAG_UNK)          // Matrix input
                    {
                        pkt << uint8(0);
                        pkt << uint8(0);
                        pkt << uint8(0);
                        pkt << uint8(0);
                        pkt << uint64(0);
                    }

                    if (securityFlags & SECURITY_FLAG_AUTHENTICATOR)    // Authenticator input
                        pkt << uint8(1);

                    uint8 secLevel = fields[4].GetUInt8();
                    _accountSecurityLevel = secLevel <= SEC_ADMINISTRATOR ? AccountTypes(secLevel) : SEC_ADMINISTRATOR;

                    _localizationName.resize(4);
                    for (int i = 0; i < 4; ++i)
                        _localizationName[i] = ch->country[4 - i - 1];

                    BASIC_LOG("[AuthChallenge] account %s is using '%c%c%c%c' locale (%u)", _login.c_str(), ch->country[3], ch->country[2], ch->country[1], ch->country[0], GetLocaleByName(_localizationName));

                    ///- All good, await client's proof
                    _status = STATUS_LOGON_PROOF;
                }
            }
            delete result;
        }
        else                                                // no account
            pkt << (uint8) WOW_FAIL_UNKNOWN_ACCOUNT;
    }

    Write((const char*)pkt.contents(), pkt.size());
    return true;
}

/// Logon Proof command handler
bool AuthSocket::_HandleLogonProof()
{
    DEBUG_LOG("Entering _HandleLogonProof");
    ///- Read the packet
    sAuthLogonProof_C lp{};
    if (!Read((char*)&lp, sizeof(sAuthLogonProof_C)))
        return false;

    ///- Session is closed unless overriden
    _status = STATUS_CLOSED;

    /// <ul><li> If the client has no valid version
    if (!FindBuildInfo(_build))
    {
        // no patch found
        ByteBuffer pkt;
        pkt << (uint8) CMD_AUTH_LOGON_CHALLENGE;
        pkt << (uint8) 0x00;
        pkt << (uint8) WOW_FAIL_VERSION_INVALID;
        DEBUG_LOG("[AuthChallenge] %u is not a valid client version!", _build);
        Write((const char*)pkt.contents(), pkt.size());
        return true;
    }
    /// </ul>

    ///- Continue the SRP6 calculation based on data received from the client
    BigNumber A;
    A.SetBinary(lp.A, 32);

    // SRP safeguard: abort if A==0
    if (A.isZero())
        return false;

    if ((A % N).isZero())
        return false;

    Sha1Hash sha;
    sha.UpdateBigNumbers(&A, &B, nullptr);
    sha.Finalize();
    BigNumber u;
    u.SetBinary(sha.GetDigest(), 20);
    BigNumber S = (A * (v.ModExp(u, N))).ModExp(b, N);

    uint8 t[32];
    uint8 t1[16];
    uint8 vK[40];
    memcpy(t, S.AsByteArray(32), 32);
    for (int i = 0; i < 16; ++i)
    {
        t1[i] = t[i * 2];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        vK[i * 2] = sha.GetDigest()[i];
    }
    for (int i = 0; i < 16; ++i)
    {
        t1[i] = t[i * 2 + 1];
    }
    sha.Initialize();
    sha.UpdateData(t1, 16);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        vK[i * 2 + 1] = sha.GetDigest()[i];
    }
    K.SetBinary(vK, 40);

    uint8 hash[20];

    sha.Initialize();
    sha.UpdateBigNumbers(&N, nullptr);
    sha.Finalize();
    memcpy(hash, sha.GetDigest(), 20);
    sha.Initialize();
    sha.UpdateBigNumbers(&g, nullptr);
    sha.Finalize();
    for (int i = 0; i < 20; ++i)
    {
        hash[i] ^= sha.GetDigest()[i];
    }
    BigNumber t3;
    t3.SetBinary(hash, 20);

    sha.Initialize();
    sha.UpdateData(_login);
    sha.Finalize();
    uint8 t4[SHA_DIGEST_LENGTH];
    memcpy(t4, sha.GetDigest(), SHA_DIGEST_LENGTH);

    sha.Initialize();
    sha.UpdateBigNumbers(&t3, nullptr);
    sha.UpdateData(t4, SHA_DIGEST_LENGTH);
    sha.UpdateBigNumbers(&s, &A, &B, &K, nullptr);
    sha.Finalize();
    BigNumber M;
    M.SetBinary(sha.GetDigest(), 20);

    ///- Check if SRP6 results match (password is correct), else send an error
    if (!memcmp(M.AsByteArray(), lp.M1, 20))
    {
        if (lp.securityFlags & SECURITY_FLAG_AUTHENTICATOR || !_token.empty())
        {
            sAuthLogonAuthenticatorData_C authData{};
            if (!Read((char*) &authData, sizeof(sAuthLogonAuthenticatorData_C)))
            {
                const char data[4] = {CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 3, 0};
                Write(data, sizeof(data));
                return true;
            }

            auto ServerToken = generateToken(_token.c_str());
            auto clientToken = atoi((const char*) authData.keys);
            if (ServerToken != clientToken)
            {
                BASIC_LOG("[AuthChallenge] Account %s tried to login with wrong pincode! Given %u Expected %u", _login.c_str(), clientToken, ServerToken);

                const char data[4] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 0, 0};
                Write(data, sizeof(data));
                return true;
            }
        }

        if (!VerifyVersion(lp.A, sizeof(lp.A), lp.crc_hash, false))
        {
            BASIC_LOG("[AuthChallenge] Account %s tried to login with modified client!", _login.c_str());

            const char data[2] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_VERSION_INVALID };
            Write(data, sizeof(data));
            return true;
        }

        BASIC_LOG("User '%s' successfully authenticated", _login.c_str());

        ///- Update the sessionkey, last_ip, last login time and reset number of failed logins in the account table for this account
        // No SQL injection (escaped user name) and IP address as received by socket
        const char* K_hex = K.AsHexStr();
        LoginDatabase.PExecute("UPDATE account SET sessionkey = '%s', last_ip = '%s', last_login = NOW(), locale = '%u', failed_logins = 0 WHERE username = '%s'", K_hex, m_address.c_str(), GetLocaleByName(_localizationName), _safelogin.c_str());
        OPENSSL_free((void*)K_hex);

        ///- Finish SRP6 and send the final result to the client
        sha.Initialize();
        sha.UpdateBigNumbers(&A, &M, &K, nullptr);
        sha.Finalize();

        SendProof(sha);

        ///- Set _status to authed!
        _status = STATUS_AUTHED;
    }
    else
    {
        if (_build > 6005)                                  // > 1.12.2
        {
            const char data[4] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT, 0, 0};
            Write(data, sizeof(data));
        }
        else
        {
            // 1.x not react incorrectly at 4-byte message use 3 as real error
            const char data[2] = { CMD_AUTH_LOGON_PROOF, WOW_FAIL_UNKNOWN_ACCOUNT};
            Write(data, sizeof(data));
        }
        BASIC_LOG("[AuthChallenge] account %s tried to login with wrong password!", _login.c_str());

        uint32 MaxWrongPassCount = sConfig.GetIntDefault("WrongPass.MaxCount", 0);
        if (MaxWrongPassCount > 0)
        {
            // Increment number of failed logins by one and if it reaches the limit temporarily ban that account or IP
            LoginDatabase.PExecute("UPDATE account SET failed_logins = failed_logins + 1 WHERE username = '%s'", _safelogin.c_str());

            if (QueryResult* loginfail = LoginDatabase.PQuery("SELECT id, failed_logins FROM account WHERE username = '%s'", _safelogin.c_str()))
            {
                Field* fields = loginfail->Fetch();
                uint32 failed_logins = fields[1].GetUInt32();

                if (failed_logins >= MaxWrongPassCount)
                {
                    uint32 WrongPassBanTime = sConfig.GetIntDefault("WrongPass.BanTime", 600);
                    bool WrongPassBanType = sConfig.GetBoolDefault("WrongPass.BanType", false);

                    if (WrongPassBanType)
                    {
                        uint32 acc_id = fields[0].GetUInt32();
                        LoginDatabase.PExecute("INSERT INTO account_banned VALUES ('%u',UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+'%u','MaNGOS realmd','Failed login autoban',1)",
                                               acc_id, WrongPassBanTime);
                        BASIC_LOG("[AuthChallenge] account %s got banned for '%u' seconds because it failed to authenticate '%u' times",
                                  _login.c_str(), WrongPassBanTime, failed_logins);
                    }
                    else
                    {
                        std::string current_ip = m_address;
                        LoginDatabase.escape_string(current_ip);
                        LoginDatabase.PExecute("INSERT INTO ip_banned VALUES ('%s',UNIX_TIMESTAMP(),UNIX_TIMESTAMP()+'%u','MaNGOS realmd','Failed login autoban')",
                                               current_ip.c_str(), WrongPassBanTime);
                        BASIC_LOG("[AuthChallenge] IP %s got banned for '%u' seconds because account %s failed to authenticate '%u' times",
                                  current_ip.c_str(), WrongPassBanTime, _login.c_str(), failed_logins);
                    }
                }
                delete loginfail;
            }
        }
    }
    return true;
}

/// Reconnect Challenge command handler
bool AuthSocket::_HandleReconnectChallenge()
{
    DEBUG_LOG("Entering _HandleReconnectChallenge");
    if (ReadLengthRemaining() < sizeof(sAuthLogonChallenge_C))
        return false;

    ///- Read the first 4 bytes (header) to get the length of the remaining of the packet
    std::vector<uint8> buf;
    buf.resize(4);

    Read((char*)&buf[0], 4);

    void* pVoid = static_cast<void*>(&buf[0]);
    uint16* pUint16 = static_cast<uint16*>(pVoid);
    EndianConvert(*pUint16);
    uint16 remaining = ((sAuthLogonChallenge_C*)&buf[0])->size;
    DEBUG_LOG("[ReconnectChallenge] got header, body is %#04x bytes", remaining);

    if ((remaining < sizeof(sAuthLogonChallenge_C) - buf.size()) || (ReadLengthRemaining() < remaining))
        return false;

    ///- Session is closed unless overriden
    _status = STATUS_CLOSED;

    // No big fear of memory outage (size is int16, i.e. < 65536)
    buf.resize(remaining + buf.size() + 1);
    buf[buf.size() - 1] = 0;
    sAuthLogonChallenge_C* ch = (sAuthLogonChallenge_C*)&buf[0];

    ///- Read the remaining of the packet
    Read((char*)&buf[4], remaining);
    DEBUG_LOG("[ReconnectChallenge] got full packet, %#04x bytes", ch->size);
    DEBUG_LOG("[ReconnectChallenge] name(%d): '%s'", ch->I_len, ch->I);

    _login = (const char*)ch->I;

    _safelogin = _login;
    LoginDatabase.escape_string(_safelogin);

    EndianConvert(ch->build);
    _build = ch->build;

    QueryResult* result = LoginDatabase.PQuery("SELECT sessionkey FROM account WHERE username = '%s'", _safelogin.c_str());

    // Stop if the account is not found
    if (!result)
    {
        sLog.outError("[ERROR] user %s tried to login and we cannot find his session key in the database.", _login.c_str());
        Close();
        return false;
    }

    Field* fields = result->Fetch();
    K.SetHexStr(fields[0].GetString());
    delete result;

    ///- All good, await client's proof
    _status = STATUS_RECON_PROOF;

    ///- Sending response
    ByteBuffer pkt;
    pkt << (uint8)  CMD_AUTH_RECONNECT_CHALLENGE;
    pkt << (uint8)  0x00;
    _reconnectProof.SetRand(16 * 8);
    pkt.append(_reconnectProof.AsByteArray(16), 16);        // 16 bytes random
    pkt.append(VersionChallenge.data(), VersionChallenge.size());
    Write((const char*)pkt.contents(), pkt.size());
    return true;
}

/// Reconnect Proof command handler
bool AuthSocket::_HandleReconnectProof()
{
    DEBUG_LOG("Entering _HandleReconnectProof");
    ///- Read the packet
    sAuthReconnectProof_C lp;
    if (!Read((char*)&lp, sizeof(sAuthReconnectProof_C)))
        return false;

    ///- Session is closed unless overriden
    _status = STATUS_CLOSED;

    if (_login.empty() || !_reconnectProof.GetNumBytes() || !K.GetNumBytes())
        return false;

    BigNumber t1;
    t1.SetBinary(lp.R1, 16);

    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateData(_login);
    sha.UpdateBigNumbers(&t1, &_reconnectProof, &K, nullptr);
    sha.Finalize();

    if (!memcmp(sha.GetDigest(), lp.R2, SHA_DIGEST_LENGTH))
    {
        if (!VerifyVersion(lp.R1, sizeof(lp.R1), lp.R3, true))
        {
            ByteBuffer pkt;
            pkt << (uint8)CMD_AUTH_RECONNECT_PROOF;
            pkt << (uint8)WOW_FAIL_VERSION_INVALID;
            Write((const char*)pkt.contents(), pkt.size());
            return true;
        }
        ///- Sending response
        ByteBuffer pkt;
        pkt << (uint8)  CMD_AUTH_RECONNECT_PROOF;
        pkt << (uint8)  WOW_SUCCESS;
        pkt << (uint16) 0x00;                               // 2 bytes zeros
        Write((const char*)pkt.contents(), pkt.size());

        ///- Set _status to authed!
        _status = STATUS_AUTHED;

        return true;
    }
    sLog.outError("[ERROR] user %s tried to login, but session invalid.", _login.c_str());
    Close();
    return false;
}

/// %Realm List command handler
bool AuthSocket::_HandleRealmList()
{
    DEBUG_LOG("Entering _HandleRealmList");
    if (ReadLengthRemaining() < 5)
        return false;

    ReadSkip(5);

    ///- Get the user id (else close the connection)
    // No SQL injection (escaped user name)

    QueryResult* result = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", _safelogin.c_str());
    if (!result)
    {
        sLog.outError("[ERROR] user %s tried to login and we cannot find him in the database.", _login.c_str());
        Close();
        return false;
    }

    uint32 id = (*result)[0].GetUInt32();
    delete result;

    ///- Update realm list if need
    sRealmList.UpdateIfNeed();

    ///- Circle through realms in the RealmList and construct the return packet (including # of user characters in each realm)
    ByteBuffer pkt;
    LoadRealmlist(pkt, id);

    ByteBuffer hdr;
    hdr << (uint8) CMD_REALM_LIST;
    hdr << (uint16)pkt.size();
    hdr.append(pkt);

    Write((const char*)hdr.contents(), hdr.size());
    return true;
}

void AuthSocket::LoadRealmlist(ByteBuffer& pkt, uint32 acctid)
{
    switch (_build)
    {
        case 5875:                                          // 1.12.1
        case 6005:                                          // 1.12.2
        case 6141:                                          // 1.12.3
        {
            pkt << uint32(0);                               // unused value
            pkt << uint8(sRealmList.size());

            for (const auto& i : sRealmList)
            {
                uint8 AmountOfCharacters;

                // No SQL injection. id of realm is controlled by the database.
                QueryResult* result = LoginDatabase.PQuery("SELECT numchars FROM realmcharacters WHERE realmid = '%d' AND acctid='%u'", i.second.m_ID, acctid);
                if (result)
                {
                    Field* fields = result->Fetch();
                    AmountOfCharacters = fields[0].GetUInt8();
                    delete result;
                }
                else
                    AmountOfCharacters = 0;

                bool ok_build = std::find(i.second.realmbuilds.begin(), i.second.realmbuilds.end(), _build) != i.second.realmbuilds.end();

                RealmBuildInfo const* buildInfo = ok_build ? FindBuildInfo(_build) : nullptr;
                if (!buildInfo)
                    buildInfo = &i.second.realmBuildInfo;

                RealmFlags realmflags = i.second.realmflags;

                // 1.x clients not support explicitly REALM_FLAG_SPECIFYBUILD, so manually form similar name as show in more recent clients
                std::string name = i.first;
                if (realmflags & REALM_FLAG_SPECIFYBUILD)
                {
                    char buf[20];
                    snprintf(buf, 20, " (%u,%u,%u)", buildInfo->major_version, buildInfo->minor_version, buildInfo->bugfix_version);
                    name += buf;
                }

                // Show offline state for unsupported client builds and locked realms (1.x clients not support locked state show)
                if (!ok_build || (i.second.allowedSecurityLevel > _accountSecurityLevel))
                    realmflags = RealmFlags(realmflags | REALM_FLAG_OFFLINE);

                pkt << uint32(i.second.icon);              // realm type
                pkt << uint8(realmflags);                   // realmflags
                pkt << name;                                // name
                pkt << i.second.address;                   // address
                pkt << float(i.second.populationLevel);
                pkt << uint8(AmountOfCharacters);
                pkt << uint8(i.second.timezone);           // realm category
                pkt << uint8(0x00);                         // unk, may be realm number/id?
            }

            pkt << uint16(0x0002);                          // unused value (why 2?)
            break;
        }

        case 8606:                                          // 2.4.3
        case 10505:                                         // 3.2.2a
        case 11159:                                         // 3.3.0a
        case 11403:                                         // 3.3.2
        case 11723:                                         // 3.3.3a
        case 12340:                                         // 3.3.5a
        default:                                            // and later
        {
            pkt << uint32(0);                               // unused value
            pkt << uint16(sRealmList.size());

            for (const auto& i : sRealmList)
            {
                uint8 AmountOfCharacters;

                // No SQL injection. id of realm is controlled by the database.
                QueryResult* result = LoginDatabase.PQuery("SELECT numchars FROM realmcharacters WHERE realmid = '%d' AND acctid='%u'", i.second.m_ID, acctid);
                if (result)
                {
                    Field* fields = result->Fetch();
                    AmountOfCharacters = fields[0].GetUInt8();
                    delete result;
                }
                else
                    AmountOfCharacters = 0;

                bool ok_build = std::find(i.second.realmbuilds.begin(), i.second.realmbuilds.end(), _build) != i.second.realmbuilds.end();

                RealmBuildInfo const* buildInfo = ok_build ? FindBuildInfo(_build) : nullptr;
                if (!buildInfo)
                    buildInfo = &i.second.realmBuildInfo;

                uint8 lock = (i.second.allowedSecurityLevel > _accountSecurityLevel) ? 1 : 0;

                RealmFlags realmFlags = i.second.realmflags;

                // Show offline state for unsupported client builds
                if (!ok_build)
                    realmFlags = RealmFlags(realmFlags | REALM_FLAG_OFFLINE);

                //if (!buildInfo) // always false since updated 10 lines above if null. ToDo: fix
                //    realmFlags = RealmFlags(realmFlags & ~REALM_FLAG_SPECIFYBUILD);

                pkt << uint8(i.second.icon);               // realm type (this is second column in Cfg_Configs.dbc)
                pkt << uint8(lock);                         // flags, if 0x01, then realm locked
                pkt << uint8(realmFlags);                   // see enum RealmFlags
                pkt << i.first;                            // name
                pkt << i.second.address;                   // address
                pkt << float(i.second.populationLevel);
                pkt << uint8(AmountOfCharacters);
                pkt << uint8(i.second.timezone);           // realm category (Cfg_Categories.dbc)
                pkt << uint8(0x2C);                         // unk, may be realm number/id?

                if (realmFlags & REALM_FLAG_SPECIFYBUILD)
                {
                    pkt << uint8(buildInfo->major_version);
                    pkt << uint8(buildInfo->minor_version);
                    pkt << uint8(buildInfo->bugfix_version);
                    pkt << uint16(_build);
                }
            }

            pkt << uint16(0x0010);                          // unused value (why 10?)
            break;
        }
    }
}

/// Resume patch transfer
bool AuthSocket::_HandleXferResume()
{
    DEBUG_LOG("Entering _HandleXferResume");

    if (ReadLengthRemaining() < 9)
        return false;

    ReadSkip(9);

    return true;
}

/// Cancel patch transfer
bool AuthSocket::_HandleXferCancel()
{
    DEBUG_LOG("Entering _HandleXferCancel");

    ReadSkip(1);
    Close();

    return true;
}

/// Accept patch transfer
bool AuthSocket::_HandleXferAccept()
{
    DEBUG_LOG("Entering _HandleXferAccept");

    ReadSkip(1);

    return true;
}

int32 AuthSocket::generateToken(char const* b32key)
{
    size_t keySize = strlen(b32key);
    size_t bufSize = (keySize + 7) / 8 * 5;
    char* encoded = new char[bufSize];
    memset(encoded, 0, bufSize);
    unsigned int hmac_result_size = HMAC_RES_SIZE;
    unsigned char hmac_result[HMAC_RES_SIZE];
    unsigned long timestamp = time(nullptr) / 30;
    unsigned char challenge[8];

    for (int i = 8; i--; timestamp >>= 8)
        challenge[i] = timestamp;

    base32_decode(b32key, encoded, bufSize);
    HMAC(EVP_sha1(), encoded, bufSize, challenge, 8, hmac_result, &hmac_result_size);
    unsigned int offset = hmac_result[19] & 0xF;
    unsigned int truncHash = (hmac_result[offset] << 24) | (hmac_result[offset + 1] << 16) | (hmac_result[offset + 2] << 8) | (hmac_result[offset + 3]);
    truncHash &= 0x7FFFFFFF;

    delete[] encoded;

    return truncHash % 1000000;
}

bool AuthSocket::VerifyVersion(uint8 const* a, int32 aLength, uint8 const* versionProof, bool isReconnect)
{
    if (!sConfig.GetBoolDefault("StrictVersionCheck", false))
        return true;

    std::array<uint8, 20> zeros = { {} };
    std::array<uint8, 20> const* versionHash = nullptr;
    if (!isReconnect)
    {
        RealmBuildInfo const* buildInfo = FindBuildInfo(_build);
        if (!buildInfo)
            return false;

        if (m_os == "Win")
            versionHash = &buildInfo->WindowsHash;
        else if (m_os == "OSX")
            versionHash = &buildInfo->MacHash;

        if (!versionHash)
            return false;

        if (!memcmp(versionHash->data(), zeros.data(), zeros.size()))
            return true;                                                            // not filled serverside
    }
    else
        versionHash = &zeros;

    Sha1Hash version;
    version.UpdateData(a, aLength);
    version.UpdateData(versionHash->data(), versionHash->size());
    version.Finalize();

    return memcmp(versionProof, version.GetDigest(), version.GetLength()) == 0;
}