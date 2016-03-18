/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MANGOS_BAG_H
#define MANGOS_BAG_H

#include "Common.h"
#include "Item.h"

// Maximum 28 Slots ( (CONTAINER_END - CONTAINER_FIELD_SLOT_1)/2
#define MAX_BAG_SIZE 28                                     // 1.12

enum InventorySlot
{
    NULL_BAG = 0,
    NULL_SLOT = 255
};

class Bag : public Item
{
    public:

        Bag();
        ~Bag();

        void AddToWorld() override;
        void RemoveFromWorld() override;

        bool Create(uint32 guidlow, uint32 itemid, Player const* owner) override;

        void StoreItem(uint8 slot, Item* pItem);
        void RemoveItem(uint8 slot);

        Item* GetItemByPos(uint8 slot) const;
        Item* GetItemByEntry(uint32 item) const;
        uint32 GetItemCount(uint32 item, Item* eItem = nullptr) const;

        uint8 GetSlotByItemGUID(ObjectGuid guid) const;
        bool IsEmpty() const;
        uint32 GetFreeSlots() const;
        uint32 GetBagSize() const { return GetUInt32Value(CONTAINER_FIELD_NUM_SLOTS); }

        // DB operations
        // overwrite virtual Item::SaveToDB
        void SaveToDB() override;
        // overwrite virtual Item::LoadFromDB
        bool LoadFromDB(uint32 guidLow, Field* fields, ObjectGuid ownerGuid = ObjectGuid()) override;
        // overwrite virtual Item::DeleteFromDB
        void DeleteFromDB() override;

        void BuildCreateUpdateBlockForPlayer(UpdateData* data, Player* target) const override;

    protected:

        // Bag Storage space
        Item* m_bagslot[MAX_BAG_SIZE];
};

inline Item* NewItemOrBag(ItemPrototype const* proto)
{
    if (proto->InventoryType == INVTYPE_BAG)
        return new Bag;

    return new Item;
}

#endif
