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

#include "PlayerbotClassAI.h"
#include "Common.h"

#include "Grids/Cell.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

PlayerbotClassAI::PlayerbotClassAI(Player& master, Player& bot, PlayerbotAI& ai)
    : m_master(master), m_bot(bot), m_ai(ai)
{

    m_MinHealthPercentTank   = 80;
    m_MinHealthPercentHealer = 60;
    m_MinHealthPercentDPS    = 30;
    m_MinHealthPercentMaster = m_MinHealthPercentDPS;

    ClearWait();
}
PlayerbotClassAI::~PlayerbotClassAI() {}

CombatManeuverReturns PlayerbotClassAI::DoFirstCombatManeuver(Unit*) { return RETURN_NO_ACTION_OK; }
CombatManeuverReturns PlayerbotClassAI::DoNextCombatManeuver(Unit*) { return RETURN_NO_ACTION_OK; }

CombatManeuverReturns PlayerbotClassAI::DoFirstCombatManeuverPVE(Unit*) { return RETURN_NO_ACTION_OK; }
CombatManeuverReturns PlayerbotClassAI::DoNextCombatManeuverPVE(Unit*) { return RETURN_NO_ACTION_OK; }
CombatManeuverReturns PlayerbotClassAI::DoFirstCombatManeuverPVP(Unit*) { return RETURN_NO_ACTION_OK; }
CombatManeuverReturns PlayerbotClassAI::DoNextCombatManeuverPVP(Unit*) { return RETURN_NO_ACTION_OK; }

void PlayerbotClassAI::DoNonCombatActions()
{
    DEBUG_LOG("[PlayerbotAI]: Warning: Using PlayerbotClassAI::DoNonCombatActions() rather than class specific function");
}

bool PlayerbotClassAI::EatDrinkBandage(bool bMana, unsigned char foodPercent, unsigned char drinkPercent, unsigned char bandagePercent)
{
    Item* drinkItem = nullptr;
    Item* foodItem = nullptr;
    if (bMana && m_ai.GetManaPercent() < drinkPercent)
        drinkItem = m_ai.FindDrink();
    if (m_ai.GetHealthPercent() < foodPercent)
        foodItem = m_ai.FindFood();
    if (drinkItem || foodItem)
    {
        if (drinkItem)
        {
            m_ai.TellMaster("I could use a drink.");
            m_ai.UseItem(drinkItem);
        }
        if (foodItem)
        {
            m_ai.TellMaster("I could use some food.");
            m_ai.UseItem(foodItem);
        }
        return true;
    }

    if (m_ai.GetHealthPercent() < bandagePercent && !m_bot.HasAura(RECENTLY_BANDAGED))
    {
        Item* bandageItem = m_ai.FindBandage();
        if (bandageItem)
        {
            m_ai.TellMaster("I could use first aid.");
            m_ai.UseItem(bandageItem);
            return true;
        }
    }

    return false;
}

bool PlayerbotClassAI::CastHoTOnTank()
{
    DEBUG_LOG("[PlayerbotAI]: Warning: Using PlayerbotClassAI::CastHoTOnTank() rather than class specific function");
    return false;
}

