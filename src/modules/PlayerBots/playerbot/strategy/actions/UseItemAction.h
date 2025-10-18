#pragma once
#include "GenericActions.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/RandomItemMgr.h"

namespace ai
{
    //This class bypasses the requirement for a bot to have a key item in their inventory when opening a lock.
    class BotUseItemSpell : public Spell 
    {
    public:
        BotUseItemSpell(WorldObject* caster, SpellEntry const* info, uint32 triggeredFlags, ObjectGuid originalCasterGUID = ObjectGuid(), SpellEntry const* triggeredBy = nullptr, bool itemCheats = false) : Spell(caster, info, triggeredFlags, originalCasterGUID, triggeredBy), itemCheats(itemCheats) {};
        SpellCastResult ForceSpellStart(SpellCastTargets const* targets, Aura* triggeredByAura = nullptr);
        bool OpenLockCheck();

    private:
        bool itemCheats;
    };

    class UseAction : public ChatCommandAction, public Qualified
    {
    public:
        UseAction(PlayerbotAI* ai, std::string name = "use", uint32 duration = sPlayerbotAIConfig.reactDelay) : ChatCommandAction(ai, name, duration), Qualified() {}

    public:
        // Used when this action is executed as a reaction
        virtual bool ShouldReactionInterruptCast() const override { return true; }

    protected:
        virtual bool Execute(Event& event) override;
        bool UseItem(Player* requester, uint32 itemId, Unit* target = nullptr);
        bool UseItem(Player* requester, uint32 itemId, GameObject* target);
        bool UseItem(Player* requester, uint32 itemId, Item* target);
        bool UseGameObject(Player* requester, Event& event, GameObject* gameObject);
        
        //void TellConsumableUse(Player* requester, Item* item, std::string action, float percent);

        bool HasItemCooldown(uint32 itemId) const;

    private:
        bool UseItemInternal(Player* requester, uint32 itemId, Unit* target, GameObject* gameObjectTarget, Item* itemTarget);
        bool UseQuestGiverItem(Player* requester, Item* item);
        bool OpenItem(Player* requester, Item* item);
#ifndef MANGOSBOT_ZERO
        bool UseGemItem(Player* requester, Item* item, Item* gem, bool replace = false);
#endif
    };

    class UseItemIdAction : public UseAction
    {
    public:
        UseItemIdAction(PlayerbotAI* ai, std::string name = "use id", uint32 duration = sPlayerbotAIConfig.reactDelay) : UseAction(ai, name, duration) {}
        virtual bool isPossible() override;
        virtual bool isUseful() override;

    protected:
        virtual bool Execute(Event& event) override;
        virtual uint32 GetItemId() { return getQualifier().empty() ? 0 : getMultiQualifierInt(getQualifier(),0, ","); }
        virtual Unit* GetTarget() override { return nullptr; }
    };

    class UseTargetedItemIdAction : public UseItemIdAction
    {
    public:
        UseTargetedItemIdAction(PlayerbotAI* ai, std::string name, uint32 duration = sPlayerbotAIConfig.reactDelay) : UseItemIdAction(ai, name, duration) {}
        virtual Unit* GetTarget() override { return Action::GetTarget(); }
        virtual uint32 GetItemId() override { return  0; }
    };

    class UseSpellItemAction : public UseAction 
    {
    public:
        UseSpellItemAction(PlayerbotAI* ai, std::string name) : UseAction(ai, name) {}
        virtual bool isUseful() override;
    };

    class UsePotionAction : public UseItemIdAction
    {
    public:
        UsePotionAction(PlayerbotAI* ai, std::string name, SpellEffects effect) : UseItemIdAction(ai, name), effect(effect) {}

        bool isUseful() override { return UseItemIdAction::isUseful() && AI_VALUE2(bool, "combat", "self target"); }

        virtual uint32 GetItemId() override
        {
            std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", getName());
            if (items.empty())
            {
                return sRandomItemMgr.GetRandomPotion(bot->GetLevel(), effect);
            }

            return items.front()->GetProto()->ItemId;
        }

