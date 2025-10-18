
#include "playerbot/playerbot.h"
#include "SayAction.h"
#include "playerbot/PlayerbotTextMgr.h"
#include "Chat/ChannelMgr.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/AiFactory.h"
#include <regex>
#include <boost/algorithm/string.hpp>
#include "playerbot/PlayerbotLLMInterface.h"

using namespace ai;

std::unordered_set<std::string> noReplyMsgs = { "all ?", "attack", "attack rti", "bank", "c", "co ?", "de ?", "dead ?", "do accept invitation", "faction", "flee", "follow", "give leader", "guard", "guild leave", "help", "home", "items", "join", "jump", "leave", "lfg", "loot", "los", "nc ?", "pet aggressive", "pet defensive", "pet passive", "pet follow", "pet stay", "pet attack", "pet dismiss", "pet call", "pull", "pull rti", "quests", "quests co", "quests in", "quests all", "react ?", "release", "repair", "reset", "reset ai", "reset strats", "revive", "roll feedback", "rtsc", "rtsc cancel", "rtsc select", "skill", "spells", "stats", "stay", "summon", "talents", "talk", "trainer" "trainer learn", "u go", "who" };

std::unordered_set<std::string> noReplyMsgParts = {  };

std::unordered_set<std::string> noReplyMsgStarts = { "@", "accept [", "accept |", "all +", "all -", "b [", "b |", "bank -", "bank [", "bank |", "boost target ", "buff target ", "cast ", "co +", "co -", "cs ", "d [", "d |", "dead +", "dead -", "destroy [", "destroy |", "drop ", "e [", "e |", "emote ", "faction ", "focus heal ", "follow target ", "go npc ", "go zone ", "items ", "jump ", "keep ", "mail ", "nc +", "nc -", "outfit ", "pet autocast ", "q [", "q |", "r [", "r |", "ra ", "range ", "react +", "react -", "repair [", "repair |", "revive target ", "rti ", "rtsc go ", "rtsc save ", "rtsc unsave ", "s [", "s |", "sendmail [", "sendmail |", "share [", "share |", "skill ", "skill unlearn ", "ss ", "t ", "talents ", "u [", "u |", " ue [", "ue |", "wait for attack time " };

SayAction::SayAction(PlayerbotAI* ai) : Action(ai, "say"), Qualified()
{
}

bool SayAction::Execute(Event& event)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target) target = AI_VALUE(Unit*, "current target");

    // set replace std::strings
    if (target) placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->getRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED);
        if (pItem)
        {
            switch (pItem->GetProto()->SubClass)
            {
            case ITEM_SUBCLASS_WEAPON_GUN:
                placeholders["<ammo>"] = "bullets";
                break;
            case ITEM_SUBCLASS_WEAPON_BOW:
            case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                placeholders["<ammo>"] = "arrows";
                break;
            }
        }
    }

    if (bot->IsInWorld())
    {
        if (AreaTableEntry const* area = GetAreaEntryByAreaID(sServerFacade.GetAreaId(bot)))
            placeholders["<subzone>"] = area->area_name[0];
    }

    // set delay before next say
    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    uint32 nextTime = time(0) + urand(1, 30);
    ai->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->getSource();
            PlayerbotAI* memberAi = member->GetPlayerbotAI();
            if (memberAi) members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (std::vector<Player*>::iterator i = members.begin(); i != members.end(); ++i)
        {
            PlayerbotAI* memberAi = (*i)->GetPlayerbotAI();
            if (memberAi)
                memberAi->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!sPlayerbotTextMgr.GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
    else
        bot->Say(text, (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));

    return true;
}


bool SayAction::isUseful()
{
    if (!ai->AllowActivity())
        return false;

    if (ai->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT))
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(0) - lastSaid) > 30;
}

void ChatReplyAction::GetAIChatPlaceholders(std::map<std::string, std::string>& placeholders, Unit* sender, Unit* receiver)
{
    if(receiver)
        placeholders["<receiver name>"] = receiver->GetName();
    else
        placeholders["<receiver name>"];

    if (sender)
        placeholders["<sender name>"] = sender->GetName();
    else
        placeholders["<sender name>"];

#ifdef MANGOSBOT_ZERO
    placeholders["<expansion name>"] = "Vanilla";
#endif
#ifdef MANGOSBOT_ONE
    placeholders["<expansion name>"] = "The Burning Crusade";
#endif
#ifdef MANGOSBOT_TWO
    placeholders["<expansion name>"] = "Wrath of the Lich King";
#endif
    return;
}

