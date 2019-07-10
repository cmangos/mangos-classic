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

#include "Accounts/AccountMgr.h"
#include "Database/DatabaseEnv.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Player.h"
#include "Policies/Singleton.h"
#include "Util.h"
#include "Auth/Sha1.h"
#include "SRP6/SRP6.h"

extern DatabaseType LoginDatabase;

INSTANTIATE_SINGLETON_1(AccountMgr);

AccountMgr::AccountMgr()
{}

AccountMgr::~AccountMgr()
{}

AccountOpResult AccountMgr::CreateAccount(std::string username, std::string password) const
{
    if (utf8length(username) > MAX_ACCOUNT_STR)
        return AOR_NAME_TOO_LONG;                           // username's too long

    normalizeString(username);
    normalizeString(password);

    if (GetId(username))
    {
        return AOR_NAME_ALREADY_EXIST;                       // username does already exist
    }

    SRP6 srp;

    srp.CalculateVerifier(CalculateShaPassHash(username, password));
    const char* s_hex = srp.GetSalt().AsHexStr();
    const char* v_hex = srp.GetVerifier().AsHexStr();

    bool update_sv = LoginDatabase.PExecute(
        "INSERT INTO account(username,v,s,joindate) VALUES('%s','%s','%s',NOW())",
            username.c_str(), v_hex, s_hex);

    OPENSSL_free((void*)s_hex);
    OPENSSL_free((void*)v_hex);

    if (!update_sv)
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error
    LoginDatabase.Execute(
        "INSERT INTO realmcharacters (realmid, acctid, numchars) SELECT realmlist.id, account.id, 0 FROM realmlist,account LEFT JOIN realmcharacters ON acctid=account.id WHERE acctid IS NULL");

    return AOR_OK;                                          // everything's fine
}

AccountOpResult AccountMgr::DeleteAccount(uint32 accid) const
{
    QueryResult* result = LoginDatabase.PQuery("SELECT 1 FROM account WHERE id='%u'", accid);
    if (!result)
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist
    delete result;

    // existing characters list
    result = CharacterDatabase.PQuery("SELECT guid FROM characters WHERE account='%u'", accid);
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guidlo = fields[0].GetUInt32();
            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, guidlo);

            // kick if player currently
            ObjectAccessor::KickPlayer(guid);
            Player::DeleteFromDB(guid, accid, false);       // no need to update realm characters
        }
        while (result->NextRow());

        delete result;
    }

    // table realm specific but common for all characters of account for realm
    CharacterDatabase.PExecute("DELETE FROM character_tutorial WHERE account = '%u'", accid);

    LoginDatabase.BeginTransaction();

    bool res =
        LoginDatabase.PExecute("DELETE FROM account WHERE id='%u'", accid) &&
        LoginDatabase.PExecute("DELETE FROM realmcharacters WHERE acctid='%u'", accid);

    LoginDatabase.CommitTransaction();

    if (!res)
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error;

    return AOR_OK;
}

AccountOpResult AccountMgr::ChangeUsername(uint32 accid, std::string new_uname, std::string new_passwd) const
{
    QueryResult* result = LoginDatabase.PQuery("SELECT 1 FROM account WHERE id='%u'", accid);
    if (!result)
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist
    delete result;

    if (utf8length(new_uname) > MAX_ACCOUNT_STR)
        return AOR_NAME_TOO_LONG;

    if (utf8length(new_passwd) > MAX_ACCOUNT_STR)
        return AOR_PASS_TOO_LONG;

    normalizeString(new_uname);
    normalizeString(new_passwd);

    SRP6 srp;

    srp.CalculateVerifier(CalculateShaPassHash(new_uname, new_passwd));

    std::string safe_new_uname = new_uname;
    LoginDatabase.escape_string(safe_new_uname);

    const char* s_hex = srp.GetSalt().AsHexStr();
    const char* v_hex = srp.GetVerifier().AsHexStr();

    bool update_sv = LoginDatabase.PExecute(
        "UPDATE account SET v='%s',s='%s',username='%s' WHERE id='%u'",
            v_hex, s_hex, safe_new_uname.c_str(), accid);

    OPENSSL_free((void*)s_hex);
    OPENSSL_free((void*)v_hex);

    if (!update_sv)
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error

    return AOR_OK;
}

