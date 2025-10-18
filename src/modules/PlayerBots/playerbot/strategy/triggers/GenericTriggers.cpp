
#include "playerbot/playerbot.h"
#include "GenericTriggers.h"
#include "playerbot/LootObjectStack.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/strategy/values/AoeValues.h"

#include <regex>

using namespace ai;

bool NoManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < 5;
}

bool LowManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.lowMana;
}

bool MediumManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.mediumMana;
}

bool HighManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") < 65;
}

bool AlmostFullManaTrigger::IsActive()
{
    return AI_VALUE2(bool, "has mana", "self target") && AI_VALUE2(uint8, "mana", "self target") > 85;
}

bool RageAvailable::IsActive()
{
    return AI_VALUE2(uint8, "rage", "self target") >= amount;
}

bool EnergyAvailable::IsActive()
{
	return AI_VALUE2(uint8, "energy", "self target") >= amount;
}

bool ComboPointsAvailableTrigger::IsActive()
{
    return AI_VALUE2(uint8, "combo", "current target") >= amount;
}

bool LoseAggroTrigger::IsActive()
{
    if(!AI_VALUE2(bool, "has aggro", "current target"))
    {
        // Check if the aggro has been taken by another tank
        if(ai->IsTank(bot))
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if(target && !target->IsPlayer())
            {
                Unit* targetsTarget = target->GetVictim();
                if(targetsTarget && targetsTarget->IsPlayer())
                {
                    Player* targetsPlayerTarget = (Player*)targetsTarget;
                    return !ai->IsTank(targetsPlayerTarget);
                }
            }
        }

        return true;
    }

    return false;
}

bool HasAggroTrigger::IsActive()
{
    return AI_VALUE2(bool, "has aggro", "current target");
}

bool PanicTrigger::IsActive()
{
    return !ai->IsInPvp() &&
           AI_VALUE2(uint8, "health", "self target") < sPlayerbotAIConfig.criticalHealth &&
		   (!AI_VALUE2(bool, "has mana", "self target") || 
            AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig.lowMana);
}

bool OutNumberedTrigger::IsActive()
{
    // Don't trigger if the bot is a dungeon or raid
    if (!bot->IsInWorld() || bot->IsBeingTeleported() || bot->GetMap()->IsDungeon() || bot->GetMap()->IsRaid())
        return false;

    // Don't trigger if the bot is in a raid group
    if (bot->GetGroup() && bot->GetGroup()->IsRaidGroup())
        return false;

    // Don't trigger if the bot is in a group with a real player
    if (bot->GetGroup() && ai->HasRealPlayerMaster())
        return false;

    int32 botLevel = bot->GetLevel();
    float healthMod = bot->GetHealthPercent() / 100.0f;
    uint32 friendPower = 100 + 100 * healthMod, foePower = 0;
    for (auto &attacker : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get())
    {
        Creature* creature = ai->GetCreature(attacker);
        if (!creature)
            continue;

        if (!creature->CanAttackOnSight(bot))
            continue;

        int32 dLevel = creature->GetLevel() - botLevel;

        healthMod = creature->GetHealthPercent() / 100.0f;

        if(dLevel > -10)
            foePower += std::max(100 + 10 * dLevel, dLevel * 200) * healthMod;
    }

    if (!foePower)
        return false;

    for (auto & helper : ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get())
    {
        Unit* player = ai->GetUnit(helper);

        if (!player || player == bot)
            continue;

        int32 dLevel = player->GetLevel() - botLevel;

        healthMod = player->GetHealthPercent() / 100.0f;

        if (dLevel > -10 && sServerFacade.GetDistance2d(bot, player) < 10.0f)
            friendPower += std::max(200 + 20 * dLevel, dLevel * 200)* healthMod;
    }

    return friendPower < foePower;
}

