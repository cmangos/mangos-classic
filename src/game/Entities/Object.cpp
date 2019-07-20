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

#include "Entities/Object.h"
#include "Globals/SharedDefines.h"
#include "WorldPacket.h"
#include "Server/Opcodes.h"
#include "Log.h"
#include "World/World.h"
#include "Entities/Creature.h"
#include "Entities/Player.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Entities/UpdateData.h"
#include "Entities/UpdateMask.h"
#include "Util.h"
#include "Maps/MapManager.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Maps/ObjectPosSelector.h"
#include "Entities/TemporarySpawn.h"
#include "Movement/packet_builder.h"
#include "Entities/CreatureLinkingMgr.h"
#include "Chat/Chat.h"
#include "Loot/LootMgr.h"
#include "Spells/SpellMgr.h"

constexpr float VisibilityDistances[AsUnderlyingType(VisibilityDistanceType::Max)] =
{
    DEFAULT_VISIBILITY_DISTANCE,
    VISIBILITY_DISTANCE_TINY,
    VISIBILITY_DISTANCE_SMALL,
    VISIBILITY_DISTANCE_LARGE,
    VISIBILITY_DISTANCE_GIGANTIC,
    MAX_VISIBILITY_DISTANCE
};

Object::Object(): m_updateFlag(0), m_itsNewObject(false)
{
    m_objectTypeId      = TYPEID_OBJECT;
    m_objectType        = TYPEMASK_OBJECT;

    m_uint32Values      = nullptr;
    m_valuesCount       = 0;

    m_inWorld           = false;
    m_objectUpdated     = false;
    loot              = nullptr;
}

Object::~Object()
{
    if (IsInWorld())
    {
        ///- Do NOT call RemoveFromWorld here, if the object is a player it will crash
        sLog.outError("Object::~Object (GUID: %u TypeId: %u) deleted but still in world!!", GetGUIDLow(), GetTypeId());
        MANGOS_ASSERT(false);
    }

    if (m_objectUpdated)
    {
        sLog.outError("Object::~Object (GUID: %u TypeId: %u) deleted but still have updated status!!", GetGUIDLow(), GetTypeId());
        MANGOS_ASSERT(false);
    }

    delete[] m_uint32Values;

    delete loot;
}

void Object::_InitValues()
{
    m_uint32Values = new uint32[ m_valuesCount ];
    memset(m_uint32Values, 0, m_valuesCount * sizeof(uint32));

    m_changedValues.resize(m_valuesCount, false);

    m_objectUpdated = false;
}

void Object::_Create(uint32 guidlow, uint32 entry, HighGuid guidhigh)
{
    if (!m_uint32Values)
        _InitValues();

    ObjectGuid guid = ObjectGuid(guidhigh, entry, guidlow);
    SetGuidValue(OBJECT_FIELD_GUID, guid);
    SetUInt32Value(OBJECT_FIELD_TYPE, m_objectType);
    m_PackGUID.Set(guid);
}

void Object::SetObjectScale(float newScale)
{
    SetFloatValue(OBJECT_FIELD_SCALE_X, newScale);
}

void Object::SendForcedObjectUpdate()
{
    if (!m_inWorld || !m_objectUpdated)
        return;

    UpdateDataMapType update_players;

    BuildUpdateData(update_players);
    RemoveFromClientUpdateList();

    // here we allocate a std::vector with a size of 0x10000
    for (auto& update_player : update_players)
    {
        for (size_t i = 0; i < update_player.second.GetPacketCount(); ++i)
        {
            WorldPacket packet = update_player.second.BuildPacket(i);
            update_player.first->GetSession()->SendPacket(packet);
        }
    }
}

void Object::BuildMovementUpdateBlock(UpdateData* data, uint8 flags) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_MOVEMENT);
    buf << GetObjectGuid();

    BuildMovementUpdate(&buf, flags);

    data->AddUpdateBlock(buf);
}

void Object::BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    if (!target)
        return;

    uint8  updatetype   = UPDATETYPE_CREATE_OBJECT;
    uint8 updateFlags  = m_updateFlag;

    /** lower flag1 **/
    if (target == this)                                     // building packet for yourself
        updateFlags |= UPDATEFLAG_SELF;

    if (m_itsNewObject)
    {
        switch (GetObjectGuid().GetHigh())
        {
            case HighGuid::HIGHGUID_DYNAMICOBJECT:
            case HighGuid::HIGHGUID_CORPSE:
            case HighGuid::HIGHGUID_PLAYER:
            case HighGuid::HIGHGUID_UNIT:
            case HighGuid::HIGHGUID_GAMEOBJECT:
                updatetype = UPDATETYPE_CREATE_OBJECT2;
                break;

            default:
                break;
        }
    }

    // DEBUG_LOG("BuildCreateUpdate: update-type: %u, object-type: %u got updateFlags: %X", updatetype, m_objectTypeId, updateFlags);

    ByteBuffer buf(500);
    buf << uint8(updatetype);
    buf << GetPackGUID();
    buf << uint8(m_objectTypeId);

    BuildMovementUpdate(&buf, updateFlags);
    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);
    _SetCreateBits(&updateMask, target);
    BuildValuesUpdate(updatetype, &buf, &updateMask, target);
    data->AddUpdateBlock(buf);
}

void Object::SendCreateUpdateToPlayer(Player* player) const
{
    // send create update to player
    UpdateData updateData;
    BuildCreateUpdateBlockForPlayer(&updateData, player);

    for (size_t i = 0; i < updateData.GetPacketCount(); ++i)
    {
        WorldPacket packet = updateData.BuildPacket(i);
        player->GetSession()->SendPacket(packet);
    }
}

void Object::BuildValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_VALUES);
    buf << GetPackGUID();

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    _SetUpdateBits(&updateMask, target);
    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target);

    data->AddUpdateBlock(buf);
}

void Object::BuildForcedValuesUpdateBlockForPlayer(UpdateData* data, Player* target) const
{
    ByteBuffer buf(500);

    buf << uint8(UPDATETYPE_VALUES);
    buf << GetPackGUID();

    UpdateMask updateMask;
    updateMask.SetCount(m_valuesCount);

    _SetCreateBits(&updateMask, target);
    BuildValuesUpdate(UPDATETYPE_VALUES, &buf, &updateMask, target);

    data->AddUpdateBlock(buf);
}

void Object::BuildOutOfRangeUpdateBlock(UpdateData* data) const
{
    data->AddOutOfRangeGUID(GetObjectGuid());
}

void Object::DestroyForPlayer(Player* target) const
{
    MANGOS_ASSERT(target);

    WorldPacket data(SMSG_DESTROY_OBJECT, 8);
    data << GetObjectGuid();
    target->GetSession()->SendPacket(data);
}