void ChatReplyAction::GetAIChatPlaceholders(std::map<std::string, std::string>& placeholders, Unit* unit, const std::string preFix, Player* observer)
{
    placeholders["<" + preFix + " name>"] = unit->GetName();
    placeholders["<" + preFix + " gender>"] = unit->getGender() == GENDER_MALE ? "male" : "female";
    placeholders["<" + preFix + " level>"] = std::to_string(unit->GetLevel());
    placeholders["<" + preFix + " class>"] = ChatHelper::formatClass(unit->getClass());
    placeholders["<" + preFix + " race>"] = ChatHelper::formatRace(unit->getRace());

    FactionTemplateEntry const* factionTemplate = unit->GetFactionTemplateEntry();
    uint32 factionId = factionTemplate ? factionTemplate->faction : 0;

    placeholders["<" + preFix + " faction>"] = ChatHelper::formatFactionName(factionId);
    WorldPosition pos(unit);
    placeholders["<" + preFix + " zone>"] = pos.getAreaName();
    placeholders["<" + preFix + " subzone>"] = pos.getAreaOverride();

    if (unit->IsPlayer())
    {
        placeholders["<" + preFix + " type>"] = "player";
        placeholders["<" + preFix + " subname>"] = "";
        placeholders["<" + preFix + " gossip>"] = "";
    }
    if (unit->IsCreature())
    {
        Creature* creature = (Creature*)unit;

        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature->GetEntry());

        switch (creature->GetCreatureType())
        {
        case CREATURE_TYPE_BEAST:
            placeholders["<" + preFix + " type>"] = "beast";
            break;
        case  CREATURE_TYPE_DRAGONKIN:
            placeholders["<" + preFix + " type>"] = "dragonkin";
            break;
        case      CREATURE_TYPE_DEMON:
            placeholders["<" + preFix + " type>"] = "demon";
            break;
        case    CREATURE_TYPE_ELEMENTAL:
            placeholders["<" + preFix + " type>"] = "elemental";
            break;
        case    CREATURE_TYPE_GIANT:
            placeholders["<" + preFix + " type>"] = "giant";
            break;
        case   CREATURE_TYPE_UNDEAD:
            placeholders["<" + preFix + " type>"] = "undead";
            break;
        case  CREATURE_TYPE_HUMANOID:
            placeholders["<" + preFix + " type>"] = "humanoid";
            break;
        case  CREATURE_TYPE_CRITTER:
            placeholders["<" + preFix + " type>"] = "critter";
            break;
        case  CREATURE_TYPE_MECHANICAL:
            placeholders["<" + preFix + " type>"] = "mechanical";
            break;
        case  CREATURE_TYPE_NOT_SPECIFIED:
            placeholders["<" + preFix + " type>"] = "being";
            break;
        case  CREATURE_TYPE_TOTEM:
            placeholders["<" + preFix + " type>"] = "totem";
            break;
        }

        placeholders["<" + preFix + " subname>"] = creature->GetSubName();

        std::string gossipText = placeholders["<" + preFix + " gossip>"];


        GossipMenusMapBounds pMenuBounds = sObjectMgr.GetGossipMenusMapBounds(creature->GetDefaultGossipMenuId());
        GossipMenuItemsMapBounds pMenuItemBounds = sObjectMgr.GetGossipMenuItemsMapBounds(creature->GetDefaultGossipMenuId());

        for (auto& gossip = pMenuBounds.first; gossip != pMenuBounds.second; gossip++)
        {
            const GossipText* gos = sObjectMgr.GetGossipText(gossip->second.text_id);
            gossipText += " " + gos->Options->Text_0;
        }

        uint32 textId = observer->GetGossipTextId(creature);

        if (textId)
        {
            const GossipText* gos = sObjectMgr.GetGossipText(textId);
            if (gos)
                gossipText += " " + gos->Options->Text_0;
        }

        for (auto& gossip = pMenuItemBounds.first; gossip != pMenuItemBounds.second; gossip++)
        {
            gossipText += " " + gossip->second.option_text;
        }

        std::map<std::string, std::string> replace;
        replace["<"] = "*";
        replace[">"] = "*";
        replace["$N"] = observer->GetName();
        replace["$B"] = "";
        replace["$c"] = ChatHelper::formatRace(observer->getRace());
        replace["$r"] = ChatHelper::formatClass(unit->getClass());
        replace["$g boy : girl;"] = unit->getGender() == GENDER_MALE ? "boy" : "girl"; //Todo replace with regexp
        replace["$g lad : lass;"] = unit->getGender() == GENDER_MALE ? "lass" : "lad";

        replace["GOSSIP_OPTION_GOSSIP"] = unit->GetName() + std::string(" can chat some.");
        replace["GOSSIP_OPTION_QUESTGIVER"] = unit->GetName() + std::string(" can offer quests.");
        replace["GOSSIP_OPTION_VENDOR"] = unit->GetName() + std::string(" can sell and buy items.");
        replace["GOSSIP_OPTION_TAXIVENDOR"] = unit->GetName() + std::string(" is a flight master.");
        replace["GOSSIP_OPTION_TRAINER"] = unit->GetName() + std::string(" can train certain skills.");
        replace["GOSSIP_OPTION_SPIRITHEALER"] = unit->GetName() + std::string(" can revive de dead.");
        replace["GOSSIP_OPTION_SPIRITGUIDE"] = unit->GetName() + std::string(" can revive de dead.");
        replace["GOSSIP_OPTION_INNKEEPER"] = unit->GetName() + std::string(" runs an inn.");
        replace["GOSSIP_OPTION_BANKER"] = unit->GetName() + std::string(" can store items in the bank.");
        replace["GOSSIP_OPTION_PETITIONER"] = unit->GetName() + std::string(" can create new guilds.");
        replace["GOSSIP_OPTION_TABARDDESIGNER"] = unit->GetName() + std::string(" can redesign the guild tabard.");
        replace["GOSSIP_OPTION_BATTLEFIELD"] = unit->GetName() + std::string(" recruits to join the battlegrounds.");
        replace["GOSSIP_OPTION_AUCTIONEER"] = unit->GetName() + std::string(" is an auctioneer.");
        replace["GOSSIP_OPTION_STABLEPET"] = unit->GetName() + std::string(" can store pets.");
        replace["GOSSIP_OPTION_ARMORER"] = unit->GetName() + std::string(" can repair armor.");
        replace["GOSSIP_OPTION_UNLEARNTALENTS"] = unit->GetName() + std::string(" can help unlearning talents.");
        replace["GOSSIP_OPTION_TRAINER"] = unit->GetName() + std::string(" can train pets.");
        replace["GOSSIP_OPTION_UNLEARNPETSKILLS"] = unit->GetName() + std::string(" can help pets unlearn their skills.");

        PlayerbotTextMgr::ReplacePlaceholders(gossipText, replace);

        placeholders["<" + preFix + " gossip>"] = gossipText;
    }
}

