#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
	class KarazhanEnterDungeonTrigger : public EnterDungeonTrigger
	{
	public:
		KarazhanEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter karazhan", "karazhan", 532) {}
	};

	class KarazhanLeaveDungeonTrigger : public LeaveDungeonTrigger
	{
	public:
		KarazhanLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave karazhan", "karazhan", 532) {}
	};

	class NetherspiteStartFightTrigger : public StartBossFightTrigger
	{
	public:
		NetherspiteStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start netherspite fight", "netherspite", 15689) {}
	};

	class NetherspiteEndFightTrigger : public EndBossFightTrigger
	{
	public:
		NetherspiteEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end netherspite fight", "netherspite", 15689) {}
	};

	class VoidZoneTooCloseTrigger : public CloseToCreatureHazardTrigger
	{
	public:
		VoidZoneTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureHazardTrigger(ai, "void zone too close", 16697, 5.0f, 99999999.0f) {}
	};

	class NetherspiteBeamsCheatNeedRefreshTrigger : public Trigger
	{
	public:
		NetherspiteBeamsCheatNeedRefreshTrigger(PlayerbotAI* ai, int checkInteval = 2) : Trigger(ai, "netherspite beams cheat need refresh", checkInteval) {}
		bool IsActive() override;
	};

	class RemoveNetherPortalDominanceTrigger : public HasAuraTrigger
	{
	public:
		RemoveNetherPortalDominanceTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "remove nether portal - dominance", 20) {}
	};

	class RemoveNetherPortalPerseverenceTrigger : public Trigger
	{
	public:
		RemoveNetherPortalPerseverenceTrigger(PlayerbotAI* ai) : Trigger(ai, "remove nether portal - perseverence", 1) {}
		bool IsActive() override
		{
			//Bot is not tank with aggro
			if (ai->IsTank(bot) && AI_VALUE2(bool, "has aggro", "current target"))
				return false;

			return ai->HasAura(30421, bot);
		}
	};

	class RemoveNetherPortalSerenityTrigger : public HasAuraTrigger
	{
	public:
		RemoveNetherPortalSerenityTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "remove nether portal - serenity", 5) {}
	};

	class PrinceMalchezaarStartFightTrigger : public StartBossFightTrigger
	{
	public:
		PrinceMalchezaarStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start prince malchezaar fight", "prince malchezaar", 15690) {}
	};

	class PrinceMalchezaarEndFightTrigger : public EndBossFightTrigger
	{
	public:
		PrinceMalchezaarEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end prince malchezaar fight", "prince malchezaar", 15690) {}
	};

	class NetherspiteInfernalTooCloseTrigger : public CloseToCreatureTrigger
	{
	public:
		NetherspiteInfernalTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureTrigger(ai, "netherspite infernal too close", 17646, 20.0f) {}
	};
}