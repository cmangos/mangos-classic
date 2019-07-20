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

#include "Entities/Corpse.h"
#include "Entities/Player.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Database/DatabaseEnv.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"

Corpse::Corpse(CorpseType type) : WorldObject(),
    lootRecipient(nullptr),
    lootForBody(false)
{
    m_objectType |= TYPEMASK_CORPSE;
    m_objectTypeId = TYPEID_CORPSE;
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_ALL | UPDATEFLAG_HAS_POSITION);

    m_valuesCount = CORPSE_END;

    m_type = type;

    m_time = time(nullptr);
}

Corpse::~Corpse()
{
}

void Corpse::AddToWorld()
{
    ///- Register the corpse for guid lookup
    if (!IsInWorld())
        sObjectAccessor.AddObject(this);

    WorldObject::AddToWorld();
}

void Corpse::RemoveFromWorld()
{
    ///- Remove the corpse from the accessor
    if (IsInWorld())
        sObjectAccessor.RemoveObject(this);

    Object::RemoveFromWorld();
}

bool Corpse::Create(uint32 guidlow)
{
    Object::_Create(guidlow, 0, HIGHGUID_CORPSE);
    return true;
}

bool Corpse::Create(uint32 guidlow, Player* owner)
{
    MANGOS_ASSERT(owner);

    WorldObject::_Create(guidlow, HIGHGUID_CORPSE);
    Relocate(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());

    // we need to assign owner's map for corpse
    // in other way we will get a crash in Corpse::SaveToDB()
    SetMap(owner->GetMap());

    if (!IsPositionValid())
    {
        sLog.outError("Corpse (guidlow %d, owner %s) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
                      guidlow, owner->GetName(), owner->GetPositionX(), owner->GetPositionY());
        return false;
    }

    SetObjectScale(DEFAULT_OBJECT_SCALE);
    SetFloatValue(CORPSE_FIELD_POS_X, GetPositionX());
    SetFloatValue(CORPSE_FIELD_POS_Y, GetPositionY());
    SetFloatValue(CORPSE_FIELD_POS_Z, GetPositionZ());
    SetFloatValue(CORPSE_FIELD_FACING, GetOrientation());
    SetOwnerGuid(owner->GetObjectGuid());

    m_grid = MaNGOS::ComputeGridPair(GetPositionX(), GetPositionY());

    return true;
}

void Corpse::SaveToDB()
{
    // bones should not be saved to DB (would be deleted on startup anyway)
    MANGOS_ASSERT(GetType() != CORPSE_BONES);

    // prevent DB data inconsistence problems and duplicates
    CharacterDatabase.BeginTransaction();
    DeleteFromDB();

    std::ostringstream ss;
    ss  << "INSERT INTO corpse (guid,player,position_x,position_y,position_z,orientation,map,time,corpse_type,instance) VALUES ("
        << GetGUIDLow() << ", "
        << GetOwnerGuid().GetCounter() << ", "
        << GetPositionX() << ", "
        << GetPositionY() << ", "
        << GetPositionZ() << ", "
        << GetOrientation() << ", "
        << GetMapId() << ", "
        << uint64(m_time) << ", "
        << uint32(GetType()) << ", "
        << int(GetInstanceId()) << ")";
    CharacterDatabase.Execute(ss.str().c_str());
    CharacterDatabase.CommitTransaction();
}

void Corpse::DeleteBonesFromWorld()
{
    MANGOS_ASSERT(GetType() == CORPSE_BONES);
    Corpse* corpse = GetMap()->GetCorpse(GetObjectGuid());

    if (!corpse)
    {
        sLog.outError("Bones %u not found in world.", GetGUIDLow());
        return;
    }

    AddObjectToRemoveList();
}

void Corpse::DeleteFromDB() const
{
    // bones should not be saved to DB (would be deleted on startup anyway)
    MANGOS_ASSERT(GetType() != CORPSE_BONES);

    // all corpses (not bones)
    static SqlStatementID id;

    SqlStatement stmt = CharacterDatabase.CreateStatement(id, "DELETE FROM corpse WHERE player = ? AND corpse_type <> '0'");
    stmt.PExecute(GetOwnerGuid().GetCounter());
}