WorldPacket ChatReplyAction::GetPacketTemplate(Opcodes op, uint32 type, Unit* sender, Unit* target, std::string channelName)
{
    Player* senderPlayer = (sender->IsPlayer()) ? (Player*)sender : nullptr;
    ObjectGuid senderGuid = sender->GetObjectGuid();
    ObjectGuid targetGuid = target ? target->GetObjectGuid() : ObjectGuid();
    Player* targetPlayer = (target && target->IsPlayer()) ? (Player*)target : nullptr;
    const char* senderName = sender->GetName();

    WorldPacket packetTemplate(op);

    if (op == CMSG_MESSAGECHAT)
        packetTemplate << type;
    else
        packetTemplate << uint8(type);

    if (senderPlayer)
        packetTemplate << ((senderPlayer->GetTeam() == ALLIANCE) ? LANG_COMMON : LANG_ORCISH);
    else if (targetPlayer)
        packetTemplate << ((targetPlayer->GetTeam() == ALLIANCE) ? LANG_COMMON : LANG_ORCISH);
    else
        packetTemplate << LANG_UNIVERSAL;

    if (op == CMSG_MESSAGECHAT)
    {
        if (type == CHAT_MSG_WHISPER)
            packetTemplate << target->GetName();

        if (!channelName.empty())
            packetTemplate << channelName;
    }


    if (op != CMSG_MESSAGECHAT)
    {
        switch (type)
        {
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_RAID_BOSS_WHISPER:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        case CHAT_MSG_MONSTER_EMOTE:
            packetTemplate << ObjectGuid(senderGuid); //Deviation from standards. To support emotes.
            packetTemplate << uint32(strlen(senderName) + 1);
            packetTemplate << senderName;
            packetTemplate << ObjectGuid(targetGuid);
            break;

        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_YELL:
            packetTemplate << ObjectGuid(senderGuid);
            packetTemplate << ObjectGuid(senderGuid);
            break;

        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_YELL:
            MANGOS_ASSERT(senderName);
            packetTemplate << ObjectGuid(senderGuid);
            packetTemplate << uint32(strlen(senderName) + 1);
            packetTemplate << senderName;
            packetTemplate << ObjectGuid(targetGuid);
            break;
        default:
            packetTemplate << ObjectGuid(senderGuid);
            break;
        }
    }
    return packetTemplate;
}

