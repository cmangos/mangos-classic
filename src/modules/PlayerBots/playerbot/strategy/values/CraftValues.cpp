#include "CraftValues.h"
#include "ItemUsageValue.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/PlayerbotAI.h"

using namespace ai;

std::vector<uint32> CraftSpellsValue::Calculate()
{
    std::vector<uint32> spellIds;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second.state == PLAYERSPELL_REMOVED || spell.second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

#ifdef MANGOSBOT_TWO
        if (pSpellInfo->Effect[0] == SPELL_EFFECT_CREATE_ITEM_2 && spellId == 61288 && bot->IsSpellReady(61288)) //Todo handle other item_2 spells
        {
            spellIds.push_back(spellId);
            continue;
        }
#endif

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_CREATE_ITEM)
            continue;      

        for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
        {
            if (pSpellInfo->EffectItemType[i])
            {
                spellIds.push_back(spellId);
                break;
            }
        }
    }

    return spellIds;
}

std::vector<uint32> EnchantSpellsValue::Calculate()
{
    std::vector<uint32> spellIds;

    PlayerSpellMap const& spellMap = bot->GetSpellMap();

    for (auto& spell : spellMap)
    {
        uint32 spellId = spell.first;

        if (spell.second.state == PLAYERSPELL_REMOVED || spell.second.disabled || IsPassiveSpell(spellId))
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_ENCHANT_ITEM || !pSpellInfo->ReagentCount[0])
            continue;


        spellIds.push_back(spellId);
        break;
    }

    return spellIds;
}

uint32 HasReagentsForValue::Calculate()
{
    if (ai->HasCheat(BotCheatMask::item))
        return true;

    uint32 spellId = stoi(getQualifier());

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    if (!pSpellInfo)
        return false;

    uint32 craftCount = 9999;

    for (uint8 i = 0; i < MAX_SPELL_REAGENTS; i++)
    {
        if (pSpellInfo->ReagentCount[i] > 0 && pSpellInfo->Reagent[i])
        {
            const ItemPrototype* reqProto = sObjectMgr.GetItemPrototype(pSpellInfo->Reagent[i]);

            uint32 count = AI_VALUE2(uint32, "item count", ChatHelper::formatItem(reqProto));

            if (count < pSpellInfo->ReagentCount[i])
                return 0;

            if (craftCount > count / pSpellInfo->ReagentCount[i])
                craftCount = count / pSpellInfo->ReagentCount[i];
        }
    }

    return craftCount;
}

bool CanCraftSpellValue::Calculate()
{
    uint32 spellId = stoi(getQualifier());

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);

    if (!pSpellInfo)
        return false;

    if (AI_VALUE2(uint32, "has reagents for", spellId) == 0)
        return false;

    return true;
}

bool ShouldCraftSpellValue::Calculate()
{
    uint32 spellId = stoi(getQualifier());

    if (SpellGivesSkillUp(spellId, bot))
        return true;

    const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(spellId);
    if (!pSpellInfo)
        return false;

#ifdef MANGOSBOT_TWO
    if (pSpellInfo->Effect[0] == SPELL_EFFECT_CREATE_ITEM_2) //todo proper check what items are created and if they are needed.
        return true;
#endif

    for (uint8 i = 0; i < MAX_EFFECT_INDEX; i++)
    {
        if (pSpellInfo->EffectItemType[i])
        {
            ItemUsage usage = AI_VALUE2_LAZY(ItemUsage, "item usage", std::to_string(pSpellInfo->EffectItemType[i]));

            bool needItem = false;

            switch (usage)
            {
                case ItemUsage::ITEM_USAGE_EQUIP:
                case ItemUsage::ITEM_USAGE_BAD_EQUIP:
                case ItemUsage::ITEM_USAGE_QUEST:
                case ItemUsage::ITEM_USAGE_USE:
                case ItemUsage::ITEM_USAGE_FORCE_NEED:
                {
                    needItem = true;
                    break;
                }
                case ItemUsage::ITEM_USAGE_SKILL:
                case ItemUsage::ITEM_USAGE_AMMO:
                case ItemUsage::ITEM_USAGE_DISENCHANT:
                case ItemUsage::ITEM_USAGE_AH:
                case ItemUsage::ITEM_USAGE_BROKEN_AH:
                case ItemUsage::ITEM_USAGE_VENDOR:
                case ItemUsage::ITEM_USAGE_FORCE_GREED:
                {
                    needItem = !ai->HasCheat(BotCheatMask::item);
                    break;
                }
                case ItemUsage::ITEM_USAGE_NONE:
                case ItemUsage::ITEM_USAGE_KEEP:
                case ItemUsage::ITEM_USAGE_BROKEN_EQUIP:
                case ItemUsage::ITEM_USAGE_GUILD_TASK:
                {
                    needItem = false;
                    break;
                }
                default:
                {
                    needItem = false;
                    break;
                }
            }

            if (needItem)
                return true;
        }
    }

    return false;
}

inline int SkillGainChance(uint32 SkillValue, uint32 GrayLevel, uint32 GreenLevel, uint32 YellowLevel)
{
    if (SkillValue >= GrayLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREY) * 10;
    else if (SkillValue >= GreenLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_GREEN) * 10;
    else if (SkillValue >= YellowLevel)
        return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_YELLOW) * 10;

    return sWorld.getConfig(CONFIG_UINT32_SKILL_CHANCE_ORANGE) * 10;
}

bool ShouldCraftSpellValue::SpellGivesSkillUp(uint32 spellId, Player* bot)
{
    SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(spellId);
    for (SkillLineAbilityMap::const_iterator _spell_idx = bounds.first; _spell_idx != bounds.second; ++_spell_idx)
    {
        SkillLineAbilityEntry const* skill = _spell_idx->second;
        if (skill->skillId)
        {
            uint32 skillValue = bot->GetSkillValuePure(skill->skillId);
            uint32 maxSkillValue = bot->GetSkillMaxPure(skill->skillId);

            if (maxSkillValue <= skillValue)
                continue;

            uint32 craft_skill_gain = sWorld.getConfig(CONFIG_UINT32_SKILL_GAIN_CRAFTING);

            if (SkillGainChance(skillValue,
                skill->max_value,
                (skill->max_value + skill->min_value) / 2,
                skill->min_value) > 0)
                return true;
        }
    }

    return false;
}