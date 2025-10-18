#pragma once
#include "playerbot/strategy/Value.h"
#include "ItemUsageValue.h"
#include "BudgetValues.h"

namespace ai
{
    class CanMoveAroundValue : public BoolCalculatedValue
    {
    public:
        CanMoveAroundValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can move around", 2) {}
        virtual bool Calculate()
        {
            if (bot->GetTradeData())
                return false;

            //if (ai->HasStrategy("stay", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("guard", BotState::BOT_STATE_NON_COMBAT))
            //    return false;

            if (!AI_VALUE(bool, "group ready"))
                return false;

            if (AI_VALUE2(bool, "trigger active", "castnc"))
                return false;

            return true;
        }
    };

    class ShouldHomeBindValue : public BoolCalculatedValue
    {
    public:
        ShouldHomeBindValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should home bind", 2) {}
        virtual bool Calculate() { return AI_VALUE2(float, "distance", "home bind") > 1000.0f; };
    };


    class ShouldRepairValue : public BoolCalculatedValue
	{
	public:
        ShouldRepairValue(PlayerbotAI* ai) : BoolCalculatedValue(ai,"should repair",2) {}
        virtual bool Calculate() { return AI_VALUE(uint8, "durability") < 30 || AI_VALUE(uint8, "lowest durability") < 10; };
    };