bool BuffTrigger::IsActive()
{
    Unit* target = GetTarget();
	return target && !ai->HasAura(spell, target, false, checkIsOwner) && target->IsAlive();
}

bool MyBuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && !ai->HasMyAura(spell, target);
}

Value<Unit*>* BuffOnPartyTrigger::GetTargetValue()
{
    const std::string qualifier = spell + "-" + (ignoreTanks ? "1" : "0");
	return context->GetValue<Unit*>("friendly unit without aura", qualifier);
}

Value<Unit*>* GreaterBuffOnPartyTrigger::GetTargetValue()
{
    const std::string qualifier = spell + "-" + (ignoreTanks ? "1" : "0");
    return context->GetValue<Unit*>("party member without aura", qualifier);
}

Value<Unit*>* MyBuffOnPartyTrigger::GetTargetValue()
{
    const std::string qualifier = spell + "-" + (ignoreTanks ? "1" : "0");
    return context->GetValue<Unit*>("party member without my aura", qualifier);
}

ai::Value<Unit*>* BuffOnTankTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("party tank without aura", spell);
}

Value<Unit*>* DebuffOnAttackerTrigger::GetTargetValue()
{
	return context->GetValue<Unit*>("attacker without aura", spell);
}

bool NoAttackersTrigger::IsActive()
{
    return !AI_VALUE(Unit*, "current target") && AI_VALUE(uint8, "my attacker count") > 0;
}

bool InvalidTargetTrigger::IsActive()
{
    return AI_VALUE2(bool, "invalid target", "current target");
}

bool NoTargetTrigger::IsActive()
{
	return !AI_VALUE(Unit*, "current target") || AI_VALUE2(bool, "invalid target", "current target");
}

bool MyAttackerCountTrigger::IsActive()
{
    return AI_VALUE2(bool, "combat", "self target") && AI_VALUE(uint8, "my attacker count") >= amount;
}

bool HighThreatTrigger::IsActive()
{
    uint8 relativeThreat = AI_VALUE2(uint8, "threat", "current target");

    if (relativeThreat >= 80)
    {
        //Watch delta.
        uint32 lastTime = MEM_AI_VALUE(float, "my threat::current target")->GetLastTime();

        if (lastTime < time(0))
        {
            float deltaThreat = LOG_AI_VALUE(float, "my threat::current target")->GetDelta(5.0f);
            float currentThreat = AI_VALUE2(float, "my threat", "current target");

            if (deltaThreat > 0)
            {
                float tankThreat = AI_VALUE2(float, "tank threat", "current target");

                float newThreat = currentThreat + deltaThreat * 5.0f; //No agro in 5 seconds.

                if (newThreat < tankThreat)
                    return false;
            }
        }

        return true;
    }

    return false;
}

bool MediumThreatTrigger::IsActive()
{
    if (AI_VALUE2(uint8, "threat", "current target") >= 60)
        return true;

    return false;
}

bool SomeThreatTrigger::IsActive()
{
    if (AI_VALUE2(uint8, "threat", "current target") >= 25)
        return true;

    return false;
}

bool NoThreatTrigger::IsActive()
{
    if (SomeThreatTrigger::IsActive())
        return false;

    return true;
}

bool AoeTrigger::IsActive()
{
    std::list<ObjectGuid> aoeEnemies = AoeCountValue::FindMaxDensity(bot, range);
    return aoeEnemies.size() >= amount;
}

bool DebuffTrigger::IsActive()
{
    Unit* target = GetTarget();
    if(target && target->IsAlive())
    {
        if (!ai->HasAura(spell, target, false, checkIsOwner))
        {
            return !HasMaxDebuffs();
        }
    }

    return false;
}

bool DebuffTrigger::HasMaxDebuffs()
{
#ifdef MANGOSBOT_TWO
    return false;
#else
    Unit* target = GetTarget();
    if(target)
    {
#ifdef MANGOSBOT_ONE
        constexpr uint32 debuffLimit = 40;
#else
        constexpr uint32 debuffLimit = 16;
#endif

        return ai->GetAuras(target, false, false).size() >= debuffLimit;
    }
#endif

    return false;
}

