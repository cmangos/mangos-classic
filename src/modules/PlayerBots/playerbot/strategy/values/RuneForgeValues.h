#pragma once
#include "playerbot/strategy/Value.h"

namespace ai 
{
    class RuneForgeSpellsValue : public CalculatedValue<std::vector<uint32>> {
    public:
        RuneForgeSpellsValue(PlayerbotAI* ai, std::string name = "runeforge spells", int checkInterval = 10)
            : CalculatedValue<std::vector<uint32>>(ai, name, checkInterval) {}
        virtual std::vector<uint32> Calculate() override;
    };

    class BestRuneForgeSpellValue : public Uint32CalculatedValue {
    public:
        BestRuneForgeSpellValue(PlayerbotAI* ai, std::string name = "best runeforge spell", int checkInterval = 10)
            : Uint32CalculatedValue(ai, name, checkInterval) {}
        virtual uint32 Calculate() override;
    };

    class ShouldRuneForgeValue : public BoolCalculatedValue {
    public:
        ShouldRuneForgeValue(PlayerbotAI* ai, std::string name = "should runeforge")
            : BoolCalculatedValue(ai, name) {}
        virtual bool Calculate() override;

        static uint32 RuneForgeEnchantFromSpell(uint32 spellId);
        static int32 CurrentRuneForgeEnchant(Player* bot);
    };
} 