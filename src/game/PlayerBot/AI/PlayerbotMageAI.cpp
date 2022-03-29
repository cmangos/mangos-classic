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

#include "PlayerbotMageAI.h"

class PlayerbotAI;

PlayerbotMageAI::PlayerbotMageAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    ARCANE_MISSILES         = m_ai.initSpell(ARCANE_MISSILES_1);
    ARCANE_EXPLOSION        = m_ai.initSpell(ARCANE_EXPLOSION_1);
    COUNTERSPELL            = m_ai.initSpell(COUNTERSPELL_1);
    ARCANE_POWER            = m_ai.initSpell(ARCANE_POWER_1);
    DAMPEN_MAGIC            = m_ai.initSpell(DAMPEN_MAGIC_1);
    AMPLIFY_MAGIC           = m_ai.initSpell(AMPLIFY_MAGIC_1);
    MAGE_ARMOR              = m_ai.initSpell(MAGE_ARMOR_1);
    ARCANE_INTELLECT        = m_ai.initSpell(ARCANE_INTELLECT_1);
    ARCANE_BRILLIANCE       = m_ai.initSpell(ARCANE_BRILLIANCE_1);
    MANA_SHIELD             = m_ai.initSpell(MANA_SHIELD_1);
    CONJURE_WATER           = m_ai.initSpell(CONJURE_WATER_1);
    CONJURE_FOOD            = m_ai.initSpell(CONJURE_FOOD_1);
    CONJURE_MANA_GEM        = m_ai.initSpell(CONJURE_MANA_GEM_1);
    EVOCATION               = m_ai.initSpell(EVOCATION_1);
    FIREBALL                = m_ai.initSpell(FIREBALL_1);
    FIRE_BLAST              = m_ai.initSpell(FIRE_BLAST_1);
    FLAMESTRIKE             = m_ai.initSpell(FLAMESTRIKE_1);
    SCORCH                  = m_ai.initSpell(SCORCH_1);
    POLYMORPH               = m_ai.initSpell(POLYMORPH_1);
    PRESENCE_OF_MIND        = m_ai.initSpell(PRESENCE_OF_MIND_1);
    PYROBLAST               = m_ai.initSpell(PYROBLAST_1);
    BLAST_WAVE              = m_ai.initSpell(BLAST_WAVE_1);
    COMBUSTION              = m_ai.initSpell(COMBUSTION_1);
    FIRE_WARD               = m_ai.initSpell(FIRE_WARD_1);
    FROSTBOLT               = m_ai.initSpell(FROSTBOLT_1);
    FROST_NOVA              = m_ai.initSpell(FROST_NOVA_1);
    BLIZZARD                = m_ai.initSpell(BLIZZARD_1);
    CONE_OF_COLD            = m_ai.initSpell(CONE_OF_COLD_1);
    ICE_BARRIER             = m_ai.initSpell(ICE_BARRIER_1);
    FROST_WARD              = m_ai.initSpell(FROST_WARD_1);
    FROST_ARMOR             = m_ai.initSpell(FROST_ARMOR_1);
    ICE_ARMOR               = m_ai.initSpell(ICE_ARMOR_1);
    ICE_BLOCK               = m_ai.initSpell(ICE_BLOCK_1);
    COLD_SNAP               = m_ai.initSpell(COLD_SNAP_1);
    MAGE_REMOVE_CURSE       = m_ai.initSpell(REMOVE_CURSE_MAGE_1);

    // TALENTS
    IMPROVED_SCORCH         = 0;
    for (unsigned int i : uiImprovedScorch)
    {
        if (m_ai.initSpell(i))
            IMPROVED_SCORCH = m_ai.initSpell(i);
    }
    FIRE_VULNERABILITY      = 22959;

    // RANGED COMBAT
    SHOOT                   = m_ai.initSpell(SHOOT_2);

    RECENTLY_BANDAGED       = 11196; // first aid check

    // racial
    ESCAPE_ARTIST           = m_ai.initSpell(ESCAPE_ARTIST_ALL); // gnome
    PERCEPTION              = m_ai.initSpell(PERCEPTION_ALL); // human
    BERSERKING              = m_ai.initSpell(BERSERKING_ALL); // troll
    WILL_OF_THE_FORSAKEN    = m_ai.initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead
}

