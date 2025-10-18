#pragma once
#include "playerbot/strategy/Trigger.h"
#include <MotionGenerators/MoveMap.h>

namespace ai
{
    template< typename... Args >
    std::string string_sprintf(const char* format, Args... args) {
        int length = std::snprintf(nullptr, 0, format, args...);
        assert(length >= 0);

        char* buf = new char[length + 1];
        std::snprintf(buf, length + 1, format, args...);

        std::string str(buf);
        delete[] buf;
        return str;
    }

    class MoveStuckTrigger : public Trigger
    {
    public:
        MoveStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
            {
                RESET_AI_VALUE(WorldPosition, "current position");
                return false;
            }

            WorldPosition botPos(bot);

            uint32 timeSinceLastMove = AI_VALUE2(uint32, "time since last change", "current position");

            if(timeSinceLastMove > 5 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Position did not change for " + std::to_string(timeSinceLastMove) + " seconds.", "debug stuck");

                return true;
            }

            uint32 distanceMoved = AI_VALUE2(uint32, "distance moved since", 10 * MINUTE);

            if (distanceMoved > 0 && distanceMoved < 50.0f)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Only moved " + std::to_string(distanceMoved) + " yards in the last 10 minutes.", "debug stuck");

                return true;
            }

            return false;
        }
    };

    class MoveLongStuckTrigger : public Trigger
    {
    public:
        MoveLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "move long stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
            {
                RESET_AI_VALUE(WorldPosition, "current position");
                RESET_AI_VALUE(uint32, "experience");
                return false;
            }

            WorldPosition botPos(bot);

            Cell const& cell = bot->GetCurrentCell();

            GridPair grid = botPos.getGridPair();

            if (grid.x_coord < 0 || grid.x_coord >= MAX_NUMBER_OF_GRIDS)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In invalid grid" + std::to_string(grid.x_coord) + "," + std::to_string(grid.y_coord), "debug stuck");

                return true;
            }

            if (grid.y_coord < 0 || grid.y_coord >= MAX_NUMBER_OF_GRIDS)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In invalid grid" + std::to_string(grid.x_coord) + "," + std::to_string(grid.y_coord), "debug stuck");

                return true;
            }

#ifdef MANGOSBOT_TWO
            if (cell.GridX() > 0 && cell.GridY() > 0 && !MMAP::MMapFactory::createOrGetMMapManager()->IsMMapTileLoaded(botPos.getMapId(), 0, cell.GridX(), cell.GridY()) && !MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), botPos.getMapId(), 0, cell.GridX(), cell.GridY(), 0))
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In unloaded grid" + std::to_string(grid.x_coord) + "," + std::to_string(grid.y_coord), "debug stuck");

                return true;
            }
#else
            if (cell.GridX() > 0 && cell.GridY() > 0 && !MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(botPos.getMapId(), cell.GridX(), cell.GridY()) 
                && !MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), botPos.getMapId(), cell.GridX(), cell.GridY()))
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In unloaded grid" + std::to_string(grid.x_coord) + "," + std::to_string(grid.y_coord), "debug stuck");

                return true;
            }
#endif

            uint32 timeSinceLastMove = AI_VALUE2(uint32, "time since last change", "current position");

            if (timeSinceLastMove > 10 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Position did not change for " + std::to_string(timeSinceLastMove) + " seconds.", "debug stuck");

                return true;
            }

            uint32 timeSinceLastXp = AI_VALUE2(uint32, "time since last change", "experience");

            if (timeSinceLastXp < 15 * MINUTE)
                return false;

            uint32 distanceMoved = AI_VALUE2(uint32, "distance moved since", 15 * MINUTE);

            if (distanceMoved > 0 && distanceMoved < 50.0f)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Only moved " + std::to_string(distanceMoved) + " yards in the last 10 minutes.", "debug stuck");

                return true;
            }

            return false;
        }
    };

    class CombatStuckTrigger : public Trigger
    {
    public:
        CombatStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->GetState() != BotState::BOT_STATE_COMBAT)
                return false;

            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
                return false;

            WorldPosition botPos(bot);

            uint32 timeSinceCombatChange = AI_VALUE2(uint32, "time since last change", "combat::self target");
           
            if (timeSinceCombatChange > 5 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Combat did not change for " + std::to_string(timeSinceCombatChange) + " seconds.", "debug stuck");

                return true;
            }

            if (bot->duel && bot->duel->startTime - time(0) > 15 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In Duel for " + std::to_string(bot->duel->startTime - time(0)) + " seconds.", "debug stuck");

                return true;
            }

            return false;
        }
    };

    class CombatLongStuckTrigger : public Trigger
    {
    public:
        CombatLongStuckTrigger(PlayerbotAI* ai) : Trigger(ai, "combat long stuck", 5) {}

        virtual bool IsActive()
        {
            if (ai->GetState() != BotState::BOT_STATE_COMBAT)
                return false;

            if (ai->HasActivePlayerMaster())
                return false;

            if (ai->GetGroupMaster() && !ai->GetGroupMaster()->GetPlayerbotAI())
                return false;

            if (!ai->AllowActivity(ALL_ACTIVITY))
                return false;

            WorldPosition botPos(bot);

            uint32 timeSinceCombatChange = AI_VALUE2(uint32, "time since last change", "combat::self target");

            if (timeSinceCombatChange > 15 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: Combat did not change for " + std::to_string(timeSinceCombatChange) + " seconds.", "debug stuck");

                return true;
            }

            if (bot->duel && bot->duel->startTime - time(0) > 15 * MINUTE)
            {
                ai->TellDebug(ai->GetMaster(), "Stuck: In Duel for " + std::to_string(bot->duel->startTime - time(0)) + " seconds.", "debug stuck");

                return true;
            }

            return false;
        }
    };

    class LeaderIsAfkTrigger : public Trigger
    {
    public:
        LeaderIsAfkTrigger(PlayerbotAI* ai) : Trigger(ai, "leader is afk", 10) {}

        virtual bool IsActive()
        {
            if (ai->HasRealPlayerMaster())
                return false;

            if (Group* group = bot->GetGroup())
            {
                Player* leader = sObjectMgr.GetPlayer(group->GetLeaderGuid(), true);
                if (!leader)
                    return false;

                return leader->isAFK();
            }

            return false;
        }
    };
}