bool SpellTrigger::IsActive()
{
	return GetTarget();
}

bool SpellCanBeCastedTrigger::IsActive()
{
	Unit* target = GetTarget();
	return target && ai->CanCastSpell(spell, target, true);
}

bool SpellNoCooldownTrigger::IsActive()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", name);
    if (!spellId)
        return false;

    return bot->IsSpellReady(spellId);
}

bool RandomTrigger::IsActive()
{
    if (time(0) - lastCheck < sPlayerbotAIConfig.repeatDelay / 1000)
        return false;

    lastCheck = time(0);
    int k = (int)(probability / sPlayerbotAIConfig.randomChangeMultiplier);
    if (k < 1) k = 1;
    return (rand() % k) == 0;
}

bool AndTrigger::IsActive()
{
    std::vector<std::string> tnames = getMultiQualifiers(getQualifier(), ",");

    for (auto tname : tnames)
    {
        Trigger* trigger1 = ai->GetAiObjectContext()->GetTrigger(tname);
        if (!trigger1 || !trigger1->IsActive())
            return false;
    }

    return true;
}

std::string AndTrigger::getName()
{
    std::string name;
    std::vector<std::string> tnames = getMultiQualifiers(getQualifier(), ",");

    for (auto tname : tnames)
    {
        if (!name.empty())
            name += " and ";
        name += tname;
    }

    return name;
}

bool OrTrigger::IsActive()
{
    std::vector<std::string> tnames = getMultiQualifiers(getQualifier(), ",");

    for (auto tname : tnames)
    {
        Trigger* trigger1 = ai->GetAiObjectContext()->GetTrigger(tname);
        if (trigger1 && trigger1->IsActive())
            return true;
    }

    return false;
}

std::string OrTrigger::getName()
{
    std::string name;
    std::vector<std::string> tnames = getMultiQualifiers(getQualifier(), ",");

    for (auto tname : tnames)
    {
        if (!name.empty())
            name += " or ";
        name += tname;
    }

    return name;
}

bool TwoTriggers::IsActive()
{
    if (name1.empty() || name2.empty())
        return false;

    Trigger* trigger1 = ai->GetAiObjectContext()->GetTrigger(name1);
    Trigger* trigger2 = ai->GetAiObjectContext()->GetTrigger(name2);

    if (!trigger1 || !trigger2)
        return false;

    return trigger1->IsActive() && trigger2->IsActive();
}

std::string TwoTriggers::getName()
{
    std::string name;
    name = name1 + " and " + name2;
    return name;
}

