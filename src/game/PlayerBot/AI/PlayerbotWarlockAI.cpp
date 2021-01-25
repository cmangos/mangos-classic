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

#include "PlayerbotWarlockAI.h"

class PlayerbotAI;
PlayerbotWarlockAI::PlayerbotWarlockAI(Player& master, Player& bot, PlayerbotAI& ai) : PlayerbotClassAI(master, bot, ai)
{
    // DESTRUCTION
    SHADOW_BOLT           = m_ai.initSpell(SHADOW_BOLT_1);
    IMMOLATE              = m_ai.initSpell(IMMOLATE_1);
    SEARING_PAIN          = m_ai.initSpell(SEARING_PAIN_1);
    CONFLAGRATE           = m_ai.initSpell(CONFLAGRATE_1);
    HELLFIRE              = m_ai.initSpell(HELLFIRE_1);
    RAIN_OF_FIRE          = m_ai.initSpell(RAIN_OF_FIRE_1);
    SOUL_FIRE             = m_ai.initSpell(SOUL_FIRE_1); // soul shard spells
    SHADOWBURN            = m_ai.initSpell(SHADOWBURN_1);
    // CURSE
    CURSE_OF_WEAKNESS     = m_ai.initSpell(CURSE_OF_WEAKNESS_1);
    CURSE_OF_THE_ELEMENTS = m_ai.initSpell(CURSE_OF_THE_ELEMENTS_1);
    CURSE_OF_AGONY        = m_ai.initSpell(CURSE_OF_AGONY_1);
    CURSE_OF_EXHAUSTION   = m_ai.initSpell(CURSE_OF_EXHAUSTION_1);
    CURSE_OF_RECKLESSNESS = m_ai.initSpell(CURSE_OF_RECKLESSNESS_1);
    CURSE_OF_SHADOW       = m_ai.initSpell(CURSE_OF_SHADOW_1);
    CURSE_OF_TONGUES      = m_ai.initSpell(CURSE_OF_TONGUES_1);
    CURSE_OF_DOOM         = m_ai.initSpell(CURSE_OF_DOOM_1);
    // AFFLICTION
    AMPLIFY_CURSE         = m_ai.initSpell(AMPLIFY_CURSE_1);
    CORRUPTION            = m_ai.initSpell(CORRUPTION_1);
    DRAIN_SOUL            = m_ai.initSpell(DRAIN_SOUL_1);
    DRAIN_LIFE            = m_ai.initSpell(DRAIN_LIFE_1);
    DRAIN_MANA            = m_ai.initSpell(DRAIN_MANA_1);
    LIFE_TAP              = m_ai.initSpell(LIFE_TAP_1);
    DARK_PACT             = m_ai.initSpell(DARK_PACT_1);
    HOWL_OF_TERROR        = m_ai.initSpell(HOWL_OF_TERROR_1);
    FEAR                  = m_ai.initSpell(FEAR_1);
    SIPHON_LIFE           = m_ai.initSpell(SIPHON_LIFE_1);
    // DEMONOLOGY
    BANISH                = m_ai.initSpell(BANISH_1);
    ENSLAVE_DEMON         = m_ai.initSpell(ENSLAVE_DEMON_1);
    DEMON_SKIN            = m_ai.initSpell(DEMON_SKIN_1);
    DEMON_ARMOR           = m_ai.initSpell(DEMON_ARMOR_1);
    SHADOW_WARD           = m_ai.initSpell(SHADOW_WARD_1);
    SOUL_LINK             = m_ai.initSpell(SOUL_LINK_1);
    SOUL_LINK_AURA        = 25228; // dummy aura applied, after spell SOUL_LINK
    HEALTH_FUNNEL         = m_ai.initSpell(HEALTH_FUNNEL_1);
    DETECT_INVISIBILITY   = m_ai.initSpell(DETECT_INVISIBILITY_1);
    CREATE_FIRESTONE      = m_ai.initSpell(CREATE_FIRESTONE_1);
    CREATE_HEALTHSTONE    = m_ai.initSpell(CREATE_HEALTHSTONE_1);
    CREATE_SOULSTONE      = m_ai.initSpell(CREATE_SOULSTONE_1);
    CREATE_SPELLSTONE     = m_ai.initSpell(CREATE_SPELLSTONE_1);
    // demon summon
    SUMMON_IMP            = m_ai.initSpell(SUMMON_IMP_1);
    SUMMON_VOIDWALKER     = m_ai.initSpell(SUMMON_VOIDWALKER_1);
    SUMMON_SUCCUBUS       = m_ai.initSpell(SUMMON_SUCCUBUS_1);
    SUMMON_FELHUNTER      = m_ai.initSpell(SUMMON_FELHUNTER_1);
    // demon skills should be initialized on demons
    BLOOD_PACT            = 0; // imp skill
    CONSUME_SHADOWS       = 0; // voidwalker skill
    // RANGED COMBAT
    SHOOT                 = m_ai.initSpell(SHOOT_3);

    RECENTLY_BANDAGED     = 11196; // first aid check

    // racial
    ESCAPE_ARTIST         = m_ai.initSpell(ESCAPE_ARTIST_ALL); // gnome
    PERCEPTION            = m_ai.initSpell(PERCEPTION_ALL); // human
    BLOOD_FURY            = m_ai.initSpell(BLOOD_FURY_ALL); // orc
    WILL_OF_THE_FORSAKEN  = m_ai.initSpell(WILL_OF_THE_FORSAKEN_ALL); // undead

    m_lastDemon           = 0;
    m_isTempImp           = false;
    m_CurrentCurse        = 0;
}