        bool Execute(Event& event) override
        {
            // Check the chance of using a potion (only in pvp)
#ifdef MANGOSBOT_ZERO
            const bool shouldUsePotion = !ai->IsInPvp() || frand(0.0f, 1.0f) < sPlayerbotAIConfig.usePotionChance;
#else
            const bool shouldUsePotion = !bot->InArena() && (!ai->IsInPvp() || frand(0.0f, 1.0f) < sPlayerbotAIConfig.usePotionChance);
#endif
            if (shouldUsePotion)
            {
                return UseItemIdAction::Execute(event);
            }
            else
            {
                // Force potion cooldown to prevent spamming this action
                const ItemPrototype* proto = sObjectMgr.GetItemPrototype(GetItemId());
                if (proto)
                {
                    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
                    {
                        _Spell const& spellData = proto->Spells[i];
                        if (spellData.SpellId)
                        {
                            // wrong triggering type
#ifdef MANGOSBOT_ZERO
                            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
                            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
#endif
                            {
                                continue;
                            }

                            const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
                            if (spellInfo)
                            {
                                bot->RemoveSpellCooldown(*spellInfo, false);
                                bot->AddCooldown(*spellInfo, proto, false);
                                break;
                            }
                        }
                    }
                }
            }

            return true;
        }

    private:
        SpellEffects effect;
    };

    class UseHealingPotionAction : public UsePotionAction
    {
    public:
        UseHealingPotionAction(PlayerbotAI* ai) : UsePotionAction(ai, "healing potion", SPELL_EFFECT_HEAL) {}
    };

    class UseManaPotionAction : public UsePotionAction
    {
    public:
        UseManaPotionAction(PlayerbotAI* ai) : UsePotionAction(ai, "mana potion", SPELL_EFFECT_ENERGIZE) {}
    };

    class UseHearthStoneAction : public UseAction
    {
    public:
        UseHearthStoneAction(PlayerbotAI* ai) : UseAction(ai, "hearthstone", 10000U) {}

        virtual bool Execute(Event& event) override;

        bool isUseful() override;
    
        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseHealthstoneAction : public UseItemIdAction
    {
    public:
        UseHealthstoneAction(PlayerbotAI* ai) : UseItemIdAction(ai, "healthstone") {}

        bool isUseful() override { return UseItemIdAction::isUseful() && AI_VALUE2(bool, "combat", "self target"); }

        uint32 GetItemId() override
        {
            std::list<Item*> items = AI_VALUE2(std::list<Item*>, "inventory items", getName());
            if (items.empty())
            {
                const uint32 level = bot->GetLevel();
                if(level < 12)
                {
                    return 5512;
                }
                else if(level >= 12 && level < 24)
                {
                    return 5511;
                }
                else if(level >= 24 && level < 36)
                {
                    return 5509;
                }
                else if(level >= 36 && level < 48)
                {
                    return 5510;
                }
                else if(level >= 48 && level < 61)
                {
                    return 9421;
                }
                else if(level >= 61 && level < 63)
                {
                    return 22103;
                }
                else if(level >= 63 && level < 71)
                {
                    return 36889;
                }
                else
                {
                    return 36892;
                }
            }

            return items.front()->GetProto()->ItemId;
        }

        bool Execute(Event& event) override
        {
            // Check the chance of using a healthstone (only in pvp)
            const bool shouldUsePotion = !ai->IsInPvp() || frand(0.0f, 1.0f) < sPlayerbotAIConfig.usePotionChance;
            if (shouldUsePotion)
            {
                return UseItemIdAction::Execute(event);
            }
            else
            {
                // Force potion cooldown to prevent spamming this action
                const ItemPrototype* proto = sObjectMgr.GetItemPrototype(GetItemId());
                if (proto)
                {
                    for (int i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
                    {
                        _Spell const& spellData = proto->Spells[i];
                        if (spellData.SpellId)
                        {
                            // wrong triggering type
#ifdef MANGOSBOT_ZERO
                            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE && spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_NO_DELAY_USE)
#else
                            if (spellData.SpellTrigger != ITEM_SPELLTRIGGER_ON_USE)
#endif
                            {
                                continue;
                            }

                            const SpellEntry* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellData.SpellId);
                            if (spellInfo)
                            {
                                bot->RemoveSpellCooldown(*spellInfo, false);
                                bot->AddCooldown(*spellInfo, proto, false);
                                break;
                            }
                        }
                    }
                }
            }

            return true;
        }
    };

    class UseWhipperRootTuberAction : public UseItemIdAction
    {
    public:
        UseWhipperRootTuberAction(PlayerbotAI* ai) : UseItemIdAction(ai, "whipper root tuber") {}

