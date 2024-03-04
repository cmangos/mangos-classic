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

#ifndef ENTITIES_MGR_H
#define ENTITIES_MGR_H

#include "Common.h"
#include <memory>
#include "ObjectGuid.h"
#include "Util/Timer.h"


#define TIMER_UPDATE_PLAYER_TIMELAPS 1000

class Map;
class WorldObject;
class Unit;
class Creature;
class GameObject;
class Player;
class Pet;
class DynamicObject;
class Corpse;
struct CreatureData;
struct GameObjectData;

typedef std::list<WorldObject*> WorldObjectList;
typedef std::set<WorldObject*> WorldObjectSet;
typedef std::unordered_set<WorldObject*> WorldObjectUnSet;
typedef std::list<Unit*> UnitList;
typedef std::list<Creature*> CreatureList;
typedef std::list<GameObject*> GameObjectList;
typedef std::list<DynamicObject*> DynamicObjectList;
typedef std::list<Corpse*> CorpseList;
typedef std::list<Player*> PlayerList;
typedef std::unordered_set<Player*> PlayerSet;
typedef std::map<uint64, Player*> PlayersMap;
typedef std::map<uint32, Creature*> CreaturesMap;
typedef std::map<uint32, GameObject*> GameObjectsMap;
typedef std::map<uint32, DynamicObject*> DynamicObjectsMap;
typedef std::map<uint32, Corpse*> PlayerCorpsesMap;

#endif