bool BoostTrigger::IsActive()
{
    if (ai->IsStateActive(BotState::BOT_STATE_COMBAT) && BuffTrigger::IsActive())
    {
        if (!ai->HasRealPlayerMaster())
        {
            return AI_VALUE(uint8, "balance") <= balance;
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool ItemCountTrigger::IsActive()
{
	return AI_VALUE2(uint32, "item count", item) < uint32(count);
}

bool InterruptSpellTrigger::IsActive()
{
	return SpellTrigger::IsActive() && ai->IsInterruptableSpellCasting(GetTarget(), getName(), true);
}

bool DeflectSpellTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsNonMeleeSpellCasted(true))
        return false;

    if (!target->HasTarget(bot->GetObjectGuid()))
        return false;

    uint32 spellid = context->GetValue<uint32>("spell id", spell)->Get();
    if (!spellid)
        return false;

    SpellEntry const *deflectSpell = sServerFacade.LookupSpellInfo(spellid);
    if (!deflectSpell)
        return false;

    // warrior deflects all
    if (spell == "spell reflection")
        return true;

    // human priest feedback
    if (spell == "feedback")
        return true;

    SpellSchoolMask deflectSchool = SpellSchoolMask(deflectSpell->EffectMiscValue[0]);
    SpellSchoolMask attackSchool = SPELL_SCHOOL_MASK_NONE;

    Spell* spell = target->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    if (spell)
    {
        SpellEntry const* tarSpellInfo = spell->m_spellInfo;
        if (tarSpellInfo)
        {
            attackSchool = GetSpellSchoolMask(tarSpellInfo);
            if (deflectSchool == attackSchool)
                return true;
        }
    }
    return false;
}

bool HasAuraTrigger::IsActive()
{
   if (!name.empty())
	{
      return ai->HasAura(name, GetTarget(), false, false, -1, false, 0, auraTypeId);
   }

   std::string str = getQualifier();
   std::regex pattern(R"(spellid::(\d+)::([^:]*)::(\d+))");
   std::smatch match;

   if (std::regex_search(str, match, pattern) && match.size() == 4)
   {
      uint32 spell_id = atoi(match[1].str().c_str());

      if (Aura* aura = ai->GetAura(spell_id, GetTarget()))
      {
         uint32 count = atoi(match[3].str().c_str());
         uint32 stack_size = aura->GetStackAmount();
         std::string comp_symb = match[2].str();

         if (comp_symb == "equal")
         {
            return stack_size == count;
         }
         else if (comp_symb == "greater or equal")
         {
            return stack_size >= count;
         }
         else if (comp_symb == "lesser or equal")
         {
            return stack_size <= count;
         }
         else if (comp_symb == "greater")
         {
            return stack_size > count;
         }
         else if (comp_symb == "lesser")
         {
            return stack_size < count;
         }
         else
         {
            return false;
         }
      }
   };

   pattern = R"(spellid::(\d+))";

   if (std::regex_search(str, match, pattern) && match.size() == 2)
   {

      uint32 spell_id = atoi(match[1].str().c_str());
      return ai->HasAura(spell_id, GetTarget());
   }

   return false;
}

std::string HasAuraTrigger::getName()
{
   if (!name.empty())
   {
      return name;
   }

   std::ostringstream ss;
   ss << "has aura with " << getQualifier();
   return ss.str();
}


bool HasNoAuraTrigger::IsActive()
{
    return !ai->HasAura(getName(), GetTarget());
}

bool TankAssistTrigger::IsActive()
{
    if (!AI_VALUE(bool, "has attackers"))
        return false;

    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    if (!currentTarget)
        return true;

    // do not switch if enemy target
    Unit* enemy = AI_VALUE(Unit*, "enemy player target");
    if (enemy)
    {
        return currentTarget != enemy;
    }

    Unit* tankTarget = AI_VALUE(Unit*, "tank target");
    if (!tankTarget || currentTarget == tankTarget)
        return false;
#ifdef CMANGOS
    return tankTarget->GetVictim() != AI_VALUE(Unit*, "self target");
#endif
#ifdef MANGOS
    return tankTarget->getVictim() != AI_VALUE(Unit*, "self target");
#endif
}

bool IsBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && AI_VALUE2(bool, "behind", "current target");
}

bool IsNotBehindTargetTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return target && !AI_VALUE2(bool, "behind", "current target");
}

bool IsNotFacingTargetTrigger::IsActive()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool HasCcTargetTrigger::IsActive()
{
    uint32 spellid = AI_VALUE2(uint32, "spell id", getName());
    if (spellid && sServerFacade.IsSpellReady(bot, spellid))
    {
        return AI_VALUE2(Unit*, "cc target", getName()) && !AI_VALUE2(Unit*, "current cc target", getName());
    }

    return false;
}

bool NoMovementTrigger::IsActive()
{
	return !AI_VALUE2(bool, "moving", "self target");
}

bool NoPossibleTargetsTrigger::IsActive()
{
    std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, "possible targets");
    return !targets.size();
}