bool Corpse::LoadFromDB(uint32 lowguid, Field* fields)
{
    ////                                                    0            1       2                  3                  4                  5                   6
    // QueryResult *result = CharacterDatabase.Query("SELECT corpse.guid, player, corpse.position_x, corpse.position_y, corpse.position_z, corpse.orientation, corpse.map,"
    ////   7     8            9         10      11    12     13           14            15              16       17
    //    "time, corpse_type, instance, gender, race, class, playerBytes, playerBytes2, equipmentCache, guildId, playerFlags FROM corpse"
    uint32 playerLowGuid = fields[1].GetUInt32();
    float positionX     = fields[2].GetFloat();
    float positionY     = fields[3].GetFloat();
    float positionZ     = fields[4].GetFloat();
    float orientation   = fields[5].GetFloat();
    uint32 mapid        = fields[6].GetUInt32();

    Object::_Create(lowguid, 0, HIGHGUID_CORPSE);

    m_time = time_t(fields[7].GetUInt64());
    m_type = CorpseType(fields[8].GetUInt32());

    if (m_type >= MAX_CORPSE_TYPE)
    {
        sLog.outError("%s Owner %s have wrong corpse type (%i), not load.", GetGuidStr().c_str(), GetOwnerGuid().GetString().c_str(), m_type);
        return false;
    }

    uint32 instanceid   = fields[9].GetUInt32();
    uint8 gender        = fields[10].GetUInt8();
    uint8 race          = fields[11].GetUInt8();
    uint8 _class        = fields[12].GetUInt8();
    uint32 playerBytes  = fields[13].GetUInt32();
    uint32 playerBytes2 = fields[14].GetUInt32();
    uint32 guildId      = fields[16].GetUInt32();
    uint32 playerFlags  = fields[17].GetUInt32();

    ObjectGuid guid = ObjectGuid(HIGHGUID_CORPSE, lowguid);
    ObjectGuid playerGuid = ObjectGuid(HIGHGUID_PLAYER, playerLowGuid);

    // overwrite possible wrong/corrupted guid
    SetGuidValue(OBJECT_FIELD_GUID, guid);
    SetOwnerGuid(playerGuid);

    SetObjectScale(DEFAULT_OBJECT_SCALE);

    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(race, _class);
    if (!info)
    {
        sLog.outError("Player %u has incorrect race/class pair.", GetGUIDLow());
        return false;
    }
    SetUInt32Value(CORPSE_FIELD_DISPLAY_ID, gender == GENDER_FEMALE ? info->displayId_f : info->displayId_m);

    // Load equipment
    Tokens data = StrSplit(fields[15].GetCppString(), " ");
    for (uint8 slot = 0; slot < EQUIPMENT_SLOT_END; ++slot)
    {
        uint32 visualbase = slot * 2;
        uint32 item_id = GetUInt32ValueFromArray(data, visualbase);
        const ItemPrototype* proto = ObjectMgr::GetItemPrototype(item_id);
        if (!proto)
        {
            SetUInt32Value(CORPSE_FIELD_ITEM + slot, 0);
            continue;
        }

        SetUInt32Value(CORPSE_FIELD_ITEM + slot, proto->DisplayInfoID | (proto->InventoryType << 24));
    }

    uint8 skin       = (uint8)(playerBytes);
    uint8 face       = (uint8)(playerBytes >> 8);
    uint8 hairstyle  = (uint8)(playerBytes >> 16);
    uint8 haircolor  = (uint8)(playerBytes >> 24);
    uint8 facialhair = (uint8)(playerBytes2);
    SetUInt32Value(CORPSE_FIELD_BYTES_1, ((0x00) | (race << 8) | (gender << 16) | (skin << 24)));
    SetUInt32Value(CORPSE_FIELD_BYTES_2, ((face) | (hairstyle << 8) | (haircolor << 16) | (facialhair << 24)));

    SetUInt32Value(CORPSE_FIELD_GUILD, guildId);

    uint32 flags = CORPSE_FLAG_UNK2;
    if (playerFlags & PLAYER_FLAGS_HIDE_HELM)
        flags |= CORPSE_FLAG_HIDE_HELM;
    if (playerFlags & PLAYER_FLAGS_HIDE_CLOAK)
        flags |= CORPSE_FLAG_HIDE_CLOAK;
    SetUInt32Value(CORPSE_FIELD_FLAGS, flags);

    // no need to mark corpse as lootable, because corpses are not saved in battle grounds

    // place
    SetLocationInstanceId(instanceid);
    SetLocationMapId(mapid);
    Relocate(positionX, positionY, positionZ, orientation);

    if (!IsPositionValid())
    {
        sLog.outError("%s Owner %s not created. Suggested coordinates isn't valid (X: %f Y: %f)",
                      GetGuidStr().c_str(), GetOwnerGuid().GetString().c_str(), GetPositionX(), GetPositionY());
        return false;
    }

    m_grid = MaNGOS::ComputeGridPair(GetPositionX(), GetPositionY());

    return true;
}

uint32 Corpse::getFaction() const
{
    if (const uint8 race = getRace())
    {
        if (const ChrRacesEntry* raceEntry = sChrRacesStore.LookupEntry(race))
            return raceEntry->FactionID;
    }
    return 0;
}

bool Corpse::isVisibleForInState(Player const* u, WorldObject const* viewPoint, bool /*inVisibleList*/) const
{
    return IsInWorld() && u->IsInWorld() && IsWithinDistInMap(viewPoint, GetMap()->GetVisibilityDistance(), false);
}

bool Corpse::IsExpired(time_t t) const
{
    if (m_type == CORPSE_BONES)
        return m_time < t - 60 * MINUTE;
    return m_time < t - 3 * DAY;
}
