/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_GRIDSEARCH_H
#define SC_GRIDSEARCH_H

#include "Entities/Object.h"
class GameObject;
class Creature;

struct ObjectDistanceOrder
{
    const Unit* m_source;

    ObjectDistanceOrder(const Unit* source) : m_source(source) {};

    bool operator()(const WorldObject* left, const WorldObject* right) const
    {
        return m_source->GetDistanceOrder(left, right);
    }
};

struct ObjectDistanceOrderReversed
{
    const Unit* m_source;

    ObjectDistanceOrderReversed(const Unit* source) : m_source(source) {};

    bool operator()(const WorldObject* left, const WorldObject* right) const
    {
        return !m_source->GetDistanceOrder(left, right);
    }
};

GameObject* GetClosestGameObjectWithEntry(WorldObject* source, uint32 entry, float maxSearchRange);
Creature* GetClosestCreatureWithEntry(WorldObject* source, uint32 entry, float maxSearchRange, bool onlyAlive = true, bool onlyDead = false, bool excludeSelf = false);

void GetGameObjectListWithEntryInGrid(GameObjectList& goList, WorldObject* source, uint32 entry, float maxSearchRange);
void GetGameObjectListWithEntryInGrid(GameObjectList& goList, WorldObject* source, std::vector<uint32> const& entries, float maxSearchRange);
void GetCreatureListWithEntryInGrid(CreatureList& creatureList, WorldObject* source, uint32 entry, float maxSearchRange);
void GetCreatureListWithEntryInGrid(CreatureList& creatureList, WorldObject* source, std::vector<uint32> const& entries, float maxSearchRange);
void GetPlayerListWithEntryInWorld(PlayerList& playerList, WorldObject* source, float maxSearchRange);

// Used in: hyjalAI.cpp
/*
class AllFriendlyCreaturesInGrid
{
    public:
        AllFriendlyCreaturesInGrid(Unit const* obj) : pUnit(obj) {}
        bool operator() (Unit* u)
        {
            if (u->IsAlive() && u->GetVisibility() == VISIBILITY_ON && u->CanAssist(pUnit))
                return true;

            return false;
        }

    private:
        Unit const* pUnit;
};
*/

#endif
