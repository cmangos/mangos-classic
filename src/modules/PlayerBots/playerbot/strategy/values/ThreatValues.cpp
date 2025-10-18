
#include "playerbot/playerbot.h"
#include "ThreatValues.h"

#include "playerbot/ServerFacade.h"
#include "Combat/ThreatManager.h"

using namespace ai;

float MyThreatValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    
    if (target->GetObjectGuid() != lastTarget) //Reset history if we switched target.
        LogCalculatedValue::Reset();

    lastTarget = target->GetObjectGuid();
      
    return ThreatValue::GetThreat(bot, target);
}

float TankThreatValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);

    return ThreatValue::GetTankThreat(ai, target);
}

uint8 ThreatValue::Calculate()
{
    if (qualifier == "aoe")
    {
        uint8 maxThreat = 0;
        std::list<ObjectGuid> attackers = context->GetValue<std::list<ObjectGuid>>("possible attack targets")->Get();
        for (std::list<ObjectGuid>::iterator i = attackers.begin(); i != attackers.end(); i++)
        {
            Unit* unit = ai->GetUnit(*i);
            if (!unit || !sServerFacade.IsAlive(unit))
                continue;

            uint8 threat = Calculate(unit);
            if (!maxThreat || threat > maxThreat)
                maxThreat = threat;
        }

        return maxThreat;
    }

    Unit* target = AI_VALUE(Unit*, qualifier);

    if (target && target->IsFriend(bot))
        target = target->GetTarget();

    return Calculate(target);
}

float ThreatValue::GetThreat(Player* player, Unit* target)
{
    if (!target || target->GetMapId() != player->GetMapId())
        return 0;

    if (dynamic_cast<Player*>(target) && (dynamic_cast<Player*>(target))->IsBeingTeleported())
        return 0;

    if (target->IsFriend(player))
        target = target->GetTarget();

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    float botThreat = sServerFacade.GetThreatManager(target).getThreat(player);

    return botThreat;
}

float ThreatValue::GetTankThreat(PlayerbotAI* ai, Unit* target)
{
    if (!target)
        return 0;

    if (target->IsFriend(ai->GetBot()))
        target = target->GetTarget();

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    Group* group = ai->GetBot()->GetGroup();
    if (!group)
        return 0;

    float maxThreat = -1.0f;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->getSource();
        if (!player || !sServerFacade.IsAlive(player) || !ai->IsSafe(player) || player == ai->GetBot())
            continue;

        if (ai->IsTank(player))
        {
            float threat = sServerFacade.GetThreatManager(target).getThreat(player);
            if (maxThreat < threat)
                maxThreat = threat;
        }
    }

    return maxThreat;
}

uint8 ThreatValue::Calculate(Unit* target)
{
    if (!target)
        return 0;

    if (target->IsFriend(bot))
        target = target->GetTarget();

    if (!target)
        return 0;

    if (target->GetObjectGuid().IsPlayer())
        return 0;

    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    float botThreat, maxThreat;
    if (qualifier == "aoe")
    {
        botThreat = GetThreat(bot, target);
        maxThreat = GetTankThreat(ai, target);
    }
    else
    {
        botThreat = AI_VALUE2(float, "my threat", qualifier);
        maxThreat = AI_VALUE2(float, "tank threat", qualifier);
    }

    if (maxThreat < 0)
        return 0;

    // calculate normal threat for fleeing targets
    bool fleeing = target->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLEEING_MOTION_TYPE ||
        target->GetMotionMaster()->GetCurrentMovementGeneratorType() == TIMED_FLEEING_MOTION_TYPE;

    // return high threat if tank has no threat
    if (target->IsInCombat() && maxThreat == 0 && !fleeing)
        return 100;

    // return low threat if mob if fleeing
    if (maxThreat > 0 && fleeing)
        return 0;

    return botThreat * 100 / maxThreat;
}