CombatManeuverReturns PlayerbotClassAI::HealPlayer(Player* target)
{
    if (!target) return RETURN_NO_ACTION_INVALIDTARGET;
    if (target->IsInDuel() || !target->IsAlive()) return RETURN_NO_ACTION_INVALIDTARGET;

    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotClassAI::ResurrectPlayer(Player* target)
{
    if (!target) return RETURN_NO_ACTION_INVALIDTARGET;
    if (target->IsAlive()) return RETURN_NO_ACTION_INVALIDTARGET;

    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotClassAI::DispelPlayer(Player* target)
{
    if (!target) return RETURN_NO_ACTION_INVALIDTARGET;
    if (target->IsInDuel() || !target->IsAlive()) return RETURN_NO_ACTION_INVALIDTARGET;

    return RETURN_NO_ACTION_OK;
}

/**
 * Buff()
 * return CombatManeuverReturns Returns RETURN_CONTINUE and buff unit target if it meets criteria to be buffed by bot with spell, else returns RETURN_NO_ACTION_OK
 *
 * params:BuffHelper boolean function that will be called back if buffing criteria are met
 * params:spellId uint32 the spell ID of the spell buff
 * params:type uint32, optional: default: JOB_ALL, the JOB_TYPE that target must have to be eligible for buffing
 *        Please note that job_type JOB_MANAONLY is a cumulative restriction. JOB_TANK | JOB_HEAL means both;
 *        JOB_TANK | JOB_MANAONLY means tanks with powertype MANA (paladins, druids)
 * params: mustBeOOC boolean, optional: default: false, will return RETURN_NO_ACTION_OK if set to true and bot is in combat
 * If false is returned, the bot is expected to perform a buff check for the single target version of the group buff.
 *
 */
CombatManeuverReturns PlayerbotClassAI::Buff(bool (*BuffHelper)(PlayerbotAI*, uint32, Unit*), uint32 spellId, uint32 type, bool mustBeOOC)
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return RETURN_NO_ACTION_ERROR;
    if (mustBeOOC && m_bot.IsInCombat()) return RETURN_NO_ACTION_ERROR;

    if (spellId == 0) return RETURN_NO_ACTION_OK;

    // First, fill the list of targets
    if (m_bot.GetGroup())
    {
        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (const auto & memberItr : groupSlot)
        {
            Player* member = sObjectMgr.GetPlayer(memberItr.guid);
            if (!member || !member->IsAlive() || member->IsInDuel())
                continue;
            // Guess the job of current member
            JOB_TYPE job = GetTargetJob(member);
            // If job matches requested job type or (mana target is requested and member is mana user (or shapeshift druid)): buff it
            if (job & type ||
                (type & JOB_MANAONLY && (member->getClass() == CLASS_DRUID || member->GetPowerType() == POWER_MANA)))
            {
                if (BuffHelper(&m_ai, spellId, member))
                    return RETURN_CONTINUE;
            }
        }
    }
    else
    {
        // Buff master if he/she is eligible
        if (!m_master.IsInDuel()
            && (GetTargetJob(&m_master) & type
                || (type & JOB_MANAONLY && (m_master.getClass() == CLASS_DRUID || m_master.GetPowerType() == POWER_MANA))))
            if (BuffHelper(&m_ai, spellId, &m_master))
                return RETURN_CONTINUE;
        // Do not check job or power type - any buff you have is always useful to self
        if (BuffHelper(&m_ai, spellId, &m_bot))
            return RETURN_CONTINUE;
    }

    return RETURN_NO_ACTION_OK;
}

/**
 * NeedGroupBuff()
 * return boolean Returns true if more than two targets in the bot's group need the group buff, else returns false
 *
 * params:groupBuffSpellId uint32 the spell ID of the group buff like Arcane Brillance
 * params:singleBuffSpellId uint32 the spell ID of the single target buff equivalent of the group buff like Arcane Intellect for group buff Arcane Brillance
 * If false is returned, the bot is expected to perform a buff check for the single target version of the group buff.
 *
 */
bool PlayerbotClassAI::NeedGroupBuff(uint32 groupBuffSpellId, uint32 singleBuffSpellId)
{
    uint8 unbuffedTargets = 0;
    // Check group players to avoid using reagent and mana with an expensive group buff
    // when only two players or less need it
    if (m_bot.GetGroup())
    {
        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (const auto& memberItr : groupSlot)
        {
            Player* member = sObjectMgr.GetPlayer(memberItr.guid);
            if (!member || !member->IsAlive())
                continue;
            // Check if group member needs buff
            if (!member->HasAura(groupBuffSpellId, EFFECT_INDEX_0) && !member->HasAura(singleBuffSpellId, EFFECT_INDEX_0))
                unbuffedTargets++;
            // Don't forget about pet
            Pet* pet = member->GetPet();
            if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && !(pet->HasAura(groupBuffSpellId, EFFECT_INDEX_0) || pet->HasAura(singleBuffSpellId, EFFECT_INDEX_0)))
                unbuffedTargets++;
        }
        // threshold is set to 2 targets because beyond that value, the group buff cost is cheaper in mana
        return unbuffedTargets >= 3;
    }
    return false;   // no group, no group buff
}

/**
 * FindTargetAndHeal()
 * return bool Returns true if a unit in need of healing was found and healed. Returns false else.
 * Find a target based on healing orders (no orders = no healing), then try to heal it
 * using own class HealPlayer() method
 */
