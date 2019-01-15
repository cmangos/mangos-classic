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

#include "Common.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "Globals/ObjectMgr.h"
#include "Spells/SpellMgr.h"
#include "Log.h"
#include "Server/Opcodes.h"
#include "Spells/Spell.h"
#include "AI/BaseAI/CreatureAI.h"
#include "Util.h"
#include "Entities/Pet.h"

void WorldSession::HandlePetAction(WorldPacket& recv_data)
{
    ObjectGuid petGuid;
    uint32 data;
    ObjectGuid targetGuid;
    recv_data >> petGuid;
    recv_data >> data;
    recv_data >> targetGuid;

    uint32 spellid = UNIT_ACTION_BUTTON_ACTION(data);
    uint8 flag = UNIT_ACTION_BUTTON_TYPE(data);             // delete = 0x07 CastSpell = C1

    DETAIL_LOG("HandlePetAction: %s flag is %u, spellid is %u, target %s.", petGuid.GetString().c_str(), uint32(flag), spellid, targetGuid.GetString().c_str());

    // used also for charmed creature/player
    Unit* petUnit = _player->GetMap()->GetUnit(petGuid);
    if (!petUnit)
    {
        sLog.outError("HandlePetAction: %s not exist.", petGuid.GetString().c_str());
        return;
    }

    if (_player->GetObjectGuid() != petUnit->GetMasterGuid())
    {
        sLog.outError("HandlePetAction: %s isn't controlled by %s.", petGuid.GetString().c_str(), _player->GetGuidStr().c_str());
        return;
    }

    if (!petUnit->isAlive())
        return;

    CharmInfo* charmInfo = petUnit->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outError("WorldSession::HandlePetAction: object (GUID: %u TypeId: %u) is considered pet-like but doesn't have a charminfo!", petUnit->GetGUIDLow(), petUnit->GetTypeId());
        return;
    }

    Pet* pet = nullptr;
    Creature* creature = nullptr;

    if (petUnit->GetTypeId() == TYPEID_UNIT)
    {
        creature = static_cast<Creature*>(petUnit);

        if (creature->IsPet())
        {
            pet = static_cast<Pet*>(petUnit);

            if (pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS)
                return;
        }
    }

    if (!pet)
    {
        if (petUnit->hasUnitState(UNIT_STAT_POSSESSED))
        {
            // possess case
            if (flag != uint8(ACT_COMMAND))
            {
                sLog.outError("PetHAndler: unknown PET flag Action %i and spellid %i. For possessed %s", uint32(flag), spellid, petUnit->GetGuidStr().c_str());
                return;
            }

            switch (spellid)
            {
                case COMMAND_STAY:
                case COMMAND_FOLLOW:
                    charmInfo->SetCommandState(CommandStates(spellid));
                    break;
                case COMMAND_ATTACK:
                {
                    Unit* targetUnit = targetGuid ? _player->GetMap()->GetUnit(targetGuid) : nullptr;

                    if (targetUnit && targetUnit != petUnit && petUnit->CanAttack(targetUnit))
                    {
                        // This is true if pet has no target or has target but targets differs.
                        if (petUnit->getVictim() != targetUnit)
                            petUnit->Attack(targetUnit, true);
                    }
                    break;
                }
                case COMMAND_DISMISS:
                    _player->BreakCharmOutgoing(petUnit);
                    break;
                default:
                    sLog.outError("PetHandler: Not allowed action %i and spellid %i. Pet %s owner is %s", uint32(flag), spellid, petUnit->GetGuidStr().c_str(), _player->GetGuidStr().c_str());
                    break;
            }
        }

        if (!petUnit->HasCharmer())
            return;
    }

    switch (flag)
    {
        case ACT_COMMAND:                                   // 0x07
            switch (spellid)
            {
                case COMMAND_STAY:                          // flat=1792  // STAY
                {
                    if (!petUnit->hasUnitState(UNIT_STAT_POSSESSED))
                    {
                        petUnit->StopMoving();
                        petUnit->GetMotionMaster()->Clear();
                    }
                    petUnit->AttackStop(true, true);
                    charmInfo->SetCommandState(COMMAND_STAY);
                    break;
                }
                case COMMAND_FOLLOW:                        // spellid=1792  // FOLLOW
                {
                    if (!petUnit->hasUnitState(UNIT_STAT_POSSESSED))
                    {
                        petUnit->StopMoving();
                        petUnit->GetMotionMaster()->Clear();
                        charmInfo->SetIsRetreating(true);
                    }
                    petUnit->AttackStop(true, true);
                    charmInfo->SetCommandState(COMMAND_FOLLOW);
                    break;
                }
                case COMMAND_ATTACK:                        // spellid=1792  // ATTACK
                {
                    charmInfo->SetIsRetreating();
                    charmInfo->SetSpellOpener();

                    Unit* targetUnit = targetGuid ? _player->GetMap()->GetUnit(targetGuid) : nullptr;

                    if (targetUnit && targetUnit != petUnit && petUnit->CanAttack(targetUnit) && targetUnit->isInAccessablePlaceFor((Creature*)petUnit))
                    {
                        // This is true if pet has no target or has target but targets differs.
                        if (petUnit->getVictim() != targetUnit)
                        {
                            petUnit->AttackStop();
                            if (!petUnit->hasUnitState(UNIT_STAT_POSSESSED))
                            {
                                petUnit->GetMotionMaster()->Clear();

                                petUnit->AI()->AttackStart(targetUnit);

                                if (pet)
                                {
                                    // 10% chance to play special warlock pet attack talk, else growl
                                    if (pet->getPetType() == SUMMON_PET && roll_chance_i(10))
                                        pet->SendPetTalk((uint32)PET_TALK_ATTACK);

                                    pet->SendPetAIReaction();
                                }
                            }
                            else
                                petUnit->Attack(targetUnit, true);
                        }
                    }
                    break;
                }
                case COMMAND_DISMISS:                       // dismiss permanent pet, remove temporary pet, uncharm unit
                {
                    if (pet)
                    {
                        pet->PlayDismissSound();
                        // No action for Hunter pets, Hunters must use their Dismiss Pet spell
                        if (pet->getPetType() != HUNTER_PET)
                            pet->ForcedDespawn();
                    }
                    else
                    {
                        // dismissing a summoned pet is like killing them (this prevents returning a soulshard...)
                        if (creature && creature->IsTemporarySummon())
                            creature->ForcedDespawn();
                        else
                            _player->BreakCharmOutgoing(petUnit);
                    }

                    charmInfo->SetStayPosition();
                    break;
                }
                default:
                    sLog.outError("WORLD: unknown PET flag Action %i and spellid %i.", uint32(flag), spellid);
            }
            break;
        case ACT_REACTION:                                  // 0x6
            switch (spellid)
            {
                case REACT_PASSIVE:                         // passive
                {
                    petUnit->AttackStop(true, true);
                    charmInfo->SetSpellOpener();
                }
                case REACT_DEFENSIVE:                       // recovery
                case REACT_AGGRESSIVE:                      // activete
                {
                    petUnit->AI()->SetReactState(ReactStates(spellid));
                    break;
                }
            }
            break;
        case ACT_DISABLED:                                  // 0x81    spell (disabled), ignore
        case ACT_PASSIVE:                                   // 0x01
        case ACT_ENABLED:                                   // 0xC1    spell
        {
            charmInfo->SetIsRetreating();
            charmInfo->SetSpellOpener();

            Unit* unit_target = targetGuid ? _player->GetMap()->GetUnit(targetGuid) : nullptr;

            // do not cast unknown spells
            SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
            if (!spellInfo)
            {
                sLog.outError("WORLD: unknown PET spell id %i", spellid);
                return;
            }

            // do not cast not learned spells
            if (IsPassiveSpell(spellInfo) || !petUnit->HasSpell(spellid))
                return;

            if (!petUnit->IsSpellReady(*spellInfo))
                return;

            for (unsigned int i : spellInfo->EffectImplicitTargetA)
            {
                if (i == TARGET_ENUM_UNITS_ENEMY_AOE_AT_SRC_LOC
                        || i == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DEST_LOC
                        || i == TARGET_ENUM_UNITS_ENEMY_AOE_AT_DYNOBJ_LOC)
                    return;
            }

            petUnit->clearUnitState(UNIT_STAT_MOVING);

            uint32 flags = TRIGGERED_NONE;
            if (!petUnit->hasUnitState(UNIT_STAT_POSSESSED))
                flags |= TRIGGERED_PET_CAST;

            Spell* spell = new Spell(petUnit, spellInfo, flags);

            SpellCastResult result = spell->CheckPetCast(unit_target);

            const SpellRangeEntry* sRange = sSpellRangeStore.LookupEntry(spellInfo->rangeIndex);

            if (unit_target && !(petUnit->IsWithinDistInMap(unit_target, sRange->maxRange) && petUnit->IsWithinLOSInMap(unit_target))
                    && petUnit->CanAttackNow(unit_target))
            {
                charmInfo->SetSpellOpener(spellid, sRange->minRange, sRange->maxRange);
                spell->finish(false);
                delete spell;

                petUnit->AttackStop();

                if (!petUnit->hasUnitState(UNIT_STAT_POSSESSED))
                {
                    petUnit->GetMotionMaster()->Clear();

                    petUnit->AI()->AttackStart(unit_target);
                    // 10% chance to play special warlock pet attack talk, else growl
                    if (pet && pet->getPetType() == SUMMON_PET && pet != unit_target && roll_chance_i(10))
                        petUnit->SendPetTalk((uint32)PET_TALK_ATTACK);

                    petUnit->SendPetAIReaction();
                }
                else
                    petUnit->Attack(unit_target, true);

                return;
            }

            // auto turn to target unless possessed
            if (result == SPELL_FAILED_UNIT_NOT_INFRONT && !petUnit->hasUnitState(UNIT_STAT_POSSESSED))
            {
                if (unit_target)
                {
                    petUnit->SetInFront(unit_target);
                    if (unit_target->GetTypeId() == TYPEID_PLAYER)
                        petUnit->SendCreateUpdateToPlayer((Player*)unit_target);
                }
                else if (Unit* unit_target2 = spell->m_targets.getUnitTarget())
                {
                    petUnit->SetInFront(unit_target2);
                    if (unit_target2->GetTypeId() == TYPEID_PLAYER)
                        petUnit->SendCreateUpdateToPlayer((Player*)unit_target2);
                }
                if (Unit* powner = petUnit->GetMaster())
                    if (powner->GetTypeId() == TYPEID_PLAYER)
                        petUnit->SendCreateUpdateToPlayer((Player*)powner);
                result = SPELL_CAST_OK;
            }

            if (result == SPELL_CAST_OK)
            {
                charmInfo->SetSpellOpener();
                spell->SpellStart(&(spell->m_targets));
            }
            else
            {
                if (creature && creature->IsSpellReady(*spellInfo))
                    GetPlayer()->SendClearCooldown(spellid, petUnit);

                charmInfo->SetSpellOpener();
                spell->finish(false);
                delete spell;
            }
            break;
        }
        default:
            sLog.outError("WORLD: unknown PET flag Action %i and spellid %i.", uint32(flag), spellid);
    }
}