void Object::BuildMovementUpdate(ByteBuffer* data, uint8 updateFlags) const
{
    Unit const* unit = nullptr;
    uint32 highGuid = 0;
    MovementFlags moveflags = MOVEFLAG_NONE;

    switch (m_objectTypeId)
    {
        case TYPEID_OBJECT:
        case TYPEID_ITEM:
        case TYPEID_CONTAINER:
        case TYPEID_GAMEOBJECT:
        case TYPEID_DYNAMICOBJECT:
        case TYPEID_CORPSE:
            highGuid = uint32(GetObjectGuid().GetHigh());
            break;

        case TYPEID_PLAYER:
            // TODO: this code must not be here
            if (static_cast<Player const*>(this)->GetTransport())
                ((Unit*)this)->m_movementInfo.AddMovementFlag(MOVEFLAG_ONTRANSPORT);
            else
                ((Unit*)this)->m_movementInfo.RemoveMovementFlag(MOVEFLAG_ONTRANSPORT);

        case TYPEID_UNIT:
            unit = static_cast<Unit const*>(this);
            moveflags = unit->m_movementInfo.GetMovementFlags();
            break;

        default:
            break;
    }

    *data << uint8(updateFlags);

    if (updateFlags & UPDATEFLAG_LIVING)
    {
        MANGOS_ASSERT(unit);
        if (unit->IsStopped() && unit->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
        {
            sLog.outError("%s is not moving but have spline movement enabled!", GetGuidStr().c_str());
            ((Unit*)this)->m_movementInfo.RemoveMovementFlag(MovementFlags(MOVEFLAG_SPLINE_ENABLED | MOVEFLAG_FORWARD));
        }

        *data << unit->m_movementInfo;
        // Unit speeds
        *data << float(unit->GetSpeed(MOVE_WALK));
        *data << float(unit->GetSpeed(MOVE_RUN));
        *data << float(unit->GetSpeed(MOVE_RUN_BACK));
        *data << float(unit->GetSpeed(MOVE_SWIM));
        *data << float(unit->GetSpeed(MOVE_SWIM_BACK));
        *data << float(unit->GetSpeed(MOVE_TURN_RATE));

        if (unit->m_movementInfo.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
            Movement::PacketBuilder::WriteCreate(*unit->movespline, *data);
    }
    else if (updateFlags & UPDATEFLAG_HAS_POSITION)
    {
        *data << ((WorldObject*)this)->GetPositionX();
        *data << ((WorldObject*)this)->GetPositionY();
        *data << ((WorldObject*)this)->GetPositionZ();
        *data << ((WorldObject*)this)->GetOrientation();
    }

    if (updateFlags & UPDATEFLAG_HIGHGUID)
        *data << highGuid;

    if (updateFlags & UPDATEFLAG_ALL)
        *data << (uint32)0x1;

    if (updateFlags & UPDATEFLAG_FULLGUID)
    {
        if (unit && unit->getVictim())
            *data << unit->getVictim()->GetPackGUID();
        else
            data->appendPackGUID(0);
    }

    if (updateFlags & UPDATEFLAG_TRANSPORT)
        *data << uint32(WorldTimer::getMSTime());
}

void Object::BuildValuesUpdate(uint8 updatetype, ByteBuffer* data, UpdateMask* updateMask, Player* target) const
{
    if (!target)
        return;

    bool IsActivateToQuest = false;
    bool IsPerCasterAuraState = false;

    if (updatetype == UPDATETYPE_CREATE_OBJECT || updatetype == UPDATETYPE_CREATE_OBJECT2)
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
                IsActivateToQuest = true;

            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);
        }
    }
    else                                                    // case UPDATETYPE_VALUES
    {
        if (isType(TYPEMASK_GAMEOBJECT) && !((GameObject*)this)->IsTransport())
        {
            if (((GameObject*)this)->ActivateToQuest(target) || target->isGameMaster())
                IsActivateToQuest = true;

            updateMask->SetBit(GAMEOBJECT_DYN_FLAGS);
            updateMask->SetBit(GAMEOBJECT_ANIMPROGRESS);
        }
    }

    MANGOS_ASSERT(updateMask && updateMask->GetCount() == m_valuesCount);

    *data << (uint8)updateMask->GetBlockCount();
    data->append(updateMask->GetMask(), updateMask->GetLength());

    // 2 specialized loops for speed optimization in non-unit case
    if (isType(TYPEMASK_UNIT))                              // unit (creature/player) case
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                if (index == UNIT_NPC_FLAGS)
                {
                    uint32 appendValue = m_uint32Values[index];

                    if (GetTypeId() == TYPEID_UNIT)
                    {
                        if (appendValue & UNIT_NPC_FLAG_TRAINER)
                        {
                            if (!((Creature*)this)->IsTrainerOf(target, false))
                                appendValue &= ~UNIT_NPC_FLAG_TRAINER;
                        }

                        if (appendValue & UNIT_NPC_FLAG_STABLEMASTER)
                        {
                            if (target->getClass() != CLASS_HUNTER)
                                appendValue &= ~UNIT_NPC_FLAG_STABLEMASTER;
                        }

                        if (appendValue & UNIT_NPC_FLAG_FLIGHTMASTER)
                        {
                            QuestRelationsMapBounds bounds = sObjectMgr.GetCreatureQuestRelationsMapBounds(((Creature*)this)->GetEntry());
                            for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);
                                if (target->CanSeeStartQuest(pQuest))
                                {
                                    appendValue &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                                    break;
                                }
                            }

                            bounds = sObjectMgr.GetCreatureQuestInvolvedRelationsMapBounds(((Creature*)this)->GetEntry());
                            for (QuestRelationsMap::const_iterator itr = bounds.first; itr != bounds.second; ++itr)
                            {
                                Quest const* pQuest = sObjectMgr.GetQuestTemplate(itr->second);
                                if (target->CanRewardQuest(pQuest, false))
                                {
                                    appendValue &= ~UNIT_NPC_FLAG_FLIGHTMASTER;
                                    break;
                                }
                            }
                        }
                    }

                    *data << uint32(appendValue);
                }
                // FIXME: Some values at server stored in float format but must be sent to client in uint32 format
                else if (index >= UNIT_FIELD_BASEATTACKTIME && index <= UNIT_FIELD_RANGEDATTACKTIME)
                {
                    // convert from float to uint32 and send
                    *data << uint32(m_floatValues[index] < 0 ? 0 : m_floatValues[index]);
                }

                // there are some float values which may be negative or can't get negative due to other checks
                else if ((index >= PLAYER_FIELD_NEGSTAT0    && index <= PLAYER_FIELD_NEGSTAT4) ||
                         (index >= PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE  && index <= (PLAYER_FIELD_RESISTANCEBUFFMODSPOSITIVE + 6)) ||
                         (index >= PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE  && index <= (PLAYER_FIELD_RESISTANCEBUFFMODSNEGATIVE + 6)) ||
                         (index >= PLAYER_FIELD_POSSTAT0    && index <= PLAYER_FIELD_POSSTAT4))
                {
                    *data << uint32(m_floatValues[index]);
                }
                else if (index == UNIT_FIELD_HEALTH || index == UNIT_FIELD_MAXHEALTH)
                {
                    uint32 value = m_uint32Values[index];

                    // Fog of War: replace absolute health values with percentages for non-allied units according to settings
                    if (!static_cast<const Unit*>(this)->IsFogOfWarVisibleHealth(target))
                    {
                        switch (index)
                        {
                            case UNIT_FIELD_HEALTH:     value = uint32(ceil((100.0 * value) / m_uint32Values[UNIT_FIELD_MAXHEALTH]));   break;
                            case UNIT_FIELD_MAXHEALTH:  value = 100;                                                                    break;
                        }
                    }

                    *data << value;
                }
                // Fog of War: hide stat values for non-allied units according to settings
                else if ((index == UNIT_FIELD_RANGEDATTACKTIME ||
                          index == UNIT_FIELD_MINDAMAGE || index == UNIT_FIELD_MAXDAMAGE ||
                          index == UNIT_FIELD_MINOFFHANDDAMAGE || index == UNIT_FIELD_MAXOFFHANDDAMAGE ||
                          (index >= UNIT_FIELD_STAT0 && index < UNIT_FIELD_BASE_MANA) ||
                          index == UNIT_FIELD_BASE_HEALTH || index == UNIT_FIELD_ATTACK_POWER ||
                          index == UNIT_FIELD_ATTACK_POWER_MODS || index == UNIT_FIELD_ATTACK_POWER_MULTIPLIER ||
                          index == UNIT_FIELD_RANGED_ATTACK_POWER || index == UNIT_FIELD_RANGED_ATTACK_POWER_MODS ||
                          index == UNIT_FIELD_RANGED_ATTACK_POWER_MULTIPLIER || index == UNIT_FIELD_MINRANGEDDAMAGE ||
                          index == UNIT_FIELD_MAXRANGEDDAMAGE || (index >= UNIT_FIELD_POWER_COST_MODIFIER && index <= UNIT_FIELD_POWER_COST_MULTIPLIER_06)) &&
                          !static_cast<const Unit*>(this)->IsFogOfWarVisibleStats(target))
                {
                    *data << uint32(0);
                }

                // Gamemasters should be always able to select units - remove not selectable flag
                else if (index == UNIT_FIELD_FLAGS && target->isGameMaster())
                {
                    *data << (m_uint32Values[index] & ~UNIT_FLAG_NOT_SELECTABLE);
                }
                // Hide lootable animation for unallowed players
                // Handle tapped flag
                else if (index == UNIT_DYNAMIC_FLAGS && GetTypeId() == TYPEID_UNIT)
                {
                    Creature* creature = (Creature*)this;
                    uint32 dynflagsValue = m_uint32Values[index];
                    bool setTapFlags = false;

                    if (creature->isAlive())
                    {
                        // creature is alive so, not lootable
                        dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_LOOTABLE;

                        if (creature->isInCombat())
                        {
                            // as creature is in combat we have to manage tap flags
                            setTapFlags = true;
                        }
                        else
                        {
                            // creature is not in combat so its not tapped
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is not in combat so not tapped by %s", this->GetGuidStr().c_str(), target->GetGuidStr().c_str());
                        }
                    }
                    else
                    {
                        // check loot flag
                        if (creature->loot && creature->loot->CanLoot(target))
                        {
                            // creature is dead and this player can loot it
                            dynflagsValue = dynflagsValue | UNIT_DYNFLAG_LOOTABLE;
                            //sLog.outString(">> %s is lootable for %s", this->GetGuidStr().c_str(), target->GetGuidStr().c_str());
                        }
                        else
                        {
                            // creature is dead but this player cannot loot it
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_LOOTABLE;
                            //sLog.outString(">> %s is not lootable for %s", this->GetGuidStr().c_str(), target->GetGuidStr().c_str());
                        }

                        // as creature is died we have to manage tap flags
                        setTapFlags = true;
                    }

                    // check tap flags
                    if (setTapFlags)
                    {
                        if (creature->IsTappedBy(target))
                        {
                            // creature is in combat or died and tapped by this player
                            dynflagsValue = dynflagsValue & ~UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is tapped by %s", this->GetGuidStr().c_str(), target->GetGuidStr().c_str());
                        }
                        else
                        {
                            // creature is in combat or died but not tapped by this player
                            dynflagsValue = dynflagsValue | UNIT_DYNFLAG_TAPPED;
                            //sLog.outString(">> %s is not tapped by %s", this->GetGuidStr().c_str(), target->GetGuidStr().c_str());
                        }
                    }

                    if (GetTypeId() == TYPEID_UNIT || GetTypeId() == TYPEID_PLAYER)
                    {
                        Unit* unit = (Unit*)this; // hunters mark effects should only be visible to owners and not all players
                        if (!unit->HasAuraTypeWithCaster(SPELL_AURA_MOD_STALKED, target->GetObjectGuid()))
                            dynflagsValue &= ~UNIT_DYNFLAG_TRACK_UNIT;
                    }

                    *data << dynflagsValue;
                }
                else if (index == UNIT_FIELD_FACTIONTEMPLATE)
                {
                    uint32 value = m_uint32Values[index];

                    // [XFACTION]: Alter faction if detected crossfaction group interaction when updating faction field:
                    if (this != target && GetTypeId() == TYPEID_PLAYER && sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                    {
                        Player const* thisPlayer = static_cast<Player const*>(this);
                        const uint32 targetTeam = target->GetTeam();

                        if (thisPlayer->GetTeam() != targetTeam && !thisPlayer->HasCharmer() && target->IsInGroup(thisPlayer))
                        {
                            switch (targetTeam)
                            {
                                case ALLIANCE:  value = 1054;   break;  // "Alliance Generic"
                                case HORDE:     value = 1495;   break;  // "Horde Generic"
                            }
                        }
                    }

                    *data << value;
                }
                else                                        // Unhandled index, just send
                {
                    // send in current format (float as float, uint32 as uint32)
                    *data << m_uint32Values[index];
                }
            }
        }
    }
    else if (isType(TYPEMASK_GAMEOBJECT))                   // gameobject case
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                if (index == GAMEOBJECT_DYN_FLAGS)
                {
                    if (IsActivateToQuest)
                    {
                        GameObject const* gameObject = static_cast<GameObject const*>(this);
                        switch (((GameObject*)this)->GetGoType())
                        {
                            case GAMEOBJECT_TYPE_QUESTGIVER:
                            case GAMEOBJECT_TYPE_CHEST:
                                if (gameObject->GetLootState() == GO_READY || gameObject->GetLootState() == GO_ACTIVATED)
                                    *data << uint16(GO_DYNFLAG_LO_ACTIVATE | GO_DYNFLAG_LO_SPARKLE);
                                else
                                    *data << uint16(0);
                                *data << uint16(0);
                                break;
                            case GAMEOBJECT_TYPE_GENERIC:
                            case GAMEOBJECT_TYPE_SPELL_FOCUS:
                            case GAMEOBJECT_TYPE_GOOBER:
                                *data << uint16(GO_DYNFLAG_LO_ACTIVATE);
                                *data << uint16(0);
                                break;
                            default:
                                *data << uint32(0);         // unknown, not happen.
                                break;
                        }
                    }
                    else
                        *data << uint32(0);                 // disable quest object
                }
                else
                    *data << m_uint32Values[index];         // other cases
            }
        }
    }
    else                                                    // other objects case (no special index checks)
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
        {
            if (updateMask->GetBit(index))
            {
                // send in current format (float as float, uint32 as uint32)
                *data << m_uint32Values[index];
            }
        }
    }
}