bool PlayerbotClassAI::FindTargetAndHeal()
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel() || !m_ai.IsHealer()) return false;

    // Heal other players/bots first
    // Select a target based on orders and some context (pets are ignored because GetHealTarget() only works on players)
    Player* targetToHeal;
    JOB_TYPE type = (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_NOT_MAIN_HEAL) ? JOB_ALL_NO_MT : JOB_ALL;
    // 1. bot has orders to focus on main tank
    if (m_ai.IsMainHealer())
        targetToHeal = GetHealTarget(JOB_MAIN_TANK);
    // 2. Look at its own group (this implies raid leader creates balanced groups, except for the MT group)
    else
        targetToHeal = GetHealTarget(type, true);
    // 3. still no target to heal, search amongst everyone
    if (!targetToHeal)
        targetToHeal = GetHealTarget(type);

    if (m_ai.GetClassAI()->HealPlayer(targetToHeal) & RETURN_CONTINUE)
        return true;

    return false;   
}

/**
 * GetHealTarget()
 * return Unit* Returns unit to be healed. First checks Main Tank(s), next 'critical' Healer(s), next regular Tank(s)
 * next Master (if different from:), next DPS.
 * If none of the healths are low enough (or multiple valid targets) against these checks, the lowest health is healed. Having a target
 * returned does not guarantee it's worth healing, merely that the target does not have 100% health.
 *
 * return NULL If NULL is returned, no healing is required. At all.
 *
 * Will need extensive re-write for co-operation amongst multiple healers. As it stands, multiple healers would all pick the same 'ideal'
 * healing target.
 */
Player* PlayerbotClassAI::GetHealTarget(JOB_TYPE type, bool onlyPickFromSameGroup)
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return nullptr;

    // define seperately for sorting purposes - DO NOT CHANGE ORDER!
    std::vector<heal_priority> targets;
    uint8 uiHealthPercentage;

    // First, fill the list of targets
    if (m_bot.GetGroup())
    {
        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* groupMember = sObjectMgr.GetPlayer(itr->guid);
            if (!groupMember || !groupMember->IsAlive() || groupMember->IsInDuel()
                || (!m_bot.GetGroup()->SameSubGroup(&m_bot, groupMember) && onlyPickFromSameGroup))
                continue;
            JOB_TYPE job = GetTargetJob(groupMember);
            if (job & type)
            {
                uiHealthPercentage = int(groupMember->GetMaxHealth() != 0 ? groupMember->GetHealth() * 100 / groupMember->GetMaxHealth() : 0);
                targets.push_back(heal_priority(groupMember, uiHealthPercentage, job));
            }
        }
    }
    else
    {
        targets.push_back(heal_priority(&m_bot, m_bot.GetHealthPercent(), GetTargetJob(&m_bot)));
        if (!m_master.IsInDuel())
        {
            uiHealthPercentage = int(m_master.GetMaxHealth() != 0 ? m_master.GetHealth() * 100 / m_master.GetMaxHealth() : 0);
            targets.push_back(heal_priority(&m_master, uiHealthPercentage, GetTargetJob(&m_master)));
        }
    }

    // Sorts according to type: Main tank first, healers second, then regular tanks, then master followed by DPS, thanks to the order of the TYPE enum
    std::sort(targets.begin(), targets.end());

    uint8 uCount = 0, i = 0;
    // x is used as 'target found' variable; i is used as the targets iterator throughout all 6 types.
    int16 x = -1;

    // Try to find a main tank in need of healing (if multiple, the lowest health one)
    while (true)
    {
        // This works because we sorted it above
        if (uint32(uCount + i) >= uint32(targets.size()) || !(targets.at(uCount).type & JOB_MAIN_TANK)) break;
        uCount++;
    }

    // We have uCount main tanks in the targets, check if any qualify for priority healing
    for (; uCount > 0; uCount--, i++)
    {
        if (targets.at(i).hp <= m_MinHealthPercentTank)
            if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                x = i;
    }
    if (x > -1) return targets.at(x).p;

    // Try to find a healer in need of healing (if multiple, the lowest health one)
    while (true)
    {
        if (uint32(uCount + i) >= uint32(targets.size()) || !(targets.at(uCount).type & (JOB_HEAL | JOB_MAIN_HEAL))) break;
        uCount++;
    }

    for (; uCount > 0; uCount--, i++)
    {
        if (targets.at(i).hp <= m_MinHealthPercentHealer)
            if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                x = i;
    }
    if (x > -1) return targets.at(x).p;

    // Try to find a tank in need of healing (if multiple, the lowest health one)
    while (true)
    {
        if (uint32(uCount + i) >= uint32(targets.size()) || !(targets.at(uCount).type & JOB_TANK)) break;
        uCount++;
    }

    for (; uCount > 0; uCount--, i++)
    {
        if (targets.at(i).hp <= m_MinHealthPercentTank)
            if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                x = i;
    }
    if (x > -1) return targets.at(x).p;

    // Try to find master in need of healing (lowest health one first)
    if (m_MinHealthPercentMaster != m_MinHealthPercentDPS)
    {
        while (true)
        {
            if (uint32(uCount + i) >= uint32(targets.size()) || !(targets.at(uCount).type & JOB_MASTER)) break;
            uCount++;
        }

        for (; uCount > 0; uCount--, i++)
        {
            if (targets.at(i).hp <= m_MinHealthPercentMaster)
                if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                    x = i;
        }
        if (x > -1) return targets.at(x).p;
    }

    // Try to find anyone else in need of healing (lowest health one first)
    while (true)
    {
        if (uint32(uCount + i) >= uint32(targets.size())) break;
        uCount++;
    }

    for (; uCount > 0; uCount--, i++)
    {
        if (targets.at(i).hp <= m_MinHealthPercentDPS)
            if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                x = i;
    }
    if (x > -1) return targets.at(x).p;

    // Nobody is critical, find anyone hurt at all, return lowest (let the healer sort out if it's worth healing or not)
    for (i = 0, uCount = targets.size(); uCount > 0; uCount--, i++)
    {
        if (targets.at(i).hp < 100)
            if (x == -1 || targets.at(x).hp > targets.at(i).hp)
                x = i;
    }
    if (x > -1) return targets.at(x).p;

    return nullptr;
}

