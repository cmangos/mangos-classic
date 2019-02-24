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

#ifndef MANGOS_GRIDNOTIFIERSIMPL_H
#define MANGOS_GRIDNOTIFIERSIMPL_H

#include "Grids/GridNotifiers.h"
#include "WorldPacket.h"
#include "Entities/Player.h"
#include "AI/BaseAI/UnitAI.h"
#include "Spells/SpellAuras.h"
#include "Server/DBCEnums.h"
#include "Server/SQLStorages.h"
#include "Spells/SpellMgr.h"

#include <memory>

template<class T>
inline void MaNGOS::VisibleNotifier::Visit(GridRefManager<T>& m)
{
    for (typename GridRefManager<T>::iterator iter = m.begin(); iter != m.end(); ++iter)
    {
        i_camera.UpdateVisibilityOf(iter->getSource(), i_data, i_visibleNow);
        i_clientGUIDs.erase(iter->getSource()->GetObjectGuid());
    }
}

inline void MaNGOS::ObjectUpdater::Visit(CreatureMapType& m)
{
    for (auto& iter : m)
        m_objectToUpdateSet.emplace(iter.getSource());
}

inline void UnitVisitObjectsNotifierWorker(Unit* unitA, Unit* unitB)
{
    if (unitA->hasUnitState(UNIT_STAT_LOST_CONTROL) ||
        unitA->IsInEvadeMode() ||
        !unitA->AI()->IsVisible(unitB))
        return;

    unitA->AI()->MoveInLineOfSight(unitB);
}

inline void PlayerVisitCreatureWorker(Player* pl, Creature* c)
{
    if (!c->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (c->AI() && c->AI()->IsVisible(pl) && !c->IsInEvadeMode())
            c->AI()->MoveInLineOfSight(pl);
    }

    if (!pl->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (pl->AI() && pl->AI()->IsVisible(c) && !pl->IsInEvadeMode())
            pl->AI()->MoveInLineOfSight(c);
    }
}

inline void PlayerVisitPlayerWorker(Player* p1, Player* p2)
{
    if (!p2->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (p2->AI() && p2->AI()->IsVisible(p1) && !p2->IsInEvadeMode())
            p2->AI()->MoveInLineOfSight(p1);
    }

    if (!p1->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (p1->AI() && p1->AI()->IsVisible(p2) && !p1->IsInEvadeMode())
            p1->AI()->MoveInLineOfSight(p2);
    }
}

inline void CreatureVisitCreatureWorker(Creature* c1, Creature* c2)
{
    if (!c1->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (c1->AI() && c1->AI()->IsVisible(c2) && !c1->IsInEvadeMode())
            c1->AI()->MoveInLineOfSight(c2);
    }

    if (!c2->hasUnitState(UNIT_STAT_LOST_CONTROL))
    {
        if (c2->AI() && c2->AI()->IsVisible(c1) && !c2->IsInEvadeMode())
            c2->AI()->MoveInLineOfSight(c1);
    }
}

template<>
inline void MaNGOS::PlayerVisitObjectsNotifier::Visit(CreatureMapType& m)
{
    if (!i_player.isAlive() || i_player.IsTaxiFlying())
        return;

    bool playerHasAI = i_player.AI() != nullptr;

    for (auto& iter : m)
    {
        Creature* creature = iter.getSource();
        if (!creature->isAlive())
            continue;

        UnitVisitObjectsNotifierWorker(creature, &i_player);

        if (playerHasAI)
            UnitVisitObjectsNotifierWorker(&i_player, creature);
    }
}

template<>
inline void MaNGOS::PlayerVisitObjectsNotifier::Visit(PlayerMapType& m)
{
    if (!i_player.isAlive() || i_player.IsTaxiFlying())
        return;

    bool playerHasAI = i_player.AI() != nullptr;

    for (auto& iter : m)
    {
        Player* player = iter.getSource();
        if (player->isAlive() && !player->IsTaxiFlying())
            continue;

        if (player->AI())
            UnitVisitObjectsNotifierWorker(player, &i_player);

        if (playerHasAI)
            UnitVisitObjectsNotifierWorker(&i_player, player);
    }
}

template<>
inline void MaNGOS::CreatureVisitObjectsNotifier::Visit(PlayerMapType& m)
{
    if (!i_creature.isAlive())
        return;

    for (auto& iter : m)
    {
        Player* player = iter.getSource();
        if (!player->isAlive() || player->IsTaxiFlying())
            continue;

        if (player->AI())
            UnitVisitObjectsNotifierWorker(player, &i_creature);

        UnitVisitObjectsNotifierWorker(&i_creature, player);
    }
}

