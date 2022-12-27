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
#include "RealmList.h"
#include "AuthCodes.h"
#include "Util/Util.h"                                           // for Tokens typedef
#include "Policies/Singleton.h"
#include "Database/DatabaseEnv.h"

INSTANTIATE_SINGLETON_1(RealmList);

extern DatabaseType LoginDatabase;

// will only support 1.12.1/1.12.2/1.12.3, TBC 2.4.3 and official release for WotLK and later, client builds 10505, 8606, 6141, 6005, 5875
// if you need more from old build then add it in cases in realmd sources code
// list sorted from high to low build and first build used as low bound for accepted by default range (any > it will accepted by realmd at least)

static const RealmBuildInfo ExpectedRealmdClientBuilds[] =
{
    {13930, 3, 3, 5, 'a', {{}}, {{}}},                      // 3.3.5a China Mainland build
    {12340, 3, 3, 5, 'a',
        {{ 0xCD, 0xCB, 0xBD, 0x51, 0x88, 0x31, 0x5E, 0x6B, 0x4D, 0x19, 0x44, 0x9D, 0x49, 0x2D, 0xBC, 0xFA, 0xF1, 0x56, 0xA3, 0x47 }},
        {{ 0xB7, 0x06, 0xD1, 0x3F, 0xF2, 0xF4, 0x01, 0x88, 0x39, 0x72, 0x94, 0x61, 0xE3, 0xF8, 0xA0, 0xE2, 0xB5, 0xFD, 0xC0, 0x34 }},
    },
    {11723, 3, 3, 3, 'a', {{}}, {{}}},
    {11403, 3, 3, 2, ' ', {{}}, {{}}},
    {11159, 3, 3, 0, 'a', {{}}, {{}}},
    {10505, 3, 2, 2, 'a', {{}}, {{}}},
    {9947,  3, 1, 3, ' ', {{}}, {{}}},
    {8606,  2, 4, 3, ' ',
        {{ 0x31, 0x9A, 0xFA, 0xA3, 0xF2, 0x55, 0x96, 0x82, 0xF9, 0xFF, 0x65, 0x8B, 0xE0, 0x14, 0x56, 0x25, 0x5F, 0x45, 0x6F, 0xB1 }},
        {{ 0xD8, 0xB0, 0xEC, 0xFE, 0x53, 0x4B, 0xC1, 0x13, 0x1E, 0x19, 0xBA, 0xD1, 0xD4, 0xC0, 0xE8, 0x13, 0xEE, 0xE4, 0x99, 0x4F }},
    },
    {6141,  1, 12, 3, ' ',
        {{ 0xEB, 0x88, 0x24, 0x3E, 0x94, 0x26, 0xC9, 0xD6, 0x8C, 0x81, 0x87, 0xF7, 0xDA, 0xE2, 0x25, 0xEA, 0xF3, 0x88, 0xD8, 0xAF }},
        {{}}
    },
    {6005,  1, 12, 2, ' ',
        {{ 0x06, 0x97, 0x32, 0x38, 0x76, 0x56, 0x96, 0x41, 0x48, 0x79, 0x28, 0xFD, 0xC7, 0xC9, 0xE3, 0x3B, 0x44, 0x70, 0xC8, 0x80 }},
        {{}}
    },
    {5875,  1, 12, 1, ' ',
        {{ 0x95, 0xED, 0xB2, 0x7C, 0x78, 0x23, 0xB3, 0x63, 0xCB, 0xDD, 0xAB, 0x56, 0xA3, 0x92, 0xE7, 0xCB, 0x73, 0xFC, 0xCA, 0x20 }},
        {{ 0x8D, 0x17, 0x3C, 0xC3, 0x81, 0x96, 0x1E, 0xEB, 0xAB, 0xF3, 0x36, 0xF5, 0xE6, 0x67, 0x5B, 0x10, 0x1B, 0xB5, 0x13, 0xE5 }},
    },
    {0,     0, 0, 0, ' ', {{}}, {{}}}                       // terminator
};

RealmBuildInfo const* FindBuildInfo(uint16 _build)
{
    // first build is low bound of always accepted range
    if (_build >= ExpectedRealmdClientBuilds[0].build)
        return &ExpectedRealmdClientBuilds[0];

    // continue from 1 with explicit equal check
    for (int i = 1; ExpectedRealmdClientBuilds[i].build; ++i)
        if (_build == ExpectedRealmdClientBuilds[i].build)
            return &ExpectedRealmdClientBuilds[i];

    // none appropriate build
    return nullptr;
}

