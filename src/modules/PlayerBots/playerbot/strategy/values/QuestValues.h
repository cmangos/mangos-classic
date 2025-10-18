#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/TravelMgr.h"

namespace ai
{          
    class GuidPosition;

    class QuestObjectMgr : ObjectMgr
    {
    public:
        QuestRelationsMap& GetCreatureQuestRelationsMap() { return m_CreatureQuestRelations; }
        QuestRelationsMap& GetCreatureQuestInvolvedRelationsMap() { return m_CreatureQuestInvolvedRelations; }
        QuestRelationsMap& GetGOQuestRelationsMap() { return m_GOQuestRelations; }
        QuestRelationsMap& GetGOQuestInvolvedRelationsMap() { return m_GOQuestInvolvedRelations; }
    };

    enum class QuestRewardOptionType : uint8
    {
        QUEST_REWARD_CONFIG_DRIVEN = 0x00,
        QUEST_REWARD_OPTION_AUTO = 0x01,
        QUEST_REWARD_OPTION_LIST = 0x02,
        QUEST_REWARD_OPTION_ASK = 0x03,
    };

    //EntryQuestRelationMap[entry][questId] = QuestRelationFlag
    typedef std::unordered_map<uint32, uint32> questRelationMap;
    typedef std::unordered_map<int32, questRelationMap> EntryQuestRelationMap;

    //questGuidpMap[questId][QuestRelationFlag][entry] = {GuidPosition}
    typedef std::unordered_map<int32, std::list<GuidPosition>> questEntryGuidps;
    typedef std::unordered_map<uint32, questEntryGuidps> questRelationGuidps;
    typedef std::unordered_map<uint32, questRelationGuidps> questGuidpMap;    

    //questGiverMap[questId] = {GuidPosition}
    typedef std::unordered_map<uint32, std::list<GuidPosition>> questGiverMap;
    
    
    //Returns the quest relation Flags for all entries and quests
    class EntryQuestRelationMapValue : public SingleCalculatedValue<EntryQuestRelationMap>
    {
    public:
        EntryQuestRelationMapValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "entry quest relation map") {}