void WorldSession::HandlePetStopAttack(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_PET_STOP_ATTACK");

    ObjectGuid petGuid;
    recv_data >> petGuid;

    Unit* pet = _player->GetMap()->GetUnit(petGuid);    // pet or controlled creature/player
    if (!pet)
    {
        sLog.outError("%s doesn't exist.", petGuid.GetString().c_str());
        return;
    }

    if (_player->GetObjectGuid() != pet->GetMasterGuid())
    {
        sLog.outError("HandlePetStopAttack: %s isn't charm/pet of %s.", petGuid.GetString().c_str(), _player->GetGuidStr().c_str());
        return;
    }

    if (!pet->isAlive())
        return;

    pet->AttackStop();
}

void WorldSession::HandlePetNameQueryOpcode(WorldPacket& recv_data)
{
    DETAIL_LOG("HandlePetNameQuery. CMSG_PET_NAME_QUERY");

    uint32 petnumber;
    ObjectGuid petguid;

    recv_data >> petnumber;
    recv_data >> petguid;

    SendPetNameQuery(petguid, petnumber);
}

void WorldSession::SendPetNameQuery(ObjectGuid petguid, uint32 petnumber) const
{
    Creature* pet = _player->GetMap()->GetAnyTypeCreature(petguid);

    // When replying to a query, verify input against public field info only (where it comes from)
    if (!pet || !pet->GetCharmInfo() || (pet->GetUInt32Value(UNIT_FIELD_PETNUMBER) && pet->GetUInt32Value(UNIT_FIELD_PETNUMBER) != petnumber))
    {
        WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4 + 1 + 4 + 1));
        data << uint32(petnumber);
        data << uint8(0);
        data << uint32(0);
        data << uint8(0);
        _player->GetSession()->SendPacket(data);
        return;
    }

    char const* name = pet->GetName();

    // creature pets have localization like other creatures
    if (!pet->GetOwnerGuid().IsPlayer())
    {
        int loc_idx = GetSessionDbLocaleIndex();
        sObjectMgr.GetCreatureLocaleStrings(pet->GetEntry(), loc_idx, &name);
    }

    WorldPacket data(SMSG_PET_NAME_QUERY_RESPONSE, (4 + 4 + strlen(name) + 1));
    data << uint32(petnumber);
    data << name;
    data << uint32(pet->GetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP));

    _player->GetSession()->SendPacket(data);
}