inline void LineToPacket(delayedPackets& delayedPackets, const WorldPacket packetTemplate, const std::string& line, uint32 MsPerChar, bool debug = false)
{
    WorldPacket packet(packetTemplate);
    if (packetTemplate.GetOpcode() != CMSG_MESSAGECHAT)
        packet << uint32(line.size() + 1 + (debug ? 2 : 0));
    packet << ((debug ? "d:" : "") + line);

    if (packetTemplate.GetOpcode() != CMSG_MESSAGECHAT)
        packet << CHAT_TAG_NONE;

    delayedPackets.push_back(std::make_pair(packet, line.size() * MsPerChar));
}

delayedPackets ChatReplyAction::LinesToPackets(const std::vector<std::string>& lines, WorldPacket packetTemplate, bool debug, uint32 MsPerChar, WorldPacket emoteTemplate)
{
    delayedPackets delayedPackets;

    WorldPacket packet;
    for (auto& line : lines)
    {
        bool useEmote = !emoteTemplate.empty() && (line.find("*") == 0 || line.find("[") == 0);

        std::string sentence = line;
        while (sentence.length() > 200) {
            size_t splitPos = sentence.rfind(' ', 200);
            if (splitPos == std::string::npos) {
                splitPos = 200;
            }

            if (!sentence.substr(0, splitPos).empty())
                LineToPacket(delayedPackets, useEmote ? emoteTemplate : packetTemplate, sentence.substr(0, splitPos), MsPerChar, debug);                

            sentence = sentence.substr(splitPos + 1);
        }

        if (!sentence.empty())
        {
            LineToPacket(delayedPackets, useEmote ? emoteTemplate : packetTemplate, sentence, MsPerChar, debug);
        }
    }
    return delayedPackets;
}

delayedPackets ChatReplyAction::GenerateResponsePackets(const std::string json
    , const WorldPacket chatTemplate, const WorldPacket emoteTemplate, const WorldPacket systemTemplate, const std::string startPattern, const std::string endPattern, const std::string deletePattern, const std::string splitPattern, bool debug)
{
    std::vector<std::string> debugLines;

    if (debug)
        debugLines = { json };

    std::string response = PlayerbotLLMInterface::Generate(json, sPlayerbotAIConfig.llmGenerationTimeout, sPlayerbotAIConfig.llmMaxSimultaniousGenerations, debugLines);

    std::vector<std::string> lines = PlayerbotLLMInterface::ParseResponse(response, startPattern, endPattern, deletePattern, splitPattern, debugLines);

    delayedPackets packets, debugPackets;

    packets = LinesToPackets(lines, chatTemplate, false, 200, emoteTemplate);

    if (!debugLines.empty())
    {
        debugPackets = LinesToPackets(debugLines, systemTemplate, true, 1);
        packets.insert(packets.begin(), debugPackets.begin(), debugPackets.end());
    }

    return packets;
}

