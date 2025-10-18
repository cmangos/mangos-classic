
#include "playerbot/playerbot.h"
#include "PossibleTargetsValue.h"
#include "PossibleAttackTargetsValue.h"

#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"

using namespace ai;
using namespace MaNGOS;

std::list<ObjectGuid> PossibleTargetsValue::Calculate()
{
    float rangeCheck = range;
    bool shouldIgnoreValidate = false;
    if (!qualifier.empty())
    {
        rangeCheck = Qualified::getMultiQualifierInt(qualifier, 0, ":");
        shouldIgnoreValidate = Qualified::getMultiQualifierInt(qualifier, 1, ":");
    }

    std::list<Unit*> targets;
    FindPossibleTargets(bot, targets, rangeCheck);

    std::list<ObjectGuid> results;
    for (std::list<Unit*>::iterator i = targets.begin(); i != targets.end(); ++i)
    {
        Unit* unit = *i;
        if (unit && (shouldIgnoreValidate || AcceptUnit(unit)))
        {
            results.push_back(unit->GetObjectGuid());
        }
    }

    return results;
}

void PossibleTargetsValue::FindUnits(std::list<Unit*> &targets)
{
    FindPossibleTargets(bot, targets, range);
}

bool PossibleTargetsValue::AcceptUnit(Unit* unit)
{
    return IsValid(unit, bot, ignoreLos);
}

void PossibleTargetsValue::FindPossibleTargets(Player* player, std::list<Unit*>& targets, float range)
{
    MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck u_check(player, range);
    MaNGOS::UnitListSearcher<MaNGOS::AnyUnfriendlyUnitInObjectRangeCheck> searcher(targets, u_check);
    Cell::VisitAllObjects(player, searcher, range);
}

bool PossibleTargetsValue::IsFriendly(Unit* target, Player* player)
{
    bool friendly = false;
    if (sServerFacade.IsFriendlyTo(target, player))
    {
        friendly = true;

#ifndef MANGOSBOT_ZERO
        // Check if the target is another player in a duel/arena
        Player* targetPlayer = dynamic_cast<Player*>(target);
        if (targetPlayer)
        {
            // If the target is in an arena with the player and is not on the same team
            if (targetPlayer->InArena() && player->InArena() && (targetPlayer->GetBGTeam() != player->GetBGTeam()))
            {
                friendly = false;
            }
        }
#endif
    }

    return friendly;
}

bool PossibleTargetsValue::IsAttackable(Unit* target, Player* player)
{
    const bool inVehicle = player->GetPlayerbotAI() && player->GetPlayerbotAI()->IsInVehicle();
    return !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_ATTACKABLE_1) &&
           !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNTARGETABLE) &&
           (inVehicle || !target->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE)) &&
           !target->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION);
}

bool PossibleTargetsValue::IsValid(Unit* target, Player* player, bool ignoreLos)
{
    // If the target is available
    if (target && target->IsInWorld() && (target->GetMapId() == player->GetMapId()))
    {
        // If the target is dead
        if (sServerFacade.UnitIsDead(target))
        {
            return false;
        }

        // If the target is friendly
        if (IsFriendly(target, player))
        {
            return false;
        }

        // If the target can't be attacked
        if (!IsAttackable(target, player))
        {
            return false;
        }

        // If the target is not visible (to the owner bot)
        if (!ignoreLos && !target->IsVisibleForOrDetect(player, player->GetCamera().GetBody(), true))
        {
            return false;
        }

        if (!PAI_VALUE2(bool, "can free attack", GuidPosition(target).to_string()))
            return false;

        return true;
    }

    return false;
}