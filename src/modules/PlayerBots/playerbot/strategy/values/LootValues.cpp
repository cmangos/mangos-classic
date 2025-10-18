#include "playerbot/playerbot.h"
#include "SharedValueContext.h"
#include "LootValues.h"
#include "playerbot/strategy/actions/LootAction.h"

using namespace ai;

std::vector<LootItem*> LootAccess::GetLootContentFor(Player* player) const
{
	std::vector<LootItem*> retvec;

	for (LootItemList::const_iterator lootItemItr = m_lootItems.begin(); lootItemItr != m_lootItems.end(); ++lootItemItr)
	{
		retvec.push_back(*lootItemItr);
	}

	return retvec;
}

// Get loot status for a specified player
uint32 LootAccess::GetLootStatusFor(Player const* player) const
{
	uint32 status = 0;

	if (m_isFakeLoot && m_playersOpened.empty())
		return LOOT_STATUS_FAKE_LOOT;

	if (m_gold != 0)
		status |= LOOT_STATUS_CONTAIN_GOLD;

	for (auto lootItem : m_lootItems)
	{
		Loot const* loot = reinterpret_cast<Loot const*>(this);
		LootSlotType slotType = lootItem->GetSlotTypeForSharedLoot(player, loot);
		if (slotType == MAX_LOOT_SLOT_TYPE)
			continue;

		status |= LOOT_STATUS_NOT_FULLY_LOOTED;

		if (lootItem->freeForAll)
			status |= LOOT_STATUS_CONTAIN_FFA;

		if (lootItem->isReleased)
			status |= LOOT_STATUS_CONTAIN_RELEASED_ITEMS;
	}
	return status;
}

// Is there is any loot available for provided player
bool LootAccess::IsLootedFor(Player const* player) const
{
	return (GetLootStatusFor(player) == 0);
}

bool LootAccess::IsLootedForAll() const
{
	for (auto itr : m_ownerSet)
	{
		Player* player = ObjectAccessor::FindPlayer(itr);
		if (!player)
			continue;

		if (!IsLootedFor(player))
			return false;
	}
	return true;
}

LootTemplateAccess const* DropMapValue::GetLootTemplate(ObjectGuid guid, LootType type)
{
	LootTemplate const* lTemplate = nullptr;

	if (guid.IsCreature())
	{
		CreatureInfo const* info = sObjectMgr.GetCreatureTemplate(guid.GetEntry());

		if (info)
		{
			if (type == LOOT_CORPSE)
				lTemplate = LootTemplates_Creature.GetLootFor(info->LootId);
			else if (type == LOOT_PICKPOCKETING && info->PickpocketLootId)
				lTemplate = LootTemplates_Pickpocketing.GetLootFor(info->PickpocketLootId);
			else if (type == LOOT_SKINNING && info->SkinningLootId)
				lTemplate = LootTemplates_Skinning.GetLootFor(info->SkinningLootId);
		}
	}
	else if (guid.IsGameObject())
	{
		GameObjectInfo const* info = sObjectMgr.GetGameObjectInfo(guid.GetEntry());

		if (info && info->GetLootId() != 0)
		{
			if (type == LOOT_CORPSE)
				lTemplate = LootTemplates_Gameobject.GetLootFor(info->GetLootId());
			else if (type == LOOT_FISHINGHOLE)
				lTemplate = LootTemplates_Fishing.GetLootFor(info->GetLootId());
		}
	}
	else if (guid.IsItem())
	{
		ItemPrototype const* proto = sObjectMgr.GetItemPrototype(guid.GetEntry());
		
		if (proto)
		{
			if (type == LOOT_CORPSE)
				lTemplate = LootTemplates_Item.GetLootFor(proto->ItemId);
			else if (type == LOOT_DISENCHANTING && proto->DisenchantID)
				lTemplate = LootTemplates_Disenchant.GetLootFor(proto->DisenchantID);
#ifdef MANGOSBOT_TWO
			if (type == LOOT_MILLING)
				lTemplate = LootTemplates_Milling.GetLootFor(proto->ItemId);
			if (type == LOOT_PROSPECTING)
				lTemplate = LootTemplates_Prospecting.GetLootFor(proto->ItemId);
#endif
		}
	}

	LootTemplateAccess const* lTemplateA = reinterpret_cast<LootTemplateAccess const*>(lTemplate);

	return lTemplateA;
}

