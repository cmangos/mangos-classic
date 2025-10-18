
#include "playerbot/playerbot.h"
#include "ChangeChatAction.h"

using namespace ai;

bool ChangeChatAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string text = event.getParam();
    ChatMsg parsed = chat->parseChat(text);
    if (parsed == CHAT_MSG_SYSTEM)
    {
        std::ostringstream out; out << "Current chat is " << chat->formatChat(*context->GetValue<ChatMsg>("chat"));
        ai->TellPlayer(requester, out);
    }
    else
    {
        context->GetValue<ChatMsg>("chat")->Set(parsed);
        std::ostringstream out; out << "Chat set to " << chat->formatChat(parsed);
        ai->TellPlayer(requester, out);
    }
    
    return true;
}
