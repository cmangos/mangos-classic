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

#include "Globals/ObjectAccessor.h"
#include "Globals/ObjectMgr.h"
#include "Policies/Singleton.h"
#include "Entities/Player.h"
#include "Entities/Item.h"
#include "Entities/Corpse.h"
#include "Maps/MapManager.h"
#include "Maps/Map.h"
#include "Grids/CellImpl.h"
#include "Grids/GridNotifiersImpl.h"
#include "Entities/ObjectGuid.h"
#include "World/World.h"

#include <mutex>

#define CLASS_LOCK MaNGOS::ClassLevelLockable<ObjectAccessor, std::mutex>
INSTANTIATE_SINGLETON_2(ObjectAccessor, CLASS_LOCK);
INSTANTIATE_CLASS_MUTEX(ObjectAccessor, std::mutex);

template<class T>
void HashMapHolder<T>::Insert(T* o)
{
    WriteGuard guard(i_lock);
    m_objectMap[o->GetObjectGuid()] = o;
}

template<class T>
void HashMapHolder<T>::Remove(T* o)
{
    WriteGuard guard(i_lock);
    m_objectMap.erase(o->GetObjectGuid());
}

template<class T>
T* HashMapHolder<T>::Find(ObjectGuid guid)
{
    ReadGuard guard(i_lock);
    typename MapType::iterator itr = m_objectMap.find(guid);
    return (itr != m_objectMap.end()) ? itr->second : nullptr;
}

template<class T>
typename HashMapHolder<T>::MapType& HashMapHolder<T>::GetContainer() { return m_objectMap; }

template<class T>
typename HashMapHolder<T>::LockType& HashMapHolder<T>::GetLock() { return i_lock; }

ObjectAccessor::ObjectAccessor() {}
ObjectAccessor::~ObjectAccessor()
{
    for (Player2CorpsesMapType::const_iterator itr = i_player2corpse.begin(); itr != i_player2corpse.end(); ++itr)
    {
        itr->second->RemoveFromWorld();
        delete itr->second;
    }
}

Unit*
ObjectAccessor::GetUnit(WorldObject const& u, ObjectGuid guid)
{
    if (!guid)
        return nullptr;

    if (guid.IsPlayer())
        return FindPlayer(guid);

    if (!u.IsInWorld())
        return nullptr;

    return u.GetMap()->GetAnyTypeCreature(guid);
}

Corpse* ObjectAccessor::GetCorpseInMap(ObjectGuid guid, uint32 mapid)
{
    Corpse* ret = HashMapHolder<Corpse>::Find(guid);
    if (!ret)
        return nullptr;
    if (ret->GetMapId() != mapid)
        return nullptr;

    return ret;
}

Player* ObjectAccessor::FindPlayer(ObjectGuid guid, bool inWorld /*= true*/)
{
    if (!guid)
        return nullptr;

    Player* plr = HashMapHolder<Player>::Find(guid);
    if (!plr || (!plr->IsInWorld() && inWorld))
        return nullptr;

    return plr;
}