template<>
inline void MaNGOS::CreatureVisitObjectsNotifier::Visit(CreatureMapType& m)
{
    if (!i_creature.isAlive())
        return;

    for (auto& iter : m)
    {
        Creature* creature = iter.getSource();
        if (creature == &i_creature || !creature->isAlive())
            continue;

        UnitVisitObjectsNotifierWorker(creature, &i_creature);

        UnitVisitObjectsNotifierWorker(&i_creature, creature);
    }
}

inline void MaNGOS::DynamicObjectUpdater::VisitHelper(Unit* target)
{
    if (!target->isAlive() || target->IsTaxiFlying())
        return;

    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsTotem())
        return;

    if (i_dynobject.GetDistance(target, true, DIST_CALC_COMBAT_REACH) > i_dynobject.GetRadius())
        return;

    // Evade target
    if (target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->IsInEvadeMode())
        return;

    // Check player targets and remove if in GM mode or GM invisibility (for not self casting case)
    if (target->GetTypeId() == TYPEID_PLAYER && target != i_check && (((Player*)target)->isGameMaster() || ((Player*)target)->GetVisibility() == VISIBILITY_OFF))
        return;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(i_dynobject.GetSpellId());
    SpellEffectIndex eff_index  = i_dynobject.GetEffIndex();
    Unit* caster = i_dynobject.GetCaster();

    SQLMultiStorage::SQLMSIteratorBounds<SpellTargetEntry> bounds = sSpellScriptTargetStorage.getBounds<SpellTargetEntry>(spellInfo->Id);
    if (bounds.first != bounds.second)
    {
        bool found = false;
        for (SQLMultiStorage::SQLMultiSIterator<SpellTargetEntry> i_spellST = bounds.first; i_spellST != bounds.second; ++i_spellST)
        {
            if (i_spellST->CanNotHitWithSpellEffect(eff_index))
                continue;

            // only creature entries supported for this target type
            if (i_spellST->type == SPELL_TARGET_TYPE_GAMEOBJECT)
                continue;

            if (target->GetEntry() == i_spellST->targetEntry)
            {
                if (i_spellST->type == SPELL_TARGET_TYPE_DEAD && ((Creature*)target)->IsCorpse())
                    found = true;
                else if (i_spellST->type == SPELL_TARGET_TYPE_CREATURE && target->isAlive())
                    found = true;

                break;
            }
        }

        if (!found)
            return;
    }
    // This condition is only needed due to missing neutral spell type
    else if(i_dynobject.GetTarget() != TARGET_ENUM_UNITS_SCRIPT_AOE_AT_DEST_LOC)
    {
        // for player casts use less strict negative and more stricted positive targeting
        if (i_positive)
        {
            if (!caster->CanAssistSpell(target, spellInfo))
                return;
        }
        else
        {
            if (!caster->CanAttackSpell(target, spellInfo, true))
                return;
        }
    }

    if (spellInfo->HasAttribute(SPELL_ATTR_EX3_TARGET_ONLY_PLAYER) && target->GetTypeId() != TYPEID_PLAYER)
        return;

    // Check target immune to spell or aura
    if (target->IsImmuneToSpell(spellInfo, false, eff_index) || target->IsImmuneToSpellEffect(spellInfo, eff_index, false))
        return;

    if (!i_dynobject.IsWithinLOSInMap(target))
        return;

    // Apply PersistentAreaAura on target
    // in case 2 dynobject overlap areas for same spell, same holder is selected, so dynobjects share holder
    SpellAuraHolder* holder = target->GetSpellAuraHolder(spellInfo->Id, i_dynobject.GetCasterGuid());

    if (holder)
    {
        if (!holder->GetAuraByEffectIndex(eff_index))
        {
            PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, eff_index, &i_dynobject.GetDamage(), &i_dynobject.GetBasePoints(), holder, target, caster);
            holder->AddAura(Aur, eff_index);
            target->AddAuraToModList(Aur);
            Aur->ApplyModifier(true, true);
        }
        else if (holder->GetAuraDuration() >= 0 && uint32(holder->GetAuraDuration()) < i_dynobject.GetDuration())
        {
            holder->SetAuraDuration(i_dynobject.GetDuration());
            holder->UpdateAuraDuration();
        }
    }
    else
    {
        holder = CreateSpellAuraHolder(spellInfo, target, caster);
        PersistentAreaAura* Aur = new PersistentAreaAura(spellInfo, eff_index, &i_dynobject.GetDamage(), &i_dynobject.GetBasePoints(), holder, target, caster);
        holder->AddAura(Aur, eff_index);
        if (!target->AddSpellAuraHolder(holder))
            delete holder;
    }

    if (!i_dynobject.IsAffecting(target))
    {
        i_dynobject.AddAffected(target);
        caster->CasterHitTargetWithSpell(caster, target, spellInfo);
    }
}

