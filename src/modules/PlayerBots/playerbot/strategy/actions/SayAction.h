#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"

namespace ai
{
    class SayAction : public Action, public Qualified
    {
    public:
        SayAction(PlayerbotAI* ai);
        virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual std::string getName() { return "say::" + qualifier; }
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
    };

    typedef std::pair<WorldPacket, uint32> delayedPacket;
    typedef std::vector<delayedPacket> delayedPackets;
    typedef std::future<delayedPackets> futurePackets;

    class ChatReplyAction : public Action
    {
    public:
        ChatReplyAction(PlayerbotAI* ai) : Action(ai, "chat message") {}
        virtual bool Execute(Event& event) { return true; }
        bool isUseful();
        virtual bool isUsefulWhenStunned() override { return true; }

        static void GetAIChatPlaceholders(std::map<std::string, std::string>& placeholders, Unit* sender = nullptr, Unit* receiver = nullptr);
        static void GetAIChatPlaceholders(std::map<std::string, std::string>& placeholders, Unit* unit, const std::string preFix = "bot", Player* observer = nullptr);
        static WorldPacket GetPacketTemplate(Opcodes op, uint32 type, Unit* sender, Unit* target = nullptr, std::string channelName = "");
        static delayedPackets LinesToPackets(const std::vector<std::string>& lines, WorldPacket packetTemplate, bool debug = false, uint32 MsPerChar = 0, WorldPacket emoteTemplate = WorldPacket());

        static delayedPackets GenerateResponsePackets(const std::string json
            , const WorldPacket chatTemplate, const WorldPacket emoteTemplate, const WorldPacket systemTemplate, const std::string startPattern, const std::string endPattern, const std::string deletePattern, const std::string splitPattern, bool debug = false);

        static void ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name);
        static bool HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name);
        static bool HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name);
        static bool HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name);
        static bool HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name);
        static bool SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string responseMessage, std::string name);
        static std::string GenerateReplyMessage(Player* bot, std::string incomingMessage, uint32 guid1, std::string name);
    };
}