DropMap* ItemDropMapValue::Calculate()
{
	DropMap* dropMap = new DropMap;

	for (uint32 itemId = 0; itemId < sItemStorage.GetMaxEntry(); itemId++)
	{
		ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(itemId);

		if (!proto)
			continue;

		if (!(proto->Flags & ITEM_FLAG_HAS_LOOT))
			continue;

		LootTemplateAccess const* lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_ITEM, itemId, uint32(1)), LOOT_CORPSE);

		if (lTemplateA)
		{
			for (LootStoreItem const& lItem : lTemplateA->Entries)
				dropMap->insert(std::make_pair(lItem.itemid, itemId));

			for (LootLootGroupAccess const& group : lTemplateA->Groups)
			{
				for (LootStoreItem const& lItem : group.ExplicitlyChanced)
					dropMap->insert(std::make_pair(lItem.itemid, itemId));

				for (LootStoreItem const& lItem : group.EqualChanced)
					dropMap->insert(std::make_pair(lItem.itemid, itemId));
			}
		}
	}

	return dropMap;
}

DropMap* DropMapValue::Calculate()
{
	DropMap* dropMap = new DropMap;

	int32 sEntry;

	for (uint32 entry = 0; entry < sCreatureStorage.GetMaxEntry(); entry++)
	{
		sEntry = entry;

		LootTemplateAccess const* lTemplateA = GetLootTemplate(ObjectGuid(HIGHGUID_UNIT, entry, uint32(1)), LOOT_CORPSE);

		if (lTemplateA)
		{
			for (LootStoreItem const& lItem : lTemplateA->Entries)
				dropMap->insert(std::make_pair(lItem.itemid, sEntry));

			for (LootLootGroupAccess const& group : lTemplateA->Groups)
			{
				for (LootStoreItem const& lItem : group.ExplicitlyChanced)
					dropMap->insert(std::make_pair(lItem.itemid, sEntry));

				for (LootStoreItem const& lItem : group.EqualChanced)
					dropMap->insert(std::make_pair(lItem.itemid, sEntry));
			}
		}
	}

	for (uint32 entry = 0; entry < sGOStorage.GetMaxEntry(); entry++)
	{
		sEntry = entry;

		LootTemplateAccess const* lTemplateA = GetLootTemplate(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, uint32(1)), LOOT_CORPSE);

		if (lTemplateA)
		{
			for (LootStoreItem const& lItem : lTemplateA->Entries)
				dropMap->insert(std::make_pair(lItem.itemid, -sEntry));

			for (LootLootGroupAccess const& group : lTemplateA->Groups)
			{
				for (LootStoreItem const& lItem : group.ExplicitlyChanced)
					dropMap->insert(std::make_pair(lItem.itemid, -sEntry));

				for (LootStoreItem const& lItem : group.EqualChanced)
					dropMap->insert(std::make_pair(lItem.itemid, -sEntry));
			}
		}
	}

	DropMap* itemDropMap = GAI_VALUE(DropMap*, "item drop map");

	//Add items that drop from items.
	for (auto& [lootItemId, sourceItemId] : *itemDropMap)
	{
		auto range = dropMap->equal_range(sourceItemId);
		for (auto itr = range.first; itr != range.second; ++itr)
			dropMap->insert(std::make_pair(lootItemId, itr->second));
	}

	return dropMap;
}