template<>
inline void MaNGOS::DynamicObjectUpdater::Visit(CreatureMapType&  m)
{
    for (auto& itr : m)
        VisitHelper(itr.getSource());
}

template<>
inline void MaNGOS::DynamicObjectUpdater::Visit(PlayerMapType&  m)
{
    for (auto& itr : m)
        VisitHelper(itr.getSource());
}

// SEARCHERS & LIST SEARCHERS & WORKERS

// WorldObject searchers & workers

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(GameObjectMapType& m)
{
    // already found
    if (i_object)
        return;

    for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(PlayerMapType& m)
{
    // already found
    if (i_object)
        return;

    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(CreatureMapType& m)
{
    // already found
    if (i_object)
        return;

    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(CorpseMapType& m)
{
    // already found
    if (i_object)
        return;

    for (CorpseMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectSearcher<Check>::Visit(DynamicObjectMapType& m)
{
    // already found
    if (i_object)
        return;

    for (DynamicObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(PlayerMapType& m)
{
    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(CreatureMapType& m)
{
    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(CorpseMapType& m)
{
    for (CorpseMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(GameObjectMapType& m)
{
    for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::WorldObjectListSearcher<Check>::Visit(DynamicObjectMapType& m)
{
    for (DynamicObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

// Gameobject searchers

template<class Check>
void MaNGOS::GameObjectSearcher<Check>::Visit(GameObjectMapType& m)
{
    // already found
    if (i_object)
        return;

    for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::GameObjectLastSearcher<Check>::Visit(GameObjectMapType& m)
{
    for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void MaNGOS::GameObjectListSearcher<Check>::Visit(GameObjectMapType& m)
{
    for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

// Unit searchers

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(CreatureMapType& m)
{
    // already found
    if (i_object)
        return;

    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::UnitSearcher<Check>::Visit(PlayerMapType& m)
{
    // already found
    if (i_object)
        return;

    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::UnitLastSearcher<Check>::Visit(CreatureMapType& m)
{
    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void MaNGOS::UnitLastSearcher<Check>::Visit(PlayerMapType& m)
{
    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(PlayerMapType& m)
{
    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::UnitListSearcher<Check>::Visit(CreatureMapType& m)
{
    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

// Creature searchers

template<class Check>
void MaNGOS::CreatureSearcher<Check>::Visit(CreatureMapType& m)
{
    // already found
    if (i_object)
        return;

    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::CreatureLastSearcher<Check>::Visit(CreatureMapType& m)
{
    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
            i_object = itr->getSource();
    }
}

template<class Check>
void MaNGOS::CreatureListSearcher<Check>::Visit(CreatureMapType& m)
{
    for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Check>
void MaNGOS::PlayerSearcher<Check>::Visit(PlayerMapType& m)
{
    // already found
    if (i_object)
        return;

    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
    {
        if (i_check(itr->getSource()))
        {
            i_object = itr->getSource();
            return;
        }
    }
}

template<class Check>
void MaNGOS::PlayerListSearcher<Check>::Visit(PlayerMapType& m)
{
    for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
        if (i_check(itr->getSource()))
            i_objects.push_back(itr->getSource());
}

template<class Builder>
void MaNGOS::LocalizedPacketDo<Builder>::operator()(Player* p)
{
    int32 loc_idx = p->GetSession()->GetSessionDbLocaleIndex();
    uint32 cache_idx = loc_idx + 1;

    // create if not cached yet
    if (i_data_cache.size() < cache_idx + 1 || !i_data_cache[cache_idx])
    {
        if (i_data_cache.size() < cache_idx + 1)
            i_data_cache.resize(cache_idx + 1);

        auto data = std::unique_ptr<WorldPacket>(new WorldPacket());

        i_builder(*data, loc_idx);

        i_data_cache[cache_idx] = std::move(data);
    }

    p->SendDirectMessage(*i_data_cache[cache_idx]);
}

template<class Builder>
void MaNGOS::LocalizedPacketListDo<Builder>::operator()(Player* p)
{
    int32 loc_idx = p->GetSession()->GetSessionDbLocaleIndex();
    uint32 cache_idx = loc_idx + 1;
    WorldPacketList* data_list;

    // create if not cached yet
    if (i_data_cache.size() < cache_idx + 1 || i_data_cache[cache_idx].empty())
    {
        if (i_data_cache.size() < cache_idx + 1)
            i_data_cache.resize(cache_idx + 1);

        data_list = &i_data_cache[cache_idx];

        i_builder(*data_list, loc_idx);
    }
    else
        data_list = &i_data_cache[cache_idx];

    for (size_t i = 0; i < data_list->size(); ++i)
        p->SendDirectMessage(*(*data_list)[i]);
}

#endif                                                      // MANGOS_GRIDNOTIFIERSIMPL_H
