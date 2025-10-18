#pragma once
#include "playerbot/strategy/Value.h"

class Unit;

namespace ai
{
    class HealthValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        HealthValue(PlayerbotAI* ai, std::string name = "health") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class IsDeadValue : public BoolCalculatedValue, public Qualified
    {
    public:
        IsDeadValue(PlayerbotAI* ai, std::string name = "dead") : BoolCalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual bool Calculate();
    };

    class PetIsDeadValue : public BoolCalculatedValue
    {
    public:
        PetIsDeadValue(PlayerbotAI* ai, std::string name = "pet dead") : BoolCalculatedValue(ai, name) {}
        virtual bool Calculate();
    };

    class PetIsHappyValue : public BoolCalculatedValue
    {
    public:
        PetIsHappyValue(PlayerbotAI* ai, std::string name = "pet happy") : BoolCalculatedValue(ai, name) {}
        virtual bool Calculate();
    };

    class RageValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        RageValue(PlayerbotAI* ai, std::string name = "rage") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class EnergyValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        EnergyValue(PlayerbotAI* ai, std::string name = "energy") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class ManaValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        ManaValue(PlayerbotAI* ai, std::string name = "mana") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class HasManaValue : public BoolCalculatedValue, public Qualified
    {
    public:
        HasManaValue(PlayerbotAI* ai, std::string name = "has mana") : BoolCalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual bool Calculate();
    };

    class ComboPointsValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        ComboPointsValue(PlayerbotAI* ai, std::string name = "combo points") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class IsMountedValue : public BoolCalculatedValue, public Qualified
    {
    public:
        IsMountedValue(PlayerbotAI* ai, std::string name = "mounted") : BoolCalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual bool Calculate();
    };

    class IsInCombatValue : public MemoryCalculatedValue<bool>, public Qualified
    {
    public:
        IsInCombatValue(PlayerbotAI* ai, std::string name = "combat") : MemoryCalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual bool EqualToLast(bool value) { return value == lastValue; }
        virtual bool Calculate();

        virtual std::string Format()
        {
            return this->Calculate() ? "true" : "false";
        }
    };

    class BagSpaceValue : public Uint8CalculatedValue
    {
    public:
        BagSpaceValue(PlayerbotAI* ai, std::string name = "bag space") : Uint8CalculatedValue(ai, name) {}
        virtual uint8 Calculate();
    };

    class DurabilityValue : public Uint8CalculatedValue
    {
    public:
        DurabilityValue(PlayerbotAI* ai, std::string name = "durability") : Uint8CalculatedValue(ai, name) {}
        virtual uint8 Calculate();
    };

    class DurabilityInventoryValue : public Uint8CalculatedValue
    {
    public:
        DurabilityInventoryValue(PlayerbotAI* ai, std::string name = "durability inventory") : Uint8CalculatedValue(ai, name) {}
        virtual uint8 Calculate();
    };

    class LowestDurabilityValue : public Uint8CalculatedValue
    {
    public:
        LowestDurabilityValue(PlayerbotAI* ai, std::string name = "lowest durability") : Uint8CalculatedValue(ai, name) {}
        virtual uint8 Calculate();
    };

     class SpeedValue : public Uint8CalculatedValue, public Qualified
    {
    public:
        SpeedValue(PlayerbotAI* ai, std::string name = "speed") : Uint8CalculatedValue(ai, name), Qualified() {}

        Unit* GetTarget()
        {
            AiObjectContext* ctx = AiObject::context;
            return ctx->GetValue<Unit*>(qualifier)->Get();
        }

        virtual uint8 Calculate();
    };

    class IsInGroupValue : public BoolCalculatedValue
    {
    public:
        IsInGroupValue(PlayerbotAI* ai, std::string name = "in group") : BoolCalculatedValue(ai, name) {}
        virtual bool Calculate() { return bot->GetGroup(); }
    };

    class DeathCountValue : public ManualSetValue<uint32>
    {
    public:
        DeathCountValue(PlayerbotAI* ai, std::string name = "death count") : ManualSetValue<uint32>(ai, 0, name) {}

        virtual std::string Format()
        {
            std::ostringstream out; out << (int)this->value;
            return out.str();
        }
    };


    class ExperienceValue : public MemoryCalculatedValue<uint32>
    {
    public:
        ExperienceValue(PlayerbotAI* ai, std::string name = "experience", uint32 checkInterval = 60) : MemoryCalculatedValue<uint32>(ai, name, checkInterval) {}
        virtual bool EqualToLast(uint32 value) { return value != lastValue; }
        virtual uint32 Calculate() { return bot->GetUInt32Value(PLAYER_XP);}

        virtual std::string Format()
        {
            std::ostringstream out; out << (int)this->Calculate() << " last change:" << LastChangeDelay() << "s";
            return out.str();
        }
    };

    class HonorValue : public ExperienceValue
    {
    public:
        HonorValue(PlayerbotAI* ai, std::string name = "honor", uint32 checkInterval = 60) : ExperienceValue(ai, name, checkInterval) {}
        virtual bool EqualToLast(uint32 value) { return value != lastValue; }
        virtual uint32 Calculate() { return bot->GetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS); }        
    };
}
