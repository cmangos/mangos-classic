#include "MountValues.h"
#include "playerbot/ChatHelper.h"
#include "playerbot/strategy/AiObjectContext.h"
#include "BudgetValues.h"
#include "SharedValueContext.h"

using namespace ai;

uint32 MountValue::GetSpeed(uint32 spellId, bool canFly)
{
#ifdef MANGOSBOT_ZERO
    if (canFly)
        return 0;
#endif

    const SpellEntry* const spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    if (!spellInfo)
        return 0;

    switch (spellInfo->Id) //Aura's hard coded in spell.cpp
    {
    case 783:  //travel form
    case 2645: //ghost wolf   
        if(!canFly)
        return 39;
        break;
    case 33943: //flight form
        if (canFly)
            return 59;
        break;
    case 40120: //swift flight form
        if (canFly)
            return 279;
        break;
    case 26656: //Black AQ mount
        if (!canFly)
            return 99;
    }

    bool isMount = false;
    for (int i = 0; i < 3; i++)
    {
        if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOUNTED)
        {
            isMount = true;
            break;
        }
    }

    if(!isMount)
        return 0;

#ifndef MANGOSBOT_ZERO
    //This part stops bots from mounting flying mounts when they can't fly. This should be tweaked if bots ever are able to normally ride flying mounts in the old-world.
    if (isMount && !canFly)
    {
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo->EffectApplyAuraName[i] == SPELL_AURA_MOD_FLIGHT_SPEED_MOUNTED)
            {
                return 0;
            }
        }
    }
#endif

#ifdef MANGOSBOT_ZERO
    uint32 effect = SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED;
#else
    uint32 effect = canFly ? SPELL_AURA_MOD_FLIGHT_SPEED_MOUNTED : SPELL_AURA_MOD_INCREASE_MOUNTED_SPEED; //If we can fly only look at flight speed. Normal mounts then don't get any speed.
#endif

    if (isMount)
    {
        for (int i = 0; i < 3; i++)
        {
            if (spellInfo->EffectApplyAuraName[i] == effect)
            {
                return spellInfo->EffectBasePoints[i];
            }
        }
    }

    return 0;
}

uint32 MountValue::GetMountSpell(uint32 itemId)
{
    const ItemPrototype* proto = sObjectMgr.GetItemPrototype(itemId);

    if (!proto)
        return 0;

    uint32 speed = 0;
    for (int j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        if (GetSpeed(proto->Spells[j].SpellId))
            return proto->Spells[j].SpellId;

        if (GetSpeed(proto->Spells[j].SpellId, true))
            return proto->Spells[j].SpellId;
    }

    return 0;
}

bool MountValue::IsValidLocation(Player* bot)
{
    if (GetSpeed(true)) //Flying mount
    {
        if (bot->GetMapId() != 530 && bot->GetMapId() != 571)
            return false;

#ifdef MANGOSBOT_ONE
        uint32 zone, area;
        bot->GetZoneAndAreaId(zone, area);
        uint32 v_map = GetVirtualMapForMapAndZone(bot->GetMapId(), zone);
        MapEntry const* mapEntry = sMapStore.LookupEntry(v_map);
        if (!mapEntry || mapEntry->addon < 1 || !mapEntry->IsContinent())
            return false;
#endif
#ifdef MANGOSBOT_TWO
        uint32 zone, area;
        bot->GetZoneAndAreaId(zone, area);
        if (!bot->CanStartFlyInArea(bot->GetMapId(), zone, area, false))
            return false;
#endif
    }

    const SpellEntry* const spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

    bool isAQ40Mounted = false;

    switch (spellInfo->Id)
    {
    case 25863:    // spell used by ingame item for Black Qiraji mount (legendary reward)
    case 26655:    // spells also related to Black Qiraji mount but use/trigger unknown
    case 26656:
    case 31700:
        if (bot->GetMapId() == 531)
            isAQ40Mounted = true;
        break;
    case 25953:    // spells of the 4 regular AQ40 mounts
    case 26054:
    case 26055:
    case 26056:
        if (bot->GetMapId() == 531)
        {
            isAQ40Mounted = true;
            break;
        }
        else
            return false; //SPELL_FAILED_NOT_HERE;
    default:
        break;
    }

    // Ignore map check if spell have AreaId. AreaId already checked and this prevent special mount spells
    if (bot->GetTypeId() == TYPEID_PLAYER &&
        !isAQ40Mounted &&   // [-ZERO] && !m_spellInfo->AreaId)
        !bot->GetMap()->IsMountAllowed())
    {
        return false;  //SPELL_FAILED_NO_MOUNTS_ALLOWED;
    }

    if (bot->GetAreaId() == 35)
        return false; //SPELL_FAILED_NO_MOUNTS_ALLOWED;

    return true;
}

uint32 CurrentMountSpeedValue::Calculate()
{
    Unit* unit = AI_VALUE(Unit*, getQualifier());

    uint32 mountSpeed = 0;

    for (uint32 auraType = SPELL_AURA_BIND_SIGHT; auraType < TOTAL_AURAS; auraType++)
    {
        Unit::AuraList const& auras = unit->GetAurasByType((AuraType)auraType);

        if (auras.empty())
            continue;

        for (Unit::AuraList::const_iterator i = auras.begin(); i != auras.end(); i++)
        {
            Aura* aura = *i;
            if (!aura)
                continue;

            SpellEntry const* auraSpell = aura->GetSpellProto();

            uint32 auraSpeed = MountValue::GetSpeed(auraSpell->Id);

            if (auraSpeed < mountSpeed)
                continue;

            mountSpeed = auraSpeed;
        }
    }

    return mountSpeed;
}