bool PossibleAddsTrigger::IsActive()
{
    return AI_VALUE(bool, "possible adds") && !AI_VALUE(ObjectGuid, "attack target");
}

bool NotDpsTargetActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
    {
        if (target->IsPlayer())
        {
            return false;
        }

        if(sServerFacade.IsAlive(target))
        {
            // do not switch if enemy target
            Unit* enemy = AI_VALUE(Unit*, "enemy player target");
            if (enemy)
            {
                return target != enemy;
            }

            Unit* dps = AI_VALUE(Unit*, "dps target");
            if (dps)
            {
                return target != dps;
            }
        }
    }

    return false;
}

bool NotDpsAoeTargetActiveTrigger::IsActive()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target)
    {
        if (target->IsPlayer())
        {
            return false;
        }

        if (sServerFacade.IsAlive(target))
        {
            // do not switch if enemy target
            Unit* enemy = AI_VALUE(Unit*, "enemy player target");
            if (enemy)
            {
                return target != enemy;
            }

            Unit* dps = AI_VALUE(Unit*, "dps aoe target");
            if (dps)
            {
                return target != dps;
            }
        }
    }

    return false;
}

bool IsSwimmingTrigger::IsActive()
{
    return AI_VALUE2(bool, "swimming", "self target");
}

bool HasNearestAddsTrigger::IsActive()
{
    std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, "nearest adds");
    return targets.size();
}

bool HasItemForSpellTrigger::IsActive()
{
    std::string spell = getName();
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && AI_VALUE2(Item*, "item for spell", spellId);
}


bool TargetChangedTrigger::IsActive()
{
    Unit* oldTarget = context->GetValue<Unit*>("old target")->Get();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    return target && oldTarget != target;
}

Value<Unit*>* InterruptEnemyHealerTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("enemy healer target", spell);
}

Value<Unit*>* SnareTargetTrigger::GetTargetValue()
{
    return context->GetValue<Unit*>("snare target", spell);
}

bool StayTimeTrigger::IsActive()
{
    time_t stayTime = AI_VALUE(time_t, "stay time");
    time_t now = time(0);
    return delay && stayTime && now > stayTime + 2 * delay / 1000;
}

bool IsMountedTrigger::IsActive()
{
    return AI_VALUE2(bool, "mounted", "self target");
}

bool CorpseNearTrigger::IsActive()
{
    return bot->GetCorpse() && bot->GetCorpse()->IsWithinDistInMap(bot, CORPSE_RECLAIM_RADIUS, true);
}

bool IsFallingTrigger::IsActive()
{
#ifndef MANGOSBOT_TWO
    return bot->HasMovementFlag(MOVEFLAG_JUMPING);
#else
    return bot->HasMovementFlag(MOVEFLAG_FALLING);
#endif
}

bool IsFallingFarTrigger::IsActive()
{
    return bot->HasMovementFlag(MOVEFLAG_FALLINGFAR);
}

bool HasAreaDebuffTrigger::IsActive()
{
    return AI_VALUE2(bool, "has area debuff", "self target");
}

bool ReturnToStayPositionTrigger::IsActive()
{
    PositionEntry stayPosition = AI_VALUE(PositionMap&, "position")["stay"];
    if (stayPosition.isSet())
    {
        const float distance = bot->GetDistance(stayPosition.x, stayPosition.y, stayPosition.z);
        return distance > ai->GetRange("follow");
    }

    return false;
}

bool ReturnToPullPositionTrigger::IsActive()
{
    PositionEntry pullPosition = AI_VALUE(PositionMap&, "position")["pull"];
    if (pullPosition.isSet())
    {
        const float distance = bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z);
        return distance > ai->GetRange("follow");
    }

    return false;
}

bool NoBuffAndComboPointsAvailableTrigger::IsActive()
{
    if (BuffTrigger::IsActive())
    {
        return AI_VALUE2(uint8, "combo", GetComboPointsTargetName()) >= comboPoints;
    }

    return false;
}

