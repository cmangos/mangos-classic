#pragma once
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    class KickInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        KickInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "kick") {}
    };

    CAN_CAST_TRIGGER_A(RiposteCastTrigger, "riposte");

    class SliceAndDiceTrigger : public NoBuffAndComboPointsAvailableTrigger
    {
    public:
        SliceAndDiceTrigger(PlayerbotAI* ai, uint8 comboPoints = 3) : NoBuffAndComboPointsAvailableTrigger(ai, "slice and dice", comboPoints) {}
    };

    class RogueBoostBuffTrigger : public BoostTrigger
    {
    public:
        RogueBoostBuffTrigger(PlayerbotAI* ai, std::string spellName) : BoostTrigger(ai, spellName, 200.0f) {}
        virtual bool IsPossible() { return !ai->HasAura("stealth", bot); }
    };

    class RuptureTrigger : public NoDebuffAndComboPointsAvailableTrigger
    {
    public:
        RuptureTrigger(PlayerbotAI* ai, uint8 comboPoints = 4) : NoDebuffAndComboPointsAvailableTrigger(ai, "rupture", comboPoints) {}
    };

    class EviscerateTrigger : public ComboPointsAvailableTrigger
    {
    public:
        EviscerateTrigger(PlayerbotAI* ai, uint8 comboPoints = 4) : ComboPointsAvailableTrigger(ai, comboPoints) {}
    };

    class CloakOfShadowsTrigger : public NeedCureTrigger
    {
    public:
        CloakOfShadowsTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "cloak of shadows", DISPEL_MAGIC) {}
    };

    /*
    class TricksOfTheTradeOnTankTrigger : public BuffOnTankTrigger 
    {
    public:
        TricksOfTheTradeOnTankTrigger(PlayerbotAI* ai) : BuffOnTankTrigger(ai, "tricks of the trade", 1) {}

        virtual bool IsActive() 
        {
            return BuffOnTankTrigger::IsActive() &&
                GetTarget() &&
                !ai->HasAura("tricks of the trade", GetTarget()) &&
#ifdef MANGOS
                (ai->GetBot()->IsInSameGroupWith((Player*)GetTarget()) || ai->GetBot()->IsInSameRaidWith((Player*)GetTarget())) &&
#endif
#ifdef CMANGOS
                (ai->GetBot()->IsInGroup((Player*)GetTarget(), true) || ai->GetBot()->IsInGroup((Player*)GetTarget()))
#endif               
                ;
        }
    };
    */

    class ExposeArmorTrigger : public NoDebuffAndComboPointsAvailableTrigger
    {
    public:
        ExposeArmorTrigger(PlayerbotAI* ai, uint8 comboPoints = 3) : NoDebuffAndComboPointsAvailableTrigger(ai, "expose armor", comboPoints) {}
    };

    class KickInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        KickInterruptEnemyHealerSpellTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "kick") {}
    };

    class InStealthTrigger : public HasAuraTrigger
    {
    public:
        InStealthTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "stealth") {}
    };

    class NoStealthTrigger : public HasNoAuraTrigger
    {
    public:
        NoStealthTrigger(PlayerbotAI* ai) : HasNoAuraTrigger(ai, "stealth") 
        {
            checkInterval = 2;
        }
    };

    class RogueUnstealthTrigger : public BuffTrigger
    {
    public:
        RogueUnstealthTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "stealth", 2) {}

        bool IsActive() override
        {
            if (ai->HasAura("stealth", bot) && !bot->InBattleGround())
            {
                if (!AI_VALUE(bool, "has attackers") && !AI_VALUE(bool, "has enemy player targets"))
                {
                    if (AI_VALUE2(bool, "moving", "self target"))
                    {
                        if (ai->GetMaster())
                        {
                            return sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "master target"), 10.0f);
                        }
                        else
                        {
                            return true;
                        }
                    }
                }
            }

            return false;
        }
    };

    class StealthTrigger : public Trigger 
    {
    public:
        StealthTrigger(PlayerbotAI* ai) : Trigger(ai, "stealth") {}

        virtual bool IsActive()
        {
            if (ai->HasAura("stealth", bot) || sServerFacade.IsInCombat(bot) || !sServerFacade.IsSpellReady(bot, 1784))
            {
                return false;
            }

            Unit* target = AI_VALUE(Unit*, "enemy player target");
            if (!target)
            {
                target = AI_VALUE(Unit*, "grind target");
            }
            if (!target)
            {
                target = AI_VALUE(Unit*, "dps target");
            }
            if (!target)
            {
                return false;
            }

            float distance = 30.0f;
            if (target && target->GetVictim())
            {
                distance -= 10;
            }

            if (sServerFacade.isMoving(target) && target->GetVictim())
            {
                distance -= 10;
            }

            if (bot->InBattleGround())
            {
                distance += 20;
            }

#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
            {
                distance += 20;
            }
#endif

            return (target && sServerFacade.GetDistance2d(bot, target) < distance);
        }
    };

    class SapTrigger : public HasCcTargetTrigger
    {
    public:
        SapTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "sap") {}

        virtual bool IsPossible()
        {
            return bot->GetLevel() > 10 && bot->HasSpell(6770) && !sServerFacade.IsInCombat(bot);
        }
    };

    class SprintTrigger : public BuffTrigger
    {
    public:
        SprintTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "sprint", 2) {}

        virtual bool IsActive()
        {
            if (!sServerFacade.IsSpellReady(bot, 2983))
            {
                return false;
            }

            float distance = ai->GetMaster() ? 45.0f : 35.0f;
            if (ai->HasAura("stealth", bot))
            {
                distance -= 10;
            }

            bool targeted = false;

            Unit* dps = AI_VALUE(Unit*, "dps target");
            if (dps)
            {
                targeted = (dps == AI_VALUE(Unit*, "current target"));
            }

            Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");
            if (enemyPlayer && !targeted)
            {
                targeted = (enemyPlayer == AI_VALUE(Unit*, "current target"));
            }

            // use sprint on players
            if (enemyPlayer && !bot->CanReachWithMeleeAttack(enemyPlayer))
            {
                return true;
            }

            if (!targeted)
            {
                return false;
            }

            if ((dps && sServerFacade.IsInCombat(dps)) || (enemyPlayer))
            {
                distance -= 10;
            }

            return  AI_VALUE2(bool, "moving", "self target") &&
                    (AI_VALUE2(bool, "moving", "dps target") ||
                    AI_VALUE2(bool, "moving", "enemy player target")) &&
                    targeted &&
                    (sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "dps target"), distance) ||
                     sServerFacade.IsDistanceGreaterThan(AI_VALUE2(float, "distance", "enemy player target"), distance));
        }
    };

    class ApplyPoisonTrigger : public Trigger
    {
    public:
        ApplyPoisonTrigger(PlayerbotAI* ai, bool inMainHand, const std::vector<uint32>& inPoisonEnchantIds, std::string name = "apply poison")
        : Trigger(ai, name, 5) 
        , mainHand(inMainHand)
        , poisonEnchantIds(inPoisonEnchantIds) {}

        bool IsActive() override
        {
            Item* weapon = mainHand ? bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND) : bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
            if (weapon)
            {
                const uint32 currentEnchantId = weapon->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT);
                if(currentEnchantId != 0)
                {
                    // Check if the current poison is the poison we need
                    return std::find(poisonEnchantIds.begin(), poisonEnchantIds.end(), currentEnchantId) == poisonEnchantIds.end();
                }

                return true;
            }

            return false;
        }

    private:
        bool mainHand;
        std::vector<uint32> poisonEnchantIds;
    };

    class ApplyDeadlyPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyDeadlyPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 7, 8, 626, 627, 2630, 2642, 2643, 3770, 3771 }, "apply deadly poison main hand") {}
    };

    class ApplyCripplingPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyCripplingPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 22, 603 }, "apply crippling poison main hand") {}
    };

    class ApplyMindPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyMindPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 35, 23, 643 }, "apply mind poison main hand") {}
    };

    class ApplyInstantPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyInstantPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 323, 324, 325, 623, 624, 625, 2641, 3768, 3769 }, "apply instant poison main hand") {}
    };

    class ApplyWoundPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyWoundPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 703, 704, 705, 706, 2644, 3772, 3773 }, "apply wound poison main hand") {}
    };

    class ApplyAnestheticPoisonTrigger : public ApplyPoisonTrigger
    {
    public:
        ApplyAnestheticPoisonTrigger(PlayerbotAI* ai, bool inMainHand) : ApplyPoisonTrigger(ai, inMainHand, { 2640, 3774 }, "apply anesthetic poison main hand") {}
    };
}