void WorldSession::HandlePetSetAction(WorldPacket& recv_data)
{
    DETAIL_LOG("HandlePetSetAction. CMSG_PET_SET_ACTION");

    ObjectGuid petGuid;

    recv_data >> petGuid;

    Unit* petUnit = _player->GetMap()->GetUnit(petGuid);

    if (!petUnit || (petUnit != _player->GetPet() && petUnit != _player->GetCharm()))
    {
        sLog.outError("HandlePetSetAction: Unknown pet or pet owner.");
        return;
    }

    Creature* petCreature = petUnit->GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(petUnit) : nullptr;
    Pet* pet = (petCreature && petCreature->IsPet()) ? static_cast<Pet*>(petUnit) : nullptr;

    // pet can have action bar disabled
    if (pet && (pet->GetModeFlags() & PET_MODE_DISABLE_ACTIONS))
        return;

    CharmInfo* charmInfo = petUnit->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outError("WorldSession::HandlePetSetAction: %s is considered pet-like but doesn't have a charminfo!", petUnit->GetGuidStr().c_str());
        return;
    }

    uint8 count = (recv_data.size() == 24) ? 2 : 1;

    uint32 position[2];
    uint32 data[2];
    bool move_command = false;

    for (uint8 i = 0; i < count; ++i)
    {
        recv_data >> position[i];
        recv_data >> data[i];

        uint8 act_state = UNIT_ACTION_BUTTON_TYPE(data[i]);

        // ignore invalid position
        if (position[i] >= MAX_UNIT_ACTION_BAR_INDEX)
            return;

        // in the normal case, command and reaction buttons can only be moved, not removed
        // at moving count ==2, at removing count == 1
        // ignore attempt to remove command|reaction buttons (not possible at normal case)
        if (act_state == ACT_COMMAND || act_state == ACT_REACTION)
        {
            if (count == 1)
                return;

            move_command = true;
        }
    }

    // check swap (at command->spell swap client remove spell first in another packet, so check only command move correctness)
    if (move_command)
    {
        uint8 act_state_0 = UNIT_ACTION_BUTTON_TYPE(data[0]);
        if (act_state_0 == ACT_COMMAND || act_state_0 == ACT_REACTION)
        {
            uint32 spell_id_0 = UNIT_ACTION_BUTTON_ACTION(data[0]);
            UnitActionBarEntry const* actionEntry_1 = charmInfo->GetActionBarEntry(position[1]);
            if (!actionEntry_1 || spell_id_0 != actionEntry_1->GetAction() ||
                    act_state_0 != actionEntry_1->GetType())
                return;
        }

        uint8 act_state_1 = UNIT_ACTION_BUTTON_TYPE(data[1]);
        if (act_state_1 == ACT_COMMAND || act_state_1 == ACT_REACTION)
        {
            uint32 spell_id_1 = UNIT_ACTION_BUTTON_ACTION(data[1]);
            UnitActionBarEntry const* actionEntry_0 = charmInfo->GetActionBarEntry(position[0]);
            if (!actionEntry_0 || spell_id_1 != actionEntry_0->GetAction() ||
                    act_state_1 != actionEntry_0->GetType())
                return;
        }
    }

    for (uint8 i = 0; i < count; ++i)
    {
        uint32 spell_id = UNIT_ACTION_BUTTON_ACTION(data[i]);
        uint8 act_state = UNIT_ACTION_BUTTON_TYPE(data[i]);

        DETAIL_LOG("Player %s has changed pet spell action. Position: %u, Spell: %u, State: 0x%X", _player->GetName(), position[i], spell_id, uint32(act_state));

        // if it's act for spell (en/disable/cast) and there is a spell given (0 = remove spell) which pet doesn't know, don't add
        if (!((act_state == ACT_ENABLED || act_state == ACT_DISABLED || act_state == ACT_PASSIVE) && spell_id && !petUnit->HasSpell(spell_id)))
        {
            // sign for autocast
            if (act_state == ACT_ENABLED && spell_id)
            {
                if (petUnit->HasCharmer())
                    charmInfo->ToggleCreatureAutocast(spell_id, true);
                else if (pet)
                    pet->ToggleAutocast(spell_id, true);
            }
            // sign for no/turn off autocast
            else if (act_state == ACT_DISABLED && spell_id)
            {
                if (petUnit->HasCharmer())
                    charmInfo->ToggleCreatureAutocast(spell_id, false);
                else if (pet)
                    pet->ToggleAutocast(spell_id, false);
            }

            charmInfo->SetActionBar(position[i], spell_id, ActiveStates(act_state));
        }
    }
}

