#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    class MountValue
    {
    public:
        MountValue(uint32 spellId) : spellId(spellId) {};
        MountValue(const ItemPrototype* proto) : proto(proto) { spellId = GetMountSpell(proto->ItemId); };
        MountValue(Item* item) { spellId = GetMountSpell(item->GetProto()->ItemId); proto = item->GetProto(); };
        MountValue(const MountValue& mount) : spellId(mount.spellId), proto(mount.proto) {};

        bool IsItem() { return proto; }
        const ItemPrototype* GetItemProto() { return proto; }
        uint32 GetSpellId() { return spellId; }
        uint32 GetSpeed(bool canFly) { return GetSpeed(spellId, canFly); }
        static uint32 IsMountSpell(uint32 spellId) { return GetSpeed(spellId, false) || GetSpeed(spellId, true); }
        static uint32 GetSpeed(uint32 spellId, bool canFly);
        static uint32 GetSpeed(uint32 spellId) { return std::max(GetSpeed(spellId, false), GetSpeed(spellId, true)); };
        static uint32 GetMountSpell(uint32 itemId);
        bool IsValidLocation(Player* bot);

    private:
        const ItemPrototype* proto = nullptr;
        uint32 spellId = 0;
    };

    class CurrentMountSpeedValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        CurrentMountSpeedValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "current mount speed", 1), Qualified() {}
        virtual uint32 Calculate();
    };

    class FullMountListValue : public SingleCalculatedValue<std::vector<MountValue>>
    {
    public:
        FullMountListValue(PlayerbotAI* ai) : SingleCalculatedValue<std::vector<MountValue>>(ai, "full mount list") {}
        virtual std::vector<MountValue> Calculate();
    };

    class MountListValue : public CalculatedValue<std::vector<MountValue>>
    {
    public:
        MountListValue(PlayerbotAI* ai) : CalculatedValue<std::vector<MountValue>>(ai, "mount list", 10) {}
        virtual std::vector<MountValue> Calculate();
        virtual std::string Format();
    };   

    class MaxMountSpeedValue : public Uint32CalculatedValue, public Qualified
    {
    public:
        MaxMountSpeedValue(PlayerbotAI* ai) : Uint32CalculatedValue(ai, "max mount speed", 1), Qualified() {}
        virtual uint32 Calculate();
    };

    class MountSkillTypeValue : public CalculatedValue<uint32>
    {
    public:
        MountSkillTypeValue(PlayerbotAI* ai) : CalculatedValue<uint32>(ai, "mount skilltype", 10) {}

        virtual uint32 Calculate() override;
    };

    class AvailableMountVendors : public CalculatedValue<std::vector<int32>>
    {
    public:
        AvailableMountVendors(PlayerbotAI* ai) : CalculatedValue<std::vector<int32>>(ai, "available mount vendors", 10) {}

        virtual std::vector<int32> Calculate() override;
    };

    class CanTrainMountValue : public BoolCalculatedValue
    { 
    public:
        CanTrainMountValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can train mount", 10) {}

        virtual bool Calculate() override;
    };

    class CanBuyMountValue : public BoolCalculatedValue
    {
    public:
        CanBuyMountValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "can buy mount", 10) {}

        virtual bool Calculate() override;
    };
}