/**
 * FleeFromAoEIfCan()
 * return boolean Check if the bot can move out of the hostile AoE spell then try to find a proper destination and move towards it
 *                The AoE is assumed to be centered on the current bot location (this is the case most of the time)
 *
 * params: spellId uint32 the spell ID of the hostile AoE the bot is supposed to move from. It is used to find the radius of the AoE spell
 * params: pTarget Unit* the creature or gameobject the bot will use to define one of the prefered direction in which to flee
 *
 * return true if bot has found a proper destination, false if none was found
 */
bool PlayerbotClassAI::FleeFromAoEIfCan(uint32 spellId, Unit* pTarget)
{
    if (!spellId) return false;

    // Step 1: Get radius from hostile AoE spell
    float radius = 0;
    SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (spellproto)
        radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spellproto->EffectRadiusIndex[EFFECT_INDEX_0]));

    // Step 2: Get current bot position to move from it
    float curr_x, curr_y, curr_z;
    m_bot.GetPosition(curr_x, curr_y, curr_z);
    return FleeFromPointIfCan(radius, pTarget, curr_x, curr_y, curr_z);
}

/**
 * FleeFromTrapGOIfCan()
 * return boolean Check if the bot can move from a hostile nearby trap, then try to find a proper destination and move towards it
 *
 * params: goEntry uint32 the ID of the hostile trap the bot is supposed to move from. It is used to find the radius of the trap
 * params: pTarget Unit* the creature or gameobject the bot will use to define one of the prefered direction in which to flee
 *
 * return true if bot has found a proper destination, false if none was found
 */
bool PlayerbotClassAI::FleeFromTrapGOIfCan(uint32 goEntry, Unit* pTarget)
{
    if (!goEntry) return false;

    // Step 1: check if the GO exists and find its trap radius
    GameObjectInfo const* trapInfo = sGOStorage.LookupEntry<GameObjectInfo>(goEntry);
    if (!trapInfo || trapInfo->type != GAMEOBJECT_TYPE_TRAP)
        return false;

    float trapRadius = float(trapInfo->trap.diameter) / 2.f;

    // Step 2: find a GO in the range around player
    GameObject* pGo = nullptr;

    MaNGOS::NearestGameObjectEntryInObjectRangeCheck go_check(m_bot, goEntry, trapRadius);
    MaNGOS::GameObjectLastSearcher<MaNGOS::NearestGameObjectEntryInObjectRangeCheck> searcher(pGo, go_check);

    Cell::VisitGridObjects(&m_bot, searcher, trapRadius);

    if (!pGo)
        return false;

    return FleeFromPointIfCan(trapRadius, pTarget, pGo->GetPositionX(), pGo->GetPositionY(), pGo->GetPositionZ());
}

