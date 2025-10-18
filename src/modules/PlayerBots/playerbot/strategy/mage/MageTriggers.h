#pragma once
#include "playerbot/strategy/triggers/GenericTriggers.h"

namespace ai
{
    DEFLECT_TRIGGER(FireWardTrigger, "fire ward");
    DEFLECT_TRIGGER(FrostWardTrigger, "frost ward");

    class BlinkTrigger : public Trigger
    {
    public:
        BlinkTrigger(PlayerbotAI* ai) : Trigger(ai, "blink", 2) {}

        virtual bool IsActive()
        {
            return bot->HasAuraType(SPELL_AURA_MOD_ROOT) ||
                   bot->HasAuraType(SPELL_AURA_MOD_STUN);
        }
    };

    class ArcaneIntellectOnPartyTrigger : public BuffOnPartyTrigger
    {
    public:
        ArcaneIntellectOnPartyTrigger(PlayerbotAI* ai) : BuffOnPartyTrigger(ai, "arcane intellect", 4) {}
        virtual bool IsActive() { return BuffOnPartyTrigger::IsActive() && !ai->HasAura("arcane brilliance", GetTarget()); }
    };

    class ArcaneIntellectTrigger : public BuffTrigger 
    {
    public:
        ArcaneIntellectTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane intellect", 4) {}
        virtual bool IsActive() { return BuffTrigger::IsActive() && !ai->HasAura("arcane brilliance", GetTarget()); }
    };

    class ArcaneBrillianceOnPartyTrigger : public GreaterBuffOnPartyTrigger
    {
    public:
        ArcaneBrillianceOnPartyTrigger(PlayerbotAI* ai) : GreaterBuffOnPartyTrigger(ai, "arcane brilliance", "arcane intellect", 4) {}
    };

