#pragma once
#include "GenericActions.h"

namespace ai
{
    class ListSpellsAction : public ChatCommandAction
    {
    public:
        ListSpellsAction(PlayerbotAI* ai, std::string name = "spells") : ChatCommandAction(ai, name) {}
        virtual bool Execute(Event& event) override;
        virtual std::list<std::pair<uint32, std::string> > GetSpellList(std::string filter = "");
        virtual bool isUsefulWhenStunned() override { return true; }

    private:
        static std::map<uint32, SkillLineAbilityEntry const*> skillSpells;
        static std::set<uint32> vendorItems;
    };
}