void WorldSession::HandlePetRename(WorldPacket& recv_data)
{
    DETAIL_LOG("HandlePetRename. CMSG_PET_RENAME");

    ObjectGuid petGuid;
    std::string name;

    recv_data >> petGuid;
    recv_data >> name;

    Pet* pet = _player->GetMap()->GetPet(petGuid);
    // check it!
    if (!pet || pet->getPetType() != HUNTER_PET ||
            !pet->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_RENAME) ||
            pet->GetOwnerGuid() != _player->GetObjectGuid() || !pet->GetCharmInfo())
        return;

    PetNameInvalidReason res = ObjectMgr::CheckPetName(name);
    if (res != PET_NAME_SUCCESS)
    {
        SendPetNameInvalid(res, name);
        return;
    }

    if (sObjectMgr.IsReservedName(name))
    {
        SendPetNameInvalid(PET_NAME_RESERVED, name);
        return;
    }

    pet->SetName(name);

    if (_player->GetGroup())
        _player->SetGroupUpdateFlag(GROUP_UPDATE_FLAG_PET_NAME);

    pet->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PET_RENAME);

    CharacterDatabase.BeginTransaction();
    CharacterDatabase.escape_string(name);
    CharacterDatabase.PExecute("UPDATE character_pet SET name = '%s', renamed = '1' WHERE owner = '%u' AND id = '%u'", name.c_str(), _player->GetGUIDLow(), pet->GetCharmInfo()->GetPetNumber());
    CharacterDatabase.CommitTransaction();

    pet->SetUInt32Value(UNIT_FIELD_PET_NAME_TIMESTAMP, uint32(time(nullptr)));
}