//What items does this entry have in its loot list?
std::list<int32> ItemDropListValue::Calculate()
{
	uint32 itemId = stoi(getQualifier());

	DropMap* dropMap = GAI_VALUE(DropMap*, "drop map");

	std::list<int32> entries;

	auto range = dropMap->equal_range(itemId);

	for (auto itr = range.first; itr != range.second; ++itr)
		entries.push_back(itr->second);

	return entries;
}

//What items does this entry have in its loot list?
std::list<uint32> EntryLootListValue::Calculate()
{
	int32 entry = stoi(getQualifier());

	std::list<uint32> items;

	DropMap* dropMap = GAI_VALUE(DropMap*, "drop map");
	for (auto it = dropMap->begin(); it != dropMap->end(); ++it)
	{
		if (it->second == entry)
		{
			items.push_back(it->first);
		}
	}

	return items;
}

//What is the item's loot chance?
float LootChanceValue::Calculate()
{
	int32 entry = getMultiQualifierInt(getQualifier(), 0, " ");
	uint32 itemId = getMultiQualifierInt(getQualifier(), 1, " ");

	LootTemplateAccess const* lTemplateA;

	if (entry > 0)
		lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_UNIT, entry, uint32(1)), LOOT_CORPSE);
	else
		lTemplateA = DropMapValue::GetLootTemplate(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, uint32(1)), LOOT_CORPSE);

	if (lTemplateA)
	{
		for (auto& item : lTemplateA->Entries)
			if (item.itemid == itemId)
				return item.chance;

		for (LootLootGroupAccess const& group : lTemplateA->Groups)
		{
			for (LootStoreItem const& item : group.ExplicitlyChanced)
				if (item.itemid == itemId)
					return item.chance;

			float equalChance = 100.0f / (float)group.EqualChanced.size();

			for (LootStoreItem const& item : group.EqualChanced)
				if (item.itemid == itemId)
					return item.chance ? item.chance : equalChance;
		}
	}

	return 0.0f;
}

itemUsageMap EntryLootUsageValue::Calculate()
{
	itemUsageMap items;

	for (auto itemId : GAI_VALUE2(std::list<uint32>, "entry loot list", getQualifier()))
	{
		items[AI_VALUE2(ItemUsage, "item usage", itemId)].push_back(itemId);
	}

	return items;
}

bool HasUpgradeValue::Calculate()
{
	for (auto itemId : GAI_VALUE2(std::list<uint32>, "entry loot list", getQualifier()))
	{
		ForceItemUsage forceUsage = AI_VALUE2_EXISTS(ForceItemUsage, "force item usage", itemId, ForceItemUsage::FORCE_USAGE_NONE);

		if (forceUsage == ForceItemUsage::FORCE_USAGE_NEED)
			return true;

		ItemQualifier qualifier(itemId);

		ItemUsage equip = ItemUsageValue::QueryItemUsageForEquip(qualifier, bot);
		if (equip == ItemUsage::ITEM_USAGE_EQUIP)
			return true;
	}
	return false;
}

//How many (stack) items can be looted while still having free space.
uint32 StackSpaceForItem::Calculate()
{
	uint32 maxValue = 999;

	uint32 itemId = stoi(getQualifier());

	ItemPrototype const* proto = sItemStorage.LookupEntry<ItemPrototype>(itemId);

	if (!proto) 
		return maxValue;

	if (proto->MaxCount > 0)
		return proto->MaxCount - AI_VALUE2(uint32, "item count", proto->Name1);

	if (ai->HasActivePlayerMaster())
		return maxValue;
	
	if (AI_VALUE(uint8, "bag space") <= 80)
		return maxValue;

	uint32 maxStack = proto->GetMaxStackSize();
	if (maxStack == 1)
		return 0;

	std::list<Item*> found = AI_VALUE2(std::list < Item*>, "inventory items", chat->formatItem(proto));

	maxValue = 0;

	for (auto stack : found)
		if (maxStack - stack->GetCount() > maxValue)
			maxValue = maxStack - stack->GetCount();

	return maxValue;
}

