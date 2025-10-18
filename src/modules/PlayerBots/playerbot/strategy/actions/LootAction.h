#pragma once
#include "playerbot/LootObjectStack.h"
#include "MovementActions.h"

namespace ai
{
    class LootAction : public MovementAction
    {
    public:
        LootAction(PlayerbotAI* ai) : MovementAction(ai, "loot") {}
        virtual bool Execute(Event& event);
    };

    class OpenLootAction : public MovementAction
    {
    public:
        OpenLootAction(PlayerbotAI* ai) : MovementAction(ai, "open loot") {}
        virtual bool Execute(Event& event);

    private:
        bool DoLoot(LootObject& lootObject);
        uint32 GetOpeningSpell(LootObject& lootObject);
        uint32 GetOpeningSpell(LootObject& lootObject, GameObject* go);
        bool CanOpenLock(LootObject& lootObject, const SpellEntry* pSpellInfo, GameObject* go);
        bool CanOpenLock(uint32 skillId, uint32 reqSkillValue);
    };

    class StoreLootAction : public Action
    {
    public:
        StoreLootAction(PlayerbotAI* ai) : Action(ai, "store loot") {}
        virtual bool Execute(Event& event) override;
        static bool IsLootAllowed(ItemQualifier& itemQualifier, PlayerbotAI *ai);
    };

    class ReleaseLootAction : public MovementAction
    {
    public:
        ReleaseLootAction(PlayerbotAI* ai) : MovementAction(ai, "release loot") {}
        virtual bool Execute(Event& event);
    };
}
