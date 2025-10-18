
#include "playerbot/playerbot.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "PositionAction.h"

using namespace ai;

void TellPosition(PlayerbotAI* ai, Player* requester, std::string name, ai::PositionEntry pos)
{
    std::ostringstream out; out << "Position " << name;
    if (pos.isSet())
    {
        float x = pos.x, y = pos.y;
        Map2ZoneCoordinates(x, y, ai->GetBot()->GetZoneId());
        out << " is set to " << x << "," << y;
    }
    else
        out << " is not set";
    ai->TellPlayer(requester, out);
}

bool PositionAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();
	if (param.empty())
		return false;

    if (!requester)
        return false;

    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    if (param == "?")
    {
        for (ai::PositionMap::iterator i = posMap.begin(); i != posMap.end(); ++i)
        {
            if (i->second.isSet())
                TellPosition(ai, requester, i->first, i->second);
        }
        return true;
    }

    std::vector<std::string> params = split(param, ' ');
    if (params.size() != 2)
    {
        ai->TellPlayer(requester, "Whisper position <name> ?/set/reset");
        return false;
    }

    std::string name = params[0];
    std::string action = params[1];
	ai::PositionEntry pos = posMap[name];
	if (action == "?")
	{
	    TellPosition(ai, requester, name, pos);
	    return true;
	}

    std::vector<std::string> coords = split(action, ',');
    if (coords.size() == 3)
    {
        pos.Set(atoi(coords[0].c_str()), atoi(coords[1].c_str()), atoi(coords[2].c_str()), ai->GetBot()->GetMapId());
        posMap[name] = pos;

        std::ostringstream out; out << "Position " << name << " is set";
        ai->TellPlayer(requester, out);
        return true;
    }

	if (action == "set")
	{
        pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), ai->GetBot()->GetMapId());
	    posMap[name] = pos;

	    std::ostringstream out; out << "Position " << name << " is set";
	    ai->TellPlayer(requester, out);
	    return true;
	}

	if (action == "reset")
	{
	    pos.Reset();
	    posMap[name] = pos;

	    std::ostringstream out; out << "Position " << name << " is reset";
	    ai->TellPlayer(requester, out);
	    return true;
	}

    return false;
}

bool MoveToPositionAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
	ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()[qualifier];
    if (!pos.isSet())
    {
        std::ostringstream out; out << "Position " << qualifier << " is not set";
        ai->TellPlayer(requester, out);
        return false;
    }

    return MoveTo(bot->GetMapId(), pos.x, pos.y, pos.z, idle);
}

bool MoveToPositionAction::isUseful()
{
    ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()[qualifier];
    float distance = AI_VALUE2(float, "distance", std::string("position_") + qualifier);
    return pos.isSet() && distance > ai->GetRange("follow") && IsMovingAllowed();
}

bool GuardAction::isUseful()
{
    if (ai->IsStateActive(BotState::BOT_STATE_COMBAT))
    {
        Unit* target = AI_VALUE(Unit*, "current target");

        if (!target)
            return true;

        if (target->GetTarget() == bot) //Try pulling target to guard position
            return true;

        if (!ai->IsRanged(bot)) //Melee bots stay in melee.
            return false;

        WorldPosition formationPosition = AI_VALUE(WorldPosition, "formation position");

        if (formationPosition.sqDistance2d(target) > ai->GetRange("spell")) //Do not move to guard if we can't attack from that position.
            return false;
    }
            
    return true;
}

bool SetReturnPositionAction::Execute(Event& event)
{
    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    ai::PositionEntry returnPos = posMap["return"];
    ai::PositionEntry randomPos = posMap["random"];
    if (returnPos.isSet() && !randomPos.isSet())
    {
        float angle = 2 * M_PI * urand(0, 1000) / 100.0f;
        float dist = ai->GetRange("follow") * urand(0, 1000) / 1000.0f;
        float x = returnPos.x + cos(angle) * dist,
             y = returnPos.y + sin(angle) * dist,
             z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);

        if (!bot->IsWithinLOS(x, y, z, true))
            return false;

        randomPos.Set(x, y, z, bot->GetMapId());
        posMap["random"] = randomPos;
        return true;
    }
    return false;
}

bool SetReturnPositionAction::isUseful()
{
    ai::PositionMap& posMap = context->GetValue<ai::PositionMap&>("position")->Get();
    return posMap["return"].isSet() && !posMap["random"].isSet();
}


bool ReturnAction::isUseful()
{
    ai::PositionEntry pos = context->GetValue<ai::PositionMap&>("position")->Get()[qualifier];
    return pos.isSet() && AI_VALUE2(float, "distance", "position_random") > ai->GetRange("follow");
}

bool ReturnToStayPositionAction::isPossible()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionEntry stayPosition = posMap["stay"];
    if (stayPosition.isSet())
    {
        const float distance = bot->GetDistance(stayPosition.x, stayPosition.y, stayPosition.z);
        if (distance > sPlayerbotAIConfig.reactDistance)
        {
            ai->TellError(GetMaster(), "The stay position is too far to return. I am going to stay where I am now");
            
            // Set the stay position to current position
            stayPosition.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
            posMap["stay"] = stayPosition;
        }

        return true;
    }

    return false;
}

bool ReturnToPullPositionAction::isPossible()
{
    PositionMap& posMap = AI_VALUE(PositionMap&, "position");
    PositionEntry stayPosition = posMap["pull"];
    if (stayPosition.isSet())
    {
        PullStrategy* strategy = PullStrategy::Get(ai);
        if (strategy && strategy->HasPullStarted())
        {
            Unit* target = strategy->GetTarget();
            if (target)
            {
                if (target->GetTarget() == bot)
                {
                    const float distance = bot->GetDistance(stayPosition.x, stayPosition.y, stayPosition.z);
                    if (distance > sPlayerbotAIConfig.reactDistance)
                    {
                        ai->TellError(GetMaster(), "The pull position is too far to return. I am going to pull where I am now");

                        // Set the stay position to current position
                        stayPosition.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
                        posMap["pull"] = stayPosition;
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