AccountOpResult AccountMgr::ChangePassword(uint32 accid, std::string new_passwd) const
{
    std::string username;

    if (!GetName(accid, username))
        return AOR_NAME_NOT_EXIST;                          // account doesn't exist

    if (utf8length(new_passwd) > MAX_ACCOUNT_STR)
        return AOR_PASS_TOO_LONG;

    normalizeString(username);
    normalizeString(new_passwd);

    SRP6 srp;

    srp.CalculateVerifier(CalculateShaPassHash(username, new_passwd));

    const char* s_hex = srp.GetSalt().AsHexStr();
    const char* v_hex = srp.GetVerifier().AsHexStr();

    bool update_sv = LoginDatabase.PExecute(
        "UPDATE account SET v='%s', s='%s' WHERE id='%u'",
            v_hex, s_hex, accid);

    OPENSSL_free((void*)s_hex);
    OPENSSL_free((void*)v_hex);

    // also reset s and v to force update at next realmd login
    if (!update_sv)
        return AOR_DB_INTERNAL_ERROR;                       // unexpected error

    return AOR_OK;
}

uint32 AccountMgr::GetId(std::string username) const
{
    LoginDatabase.escape_string(username);
    QueryResult* result = LoginDatabase.PQuery("SELECT id FROM account WHERE username = '%s'", username.c_str());
    if (!result)
        return 0;
    uint32 id = (*result)[0].GetUInt32();
    delete result;
    return id;
}

AccountTypes AccountMgr::GetSecurity(uint32 acc_id)
{
    QueryResult* result = LoginDatabase.PQuery("SELECT gmlevel FROM account WHERE id = '%u'", acc_id);
    if (result)
    {
        AccountTypes sec = AccountTypes((*result)[0].GetInt32());
        delete result;
        return sec;
    }

    return SEC_PLAYER;
}

bool AccountMgr::GetName(uint32 acc_id, std::string& name) const
{
    QueryResult* result = LoginDatabase.PQuery("SELECT username FROM account WHERE id = '%u'", acc_id);
    if (result)
    {
        name = (*result)[0].GetCppString();
        delete result;
        return true;
    }

    return false;
}

uint32 AccountMgr::GetCharactersCount(uint32 acc_id) const
{
    // check character count
    QueryResult* result = CharacterDatabase.PQuery("SELECT COUNT(guid) FROM characters WHERE account = '%u'", acc_id);
    if (result)
    {
        Field* fields = result->Fetch();
        uint32 charcount = fields[0].GetUInt32();
        delete result;
        return charcount;
    }
    return 0;
}

bool AccountMgr::CheckPassword(uint32 accid, std::string passwd) const
{
    std::string username;
    if (!GetName(accid, username))
        return false;

    normalizeString(passwd);
    normalizeString(username);

    QueryResult* result = LoginDatabase.PQuery("SELECT s, v FROM account WHERE id='%u'", accid);
    if (result)
    {
        Field* fields = result->Fetch();
        SRP6 srp;

        bool calcv = srp.CalculateVerifier(
            CalculateShaPassHash(username, passwd), fields[0].GetCppString().c_str());

        if (calcv && srp.ProofVerifier(fields[1].GetCppString()))
        {
            delete result;
            return true;
        }

        delete result;
    }

    return false;
}

bool AccountMgr::normalizeString(std::string& utf8str)
{
    std::wstring wstr_buf;
    if (!Utf8toWStr(utf8str, wstr_buf))
        return false;

    if (wstr_buf.size() > MAX_ACCOUNT_STR)
        return false;

    std::transform(wstr_buf.begin(), wstr_buf.end(), wstr_buf.begin(), wcharToUpperOnlyLatin);

    return WStrToUtf8(wstr_buf, utf8str);
}

std::string AccountMgr::CalculateShaPassHash(std::string& name, std::string& password) const
{
    Sha1Hash sha;
    sha.Initialize();
    sha.UpdateData(name);
    sha.UpdateData(":");
    sha.UpdateData(password);
    sha.Finalize();

    std::string encoded;
    hexEncodeByteArray(sha.GetDigest(), Sha1Hash::GetLength(), encoded);

    return encoded;
}