/**
 * FleeFromNpcWithAuraIfCan()
 * return boolean Check if the bot can move from a creature having a specific aura, then try to find a proper destination and move towards it
 *
 * params: goEntry uint32 the ID of the hostile trap the bot is supposed to move from. It is used to find the radius of the trap
 * params: spellId uint32 the spell ID of the aura the creature is supposed to have (directly or from triggered spell). It is used to find the radius of the aura
 * params: pTarget Unit* the creature or gameobject the bot will use to define one of the prefered direction in which to flee
 *
 * return true if bot has found a proper destination, false if none was found
 */
bool PlayerbotClassAI::FleeFromNpcWithAuraIfCan(uint32 NpcEntry, uint32 spellId, Unit* pTarget)
{
    if (!NpcEntry) return false;
    if (!spellId) return false;

    // Step 1: Get radius from hostile aura spell
    float radius = 0;
    SpellEntry const* spellproto = sSpellTemplate.LookupEntry<SpellEntry>(spellId);
    if (spellproto)
        radius = GetSpellRadius(sSpellRadiusStore.LookupEntry(spellproto->EffectRadiusIndex[EFFECT_INDEX_0]));

    if (radius == 0)
        return false;

    // Step 2: find a close creature with the right entry:
    Creature* pCreature = nullptr;

    MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck creature_check(m_bot, NpcEntry, false, false, radius, true);
    MaNGOS::CreatureLastSearcher<MaNGOS::NearestCreatureEntryWithLiveStateInObjectRangeCheck> searcher(pCreature, creature_check);

    Cell::VisitGridObjects(&m_bot, searcher, radius);

    if (!pCreature)
        return false;

    // Force to flee on a direction opposite to the position of the creature (fleeing from it, not only avoiding it)
    return FleeFromPointIfCan(radius, pTarget, pCreature->GetPositionX(), pCreature->GetPositionY(), pCreature->GetPositionZ(), M_PI_F);
}

/**
 * FleeFromPointIfCan()
 * return boolean Check if the bot can move from a provided point (x, y, z) to given distance radius
 *
 * params: radius uint32 the minimal radius (distance) used by the bot to look for a destination from the provided position
 * params: pTarget Unit* the creature or gameobject the bot will use to define one of the prefered direction in which to flee
 * params: x0, y0, z0 float the coordinates of the origin point used to calculate the destination point
 * params: forcedAngle float (optional) when iterating to find a proper point in world to move the bot to, this angle will be prioritly used over other angles if it is provided
 *
 * return true if bot has found a proper destination, false if none was found
 */
bool PlayerbotClassAI::FleeFromPointIfCan(uint32 radius, Unit* pTarget, float x0, float y0, float z0, float forcedAngle /* = 0.0f */)
{
    // Get relative position to current target
    // the bot will try to move on a tangential axis from it
    float dist_from_target, angle_to_target;
    if (pTarget)
    {
        dist_from_target = pTarget->GetDistance(&m_bot);
        if (dist_from_target > 0.2f)
            angle_to_target = pTarget->GetAngle(&m_bot);
        else
            angle_to_target = frand(0, 2 * M_PI_F);
    }
    else
    {
        dist_from_target = 0.0f;
        angle_to_target = frand(0, 2 * M_PI_F);
    }

    // Find coords to move to
    // The bot will move for a distance equal to the spell radius + 1 yard for more safety
    float dist = radius + 1.0f;

    float moveAngles[3] = {- M_PI_F / 2, M_PI_F / 2, 0.0f};
    float angle, x, y, z;
    bool foundCoords;
    for (uint8 i = 0; i < 3; i++)
    {
        // define an angle tangential to target's direction
        angle = angle_to_target + moveAngles[i];
        // if an angle was provided, use it instead but only for the first iteration in case this does not lead to a valid point
        if (forcedAngle != 0.0f)
        {
            angle = forcedAngle;
            forcedAngle = 0.0f;
        }
        foundCoords = true;

        x = x0 + dist * cos(angle);
        y = y0 + dist * sin(angle);
        z = z0 + 0.5f;

        // try to fix z
        if (!m_bot.GetMap()->GetHeightInRange(x, y, z))
            foundCoords = false;

        // check any collision
        float testZ = z + 0.5f; // needed to avoid some false positive hit detection of terrain or passable little object
        if (m_bot.GetMap()->GetHitPosition(x0, y0, z0 + 0.5f, x, y, testZ, -0.1f))
        {
            z = testZ;
            if (!m_bot.GetMap()->GetHeightInRange(x, y, z))
                foundCoords = false;
        }

        if (foundCoords)
        {
            m_ai.InterruptCurrentCastingSpell();
            m_bot.GetMotionMaster()->MovePoint(0, x, y, z);
            m_ai.SetIgnoreUpdateTime(2);
            return true;
        }
    }

    return false;
}

