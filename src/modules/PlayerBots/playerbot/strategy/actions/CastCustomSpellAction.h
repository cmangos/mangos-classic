#pragma once
#include "ListSpellsAction.h"
#include "playerbot/strategy/values/CraftValues.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "GenericActions.h"
#include "playerbot/RandomItemMgr.h"

namespace ai
{
    class CastCustomSpellAction : public ChatCommandAction, public Qualified
    {
    public:
        CastCustomSpellAction(PlayerbotAI* ai, std::string name = "cast") : ChatCommandAction(ai, name), Qualified() {}
        virtual bool Execute(Event& event) override;
        virtual std::string castString(WorldObject* target) { return "cast"; }
        virtual uint32 getDuration() const { return 3000U; }
        virtual std::string GetTargetName() override { return "current target"; }
    private:

        bool CastSummonPlayer(Player* requester, std::string command);

    protected:
        bool ncCast = false;
    };

    class CastCustomNcSpellAction : public CastCustomSpellAction
    {
    public:
        CastCustomNcSpellAction(PlayerbotAI* ai, std::string name = "cast custom nc spell") : CastCustomSpellAction(ai, name) {}
        virtual bool isUseful() { return !bot->IsMoving(); }
        virtual std::string castString(WorldObject* target) { return "castnc" +(target ? " "+ chat->formatWorldobject(target):""); }
    };

    class CastRandomSpellAction : public ListSpellsAction
    {
    public:
        CastRandomSpellAction(PlayerbotAI* ai, std::string name = "cast random spell") : ListSpellsAction(ai, name) {}

        virtual bool AcceptSpell(const SpellEntry* pSpellInfo)
        {
            bool isTradeSkill = pSpellInfo->Effect[0] == SPELL_EFFECT_CREATE_ITEM && pSpellInfo->ReagentCount[0] > 0 &&
#ifdef MANGOSBOT_ZERO
                pSpellInfo->School == 0;
#else
                pSpellInfo->SchoolMask == 1;
#endif

            bool hasCost = pSpellInfo->manaCost > 0;

            return hasCost && !isTradeSkill && (GetSpellRecoveryTime(pSpellInfo) < MINUTE * IN_MILLISECONDS || !ai->HasActivePlayerMaster());
        }

        virtual uint32 GetSpellPriority(const SpellEntry* pSpellInfo) { return 1; }
        virtual bool Execute(Event& event) override;

        virtual bool castSpell(uint32 spellId, WorldObject* wo, Player* requester);
    };
       
    class CraftRandomItemAction : public CastRandomSpellAction
    {
    public:
        CraftRandomItemAction(PlayerbotAI* ai) : CastRandomSpellAction(ai, "craft random item") {}
    public:
        virtual bool Execute(Event& event) override;
    };

    class DisenchantRandomItemAction : public CastCustomSpellAction
    {
    public:
        DisenchantRandomItemAction(PlayerbotAI* ai) : CastCustomSpellAction(ai, "disenchant random item")  {}
        virtual bool isUseful() { return ai->HasSkill(SKILL_ENCHANTING) && !bot->IsInCombat() && AI_VALUE2(uint32, "item count", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_DISENCHANT)) > 0; }
        virtual bool Execute(Event& event) override;
    };

    class EnchantRandomItemAction : public CastRandomSpellAction
    {
    public:
        EnchantRandomItemAction(PlayerbotAI* ai) : CastRandomSpellAction(ai, "enchant random item") {}
        virtual bool isUseful() { return ai->HasSkill(SKILL_ENCHANTING); }

        virtual bool AcceptSpell(const SpellEntry* pSpellInfo)
        {
            return pSpellInfo->Effect[0] == SPELL_EFFECT_ENCHANT_ITEM && pSpellInfo->ReagentCount[0] > 0;
        }

        virtual uint32 GetSpellPriority(const SpellEntry* pSpellInfo)
        {
            if (pSpellInfo->Effect[0] != SPELL_EFFECT_ENCHANT_ITEM)
                return 0;

            uint32 enchant_id = pSpellInfo->EffectMiscValue[0];

            Item* item = AI_VALUE2(Item*, "item for spell", pSpellInfo->Id);

            if (!item)
                return 0;

            uint32 castCount = AI_VALUE2(uint32, "has reagents for", pSpellInfo->Id);

            if (!castCount)
                return 0;

            ItemPrototype const* proto = item->GetProto();

            //Upgrade current equiped item enchantment.
            if (item->GetBagSlot() == INVENTORY_SLOT_BAG_0) 
            {
                if (HAS_AI_VALUE2("force item usage", proto->ItemId))
                    return 0;

                uint32 currentEnchnatWeight = 0;
                if (item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT))
                    currentEnchnatWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), item->GetEnchantmentId(PERM_ENCHANTMENT_SLOT));

                uint32 newEnchantWeight = sRandomItemMgr.CalculateEnchantWeight(bot->getClass(), sRandomItemMgr.GetPlayerSpecId(bot), enchant_id);

                if (currentEnchnatWeight > newEnchantWeight) //Do not replace better enchants.
                    return 0;

                if (currentEnchnatWeight < newEnchantWeight) //Place better enchants.
                    return 100;
            }

            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", proto->ItemId);

            if (usage != ItemUsage::ITEM_USAGE_AH && usage != ItemUsage::ITEM_USAGE_BROKEN_AH && usage != ItemUsage::ITEM_USAGE_VENDOR && usage != ItemUsage::ITEM_USAGE_DISENCHANT && usage != ItemUsage::ITEM_USAGE_NONE)
                return 0;

            //Enchant for skillup
            if (ShouldCraftSpellValue::SpellGivesSkillUp(pSpellInfo->Id, bot))
                return 10;

            return 0;
        }
    };
}
