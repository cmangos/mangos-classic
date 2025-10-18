#pragma once
#include "DungeonTriggers.h"
#include "GenericTriggers.h"

namespace ai
{
    class MoltenCoreEnterDungeonTrigger : public EnterDungeonTrigger
    {
    public:
        MoltenCoreEnterDungeonTrigger(PlayerbotAI* ai) : EnterDungeonTrigger(ai, "enter molten core", "molten core", 409) {}
    };

    class MoltenCoreLeaveDungeonTrigger : public LeaveDungeonTrigger
    {
    public:
        MoltenCoreLeaveDungeonTrigger(PlayerbotAI* ai) : LeaveDungeonTrigger(ai, "leave molten core", "molten core", 409) {}
    };

    class MagmadarStartFightTrigger : public StartBossFightTrigger
    {
    public:
        MagmadarStartFightTrigger(PlayerbotAI* ai) : StartBossFightTrigger(ai, "start magmadar fight", "magmadar", 11982) {}
    };

    class MagmadarEndFightTrigger : public EndBossFightTrigger
    {
    public:
        MagmadarEndFightTrigger(PlayerbotAI* ai) : EndBossFightTrigger(ai, "end magmadar fight", "magmadar", 11982) {}
    };

    class MagmadarLavaBombTrigger : public CloseToGameObjectHazardTrigger
    {
    public:
        MagmadarLavaBombTrigger(PlayerbotAI* ai) : CloseToGameObjectHazardTrigger(ai, "magmadar lava bomb", 177704, 5.0f, 60) {}
    };

    class MagmadarTooCloseTrigger : public CloseToCreatureTrigger
    {
    public:
        MagmadarTooCloseTrigger(PlayerbotAI* ai) : CloseToCreatureTrigger(ai, "magmadar too close", 11982, 30.0f) {}
    };

    class FireProtectionPotionReadyTrigger : public ItemBuffReadyTrigger
    {
    public:
        FireProtectionPotionReadyTrigger(PlayerbotAI* ai) : ItemBuffReadyTrigger(ai, "fire protection potion ready", 13457, 17543) {}
    };

    class MCRuneInSightTrigger : public ValueTrigger
    {
    public:
        MCRuneInSightTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune in sight", 1)
        {
            qualifier = "and::{"
                "action possible::use id::17333,"
                "has object::go usable filter::go trapped filter::entry filter::{gos in sight,mc runes},"
                "not::has object::entry filter::{gos close,mc runes}"
                "}";
        }
    };

    class MCRuneCloseTrigger : public ValueTrigger
    {
    public:
        MCRuneCloseTrigger(PlayerbotAI* ai) : ValueTrigger(ai, "mc rune close", 1) { qualifier = "has object::go usable filter::entry filter::{gos close,mc runes}"; }
    };
}