#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class ChatValue : public ManualSetValue<ChatMsg>
	{
	public:
        ChatValue(PlayerbotAI* ai, std::string name = "chat") : ManualSetValue<ChatMsg>(ai, CHAT_MSG_WHISPER, name) {}
    };
}