bool ShouldLootObject::Calculate()
{
	GuidPosition guid(stoull(getQualifier()), WorldPosition(bot));

	if (!guid)
		return false;

	WorldObject* object = guid.GetWorldObject(bot->GetInstanceId());

	if (!object)
		return false;

	if (!object->m_loot)
    {
		if (!object->IsGameObject())
			return true;

		GameObject* go = static_cast<GameObject*>(object);

		if (go->GetGoType() != GAMEOBJECT_TYPE_GOOBER)
			return true;

		uint32 spellId = go->GetSpellId();

		if (!spellId)
            return true;

		SpellEntry const* lootSpell = GetSpellStore()->LookupEntry<SpellEntry>(spellId);

		if (!lootSpell || lootSpell->Effect[0] != SPELL_EFFECT_CREATE_ITEM)
            return true;

		uint32 itemId = lootSpell->EffectItemType[0];

		if (!AI_VALUE2(uint32, "stack space for item", itemId))
            return false;

        ItemQualifier ltemQualifier(itemId);

        if (!StoreLootAction::IsLootAllowed(ltemQualifier, ai))
            return false;

		return true;				
    }

	if (object->m_loot->GetGoldAmount() > 0)
		return true;

	LootAccess const* lootAccess = reinterpret_cast<LootAccess const*>(object->m_loot);

	if (!lootAccess)
		return false;

	if (lootAccess->m_lootMethod != NOT_GROUP_TYPE_LOOT && !lootAccess->m_isChecked) //Open loot once to start rolls.
		return true;

	for (auto& lItem : lootAccess->GetLootContentFor(bot))
	{
		if (!lItem->itemId)
			continue;

		uint32 canLootAmount = AI_VALUE2(uint32, "stack space for item", lItem->itemId);

		if (canLootAmount < lItem->count)
			continue;

		ItemQualifier ltemQualifier(lItem);

		if (lootAccess->m_lootType != LOOT_SKINNING && !StoreLootAction::IsLootAllowed(ltemQualifier, ai))
			continue;

		return true;
	}

	return false;
}

void ActiveRolls::CleanUp(Player* bot, LootRollMap& rollMap, ObjectGuid guid, uint32 slot)
{
	for (auto roll = rollMap.begin(); roll != rollMap.end();)
	{
		if (guid && roll->first != guid)
		{
			++roll;
			continue;
		}

		if (slot && roll->second != slot)
		{
			++roll;
			continue;
		}

		Loot* loot = sLootMgr.GetLoot(bot, roll->first);
		if (!loot)
		{
			roll = rollMap.erase(roll);
			continue;
		}

		GroupLootRoll* lootRoll = loot->GetRollForSlot(roll->second);
		if (!lootRoll)
		{
			roll = rollMap.erase(roll);
			continue;
		}

		if(guid)
		{
			roll = rollMap.erase(roll);
			continue;
		}

		++roll;
	}
}

std::string ActiveRolls::Format()
{
	std::ostringstream out;

	for (auto& roll : value)
	{
		WorldObject* wo = ai->GetWorldObject(roll.first);

		if (wo)
			out << wo->GetName();
		else
			out << roll.first;

		std::string itemLink;

		Loot* loot = sLootMgr.GetLoot(bot, roll.first);
		if (loot)
		{
			LootItem* item = loot->GetLootItemInSlot(roll.second);

			if (item)
			{
				const ItemPrototype* proto = sItemStorage.LookupEntry<ItemPrototype>(item->itemId);

				if (proto)
				{
					itemLink = ChatHelper::formatItem(proto);
				}
			}
		}

		if (itemLink.empty())
			out << roll.second;
		else
			out << itemLink;

		out << ",";
	}

	return out.str().c_str();
}