void ChatReplyAction::ChatReplyDo(Player* bot, uint32 type, uint32 guid1, uint32 guid2, std::string msg, std::string chanName, std::string name)
{
    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.find(msg) != noReplyMsgs.end())
    {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore exact blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    // second one is for partial matches like + or - where we change strats
    if (std::any_of(noReplyMsgParts.begin(), noReplyMsgParts.end(), [&msg](const std::string& part) { return msg.find(part) != std::string::npos; }))
    {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore partial blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);

        return;
    }

    if (std::any_of(noReplyMsgStarts.begin(), noReplyMsgStarts.end(), [&msg](const std::string& start) {
        return msg.find(start) == 0;  // Check if the start matches the beginning of msg
        }))
    {
        //ostringstream out;
        //out << "DEBUG ChatReplyDo decided to ignore start blocklist match" << msg;
        //bot->Say(out.str(), LANG_UNIVERSAL);
        return;
    }

    ChatChannelSource chatChannelSource = bot->GetPlayerbotAI()->GetChatChannelSource(bot, type, chanName);

    if ((boost::algorithm::istarts_with(msg, "LFG") || boost::algorithm::istarts_with(msg, "LFM"))
        && HandleLFGQuestsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    if ((boost::algorithm::istarts_with(msg, "WTB"))
        && HandleWTBItemsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    //toxic links
    if (boost::algorithm::istarts_with(msg, sPlayerbotAIConfig.toxicLinksPrefix)
        && (bot->GetPlayerbotAI()->GetChatHelper()->ExtractAllItemIds(msg).size() > 0 || bot->GetPlayerbotAI()->GetChatHelper()->ExtractAllQuestIds(msg).size() > 0))
    {
        HandleToxicLinksReply(bot, chatChannelSource, msg, name);
        return;
    }

    //thunderfury
    if (bot->GetPlayerbotAI()->GetChatHelper()->ExtractAllItemIds(msg).count(19019))
    {
        HandleThunderfuryReply(bot, chatChannelSource, msg, name);
        return;
    }

    if (bot->GetPlayerbotAI() && sPlayerbotAIConfig.llmEnabled > 0 && (bot->GetPlayerbotAI()->HasStrategy("ai chat", BotState::BOT_STATE_NON_COMBAT) || sPlayerbotAIConfig.llmEnabled == 3) && chatChannelSource != ChatChannelSource::SRC_UNDEFINED && sPlayerbotAIConfig.llmBlockedReplyChannels.find(chatChannelSource) == sPlayerbotAIConfig.llmBlockedReplyChannels.end()
        )
    {
        Player* player = sObjectAccessor.FindPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1));

        PlayerbotAI* ai = bot->GetPlayerbotAI();
        AiObjectContext* context = ai->GetAiObjectContext();

        if (!chanName.empty() && !ai->ChannelHasRealPlayer(chanName))
            player = nullptr;

        std::string llmChannel;

        if (!sPlayerbotAIConfig.llmGlobalContext)
            llmChannel = ((chatChannelSource == ChatChannelSource::SRC_WHISPER) ? name : std::to_string(chatChannelSource));

        std::string llmContext = AI_VALUE(std::string, "manual string::llmcontext" + llmChannel);

        if (player)
        {
            std::string playerName = player->GetName();

            if (player != bot && (player->isRealPlayer() || (sPlayerbotAIConfig.llmBotToBotChatChance && urand(0, 99) < sPlayerbotAIConfig.llmBotToBotChatChance)))
            {
                std::map<std::string, std::string> placeholders;

                GetAIChatPlaceholders(placeholders, bot, player);
                GetAIChatPlaceholders(placeholders, bot, "bot");
                GetAIChatPlaceholders(placeholders, player, "other");

                std::map<ChatChannelSource, std::string> sourceName;
                sourceName[ChatChannelSource::SRC_GUILD] = "in guild chat";
                sourceName[ChatChannelSource::SRC_WORLD] = "in world chat";
                sourceName[ChatChannelSource::SRC_GENERAL] = "in the general channel";
                sourceName[ChatChannelSource::SRC_TRADE] = "in the trade channel";
                sourceName[ChatChannelSource::SRC_LOOKING_FOR_GROUP] = "in looking for group";
                sourceName[ChatChannelSource::SRC_LOCAL_DEFENSE] = "in the local defence channel";
                sourceName[ChatChannelSource::SRC_WORLD_DEFENSE] = "in the world defence channel";
                sourceName[ChatChannelSource::SRC_GUILD_RECRUITMENT] = "in guild recruitement";
                sourceName[ChatChannelSource::SRC_SAY] = "directly";
                sourceName[ChatChannelSource::SRC_WHISPER] = "in private message";
                sourceName[ChatChannelSource::SRC_EMOTE] = "with body language";
                sourceName[ChatChannelSource::SRC_TEXT_EMOTE] = "with an emote";
                sourceName[ChatChannelSource::SRC_YELL] = "with a yell";
                sourceName[ChatChannelSource::SRC_PARTY] = "in party chat";
                sourceName[ChatChannelSource::SRC_RAID] = "in raid chat";

                placeholders["<channel name>"] = sourceName[chatChannelSource];


                placeholders["<initial message>"] = msg;

                std::string llmPromptCustom = AI_VALUE(std::string, "manual saved string::llmdefaultprompt");

                std::map<std::string, std::string> jsonFill;
                jsonFill["<pre prompt>"] = sPlayerbotAIConfig.llmPrePrompt + " " + llmPromptCustom;
                jsonFill["<prompt>"] = sPlayerbotAIConfig.llmPrompt;
                jsonFill["<post prompt>"] = sPlayerbotAIConfig.llmPostPrompt;

                for (auto& prompt : jsonFill)
                {
                    prompt.second = BOT_TEXT2(prompt.second, placeholders);
                }

                uint32 currentLength = jsonFill["<pre prompt>"].size() + jsonFill["<context>"].size() + jsonFill["<prompt>"].size() + llmContext.size();
                PlayerbotLLMInterface::LimitContext(llmContext, currentLength);
                jsonFill["<context>"] = llmContext;

                llmContext += " " + jsonFill["<prompt>"];

                for (auto& prompt : jsonFill)
                {
                    prompt.second = PlayerbotLLMInterface::SanitizeForJson(prompt.second);
                }

                for (auto& prompt : placeholders) //Sanitize now instead of earlier to prevent double Sanitation
                {
                    prompt.second = PlayerbotLLMInterface::SanitizeForJson(prompt.second);
                }

                std::string startPattern, endPattern, deletePattern, splitPattern;
                startPattern = PlayerbotTextMgr::GetReplacePlaceholders(sPlayerbotAIConfig.llmResponseStartPattern, placeholders);
                endPattern = PlayerbotTextMgr::GetReplacePlaceholders(sPlayerbotAIConfig.llmResponseEndPattern, placeholders);
                deletePattern = PlayerbotTextMgr::GetReplacePlaceholders(sPlayerbotAIConfig.llmResponseDeletePattern, placeholders);
                splitPattern = PlayerbotTextMgr::GetReplacePlaceholders(sPlayerbotAIConfig.llmResponseSplitPattern, placeholders);

                std::string json = PlayerbotTextMgr::GetReplacePlaceholders(sPlayerbotAIConfig.llmApiJson, jsonFill);

                json = PlayerbotTextMgr::GetReplacePlaceholders(json, placeholders);

                uint32 type = CHAT_MSG_WHISPER;
                std::string channelName;

                switch (chatChannelSource)
                {
                case ChatChannelSource::SRC_WHISPER:
                {
                    type = CHAT_MSG_WHISPER;
                    break;
                }
                case ChatChannelSource::SRC_SAY:
                {
                    type = CHAT_MSG_SAY;
                    break;
                }
                case ChatChannelSource::SRC_YELL:
                {
                    type = CHAT_MSG_YELL;
                    break;
                }
                case ChatChannelSource::SRC_PARTY:
                {
                    type = CHAT_MSG_PARTY;
                    break;
                }
                case ChatChannelSource::SRC_GUILD:
                {
                    type = CHAT_MSG_GUILD;
                    break;
                }
                case ChatChannelSource::SRC_WORLD:
                case ChatChannelSource::SRC_GENERAL:
                case ChatChannelSource::SRC_TRADE:
                case ChatChannelSource::SRC_LOCAL_DEFENSE:
                case ChatChannelSource::SRC_WORLD_DEFENSE:
                case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
                case ChatChannelSource::SRC_GUILD_RECRUITMENT:
                {
                    type = CHAT_MSG_CHANNEL;
                    channelName = chanName;
                }
                }

                bool debug = bot->GetPlayerbotAI()->HasStrategy("debug llm", BotState::BOT_STATE_NON_COMBAT);

                WorldSession* session = bot->GetSession();

                WorldPacket chatTemplate = GetPacketTemplate(CMSG_MESSAGECHAT, type, bot, player, channelName);
                WorldPacket emoteTemplate = (type == CHAT_MSG_SAY || type == CHAT_MSG_WHISPER) ? GetPacketTemplate(CMSG_MESSAGECHAT, CHAT_MSG_EMOTE, bot, player) : WorldPacket();
                WorldPacket systemTemplate = GetPacketTemplate(CMSG_MESSAGECHAT, CHAT_MSG_WHISPER, bot, player);

                futurePackets futPackets = std::async(std::launch::async, ChatReplyAction::GenerateResponsePackets, json, chatTemplate, emoteTemplate, systemTemplate, startPattern, endPattern, deletePattern, splitPattern, debug);

                ai->SendDelayedPacket(session, std::move(futPackets));
            }
            else if (player != bot || sPlayerbotAIConfig.llmBotToBotChatChance)
            {
                if (msg.find("d:") != std::string::npos)
                    return;

                llmContext = llmContext + " " + playerName + ":" + msg;
                PlayerbotLLMInterface::LimitContext(llmContext, llmContext.size());
            }
            SET_AI_VALUE(std::string, "manual string::llmcontext" + llmChannel, llmContext);

            return;
        }
    }

    SendGeneralResponse(bot, chatChannelSource, GenerateReplyMessage(bot, msg, guid1, name), name);
}