        bool isUseful() override { return bot->GetLevel() >= 45 && UseItemIdAction::isUseful() && AI_VALUE2(bool, "combat", "self target"); }

        uint32 GetItemId() override { return 11951; }
    };

    class UseRandomRecipeAction : public UseAction
    {
    public:
        UseRandomRecipeAction(PlayerbotAI* ai) : UseAction(ai, "random recipe", 6000U) {}

        virtual bool isUseful() override;
        virtual bool isPossible() override {return AI_VALUE2(uint32,"item count", "recipe") > 0; }
      
        virtual bool Execute(Event& event) override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class OpenRandomItemAction : public UseAction
    {
    public:
        OpenRandomItemAction(PlayerbotAI* ai) : UseAction(ai, "open random item") {}

        virtual bool isUseful() override;

        virtual bool isPossible() override { return AI_VALUE2(uint32, "item count", "open") > 0; }

        virtual bool Execute(Event& event) override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseRandomQuestItemAction : public UseAction
    {
    public:
        UseRandomQuestItemAction(PlayerbotAI* ai) : UseAction(ai, "use random quest item") {}

        virtual bool isUseful() override;
        virtual bool isPossible() override { return AI_VALUE2(uint32, "item count", "quest") > 0;}

        virtual bool Execute(Event& event) override;

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    // goblin sappers
    class UseGoblinSapperChargeAction : public UseItemIdAction
    {
    public:
        UseGoblinSapperChargeAction(PlayerbotAI* ai) : UseItemIdAction(ai, "goblin sapper") {}
        virtual bool isUseful() override { return UseItemIdAction::isUseful() && bot->GetSkillValue(202) >= 205 && bot->GetHealth() > 1000; }

        virtual uint32 GetItemId() override
        { 
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif
            return (bot->GetLevel() >= 68) ? 23827 : 10646; 
        }
    };

    // oil of immolation
    class UseOilOfImmolationAction : public UseItemIdAction
    {
    public:
        UseOilOfImmolationAction(PlayerbotAI* ai) : UseItemIdAction(ai, "oil of immolation") {}
        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif
            return UseItemIdAction::isUseful() && bot->GetLevel() >= 31 && !ai->HasAura(11350, bot);
        }

        virtual uint32 GetItemId() override { return 8956; }
    };

    // stoneshield potion
    class UseStoneshieldPotionAction : public UseItemIdAction
    {
    public:
        UseStoneshieldPotionAction(PlayerbotAI* ai) : UseItemIdAction(ai, "stoneshield potion") {}
        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif
            return UseItemIdAction::isUseful() && bot->GetLevel() >= 46 && !ai->HasAura(17540, bot);
        }

        virtual uint32 GetItemId() override { return 13455; }
    };

    class UseBgBannerAction : public UseItemIdAction
    {
    public:
        UseBgBannerAction(PlayerbotAI* ai) : UseItemIdAction(ai, "bg banner") {}

        virtual bool isUseful() override
        {
            if (!UseItemIdAction::isUseful())
                return false;

            if (!bot->InBattleGround() || bot->GetLevel() < 60 || !bot->IsInCombat())
                return false;

            std::list<ObjectGuid> units = *context->GetValue<std::list<ObjectGuid> >("nearest npcs no los");
            for (std::list<ObjectGuid>::iterator i = units.begin(); i != units.end(); i++)
            {
                Unit* unit = ai->GetUnit(*i);
                if (!unit)
                    continue;

                if (bot->GetTeam() == HORDE && unit->GetEntry() == 14466)
                    return false;

                if (bot->GetTeam() == ALLIANCE && unit->GetEntry() == 14465)
                    return false;
            }

            return true;
        }

        virtual uint32 GetItemId() override
        {
            return bot->GetTeam() == ALLIANCE ? 18606 : 18607;
        }

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class UseRocketBootsAction : public UseItemIdAction
    {
    public:
        UseRocketBootsAction(PlayerbotAI* ai) : UseItemIdAction(ai, "rocket boots") {}

        virtual bool isUseful() override
        {
            if(!UseItemIdAction::isUseful())
                return false;

            if (ai->HasAnyAuraOf(bot, "sprint", "speed", "goblin rocket boots", "dash", NULL))
                return false;

            return true;
        }

        virtual uint32 GetItemId() override
        {
            return 7189;
        }
    };