PlayerbotMageAI::~PlayerbotMageAI() {}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuver(Unit* pTarget)
{
    // There are NPCs in BGs and Open World PvP, so don't filter this on PvP scenarios (of course if PvP targets anyone but tank, all bets are off anyway)
    // Wait until the tank says so, until any non-tank gains aggro or X seconds - whichever is shortest
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.GroupTankHoldsAggro())
        {
            return RETURN_NO_ACTION_OK; // wait it out
        }
        else
        {
            m_ai.ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_TANKAGGRO);
        }
    }

    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_TEMP_WAIT_OOC)
    {
        if (m_WaitUntil > m_ai.CurrentTime() && m_ai.IsGroupReady())
            return RETURN_NO_ACTION_OK; // wait it out
        else
            m_ai.ClearGroupCombatOrder(PlayerbotAI::ORDERS_TEMP_WAIT_OOC);
    }

    switch (m_ai.GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoFirstCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoFirstCombatManeuverPVE(pTarget);
    }
}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotMageAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuver(Unit* pTarget)
{
    // Face enemy, make sure bot is attacking
    m_ai.FaceTarget(pTarget);

    switch (m_ai.GetScenarioType())
    {
        case PlayerbotAI::SCENARIO_PVP_DUEL:
        case PlayerbotAI::SCENARIO_PVP_BG:
        case PlayerbotAI::SCENARIO_PVP_ARENA:
        case PlayerbotAI::SCENARIO_PVP_OPENWORLD:
            return DoNextCombatManeuverPVP(pTarget);
        case PlayerbotAI::SCENARIO_PVE:
        case PlayerbotAI::SCENARIO_PVE_ELITE:
        case PlayerbotAI::SCENARIO_PVE_RAID:
        default:
            return DoNextCombatManeuverPVE(pTarget);
    }
}

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    Unit* pVictim = pTarget->GetVictim();
    bool meleeReach = m_bot.CanReachWithMeleeAttack(pTarget);

    uint32 spec = m_bot.GetSpec();

    if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_RANGED && !meleeReach)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    // switch to melee if in melee range AND can't shoot OR have no ranged (wand) equipped
    else if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE
             && meleeReach
             && (SHOOT == 0 || !m_bot.GetWeaponForAttack(RANGED_ATTACK, true, true)))
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);

    // Remove curse on group members
    if (m_ai.HasDispelOrder() && DispelPlayer(GetDispelTarget(DISPEL_CURSE)) & RETURN_CONTINUE)
        return RETURN_CONTINUE;

    if (newTarget && !PlayerbotAI::IsNeutralized(newTarget)) // Bot has aggro and the mob is not already crowd controled
    {
        if (newTarget->GetHealthPercent() > 25)
        {
            // If elite
            if (m_ai.IsElite(newTarget))
            {
                // If the attacker is a beast or humanoid, let's the bot give it a form more suited to the low intellect of something fool enough to attack a mage
                // No need to check if target is a player as IsElite() returns false for players
                Creature* creature = (Creature*) newTarget;
                if (creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID || creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_BEAST)
                {
                    if (POLYMORPH > 0 && !PlayerbotAI::IsImmuneToSchool(newTarget, SPELL_SCHOOL_MASK_ARCANE) && CastSpell(POLYMORPH, newTarget))
                        return RETURN_CONTINUE;
                }

                // Things are getting dire: cast Ice block
                if (ICE_BLOCK > 0 && m_bot.IsSpellReady(ICE_BLOCK) && m_ai.GetHealthPercent() < 30 && !m_bot.HasAura(ICE_BLOCK, EFFECT_INDEX_0) && m_ai.CastSpell(ICE_BLOCK) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;

                // Cast Ice Barrier if health starts to goes low
                if (ICE_BARRIER > 0 && m_bot.IsSpellReady(ICE_BARRIER) && m_ai.GetHealthPercent() < 50 && !m_bot.HasAura(ICE_BARRIER) && m_ai.SelfBuff(ICE_BARRIER) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;

                // Have threat, can't quickly lower it. 3 options remain: Stop attacking, lowlevel damage (wand), keep on keeping on.
                return CastSpell(SHOOT, pTarget);
            }
            else // not elite
            {
                // Cast mana shield if no shield is already up
                if (MANA_SHIELD > 0 && m_ai.GetHealthPercent() < 70 && !m_bot.HasAura(MANA_SHIELD) && !m_bot.HasAura(ICE_BARRIER) && m_ai.SelfBuff(MANA_SHIELD) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
            }
        }
    }

    // Mana check and replenishment
    if (EVOCATION && m_ai.GetManaPercent() <= 10 && m_bot.IsSpellReady(EVOCATION) && !newTarget && m_ai.SelfBuff(EVOCATION) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (m_ai.GetManaPercent() <= 20)
    {
        Item* gem = FindManaGem();
        if (gem)
            m_ai.UseItem(gem);
    }

    // If bot has frost/fire resist order use Frost/Fire Ward when available
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FROST && FROST_WARD && m_bot.IsSpellReady(FROST_WARD) && m_ai.SelfBuff(FROST_WARD) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (m_ai.GetCombatOrder() & PlayerbotAI::ORDERS_RESIST_FIRE && FIRE_WARD && m_bot.IsSpellReady(FIRE_WARD) && m_ai.SelfBuff(FIRE_WARD) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    if (COUNTERSPELL > 0 && m_bot.IsSpellReady(COUNTERSPELL) && pTarget->IsNonMeleeSpellCasted(true) && CastSpell(COUNTERSPELL, pTarget))
        return RETURN_CONTINUE;

    // If Clearcasting is active, cast arcane missiles
    // Bot could also cast flamestrike or blizzard for free, but the AoE could break some crowd control
    // or add threat on mobs ignoring the bot currently, so only focus on the bot's current target
    if (m_bot.HasAura(CLEARCASTING_1) && ARCANE_MISSILES > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_ARCANE) && CastSpell(ARCANE_MISSILES, pTarget))
    {
        m_ai.SetIgnoreUpdateTime(3);
        return RETURN_CONTINUE;
    }

    switch (spec)
    {
        case MAGE_SPEC_FROST:
            if (COLD_SNAP && m_bot.IsSpellReady(COLD_SNAP) && CheckFrostCooldowns() > 2 && m_ai.SelfBuff(COLD_SNAP) == SPELL_CAST_OK)  // Clear frost spell cooldowns if bot has more than 2 active
                return RETURN_CONTINUE;
            if (CONE_OF_COLD > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_bot.IsSpellReady(CONE_OF_COLD) && meleeReach)
            {
                // Cone of Cold does not require a target, so ensure that the bot faces the current one before casting
                m_ai.FaceTarget(pTarget);
                if (m_ai.CastSpell(CONE_OF_COLD) == SPELL_CAST_OK)
                    return RETURN_CONTINUE;
            }
            if (FROSTBOLT > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_ai.In_Reach(pTarget, FROSTBOLT) && !pTarget->HasAura(FROSTBOLT, EFFECT_INDEX_0) && CastSpell(FROSTBOLT, pTarget))
                return RETURN_CONTINUE;
            if (FROST_NOVA > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_bot.IsSpellReady(FROST_NOVA) && meleeReach && !pTarget->HasAura(FROST_NOVA, EFFECT_INDEX_0) && CastSpell(FROST_NOVA, pTarget))
                return RETURN_CONTINUE;
            // Default frost spec action
            if (FROSTBOLT > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_ai.In_Reach(pTarget, FROSTBOLT))
                return CastSpell(FROSTBOLT, pTarget);
            /*
            if (BLIZZARD > 0 && !m_ai.IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_ai.In_Reach(pTarget,BLIZZARD) && m_ai.GetAttackerCount() >= 5 && CastSpell(BLIZZARD, pTarget))
            {
                m_ai.SetIgnoreUpdateTime(8);
                return RETURN_CONTINUE;
            }
            */
            break;

        case MAGE_SPEC_FIRE:
            if (COMBUSTION > 0 && m_ai.SelfBuff(COMBUSTION) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            if (BLAST_WAVE > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.GetAttackerCount() >= 3 && meleeReach && CastSpell(BLAST_WAVE, pTarget))
                return RETURN_CONTINUE;
            // Try to have 3 scorch stacks to let tank build aggro while getting a nice crit% bonus
            if (IMPROVED_SCORCH > 0 && SCORCH > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE))
            {
                if (!pTarget->HasAura(FIRE_VULNERABILITY, EFFECT_INDEX_0) && CastSpell(SCORCH, pTarget))   // no stacks: cast it
                    return RETURN_CONTINUE;
                else
                {
                    SpellAuraHolder* holder = pTarget->GetSpellAuraHolder(FIRE_VULNERABILITY);
                    if (holder && (holder->GetStackAmount() < 3) && CastSpell(SCORCH, pTarget))
                        return RETURN_CONTINUE;
                }
            }
            // At least 3 stacks of Scorch: cast an opening fireball
            if (FIREBALL > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && !pTarget->HasAura(FIREBALL, EFFECT_INDEX_1) && CastSpell(FIREBALL, pTarget))
                return RETURN_CONTINUE;
            // 3 stacks of Scorch and fireball DoT: use fire blast if available
            if (FIRE_BLAST > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_bot.IsSpellReady(FIRE_BLAST) && CastSpell(FIRE_BLAST, pTarget))
                return RETURN_CONTINUE;
            // All DoTs, cooldowns used, try to maximise scorch stacks (5) to get a even nicer crit% bonus
            if (IMPROVED_SCORCH > 0 && SCORCH > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE))
            {
                SpellAuraHolder* holder = pTarget->GetSpellAuraHolder(FIRE_VULNERABILITY);
                if (holder && (holder->GetStackAmount() < 5) && CastSpell(SCORCH, pTarget))
                    return RETURN_CONTINUE;
            }
            // Default fire spec action
            if (FIREBALL > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, FIREBALL))
                return CastSpell(FIREBALL, pTarget);
            /*
            if (FLAMESTRIKE > 0 && !m_ai.IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget,FLAMESTRIKE) && CastSpell(FLAMESTRIKE, pTarget))
                return RETURN_CONTINUE;
            */
            break;

        case MAGE_SPEC_ARCANE:
            if (ARCANE_POWER > 0 && m_bot.IsSpellReady(ARCANE_POWER) && m_ai.IsElite(pTarget) && m_ai.CastSpell(ARCANE_POWER) == SPELL_CAST_OK)    // Do not waste Arcane Power on normal NPCs as the bot is likely in a group
                return RETURN_CONTINUE;
            if (PRESENCE_OF_MIND > 0 && !m_bot.HasAura(PRESENCE_OF_MIND) && m_bot.IsSpellReady(PRESENCE_OF_MIND) && m_ai.IsElite(pTarget) && m_ai.SelfBuff(PRESENCE_OF_MIND) == SPELL_CAST_OK)
                return RETURN_CONTINUE;
            // If bot has presence of mind active, cast long casting time spells
            if (PRESENCE_OF_MIND && m_bot.HasAura(PRESENCE_OF_MIND))
            {
                // Instant Pyroblast, yeah! Tanks will probably hate this, but what do they know about power? Nothing...
                if (PYROBLAST > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && CastSpell(PYROBLAST, pTarget))
                    return RETURN_CONTINUE;
                if (FIREBALL > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && CastSpell(FIREBALL, pTarget))
                    return RETURN_CONTINUE;
            }
            if (ARCANE_EXPLOSION > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_ARCANE) && m_ai.GetAttackerCount() >= 3 && meleeReach && CastSpell(ARCANE_EXPLOSION, pTarget))
                return RETURN_CONTINUE;
            // Default arcane spec actions (yes, two fire spells)
            if (FIRE_BLAST > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_bot.IsSpellReady(FIRE_BLAST) && CastSpell(FIRE_BLAST, pTarget))
                return RETURN_CONTINUE;
            if (FIREBALL > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, FIREBALL))
                return CastSpell(FIREBALL, pTarget);
            // If no fireball, arcane missiles
            if (ARCANE_MISSILES > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_ARCANE) && CastSpell(ARCANE_MISSILES, pTarget))
            {
                m_ai.SetIgnoreUpdateTime(3);
                return RETURN_CONTINUE;
            }
            break;
    }

    // No spec due to low level OR no spell found yet
    if (FROSTBOLT > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FROST) && m_ai.In_Reach(pTarget, FROSTBOLT) && !pTarget->HasAura(FROSTBOLT, EFFECT_INDEX_0) && CastSpell(FROSTBOLT, pTarget))
        return RETURN_CONTINUE;
    if (FIREBALL > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, FIREBALL) && CastSpell(FIREBALL, pTarget)) // Very low levels
        return RETURN_CONTINUE;

    // Default: shoot with wand
    return CastSpell(SHOOT, pTarget);
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotMageAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (FIREBALL && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, FIREBALL) && m_ai.CastSpell(FIREBALL) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