void WorldSession::HandlePetAbandon(WorldPacket& recv_data)
{
    ObjectGuid guid;
    recv_data >> guid;                                      // pet guid

    DETAIL_LOG("HandlePetAbandon. CMSG_PET_ABANDON pet guid is %s", guid.GetString().c_str());

    if (!_player->IsInWorld())
        return;

    // pet/charmed
    if (Unit* petUnit = _player->GetMap()->GetUnit(guid))
    {
        if (petUnit->GetOwnerGuid() != _player->GetObjectGuid() || !petUnit->GetCharmInfo())
            return;

        Creature* petCreature = petUnit->GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(petUnit) : nullptr;
        Pet* pet = (petCreature && petCreature->IsPet()) ? static_cast<Pet*>(petUnit) : nullptr;

        if (pet)
        {
            // Permanently abandon pet
            if (pet->getPetType() == HUNTER_PET)
                pet->Unsummon(PET_SAVE_AS_DELETED, _player);
            // Simply dismiss
            else
                petUnit->SetDeathState(CORPSE);
        }
        else
            _player->BreakCharmOutgoing(petUnit);
    }
}

void WorldSession::HandlePetUnlearnOpcode(WorldPacket& recvPacket)
{
    DETAIL_LOG("CMSG_PET_UNLEARN");

    ObjectGuid guid;
    recvPacket >> guid;                 // Pet guid

    Pet* pet = _player->GetPet();

    if (!pet || guid != pet->GetObjectGuid())
    {
        sLog.outError("HandlePetUnlearnOpcode. %s isn't pet of %s .", guid.GetString().c_str(), _player->GetGuidStr().c_str());
        return;
    }

    if (pet->getPetType() != HUNTER_PET || pet->m_spells.size() <= 1)
        return;

    CharmInfo* charmInfo = pet->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outError("WorldSession::HandlePetUnlearnOpcode: %s is considered pet-like but doesn't have a charminfo!", pet->GetGuidStr().c_str());
        return;
    }

    uint32 cost = pet->resetTalentsCost();

    if (_player->GetMoney() < cost)
    {
        _player->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
        return;
    }

    for (PetSpellMap::iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end();)
    {
        uint32 spell_id = itr->first;                       // Pet::removeSpell can invalidate iterator at erase NEW spell
        ++itr;
        pet->unlearnSpell(spell_id, false);
    }

    pet->SetTP(pet->getLevel() * (pet->GetLoyaltyLevel() - 1));

    for (int i = 0; i < MAX_UNIT_ACTION_BAR_INDEX; ++i)
        if (UnitActionBarEntry const* ab = charmInfo->GetActionBarEntry(i))
            if (ab->GetAction() && ab->IsActionBarForSpell())
                charmInfo->SetActionBar(i, 0, ACT_DISABLED);

    // relearn pet passives
    pet->LearnPetPassives();

    pet->m_resetTalentsTime = time(nullptr);
    pet->m_resetTalentsCost = cost;
    _player->ModifyMoney(-(int32)cost);

    _player->PetSpellInitialize();
}