bool InPvpTrigger::IsActive()
{
    return ai->IsInPvp();
}

bool InPveTrigger::IsActive()
{
    return ai->IsInPve();
}

bool InRaidFightTrigger::IsActive()
{
    return ai->IsInRaid();
}

bool GreaterBuffOnPartyTrigger::IsActive()
{
    Unit* target = GetTarget();
    return target && bot->IsInGroup(target) && BuffOnPartyTrigger::IsActive() && !ai->HasAura(lowerSpell, target, false, checkIsOwner);
}

bool TargetOfAttacker::IsActive()
{
    return !AI_VALUE(std::list<ObjectGuid>, "attackers targeting me").empty();
}

bool TargetOfAttackerInRange::IsActive()
{
    const Unit* closestAttacker = AI_VALUE(Unit*, "closest attacker targeting me");
    return closestAttacker && bot->GetDistance(closestAttacker, true, DIST_CALC_COMBAT_REACH) <= (distance - sPlayerbotAIConfig.contactDistance);
}

bool TargetOfCastedAuraTypeTrigger::IsActive()
{
    const std::list<ObjectGuid>& attackers = AI_VALUE(std::list<ObjectGuid>, "attackers");
    for (const ObjectGuid& attackerGuid : attackers)
    {
        // Check against the given creature id
        Unit* attacker = ai->GetUnit(attackerGuid);
        if (attacker)
        {
            const Spell* auraTypeSpell = nullptr;
            const Spell* genericSpell = attacker->GetCurrentSpell(CurrentSpellTypes::CURRENT_GENERIC_SPELL);
            if (genericSpell)
            {
                const SpellEntry* spellInfo = genericSpell->m_spellInfo;
                if (spellInfo)
                {
                    for (int32 i = SpellEffectIndex::EFFECT_INDEX_0; i <= SpellEffectIndex::EFFECT_INDEX_2; i++)
                    {
                        if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA) && (spellInfo->EffectApplyAuraName[i] == auraType))
                        {
                            auraTypeSpell = genericSpell;
                            break;
                        }
                    }
                }
            }

            if (!auraTypeSpell)
            {
                const Spell* channeledSpell = attacker->GetCurrentSpell(CurrentSpellTypes::CURRENT_CHANNELED_SPELL);
                if (channeledSpell)
                {
                    const SpellEntry* spellInfo = channeledSpell->m_spellInfo;
                    if (spellInfo)
                    {
                        for (int32 i = SpellEffectIndex::EFFECT_INDEX_0; i <= SpellEffectIndex::EFFECT_INDEX_2; i++)
                        {
                            if ((spellInfo->Effect[i] == SPELL_EFFECT_APPLY_AURA) && (spellInfo->EffectApplyAuraName[i] == auraType))
                            {
                                auraTypeSpell = channeledSpell;
                                break;
                            }
                        }
                    }
                }
            }

            if (auraTypeSpell)
            {
                Unit* spellTarget = auraTypeSpell->m_targets.getUnitTarget();
                if (spellTarget == bot)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool BuffOnTargetTrigger::IsActive()
{
    const Unit* target = GetTarget();
    return target && target->HasAura(buffID);
}

bool DispelOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (target)
    {
        const uint32 dispelMask = GetDispellMask(dispelType);
        const std::vector<Aura*> auras = ai->GetAuras(target);
        for (const Aura* aura : auras)
        {
            const SpellEntry* spellInfo = aura->GetSpellProto();
            if (spellInfo && ((1 << spellInfo->Dispel) & dispelMask))
            {
                return true;
            }
        }
    }

    return false;
}

bool RtscJumpTrigger::IsActive()
{
    return AI_VALUE2(WorldPosition, "RTSC saved location", "jump point") && AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
}

bool SpellTargetTrigger::IsActive()
{
    if (IsSpellReady())
    {
        // Check for assigned targets
        const std::list<ObjectGuid>& possibleTargets = AI_VALUE(std::list<ObjectGuid>, targetsValue);
        if (!possibleTargets.empty())
        {
            for (const ObjectGuid& possibleTargetGuid : possibleTargets)
            {
                if (IsTargetValid(ai->GetUnit(possibleTargetGuid)))
                {
                    return true;
                }
            }
        }
        else
        {
            // Check for the default target
            if (IsTargetValid(GetTarget()))
            {
                return true;
            }
        }
    }

    return false;
}

bool SpellTargetTrigger::IsTargetValid(Unit* target)
{
    return target &&
           ai->IsSafe(target) &&
           (bot == target || sServerFacade.GetDistance2d(bot, target) < sPlayerbotAIConfig.sightDistance) &&
           (bot->IsInGroup(target)) &&
           (!aliveCheck || !target->IsDead()) &&
           (!auraCheck || !ai->HasAura(spell, target));
}

bool SpellTargetTrigger::IsSpellReady()
{
    uint32 spellId = AI_VALUE2(uint32, "spell id", spell);
    return spellId && bot->IsSpellReady(spellId);
}

bool ItemTargetTrigger::IsTargetValid(Unit* target)
{
    if (SpellTargetTrigger::IsTargetValid(target))
    {
        if (itemAuraCheck)
        {
            const uint32 itemId = GetItemId();
            const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
            if (proto)
            {
                for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
                {
#ifdef MANGOSBOT_ZERO
                    if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE || proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
                    if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
#endif
                    {
                        if (proto->Spells[i].SpellId > 0 && ai->HasAura(proto->Spells[i].SpellId, target))
                        {
                            return false;
                        }
                    }
                }

                return true;
            }
        }
        else
        {
            return true;
        }
    }

    return false;
}

bool ItemTargetTrigger::IsSpellReady()
{
    const uint32 itemId = GetItemId();
    if (!ai->HasCheat(BotCheatMask::item) && !bot->HasItemCount(itemId, 1))
        return false;

    const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);
    if (proto)
    {
        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
#ifdef MANGOSBOT_ZERO
            if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE || proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
            if (proto->Spells[i].SpellTrigger == ITEM_SPELLTRIGGER_ON_USE)
#endif
            {
                if (proto->Spells[i].SpellId > 0)
                {
                    if (!sServerFacade.IsSpellReady(bot, proto->Spells[i].SpellId) ||
                        !sServerFacade.IsSpellReady(bot, proto->Spells[i].SpellId, itemId))
                    {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    return false;
}

bool AtWarTrigger::IsActive()
{
    ReputationMgr& mgr = bot->GetReputationMgr();

#ifndef MANGOSBOT_ONE
    for (uint32 id = 0; id < sFactionStore.GetNumRows(); ++id)
#else
    for (uint32 id = 0; id < sFactionStore.GetMaxEntry(); ++id)
#endif
    {
#ifndef MANGOSBOT_ONE
        const FactionEntry* factionEntry = sFactionStore.LookupEntry(id);
#else
        const FactionEntry* factionEntry = sFactionStore.LookupEntry<FactionEntry>(id);
#endif

        if (!factionEntry)
            continue;

        FactionState const* repState = mgr.GetState(factionEntry);

        if (!repState)
            continue;

        if (!(repState->Flags & FACTION_FLAG_VISIBLE))
            continue;

        if (repState->Flags & FACTION_FLAG_HIDDEN)
            continue;

        if (repState->Flags & FACTION_FLAG_INVISIBLE_FORCED)
            continue;

        if (repState->Flags & FACTION_FLAG_PEACE_FORCED)
            continue;

        if (!(repState->Flags & FACTION_FLAG_AT_WAR))
            continue;

        if (mgr.GetRank(factionEntry) < REP_HOSTILE)
            continue;

        return true;
    }

    return false;
}