
#include "playerbot/playerbot.h"
#include "SeeSpellAction.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/ServerFacade.h"
#include "MotionGenerators/MovementGenerator.h"
#ifdef MANGOS
#include "luaEngine.h"
#endif

#include <MotionGenerators/PathFinder.h>
#include "RtscAction.h"
#include "playerbot/TravelMgr.h"


using namespace ai;

Creature* SeeSpellAction::CreateWps(Player* wpOwner, float x, float y, float z, float o, uint32 entry, Creature* lastWp, bool important)
{
    float dist = wpOwner->GetDistance(x, y, z);
    float delay = 1000.0f * dist / wpOwner->GetSpeed(MOVE_RUN) + sPlayerbotAIConfig.reactDelay;

    if (!important)
        delay *= 0.25;
    Creature* wpCreature = wpOwner->SummonCreature(entry, x, y, z - 1, o, TEMPSPAWN_TIMED_DESPAWN, delay);

    if (!important)
        wpCreature->SetObjectScale(0.2f);

    return wpCreature;
}

float GetAngle(const float x1, const float y1, const float x2, const float y2)
{
    float dx = x1 - x2;
    float dy = y1 - y2;

    float ang = atan2(dy, dx);                              // returns value between -Pi..Pi
    ang = (ang >= 0) ? ang : 2 * M_PI_F + ang;
    return ang;
}

bool SeeSpellAction::isUseful()
{
    return ai->GetMaster() && ai->HasStrategy("rtsc", ai->GetState());
}

bool SeeSpellAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    WorldPacket p(event.getPacket());
    uint32 spellId;

    p.rpos(0);
#ifndef MANGOSBOT_TWO
    p >> spellId;
#endif

#ifdef MANGOSBOT_ONE
    uint8  cast_count;
    p >> cast_count;
#endif
#ifdef MANGOSBOT_TWO
    uint8  cast_count, cast_flags;
    p >> cast_count;
    p >> spellId;
    p >> cast_flags;
#endif

    if (spellId != RTSC_MOVE_SPELL)
        return false;

    SpellEntry const* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    SpellCastTargets targets;

    p >> targets.ReadForCaster(requester);
    WorldPosition spellPosition(requester->GetMapId(), targets.m_destPos);
    SET_AI_VALUE(WorldPosition, "see spell location", spellPosition);

    if (ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
    {
        PathFinder path(bot);

        float x = spellPosition.getX();
        float y = spellPosition.getY();
        float z = spellPosition.getZ();

        std::ostringstream out;

        if (spellPosition.isOutside())
            out << "[outside]";

        out << " area = ";

        out << path.getArea(bot->GetMapId(), x, y, z);

        unsigned short flags = path.getFlags(bot->GetMapId(), x, y, z);

        out << " flags = " << flags;

        if (flags & NAV_GROUND)
            out << ", ground";
        if (flags & NAV_EMPTY)
            out << ", empty";
        if (flags & NAV_GROUND_STEEP)
            out << ", slope";
        if (flags & NAV_WATER)
            out << ", water";
        if (flags & NAV_MAGMA_SLIME)
            out << ", magma slime";

        ai->TellPlayer(requester, out);
    }

    bool selected = AI_VALUE(bool, "RTSC selected");
    bool inRange = spellPosition.distance(bot) <= 10;
    std::string nextAction = AI_VALUE(std::string, "RTSC next spell action");

    if (nextAction.empty())
    {
        if (!inRange && selected)
            requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 6372);
        else if (inRange && !selected)
            requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);

        SET_AI_VALUE(bool, "RTSC selected", inRange);

        if (selected)
            return MoveToSpell(requester, spellPosition);

        return inRange;
    }
    else if (nextAction == "move")
    {
        return MoveToSpell(requester, spellPosition);
    }
    else if (nextAction == "jump")
    {
        RESET_AI_VALUE(std::string, "RTSC next spell action");
        SET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump", spellPosition);
        bool success = ai->DoSpecificAction("jump::rtsc", Event(), true);
        if (!success)
        {
            RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
            RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
            ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
            std::ostringstream out;
            out << "Can't find a way to jump!";
            ai->TellError(requester, out.str());
            return false;
        }
        else
        {
            WorldPosition jumpPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
            if (jumpPosition)
            {
                bool couldMove = MoveToSpell(requester, jumpPosition, false);
                if (couldMove)
                {
                    ai->ChangeStrategy("+rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                    return true;
                }

                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                std::ostringstream out;
                out << "Can't move to jump position!";
                ai->TellError(requester, out.str());
                return false;
            }
        }

        return success;
    }
    else if (nextAction.find("save ") != std::string::npos)
    {
        std::string locationName;
        if (nextAction.find("save selected ") != std::string::npos)
        {
            if (!selected)
                return false;
            locationName = nextAction.substr(14);
        }
        else
            locationName = nextAction.substr(5);

        SetFormationOffset(requester, spellPosition);

        SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);
        
        Creature* wpCreature = bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
        wpCreature->SetObjectScale(0.5f);
        RESET_AI_VALUE(std::string, "RTSC next spell action");

        return true;
    }

    return false;        
}

bool SeeSpellAction::SelectSpell(Player* requester, WorldPosition& spellPosition)
{
    if (spellPosition.distance(bot) <= 5 || AI_VALUE(bool, "RTSC selected"))
    {
        SET_AI_VALUE(bool, "RTSC selected", true);
        requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);
    }
    return true;
}

bool SeeSpellAction::MoveToSpell(Player* requester, WorldPosition& spellPosition, bool inFormation)
{
    if(inFormation)
        SetFormationOffset(requester, spellPosition);

    if (ai->HasStrategy("stay", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry stayPosition = posMap["stay"];

        stayPosition.Set(spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getMapId());
        posMap["stay"] = stayPosition;
        posMap["return"] = stayPosition;
    }
    else if (ai->HasStrategy("guard", ai->GetState()))
    {
        PositionMap& posMap = AI_VALUE(PositionMap&, "position");
        PositionEntry guardPosition = posMap["guard"];

        guardPosition.Set(spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getMapId());
        posMap["guard"] = guardPosition;
        posMap["return"] = guardPosition;
    }
    else if (ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT) && requester)
    {
        FormationValue* formation = (FormationValue*)context->GetValue<Formation*>("formation");

        if (formation->getName() == "custom")
        {
            PositionMap& posMap = AI_VALUE(PositionMap&, "position");
            PositionEntry followPosition = posMap["follow"];

            spellPosition -= WorldPosition(requester);
            spellPosition.rotateXY(-1 * requester->GetOrientation());

            followPosition.Set(spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getMapId());
            posMap["follow"] = followPosition;
            posMap["return"] = followPosition;
        }
    }

    return MoveTo(spellPosition.getMapId(), spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), false, false);
}

void SeeSpellAction::SetFormationOffset(Player* requester, WorldPosition& spellPosition)
{
    Formation* formation = AI_VALUE(Formation*, "formation");

    if (formation->getName() == "custom")
        return;

    WorldLocation formationLocation = formation->GetLocation();

    if (formationLocation.coord_x != 0 || formationLocation.coord_y != 0)
    {
        spellPosition -= WorldPosition(requester);
        spellPosition += formationLocation;
    }
}