        virtual EntryQuestRelationMap Calculate() override;
    };
        
    //Generic quest object finder
    class FindQuestObjectData
    {
    public:
        FindQuestObjectData() { GetObjectiveEntries(); }

        void GetObjectiveEntries();
        bool operator()(CreatureDataPair const& dataPair);
        bool operator()(GameObjectDataPair const& dataPair);
        questGuidpMap GetResult() const { return data; };
    private:
        std::unordered_map<int32, std::vector<std::pair<uint32, TravelDestinationPurpose>>> entryMap;
        std::unordered_map<uint32, std::vector<std::pair<uint32, TravelDestinationPurpose>>> itemMap;

        EntryQuestRelationMap relationMap;

        questGuidpMap data;
    };
   
    //All objects to start, do or finish a quest.
    class QuestGuidpMapValue : public SingleCalculatedValue<questGuidpMap>
    {
    public:
        QuestGuidpMapValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "quest guidp map") {}

        virtual questGuidpMap Calculate() override;
    };       
    
    //All questgivers and their quests that are useful for a specific level
    class QuestGiversValue : public SingleCalculatedValue<questGiverMap>, public Qualified
	{
	public:
        QuestGiversValue(PlayerbotAI* ai) : SingleCalculatedValue(ai, "quest givers"), Qualified() {}
        virtual questGiverMap Calculate() override;
    };
    
    //All questgivers that have a quest for the bot.
    class ActiveQuestGiversValue : public CalculatedValue<std::list<GuidPosition>>
    {
    public:
        ActiveQuestGiversValue(PlayerbotAI* ai) : CalculatedValue(ai, "active quest givers", 5) {}
        virtual std::list<GuidPosition> Calculate() override;
    };    
    
    //All quest takers that the bot has a quest for.
    class ActiveQuestTakersValue : public CalculatedValue<std::list<GuidPosition>>
    {
    public:
        ActiveQuestTakersValue(PlayerbotAI* ai) : CalculatedValue(ai, "active quest takers", 5) {}
        virtual std::list<GuidPosition> Calculate() override;
    };

    //All objectives that the bot still has to complete.
    class ActiveQuestObjectivesValue : public CalculatedValue<std::list<GuidPosition>>
    {
    public:
        ActiveQuestObjectivesValue(PlayerbotAI* ai) : CalculatedValue(ai, "active quest objectives", 5) {}
        virtual std::list<GuidPosition> Calculate() override;
    };

    //Check if a specific entry is a quest objective that the bot has to complete.
    class NeedForQuestValue : public BoolCalculatedValue, public Qualified
    {
    public:
        NeedForQuestValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "need for quest", 5), Qualified() {}
        virtual bool Calculate() override;
    };
    
    //Free quest log slots
    class FreeQuestLogSlotValue : public Uint8CalculatedValue
    {
    public:
        FreeQuestLogSlotValue(PlayerbotAI* ai) : Uint8CalculatedValue(ai, "free quest log slots", 2) {}
        virtual uint8 Calculate() override;
    };
    
    //Dialog status npc 
    class DialogStatusValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        DialogStatusValue(PlayerbotAI* ai, std::string name = "dialog status") : Uint32CalculatedValue(ai, name , 2), Qualified() {}
        static uint32 getDialogStatus(Player* bot, int32 questgiver, uint32 questId = 0);
        virtual uint32 Calculate() override  { return getDialogStatus(bot, stoi(getQualifier())); }
    };

    //Dialog status npc quest
    class DialogStatusQuestValue : public DialogStatusValue
    {
    public:
        DialogStatusQuestValue(PlayerbotAI* ai) : DialogStatusValue(ai, "dialog status quest") {}
        virtual uint32 Calculate() override { return getDialogStatus(bot, getMultiQualifierInt(getQualifier(), 0, " "), getMultiQualifierInt(getQualifier(), 1, " ")); }
    };

    //Can accept quest from npc
    class CanAcceptQuestValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanAcceptQuestValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can accept quest npc"), Qualified() {}
        virtual bool Calculate() override  { return AI_VALUE2(uint32, "dialog status", getQualifier()) == DIALOG_STATUS_AVAILABLE;};
    };

    //Can accept low level quest from npc
    class CanAcceptQuestLowLevelValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanAcceptQuestLowLevelValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can accept quest low level npc"), Qualified() {}

        virtual bool Calculate()
        {
            uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
#ifndef MANGOSBOT_TWO
            return dialogStatus == DIALOG_STATUS_CHAT;
#else
            return dialogStatus == DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
#endif   
        };
    };

    //Has good reward
    class NeedQuestRewardValue : public BoolCalculatedValue, public Qualified
    {
    public:
        NeedQuestRewardValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "need quest reward"), Qualified() {}
        virtual bool Calculate() override;
    };

    //Can hand in quest to npc
    class CanTurnInQuestValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanTurnInQuestValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can turn in quest npc"), Qualified() {}

        virtual bool Calculate() override
        {
            uint32 dialogStatus = AI_VALUE2(uint32, "dialog status", getQualifier());
#ifdef MANGOSBOT_ZERO  
            return dialogStatus == DIALOG_STATUS_REWARD2;
#else
            return dialogStatus == DIALOG_STATUS_REWARD2 || dialogStatus == DIALOG_STATUS_REWARD;
#endif  
        };
    };

    //Repeatable in quest to npc
    class CanRepeatQuestValue : public BoolCalculatedValue, public Qualified
    {
    public:
        CanRepeatQuestValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can repeat quest npc"), Qualified() {}

        virtual bool Calculate() override
        {
            return AI_VALUE2(uint32, "dialog status", getQualifier()) == DIALOG_STATUS_REWARD_REP;
        };
    };

    //Need objective
    class NeedQuestObjectiveValue : public BoolCalculatedValue, public Qualified
    {
    public:
        NeedQuestObjectiveValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "need quest objective"), Qualified() {}
        static bool CanGetItemSomewhere(const uint32 itemId, const uint32 reqCount, Player* bot);
        virtual bool Calculate() override;
    };

    //Can use item on
    class CanUseItemOn : public BoolCalculatedValue, public Qualified
    {
    public:
        CanUseItemOn(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can use item on"), Qualified() {}
        virtual bool Calculate() override;
    };

    // quest reward
    class QuestRewardValue : public ManualSetValue<uint8>, public Qualified
    {
    public:
        QuestRewardValue(PlayerbotAI* ai) :
            ManualSetValue<uint8>(ai, static_cast<uint8>(QuestRewardOptionType::QUEST_REWARD_CONFIG_DRIVEN), "quest reward"),
            Qualified() {}
    };

    class HasNearbyQuestTakerValue : public BoolCalculatedValue
    {
    public:
        HasNearbyQuestTakerValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "has nearby quest taker", 30) {}

        virtual bool Calculate() override;
    };
}
