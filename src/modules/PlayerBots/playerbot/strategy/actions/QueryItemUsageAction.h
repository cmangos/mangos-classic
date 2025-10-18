#pragma once
#include "GenericActions.h"

namespace ai
{
    class ItemQualifier;

    class QueryItemUsageAction : public ChatCommandAction
    {
    public:
        QueryItemUsageAction(PlayerbotAI* ai, std::string name = "query item usage") : ChatCommandAction(ai, name) {}
        virtual bool isUsefulWhenStunned() override { return true; }
    protected:
        virtual bool Execute(Event& event) override;
        uint32 GetCount(ItemQualifier& qualifier);
        std::string QueryItem(ItemQualifier& qualifier, uint32 count, uint32 total);
        std::string QueryItemUsage(ItemQualifier& qualifier);
        std::string QueryItemPrice(ItemQualifier& qualifier);
        std::string QueryQuestItem(uint32 itemId, const Quest *questTemplate, const QuestStatusData *questStatus);
        std::string QueryQuestItem(uint32 itemId);
        std::string QueryItemPower(ItemQualifier& qualifier);

    private:
        std::ostringstream out;
    };
}
