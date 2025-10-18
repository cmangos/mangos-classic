
#include "playerbot/playerbot.h"
#include "HelpAction.h"
#include "ChatActionContext.h"
#include "playerbot/PlayerbotHelpMgr.h"

using namespace ai;

HelpAction::HelpAction(PlayerbotAI* ai) : ChatCommandAction(ai, "help")
{
    chatContext = new ChatActionContext();
}

HelpAction::~HelpAction()
{
    delete chatContext;
}

bool HelpAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string param = event.getParam();
    std::string helpTopic;

    if(param.find("Hvalue:help") != std::string::npos)
    {
        helpTopic = ChatHelper::parseValue("help",param);
    }
    else if (param.find("[h:") != std::string::npos)
    {
        helpTopic = param.substr(3,param.size()-4);
    }

    if (param.empty())
    {
        helpTopic = "help:main";
    }

    std::ostringstream out;
    std::string helpTekst = sPlayerbotHelpMgr.GetBotText(helpTopic);

    if (helpTopic.empty() && !param.empty())
    {
        std::vector<std::string> list = sPlayerbotHelpMgr.FindBotText(param);

        if(list.size() == 1)
            helpTekst = sPlayerbotHelpMgr.GetBotText(list.front());
        else if (!list.empty())
        {
            std::sort(list.begin(), list.end());
            helpTekst = sPlayerbotHelpMgr.GetBotText("help:search");

            std::string topics = sPlayerbotHelpMgr.makeList(list, "[h:<part>|<part>]");

            if (!helpTekst.empty())
                sPlayerbotHelpMgr.replace(helpTekst, "<found>", topics);
            else
                helpTekst = topics; //Fallback for old help table.

            sPlayerbotHelpMgr.FormatHelpTopic(helpTekst);
        }
    }

    if (!helpTekst.empty())
    {
        std::vector<std::string> lines = Qualified::getMultiQualifiers(helpTekst, "\n");

        for (auto& line : lines)
        {
            ai->TellPlayerNoFacing(requester, line, PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL, true, false);
        }

        return true;
    }
    
    TellChatCommands(requester);
    TellStrategies(requester);

    return true;
}

void HelpAction::TellChatCommands(Player* requester)
{
    std::ostringstream out;
    out << "Whisper any of: ";
    out << CombineSupported(chatContext->supports());
    out << ", [item], [quest] or [object] link";
    ai->TellPlayer(requester, out.str());
}

void HelpAction::TellStrategies(Player* requester)
{
    std::ostringstream out;
    out << "Possible strategies (co/nc/dead commands): ";
    out << CombineSupported(ai->GetAiObjectContext()->GetSupportedStrategies());
    ai->TellPlayer(requester, out.str());
}

std::string HelpAction::CombineSupported(std::set<std::string> commands)
{
    std::ostringstream out;

    for (std::set<std::string>::iterator i = commands.begin(); i != commands.end(); )
	{
        out << *i;
		if (++i != commands.end())
			out << ", ";
	}

    return out.str();
}