// Function to keep track of active frost cooldowns to clear with Cold Snap
uint8 PlayerbotMageAI::CheckFrostCooldowns()
{
    uint8 uiFrostActiveCooldown = 0;
    if (FROST_NOVA && !m_bot.IsSpellReady(FROST_NOVA))
        uiFrostActiveCooldown++;
    if (ICE_BARRIER && !m_bot.IsSpellReady(ICE_BARRIER))
        uiFrostActiveCooldown++;
    if (CONE_OF_COLD && !m_bot.IsSpellReady(CONE_OF_COLD))
        uiFrostActiveCooldown++;
    if (ICE_BLOCK && !m_bot.IsSpellReady(ICE_BLOCK))
        uiFrostActiveCooldown++;
    if (FROST_WARD && !m_bot.IsSpellReady(FROST_WARD))
        uiFrostActiveCooldown++;

    return uiFrostActiveCooldown;
}

static const uint32 uPriorizedManaGemIds[4] =
{
    MANA_RUBY_DISPLAYID, MANA_CITRINE_DISPLAYID, MANA_AGATE_DISPLAYID, MANA_JADE_DISPLAYID
};

// Return a mana gem Item based on the priorized list
Item* PlayerbotMageAI::FindManaGem() const
{
    Item* gem;
    for (unsigned int priorizedManaGemId : uPriorizedManaGemIds)
    {
        gem = m_ai.FindConsumable(priorizedManaGemId);
        if (gem)
            return gem;
    }
    return nullptr;
}