bool ChatReplyAction::HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name)
{
    std::map<std::string, std::string> placeholders;
    ItemPrototype const* thunderfuryProto = sObjectMgr.GetItemPrototype(19019);
    placeholders["%thunderfury_link"] = bot->GetPlayerbotAI()->GetChatHelper()->formatItem(thunderfuryProto);

    std::string responseMessage = BOT_TEXT2("thunderfury_spam", placeholders);

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            bot->GetPlayerbotAI()->SayToWorld(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            bot->GetPlayerbotAI()->SayToGeneral(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_YELL:
        {
            bot->GetPlayerbotAI()->Yell(responseMessage);
            break;
        }
    }

    bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));

    return true;
}

bool ChatReplyAction::HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name)
{
    //quests
    std::vector<uint32> incompleteQuests;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
            incompleteQuests.push_back(questId);
    }

    //items
    std::vector<Item*> botItems = bot->GetPlayerbotAI()->GetInventoryAndEquippedItems();

    //spells
    //?

    std::map<std::string, std::string> placeholders;

    placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? bot->GetPlayerbotAI()->GetChatHelper()->formatItem(botItems[rand() % botItems.size()]) : BOT_TEXT("string_empty_link");
    placeholders["%prefix"] = sPlayerbotAIConfig.toxicLinksPrefix;

    if (incompleteQuests.size() > 0)
    {
        Quest const* quest = sObjectMgr.GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);
        placeholders["%random_taken_quest_or_item_link"] = bot->GetPlayerbotAI()->GetChatHelper()->formatQuest(quest);
    }
    else
    {
        placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
    }

    placeholders["%my_role"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
    AreaTableEntry const* current_area = bot->GetPlayerbotAI()->GetCurrentArea();
    AreaTableEntry const* current_zone = bot->GetPlayerbotAI()->GetCurrentZone();
    placeholders["%area_name"] = current_area ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
    placeholders["%zone_name"] = current_zone ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
    placeholders["%my_class"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot->getClass());
    placeholders["%my_race"] = bot->GetPlayerbotAI()->GetChatHelper()->formatRace(bot->getRace());
    placeholders["%my_level"] = std::to_string(bot->GetLevel());

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            bot->GetPlayerbotAI()->SayToWorld(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            bot->GetPlayerbotAI()->SayToGeneral(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            bot->GetPlayerbotAI()->SayToGuild(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_SAY:
        {
            bot->GetPlayerbotAI()->Say(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_YELL:
        {
            bot->GetPlayerbotAI()->Yell(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_PARTY:
        {
            bot->GetPlayerbotAI()->SayToParty(BOT_TEXT2("suggest_toxic_links", placeholders));
            break;
        }
    }

    bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 60));

    return true;
}

/*
* @return true if message contained item ids
*/
bool ChatReplyAction::HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name)
{
    auto messageItemIds = bot->GetPlayerbotAI()->GetChatHelper()->ExtractAllItemIds(msg);

    if (messageItemIds.empty())
    {
        return false;
    }

    std::set<uint32> matchingItemIds;

    for (auto messageItemId : messageItemIds)
    {
        if (bot->GetPlayerbotAI()->HasItemInInventory(messageItemId))
        {
            matchingItemIds.insert(messageItemId);
        }
    }

    if (!matchingItemIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        AreaTableEntry const* current_area = bot->GetPlayerbotAI()->GetCurrentArea();
        AreaTableEntry const* current_zone = bot->GetPlayerbotAI()->GetCurrentZone();
        placeholders["%area_name"] = current_area ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = bot->GetPlayerbotAI()->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());
        placeholders["%my_role"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%formatted_item_links"] = "";

        for (auto matchingItemId : matchingItemIds)
        {
            ItemPrototype const* proto = sObjectMgr.GetItemPrototype(matchingItemId);
            placeholders["%formatted_item_links"] += bot->GetPlayerbotAI()->GetChatHelper()->formatItem(proto, bot->GetPlayerbotAI()->GetInventoryItemsCountWithId(matchingItemId));
            placeholders["%formatted_item_links"] += " ";
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    bot->GetPlayerbotAI()->SayToWorld(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    bot->GetPlayerbotAI()->SayToGeneral(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_TRADE:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_channel", placeholders);
                    bot->GetPlayerbotAI()->SayToTrade(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2("response_wtb_items_whisper", placeholders);
                    bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                }
                break;
            }
        }
        bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 60));
    }

    return true;
}

/*
* @return true if message contained quest ids
*/
bool ChatReplyAction::HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string msg, std::string name)
{
    auto messageQuestIds = bot->GetPlayerbotAI()->GetChatHelper()->ExtractAllQuestIds(msg);

    if (messageQuestIds.empty())
    {
        return false;
    }

    auto botQuestIds = bot->GetPlayerbotAI()->GetAllCurrentQuestIds();

    std::set<uint32> matchingQuestIds;
    for (auto botQuestId : botQuestIds)
    {
        if (messageQuestIds.count(botQuestId) != 0)
        {
            matchingQuestIds.insert(botQuestId);
        }
    }

    if (!matchingQuestIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        AreaTableEntry const* current_area = bot->GetPlayerbotAI()->GetCurrentArea();
        AreaTableEntry const* current_zone = bot->GetPlayerbotAI()->GetCurrentZone();
        placeholders["%area_name"] = current_area ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_area) : BOT_TEXT("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? bot->GetPlayerbotAI()->GetLocalizedAreaName(current_zone) : BOT_TEXT("string_unknown_area");
        placeholders["%my_class"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot->getClass());
        placeholders["%my_race"] = bot->GetPlayerbotAI()->GetChatHelper()->formatRace(bot->getRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());
        placeholders["%my_role"] = bot->GetPlayerbotAI()->GetChatHelper()->formatClass(bot, AiFactory::GetPlayerSpecTab(bot));
        placeholders["%quest_links"] = "";
        for (auto matchingQuestId : matchingQuestIds)
        {
            Quest const* quest = sObjectMgr.GetQuestTemplate(matchingQuestId);
            placeholders["%quest_links"] += bot->GetPlayerbotAI()->GetChatHelper()->formatQuest(quest);
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2(bot->GetGroup() ? "response_lfg_quests_channel_in_group" : "response_lfg_quests_channel", placeholders);
                    bot->GetPlayerbotAI()->SayToWorld(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2(bot->GetGroup() ? "response_lfg_quests_whisper_in_group" : "response_lfg_quests_whisper", placeholders);
                    bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = BOT_TEXT2(bot->GetGroup() ? "response_lfg_quests_channel_in_group" : "response_lfg_quests_channel", placeholders);
                    bot->GetPlayerbotAI()->SayToGeneral(responseMessage);
                }
                else
                {
                    std::string responseMessage = BOT_TEXT2(bot->GetGroup() ? "response_lfg_quests_whisper_in_group" : "response_lfg_quests_whisper", placeholders);
                    bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
            {
                //do not reply to the chat
                //may whisper
                std::string responseMessage = BOT_TEXT2(bot->GetGroup() ? "response_lfg_quests_whisper_in_group" : "response_lfg_quests_whisper", placeholders);
                bot->GetPlayerbotAI()->Whisper(responseMessage, name);
                break;
            }
        }
        bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));
    }

    return true;
}

