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

#ifndef MANGOS_OBJECTACCESSOR_H
#define MANGOS_OBJECTACCESSOR_H

#include "Common.h"
#include "Platform/Define.h"
#include "Policies/Singleton.h"
#include "Policies/ThreadingModel.h"

#include "UpdateData.h"

#include "GridDefines.h"
#include "Object.h"
#include "Player.h"
#include "Corpse.h"

#include <mutex>

class Unit;
class WorldObject;
class Map;

template <class T>
class HashMapHolder
{
    public:

        typedef std::unordered_map<ObjectGuid, T*>   MapType;
        typedef std::mutex LockType;
        typedef std::lock_guard<std::mutex> ReadGuard;
        typedef std::lock_guard<std::mutex> WriteGuard;

        static void Insert(T* o)
        {
            WriteGuard guard(i_lock);
            m_objectMap[o->GetObjectGuid()] = o;
        }

        static void Remove(T* o)
        {
            WriteGuard guard(i_lock);
            m_objectMap.erase(o->GetObjectGuid());
        }

        static T* Find(ObjectGuid guid)
        {
            ReadGuard guard(i_lock);
            typename MapType::iterator itr = m_objectMap.find(guid);
            return (itr != m_objectMap.end()) ? itr->second : nullptr;
        }

        static MapType& GetContainer() { return m_objectMap; }

        static LockType& GetLock() { return i_lock; }

    private:

        // Non instanceable only static
        HashMapHolder() {}

        static LockType i_lock;
        static MapType  m_objectMap;
};

class ObjectAccessor : public MaNGOS::Singleton<ObjectAccessor, MaNGOS::ClassLevelLockable<ObjectAccessor, std::mutex> >
{
        friend class MaNGOS::OperatorNew<ObjectAccessor>;

        ObjectAccessor();
        ~ObjectAccessor();
        ObjectAccessor(const ObjectAccessor&);
        ObjectAccessor& operator=(const ObjectAccessor&);

    public:
        typedef std::unordered_map<ObjectGuid, Corpse*> Player2CorpsesMapType;

        // Search player at any map in world and other objects at same map with `obj`
        // Note: recommended use Map::GetUnit version if player also expected at same map only
        static Unit* GetUnit(WorldObject const& obj, ObjectGuid guid);

        // Player access
        static Player* FindPlayer(ObjectGuid guid, bool inWorld = true);// if need player at specific map better use Map::GetPlayer
        static Player* FindPlayerByName(const char* name);
        static void KickPlayer(ObjectGuid guid);

        HashMapHolder<Player>::MapType& GetPlayers()
        {
            return HashMapHolder<Player>::GetContainer();
        }

        void SaveAllPlayers();

        // Corpse access
        Corpse* GetCorpseForPlayerGUID(ObjectGuid guid);
        static Corpse* GetCorpseInMap(ObjectGuid guid, uint32 mapid);
        void RemoveCorpse(Corpse* corpse);
        void AddCorpse(Corpse* corpse);
        void AddCorpsesToGrid(GridPair const& gridpair, GridType& grid, Map* map);
        Corpse* ConvertCorpseForPlayer(ObjectGuid player_guid, bool insignia = false);
        void RemoveOldCorpses();

        // For call from Player/Corpse AddToWorld/RemoveFromWorld only
        void AddObject(Corpse* object) { HashMapHolder<Corpse>::Insert(object); }
        void AddObject(Player* object) { HashMapHolder<Player>::Insert(object); }
        void RemoveObject(Corpse* object) { HashMapHolder<Corpse>::Remove(object); }
        void RemoveObject(Player* object) { HashMapHolder<Player>::Remove(object); }

    private:

        Player2CorpsesMapType   i_player2corpse;

        typedef std::mutex LockType;
        typedef MaNGOS::GeneralLock<LockType > Guard;

        LockType i_playerGuard;
        LockType i_corpseGuard;
};

#define sObjectAccessor ObjectAccessor::Instance()

#endif
