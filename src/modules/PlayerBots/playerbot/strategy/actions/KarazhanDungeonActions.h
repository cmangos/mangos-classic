#pragma once
#include "DungeonActions.h"
#include "ChangeStrategyAction.h"
#include "UseItemAction.h"

namespace ai
{
    class KarazhanEnableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        KarazhanEnableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable karazhan strategy", "+karazhan") {}
    };

    class KarazhanDisableDungeonStrategyAction : public ChangeAllStrategyAction
    {
    public:
        KarazhanDisableDungeonStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable karazhan strategy", "-karazhan") {}
    };

    class NetherspiteEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NetherspiteEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable netherspite fight strategy", "+netherspite") {}
    };

    class NetherspiteDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        NetherspiteDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable netherspite fight strategy", "-netherspite") {}
    };

    class VoidZoneMoveAwayAction : public MoveAwayFromCreature
    {
    public:
        VoidZoneMoveAwayAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "move away from void zone", 16697, 6.0f) {}
    };

    class RemoveNetherPortalBuffsFromNetherspiteAction : public Action
    {
    public:
        RemoveNetherPortalBuffsFromNetherspiteAction(PlayerbotAI* ai) : Action(ai, "remove nether portal buffs from netherspite") {}
        virtual bool Execute(Event& event) override;
    };

    class AddNetherPortalPerseverenceForTankAction : public Action
    {
    public:
        AddNetherPortalPerseverenceForTankAction(PlayerbotAI* ai) : Action(ai, "add nether portal - perseverence for tank") {}
        bool Execute(Event& event) override
        {
            ai->AddAura(bot, 30421);
            return true;
        }
    };

    class RemoveNetherPortalPerseverenceAction : public Action
    {
    public:
        RemoveNetherPortalPerseverenceAction(PlayerbotAI* ai) : Action(ai, "remove nether portal - perseverence") {}
        bool Execute(Event& event) override
        {
            bot->RemoveAurasDueToSpell(30421);
            return true;
        }
    };

    class RemoveNetherPortalSerenityAction : public Action
    {
    public:
        RemoveNetherPortalSerenityAction(PlayerbotAI* ai) : Action(ai, "remove nether portal - serenity") {}
        bool Execute(Event& event) override
        {
            bot->RemoveAurasDueToSpell(30422);
            return true;
        }
    };

    class RemoveNetherPortalDominanceAction : public Action
    {
    public:
        RemoveNetherPortalDominanceAction(PlayerbotAI* ai) : Action(ai, "remove nether portal - dominance") {}
        bool Execute(Event& event) override
        {
            bot->RemoveAurasDueToSpell(30423);
            return true;
        }
    };

    class PrinceMalchezaarEnableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        PrinceMalchezaarEnableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "enable prince malchezaar fight strategy", "+prince malchezaar") {}
    };

    class PrinceMalchezaarDisableFightStrategyAction : public ChangeAllStrategyAction
    {
    public:
        PrinceMalchezaarDisableFightStrategyAction(PlayerbotAI* ai) : ChangeAllStrategyAction(ai, "disable prince malchezaar fight strategy", "-prince malchezaar") {}
    };

    class NetherspiteInfernalMoveAwayAction : public MoveAwayFromCreature
    {
    public:
        NetherspiteInfernalMoveAwayAction(PlayerbotAI* ai) : MoveAwayFromCreature(ai, "move away from netherspite infernal", 17646, 21.0f) {}
    };
}