    class UseBandageAction : public UseTargetedItemIdAction
    {
    public:
        UseBandageAction(PlayerbotAI* ai) : UseTargetedItemIdAction(ai, "use bandage", 8000U) {}

        virtual bool isUseful() override
        {
            if (!UseTargetedItemIdAction::isUseful())
                return false;

            if (bot->HasAura(11196))
                return false;

            if (AI_VALUE(uint8, "my attacker count") > 0 || bot->HasAuraType(SPELL_AURA_PERIODIC_DAMAGE))
                return false;

            if (bot->GetSkillValue(129) < 1)
                return false;
              
            // Prevent tanks from bandaging in dungeons and raids
            if (ai->IsTank(bot) && ai->IsStateActive(BotState::BOT_STATE_COMBAT))
            {
                const Map* map = bot->GetMap();
                if (map->IsDungeon() || map->IsRaid())
                {
                    return false;
                }
            }

            return true;
        }

        virtual std::string GetTargetName() override { return "self target"; }

        virtual uint32 GetItemId() override
        {
            int firstAidSkillValue = bot->GetSkillValue(129);
#ifdef MANGOSBOT_TWO
            if (firstAidSkillValue >= 400)
                return 34722;
            if (firstAidSkillValue >= 350)
                return 34721;
#endif
#ifndef MANGOSBOT_ZERO
            if (firstAidSkillValue >= 325)
                return 21991;
            if (firstAidSkillValue >= 300)
                return 21990;
#endif
            if (firstAidSkillValue >= 225)
                return 14530;
            if (firstAidSkillValue >= 200)
                return 14529;
            if (firstAidSkillValue >= 175)
                return 8545;
            if (firstAidSkillValue >= 150)
                return 8544;
            if (firstAidSkillValue >= 125)
                return 6451;
            if (firstAidSkillValue >= 100)
                return 6450;
            if (firstAidSkillValue >= 75)
                return 3531;
            if (firstAidSkillValue >= 50)
                return 3530;
            if (firstAidSkillValue >= 20)
                return 2581;
            return 1251;
        }

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptMovement() const override { return true; }
    };

    class ThrowGrenadeAction : public UseTargetedItemIdAction
    {
    public:
        ThrowGrenadeAction(PlayerbotAI* ai) : UseTargetedItemIdAction(ai, "throw grenade") {}

        virtual std::string GetTargetName() override { return "current target"; }

        virtual bool isUseful() override
        {
#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif

            Unit* target = GetTarget();
            if (!target)
                return false;

            if (!UseTargetedItemIdAction::isUseful())
                return false;

            if (!target->IsNonMeleeSpellCasted(false) && target->IsStopped())
                return false;

            if (bot->GetSkillValue(202) < 175)
                return false;

            return bot->GetLevel() >= 52;
        }

        virtual uint32 GetItemId() override
        { 
            if (bot->GetSkillValue(202) >= 325)
                return 23737;
            if (bot->GetSkillValue(202) >= 260)
                return 15993;
            return 4390; 
        }
    };

    class UseDarkRuneAction : public UseItemIdAction
    {
    public:
        UseDarkRuneAction(PlayerbotAI* ai) : UseItemIdAction(ai, "dark rune") {}

        virtual bool isUseful() override
        {
            if(!UseItemIdAction::isUseful())
                return false;

#ifndef MANGOSBOT_ZERO
            if (bot->InArena())
                return false;
#endif

            if (bot->getClass() == CLASS_MAGE) // mage should use mana gem, shares cd with dark rune
                return false;

            return bot->GetHealth() > 1000;
        }

        virtual uint32 GetItemId() override { return 20520; }
    };

    class UseFireProtectionPotionAction : public UseItemIdAction
    {
    public:
        UseFireProtectionPotionAction(PlayerbotAI* ai) : UseItemIdAction(ai, "fire protection potion") {}

        virtual bool isUseful() override
        {
            return UseItemIdAction::isUseful() && !bot->HasAura(17543) && (bot->GetLevel() >= 48);
        }

        virtual uint32 GetItemId() override { return 13457; }
    };

    class UseFreeActionPotionAction : public UseItemIdAction
    {
    public:
        UseFreeActionPotionAction(PlayerbotAI* ai) : UseItemIdAction(ai, "free action potion") {}