void Object::ClearUpdateMask(bool remove)
{
    if (m_uint32Values)
    {
        for (uint16 index = 0; index < m_valuesCount; ++index)
            m_changedValues[index] = false;
    }

    if (m_objectUpdated)
    {
        if (remove)
            RemoveFromClientUpdateList();
        m_objectUpdated = false;
    }
}

bool Object::LoadValues(const char* data)
{
    if (!m_uint32Values) _InitValues();

    Tokens tokens = StrSplit(data, " ");

    if (tokens.size() != m_valuesCount)
        return false;

    Tokens::iterator iter;
    int index;
    for (iter = tokens.begin(), index = 0; index < m_valuesCount; ++iter, ++index)
    {
        m_uint32Values[index] = std::stoul(*iter);
    }

    return true;
}

void Object::_SetUpdateBits(UpdateMask* updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; ++index)
        if (m_changedValues[index])
            updateMask->SetBit(index);
}

void Object::_SetCreateBits(UpdateMask* updateMask, Player* /*target*/) const
{
    for (uint16 index = 0; index < m_valuesCount; ++index)
        if (GetUInt32Value(index) != 0)
            updateMask->SetBit(index);
}

void Object::SetInt32Value(uint16 index, int32 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_int32Values[index] != value)
    {
        m_int32Values[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetUInt32Value(uint16 index, uint32 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_uint32Values[index] != value)
    {
        m_uint32Values[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetUInt64Value(uint16 index, const uint64& value)
{
    MANGOS_ASSERT(index + 1 < m_valuesCount || PrintIndexError(index, true));
    if (*((uint64*) & (m_uint32Values[index])) != value)
    {
        m_uint32Values[index] = *((uint32*)&value);
        m_uint32Values[index + 1] = *(((uint32*)&value) + 1);
        m_changedValues[index] = true;
        m_changedValues[index + 1] = true;
        MarkForClientUpdate();
    }
}

void Object::SetFloatValue(uint16 index, float value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (m_floatValues[index] != value)
    {
        m_floatValues[index] = value;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetByteValue(uint16 index, uint8 offset, uint8 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::SetByteValue: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> (offset * 8)) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFF) << (offset * 8));
        m_uint32Values[index] |= uint32(uint32(value) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetUInt16Value(uint16 index, uint8 offset, uint16 value)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 2)
    {
        sLog.outError("Object::SetUInt16Value: wrong offset %u", offset);
        return;
    }

    if (uint16(m_uint32Values[index] >> (offset * 16)) != value)
    {
        m_uint32Values[index] &= ~uint32(uint32(0xFFFF) << (offset * 16));
        m_uint32Values[index] |= uint32(uint32(value) << (offset * 16));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetStatFloatValue(uint16 index, float value)
{
    if (value < 0)
        value = 0.0f;

    SetFloatValue(index, value);
}

void Object::SetStatInt32Value(uint16 index, int32 value)
{
    if (value < 0)
        value = 0;

    SetUInt32Value(index, uint32(value));
}

void Object::ApplyModUInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetUInt32Value(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetUInt32Value(index, cur);
}

void Object::ApplyModInt32Value(uint16 index, int32 val, bool apply)
{
    int32 cur = GetInt32Value(index);
    cur += (apply ? val : -val);
    SetInt32Value(index, cur);
}

void Object::ApplyModSignedFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    SetFloatValue(index, cur);
}

void Object::ApplyModPositiveFloatValue(uint16 index, float  val, bool apply)
{
    float cur = GetFloatValue(index);
    cur += (apply ? val : -val);
    if (cur < 0)
        cur = 0;
    SetFloatValue(index, cur);
}

void Object::SetFlag(uint16 index, uint32 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval | newFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveFlag(uint16 index, uint32 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));
    uint32 oldval = m_uint32Values[index];
    uint32 newval = oldval & ~oldFlag;

    if (oldval != newval)
    {
        m_uint32Values[index] = newval;
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetByteFlag(uint16 index, uint8 offset, uint8 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::SetByteFlag: wrong offset %u", offset);
        return;
    }

    if (!(uint8(m_uint32Values[index] >> (offset * 8)) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveByteFlag(uint16 index, uint8 offset, uint8 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (offset > 4)
    {
        sLog.outError("Object::RemoveByteFlag: wrong offset %u", offset);
        return;
    }

    if (uint8(m_uint32Values[index] >> (offset * 8)) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << (offset * 8));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::SetShortFlag(uint16 index, bool highpart, uint16 newFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (!(uint16(m_uint32Values[index] >> (highpart ? 16 : 0)) & newFlag))
    {
        m_uint32Values[index] |= uint32(uint32(newFlag) << (highpart ? 16 : 0));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

void Object::RemoveShortFlag(uint16 index, bool highpart, uint16 oldFlag)
{
    MANGOS_ASSERT(index < m_valuesCount || PrintIndexError(index, true));

    if (uint16(m_uint32Values[index] >> (highpart ? 16 : 0)) & oldFlag)
    {
        m_uint32Values[index] &= ~uint32(uint32(oldFlag) << (highpart ? 16 : 0));
        m_changedValues[index] = true;
        MarkForClientUpdate();
    }
}

bool Object::PrintIndexError(uint32 index, bool set) const
{
    sLog.outError("Attempt %s nonexistent value field: %u (count: %u) for object typeid: %u type mask: %u", (set ? "set value to" : "get value from"), index, m_valuesCount, GetTypeId(), m_objectType);

    // ASSERT must fail after function call
    return false;
}

bool Object::PrintEntryError(char const* descr) const
{
    sLog.outError("Object Type %u, Entry %u (lowguid %u) with invalid call for %s", GetTypeId(), GetEntry(), GetObjectGuid().GetCounter(), descr);

    // always false for continue assert fail
    return false;
}


void Object::BuildUpdateDataForPlayer(Player* pl, UpdateDataMapType& update_players) const
{
    UpdateDataMapType::iterator iter = update_players.find(pl);

    if (iter == update_players.end())
    {
        std::pair<UpdateDataMapType::iterator, bool> p = update_players.insert(UpdateDataMapType::value_type(pl, UpdateData()));
        MANGOS_ASSERT(p.second);
        iter = p.first;
    }

    BuildValuesUpdateBlockForPlayer(&iter->second, iter->first);
}

void Object::AddToClientUpdateList()
{
    sLog.outError("Unexpected call of Object::AddToClientUpdateList for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::RemoveFromClientUpdateList()
{
    sLog.outError("Unexpected call of Object::RemoveFromClientUpdateList for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::BuildUpdateData(UpdateDataMapType& /*update_players */)
{
    sLog.outError("Unexpected call of Object::BuildUpdateData for object (TypeId: %u Update fields: %u)", GetTypeId(), m_valuesCount);
    MANGOS_ASSERT(false);
}

void Object::MarkForClientUpdate()
{
    if (m_inWorld)
    {
        if (!m_objectUpdated)
        {
            AddToClientUpdateList();
            m_objectUpdated = true;
        }
    }
}

void Object::ForceValuesUpdateAtIndex(uint16 index)
{
    m_changedValues[index] = true;
    if (m_inWorld && !m_objectUpdated)
    {
        AddToClientUpdateList();
        m_objectUpdated = true;
    }
}

WorldObject::WorldObject() :
    m_isOnEventNotified(false),
    m_currMap(nullptr), m_mapId(0),
    m_InstanceId(0), m_isActiveObject(false),
    m_visibilityDistanceOverride(0.f)
{
}

void WorldObject::CleanupsBeforeDelete()
{
    RemoveFromWorld();
}

void WorldObject::_Create(uint32 guidlow, HighGuid guidhigh)
{
    Object::_Create(guidlow, 0, guidhigh);
}

void WorldObject::Relocate(float x, float y, float z, float orientation)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;
    m_position.o = orientation;

    if (isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangePosition(x, y, z, orientation);
}

void WorldObject::Relocate(float x, float y, float z)
{
    m_position.x = x;
    m_position.y = y;
    m_position.z = z;

    if (isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangePosition(x, y, z, GetOrientation());
}

void WorldObject::SetOrientation(float orientation)
{
    m_position.o = orientation;

    if (isType(TYPEMASK_UNIT))
        ((Unit*)this)->m_movementInfo.ChangeOrientation(orientation);
}

uint32 WorldObject::GetZoneId() const
{
    return GetTerrain()->GetZoneId(m_position.x, m_position.y, m_position.z);
}

uint32 WorldObject::GetAreaId() const
{
    return GetTerrain()->GetAreaId(m_position.x, m_position.y, m_position.z);
}

void WorldObject::GetZoneAndAreaId(uint32& zoneid, uint32& areaid) const
{
    GetTerrain()->GetZoneAndAreaId(zoneid, areaid, m_position.x, m_position.y, m_position.z);
}

InstanceData* WorldObject::GetInstanceData() const
{
    return GetMap()->GetInstanceData();
}

float WorldObject::GetDistance(const WorldObject* obj, bool is3D, DistanceCalculation distcalc) const
{
    float dx = GetPositionX() - obj->GetPositionX();
    float dy = GetPositionY() - obj->GetPositionY();
    float distsq = dx * dx + dy * dy;

    if (is3D)
    {
        float dz = GetPositionZ() - obj->GetPositionZ();
        distsq += dz * dz;
    }

    switch (distcalc)
    {
        case DIST_CALC_BOUNDING_RADIUS:
        {
            float sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
            float dist = sqrt(distsq) - sizefactor;
            return dist > 0.0f ? dist : 0.0f;
        }
        case DIST_CALC_COMBAT_REACH:
        case DIST_CALC_COMBAT_REACH_WITH_MELEE:
        {
            float sizefactor = GetCombinedCombatReach(obj, distcalc == DIST_CALC_COMBAT_REACH_WITH_MELEE);
            float dist = sqrt(distsq) - sizefactor;
            return dist > 0.0f ? dist : 0.0f;
        }
        default: return distsq;
    }
}

float WorldObject::GetDistance(float x, float y, float z, DistanceCalculation distcalc) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dz = GetPositionZ() - z;
    float dist = dx * dx + dy * dy + dz * dz;

    switch (distcalc)
    {
        case DIST_CALC_BOUNDING_RADIUS:
        {
            float sizefactor = GetObjectBoundingRadius();
            dist = sqrt(dist) - sizefactor;
            return dist > 0.0f ? dist : 0.0f;
        }
        case DIST_CALC_COMBAT_REACH:
        case DIST_CALC_COMBAT_REACH_WITH_MELEE:
        {
            float reach = GetCombinedCombatReach(distcalc == DIST_CALC_COMBAT_REACH_WITH_MELEE);
            dist = sqrt(dist) - reach;
            return dist > 0.0f ? dist : 0.0f;
        }
        default: return dist;
    }
}

float WorldObject::GetDistance2d(float x, float y, DistanceCalculation distcalc) const
{
    float dx = GetPositionX() - x;
    float dy = GetPositionY() - y;
    float dist = dx * dx + dy * dy;

    switch (distcalc)
    {
        case DIST_CALC_BOUNDING_RADIUS:
        {
            float sizefactor = GetObjectBoundingRadius();
            dist = sqrt(dist) - sizefactor;
            return dist > 0.0f ? dist : 0.0f;
        }
        case DIST_CALC_COMBAT_REACH:
        case DIST_CALC_COMBAT_REACH_WITH_MELEE:
        {
            float reach = GetCombinedCombatReach(distcalc == DIST_CALC_COMBAT_REACH_WITH_MELEE);
            dist = sqrt(dist) - reach;
            return dist > 0.0f ? dist : 0.0f;
        }
        default: return dist;
    }
}

float WorldObject::GetDistanceZ(const WorldObject* obj) const
{
    float dz = fabs(GetPositionZ() - obj->GetPositionZ());
    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float dist = dz - sizefactor;
    return dist > 0 ? dist : 0;
}

bool WorldObject::IsWithinDist3d(float x, float y, float z, float dist2compare) const
{
    float distsq = GetDistance(x, y, z, DIST_CALC_NONE);
    float sizefactor = GetCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinDist2d(float x, float y, float dist2compare) const
{
    float distsq = GetDistance2d(x, y, DIST_CALC_NONE);
    float sizefactor = GetCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::_IsWithinDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float distsq = GetDistance(obj, is3D, DIST_CALC_NONE);
    float sizefactor = GetCombatReach() + obj->GetCombatReach();
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::_IsWithinCombatDist(WorldObject const* obj, float dist2compare, bool is3D) const
{
    float distsq = GetDistance(obj, is3D, DIST_CALC_NONE);
    float sizefactor = GetCombinedCombatReach(obj);
    float maxdist = dist2compare + sizefactor;

    return distsq < maxdist * maxdist;
}

bool WorldObject::IsWithinLOSInMap(const WorldObject* obj, bool ignoreM2Model) const
{
    if (!IsInMap(obj)) return false;
    float ox, oy, oz;
    obj->GetPosition(ox, oy, oz);
    return IsWithinLOS(ox, oy, oz, ignoreM2Model);
}

bool WorldObject::IsWithinLOS(float ox, float oy, float oz, bool ignoreM2Model) const
{
    float x, y, z;
    GetPosition(x, y, z);
    return GetMap()->IsInLineOfSight(x, y, z + 2.0f, ox, oy, oz + 2.0f, ignoreM2Model);
}

bool WorldObject::GetDistanceOrder(WorldObject const* obj1, WorldObject const* obj2, bool is3D /* = true */, DistanceCalculation distcalc /* = NONE */) const
{
    float distsq1 = GetDistance(obj1, is3D, distcalc);
    float distsq2 = GetDistance(obj2, is3D, distcalc);
    return distsq1 < distsq2;
}

bool WorldObject::IsInRange(WorldObject const* obj, float minRange, float maxRange, bool is3D /* = true */, bool combat /*= false*/) const
{
    float distsq = GetDistance(obj, is3D, DIST_CALC_NONE);
    float sizefactor;
    if (combat)
        sizefactor = GetObjectBoundingRadius() + obj->GetObjectBoundingRadius();
    else
        sizefactor = GetCombatReach() + obj->GetCombatReach();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange2d(float x, float y, float minRange, float maxRange, bool combat /*= false*/) const
{
    float distsq = GetDistance2d(x, y, DIST_CALC_NONE);
    float sizefactor;
    if (combat)
        sizefactor = GetObjectBoundingRadius();
    else
        sizefactor = GetCombatReach();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

bool WorldObject::IsInRange3d(float x, float y, float z, float minRange, float maxRange, bool combat /*= false*/) const
{
    float distsq = GetDistance(x, y, z, DIST_CALC_NONE);
    float sizefactor;
    if (combat)
        sizefactor = GetObjectBoundingRadius();
    else
        sizefactor = GetCombatReach();

    // check only for real range
    if (minRange > 0.0f)
    {
        float mindist = minRange + sizefactor;
        if (distsq < mindist * mindist)
            return false;
    }

    float maxdist = maxRange + sizefactor;
    return distsq < maxdist * maxdist;
}

float WorldObject::GetAngle(const WorldObject* obj) const
{
    if (!obj)
        return 0.0f;

    // Rework the assert, when more cases where such a call can happen have been fixed
    // MANGOS_ASSERT(obj != this || PrintEntryError("GetAngle (for self)"));
    if (obj == this)
    {
        sLog.outError("INVALID CALL for GetAngle for %s", obj->GetGuidStr().c_str());
        return 0.0f;
    }
    return GetAngle(obj->GetPositionX(), obj->GetPositionY());
}

// Return angle in range 0..2*pi
float WorldObject::GetAngle(const float x, const float y) const
{
    float dx = x - GetPositionX();
    float dy = y - GetPositionY();

    float ang = atan2(dy, dx);                              // returns value between -Pi..Pi
    ang = (ang >= 0) ? ang : 2 * M_PI_F + ang;
    return ang;
}

bool WorldObject::HasInArc(const WorldObject* target, float arc /*= M_PI*/) const
{
    // always have self in arc
    if (target == this)
        return true;

    // move arc to range 0.. 2*pi
    arc = MapManager::NormalizeOrientation(arc);

    float angle = GetAngle(target);
    angle -= m_position.o;

    // move angle to range -pi ... +pi
    angle = MapManager::NormalizeOrientation(angle);
    if (angle > M_PI_F)
        angle -= 2.0f * M_PI_F;

    float lborder =  -1 * (arc / 2.0f);                     // in range -pi..0
    float rborder = (arc / 2.0f);                           // in range 0..pi
    return ((angle >= lborder) && (angle <= rborder));
}

bool WorldObject::IsFacingTargetsBack(const WorldObject* target, float arc /*= M_PI_F*/) const
{
    if (!target)
        return false;

    //if target is facing the current object then we know its not possible that the current object would be facing the targets back
    if (target->HasInArc(this, arc))
        return false;

    //if current object is not facing the target then we know the current object is not facing the target at all
    if (!this->HasInArc(target, arc))
        return false;

    return true;
}

bool WorldObject::IsFacingTargetsFront(const WorldObject* target, float arc /*= M_PI_F*/) const
{
    if (!target)
        return false;

    //if target is not facing the current object then we know its not possible that the current object would be facing the targets front
    if (!target->HasInArc(this, arc))
        return false;

    //if current object is not facing the target then we know the current object is not facing the target at all
    if (!this->HasInArc(target, arc))
        return false;

    return true;
}

bool WorldObject::isInFrontInMap(WorldObject const* target, float distance,  float arc /*= M_PI_F*/) const
{
    return IsInMap(target) && isInFront(target, distance, arc);
}

bool WorldObject::isInBackInMap(WorldObject const* target, float distance, float arc /*= M_PI_F*/) const
{
    return IsInMap(target) && isInBack(target, distance, arc);
}

bool WorldObject::isInFront(WorldObject const* target, float distance,  float arc /*= M_PI_F*/) const
{
    return target->GetDistance(GetPositionX(), GetPositionY(), GetPositionZ(), DIST_CALC_COMBAT_REACH) <= distance && HasInArc(target, arc);
}

bool WorldObject::isInBack(WorldObject const* target, float distance, float arc /*= M_PI_F*/) const
{
    return target->GetDistance(GetPositionX(), GetPositionY(), GetPositionZ(), DIST_CALC_COMBAT_REACH) <= distance && !HasInArc(target, 2 * M_PI_F - arc);
}

void WorldObject::GetRandomPoint(float x, float y, float z, float distance, float& rand_x, float& rand_y, float& rand_z, float minDist /*=0.0f*/, float const* ori /*=nullptr*/) const
{
    if (distance == 0)
    {
        rand_x = x;
        rand_y = y;
        rand_z = z;
        return;
    }

    // angle to face `obj` to `this`
    float angle;
    if (!ori)
        angle = rand_norm_f() * 2 * M_PI_F;
    else
        angle = *ori;

    float new_dist;
    if (minDist == 0.0f)
        new_dist = rand_norm_f() * distance;
    else
        new_dist = minDist + rand_norm_f() * (distance - minDist);

    rand_x = x + new_dist * cos(angle);
    rand_y = y + new_dist * sin(angle);
    rand_z = z;

    MaNGOS::NormalizeMapCoord(rand_x);
    MaNGOS::NormalizeMapCoord(rand_y);
    UpdateAllowedPositionZ(rand_x, rand_y, rand_z);          // update to LOS height if available
}

void WorldObject::UpdateGroundPositionZ(float x, float y, float& z) const
{
    float new_z = GetMap()->GetHeight(x, y, z);
    if (new_z > INVALID_HEIGHT)
        z = new_z + 0.05f;                                  // just to be sure that we are not a few pixel under the surface
}

void WorldObject::UpdateAllowedPositionZ(float x, float y, float& z, Map* atMap /*=nullptr*/) const
{
    if (!atMap)
        atMap = GetMap();

    float ground_z = atMap->GetHeight(x, y, z);
    if (ground_z > INVALID_HEIGHT)
        z = ground_z;
}

void WorldObject::MovePositionToFirstCollision(WorldLocation &pos, float dist, float angle)
{
    float destX = pos.coord_x + dist * cos(angle);
    float destY = pos.coord_y + dist * sin(angle);
    float destZ = pos.coord_z;

    UpdateAllowedPositionZ(destX, destY, destZ);
    bool colPoint = GetMap()->GetHitPosition(pos.coord_x, pos.coord_y, pos.coord_z + 0.5f, destX, destY, destZ, -0.5f);

    if (colPoint)
    {
        destX -= CONTACT_DISTANCE * cos(angle);
        destY -= CONTACT_DISTANCE * sin(angle);
        dist = sqrt((pos.coord_x - destX)*(pos.coord_x - destX) + (pos.coord_y - destY)*(pos.coord_y - destY));
    }

    float step = dist / 10.0f;

    for (int i = 0; i < 10; i++)
    {
        if (fabs(pos.coord_z - destZ) > ATTACK_DISTANCE)
        {
            destX -= step * cos(angle);
            destY -= step * sin(angle);
            UpdateAllowedPositionZ(destX, destY, destZ);
        }
        else
        {
            pos.coord_x = destX;
            pos.coord_y = destY;
            pos.coord_z = destZ;
            break;
        }
    }

    MaNGOS::NormalizeMapCoord(pos.coord_x);
    MaNGOS::NormalizeMapCoord(pos.coord_y);
    UpdateAllowedPositionZ(pos.coord_x, pos.coord_y, pos.coord_z);
    pos.orientation = m_position.o;
}

float WorldObject::GetCombinedCombatReach(WorldObject const* pVictim, bool forMeleeRange, float flat_mod) const
{
    return GetCombinedCombatReach(forMeleeRange, flat_mod + pVictim->GetCombatReach());
}

float WorldObject::GetCombinedCombatReach(bool forMeleeRange, float flat_mod) const
{
    // The measured values show BASE_MELEE_OFFSET in (1.3224, 1.342)
    float reach = GetCombatReach() + flat_mod;

    if (forMeleeRange)
    {
        reach += BASE_MELEERANGE_OFFSET;
        if (reach < ATTACK_DISTANCE)
            reach = ATTACK_DISTANCE;
    }

    return reach;
}

bool WorldObject::IsPositionValid() const
{
    return MaNGOS::IsValidMapCoord(m_position.x, m_position.y, m_position.z, m_position.o);
}

void WorldObject::MonsterSay(const char* text, uint32 /*language*/, Unit const* target) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_SAY, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY), true);
}

void WorldObject::MonsterYell(const char* text, uint32 /*language*/, Unit const* target) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL), true);
}

void WorldObject::MonsterTextEmote(const char* text, Unit const* target, bool IsBossEmote) const
{
    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossEmote ? CHAT_MSG_RAID_BOSS_EMOTE : CHAT_MSG_MONSTER_EMOTE, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target ? target->GetObjectGuid() : ObjectGuid(), target ? target->GetName() : "");
    SendMessageToSetInRange(data, sWorld.getConfig(IsBossEmote ? CONFIG_FLOAT_LISTEN_RANGE_YELL : CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE), true);
}

void WorldObject::MonsterWhisper(const char* text, Unit const* target, bool IsBossWhisper) const
{
    if (!target || target->GetTypeId() != TYPEID_PLAYER)
        return;

    WorldPacket data;
    ChatHandler::BuildChatPacket(data, IsBossWhisper ? CHAT_MSG_RAID_BOSS_WHISPER : CHAT_MSG_MONSTER_WHISPER, text, LANG_UNIVERSAL, CHAT_TAG_NONE, GetObjectGuid(), GetName(),
                                 target->GetObjectGuid(), target->GetName());
    ((Player*)target)->GetSession()->SendPacket(data);
}

namespace MaNGOS
{
    class MonsterChatBuilder
    {
        public:
            MonsterChatBuilder(WorldObject const& obj, ChatMsg msgtype, MangosStringLocale const* textData, Language language, Unit const* target)
                : i_object(obj), i_msgtype(msgtype), i_textData(textData), i_language(language), i_target(target) {}
            void operator()(WorldPacket& data, int32 loc_idx)
            {
                char const* text;
                if ((int32)i_textData->Content.size() > loc_idx + 1 && !i_textData->Content[loc_idx + 1].empty())
                    text = i_textData->Content[loc_idx + 1].c_str();
                else
                    text = i_textData->Content[0].c_str();

                ChatHandler::BuildChatPacket(data, i_msgtype, text, i_language, CHAT_TAG_NONE, i_object.GetObjectGuid(), i_object.GetNameForLocaleIdx(loc_idx),
                                             i_target ? i_target->GetObjectGuid() : ObjectGuid(), i_target ? i_target->GetNameForLocaleIdx(loc_idx) : "");
            }

        private:
            WorldObject const& i_object;
            ChatMsg i_msgtype;
            MangosStringLocale const* i_textData;
            Language i_language;
            Unit const* i_target;
    };
}                                                           // namespace MaNGOS

/// Helper function to create localized around a source
void _DoLocalizedTextAround(WorldObject const* source, MangosStringLocale const* textData, ChatMsg msgtype, Language language, Unit const* target, float range)
{
    MaNGOS::MonsterChatBuilder say_build(*source, msgtype, textData, language, target);
    MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
    MaNGOS::CameraDistWorker<MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> > say_worker(source, range, say_do);
    Cell::VisitWorldObjects(source, say_worker, range);
}

/// Function that sends a text associated to a MangosString
void WorldObject::MonsterText(MangosStringLocale const* textData, Unit const* target) const
{
    MANGOS_ASSERT(textData);

    switch (textData->Type)
    {
        case CHAT_TYPE_SAY:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_SAY, textData->LanguageId, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_SAY));
            break;
        case CHAT_TYPE_YELL:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_YELL, textData->LanguageId, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL));
            break;
        case CHAT_TYPE_TEXT_EMOTE:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_MONSTER_EMOTE, LANG_UNIVERSAL, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_TEXTEMOTE));
            break;
        case CHAT_TYPE_BOSS_EMOTE:
            _DoLocalizedTextAround(this, textData, CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, target, sWorld.getConfig(CONFIG_FLOAT_LISTEN_RANGE_YELL));
            break;
        case CHAT_TYPE_WHISPER:
        {
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_WHISPER, textData, LANG_UNIVERSAL, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            say_do((Player*)target);
            break;
        }
        case CHAT_TYPE_BOSS_WHISPER:
        {
            if (!target || target->GetTypeId() != TYPEID_PLAYER)
                return;
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_RAID_BOSS_WHISPER, textData, LANG_UNIVERSAL, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            say_do((Player*)target);
            break;
        }
        case CHAT_TYPE_ZONE_YELL:
        {
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_YELL, textData, textData->LanguageId, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            uint32 zoneId = GetZoneId();
            GetMap()->ExecuteMapWorkerZone(zoneId, std::bind(&MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder>::operator(), &say_do, std::placeholders::_1));
            break;
        }
        case CHAT_TYPE_ZONE_EMOTE:
        {
            MaNGOS::MonsterChatBuilder say_build(*this, CHAT_MSG_MONSTER_EMOTE, textData, textData->LanguageId, target);
            MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder> say_do(say_build);
            uint32 zoneId = GetZoneId();
            GetMap()->ExecuteMapWorkerZone(zoneId, std::bind(&MaNGOS::LocalizedPacketDo<MaNGOS::MonsterChatBuilder>::operator(), &say_do, std::placeholders::_1));
            break;
        }
    }
}

void WorldObject::SendMessageToSet(WorldPacket const& data, bool /*bToSelf*/) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
        GetMap()->MessageBroadcast(this, data);
}

void WorldObject::SendMessageToSetInRange(WorldPacket const& data, float dist, bool /*bToSelf*/) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
        GetMap()->MessageDistBroadcast(this, data, dist);
}

void WorldObject::SendMessageToSetExcept(WorldPacket& data, Player const* skipped_receiver) const
{
    // if object is in world, map for it already created!
    if (IsInWorld())
    {
        MaNGOS::MessageDelivererExcept notifier(data, skipped_receiver);
        Cell::VisitWorldObjects(this, notifier, GetMap()->GetVisibilityDistance());
    }
}

void WorldObject::SendObjectDeSpawnAnim(ObjectGuid guid) const
{
    WorldPacket data(SMSG_GAMEOBJECT_DESPAWN_ANIM, 8);
    data << ObjectGuid(guid);
    SendMessageToSet(data, true);
}

void WorldObject::SendGameObjectCustomAnim(ObjectGuid guid, uint32 animId /*= 0*/) const
{
    WorldPacket data(SMSG_GAMEOBJECT_CUSTOM_ANIM, 8 + 4);
    data << ObjectGuid(guid);
    data << uint32(animId);
    SendMessageToSet(data, true);
}

void WorldObject::SetMap(Map* map)
{
    MANGOS_ASSERT(map);
    m_currMap = map;
    // lets save current map's Id/instanceId
    m_mapId = map->GetId();
    m_InstanceId = map->GetInstanceId();
}

void WorldObject::AddToWorld()
{
    if (m_isOnEventNotified)
        m_currMap->AddToOnEventNotified(this);

    Object::AddToWorld();
}

void WorldObject::RemoveFromWorld()
{
    if (m_isOnEventNotified)
        m_currMap->RemoveFromOnEventNotified(this);

    Object::RemoveFromWorld();
}

TerrainInfo const* WorldObject::GetTerrain() const
{
    MANGOS_ASSERT(m_currMap);
    return m_currMap->GetTerrain();
}

void WorldObject::AddObjectToRemoveList()
{
    GetMap()->AddObjectToRemoveList(this);
}

Creature* WorldObject::SummonCreature(uint32 id, float x, float y, float z, float ang, TempSpawnType spwtype, uint32 despwtime, bool asActiveObject, bool setRun, uint32 pathId, uint32 faction, bool spawnCounting, bool forcedOnTop)
{
    CreatureInfo const* cinfo = ObjectMgr::GetCreatureTemplate(id);
    if (!cinfo)
    {
        sLog.outErrorDb("WorldObject::SummonCreature: Creature (Entry: %u) not existed for summoner: %s. ", id, GetGuidStr().c_str());
        return nullptr;
    }

    TemporarySpawn* pCreature = new TemporarySpawn(GetObjectGuid());

    CreatureCreatePos pos(GetMap(), x, y, z, ang);

    if (x == 0.0f && y == 0.0f && z == 0.0f)
    {
        float dist = forcedOnTop ? 0.0f : CONTACT_DISTANCE;
        pos = CreatureCreatePos(this, GetOrientation(), dist, ang);
    }

    if (!pCreature->Create(GetMap()->GenerateLocalLowGuid(cinfo->GetHighGuid()), pos, cinfo))
    {
        delete pCreature;
        return nullptr;
    }

    pCreature->SetRespawnCoord(pos);

    // Set run or walk before any other movement starts
    pCreature->SetWalk(!setRun);

    // Active state set before added to map
    pCreature->SetActiveObjectState(asActiveObject);

    if (faction)
        pCreature->setFaction(faction);

    if (spawnCounting)
        pCreature->SetSpawnCounting(true);

    pCreature->GetMotionMaster()->SetDefaultPathId(pathId);

    pCreature->Summon(spwtype, despwtime);                  // Also initializes the AI and MMGen

    if (GetTypeId() == TYPEID_UNIT && ((Creature*)this)->AI())
        ((Creature*)this)->AI()->JustSummoned(pCreature);

    // Creature Linking, Initial load is handled like respawn
    if (pCreature->IsLinkingEventTrigger())
        GetMap()->GetCreatureLinkingHolder()->DoCreatureLinkingEvent(LINKING_EVENT_RESPAWN, pCreature);

    // return the creature therewith the summoner has access to it
    return pCreature;
}

// how much space should be left in front of/ behind a mob that already uses a space
#define OCCUPY_POS_DEPTH_FACTOR                          1.8f

namespace MaNGOS
{
    class NearUsedPosDo
    {
        public:
            NearUsedPosDo(WorldObject const& obj, WorldObject const* searcher, float absAngle, ObjectPosSelector& selector)
                : i_object(obj), i_searcher(searcher), i_absAngle(MapManager::NormalizeOrientation(absAngle)), i_selector(selector) {}

            void operator()(Corpse*) const {}
            void operator()(DynamicObject*) const {}

            void operator()(Creature* c) const
            {
                // skip self or target
                if (c == i_searcher || c == &i_object)
                    return;

                float x, y, z;

                if (c->IsStopped() || !c->GetMotionMaster()->GetDestination(x, y, z))
                {
                    x = c->GetPositionX();
                    y = c->GetPositionY();
                }

                add(c, x, y);
            }

            template<class T>
            void operator()(T* u) const
            {
                // skip self or target
                if (u == i_searcher || u == &i_object)
                    return;

                float x = u->GetPositionX();
                float y = u->GetPositionY();

                add(u, x, y);
            }

            // we must add used pos that can fill places around center
            void add(WorldObject* u, float x, float y) const
            {
                float dx = i_object.GetPositionX() - x;
                float dy = i_object.GetPositionY() - y;
                float dist2d = sqrt((dx * dx) + (dy * dy));

                // It is ok for the objects to require a bit more space
                float delta = u->GetObjectBoundingRadius();
                if (i_selector.m_searchPosFor && i_selector.m_searchPosFor != u)
                    delta += i_selector.m_searchPosFor->GetObjectBoundingRadius();

                delta *= OCCUPY_POS_DEPTH_FACTOR;           // Increase by factor

                // u is too near/far away from i_object. Do not consider it to occupy space
                if (fabs(i_selector.m_searcherDist - dist2d) > delta)
                    return;

                float angle = i_object.GetAngle(u) - i_absAngle;

                // move angle to range -pi ... +pi, range before is -2Pi..2Pi
                if (angle > M_PI_F)
                    angle -= 2.0f * M_PI_F;
                else if (angle < -M_PI_F)
                    angle += 2.0f * M_PI_F;

                i_selector.AddUsedArea(u, angle, dist2d);
            }
        private:
            WorldObject const& i_object;
            WorldObject const* i_searcher;
            float              i_absAngle;
            ObjectPosSelector& i_selector;
    };
}                                                           // namespace MaNGOS

//===================================================================================================

void WorldObject::GetNearPoint2D(float& x, float& y, float distance2d, float absAngle) const
{
    x = GetPositionX() + distance2d * cos(absAngle);
    y = GetPositionY() + distance2d * sin(absAngle);

    MaNGOS::NormalizeMapCoord(x);
    MaNGOS::NormalizeMapCoord(y);
}

void WorldObject::GetNearPoint(WorldObject const* searcher, float& x, float& y, float& z, float searcher_bounding_radius, float distance2d, float absAngle, bool isInWater) const
{
    GetNearPoint2D(x, y, distance2d, absAngle);
    const float init_z = z = GetPositionZ();

    // if detection disabled, return first point
    if (!sWorld.getConfig(CONFIG_BOOL_DETECT_POS_COLLISION))
    {
        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else if (!isInWater)
            UpdateGroundPositionZ(x, y, z);
        return;
    }

    // or remember first point
    float first_x = x;
    float first_y = y;
    bool first_los_conflict = false;                        // first point LOS problems

    const float dist = distance2d + searcher_bounding_radius + GetObjectBoundingRadius();

    // prepare selector for work
    ObjectPosSelector selector(GetPositionX(), GetPositionY(), distance2d, searcher_bounding_radius, searcher);

    // adding used positions around object - unused because its not blizzlike
    //{
    //    MaNGOS::NearUsedPosDo u_do(*this, searcher, absAngle, selector);
    //    MaNGOS::WorldObjectWorker<MaNGOS::NearUsedPosDo> worker(u_do);

    //    Cell::VisitAllObjects(this, worker, dist);
    //}

    // maybe can just place in primary position
    if (selector.CheckOriginalAngle())
    {
        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else if (!isInWater)
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;

        first_los_conflict = true;                          // first point have LOS problems
    }

    // set first used pos in lists
    selector.InitializeAngle();

    float angle;                                            // candidate of angle for free pos

    // select in positions after current nodes (selection one by one)
    while (selector.NextAngle(angle))                       // angle for free pos
    {
        GetNearPoint2D(x, y, distance2d, absAngle + angle);
        z = GetPositionZ();

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else if (!isInWater)
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;
    }

    // BAD NEWS: not free pos (or used or have LOS problems)
    // Attempt find _used_ pos without LOS problem
    if (!first_los_conflict)
    {
        x = first_x;
        y = first_y;

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else if (!isInWater)
            UpdateGroundPositionZ(x, y, z);
        return;
    }

    // set first used pos in lists
    selector.InitializeAngle();

    // select in positions after current nodes (selection one by one)
    while (selector.NextUsedAngle(angle))                   // angle for used pos but maybe without LOS problem
    {
        GetNearPoint2D(x, y, distance2d, absAngle + angle);
        z = GetPositionZ();

        if (searcher)
            searcher->UpdateAllowedPositionZ(x, y, z, GetMap()); // update to LOS height if available
        else if (!isInWater)
            UpdateGroundPositionZ(x, y, z);

        if (fabs(init_z - z) < dist && IsWithinLOS(x, y, z))
            return;
    }

    // BAD BAD NEWS: all found pos (free and used) have LOS problem :(
    x = first_x;
    y = first_y;

    if (searcher)
        searcher->UpdateAllowedPositionZ(x, y, z, GetMap());// update to LOS height if available
    else if (!isInWater)
        UpdateGroundPositionZ(x, y, z);
}

void WorldObject::PlayDistanceSound(uint32 sound_id, PlayPacketParameters parameters /*= PlayPacketParameters(PLAY_SET)*/) const
{
    WorldPacket data(SMSG_PLAY_OBJECT_SOUND, 4 + 8);
    data << uint32(sound_id);
    data << GetObjectGuid();
    HandlePlayPacketSettings(data, parameters);
}

void WorldObject::PlayDirectSound(uint32 sound_id, PlayPacketParameters parameters /*= PlayPacketParameters(PLAY_SET)*/) const
{
    WorldPacket data(SMSG_PLAY_SOUND, 4);
    data << uint32(sound_id);
    HandlePlayPacketSettings(data, parameters);
}

void WorldObject::PlayMusic(uint32 sound_id, PlayPacketParameters parameters /*= PlayPacketParameters(PLAY_SET)*/) const
{
    WorldPacket data(SMSG_PLAY_MUSIC, 4);
    data << uint32(sound_id);
    HandlePlayPacketSettings(data, parameters);
}

void WorldObject::PlaySpellVisual(uint32 artKitId, PlayPacketParameters parameters /*= PlayPacketParameters(PLAY_SET)*/) const
{
    WorldPacket data(SMSG_PLAY_SPELL_VISUAL, 4);
    data << GetObjectGuid();
    data << artKitId; // index from SpellVisualKit.dbc
    HandlePlayPacketSettings(data, parameters);
}

void WorldObject::HandlePlayPacketSettings(WorldPacket& msg, PlayPacketParameters& parameters) const
{
    switch (parameters.setting)
    {
        case PLAY_SET:
            SendMessageToSet(msg, true);
            break;
        case PLAY_TARGET:
            if (Player const* target = parameters.target.target)
                target->SendDirectMessage(msg);
            break;
        case PLAY_MAP:
            if (IsInWorld())
                GetMap()->MessageMapBroadcast(this, msg);
            break;
        case PLAY_ZONE:
            if (IsInWorld())
                GetMap()->MessageMapBroadcastZone(this, msg, parameters.areaOrZone.id);
            break;
        case PLAY_AREA:
            if (IsInWorld())
                GetMap()->MessageMapBroadcastArea(this, msg, parameters.areaOrZone.id);
            break;
    }
}

void WorldObject::UpdateVisibilityAndView()
{
    GetViewPoint().Call_UpdateVisibilityForOwner();
    UpdateObjectVisibility();
    GetViewPoint().Event_ViewPointVisibilityChanged();
}

void WorldObject::UpdateObjectVisibility()
{
    CellPair p = MaNGOS::ComputeCellPair(GetPositionX(), GetPositionY());
    Cell cell(p);

    GetMap()->UpdateObjectVisibility(this, cell, p);
}

void WorldObject::AddToClientUpdateList()
{
    GetMap()->AddUpdateObject(this);
}

void WorldObject::RemoveFromClientUpdateList()
{
    GetMap()->RemoveUpdateObject(this);
}

struct WorldObjectChangeAccumulator
{
    UpdateDataMapType& i_updateDatas;
    WorldObject& i_object;
    WorldObjectChangeAccumulator(WorldObject& obj, UpdateDataMapType& d) : i_updateDatas(d), i_object(obj)
    {
        // send self fields changes in another way, otherwise
        // with new camera system when player's camera too far from player, camera wouldn't receive packets and changes from player
        if (i_object.isType(TYPEMASK_PLAYER))
            i_object.BuildUpdateDataForPlayer((Player*)&i_object, i_updateDatas);
    }

    void Visit(CameraMapType& m)
    {
        for (auto& iter : m)
        {
            Player* owner = iter.getSource()->GetOwner();
            if (owner != &i_object && owner->HaveAtClient(&i_object))
                i_object.BuildUpdateDataForPlayer(owner, i_updateDatas);
        }
    }

    template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
};

void WorldObject::BuildUpdateData(UpdateDataMapType& update_players)
{
    WorldObjectChangeAccumulator notifier(*this, update_players);
    Cell::VisitWorldObjects(this, notifier, GetMap()->GetVisibilityDistance());

    ClearUpdateMask(false);
}

bool WorldObject::IsControlledByPlayer() const
{
    switch (GetTypeId())
    {
        case TYPEID_GAMEOBJECT:
            return ((GameObject*)this)->GetOwnerGuid().IsPlayer();
        case TYPEID_UNIT:
        case TYPEID_PLAYER:
            return ((Unit*)this)->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);
        case TYPEID_DYNAMICOBJECT:
            return ((DynamicObject*)this)->GetCasterGuid().IsPlayer();
        case TYPEID_CORPSE:
            return true;
        default:
            return false;
    }
}

bool WorldObject::PrintCoordinatesError(float x, float y, float z, char const* descr) const
{
    sLog.outError("%s with invalid %s coordinates: mapid = %uu, x = %f, y = %f, z = %f", GetGuidStr().c_str(), descr, GetMapId(), x, y, z);
    return false;                                           // always false for continue assert fail
}

void WorldObject::SetActiveObjectState(bool active)
{
    if (m_isActiveObject == active || (isType(TYPEMASK_PLAYER) && !active))  // player shouldn't became inactive, never
        return;

    if (IsInWorld() && !isType(TYPEMASK_PLAYER))
        // player's update implemented in a different from other active worldobject's way
        // it's considired to use generic way in future
    {
        if (isActiveObject() && !active)
            GetMap()->RemoveFromActive(this);
        else if (!isActiveObject() && active)
            GetMap()->AddToActive(this);
    }
    m_isActiveObject = active;
}

void WorldObject::SetVisibilityDistanceOverride(VisibilityDistanceType type)
{
    MANGOS_ASSERT(type < VisibilityDistanceType::Max);
    if (GetTypeId() == TYPEID_PLAYER)
        return;

    m_visibilityDistanceOverride = VisibilityDistances[AsUnderlyingType(type)];
}

float WorldObject::GetVisibilityDistance() const
{
    if (IsVisibilityOverridden())
        return m_visibilityDistanceOverride;
    else
        return GetMap()->GetVisibilityDistance();
}

float WorldObject::GetVisibilityDistanceFor(WorldObject* obj) const
{
    if (IsVisibilityOverridden() && obj->GetTypeId() == TYPEID_PLAYER)
        return m_visibilityDistanceOverride;
    else
        return obj->GetVisibilityDistance();
}

void WorldObject::SetNotifyOnEventState(bool state)
{
    if (state == m_isOnEventNotified)
        return;

    m_isOnEventNotified = state;

    if (!IsInWorld())
        return;

    if (state)
        GetMap()->AddToOnEventNotified(this);
    else
        GetMap()->RemoveFromOnEventNotified(this);
}

void WorldObject::AddGCD(SpellEntry const& spellEntry, uint32 forcedDuration /*= 0*/, bool /*updateClient = false*/)
{
    uint32 gcdRecTime = forcedDuration ? forcedDuration : spellEntry.StartRecoveryTime;
    if (!gcdRecTime)
        return;

    m_GCDCatMap.emplace(spellEntry.StartRecoveryCategory, std::chrono::milliseconds(gcdRecTime) + GetMap()->GetCurrentClockTime());
}

bool WorldObject::HasGCD(SpellEntry const* spellEntry) const
{
    if (spellEntry)
    {
        auto gcdItr = m_GCDCatMap.find(spellEntry->StartRecoveryCategory);
        return gcdItr != m_GCDCatMap.end();
    }

    return !m_GCDCatMap.empty();
}

void WorldObject::AddCooldown(SpellEntry const& spellEntry, ItemPrototype const* /*itemProto = nullptr*/, bool /*permanent = false*/, uint32 forcedDuration /*= 0*/)
{
    uint32 recTimeDuration = forcedDuration ? forcedDuration : spellEntry.RecoveryTime;
    if (recTimeDuration || spellEntry.CategoryRecoveryTime)
        m_cooldownMap.AddCooldown(GetMap()->GetCurrentClockTime(), spellEntry.Id, recTimeDuration, spellEntry.Category, spellEntry.CategoryRecoveryTime);
}

void WorldObject::UpdateCooldowns(TimePoint const& now)
{
    // handle GCD
    auto cdItr = m_GCDCatMap.begin();
    while (cdItr != m_GCDCatMap.end())
    {
        auto& cd = cdItr->second;
        if (cd <= now)
            cdItr = m_GCDCatMap.erase(cdItr);
        else
            ++cdItr;
    }

    // handle spell and category cooldowns
    m_cooldownMap.Update(now);

    // handle spell lockouts
    auto lockoutCDItr = m_lockoutMap.begin();
    while (lockoutCDItr != m_lockoutMap.end())
    {
        if (lockoutCDItr->second <= now)
            lockoutCDItr = m_lockoutMap.erase(lockoutCDItr);
        else
            ++lockoutCDItr;
    }
}

bool WorldObject::CheckLockout(SpellSchoolMask schoolMask) const
{
    for (auto& lockoutItr : m_lockoutMap)
    {
        SpellSchoolMask lockoutSchoolMask = SpellSchoolMask(1 << lockoutItr.first);
        if (lockoutSchoolMask & schoolMask)
            return true;
    }

    return false;
}

bool WorldObject::GetExpireTime(SpellEntry const& spellEntry, TimePoint& expireTime, bool& isPermanent) const
{
    auto spellItr = m_cooldownMap.FindBySpellId(spellEntry.Id);
    if (spellItr != m_cooldownMap.end())
    {
        auto& cdData = spellItr->second;
        if (cdData->IsPermanent())
        {
            isPermanent = true;
            return true;
        }

        TimePoint spellExpireTime = TimePoint();
        TimePoint catExpireTime = TimePoint();
        bool foundSpellCD = cdData->GetSpellCDExpireTime(spellExpireTime);
        bool foundCatCD = cdData->GetSpellCDExpireTime(catExpireTime);
        if (foundCatCD || foundSpellCD)
        {
            expireTime = spellExpireTime > catExpireTime ? spellExpireTime : catExpireTime;
            return true;
        }
    }
    return false;
}

bool WorldObject::IsSpellReady(SpellEntry const& spellEntry, ItemPrototype const* itemProto /*= nullptr*/) const
{
    uint32 spellCategory = spellEntry.Category;

    // overwrite category by provided category in item prototype during item cast if need
    if (itemProto)
    {
        for (const auto& Spell : itemProto->Spells)
        {
            if (Spell.SpellId == spellEntry.Id)
            {
                spellCategory = Spell.SpellCategory;
                break;
            }
        }
    }

    if (m_cooldownMap.FindBySpellId(spellEntry.Id) != m_cooldownMap.end())
        return false;

    if (spellCategory && m_cooldownMap.FindByCategory(spellCategory) != m_cooldownMap.end())
        return false;

    if (spellEntry.PreventionType == SPELL_PREVENTION_TYPE_SILENCE && CheckLockout(GetSpellSchoolMask(&spellEntry)))
        return false;

    return true;
}

bool WorldObject::IsSpellReady(uint32 spellId, ItemPrototype const* itemProto /*= nullptr*/) const
{
    SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellEntry)
        return false;

    return IsSpellReady(*spellEntry, itemProto);
}

void WorldObject::LockOutSpells(SpellSchoolMask schoolMask, uint32 duration)
{
    for (uint32 i = 0; i < MAX_SPELL_SCHOOL; ++i)
    {
        if (schoolMask & (1 << i))
            m_lockoutMap.emplace(SpellSchools(i), std::chrono::milliseconds(duration) + GetMap()->GetCurrentClockTime());
    }
}

void WorldObject::RemoveSpellCooldown(uint32 spellId, bool updateClient /*= true*/)
{
    SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (!spellEntry)
        return;

    RemoveSpellCooldown(*spellEntry, updateClient);
}

void WorldObject::RemoveSpellCooldown(SpellEntry const& spellEntry, bool /*updateClient = true*/)
{
    m_cooldownMap.RemoveBySpellId(spellEntry.Id);
}

void WorldObject::RemoveSpellCategoryCooldown(uint32 category, bool /*updateClient = true*/)
{
    m_cooldownMap.RemoveByCategory(category);
}

void WorldObject::ResetGCD(SpellEntry const* spellEntry /*= nullptr*/)
{
    if (!spellEntry)
    {
        m_GCDCatMap.clear();
        return;
    }

    auto gcdItr = m_GCDCatMap.find(spellEntry->StartRecoveryCategory);
    if (gcdItr != m_GCDCatMap.end())
        m_GCDCatMap.erase(gcdItr);
}

void ConvertMillisecondToStr(std::chrono::milliseconds& duration, std::stringstream& durationStr)
{
    std::chrono::minutes mm = std::chrono::duration_cast<std::chrono::minutes>(duration % std::chrono::hours(1));
    std::chrono::seconds ss = std::chrono::duration_cast<std::chrono::seconds>(duration % std::chrono::minutes(1));
    std::chrono::milliseconds msec = std::chrono::duration_cast<std::chrono::milliseconds>(duration % std::chrono::seconds(1));
    durationStr << mm.count() << "m " << ss.count() << "s " << msec.count() << "ms";
}

void WorldObject::PrintCooldownList(ChatHandler& chat) const
{
    // print gcd
    auto now = GetMap()->GetCurrentClockTime();
    uint32 cdCount = 0;
    uint32 permCDCount = 0;

    for (auto& cdItr : m_GCDCatMap)
    {
        auto& cdData = cdItr.second;
        std::stringstream cdLine;
        std::stringstream durationStr;
        if (cdData > now)
        {
            auto cdDuration = cdData - now;
            ConvertMillisecondToStr(cdDuration, durationStr);
            ++cdCount;
        }
        else
            continue;

        cdLine << "GCD category" << "(" << cdItr.first << ") have " << durationStr.str() << " cd";
        chat.PSendSysMessage("%s", cdLine.str().c_str());
    }

    // print spell and category cd
    for (auto& cdItr : m_cooldownMap)
    {
        auto& cdData = cdItr.second;
        std::stringstream cdLine;
        std::stringstream durationStr("permanent");
        std::stringstream spellStr;
        std::stringstream catStr;
        if (cdData->IsPermanent())
            ++permCDCount;
        else
        {
            TimePoint spellExpireTime;
            TimePoint catExpireTime;
            bool foundSpellCD = cdData->GetSpellCDExpireTime(spellExpireTime);
            bool foundcatCD = cdData->GetCatCDExpireTime(catExpireTime);

            if (foundSpellCD && spellExpireTime > now)
            {
                auto cdDuration = std::chrono::duration_cast<std::chrono::milliseconds>(spellExpireTime - now);
                spellStr << "RecTime(";
                ConvertMillisecondToStr(cdDuration, spellStr);
                spellStr << ")";
            }

            if (foundcatCD && catExpireTime > now)
            {
                auto cdDuration = std::chrono::duration_cast<std::chrono::milliseconds>(catExpireTime - now);
                if (foundSpellCD)
                    catStr << ", ";
                catStr << "CatRecTime(";
                ConvertMillisecondToStr(cdDuration, catStr);
                catStr << ")";
            }

            if (!foundSpellCD && !foundcatCD)
                continue;

            durationStr << spellStr.str() << catStr.str();
            ++cdCount;
        }

        cdLine << "Spell" << "(" << cdItr.first << ") have " << durationStr.str() << " cd";
        chat.PSendSysMessage("%s", cdLine.str().c_str());
    }

    // print spell lockout
    static std::string schoolName[] = { "SPELL_SCHOOL_NORMAL", "SPELL_SCHOOL_HOLY", "SPELL_SCHOOL_FIRE", "SPELL_SCHOOL_NATURE", "SPELL_SCHOOL_FROST", "SPELL_SCHOOL_SHADOW", "SPELL_SCHOOL_ARCANE" };

    for (auto& lockoutItr : m_lockoutMap)
    {
        std::stringstream cdLine;
        std::stringstream durationStr;
        auto& cdData = lockoutItr.second;
        if (cdData > now)
        {
            auto cdDuration = std::chrono::duration_cast<std::chrono::milliseconds>(cdData - now);
            ConvertMillisecondToStr(cdDuration, durationStr);
            ++cdCount;
        }
        else
            continue;
        cdLine << "LOCKOUT for " << schoolName[lockoutItr.first] << " with " << durationStr.str() << " remaining time cd";
        chat.PSendSysMessage("%s", cdLine.str().c_str());
    }

    chat.PSendSysMessage("Found %u cooldown%s.", cdCount, (cdCount > 1) ? "s" : "");
    chat.PSendSysMessage("Found %u permanent cooldown%s.", permCDCount, (permCDCount > 1) ? "s" : "");
}