Player* ObjectAccessor::FindPlayerByName(const char* name)
{
    HashMapHolder<Player>::ReadGuard g(HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (HashMapHolder<Player>::MapType::iterator iter = m.begin(); iter != m.end(); ++iter)
        if (iter->second->IsInWorld() && (::strcmp(name, iter->second->GetName()) == 0))
            return iter->second;

    return nullptr;
}

void
ObjectAccessor::SaveAllPlayers() const
{
    HashMapHolder<Player>::ReadGuard g(HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType& m = sObjectAccessor.GetPlayers();
    for (auto& itr : m)
    {
        if (itr.second->IsInWorld())
            itr.second->GetMap()->GetMessager().AddMessage([guid = itr.second->GetObjectGuid()](Map* map)
            {
                if (Player* player = map->GetPlayer(guid))
                    player->SaveToDB();
            });
        else
            itr.second->SaveToDB();
    }
}

void ObjectAccessor::KickPlayer(ObjectGuid guid)
{
    if (Player* p = ObjectAccessor::FindPlayer(guid, false))
    {
        WorldSession* s = p->GetSession();
        s->KickPlayer();                            // mark session to remove at next session list update
    }
}

Corpse*
ObjectAccessor::GetCorpseForPlayerGUID(ObjectGuid guid)
{
    Guard guard(i_corpseGuard);

    Player2CorpsesMapType::iterator iter = i_player2corpse.find(guid);
    if (iter == i_player2corpse.end())
        return nullptr;

    MANGOS_ASSERT(iter->second->GetType() != CORPSE_BONES);

    return iter->second;
}

void
ObjectAccessor::RemoveCorpse(Corpse* corpse)
{
    MANGOS_ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    Guard guard(i_corpseGuard);
    Player2CorpsesMapType::iterator iter = i_player2corpse.find(corpse->GetOwnerGuid());
    if (iter == i_player2corpse.end())
        return;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord * TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr.DeleteCorpseCellData(corpse->GetMapId(), cell_id, corpse->GetOwnerGuid().GetCounter());
    corpse->RemoveFromWorld();

    i_player2corpse.erase(iter);
}

void
ObjectAccessor::AddCorpse(Corpse* corpse)
{
    MANGOS_ASSERT(corpse && corpse->GetType() != CORPSE_BONES);

    Guard guard(i_corpseGuard);
    MANGOS_ASSERT(i_player2corpse.find(corpse->GetOwnerGuid()) == i_player2corpse.end());
    i_player2corpse[corpse->GetOwnerGuid()] = corpse;

    // build mapid*cellid -> guid_set map
    CellPair cell_pair = MaNGOS::ComputeCellPair(corpse->GetPositionX(), corpse->GetPositionY());
    uint32 cell_id = (cell_pair.y_coord * TOTAL_NUMBER_OF_CELLS_PER_MAP) + cell_pair.x_coord;

    sObjectMgr.AddCorpseCellData(corpse->GetMapId(), cell_id, corpse->GetOwnerGuid().GetCounter(), corpse->GetInstanceId());
}

void
ObjectAccessor::AddCorpsesToGrid(GridPair const& gridpair, GridType& grid, Map* map)
{
    Guard guard(i_corpseGuard);
    for (auto& iter : i_player2corpse)
        if (iter.second->GetGrid() == gridpair)
        {
            // verify, if the corpse in our instance (add only corpses which are)
            if (map->Instanceable())
            {
                if (iter.second->GetInstanceId() == map->GetInstanceId())
                {
                    grid.AddWorldObject(iter.second);
                }
            }
            else
            {
                grid.AddWorldObject(iter.second);
            }
        }
}

Corpse*
ObjectAccessor::ConvertCorpseForPlayer(ObjectGuid player_guid, bool insignia)
{
    Corpse* corpse = GetCorpseForPlayerGUID(player_guid);
    if (!corpse)
    {
        // in fact this function is called from several places
        // even when player doesn't have a corpse, not an error
        // sLog.outError("Try remove corpse that not in map for GUID %ul", player_guid);
        return nullptr;
    }

    DEBUG_LOG("Deleting Corpse and spawning bones.");

    // remove corpse from player_guid -> corpse map
    RemoveCorpse(corpse);

    // remove resurrectable corpse from grid object registry (loaded state checked into call)
    // do not load the map if it's not loaded
    Map* map = sMapMgr.FindMap(corpse->GetMapId(), corpse->GetInstanceId());
    if (map)
        map->Remove(corpse, false);

    // remove corpse from DB
    corpse->DeleteFromDB();

    Corpse* bones = nullptr;
    // create the bones only if the map and the grid is loaded at the corpse's location
    // ignore bones creating option in case insignia
    if (map && (insignia ||
                (map->IsBattleGround() ? sWorld.getConfig(CONFIG_BOOL_DEATH_BONES_BG) : sWorld.getConfig(CONFIG_BOOL_DEATH_BONES_WORLD))) &&
            !map->IsRemovalGrid(corpse->GetPositionX(), corpse->GetPositionY()))
    {
        // Create bones, don't change Corpse
        bones = new Corpse;
        bones->Create(corpse->GetGUIDLow());

        for (int i = 3; i < CORPSE_END; ++i)                // don't overwrite guid and object type
            bones->SetUInt32Value(i, corpse->GetUInt32Value(i));

        bones->SetGrid(corpse->GetGrid());
        // bones->m_time = m_time;                          // don't overwrite time
        // bones->m_inWorld = m_inWorld;                    // don't overwrite world state
        // bones->m_type = m_type;                          // don't overwrite type
        bones->Relocate(corpse->GetPositionX(), corpse->GetPositionY(), corpse->GetPositionZ(), corpse->GetOrientation());

        bones->SetUInt32Value(CORPSE_FIELD_FLAGS, CORPSE_FLAG_UNK2 | CORPSE_FLAG_BONES);
        bones->SetOwnerGuid(player_guid);

        for (int i = 0; i < EQUIPMENT_SLOT_END; ++i)
        {
            if (corpse->GetUInt32Value(CORPSE_FIELD_ITEM + i))
                bones->SetUInt32Value(CORPSE_FIELD_ITEM + i, 0);
        }

        // add bones in grid store if grid loaded where corpse placed
        map->Add(bones);
    }

    // all references to the corpse should be removed at this point
    delete corpse;

    return bones;
}

void ObjectAccessor::RemoveOldCorpses()
{
    time_t now = time(nullptr);
    Player2CorpsesMapType::iterator next;
    for (Player2CorpsesMapType::iterator itr = i_player2corpse.begin(); itr != i_player2corpse.end(); itr = next)
    {
        next = itr;
        ++next;

        if (!itr->second->IsExpired(now))
            continue;

        ConvertCorpseForPlayer(itr->first);
    }
}

/// Define the static member of HashMapHolder

template <class T> typename HashMapHolder<T>::MapType HashMapHolder<T>::m_objectMap;
template <class T> std::mutex HashMapHolder<T>::i_lock;

/// Global definitions for the hashmap storage

template class HashMapHolder<Player>;
template class HashMapHolder<Corpse>;