PlayerbotWarlockAI::~PlayerbotWarlockAI() {}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuver(Unit* pTarget)
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
            break;
    }

    return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuverPVE(Unit* /*pTarget*/)
{
    m_CurrentCurse = 0;
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarlockAI::DoFirstCombatManeuverPVP(Unit* /*pTarget*/)
{
    return RETURN_NO_ACTION_OK;
}

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuver(Unit* pTarget)
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
            break;
    }

    return RETURN_NO_ACTION_ERROR;
}

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuverPVE(Unit* pTarget)
{
    //Unit* pVictim = pTarget->GetVictim();
    bool meleeReach = m_bot.CanReachWithMeleeAttack(pTarget);
    Pet* pet = m_bot.GetPet();
    uint32 spec = m_bot.GetSpec();
    uint8 shardCount = m_bot.GetItemCount(SOUL_SHARD, false, nullptr);

    // Voidwalker is near death - sacrifice it for a shield
    if (pet && pet->GetEntry() == DEMON_VOIDWALKER && SACRIFICE && !m_bot.HasAura(SACRIFICE) && pet->GetHealthPercent() < 10)
        m_ai.CastPetSpell(SACRIFICE);

    // Use healthstone
    if (m_ai.GetHealthPercent() < 30)
    {
        Item* healthStone = m_ai.FindConsumable(HEALTHSTONE_DISPLAYID);
        if (healthStone)
            m_ai.UseItem(healthStone);
    }

    // Voidwalker sacrifice gives shield - but you lose the pet (and it's DPS/tank) - use only as last resort for your own health!
    if (m_ai.GetHealthPercent() < 20 && pet && pet->GetEntry() == DEMON_VOIDWALKER && SACRIFICE && !m_bot.HasAura(SACRIFICE))
        m_ai.CastPetSpell(SACRIFICE);

    if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_RANGED && !meleeReach)
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_RANGED);
    // switch to melee if in melee range AND can't shoot OR have no ranged (wand) equipped
    else if (m_ai.GetCombatStyle() != PlayerbotAI::COMBAT_MELEE
             && meleeReach
             && (SHOOT == 0 || !m_bot.GetWeaponForAttack(RANGED_ATTACK, true, true)))
        m_ai.SetCombatStyle(PlayerbotAI::COMBAT_MELEE);

    //Used to determine if this bot is highest on threat
    Unit* newTarget = m_ai.FindAttacker((PlayerbotAI::ATTACKERINFOTYPE)(PlayerbotAI::AIT_VICTIMSELF | PlayerbotAI::AIT_HIGHESTTHREAT), &m_bot);
    if (newTarget && !PlayerbotAI::IsNeutralized(newTarget)) // TODO: && party has a tank
    {
        // Have threat, can't quickly lower it. 3 options remain: Stop attacking, lowlevel damage (wand), keep on keeping on.
        if (newTarget->GetHealthPercent() > 25)
        {
            // If elite
            if (m_ai.IsElite(newTarget))
            {
                // let warlock pet handle it to win some time
                // no need to check if newTarget can be a player because IsElite() returns false for players
                auto* creature = (Creature*) newTarget;
                if (pet)
                {
                    switch (pet->GetEntry())
                    {
                        // taunt the elite and tank it
                        case DEMON_VOIDWALKER:
                            if (TORMENT && m_ai.CastPetSpell(TORMENT, newTarget) == SPELL_CAST_OK)
                                return RETURN_NO_ACTION_OK;
                        // maybe give it some love?
                        case DEMON_SUCCUBUS:
                            if (creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID)
                                if (SEDUCTION && !newTarget->HasAura(SEDUCTION) && m_ai.CastPetSpell(SEDUCTION, newTarget) == SPELL_CAST_OK)
                                    return RETURN_NO_ACTION_OK;
                    }

                }
                // if aggroed mob is a demon or an elemental: banish it
                if (creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_DEMON || creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_ELEMENTAL)
                {
                    if (BANISH && !newTarget->HasAura(BANISH) && CastSpell(BANISH, newTarget))
                        return RETURN_CONTINUE;
                }

                return RETURN_NO_ACTION_OK; // do nothing and pray tank gets aggro off you
            }

            // Not an elite. You could insert FEAR here but in any PvE situation that's 90-95% likely
            // to worsen the situation for the group. ... So please don't.
            return CastSpell(SHOOT, pTarget);
        }
    }

    // Create soul shard (only on non-worldboss)
    uint8 freeSpace = m_ai.GetFreeBagSpace();
    uint8 HPThreshold = (m_ai.IsElite(pTarget) ? 10 : 25);
    if (!m_ai.IsElite(pTarget, true) && pTarget->GetHealthPercent() < HPThreshold && (shardCount < MAX_SHARD_COUNT && freeSpace > 0))
    {
        if (SHADOWBURN && m_ai.In_Reach(pTarget, SHADOWBURN) && !pTarget->HasAura(SHADOWBURN) && m_bot.IsSpellReady(SHADOWBURN) && CastSpell(SHADOWBURN, pTarget))
            return RETURN_CONTINUE;

        // Do not cast Drain Soul if Shadowburn is active on target
        if (DRAIN_SOUL && m_ai.In_Reach(pTarget, DRAIN_SOUL) && !pTarget->HasAura(DRAIN_SOUL) && !pTarget->HasAura(SHADOWBURN) && CastSpell(DRAIN_SOUL, pTarget))
        {
            m_ai.SetIgnoreUpdateTime(15);
            return RETURN_CONTINUE;
        }
    }

    if (pet && DARK_PACT && (100 * pet->GetPower(POWER_MANA) / pet->GetMaxPower(POWER_MANA)) > 10 && m_ai.GetManaPercent() <= 20)
        if (m_ai.CastSpell(DARK_PACT, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;

    // Mana check and replenishment
    if (LIFE_TAP && m_ai.GetManaPercent() <= 20 && m_ai.GetHealthPercent() > 50)
        if (m_ai.CastSpell(LIFE_TAP, m_bot) == SPELL_CAST_OK)
            return RETURN_CONTINUE;

    // HP, mana and aggro checks done
    // Curse the target
    if (CheckCurse(pTarget))
        return RETURN_CONTINUE;

    // Damage Spells
    if (spec)
    {
        switch (spec)
        {
            case WARLOCK_SPEC_AFFLICTION:
                if (CORRUPTION && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                    return RETURN_CONTINUE;
                if (IMMOLATE && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                    return RETURN_CONTINUE;
                if (SIPHON_LIFE > 0 && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SIPHON_LIFE) && !pTarget->HasAura(SIPHON_LIFE) && CastSpell(SIPHON_LIFE, pTarget))
                    return RETURN_CONTINUE;
                break;

            case WARLOCK_SPEC_DEMONOLOGY:
                if (CORRUPTION && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                    return RETURN_CONTINUE;
                if (IMMOLATE && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                    return RETURN_CONTINUE;
                break;

            case WARLOCK_SPEC_DESTRUCTION:
                if (SHADOWBURN && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && pTarget->GetHealthPercent() < (HPThreshold / 2.0) && m_ai.In_Reach(pTarget, SHADOWBURN) && !pTarget->HasAura(SHADOWBURN) && CastSpell(SHADOWBURN, pTarget))
                    return RETURN_CONTINUE;
                if (CORRUPTION && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
                    return RETURN_CONTINUE;
                if (IMMOLATE && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
                    return RETURN_CONTINUE;
                if (CONFLAGRATE && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, CONFLAGRATE) && pTarget->HasAura(IMMOLATE) && m_bot.IsSpellReady(CONFLAGRATE) && CastSpell(CONFLAGRATE, pTarget))
                    return RETURN_CONTINUE;
                break;
        }

        // Shadow bolt is common to all specs
        if (SHADOW_BOLT && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SHADOW_BOLT) && CastSpell(SHADOW_BOLT, pTarget))
            return RETURN_CONTINUE;

        // Default: shoot with wand
        return CastSpell(SHOOT, pTarget);
    }

    // No spec due to low level OR no spell found yet
    if (CORRUPTION && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, CORRUPTION) && !pTarget->HasAura(CORRUPTION) && CastSpell(CORRUPTION, pTarget))
        return RETURN_CONTINUE;
    if (IMMOLATE && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_FIRE) && m_ai.In_Reach(pTarget, IMMOLATE) && !pTarget->HasAura(IMMOLATE) && CastSpell(IMMOLATE, pTarget))
        return RETURN_CONTINUE;
    if (SHADOW_BOLT && !PlayerbotAI::IsImmuneToSchool(pTarget, SPELL_SCHOOL_MASK_SHADOW) && m_ai.In_Reach(pTarget, SHADOW_BOLT))
        return CastSpell(SHADOW_BOLT, pTarget);

    // Default: shoot with wand
    return CastSpell(SHOOT, pTarget);
} // end DoNextCombatManeuver

CombatManeuverReturns PlayerbotWarlockAI::DoNextCombatManeuverPVP(Unit* pTarget)
{
    if (FEAR && m_ai.In_Reach(pTarget, FEAR) && m_ai.CastSpell(FEAR, *pTarget) == SPELL_CAST_OK)
        return RETURN_CONTINUE;
    if (SHADOW_BOLT && m_ai.In_Reach(pTarget, SHADOW_BOLT) && m_ai.CastSpell(SHADOW_BOLT) == SPELL_CAST_OK)
        return RETURN_CONTINUE;

    return DoNextCombatManeuverPVE(pTarget); // TODO: bad idea perhaps, but better than the alternative
}

// Decision tree for putting a curse on the current target
bool PlayerbotWarlockAI::CheckCurse(Unit* target)
{
    uint32 curseToCast = 0;
    if (target->GetTypeId() == TYPEID_UNIT)
    {
        // Target is immune to shadow, no need to apply a curse
        if (PlayerbotAI::IsImmuneToSchool(target, SPELL_SCHOOL_MASK_SHADOW))
            return false;

        auto* creature = (Creature*) target;
        // Prevent low health humanoid from fleeing or fleeing too fast
        // Curse of Exhaustion first to avoid increasing damage output on tank
        if (creature->GetCreatureInfo()->CreatureType == CREATURE_TYPE_HUMANOID && target->GetHealthPercent() < 20 && !creature->IsWorldBoss())
        {
            if (CURSE_OF_EXHAUSTION && m_ai.In_Reach(target, CURSE_OF_EXHAUSTION) && !target->HasAura(CURSE_OF_EXHAUSTION))
            {
                if (AMPLIFY_CURSE && m_bot.IsSpellReady(AMPLIFY_CURSE))
                    CastSpell(AMPLIFY_CURSE, &m_bot);

                if (CastSpell(CURSE_OF_EXHAUSTION, target))
                {
                    m_CurrentCurse = CURSE_OF_EXHAUSTION;
                    return true;
                }
            }
            else if (CURSE_OF_RECKLESSNESS && m_ai.In_Reach(target, CURSE_OF_RECKLESSNESS) && !target->HasAura(CURSE_OF_RECKLESSNESS) && !target->HasAura(CURSE_OF_EXHAUSTION) && CastSpell(CURSE_OF_RECKLESSNESS, target))
            {
                m_CurrentCurse = CURSE_OF_RECKLESSNESS;
                return true;
            }
        }
    }

    // If bot already put a curse and curse is still active on target: no need to go further
    if (m_CurrentCurse > 0 && target->HasAura(m_CurrentCurse))
        return false;

    // No curse or effect worn off: choose again which curse to use

    // Target is a boss and bot is part of a group
    if (m_ai.IsElite(target, true) && m_bot.GetGroup())
    {
        uint8 mages = 0;
        uint8 warlocks = 1;
        Group::MemberSlotList const& groupSlot = m_bot.GetGroup()->GetMemberSlots();
        for (auto itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
        {
            Player* groupMember = sObjectMgr.GetPlayer(itr->guid);
            if (!groupMember || !groupMember->IsAlive())
                continue;
            switch (groupMember->getClass())
            {
                case CLASS_WARLOCK:
                    warlocks++;
                    continue;
                case CLASS_MAGE:
                    mages++;
                    continue;
            }
        }
        if (warlocks > 1 && warlocks > mages)
            curseToCast = CURSE_OF_SHADOW;
        else if (mages > warlocks)
            curseToCast = CURSE_OF_THE_ELEMENTS;
        else
            curseToCast = CURSE_OF_AGONY;
    }
    // Enemy elite mages have low health but can cast dangerous spells: group safety before bot DPS OR if facing mana user player
    else if ((target->GetTypeId() == TYPEID_UNIT && ((Creature*)target)->GetCreatureInfo()->UnitClass == 8) ||
             (target->GetTypeId() == TYPEID_PLAYER && ((Player*)target)->GetPowerType() == POWER_MANA))
        curseToCast = CURSE_OF_TONGUES;

    // Try to curse the target with the selected curse
    if (curseToCast && m_ai.In_Reach(target, curseToCast) && !target->HasAura(curseToCast))
    {
        if (curseToCast == CURSE_OF_AGONY)
        {
            if (AMPLIFY_CURSE && m_bot.IsSpellReady(AMPLIFY_CURSE))
                CastSpell(AMPLIFY_CURSE, &m_bot);
        }

        if (CastSpell(curseToCast, target))
        {
            m_CurrentCurse = curseToCast;
            return true;
        }
    }
    // else: go for Curse of Agony: in the long run: go for direct damage
    else if (CURSE_OF_AGONY && m_ai.In_Reach(target, CURSE_OF_AGONY) && !target->HasAura(CURSE_OF_AGONY))
    {
        if (AMPLIFY_CURSE && m_bot.IsSpellReady(AMPLIFY_CURSE))
            CastSpell(AMPLIFY_CURSE, &m_bot);

        if (CastSpell(CURSE_OF_AGONY, target))
        {
            m_CurrentCurse = CURSE_OF_AGONY;
            return true;
        }
    }
    // else: go for Curse of Weakness
    else if (CURSE_OF_WEAKNESS && !target->HasAura(CURSE_OF_WEAKNESS) && !target->HasAura(CURSE_OF_AGONY))
    {
        if (AMPLIFY_CURSE && m_bot.IsSpellReady(AMPLIFY_CURSE))
            CastSpell(AMPLIFY_CURSE, &m_bot);

        if (CastSpell(CURSE_OF_WEAKNESS, target))
        {
            m_CurrentCurse = CURSE_OF_WEAKNESS;
            return true;
        }
    }
    return false;
}

void PlayerbotWarlockAI::CheckDemon()
{
    uint32 spec = m_bot.GetSpec();
    uint8 shardCount = m_bot.GetItemCount(SOUL_SHARD, false, nullptr);
    Pet* pet = m_bot.GetPet();
    uint32 demonOfChoice;

    // If pet other than imp is active: return
    if (pet && pet->GetEntry() != DEMON_IMP)
        return;

    // Assign demon of choice based on spec
    if (spec == WARLOCK_SPEC_AFFLICTION)
        demonOfChoice = DEMON_FELHUNTER;
    else if (spec == WARLOCK_SPEC_DEMONOLOGY)
        demonOfChoice = DEMON_SUCCUBUS;
    else    // Destruction spec or no spec found
        demonOfChoice = DEMON_IMP;

    // Summon demon
    if (!pet || m_isTempImp)
    {
        uint32 summonSpellId;
        if (demonOfChoice != DEMON_IMP && shardCount > 0)
        {
            switch (demonOfChoice)
            {
                case DEMON_VOIDWALKER:
                    summonSpellId = SUMMON_VOIDWALKER;
                    break;

                case DEMON_FELHUNTER:
                    summonSpellId = SUMMON_FELHUNTER;
                    break;

                case DEMON_SUCCUBUS:
                    summonSpellId = SUMMON_SUCCUBUS;
                    break;

                default:
                    summonSpellId = 0;
            }

            if (summonSpellId && m_ai.CastSpell(summonSpellId) == SPELL_CAST_OK)
            {
                //m_ai.TellMaster("Summoning favorite demon...");
                m_isTempImp = false;
                return;
            }
        }

        if (!pet && SUMMON_IMP && m_ai.CastSpell(SUMMON_IMP) == SPELL_CAST_OK)
        {
            m_isTempImp = demonOfChoice != DEMON_IMP;

            //m_ai.TellMaster("Summoning Imp...");
            return;
        }
    }
}

void PlayerbotWarlockAI::DoNonCombatActions()
{
    //uint32 spec = m_bot.GetSpec();
    Pet* pet = m_bot.GetPet();

    // Initialize pet spells
    if (pet && pet->GetEntry() != m_lastDemon)
    {
        switch (pet->GetEntry())
        {
            case DEMON_IMP:
                BLOOD_PACT       = m_ai.initPetSpell(BLOOD_PACT_ICON);
                FIREBOLT         = m_ai.initPetSpell(FIREBOLT_ICON);
                FIRE_SHIELD      = m_ai.initPetSpell(FIRE_SHIELD_ICON);
                break;

            case DEMON_VOIDWALKER:
                CONSUME_SHADOWS  = m_ai.initPetSpell(CONSUME_SHADOWS_ICON);
                SACRIFICE        = m_ai.initPetSpell(SACRIFICE_ICON);
                SUFFERING        = m_ai.initPetSpell(SUFFERING_ICON);
                TORMENT          = m_ai.initPetSpell(TORMENT_ICON);
                break;

            case DEMON_SUCCUBUS:
                LASH_OF_PAIN     = m_ai.initPetSpell(LASH_OF_PAIN_ICON);
                SEDUCTION        = m_ai.initPetSpell(SEDUCTION_ICON);
                SOOTHING_KISS    = m_ai.initPetSpell(SOOTHING_KISS_ICON);
                break;

            case DEMON_FELHUNTER:
                DEVOUR_MAGIC     = m_ai.initPetSpell(DEVOUR_MAGIC_ICON);
                SPELL_LOCK       = m_ai.initPetSpell(SPELL_LOCK_ICON);
                break;
        }

        m_lastDemon = pet->GetEntry();
    }

    // Destroy extra soul shards
    uint8 shardCount = m_bot.GetItemCount(SOUL_SHARD, false, nullptr);
    uint8 freeSpace = m_ai.GetFreeBagSpace();
    if (shardCount > MAX_SHARD_COUNT || (freeSpace == 0 && shardCount > 1))
        m_bot.DestroyItemCount(SOUL_SHARD, shardCount > MAX_SHARD_COUNT ? shardCount - MAX_SHARD_COUNT : 1, true, false);

    // buff myself DEMON_SKIN, DEMON_ARMOR, FEL_ARMOR - Strongest one available is chosen
    if (DEMON_ARMOR)
    {
        if (m_ai.SelfBuff(DEMON_ARMOR) == SPELL_CAST_OK)
            return;
    }
    else if (DEMON_SKIN)
        if (m_ai.SelfBuff(DEMON_SKIN) == SPELL_CAST_OK)
            return;

    // healthstone creation
    if (CREATE_HEALTHSTONE && shardCount > 0)
    {
        Item* const healthStone = m_ai.FindConsumable(HEALTHSTONE_DISPLAYID);
        if (!healthStone && m_ai.CastSpell(CREATE_HEALTHSTONE) == SPELL_CAST_OK)
            return;
    }

    // soulstone creation and use
    if (CREATE_SOULSTONE)
    {
        Item* soulStone = m_ai.FindConsumable(SOULSTONE_DISPLAYID);
        if (!soulStone)
        {
            // try create one only if there is place on bags
            if (m_ai.CanStore())
            {
                if (shardCount > 0 && m_bot.IsSpellReady(CREATE_SOULSTONE) && m_ai.CastSpell(CREATE_SOULSTONE) == SPELL_CAST_OK)
                    return;
            }
        }
        else
        {
            uint32 soulStoneSpell = soulStone->GetProto()->Spells[0].SpellId;
            if (!m_master.HasAura(soulStoneSpell) && m_bot.IsSpellReady(soulStoneSpell))
            {
                // TODO: first choice: healer. Second choice: anyone else with revive spell. Third choice: self or master.
                m_ai.UseItem(soulStone, &m_master);
                return;
            }
        }
    }

    // hp/mana check
    if (pet && DARK_PACT && (100 * pet->GetPower(POWER_MANA) / pet->GetMaxPower(POWER_MANA)) > 40 && m_ai.GetManaPercent() <= 60)
        if (m_ai.CastSpell(DARK_PACT, m_bot) == SPELL_CAST_OK)
            return;

    if (LIFE_TAP && m_ai.GetManaPercent() <= 80 && m_ai.GetHealthPercent() > 50)
        if (m_ai.CastSpell(LIFE_TAP, m_bot) == SPELL_CAST_OK)
            return;

    // Do not waste time/soul shards to create spellstone or firestone
    // if two-handed weapon (staff) or off-hand item are already equiped
    // Spellstone creation and use (Spellstone dominates firestone completely as I understand it)
    Item* const weapon = m_bot.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    Item* const offweapon = m_bot.GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (weapon && !offweapon && weapon->GetProto()->SubClass != ITEM_SUBCLASS_WEAPON_STAFF && weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT) == 0)
    {
        Item* const stone = m_ai.FindConsumable(SPELLSTONE_DISPLAYID);
        Item* const stone2 = m_ai.FindConsumable(FIRESTONE_DISPLAYID);
        uint8 spellstone_count = m_bot.GetItemCount(SPELLSTONE, false, nullptr);
        if (spellstone_count == 0)
            spellstone_count = m_bot.GetItemCount(GREATER_SPELLSTONE, false, nullptr);
        if (spellstone_count == 0)
            spellstone_count = m_bot.GetItemCount(MAJOR_SPELLSTONE, false, nullptr);
        uint8 firestone_count = m_bot.GetItemCount(LESSER_FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot.GetItemCount(FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot.GetItemCount(GREATER_FIRESTONE, false, nullptr);
        if (firestone_count == 0)
            firestone_count = m_bot.GetItemCount(MAJOR_FIRESTONE, false, nullptr);
        if (spellstone_count == 0 && firestone_count == 0)
        {
            if ((CREATE_SPELLSTONE && shardCount > 0 && m_ai.CastSpell(CREATE_SPELLSTONE) == SPELL_CAST_OK) ||
                (CREATE_SPELLSTONE == 0 && CREATE_FIRESTONE > 0 && shardCount > 0 && m_ai.CastSpell(CREATE_FIRESTONE) == SPELL_CAST_OK))
                return;
        }
        else if (stone)
        {
            m_ai.UseItem(stone, EQUIPMENT_SLOT_OFFHAND);
            return;
        }
        else
        {
            m_ai.UseItem(stone2, EQUIPMENT_SLOT_OFFHAND);
            return;
        }
    }

    if (EatDrinkBandage())
        return;

    //Heal Voidwalker
    if (pet && pet->GetEntry() == DEMON_VOIDWALKER && CONSUME_SHADOWS && pet->GetHealthPercent() < 75 && !pet->HasAura(CONSUME_SHADOWS))
        m_ai.CastPetSpell(CONSUME_SHADOWS);

    CheckDemon();

    // Soul link demon
    if (pet && SOUL_LINK && !m_bot.HasAura(SOUL_LINK_AURA) && m_ai.CastSpell(SOUL_LINK, m_bot) == SPELL_CAST_OK)
        return;

    // Check demon buffs
    if (pet && pet->GetEntry() == DEMON_IMP && BLOOD_PACT && !m_bot.HasAura(BLOOD_PACT) && m_ai.CastPetSpell(BLOOD_PACT) == SPELL_CAST_OK)
        return;
} // end DoNonCombatActions

// Return to UpdateAI the spellId usable to neutralize a target with creaturetype
uint32 PlayerbotWarlockAI::Neutralize(uint8 creatureType)
{
    if (!creatureType)  return 0;

    // TODO: add a way to handle spell cast by pet like Seduction
    if (creatureType != CREATURE_TYPE_DEMON && creatureType != CREATURE_TYPE_ELEMENTAL)
    {
        m_ai.TellMaster("I can't banish that target.");
        return 0;
    }

    if (BANISH)
        return BANISH;
    else
        return 0;
}