    class AnyMageArmorTrigger : public BuffTrigger 
    {
    public:
        AnyMageArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "any mage armor", 5) {}
        virtual bool IsActive();
    };

    class MageArmorTrigger : public BuffTrigger
    {
    public:
        MageArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mage armor", 5) {}
        virtual bool IsActive();
    };

    class IceArmorTrigger : public BuffTrigger
    {
    public:
        IceArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "ice armor", 5) {}
        virtual bool IsActive();
    };

    class MoltenArmorTrigger : public BuffTrigger
    {
    public:
        MoltenArmorTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "molten armor", 5) {}
        virtual bool IsActive();
    };

    class FireballTrigger : public DebuffTrigger 
    {
    public:
        FireballTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "fireball") {}
	};

    class PyroblastTrigger : public DebuffTrigger 
    {
    public:
        PyroblastTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "pyroblast", 10) {}
    };

    class MissileBarrageTrigger : public HasAuraTrigger 
    {
    public:
        MissileBarrageTrigger(PlayerbotAI* ai) : HasAuraTrigger(ai, "missile barrage") {}
    };

    class ArcaneBlastTrigger : public BuffTrigger 
    {
    public:
        ArcaneBlastTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane blast") {}
    };

    class CounterspellInterruptSpellTrigger : public InterruptSpellTrigger
    {
    public:
        CounterspellInterruptSpellTrigger(PlayerbotAI* ai) : InterruptSpellTrigger(ai, "counterspell") {}
    };

    class CombustionTrigger : public BoostTrigger
    {
    public:
        CombustionTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "combustion") {}
    };

    class IcyVeinsTrigger : public BoostTrigger
    {
    public:
        IcyVeinsTrigger(PlayerbotAI* ai) : BoostTrigger(ai, "icy veins") {}
    };

    BOOST_TRIGGER(WaterElementalBoostTrigger, "summon water elemental");

    class PolymorphTrigger : public HasCcTargetTrigger
    {
    public:
        PolymorphTrigger(PlayerbotAI* ai) : HasCcTargetTrigger(ai, "polymorph") {}
    };

    class RemoveCurseTrigger : public NeedCureTrigger
    {
    public:
        RemoveCurseTrigger(PlayerbotAI* ai) : NeedCureTrigger(ai, "remove curse", DISPEL_CURSE) {}
    };

    class PartyMemberRemoveCurseTrigger : public PartyMemberNeedCureTrigger
    {
    public:
        PartyMemberRemoveCurseTrigger(PlayerbotAI* ai) : PartyMemberNeedCureTrigger(ai, "remove curse", DISPEL_CURSE) {}
    };

    class SpellstealTrigger : public TargetAuraDispelTrigger
    {
    public:
        SpellstealTrigger(PlayerbotAI* ai) : TargetAuraDispelTrigger(ai, "spellsteal", DISPEL_MAGIC) {}
    };

    class CounterspellEnemyHealerTrigger : public InterruptEnemyHealerTrigger
    {
    public:
        CounterspellEnemyHealerTrigger(PlayerbotAI* ai) : InterruptEnemyHealerTrigger(ai, "counterspell") {}
    };

    class ArcanePowerTrigger : public BuffTrigger
    {
    public:
        ArcanePowerTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "arcane power") {}
    };

    class PresenceOfMindTrigger : public BuffTrigger
    {
    public:
        PresenceOfMindTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "presence of mind") {}
    };

    HAS_AURA_TRIGGER(PresenceOfMindAuraTrigger, "presence of mind");

    class ManaShieldTrigger : public BuffTrigger
    {
    public:
        ManaShieldTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "mana shield", 5) {}
        virtual bool IsActive();
    };

    class IceBarrierTrigger : public BuffTrigger
    {
    public:
        IceBarrierTrigger(PlayerbotAI* ai) : BuffTrigger(ai, "ice barrier", 5) {}
    };

    class FireSpellsLocked : public Trigger
    {
    public:
        FireSpellsLocked(PlayerbotAI* ai) : Trigger(ai, "fire spells locked", 2) {}

        virtual bool IsActive()
        {
            return !bot->IsSpellReady(133)      //fireball
                && !bot->IsSpellReady(2948);    //scorch
        }
    };

    class IceLanceTrigger : public Trigger
    {
    public:
        IceLanceTrigger(PlayerbotAI* ai) : Trigger(ai, "ice lance") {}
        bool IsActive() override;
        std::string GetTargetName() override { return "current target"; }
    };

    BUFF_TRIGGER(MirrorImageTrigger, "mirror image");
    BUFF_TRIGGER_A(HotStreakTrigger, "hot streak");
    BUFF_TRIGGER_A(FireballOrFrostfireBoltFreeTrigger, "fireball!");
    DEBUFF_TRIGGER_A(NoImprovedScorchDebuffTrigger, "improved scorch");
    HAS_AURA_TRIGGER(FingersOfFrostTrigger, "fingers of frost");

    class NoFireVulnerabilityTrigger : public Trigger
    {
    public:
        NoFireVulnerabilityTrigger(PlayerbotAI* ai) : Trigger(ai, "no fire vulnerability") {}

        virtual bool IsActive()
        {
            // Required non-player target
            Unit* target = GetTarget();
            if (target && !target->IsPlayer())
            {
                // Required Improved Scorch talent
                if (bot->HasSpell(11095) || bot->HasSpell(12872) || bot->HasSpell(12873))
                {
                    // Check if have max stacks of fire vulnerability expiring in less than 7 seconds
                    Aura* aura = ai->GetAura(22959, target);
                    if (aura)
                    {
                        // Check if the stack is at the maximum
                        const uint32 maxStackAmount = aura->GetSpellProto()->StackAmount;
                        const uint32 stackAmount = aura->GetHolder()->GetStackAmount();
                        if (stackAmount >= maxStackAmount)
                        {
                            // Check if the aura is about to expire
                            if (aura->GetHolder()->GetAuraDuration() > 7000)
                            {
                                return false;
                            }
                        }
                    }

                    return true;
                }                    
            }

            return false;
        }

        virtual std::string GetTargetName() { return "current target"; }
    };

    class LivingBombTrigger : public DebuffTrigger 
    {
    public:
        LivingBombTrigger(PlayerbotAI* ai) : DebuffTrigger(ai, "living bomb") {}
    };

    class ColdSnapTrigger : public Trigger
    {
    public:
        ColdSnapTrigger(PlayerbotAI* ai) : Trigger(ai, "cold snap", 2) {}

        virtual bool IsActive()
        {
            return !bot->IsSpellReady(12472)    //icy veins on cooldown
                && bot->IsSpellReady(11958);    //cold snap not on cooldown
        }
    };

    class NoFoodTrigger : public Trigger
    {
    public:
        NoFoodTrigger(PlayerbotAI* ai) : Trigger(ai, "no food trigger", 5) {}

        virtual bool IsActive()
        {
            return !ai->HasCheat(BotCheatMask::item) && AI_VALUE2(std::list<Item*>, "inventory items", "conjured food").empty();
        }
    };

    class NoDrinkTrigger : public Trigger
    {
    public:
        NoDrinkTrigger(PlayerbotAI* ai) : Trigger(ai, "no drink trigger", 5) {}

        virtual bool IsActive()
        {
            return !ai->HasCheat(BotCheatMask::item) && AI_VALUE2(std::list<Item*>, "inventory items", "conjured water").empty();
        }
    };

    class NoManaGemTrigger : public Trigger
    {
    public:
        NoManaGemTrigger(PlayerbotAI* ai) : Trigger(ai, "no mana gem trigger", 5) {}

        virtual bool IsActive()
        {
            if (!ai->HasCheat(BotCheatMask::item))
            {
                uint32 itemId = 0;
                const uint32 level = bot->GetLevel();
                const std::vector<uint32> manaGemIds = { 5514, 5513, 8007, 8008, 22044, 33312 };
                for (const uint32 manaGemId : manaGemIds)
                {
                    if (bot->HasItemCount(manaGemId, 1))
                    {
                        return false;
                    }
                }

                return true;
            }

            return false;
        }
    };
}
