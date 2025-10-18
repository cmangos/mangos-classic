#pragma once
#include "Config/Config.h"
#include "AhBotConfig.h"
#include "PricingStrategy.h"
#include "Entities/ItemPrototype.h"
#include "Globals/SharedDefines.h"

namespace ahbot
{
    class Category
    {
    public:
        Category() : pricingStrategy(NULL) {}
        virtual ~Category() { if (pricingStrategy) delete pricingStrategy; }

    public:
        virtual bool Contains(ItemPrototype const* proto) { return false; }
        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return GetName(); }

        virtual uint32 GetStackCount(ItemPrototype const* proto);
        virtual uint32 GetSkillId() { return 0; }

        virtual PricingStrategy* GetPricingStrategy();

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }
    protected:
        std::string typeName = "default";
        int32 defaultMaxType = 1;
    private:
        PricingStrategy *pricingStrategy;
    };

    class Consumable : public Category
    {
    public:
        Consumable() : Category() {}
    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_CONSUMABLE;
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "consumables"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }
    protected:
        std::string typeName = "consumable";
        int32 defaultMaxType = 10;
    };

    class Quest : public Category
    {
    public:
        Quest() : Category() {}
    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_QUEST;
        }
        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "quest items"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }
    protected:
        std::string typeName = "quest";
        int32 defaultMaxType = 5;
    };

    class Trade : public Category
    {
    public:
        Trade() : Category() {}
    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_TRADE_GOODS ||
                    proto->Class == ITEM_CLASS_MISC ||
                    proto->Class == ITEM_CLASS_REAGENT
#ifndef MANGOSBOT_ZERO
                ||
                    proto->Class == ITEM_CLASS_GEM
#endif
                ;
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }
    protected:
        std::string typeName = "trade";
        int32 defaultMaxType = 5;
    };

    class Reagent : public Category
    {
    public:
        Reagent() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_REAGENT;
        }
        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "reagents"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }
    protected:
        std::string typeName = "reagent";
        int32 defaultMaxType = 1;
    };

    class Recipe : public Category
    {
    public:
        Recipe() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_RECIPE;
        }
        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "recipes and patterns"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

        virtual uint32 GetStackCount(ItemPrototype const* proto)
        {
            return 1;
        }
    protected:
        std::string typeName = "recipe";
        int32 defaultMaxType = 1;
    };

    class Equip : public Category
    {
    public:
        Equip() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return (proto->Class == ITEM_CLASS_WEAPON ||
                proto->Class == ITEM_CLASS_ARMOR) && proto->ItemLevel > 1;
        }
        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "armor and weapons"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

        virtual uint32 GetStackCount(ItemPrototype const* proto)
        {
            return 1;
        }
    protected:
        std::string typeName = "equip";
        int32 defaultMaxType = 1;
    };

    class Quiver : public Category
    {
    public:
        Quiver() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_QUIVER && proto->ItemLevel > 1;
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "quivers and ammo poaches"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

        virtual uint32 GetStackCount(ItemPrototype const* proto)
        {
            return 1;
        }

    protected:
        std::string typeName = "quiver";
        int32 defaultMaxType = 1;
    };

    class Projectile : public Category
    {
    public:
        Projectile() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_PROJECTILE;
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "projectiles"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

        virtual uint32 GetStackCount(ItemPrototype const* proto)
        {
            return proto->GetMaxStackSize();
        }

    protected:
        std::string typeName = "projectile";
        int32 defaultMaxType = 5;
    };

    class Container : public Category
    {
    public:
        Container() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_CONTAINER;
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "containers"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

        virtual uint32 GetStackCount(ItemPrototype const* proto)
        {
            return 1;
        }

    protected:
        std::string typeName = "container";
        int32 defaultMaxType = 1;
    };

    class DevicesAndParts : public Category
    {
    public:
        DevicesAndParts() : Category() {}

    public:
        virtual bool Contains(ItemPrototype const* proto)
        {
            return proto->Class == ITEM_CLASS_TRADE_GOODS &&
                    (proto->SubClass == ITEM_SUBCLASS_PARTS ||
                    proto->SubClass == ITEM_SUBCLASS_DEVICES ||
                    proto->SubClass == ITEM_SUBCLASS_EXPLOSIVES);
        }

        virtual std::string GetName() { return typeName; }
        virtual std::string GetDisplayName() { return GetName(); }
        virtual std::string GetLabel() { return "devices and explosives"; }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedAuctionCount(typeName, 5));
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(GetName(), sAhBotConfig.GetMaxAllowedItemAuctionCount(typeName, defaultMaxType));
        }

    protected:
        std::string typeName = "devices";
        int32 defaultMaxType = 1;
    };

    class QualityCategoryWrapper : public Category
    {
    public:
        QualityCategoryWrapper(Category* category, uint32 quality);

    public:
        virtual bool Contains(ItemPrototype const* proto);
        virtual std::string GetName() { return category->GetName(); }
        virtual std::string GetDisplayName() { return combinedName; }
        virtual std::string GetLabel() { return category->GetLabel(); }
        virtual uint32 GetStackCount(ItemPrototype const* proto) { return category->GetStackCount(proto); }
        virtual PricingStrategy* GetPricingStrategy() { return category->GetPricingStrategy(); }
        virtual uint32 GetSkillId() { return category->GetSkillId(); }

        virtual uint32 GetMaxAllowedAuctionCount()
        {
            return sAhBotConfig.GetMaxAllowedAuctionCount(combinedName, category->GetMaxAllowedAuctionCount());
        }

        virtual uint32 GetMaxAllowedItemAuctionCount(ItemPrototype const* proto)
        {
            return sAhBotConfig.GetMaxAllowedItemAuctionCount(combinedName, category->GetMaxAllowedItemAuctionCount(proto));
        }

    private:
        uint32 quality;
        Category* category;
        std::string combinedName;
    };
};