    class CanRepairValue : public BoolCalculatedValue
    {
    public:
        CanRepairValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can repair",2) {}
        virtual bool Calculate() { return  ai->HasStrategy("rpg maintenance", BotState::BOT_STATE_NON_COMBAT) && AI_VALUE(uint8, "durability inventory") < 100 && AI_VALUE(uint32, "min repair cost") < AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::repair); };
    };

    class ShouldSellValue : public BoolCalculatedValue
    {
    public:
        ShouldSellValue(PlayerbotAI* ai, std::string name = "should sell", int checkInterval = 2) : BoolCalculatedValue(ai, name , checkInterval) {}
        virtual bool Calculate() { return AI_VALUE(uint8, "bag space") > 80; };
    };

    class CanSellValue : public BoolCalculatedValue
    {
    public:
        CanSellValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can sell",2) {}
        virtual bool Calculate() { return ai->HasStrategy("rpg vendor", BotState::BOT_STATE_NON_COMBAT) && AI_VALUE2(uint32, "item count", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_VENDOR)) > 0; };
    };

    class CanBuyValue : public BoolCalculatedValue
    {
    public:
        CanBuyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can buy", 2) {}
        virtual bool Calculate() { return ai->HasStrategy("rpg vendor", BotState::BOT_STATE_NON_COMBAT) && !AI_VALUE(bool, "should repair") && AI_VALUE(uint8, "bag space") < 90 && !AI_VALUE(bool, "can get mail") && (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ammo) || AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::consumables) || AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::gear) || AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::tradeskill)); };
    };

    class ShouldAHSellValue : public ShouldSellValue
    {
    public:
        ShouldAHSellValue(PlayerbotAI* ai) : ShouldSellValue(ai, "should ah sell", 2) {}
        virtual bool Calculate();
    };

    class CanAHSellValue : public BoolCalculatedValue
    {
    public:
        CanAHSellValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can ah sell", 2) {}
        virtual bool Calculate() { return ai->HasStrategy("rpg vendor", BotState::BOT_STATE_NON_COMBAT) && AI_VALUE2(uint32, "item count", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_AH)) > 1 && AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah) > GetAuctionDeposit(); };

        uint32 GetAuctionDeposit()
        {
            float minDeposit = 0;
            for (auto item : AI_VALUE2(std::list<Item*>, "inventory items", "usage " + std::to_string((uint8)ItemUsage::ITEM_USAGE_AH)))
            {
                uint32 deposit = ItemUsageValue::GetAhDepositCost(item->GetProto(), item->GetCount());

                if (minDeposit == 0 || deposit < minDeposit)
                    minDeposit = deposit;
            }

            return minDeposit;
        }
    };

    class CanAHBuyValue : public BoolCalculatedValue
    {
    public:
        CanAHBuyValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can ah buy", 2) {}
        virtual bool Calculate() { return ai->HasStrategy("rpg vendor", BotState::BOT_STATE_NON_COMBAT) && !AI_VALUE(bool, "should repair") && !AI_VALUE(bool, "should sell") && !AI_VALUE(bool, "can get mail") && AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::ah) > 0; };
    };


    class CanGetMailValue : public BoolCalculatedValue
    {
    public:
        CanGetMailValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can get mail", 2) {}
        virtual bool Calculate();
    };

    class ShouldGetMailValue : public BoolCalculatedValue
    {
    public:
        ShouldGetMailValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should get mail", 60) {}
        virtual bool Calculate();
    };

    class CanFightEqualValue: public BoolCalculatedValue
    {
    public:
        CanFightEqualValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight equal",2) {}
        virtual bool Calculate() { return AI_VALUE(uint8, "durability") > 20 && !ai->HasAura(SPELL_ID_PASSIVE_RESURRECTION_SICKNESS,bot); };
    };

    class CanFightEliteValue : public BoolCalculatedValue
    {
    public:
        CanFightEliteValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight elite") {}
        virtual bool Calculate() { return bot->GetGroup() && AI_VALUE2(bool, "group and", "can fight equal") && AI_VALUE2(bool, "group and", "following party") && !AI_VALUE2(bool, "group or", "should sell,can sell"); };
    };

    class CanFightBossValue : public BoolCalculatedValue
    {
    public:
        CanFightBossValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can fight boss") {}
        virtual bool Calculate() { return bot->GetGroup() && bot->GetGroup()->GetMembersCount() > 3 && AI_VALUE2(bool, "group and", "can fight equal") && AI_VALUE2(bool, "group and", "following party") && !AI_VALUE2(bool, "group or", "should sell,can sell"); };
    };        

    class ShouldDrinkValue : public BoolCalculatedValue
    {
    public:
        ShouldDrinkValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should drink", 2) {}
        virtual bool Calculate()
        {
            if (!bot->HasMana())
                return false;

            if (AI_VALUE2(uint8, "mana", "self target") >= 85)
                return false;

            Player* master = ai->GetMaster();
            if (!master)
                return true;

            if (!bot->GetGroup())
                return true;

            if (!ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                return true;

            if (!bot->IsWithinDist(master, sPlayerbotAIConfig.EatDrinkMaxDistance))
                return true;

            if (!master->IsMoving())
                return true;

            float minDistance = sPlayerbotAIConfig.EatDrinkMinDistance;
            if (!bot->GetGroup()->IsRaidGroup())
                minDistance += sPlayerbotAIConfig.followDistance;
            else
                minDistance += sPlayerbotAIConfig.raidFollowDistance;
            
            if (bot->IsWithinDist(master, minDistance))
                return true;

            return false;
        }
    };

    class ShouldEatValue : public BoolCalculatedValue
    {
    public:
        ShouldEatValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "should eat", 2) {}
        virtual bool Calculate()
        {
            if (AI_VALUE2(uint8, "health", "self target") >= sPlayerbotAIConfig.lowHealth)
                return false;

            Player* master = ai->GetMaster();
            if (!master)
                return true;

            if (!bot->GetGroup())
                return true;

            if (!ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                return true;

            if (!bot->IsWithinDist(master, sPlayerbotAIConfig.EatDrinkMaxDistance))
                return true;

            if (!master->IsMoving())
                return true;

            float minDistance = sPlayerbotAIConfig.EatDrinkMinDistance;
            if (!bot->GetGroup()->IsRaidGroup())
                minDistance += sPlayerbotAIConfig.followDistance;
            else
                minDistance += sPlayerbotAIConfig.raidFollowDistance;

            if (bot->IsWithinDist(master, minDistance))
                return true;

            return false;
        }
    };

    class DrinkDurationValue : public FloatCalculatedValue
    {
    public:
        DrinkDurationValue(PlayerbotAI* ai) : FloatCalculatedValue(ai, "drink duration") {}
        virtual float Calculate() override
        {
            Player* master = ai->GetMaster();

            float mpMissingPct = 100.0f - bot->GetPowerPercent();
            float multiplier = bot->InBattleGround() ? 20000.0f : 27000.0f;
            float drinkDuration = multiplier * (mpMissingPct / 100.0f);

            if (!master)
                return drinkDuration;

            if (!bot->GetGroup())
                return drinkDuration;

            if (!ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                return drinkDuration;

            float minDistance = sPlayerbotAIConfig.followDistance;

            if (bot->GetGroup()->IsRaidGroup())
                minDistance = sPlayerbotAIConfig.raidFollowDistance;

            if (!master->IsMoving())
                minDistance += sPlayerbotAIConfig.EatDrinkMinDistance;

            if (bot->IsWithinDist(master, minDistance))
                return drinkDuration;

            float masterOrientation = master->GetOrientation();
            float angleToBot = master->GetAngle(bot);
            float angleDiff = fabs(masterOrientation - angleToBot);

            if (angleDiff > M_PI / 2 && angleDiff < 3 * M_PI / 2)
            {
                drinkDuration *= 0.25f;
            }
            return drinkDuration;
        }
    };

    class EatDurationValue : public FloatCalculatedValue
    {
    public:
        EatDurationValue(PlayerbotAI* ai) : FloatCalculatedValue(ai, "eat duration") {}
        virtual float Calculate() override
        {
            Player* master = ai->GetMaster();

            float hpMissingPct = 100.0f - bot->GetHealthPercent();
            float multiplier = bot->InBattleGround() ? 20000.0f : 27000.0f;
            float eatDuration = multiplier * (hpMissingPct / 100.0f);
          
            if (!master)
                return eatDuration;

            if (!bot->GetGroup())
                return eatDuration;

            if (!ai->HasStrategy("follow", BotState::BOT_STATE_NON_COMBAT))
                return eatDuration;

            float minDistance = sPlayerbotAIConfig.followDistance;

            if (bot->GetGroup()->IsRaidGroup())
                minDistance = sPlayerbotAIConfig.raidFollowDistance;

            if (!master->IsMoving())
                minDistance += sPlayerbotAIConfig.EatDrinkMinDistance;

            if (bot->IsWithinDist(master, minDistance))
                return eatDuration;

            float masterOrientation = master->GetOrientation();
            float angleToBot = master->GetAngle(bot);
            float angleDiff = fabs(masterOrientation - angleToBot);

            if (angleDiff > M_PI / 2 && angleDiff < 3 * M_PI / 2)
            {
                eatDuration *= 0.25f;
            }
            return eatDuration;
        }
    };
}
