
#include <iomanip>
#include <fstream>
#include "playerbot/playerbot.h"
#include "RtscAction.h"


using namespace ai;

bool RTSCAction::Execute(Event& event)
{
	std::string command = event.getParam();
	Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

	if (command.empty() && !qualifier.empty())
	{
		command = qualifier;
	}

	if (!requester)
		return false;

	if (command != "reset" && !requester->HasSpell(RTSC_MOVE_SPELL))
	{
		requester->learnSpell(RTSC_MOVE_SPELL, false);
		ai->TellPlayerNoFacing(requester, "RTS control enabled.");
		ai->TellPlayerNoFacing(requester, "Aedm (Awesome energetic do move) spell trained.");
	}
	else if (command == "reset")
	{
		if (requester->HasSpell(RTSC_MOVE_SPELL))
		{
			requester->removeSpell(RTSC_MOVE_SPELL);
			ai->TellPlayerNoFacing(requester, "RTS control spell removed.");
		}

		RESET_AI_VALUE(bool, "RTSC selected");
		RESET_AI_VALUE(std::string, "RTSC next spell action");

		for (auto value : ai->GetAiObjectContext()->GetValues())
			if (value.find("RTSC saved location::") != std::string::npos)
				RESET_AI_VALUE(WorldPosition, value.c_str());

		return true;
	}

	bool selected = AI_VALUE(bool, "RTSC selected");

	if (command == "select" && !selected)
	{
		SET_AI_VALUE(bool, "RTSC selected", true);
		requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);
		return true;
	}
	else if (command == "cancel")
	{
		RESET_AI_VALUE(bool, "RTSC selected");
		RESET_AI_VALUE(std::string, "RTSC next spell action");
		if(selected)
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 6372);
		return true;
	}
	else if (command == "toggle")
	{
		if (!selected)
		{
			SET_AI_VALUE(bool, "RTSC selected", true);
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 5036);
		}
		else
		{
			SET_AI_VALUE(bool, "RTSC selected", false);
			requester->GetSession()->SendPlaySpellVisual(bot->GetObjectGuid(), 6372);
		}

		return true;
	}
	else if (command.find("save here ") != std::string::npos)
	{
		std::string locationName = command.substr(10);

		WorldPosition spellPosition(bot);
		SET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName, spellPosition);

		Creature* wpCreature = bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
		wpCreature->SetObjectScale(0.5f);

		return true;
	}
	else if (command.find("unsave ") != std::string::npos)
	{
		std::string locationName = command.substr(7);

		RESET_AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		return true;
	}
	if (command.find("save ") == 0 || command == "move")
	{	
		SET_AI_VALUE(std::string, "RTSC next spell action", command);

		return true;
	}
	if (command.find("file ") != std::string::npos)
	{
		std::vector<std::string> args = ChatHelper::splitString(command, " ");

		//save/load file_name <position_name_part> <bot_name_part>

		if (args.size() < 3)
		{
			ai->TellPlayerNoFacing(requester, "rtsc file needs atleast 2 parameters: rtsc file save/load file_name optional:position_name_part optional:bot_name_part");
			return false;
		}
		if (args.size() < 4)
		{
			args.push_back("*"); //position
		}

		bool filePerBot = args[2].find("BOTNAME") != std::string::npos;

		if (args[1] == "save")
		{
			if(!filePerBot)
				sPlayerbotAIConfig.openLog(args[2], "w", true);

			for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "group members"))
			{
				Player* player = sObjectMgr.GetPlayer(guid);

				if (!player)
					continue;

				if (player->GetMapId() != bot->GetMapId())
					continue;

				if (!player->GetPlayerbotAI())
					continue;

				if (args.size() < 5 && player != bot)
					continue;

				std::string playerName = player->GetName();

				if (args.size() == 5 && args[4] != "*" && playerName.find(args[4]) == std::string::npos)
					continue;

				std::string fileName = args[2];

				if (filePerBot)
				{
					PlayerbotTextMgr::ReplaceAll(fileName, "BOTNAME", playerName);
					sPlayerbotAIConfig.openLog(fileName, "w", true);
				}

				uint32 cnt = 0;

				std::set<std::string> names = context->GetValues();
				for (auto name : names)
				{
					UntypedValue* value = context->GetUntypedValue(name);
					if (!value)
						continue;

					if (name.find("RTSC saved location::") != 0)
						continue;

					if (args[3] != "*" && name.find(args[3]) == std::string::npos)
						continue;

					WorldPosition point = PAI_VALUE(WorldPosition, name);

					//BotName,LocationName,location

					std::ostringstream out;

						out << ((args.size() == 5 && !filePerBot) ? playerName.c_str() : "BOTNAME") << ",";

					out << name.substr(std::string("RTSC saved location::").size()) << ",";
					
					out << std::fixed << std::setprecision(2);
					out << point.print().c_str();
					
					sPlayerbotAIConfig.log(fileName, out.str().c_str());

					cnt++;
				}

				if(cnt)
					ai->TellPlayerNoFacing(requester, playerName + " " + std::to_string(cnt) + " points saved to " + fileName);
			}
		}
		else
		{
			for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "group members"))
			{
				Player* player = sObjectMgr.GetPlayer(guid);

				if (!player)
					continue;

				if (player->GetMapId() != bot->GetMapId())
					continue;

				if (!player->GetPlayerbotAI())
					continue;

				if (args.size() < 5 && player != bot)
					continue;

				std::string playerName = player->GetName();

				if (args.size() == 5 && args[5] != "*" && playerName.find(args[5]) == std::string::npos)
					continue;

				std::string fileName = args[2];

				if (filePerBot)
				{
					PlayerbotTextMgr::ReplaceAll(fileName, "BOTNAME", playerName);
				}

				std::string m_logsDir = sConfig.GetStringDefault("LogsDir");
				if (!m_logsDir.empty())
				{
					if ((m_logsDir.at(m_logsDir.length() - 1) != '/') && (m_logsDir.at(m_logsDir.length() - 1) != '\\'))
						m_logsDir.append("/");
				}
				std::ifstream in(m_logsDir + fileName, std::ifstream::in);

				if (in.fail())
					continue;
				
				uint32 cnt = 0;

				do
				{
					std::string line;
					std::getline(in, line);

					if (!line.length())
						continue;

					Tokens tokens = StrSplit(line, ",");

					//BotName,LocationName,location

					if (tokens.size() != 3)
						continue;

					if (tokens[0] != "BOTNAME" && tokens[0] != playerName)
						continue;

					if (args[3] != "*" && tokens[1].find(args[3]) == std::string::npos)
						continue;

					WorldPosition p(tokens[2]);

					SET_PAI_VALUE2(WorldPosition, "RTSC saved location", tokens[1], p);

					cnt++;
				} while (in.good());

				if (cnt)
					ai->TellPlayerNoFacing(requester, playerName + " " + std::to_string(cnt) + " points loaded from " + fileName);
			}
		}

		return true;
	}
	if (command.find("show ") != std::string::npos)
	{
		std::string locationName = command.substr(5);
		WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		if (spellPosition)
		{
			Creature* wpCreature = bot->SummonCreature(15631, spellPosition.getX(), spellPosition.getY(), spellPosition.getZ(), spellPosition.getO(), TEMPSPAWN_TIMED_DESPAWN, 2000.0f);
			wpCreature->SetObjectScale(0.5f);
		}

		return true;
	}
	if (command.find("show") != std::string::npos)
	{
		std::ostringstream out; out << "saved: ";

		for (auto value : ai->GetAiObjectContext()->GetValues())
			if (value.find("RTSC saved location::") != std::string::npos)
				if (AI_VALUE2(WorldPosition, "RTSC saved location", value.substr(21).c_str()))
					out << value.substr(21).c_str() << ",";

		out.seekp(-1, out.cur);
		out << ".";

		ai->TellPlayerNoFacing(requester, out);
	}
	if (command.find("go ") != std::string::npos)
	{
		std::string locationName = command.substr(3);
		WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", locationName);

		if(spellPosition)
			return MoveToSpell(requester, spellPosition, false);

		return true;
	}
	else if (command == "last")
	{
		WorldPosition spellPosition = AI_VALUE(WorldPosition, "see spell location");
		if (spellPosition)
			return MoveToSpell(requester, spellPosition);
	}

    if (command == "jump")
    {
        WorldPosition spellPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
        if (!spellPosition)
        {
            SET_AI_VALUE(std::string, "RTSC next spell action", command);
        }
        else
        {
            WorldPosition jumpPosition = AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
            if (!jumpPosition)
            {
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
                RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
                ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);
                std::ostringstream out;
                out << "Can't finish previous jump! Cancelling...";
                ai->TellError(requester, out.str());
            }
            else
            {
                std::ostringstream out;
                out << "Another jump is in process! Use 'rtsc jump reset' to stop it";
                ai->TellError(requester, out.str());
                return false;
            }
        }

        return true;
    }
    if (command == "jump reset")
    {
        RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump");
        RESET_AI_VALUE2(WorldPosition, "RTSC saved location", "jump point");
        ai->ChangeStrategy("-rtsc jump", BotState::BOT_STATE_NON_COMBAT);

        return true;
    }

	return false;
}