        virtual bool isUseful() override
        {
            if (!UseItemIdAction::isUseful())
                return false;

            if (bot->GetLevel() < 20 || bot->HasAura(6615))
                return false;

            Unit* target = AI_VALUE(Unit*, "current target");
            if (!target || !target->IsPlayer())
                return false;

            if (!bot->InBattleGround() && urand(0, 1))
                return false;

            return true;
        }

        virtual uint32 GetItemId() override { return 5634; }
    };

    class DrinkAction : public UseAction
    {
    public:
        DrinkAction(PlayerbotAI* ai) : UseAction(ai, "drink") {}

        bool Execute(Event& event) override
        {
            if (sServerFacade.IsInCombat(bot))
                return false;

            if (!bot->HasMana())
                return false;

            if (ai->HasCheat(BotCheatMask::item))
            {
                if (bot->IsNonMeleeSpellCasted(true))
                    return false;

                bot->clearUnitState(UNIT_STAT_CHASE);
                bot->clearUnitState(UNIT_STAT_FOLLOW);

                if (ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    ai->StopMoving();
                }

                if (sServerFacade.isMoving(bot))
                {
                    ai->StopMoving();
                    SetDuration(sPlayerbotAIConfig.globalCoolDown);
                    return false;
                }

                bot->addUnitState(UNIT_STAND_STATE_SIT);
                ai->InterruptSpell();

                float drinkDuration = AI_VALUE(float, "drink duration");

                const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(24355);
                if (!pSpellInfo)
                    return false;

                ai->Unmount();

                ai->CastSpell(24355, bot);
                SetDuration(drinkDuration);
                bot->RemoveSpellCooldown(*pSpellInfo);

                // Eat and drink at the same time

                if (AI_VALUE(bool, "should eat"))
                {
                    const SpellEntry* pSpellInfo2 = sServerFacade.LookupSpellInfo(24005);
                    if (pSpellInfo2)
                    {
                        ai->AddAura(bot, 24005);
                        bot->RemoveSpellCooldown(*pSpellInfo2);
                    }
                }

                return true;
            }

            return UseAction::Execute(event);
        }

        bool isUseful() override
        {
            return UseAction::isUseful() && AI_VALUE(bool, "should drink");
        }

        bool isPossible() override
        {
            return !sServerFacade.IsInCombat(bot) && UseAction::isPossible();
        }
    };

    class EatAction : public UseAction
    {
    public:
        EatAction(PlayerbotAI* ai) : UseAction(ai, "food") {}

        bool Execute(Event& event) override
        {
            if (sServerFacade.IsInCombat(bot))
                return false;

            if (ai->HasCheat(BotCheatMask::item))
            {
                if (bot->IsNonMeleeSpellCasted(true))
                    return false;

                bot->clearUnitState(UNIT_STAT_CHASE);
                bot->clearUnitState(UNIT_STAT_FOLLOW);

                if (ai->GetBot()->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
                {
                    ai->StopMoving();
                }

                if (sServerFacade.isMoving(bot))
                {
                    ai->StopMoving();
                    SetDuration(sPlayerbotAIConfig.globalCoolDown);
                    return false;
                }

                bot->addUnitState(UNIT_STAND_STATE_SIT);
                ai->InterruptSpell();

                float eatDuration = AI_VALUE(float, "eat duration");

                const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(24005);
                if (!pSpellInfo)
                    return false;

                ai->Unmount();

                ai->CastSpell(24005, bot);
                SetDuration(eatDuration);
                bot->RemoveSpellCooldown(*pSpellInfo);

                // Eat and drink at the same time
                if (AI_VALUE(bool, "should drink"))
                {
                    const SpellEntry* pSpellInfo2 = sServerFacade.LookupSpellInfo(24355);
                    if (pSpellInfo2)
                    {
                        ai->AddAura(bot, 24355);
                        bot->RemoveSpellCooldown(*pSpellInfo2);
                    }
                }

                return true;
            }

            return UseAction::Execute(event);
        }

        bool isUseful() override
        {
            return UseAction::isUseful() && AI_VALUE(bool, "should eat");
        }

        bool isPossible() override
        {
            return !sServerFacade.IsInCombat(bot) && UseAction::isPossible();
        }
    };
}