void WorldSession::HandlePetSpellAutocastOpcode(WorldPacket& recvPacket)
{
    DETAIL_LOG("CMSG_PET_SPELL_AUTOCAST");

    ObjectGuid guid;
    uint32 spellid;
    uint8  state;                                           // 1 for on, 0 for off
    recvPacket >> guid >> spellid >> state;

    Unit* petUnit = _player->GetMap()->GetUnit(guid);
    if (!petUnit || (guid != _player->GetPetGuid() && !_player->HasCharm(guid)))
    {
        sLog.outError("HandlePetSpellAutocastOpcode. %s isn't pet of %s .", guid.GetString().c_str(), _player->GetGuidStr().c_str());
        return;
    }

    Creature* petCreature = petUnit->GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(petUnit) : nullptr;
    Pet* pet = (petCreature && petCreature->IsPet()) ? static_cast<Pet*>(petUnit) : nullptr;

    // do not add not learned spells/ passive spells
    if (!petUnit->HasSpell(spellid) || !IsAutocastable(spellid))
        return;

    CharmInfo* charmInfo = petUnit->GetCharmInfo();
    if (!charmInfo)
    {
        sLog.outError("WorldSession::HandlePetSpellAutocastOpcod: %s is considered pet-like but doesn't have a charminfo!", guid.GetString().c_str());
        return;
    }

    if (petUnit->HasCharmer())
        // state can be used as boolean
        petUnit->GetCharmInfo()->ToggleCreatureAutocast(spellid, state != 0);
    else if (pet)
        pet->ToggleAutocast(spellid, state != 0);

    charmInfo->SetSpellAutocast(spellid, state != 0);
}