/**
 * GetDispelTarget()
 * return Unit* Returns unit to be dispelled. First checks Main Tank(s), next 'critical' Healer(s), next regular Tank(s)
 * next Master (if different from:), next DPS.
 *
 * return NULL If NULL is returned, no healing is required. At all.
 *
 * Will need extensive re-write for co-operation amongst multiple healers. As it stands, multiple healers would all pick the same 'ideal'
 * healing target.
 */
Player* PlayerbotClassAI::GetDispelTarget(DispelType dispelType, JOB_TYPE type, bool bMustBeOOC)
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return nullptr;
    if (bMustBeOOC && m_bot.IsInCombat()) return nullptr;

    // First, fill the list of targets
    if (m_bot.GetGroup())
    {
        // define seperately for sorting purposes - DO NOT CHANGE ORDER!
        std::vector<heal_priority> targets;

        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* groupMember = sObjectMgr.GetPlayer(itr->guid);
            if (!groupMember || !groupMember->IsAlive())
                continue;
            JOB_TYPE job = GetTargetJob(groupMember);
            if (job & type)
            {
                uint32 dispelMask  = GetDispellMask(dispelType);
                Unit::SpellAuraHolderMap const& auras = groupMember->GetSpellAuraHolderMap();
                for (Unit::SpellAuraHolderMap::const_iterator itr = auras.begin(); itr != auras.end(); ++itr)
                {
                    SpellAuraHolder* holder = itr->second;
                    // Only return group members with negative magic effect
                    if (dispelType == DISPEL_MAGIC && holder->IsPositive())
                        continue;
                    // poison, disease and curse are always negative: return everyone
                    if ((1 << holder->GetSpellProto()->Dispel) & dispelMask)
                        targets.push_back(heal_priority(groupMember, 0, job));
                }
            }
        }

        // Sorts according to type: Main tank first, healers second, then regular tanks, then master followed by DPS, thanks to the order of the TYPE enum
        std::sort(targets.begin(), targets.end());

        if (targets.size())
            return targets.at(0).p;
    }

    return nullptr;
}

Player* PlayerbotClassAI::GetResurrectionTarget(JOB_TYPE type, bool bMustBeOOC)
{
    if (!m_bot.IsAlive() || m_bot.IsInDuel()) return nullptr;
    if (bMustBeOOC && m_bot.IsInCombat()) return nullptr;

    // First, fill the list of targets
    if (m_bot.GetGroup())
    {
        // define seperately for sorting purposes - DO NOT CHANGE ORDER!
        std::vector<heal_priority> targets;

        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* groupMember = sObjectMgr.GetPlayer(itr->guid);
            if (!groupMember || groupMember->IsAlive())
                continue;
            JOB_TYPE job = GetTargetJob(groupMember);
            if (job & type)
                targets.push_back(heal_priority(groupMember, 0, job));
        }

        // Sorts according to type: Main tank first, healers second, then regular tanks, then master followed by DPS, thanks to the order of the TYPE enum
        std::sort(targets.begin(), targets.end());

        if (targets.size())
            return targets.at(0).p;
    }
    else if (!m_master.IsAlive())
        return &m_master;

    return nullptr;
}

JOB_TYPE PlayerbotClassAI::GetBotJob(Player* target)
{
        if (target->GetPlayerbotAI()->IsMainHealer())
            return JOB_MAIN_HEAL;
        if (target->GetPlayerbotAI()->IsHealer())
            return JOB_HEAL;
        if (target->GetPlayerbotAI()->IsMainTank())
            return JOB_MAIN_TANK;
        if (target->GetPlayerbotAI()->IsTank())
            return JOB_TANK;
        return JOB_DPS;
}