CombatManeuverReturns PlayerbotMageAI::DispelPlayer(Player* cursedTarget)
{
    CombatManeuverReturns r = PlayerbotClassAI::DispelPlayer(cursedTarget);
    if (r != RETURN_NO_ACTION_OK)
        return r;

    if (MAGE_REMOVE_CURSE > 0 && CastSpell(MAGE_REMOVE_CURSE, cursedTarget))
        return RETURN_CONTINUE;

    return RETURN_NO_ACTION_OK;
}

void PlayerbotMageAI::DoNonCombatActions()
{
    // Remove curse on group members
    if (m_ai.HasDispelOrder() && DispelPlayer(GetDispelTarget(DISPEL_CURSE)) & RETURN_CONTINUE)
        return;

    // Buff armor
    if (MAGE_ARMOR)
    {
        if (m_ai.SelfBuff(MAGE_ARMOR) == SPELL_CAST_OK)
            return;
    }
    else if (ICE_ARMOR)
    {
        if (m_ai.SelfBuff(ICE_ARMOR) == SPELL_CAST_OK)
            return;
    }
    else if (FROST_ARMOR)
    {
        if (m_ai.SelfBuff(FROST_ARMOR) == SPELL_CAST_OK)
            return;
    }

    if (COMBUSTION && m_bot.IsSpellReady(COMBUSTION) && m_ai.SelfBuff(COMBUSTION) == SPELL_CAST_OK)
        return;

    // buff group
    // the check for group targets is performed by NeedGroupBuff (if group is found for bots by the function)
    if (NeedGroupBuff(ARCANE_BRILLIANCE, ARCANE_INTELLECT) && m_ai.HasSpellReagents(ARCANE_BRILLIANCE))
    {
        if (Buff(&PlayerbotMageAI::BuffHelper, ARCANE_BRILLIANCE) & RETURN_CONTINUE)
            return;
    }
    else if (Buff(&PlayerbotMageAI::BuffHelper, ARCANE_INTELLECT, JOB_MANAONLY) & RETURN_CONTINUE)
        return;

    // if there is space on bag try to conjure some consumables
    if (m_ai.CanStore())
    {
        Item* gem = FindManaGem();
        if (!gem && CONJURE_MANA_GEM && m_ai.CastSpell(CONJURE_MANA_GEM, m_bot) == SPELL_CAST_OK)
        {
            m_ai.SetIgnoreUpdateTime(3);
            return;
        }

        // TODO: The beauty of a mage is not only its ability to supply itself with water, but to share its water
        // So, conjure at *least* 1.25 stacks, ready to trade a stack and still have some left for self
        if (m_ai.FindDrink() == nullptr && CONJURE_WATER && m_ai.CastSpell(CONJURE_WATER, m_bot) == SPELL_CAST_OK)
        {
            m_ai.TellMaster("I'm conjuring some water.");
            m_ai.SetIgnoreUpdateTime(3);
            return;
        }
        if (m_ai.FindFood() == nullptr && CONJURE_FOOD && m_ai.CastSpell(CONJURE_FOOD, m_bot) == SPELL_CAST_OK)
        {
            m_ai.TellMaster("I'm conjuring some food.");
            m_ai.SetIgnoreUpdateTime(3);
            return;
        }
    }

    if (EatDrinkBandage())
        return;
} // end DoNonCombatActions

// TODO: this and priest's BuffHelper are identical and thus could probably go in PlayerbotClassAI.cpp somewhere
bool PlayerbotMageAI::BuffHelper(PlayerbotAI* ai, uint32 spellId, Unit* target)
{
    if (!ai)          return false;
    if (spellId == 0) return false;
    if (!target)      return false;

    Pet* pet = target->GetPet();
    if (pet && !pet->HasAuraType(SPELL_AURA_MOD_UNATTACKABLE) && ai->Buff(spellId, pet) == SPELL_CAST_OK)
        return true;

    return ai->Buff(spellId, target) == SPELL_CAST_OK;
}

// Return to UpdateAI the spellId usable to neutralize a target with creaturetype
uint32 PlayerbotMageAI::Neutralize(uint8 creatureType)
{
    if (!creatureType)  return 0;

    if (creatureType != CREATURE_TYPE_HUMANOID && creatureType != CREATURE_TYPE_BEAST)
    {
        m_ai.TellMaster("I can't polymorph that target.");
        return 0;
    }

    if (POLYMORPH)
        return POLYMORPH;
    else
        return 0;
}