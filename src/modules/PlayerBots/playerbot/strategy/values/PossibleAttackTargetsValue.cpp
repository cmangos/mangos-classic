
#include "playerbot/playerbot.h"
#include "PossibleAttackTargetsValue.h"
#include "PossibleTargetsValue.h"

#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "AttackersValue.h"
#include "EnemyPlayerValue.h"

using namespace ai;
using namespace MaNGOS;

std::list<ObjectGuid> PossibleAttackTargetsValue::Calculate()
{
    std::list<ObjectGuid> result;
    if (ai->AllowActivity(ALL_ACTIVITY))
    {
        if (bot->IsInWorld() && !bot->IsBeingTeleported())
        {
            // Check if we only need one possible attack target
            bool getOne = false;
            if (!qualifier.empty())
            {
                getOne = stoi(qualifier);
            }

            if (getOne)
            {
                // Try to get one possible attack target
                result = AI_VALUE2(std::list<ObjectGuid>, "attackers", 1);
                RemoveNonThreating(result, getOne);
            }

            // If the one possible attack target failed, retry with multiple attackers
            if (result.empty())
            {
                result = AI_VALUE(std::list<ObjectGuid>, "attackers");
                RemoveNonThreating(result, getOne);
            }
        }
    }

	return result;
}

void PossibleAttackTargetsValue::RemoveNonThreating(std::list<ObjectGuid>& targets, bool getOne)
{
    std::list<ObjectGuid> breakableCC;
    std::list<ObjectGuid> unBreakableCC;

    for(std::list<ObjectGuid>::iterator tIter = targets.begin(); tIter != targets.end();)
    {
        Unit* target = ai->GetUnit(*tIter);
        if (!IsValid(target, bot, sPlayerbotAIConfig.sightDistance, true, false))
        {
            std::list<ObjectGuid>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else if (!HasIgnoreCCRti(target, bot) && HasBreakableCC(target, bot))
        {
            breakableCC.push_back(*tIter);
            std::list<ObjectGuid>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else if (!HasIgnoreCCRti(target, bot) && HasUnBreakableCC(target, bot))
        {
            unBreakableCC.push_back(*tIter);
            std::list<ObjectGuid>::iterator tIter2 = tIter;
            ++tIter;
            targets.erase(tIter2);
        }
        else
        {
            if (getOne)
            {
                // If the target is valid return it straight away
                std::list<ObjectGuid> result = { *tIter };
                targets = result;
                break;
            }
            else
            {
                ++tIter;
            }
        }
    }

    if (targets.empty())
    {
        if (!unBreakableCC.empty())
        {
            targets = unBreakableCC;
        }
        else if(!breakableCC.empty())
        {
            targets = breakableCC;
        }
    }
}

bool PossibleAttackTargetsValue::HasIgnoreCCRti(Unit* target, Player* player)
{
    Group* group = player->GetGroup();
    return group && (group->GetTargetIcon(7) == target->GetObjectGuid());
}

bool PossibleAttackTargetsValue::HasBreakableCC(Unit* target, Player* player)
{
    if (target->IsPolymorphed())
    {
        return true;
    }

    if (sServerFacade.IsFrozen(target))
    {
        return true;
    }

    PlayerbotAI* ai = player->GetPlayerbotAI();
    if (ai)
    {
        if (ai->HasAura("sap", target))
        {
            return true;
        }

        if (ai->HasAura("gouge", target))
        {
            return true;
        }

        if (ai->HasAura("shackle undead", target))
        {
            return true;
        }
    }

    return false;
}

bool PossibleAttackTargetsValue::HasUnBreakableCC(Unit* target, Player* player)
{
    if (target->IsStunned())
    {
        return true;
    }

    if (sServerFacade.IsFeared(target))
    {
        return true;
    }

    if (sServerFacade.IsInRoots(target))
    {
        return true;
    }

    return false;
}

bool PossibleAttackTargetsValue::IsImmuneToDamage(Unit* target, Player* player)
{
    // Charmed
    if (sServerFacade.IsCharmed(target) && target->IsInTeam(player, true))
    {
        return true;
    }

    // Immune to damage
    PlayerbotAI* ai = player->GetPlayerbotAI();
    if (ai)
    {
        for (const Aura* aura : ai->GetAuras(target))
        {
            const SpellEntry* spellInfo = aura->GetSpellProto();
            if (spellInfo)
            {
                if (spellInfo->Mechanic == MECHANIC_BANISH || 
                    spellInfo->Mechanic == MECHANIC_INVULNERABILITY ||
                    spellInfo->Mechanic == MECHANIC_IMMUNE_SHIELD)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

std::string PossibleAttackTargetsValue::Format()
{
    std::ostringstream out;

    for (auto& target : value)
    {
        if (target != value.front())
            out << ",";

        WorldObject* wo = ai->GetWorldObject(target);

        if (wo)
            out << wo->GetName();
        else
            out << target;
    }

    return out.str().c_str();
}

bool PossibleAttackTargetsValue::IsTapped(Unit* target, Player* player)
{
    if (player)
    {
        PlayerbotAI* ai = player->GetPlayerbotAI();

        if (ai && ai->HasAura("tame beast", target))
            return false;

        const Creature* creature = dynamic_cast<Creature*>(target);
        if (creature)
        {
            Unit* victim = creature->GetVictim();
            Player* master = ai ? ai->GetMaster() : nullptr;
            PlayerbotAI* ai = player->GetPlayerbotAI();

             if (!victim) //Target is not attacking anything.
                return true;

            if (master && victim == master) //Target is attacking master.
                return true;

            if (player->IsInGroup(victim)) //Target is attacking groupmember.
                return true;

            if (!creature->HasLootRecipient()) //Target is untapped.
                return true;

            if (creature->IsTappedBy(player)) //Target is tapped by player.
                return true;

            if (master && target->getThreatManager().getThreat(master)) //Master as threat
                return true;

            if (ai && ai->HasStrategy("attack tagged", BotState::BOT_STATE_NON_COMBAT)) //Can attack tagged.
                if(ai->HasActivePlayerMaster() || !player->GetGroup() || player->GetGroup()->IsLeader(player->GetObjectGuid())) //Playing with a player or not in a group or master of group.
                    return true;
        }
    }

    return false;
}

bool PossibleAttackTargetsValue::IsValid(Unit* target, Player* player, float range, bool ignoreCC, bool checkAttackerValid)
{
    // Check for the valid attackers value
    if (checkAttackerValid && !AttackersValue::IsValid(target, player))
    {
        return false;
    }

    if (!IsPossibleTarget(target, player, range, ignoreCC))
        return false;

    if (sServerFacade.GetThreatManager(target).getCurrentVictim())
        return true;

    if (target->GetGuidValue(UNIT_FIELD_TARGET))
        return true;

    if (target->GetObjectGuid().IsPlayer())
        return true;

    if (player->GetPlayerbotAI() && (target->GetObjectGuid() == PAI_VALUE(ObjectGuid, "attack target")))
        return true;

    if(!HasIgnoreCCRti(target, player) && (HasBreakableCC(target, player) || HasUnBreakableCC(target, player)))
        return true;

    if (player->GetPlayerbotAI() && !player->GetPlayerbotAI()->HasActivePlayerMaster()&& PAI_VALUE(Unit*, "rti target") == target)
        return true;

    return false;
}

bool PossibleAttackTargetsValue::IsPossibleTarget(Unit* target, Player* player, float range, bool ignoreCC)
{
    if(target)
    {
        // If the target is in an attackable distance
        if(!player->IsWithinDistInMap(target, range))
        {
            return false;
        }

        // If the target is immune to any damage
        if (IsImmuneToDamage(target, player))
        {
            return false;
        }

        // If the target is CC'ed
        if(!ignoreCC && !HasIgnoreCCRti(target, player) && (HasBreakableCC(target, player) || HasUnBreakableCC(target, player)))
        {
            return false;
        }

        // If the target is a NPC
        Player* enemyPlayer = dynamic_cast<Player*>(target);
        if (!enemyPlayer)
        {
            // If the target is not tapped (the player doesn't have the loot rights (gray name))
            if (!IsTapped(target, player))
            {
                return false;
            }
        }

        return true;
    }

    return false;
}

bool PossibleAddsValue::Calculate()
{
    PlayerbotAI *ai = bot->GetPlayerbotAI();
    std::list<ObjectGuid> possible = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible targets no los")->Get();
    std::list<ObjectGuid> attackers = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();

    for (std::list<ObjectGuid>::iterator i = possible.begin(); i != possible.end(); ++i)
    {
        ObjectGuid guid = *i;
        if (find(attackers.begin(), attackers.end(), guid) != attackers.end()) continue;

        Unit* add = ai->GetUnit(guid);
        if (add && !add->GetGuidValue(UNIT_FIELD_TARGET) && !sServerFacade.GetThreatManager(add).getCurrentVictim() && sServerFacade.IsHostileTo(add, bot))
        {
            for (std::list<ObjectGuid>::iterator j = attackers.begin(); j != attackers.end(); ++j)
            {
                Unit* attacker = ai->GetUnit(*j);
                if (!attacker) continue;

                float dist = sServerFacade.GetDistance2d(attacker, add);
                if (sServerFacade.IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig.aoeRadius * 1.5f)) continue;
                if (sServerFacade.IsDistanceLessOrEqualThan(dist, sPlayerbotAIConfig.aggroDistance)) return true;
            }
        }
    }
    return false;
}