JOB_TYPE PlayerbotClassAI::GetTargetJob(Player* target)
{
    // is a bot
    if (target->GetPlayerbotAI())
        return GetBotJob(target);

    // figure out what to do with human players - i.e. figure out if they're tank, DPS or healer
    uint32 uSpec = target->GetSpec();
    switch (target->getClass())
    {
        case CLASS_PALADIN:
            if (uSpec == PALADIN_SPEC_HOLY)
                return JOB_HEAL;
            if (uSpec == PALADIN_SPEC_PROTECTION)
                return JOB_TANK;
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
        case CLASS_DRUID:
            if (uSpec == DRUID_SPEC_RESTORATION)
                return JOB_HEAL;
            // Feral can be used for both Tank or DPS... play it safe and assume tank. If not... he best be good at threat management or he'll ravage the healer's mana
            else if (uSpec == DRUID_SPEC_FERAL)
                return JOB_TANK;
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
        case CLASS_PRIEST:
            // Since Discipline can be used for both healer or DPS assume DPS
            if (uSpec == PRIEST_SPEC_HOLY)
                return JOB_HEAL;
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
        case CLASS_SHAMAN:
            if (uSpec == SHAMAN_SPEC_RESTORATION)
                return JOB_HEAL;
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
        case CLASS_WARRIOR:
            if (uSpec == WARRIOR_SPEC_PROTECTION)
                return JOB_TANK;
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
        case CLASS_MAGE:
        case CLASS_WARLOCK:
        case CLASS_ROGUE:
        case CLASS_HUNTER:
        default:
            return (&m_master == target) ? JOB_MASTER : JOB_DPS;
    }
}

CombatManeuverReturns PlayerbotClassAI::CastSpellNoRanged(uint32 nextAction, Unit* pTarget)
{
    if (nextAction == 0)
        return RETURN_NO_ACTION_OK; // Asked to do nothing so... yeh... Dooone.

    if (pTarget != nullptr)
        return (m_ai.CastSpell(nextAction, *pTarget) == SPELL_CAST_OK ? RETURN_CONTINUE : RETURN_NO_ACTION_ERROR);
    else
        return (m_ai.CastSpell(nextAction) == SPELL_CAST_OK ? RETURN_CONTINUE : RETURN_NO_ACTION_ERROR);
}

CombatManeuverReturns PlayerbotClassAI::CastSpellWand(uint32 nextAction, Unit* pTarget, uint32 SHOOT)
{
    if (SHOOT > 0 && m_bot.FindCurrentSpellBySpellId(SHOOT) && m_bot.GetWeaponForAttack(RANGED_ATTACK, true, true))
    {
        if (nextAction == SHOOT)
            // At this point we're already shooting and are asked to shoot. Don't cause a global cooldown by stopping to shoot! Leave it be.
            return RETURN_CONTINUE; // ... We're asked to shoot and are already shooting so... Task accomplished?

        // We are shooting but wish to cast a spell. Stop 'casting' shoot.
        m_bot.InterruptNonMeleeSpells(true, SHOOT);
        // ai->TellMaster("Interrupting auto shot.");
    }

    // We've stopped ranged (if applicable), if no nextAction just return
    if (nextAction == 0)
        return RETURN_CONTINUE; // Asked to do nothing so... yeh... Dooone.

    if (nextAction == SHOOT)
    {
        if (SHOOT > 0 && m_ai.GetCombatStyle() == PlayerbotAI::COMBAT_RANGED && !m_bot.FindCurrentSpellBySpellId(SHOOT) && m_bot.GetWeaponForAttack(RANGED_ATTACK, true, true))
            return (m_ai.CastSpell(SHOOT, *pTarget) == SPELL_CAST_OK ? RETURN_CONTINUE : RETURN_NO_ACTION_ERROR);
        else
            // Do Melee attack
            return RETURN_NO_ACTION_UNKNOWN; // We're asked to shoot and aren't.
    }

    if (pTarget != nullptr)
        return (m_ai.CastSpell(nextAction, *pTarget) == SPELL_CAST_OK ? RETURN_CONTINUE : RETURN_NO_ACTION_ERROR);
    else
        return (m_ai.CastSpell(nextAction) == SPELL_CAST_OK ? RETURN_CONTINUE : RETURN_NO_ACTION_ERROR);
}
