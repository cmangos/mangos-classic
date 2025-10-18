
#include "playerbot/playerbot.h"
#include "TravelAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/ServerFacade.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "World/WorldState.h"
#include "playerbot/TravelMgr.h"


using namespace ai;
using namespace MaNGOS;

bool TravelAction::Execute(Event& event)
{    
    TravelTarget * target = AI_VALUE(TravelTarget *, "travel target");
    
    target->CheckStatus();     

    SET_AI_VALUE2(time_t, "manual time", "next travel check", time(0) + 5);

    return false;
}

bool TravelAction::isUseful()
{
    if (!AI_VALUE(bool,"travel target active"))
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
        if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            return false;

    if (sServerFacade.isMoving(bot))
        return false;

    if (AI_VALUE2(time_t, "manual time", "next travel check") > time(0))
        return false;

    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");
    if (target->GetStatus() == TravelStatus::TRAVEL_STATUS_WORK)
        return true;
    
    if (target->GetStatus() == TravelStatus::TRAVEL_STATUS_COOLDOWN)
        return true;

    if (target->GetStatus() == TravelStatus::TRAVEL_STATUS_READY && !urand(0, 20))
        return true;

    return false;
}

bool MoveToDarkPortalAction::Execute(Event& event)
{
    if (bot->GetGroup())
        if (!ai->IsGroupLeader() && ai->HasActivePlayerMaster() && !bot->GetPlayerbotAI()->GetGroupMaster()->GetPlayerbotAI())
            return false;

#ifndef MANGOSBOT_ZERO
    if (sWorldState.GetExpansion() == EXPANSION_TBC && bot->GetLevel() > 57)
    {
        if ((bot->GetTeam() == ALLIANCE && bot->GetQuestStatus(10119) == QUEST_STATUS_NONE) || (bot->GetTeam() == HORDE && bot->GetQuestStatus(9407) == QUEST_STATUS_NONE))
        {
            if (!sServerFacade.IsInCombat(bot))
            {
                if (bot->GetTeam() == ALLIANCE)
                {
                    Quest const* quest = sObjectMgr.GetQuestTemplate(10119);
                    if (quest && bot->CanAddQuest(quest, false))
                    {
                        CreatureDataPair const* dataPair = sRandomPlayerbotMgr.GetCreatureDataByEntry(16841);
                        if (dataPair)
                        {
                            ObjectGuid npcGuid = ObjectGuid(HIGHGUID_UNIT, 16841, dataPair->first);
                            Creature* npc = bot->GetMap()->GetCreature(npcGuid);
                            bot->AddQuest(quest, npc);
                        }
                    }
                }
                else
                {
                    Quest const* quest = sObjectMgr.GetQuestTemplate(9407);
                    if (quest && bot->CanAddQuest(quest, false))
                    {
                        CreatureDataPair const* dataPair = sRandomPlayerbotMgr.GetCreatureDataByEntry(19254);
                        if (dataPair)
                        {
                            ObjectGuid npcGuid = ObjectGuid(HIGHGUID_UNIT, 19254, dataPair->first);
                            Creature* npc = bot->GetMap()->GetCreature(npcGuid);
                            bot->AddQuest(quest, npc);
                        }
                    }
                }
            }
            return MoveTo(0, -11815.1f, -3190.39f, -30.7338f, false, true);
        }
        else
        {
            return MoveTo(0, -11906.9f, -3208.53f, -14.8616f, false, true);
        }
    }
    else if (sWorldState.GetExpansion() == EXPANSION_NONE)
    {
        if (urand(0, 1))
            return MoveTo(0, -11891.500f, -3207.010f, -14.798f, false, true);
        else
            return MoveTo(0, -11815.1f, -3190.39f, -30.7338f, false, true);
    }
#endif
    return false;
}

bool MoveToDarkPortalAction::isUseful()
{
#ifndef MANGOSBOT_ZERO
    return bot->GetLevel() > 54;
#endif
    return false;
}

bool DarkPortalAzerothAction::Execute(Event& event)
{
#ifndef MANGOSBOT_ZERO
    if (sWorldState.GetExpansion() == EXPANSION_TBC && bot->GetLevel() > 57)
    {
        WorldPacket packet(CMSG_AREATRIGGER);
        packet << 4354;

        return bot->GetPlayerbotAI()->DoSpecificAction("reach area trigger", Event("travel action", packet));
    }
#endif
    return false;
}

bool DarkPortalAzerothAction::isUseful()
{
#ifndef MANGOSBOT_ZERO
    return bot->GetLevel() > 57;
#endif
    return false;
}

bool MoveFromDarkPortalAction::Execute(Event& event)
{
    RESET_AI_VALUE(GuidPosition,"rpg target");

    if (bot->GetTeam() == ALLIANCE)
        return MoveTo(530, -319.261f, 1027.213f, 54.172638f, false, true);
    else
        return MoveTo(530, -180.444f, 1027.947f, 54.181538f, false, true);

    return false;
}