std::vector<MountValue> FullMountListValue::Calculate()
{
    std::vector<MountValue> mounts;

    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const* pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
        if (!pProto)
            continue;

        if (!MountValue::GetMountSpell(pProto->ItemId))
            continue;

        mounts.push_back(MountValue(pProto));

        GAI_VALUE2(std::list<int32>, "item vendor list", pProto->ItemId);
    }

    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (!spellInfo)
            continue;

        if (!MountValue::IsMountSpell(spellInfo->Id))
            continue;

        mounts.push_back(MountValue(spellInfo->Id));
    }

    return mounts;
}

std::vector<MountValue> MountListValue::Calculate()
{
    std::vector<MountValue> mounts;

	for (auto& mount : AI_VALUE2(std::list<Item*>, "inventory items", "mount"))
		mounts.push_back(MountValue(mount));

    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        if (itr->second.state != PLAYERSPELL_REMOVED && !itr->second.disabled && !IsPassiveSpell(itr->first))
            if(MountValue::IsMountSpell(itr->first))
                mounts.push_back(MountValue(itr->first));

    return mounts;
}

uint32 MaxMountSpeedValue::Calculate()
{
    bool canFly = !qualifier.empty();

    std::vector<MountValue> mounts = AI_VALUE(std::vector<MountValue>, "mount list");

    uint32 maxSpeed = 0;

    for (auto& mount : mounts)
        maxSpeed = std::max(maxSpeed, mount.GetSpeed(canFly));

    return maxSpeed;
}

std::string MountListValue::Format()
{
    std::ostringstream out; out << "{";
    for (auto& mount : this->Calculate())
    {
        std::string speed = std::to_string(mount.GetSpeed(false) + 1) + "%" + (mount.GetSpeed(true) ? ("/" + (std::to_string(mount.GetSpeed(true) + 1) + "%")) : "");
        out << (mount.IsItem() ? "(item)" : "(spell)") << chat->formatSpell(mount.GetSpellId()) << "(" << speed.c_str() << "),";
    }
    out << "}";
    return out.str();
}

uint32 MountSkillTypeValue::Calculate()
{
#ifdef MANGOSBOT_ZERO
    switch (bot->getRace())
    {
    case RACE_HUMAN:
        return SKILL_RIDING_HORSE;
    case RACE_ORC:
        return SKILL_RIDING_WOLF;
        break;
    case RACE_NIGHTELF:
        return SKILL_RIDING_TIGER;
        break;
    case RACE_DWARF:
        return SKILL_RIDING_RAM;
        break;
    case RACE_TROLL:
        return SKILL_RIDING_RAPTOR;
        break;
    case RACE_GNOME:
        return SKILL_RIDING_MECHANOSTRIDER;
        break;
    case RACE_UNDEAD:
        return SKILL_RIDING_UNDEAD_HORSE;
        break;
    case RACE_TAUREN:
        return SKILL_RIDING_KODO;
        break;
    default:
        return SKILL_RIDING;
        break;
    }
#else
    return SKILL_RIDING;
#endif
}

std::vector<int32> AvailableMountVendors::Calculate()
{
    std::vector<int32> mountVendors;
    std::vector<MountValue> mountList = GAI_VALUE(std::vector<MountValue>, "full mount list");
    
    for (auto& mount : mountList)
    {
        if (!mount.IsItem())
            continue;

        uint32 itemId = mount.GetItemProto()->ItemId;

        ItemUsage usage = AI_VALUE2_LAZY(ItemUsage, "item usage", itemId);

        if (usage != ItemUsage::ITEM_USAGE_EQUIP)
            continue;

        for (auto& vendor : GAI_VALUE2(std::list<int32>, "item vendor list", itemId))
            if(std::find(mountVendors.begin(), mountVendors.end(), vendor) == mountVendors.end())
                mountVendors.push_back(vendor);
    }

    return mountVendors;
}

bool CanTrainMountValue::Calculate()
{
    if (bot->GetSkill(AI_VALUE(uint32, "mount skilltype"), true, true) >= bot->GetSkill(AI_VALUE(uint32, "mount skilltype"), true, true, true))
        return false;

    if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::mount) < AI_VALUE2(uint32, "total money needed for", (uint32)NeedMoneyFor::mount))
        return false;

    return true;
}

bool CanBuyMountValue::Calculate()
{
#ifdef MANGOSBOT_ZERO
    uint8 minRidingLevel = 40;
#endif
#ifdef MANGOSBOT_ONE
    uint8 minRidingLevel = 30;
#endif
#ifdef MANGOSBOT_TWO
    uint8 minRidingLevel = 20;
#endif
    if (bot->GetLevel() < minRidingLevel)
        return false;

    if (!AI_VALUE(bool, "can buy"))
        return false;

    if (AI_VALUE2(uint32, "money needed for", (uint32)NeedMoneyFor::mount) == 0)
        return false;

    if (AI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::mount) < AI_VALUE2(uint32, "total money needed for", (uint32)NeedMoneyFor::mount))
        return false;

    return true;
}