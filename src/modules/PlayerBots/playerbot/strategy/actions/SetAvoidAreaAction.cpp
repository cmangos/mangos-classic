
#include "playerbot/playerbot.h"
#include "SetAvoidAreaAction.h"

#include "playerbot/strategy/values/PositionValue.h"
#include "MotionGenerators/PathFinder.h"
using namespace ai;


bool SetAvoidAreaAction::Execute(Event& event)
{
    PathFinder pathfinder(bot->GetMapId(), bot->GetInstanceId());

    SET_AI_VALUE2(PositionEntry, "pos", "last avoid", PositionEntry(bot));

    std::list<ObjectGuid> targets = AI_VALUE_LAZY(std::list<ObjectGuid>, "possible targets");

    if (targets.empty())
        return false;

    FactionTemplateEntry const* humanFaction = sFactionTemplateStore.LookupEntry(1);
    FactionTemplateEntry const* orcFaction = sFactionTemplateStore.LookupEntry(2);

    for (auto& target : targets)
    {
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(target.GetEntry());

        if (!cInfo)
            continue;

        if (cInfo->NpcFlags > 0) //Ignore npcs.
            continue;

        if (cInfo->MaxLevel < bot->GetLevel() - 3) //Ignore lower level mobs.
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);
        ReputationRank reactionHum = PlayerbotAI::GetFactionReaction(humanFaction, factionEntry);
        ReputationRank reactionOrc = PlayerbotAI::GetFactionReaction(orcFaction, factionEntry);

        if (reactionHum >= REP_NEUTRAL || reactionOrc >= REP_NEUTRAL) //ignore mobs friendly to opposing faction (they share avoid maps)
            continue;

        Unit* targetUnit = ai->GetUnit(target);

        if (!targetUnit)
            continue;

        WorldPosition point(targetUnit);
        pathfinder.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 12, targetUnit->GetAttackDistance(bot) * 2.5);
        pathfinder.setArea(point.getMapId(), point.getX(), point.getY(), point.getZ(), 13, targetUnit->GetAttackDistance(bot));
    }

    return true;
}

bool SetAvoidAreaAction::isUseful()
{
    if (bot->GetInstanceId())
        return false;

    PositionEntry p = AI_VALUE2(PositionEntry, "pos", "last avoid");

    if (!p.isSet())
        return true;

    return p.Get().distance(bot) > 20.0f;
}