RealmList::RealmList() : m_UpdateInterval(0), m_NextUpdateTime(time(nullptr))
{
}

RealmList& sRealmList
{
    static RealmList realmlist;
    return realmlist;
}

/// Load the realm list from the database
void RealmList::Initialize(uint32 updateInterval)
{
    m_UpdateInterval = updateInterval;

    ///- Get the content of the realmlist table in the database
    UpdateRealms(true);
}

void RealmList::UpdateRealm(uint32 ID, const std::string& name, const std::string& address, uint32 port, uint8 icon, RealmFlags realmflags, uint8 timezone, AccountTypes allowedSecurityLevel, float popu, const std::string& builds)
{
    ///- Create new if not exist or update existed
    Realm& realm = m_realms[name];

    realm.m_ID       = ID;
    realm.icon       = icon;
    realm.realmflags = realmflags;
    realm.timezone   = timezone;
    realm.allowedSecurityLevel = allowedSecurityLevel;
    realm.populationLevel      = popu;

    Tokens tokens = StrSplit(builds, " ");

    for (auto& token : tokens)
    {
        uint32 build = atol(token.c_str());
        realm.realmbuilds.insert(build);
    }

    uint16 first_build = !realm.realmbuilds.empty() ? *realm.realmbuilds.begin() : 0;

    realm.realmBuildInfo.build = first_build;
    realm.realmBuildInfo.major_version = 0;
    realm.realmBuildInfo.minor_version = 0;
    realm.realmBuildInfo.bugfix_version = 0;
    realm.realmBuildInfo.hotfix_version = ' ';

    if (first_build)
        if (RealmBuildInfo const* bInfo = FindBuildInfo(first_build))
            if (bInfo->build == first_build)
                realm.realmBuildInfo = *bInfo;

    ///- Append port to IP address.
    std::ostringstream ss;
    ss << address << ":" << port;
    realm.address   = ss.str();
}

void RealmList::UpdateIfNeed()
{
    // maybe disabled or updated recently
    if (!m_UpdateInterval || m_NextUpdateTime > time(nullptr))
        return;

    m_NextUpdateTime = time(nullptr) + m_UpdateInterval;

    // Clears Realm list
    m_realms.clear();

    // Get the content of the realmlist table in the database
    UpdateRealms(false);
}

void RealmList::UpdateRealms(bool init)
{
    DETAIL_LOG("Updating Realm List...");

    ////                                               0   1     2        3     4     5           6         7                     8           9
    QueryResult* result = LoginDatabase.Query("SELECT id, name, address, port, icon, realmflags, timezone, allowedSecurityLevel, population, realmbuilds FROM realmlist WHERE (realmflags & 1) = 0 ORDER BY name");

    ///- Circle through results and add them to the realm map
    if (result)
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 Id                  = fields[0].GetUInt32();
            std::string name           = fields[1].GetCppString();
            uint8 realmflags           = fields[5].GetUInt8();
            uint8 allowedSecurityLevel = fields[7].GetUInt8();

            if (realmflags & ~(REALM_FLAG_OFFLINE | REALM_FLAG_NEW_PLAYERS | REALM_FLAG_RECOMMENDED | REALM_FLAG_SPECIFYBUILD))
            {
                sLog.outError("Realm (id %u, name '%s') can only be flagged as OFFLINE (mask 0x02), NEWPLAYERS (mask 0x20), RECOMMENDED (mask 0x40), or SPECIFICBUILD (mask 0x04) in DB", Id, name.c_str());
                realmflags &= (REALM_FLAG_OFFLINE | REALM_FLAG_NEW_PLAYERS | REALM_FLAG_RECOMMENDED | REALM_FLAG_SPECIFYBUILD);
            }

            UpdateRealm(
                Id, name, fields[2].GetCppString(), fields[3].GetUInt32(),
                fields[4].GetUInt8(), RealmFlags(realmflags), fields[6].GetUInt8(),
                (allowedSecurityLevel <= SEC_ADMINISTRATOR ? AccountTypes(allowedSecurityLevel) : SEC_ADMINISTRATOR),
                fields[8].GetFloat(), fields[9].GetCppString());

            if (init)
                sLog.outString("Added realm id %u, name '%s'",  Id, name.c_str());
        }
        while (result->NextRow());
        delete result;
    }
}
