#pragma once

#include "playerbot/strategy/Action.h"
#include "QuestAction.h"
#include "UseItemAction.h"

namespace ai
{
   class ImbueWithStoneAction : public UseAction
   {
   public:
      ImbueWithStoneAction(PlayerbotAI* ai) : UseAction(ai, "apply stone") {}
      bool Execute(Event& event) override;
      bool isUseful() override;
   };

   class ImbueWithOilAction : public UseAction
   {
   public:
      ImbueWithOilAction(PlayerbotAI* ai) : UseAction(ai, "apply oil") {}
      bool Execute(Event& event) override;
      bool isUseful() override;
   };

   class TryEmergencyAction : public Action
   {
   public:
      TryEmergencyAction(PlayerbotAI* ai) : Action(ai, "try emergency") {}
      bool Execute(Event& event) override;
   };
}

static const uint32 uPriorizedHealingItemIds[19] =
{
    HEALTHSTONE_DISPLAYID, FEL_REGENERATION_POTION, SUPER_HEALING_POTION, CRYSTAL_HEALING_POTION, MAJOR_DREAMLESS_SLEEP_POTION, VOLATILE_HEALING_POTION,
    MAJOR_HEALING_POTION, WHIPPER_ROOT_TUBER, NIGHT_DRAGON_BREATH, LIMITED_INVULNERABILITY_POTION, GREATER_DREAMLESS_SLEEP_POTION,
    SUPERIOR_HEALING_POTION, CRYSTAL_RESTORE, DREAMLESS_SLEEP_POTION, GREATER_HEALING_POTION, HEALING_POTION, LESSER_HEALING_POTION, DISCOLORED_HEALING_POTION, MINOR_HEALING_POTION,
};