void WorldSession::HandlePetCastSpellOpcode(WorldPacket& recvPacket)
{
    DETAIL_LOG("WORLD: CMSG_PET_CAST_SPELL");

    ObjectGuid guid;
    uint32 spellid;

    recvPacket >> guid >> spellid;

    DEBUG_LOG("WORLD: CMSG_PET_CAST_SPELL, %s, spellid %u", guid.GetString().c_str(), spellid);

    Unit* petUnit = _player->GetMap()->GetUnit(guid);
    if (!petUnit || (guid != _player->GetPetGuid() && !_player->HasCharm(guid)))
    {
        sLog.outError("HandlePetSpellAutocastOpcode. %s isn't pet of %s .", guid.GetString().c_str(), _player->GetGuidStr().c_str());
        return;
    }

    //TODO: all those typecasting are probably not needed anymore
    Creature* petCreature = petUnit->GetTypeId() == TYPEID_UNIT ? static_cast<Creature*>(petUnit) : nullptr;
    Pet* pet = (petCreature && petCreature->IsPet()) ? static_cast<Pet*>(petUnit) : nullptr;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
    if (!spellInfo)
    {
        sLog.outError("WORLD: unknown PET spell id %i", spellid);
        return;
    }

    if (!petUnit->IsSpellReady(*spellInfo))
        return;


    // do not cast not learned spells
    if (!petUnit->HasSpell(spellid) || IsPassiveSpell(spellInfo))
        return;

    SpellCastTargets targets;

    recvPacket >> targets.ReadForCaster(petUnit);

    petUnit->clearUnitState(UNIT_STAT_MOVING);

    Spell* spell = new Spell(petUnit, spellInfo, TRIGGERED_PET_CAST);
    spell->m_targets = targets;

    SpellCastResult result = spell->CheckPetCast(nullptr);
    if (result == SPELL_CAST_OK)
    {
        if (pet)
            pet->CheckLearning(spellid);

        spell->SpellStart(&(spell->m_targets));
    }
    else
    {
        petUnit->SendPetCastFail(spellid, result);
        if (petCreature && petCreature->IsSpellReady(spellid))
            GetPlayer()->SendClearCooldown(spellid, petUnit);

        spell->finish(false);
        delete spell;
    }
}

void WorldSession::SendPetNameInvalid(uint32 error, const std::string& name) const
{
    // [-ZERO] Need check
    WorldPacket data(SMSG_PET_NAME_INVALID, 4 + name.size() + 1 + 1);
    data << uint32(error);
    data << name;
    data << uint8(0);                                       // possible not exist in 1.12.*
    SendPacket(data);
}