bool ChatReplyAction::SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string responseMessage, std::string name)
{
    // send responds
    switch (chatChannelSource)
    {
    case ChatChannelSource::SRC_WORLD:
    {
        //may reply to the same channel or whisper
        bot->GetPlayerbotAI()->SayToWorld(responseMessage);
        break;
    }
    case ChatChannelSource::SRC_GENERAL:
    {
        //may reply to the same channel or whisper
        //bot->GetPlayerbotAI()->SayToGeneral(responseMessage);
        bot->GetPlayerbotAI()->Whisper(responseMessage, name);
        break;
    }
    case ChatChannelSource::SRC_TRADE:
    {
        //do not reply to the chat
        //may whisper
        break;
    }
    case ChatChannelSource::SRC_LOCAL_DEFENSE:
    {
        //may reply to the same channel or whisper
        bot->GetPlayerbotAI()->SayToLocalDefense(responseMessage);
        break;
    }
    case ChatChannelSource::SRC_WORLD_DEFENSE:
    {
        //may reply only if rank 11+ for MANGOSBOT_ZERO, may always reply for others
        //may whisper
        break;
    }
    case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
    {
        //do not reply to the chat
        //may whisper
        break;
    }
    case ChatChannelSource::SRC_GUILD_RECRUITMENT:
    {
        //do not reply to the chat
        //may whisper
        break;
    }
    case ChatChannelSource::SRC_WHISPER:
    {
        bot->GetPlayerbotAI()->Whisper(responseMessage, name);
        break;
    }
    case ChatChannelSource::SRC_SAY:
    {
        bot->GetPlayerbotAI()->Say(responseMessage);
        break;
    }
    case ChatChannelSource::SRC_YELL:
    {
        bot->GetPlayerbotAI()->Yell(responseMessage);
        break;
    }
    case ChatChannelSource::SRC_GUILD:
    {
        bot->GetPlayerbotAI()->SayToGuild(responseMessage);
        break;
    }
    default:
        break;
    }
    bot->GetPlayerbotAI()->GetAiObjectContext()->GetValue<time_t>("last said", "chat")->Set(time(0) + urand(5, 25));

    return true;
}

std::string ChatReplyAction::GenerateReplyMessage(Player* bot, std::string incomingMessage, uint32 guid1, std::string name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand

    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(incomingMessage);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (incomingMessage.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = sObjectMgr.GetPlayer(ObjectGuid(HIGHGUID_PLAYER, guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2; // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3; // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1; // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (incomingMessage.find(bot->GetName()) == std::string::npos)
                {
                    continue; // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found || urand(0, 4))
    {
        // Name Responds
        if (incomingMessage.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else if (!found) // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // load text if needed
    if (respondsText.empty())
    {
        respondsText = BOT_TEXT2(replyType, name);
    }

    if (respondsText.size() > 255)
    {
        respondsText.resize(255);
    }

    return respondsText;

}

bool ChatReplyAction::isUseful()
{
    return !ai->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT);
}