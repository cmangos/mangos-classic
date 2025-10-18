#pragma once
#include "playerbot/strategy/Value.h"

namespace ai
{
    enum class GlyphSlotType : uint8
    {
          MAJOR_SLOT = 0
        , MINOR_SLOT = 1
        , NO_SLOT = 98
        , LOCKED_SLOT = 99
    };

    class AvailableGlyphsValue : public CalculatedValue<std::vector<uint32>>
    {
    public:
        AvailableGlyphsValue(PlayerbotAI* ai, std::string name = "available glyphs") : CalculatedValue<std::vector<uint32>>(ai, name, 30) {}

        std::vector<uint32> Calculate();

        static uint32 GetGlyphIdFromProto(const ItemPrototype* glyphProto);
        static const ItemPrototype* GetGlyphProtoFromGlyphId(uint32 glyphId, uint32 classMask);

        static GlyphSlotType GetGlyphSlotTypeFromItemId(uint32 itemId);
        static GlyphSlotType GetGlyphSlotTypeFromSlot(uint8 slotId, uint32 level = 80);
    };

    class WantedGlyphsValue : public AvailableGlyphsValue
    {
    public:
        WantedGlyphsValue(PlayerbotAI* ai) : AvailableGlyphsValue(ai, "wanted glyphs") {}

        std::vector<uint32> Calculate();
    };

    class EquipedGlyphsValue : public AvailableGlyphsValue
    {
    public:
        EquipedGlyphsValue(PlayerbotAI* ai) : AvailableGlyphsValue(ai, "equiped glyphs") {}

        std::vector<uint32> Calculate();
    };

    class GlyphIsUpgradeValue : public BoolCalculatedValue, public Qualified
    {
    public:
        GlyphIsUpgradeValue(PlayerbotAI* ai) : BoolCalculatedValue(ai, "glyph is upgrade"), Qualified() {}

        static size_t GetWantedRank(uint32 itemId, const std::vector<uint32>& wantedGlyhps);

        bool Calculate();
    };
}
