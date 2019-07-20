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

#ifndef MANGOS_GRIDNOTIFIERS_H
#define MANGOS_GRIDNOTIFIERS_H

#include "Entities/UpdateData.h"

#include "Entities/Corpse.h"
#include "Entities/Object.h"
#include "Entities/DynamicObject.h"
#include "Entities/GameObject.h"
#include "Entities/Player.h"
#include "Entities/Unit.h"

#include <memory>

namespace MaNGOS
{
    struct VisibleNotifier
    {
        Camera& i_camera;
        UpdateData i_data;
        GuidSet i_clientGUIDs;
        WorldObjectSet i_visibleNow;

        explicit VisibleNotifier(Camera& c) : i_camera(c), i_clientGUIDs(c.GetOwner()->m_clientGUIDs) {}
        template<class T> void Visit(GridRefManager<T>& m);
        void Visit(CameraMapType& /*m*/) {}
        void Notify(void);
    };

    struct VisibleChangesNotifier
    {
        WorldObject& i_object;

        explicit VisibleChangesNotifier(WorldObject& object) : i_object(object) {}
        template<class T> void Visit(GridRefManager<T>&) {}
        void Visit(CameraMapType&);
    };

    struct MessageDeliverer
    {
        Player const& i_player;
        WorldPacket const& i_message;
        bool i_toSelf;
        MessageDeliverer(Player const& pl, WorldPacket const& msg, bool to_self) : i_player(pl), i_message(msg), i_toSelf(to_self) {}
        void Visit(CameraMapType& m);
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct MessageDelivererExcept
    {
        WorldPacket const&  i_message;
        Player const* i_skipped_receiver;

        MessageDelivererExcept(WorldPacket const& msg, Player const* skipped)
            : i_message(msg), i_skipped_receiver(skipped) {}

        void Visit(CameraMapType& m);
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct ObjectMessageDeliverer
    {
        WorldPacket const& i_message;
        explicit ObjectMessageDeliverer(WorldPacket const& msg) : i_message(msg) {}
        void Visit(CameraMapType& m);
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct MessageDistDeliverer
    {
        Player const& i_player;
        WorldPacket const& i_message;
        bool i_toSelf;
        bool i_ownTeamOnly;
        float i_dist;

        MessageDistDeliverer(Player const& pl, WorldPacket const& msg, float dist, bool to_self, bool ownTeamOnly)
            : i_player(pl), i_message(msg), i_toSelf(to_self), i_ownTeamOnly(ownTeamOnly), i_dist(dist) {}
        void Visit(CameraMapType& m);
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct ObjectMessageDistDeliverer
    {
        WorldObject const& i_object;
        WorldPacket const& i_message;
        float i_dist;
        ObjectMessageDistDeliverer(WorldObject const& obj, WorldPacket const& msg, float dist) : i_object(obj), i_message(msg), i_dist(dist) {}
        void Visit(CameraMapType& m);
        template<class SKIP> void Visit(GridRefManager<SKIP>&) {}
    };

    struct ObjectUpdater
    {
        explicit ObjectUpdater(WorldObjectUnSet& otus) : m_objectToUpdateSet(otus) {}
        template<class T> void Visit(GridRefManager<T>& m);
        void Visit(PlayerMapType&) {}
        void Visit(CorpseMapType&) {}
        void Visit(CameraMapType&) {}
        void Visit(CreatureMapType&);

      private:
      WorldObjectUnSet& m_objectToUpdateSet;
    };

    struct PlayerVisitObjectsNotifier
    {
        Player& i_player;
        PlayerVisitObjectsNotifier(Player& pl) : i_player(pl) {}
        template<class T> void Visit(GridRefManager<T>&) {}
#ifdef _MSC_VER
        template<> void Visit(PlayerMapType&);
        template<> void Visit(CreatureMapType&);
#endif
    };

    struct CreatureVisitObjectsNotifier
    {
        Creature& i_creature;
        CreatureVisitObjectsNotifier(Creature& c) : i_creature(c) {}
        template<class T> void Visit(GridRefManager<T>&) {}
#ifdef _MSC_VER
        template<> void Visit(PlayerMapType&);
        template<> void Visit(CreatureMapType&);
#endif
    };

    struct DynamicObjectUpdater
    {
        DynamicObject& i_dynobject;
        Unit* i_check;
        bool i_positive;
        DynamicObjectUpdater(DynamicObject& dynobject, Unit* caster, bool positive) : i_dynobject(dynobject), i_positive(positive)
        {
            i_check = caster;
            Unit* owner = i_check->GetOwner();
            if (owner)
                i_check = owner;
        }

        template<class T> inline void Visit(GridRefManager<T>&) {}
#ifdef _MSC_VER
        template<> inline void Visit<Player>(PlayerMapType&);
        template<> inline void Visit<Creature>(CreatureMapType&);
#endif

        void VisitHelper(Unit* target);
    };

    // SEARCHERS & LIST SEARCHERS & WORKERS

    /* Model Searcher class:
    template<class Check>
    struct SomeSearcher
    {
        ResultType& i_result;
        Check & i_check;

        SomeSearcher(ResultType& result, Check & check)
            : i_phaseMask(check.GetFocusObject().GetPhaseMask()), i_result(result), i_check(check) {}

        void Visit(CreatureMapType &m);
        {
            ..some code fast return if result found

            for(CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                if (!itr->getSource()->InSamePhase(i_phaseMask))
                    continue;

                if (!i_check(itr->getSource()))
                    continue;

                ..some code for update result and possible stop search
            }
        }

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED> &) {}
    };
    */

    // WorldObject searchers & workers

    template<class Check>
    struct WorldObjectSearcher
    {
        WorldObject*& i_object;
        Check& i_check;

        WorldObjectSearcher(WorldObject*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(GameObjectMapType& m);
        void Visit(PlayerMapType& m);
        void Visit(CreatureMapType& m);
        void Visit(CorpseMapType& m);
        void Visit(DynamicObjectMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Check>
    struct WorldObjectListSearcher
    {
        WorldObjectList& i_objects;
        Check& i_check;

        WorldObjectListSearcher(WorldObjectList& objects, Check& check) : i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType& m);
        void Visit(CreatureMapType& m);
        void Visit(CorpseMapType& m);
        void Visit(GameObjectMapType& m);
        void Visit(DynamicObjectMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Do>
    struct WorldObjectWorker
    {
        Do const& i_do;

        explicit WorldObjectWorker(Do const& _do) : i_do(_do) {}

        void Visit(GameObjectMapType& m)
        {
            for (GameObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        void Visit(PlayerMapType& m)
        {
            for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }
        void Visit(CreatureMapType& m)
        {
            for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        void Visit(CorpseMapType& m)
        {
            for (CorpseMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        void Visit(DynamicObjectMapType& m)
        {
            for (DynamicObjectMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Gameobject searchers

    template<class Check>
    struct GameObjectSearcher
    {
        GameObject*& i_object;
        Check& i_check;

        GameObjectSearcher(GameObject*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(GameObjectMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Last accepted by Check GO if any (Check can change requirements at each call)
    template<class Check>
    struct GameObjectLastSearcher
    {
        GameObject*& i_object;
        Check& i_check;

        GameObjectLastSearcher(GameObject*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(GameObjectMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Check>
    struct GameObjectListSearcher
    {
        GameObjectList& i_objects;
        Check& i_check;

        GameObjectListSearcher(GameObjectList& objects, Check& check) : i_objects(objects), i_check(check) {}

        void Visit(GameObjectMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Unit searchers

    // First accepted by Check Unit if any
    template<class Check>
    struct UnitSearcher
    {
        Unit*& i_object;
        Check& i_check;

        UnitSearcher(Unit*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(CreatureMapType& m);
        void Visit(PlayerMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Last accepted by Check Unit if any (Check can change requirements at each call)
    template<class Check>
    struct UnitLastSearcher
    {
        Unit*& i_object;
        Check& i_check;

        UnitLastSearcher(Unit*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(CreatureMapType& m);
        void Visit(PlayerMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // All accepted by Check units if any
    template<class Check>
    struct UnitListSearcher
    {
        UnitList& i_objects;
        Check& i_check;

        UnitListSearcher(UnitList& objects, Check& check) : i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType& m);
        void Visit(CreatureMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Creature searchers

    template<class Check>
    struct CreatureSearcher
    {
        Creature*& i_object;
        Check& i_check;

        CreatureSearcher(Creature*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(CreatureMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Last accepted by Check Creature if any (Check can change requirements at each call)
    template<class Check>
    struct CreatureLastSearcher
    {
        Creature*& i_object;
        Check& i_check;

        CreatureLastSearcher(Creature*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(CreatureMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Check>
    struct CreatureListSearcher
    {
        CreatureList& i_objects;
        Check& i_check;

        CreatureListSearcher(CreatureList& objects, Check& check) : i_objects(objects), i_check(check) {}

        void Visit(CreatureMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Do>
    struct CreatureWorker
    {
        Do& i_do;

        CreatureWorker(WorldObject const* /*searcher*/, Do& _do) : i_do(_do) {}

        void Visit(CreatureMapType& m)
        {
            for (CreatureMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // Player searchers

    template<class Check>
    struct PlayerSearcher
    {
        Player*& i_object;
        Check& i_check;

        PlayerSearcher(Player*& result, Check& check) : i_object(result), i_check(check) {}

        void Visit(PlayerMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Check>
    struct PlayerListSearcher
    {
        PlayerList& i_objects;
        Check& i_check;

        PlayerListSearcher(PlayerList& objects, Check& check)
            : i_objects(objects), i_check(check) {}

        void Visit(PlayerMapType& m);

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Do>
    struct PlayerWorker
    {
        Do& i_do;

        explicit PlayerWorker(Do& _do) : i_do(_do) {}

        void Visit(PlayerMapType& m)
        {
            for (PlayerMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
                i_do(itr->getSource());
        }

        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    template<class Do>
    struct CameraDistWorker
    {
        WorldObject const* i_searcher;
        float i_dist;
        Do& i_do;

        CameraDistWorker(WorldObject const* searcher, float _dist, Do& _do)
            : i_searcher(searcher), i_dist(_dist), i_do(_do) {}

        void Visit(CameraMapType& m)
        {
            for (CameraMapType::iterator itr = m.begin(); itr != m.end(); ++itr)
            {
                Camera* camera = itr->getSource();
                if (camera->GetBody()->IsWithinDist(i_searcher, i_dist))
                    i_do(camera->GetOwner());
            }
        }
        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    struct CameraDistLambdaWorker
    {
        WorldObject const* i_searcher;
        float i_dist;
        std::function<void(Player*)> const& i_do;

        CameraDistLambdaWorker(WorldObject const* searcher, float _dist, std::function<void(Player*)> const& _do)
            : i_searcher(searcher), i_dist(_dist), i_do(_do) {}

        void Visit(CameraMapType& m)
        {
            for (auto& itr : m)
            {
                Camera* camera = itr.getSource();
                if (camera->GetBody()->IsWithinDist(i_searcher, i_dist))
                    i_do(camera->GetOwner());
            }
        }
        template<class NOT_INTERESTED> void Visit(GridRefManager<NOT_INTERESTED>&) {}
    };

    // CHECKS && DO classes

    /* Model Check class:
    class SomeCheck
    {
        public:
            SomeCheck(SomeObjecType const* fobj, ..some other args) : i_fobj(fobj), ...other inits {}
            WorldObject const& GetFocusObject() const { return *i_fobj; }
            bool operator()(Creature* u)                    and for other intresting typs (Player/GameObject/Camera
            {
                return ..(code return true if Object fit to requirenment);
            }
            template<class NOT_INTERESTED> bool operator()(NOT_INTERESTED*) { return false; }
        private:
            SomeObjecType const* i_fobj;                    // Focus object used for check distance from, phase, so place in world
            ..other values need for check
    };
    */

    // Can only work for unit since WO -> Corpse reaction is undefined
    class CannibalizeObjectCheck
    {
        public:
            CannibalizeObjectCheck(Unit const* fobj, float range) : i_fobj(fobj), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_fobj; }
            bool operator()(Player* u)
            {
                if (i_fobj->CanAssist(u) || u->isAlive() || u->IsTaxiFlying())
                    return false;

                return i_fobj->IsWithinDistInMap(u, i_range);
            }
            bool operator()(Corpse* u);
            bool operator()(Creature* u)
            {
                if (i_fobj->CanAssist(u) || u->isAlive() || u->IsTaxiFlying() ||
                        (u->GetCreatureTypeMask() & CREATURE_TYPEMASK_HUMANOID_OR_UNDEAD) == 0)
                    return false;

                return i_fobj->IsWithinDistInMap(u, i_range);
            }
            template<class NOT_INTERESTED> bool operator()(NOT_INTERESTED*) { return false; }
        private:
            Unit const* i_fobj;
            float i_range;
    };

    // WorldObject do classes

    class RespawnDo
    {
        public:
            RespawnDo() {}
            void operator()(Creature* u) const;
            void operator()(GameObject* u) const;
            void operator()(WorldObject*) const {}
            void operator()(Corpse*) const {}
    };

    // GameObject checks

    class GameObjectFocusCheck
    {
        public:
            GameObjectFocusCheck(Unit const* unit, uint32 focusId) : i_unit(unit), i_focusId(focusId) {}
            WorldObject const& GetFocusObject() const { return *i_unit; }
            bool operator()(GameObject* go) const
            {
                GameObjectInfo const* goInfo = go->GetGOInfo();
                if (goInfo->type != GAMEOBJECT_TYPE_SPELL_FOCUS)
                    return false;

                if (goInfo->spellFocus.focusId != i_focusId)
                    return false;

                float dist = (float)goInfo->spellFocus.dist;

                return go->IsWithinDistInMap(i_unit, dist);
            }
        private:
            Unit const* i_unit;
            uint32 i_focusId;
    };

    // Find the nearest Fishing hole and return true only if source object is in range of hole
    class NearestGameObjectFishingHoleCheck
    {
        public:
            NearestGameObjectFishingHoleCheck(WorldObject const& obj, float range) : i_obj(obj), i_range(range) {}
            WorldObject const& GetFocusObject() const { return i_obj; }
            bool operator()(GameObject* go)
            {
                if (go->GetGOInfo()->type == GAMEOBJECT_TYPE_FISHINGHOLE && go->IsSpawned() && i_obj.IsWithinDistInMap(go, i_range) && i_obj.IsWithinDistInMap(go, (float)go->GetGOInfo()->fishinghole.radius))
                {
                    i_range = i_obj.GetDistance(go, true, DIST_CALC_COMBAT_REACH);
                    return true;
                }
                return false;
            }
            float GetLastRange() const { return i_range; }
        private:
            WorldObject const& i_obj;
            float  i_range;

            // prevent clone
            NearestGameObjectFishingHoleCheck(NearestGameObjectFishingHoleCheck const&);
    };

    // Success at unit in range, range update for next check (this can be used with GameobjectLastSearcher to find nearest GO)
    class NearestGameObjectEntryInObjectRangeCheck
    {
        public:
            NearestGameObjectEntryInObjectRangeCheck(WorldObject const& obj, uint32 entry, float range) : i_obj(obj), i_entry(entry), i_range(range) {}
            WorldObject const& GetFocusObject() const { return i_obj; }
            bool operator()(GameObject* go)
            {
                if (go->GetEntry() == i_entry && i_obj.IsWithinDistInMap(go, i_range))
                {
                    i_range = i_obj.GetDistance(go);        // use found GO range as new range limit for next check
                    return true;
                }
                return false;
            }
            float GetLastRange() const { return i_range; }
        private:
            WorldObject const& i_obj;
            uint32 i_entry;
            float  i_range;

            // prevent clone this object
            NearestGameObjectEntryInObjectRangeCheck(NearestGameObjectEntryInObjectRangeCheck const&);
    };

    // Success at go in range
    class AllGameObjectEntriesListInObjectRangeCheck
    {
        public:
            AllGameObjectEntriesListInObjectRangeCheck(WorldObject const& obj, std::set<uint32>& entries, float range, bool is3D = true) : i_obj(obj), i_entries(entries), i_range(range), i_is3D(is3D) {}
            WorldObject const& GetFocusObject() const { return i_obj; }
            bool operator()(GameObject* go)
            {
                if (go->IsSpawned() && i_entries.find(go->GetEntry()) != i_entries.end() && i_obj.IsWithinDistInMap(go, i_range, i_is3D))
                    return true;

                return false;
            }

            std::vector<uint32> i_ranges;
        private:
            WorldObject const& i_obj;
            std::set<uint32>& i_entries;
            float  i_range;
            bool   i_is3D;

            // prevent clone this object
            AllGameObjectEntriesListInObjectRangeCheck(AllGameObjectEntriesListInObjectRangeCheck const&);
    };

    // x y z version of above
    class AllGameObjectEntriesListInPosRangeCheck
    {
    public:
        AllGameObjectEntriesListInPosRangeCheck(float x, float y, float z, std::set<uint32>& entries, float range, bool is3D = true) : i_x(x), i_y(y), i_z(z), i_entries(entries), i_range(range), i_is3D(is3D) {}
        bool operator()(GameObject* go)
        {
            if (go->IsSpawned() && i_entries.find(go->GetEntry()) != i_entries.end() && go->GetDistance(i_x, i_y, i_z, DIST_CALC_COMBAT_REACH) < i_range)
                return true;

            return false;
        }

        std::vector<uint32> i_ranges;
    private:
        float i_x, i_y, i_z;
        std::set<uint32>& i_entries;
        float  i_range;
        bool   i_is3D;

        // prevent clone this object
        AllGameObjectEntriesListInPosRangeCheck(AllGameObjectEntriesListInObjectRangeCheck const&);
    };

    // Success at gameobject in range of xyz, range update for next check (this can be use with GameobjectLastSearcher to find nearest GO)
    class NearestGameObjectEntryInPosRangeCheck
    {
        public:
            NearestGameObjectEntryInPosRangeCheck(WorldObject const& obj, uint32 entry, float x, float y, float z, float range)
                : i_obj(obj), i_entry(entry), i_x(x), i_y(y), i_z(z), i_range(range) {}

            WorldObject const& GetFocusObject() const { return i_obj; }

            bool operator()(GameObject* go)
            {
                if (go->GetEntry() == i_entry && go->IsWithinDist3d(i_x, i_y, i_z, i_range))
                {
                    // use found GO range as new range limit for next check
                    i_range = go->GetDistance(i_x, i_y, i_z);
                    return true;
                }

                return false;
            }

            float GetLastRange() const { return i_range; }

        private:
            WorldObject const& i_obj;
            uint32 i_entry;
            float i_x, i_y, i_z;
            float i_range;

            // prevent clone this object
            NearestGameObjectEntryInPosRangeCheck(NearestGameObjectEntryInPosRangeCheck const&);
    };

    // Success at gameobject with entry in range of provided xyz
    class GameObjectEntryInPosRangeCheck
    {
        public:
            GameObjectEntryInPosRangeCheck(WorldObject const& obj, uint32 entry, float x, float y, float z, float range)
                : i_obj(obj), i_entry(entry), i_x(x), i_y(y), i_z(z), i_range(range) {}

            WorldObject const& GetFocusObject() const { return i_obj; }

            bool operator()(GameObject* go)
            {
                return go->GetEntry() == i_entry && go->IsWithinDist3d(i_x, i_y, i_z, i_range);
            }

            float GetLastRange() const { return i_range; }

        private:
            WorldObject const& i_obj;
            uint32 i_entry;
            float i_x, i_y, i_z;
            float i_range;

            // prevent clone this object
            GameObjectEntryInPosRangeCheck(GameObjectEntryInPosRangeCheck const&);
    };

    class GameObjectInPosRangeCheck
    {
        public:
            GameObjectInPosRangeCheck(WorldObject const& obj, float x, float y, float z, float range)
                : i_obj(obj), i_x(x), i_y(y), i_z(z), i_range(range) {}

            WorldObject const& GetFocusObject() const { return i_obj; }

            bool operator()(GameObject* go)
            {
                return go->IsWithinDist3d(i_x, i_y, i_z, i_range);
            }

            float GetLastRange() const { return i_range; }

        private:
            WorldObject const& i_obj;
            float i_x, i_y, i_z;
            float i_range;

            // prevent clone this object
            GameObjectInPosRangeCheck(GameObjectEntryInPosRangeCheck const&);
    };

    // Unit checks

    class MostHPMissingInRangeCheck
    {
        public:
            MostHPMissingInRangeCheck(Unit const* obj, float range, float hp, bool onlyInCombat, bool targetSelf) : i_obj(obj), i_range(range), i_hp(hp), i_onlyInCombat(onlyInCombat), i_targetSelf(targetSelf) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                if (!u->isAlive() || (i_onlyInCombat && !u->isInCombat()))
                    return false;

                if (!i_targetSelf && u == i_obj)
                    return false;

                if (i_obj->CanAssist(u) && i_obj->IsWithinDistInMap(u, i_range))
                {
                    if (u->GetMaxHealth() - u->GetHealth() > i_hp)
                    {
                        i_hp = u->GetMaxHealth() - u->GetHealth();
                        return true;
                    }
                }
                return false;
            }
        private:
            Unit const* i_obj;
            float i_range;
            float i_hp;
            bool i_onlyInCombat;
            bool i_targetSelf;
    };

    class MostHPPercentMissingInRangeCheck
    {
        public:
            MostHPPercentMissingInRangeCheck(Unit const* obj, float range, float hp, bool onlyInCombat, bool targetSelf) : i_obj(obj), i_range(range), i_hp(hp), i_onlyInCombat(onlyInCombat), i_targetSelf(targetSelf) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                if (!u->isAlive() || (i_onlyInCombat && !u->isInCombat()))
                    return false;

                if (!i_targetSelf && u == i_obj)
                    return false;

                if (i_obj->CanAssist(u) && i_obj->IsWithinDistInMap(u, i_range))
                {
                    if (100.f - u->GetHealthPercent() > i_hp)
                    {
                        i_hp = 100.f - u->GetHealthPercent();
                        return true;
                    }
                }
                return false;
            }
        private:
            Unit const* i_obj;
            float i_range;
            float i_hp;
            bool i_onlyInCombat;
            bool i_targetSelf;
    };

    class FriendlyCCedInRangeCheck
    {
        public:
            FriendlyCCedInRangeCheck(Unit const* obj, float range) : i_obj(obj), i_range(range) {}
            Unit const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                return u->isAlive() && u->isInCombat() && i_obj->CanAssist(u) && i_obj->IsWithinDistInMap(u, i_range) && (u->IsImmobilized() || u->GetMaxNegativeAuraModifier(SPELL_AURA_MOD_DECREASE_SPEED) || u->isFeared() || u->IsPolymorphed() || u->isFrozen() || u->hasUnitState(UNIT_STAT_CAN_NOT_REACT));
            }
        private:
            Unit const* i_obj;
            float i_range;
    };

    class FriendlyMissingBuffInRangeCheck
    {
        public:
            FriendlyMissingBuffInRangeCheck(Unit const* obj, float range, uint32 spellid) : i_obj(obj), i_range(range), i_spell(spellid) {}
            Unit const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                return u->isAlive() && u->isInCombat() && i_obj->CanAssist(u) && i_obj->IsWithinDistInMap(u, i_range) && !(u->HasAura(i_spell, EFFECT_INDEX_0) || u->HasAura(i_spell, EFFECT_INDEX_1) || u->HasAura(i_spell, EFFECT_INDEX_2));
            }
        private:
            Unit const* i_obj;
            float i_range;
            uint32 i_spell;
    };

    class AnyUnfriendlyUnitInObjectRangeCheck
    {
        public:
            AnyUnfriendlyUnitInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range)
            {
                i_controlledByPlayer = obj->IsControlledByPlayer();
            }
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u) const
            {
                return u->isAlive() && i_obj->CanAttackSpell(u) && i_obj->IsWithinDistInMap(u, i_range) && i_obj->IsWithinLOSInMap(u);
            }
        private:
            WorldObject const* i_obj;
            bool i_controlledByPlayer;
            float i_range;
    };

    class AnyFriendlyUnitInObjectRangeCheck
    {
        public:
            AnyFriendlyUnitInObjectRangeCheck(WorldObject const* obj, SpellEntry const* spellInfo, float range) : i_obj(obj), i_spellInfo(spellInfo), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                return u->isAlive() && i_obj->IsWithinDistInMap(u, i_range) && i_obj->CanAssistSpell(u, i_spellInfo);
            }
        private:
            WorldObject const* i_obj;
            SpellEntry const* i_spellInfo;
            float i_range;
    };

    class AnyFriendlyOrGroupMemberUnitInUnitRangeCheck
    {
        public:
            AnyFriendlyOrGroupMemberUnitInUnitRangeCheck(Unit const* obj, Group const* group, SpellEntry const* spellInfo, float range)
                : i_group(group), i_obj(obj), i_spellInfo(spellInfo), i_range(range) {}
            Unit const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                if (!u->isAlive() || !i_obj->IsWithinDistInMap(u, i_range) || !i_obj->CanAssistSpell(u, i_spellInfo))
                    return false;

                //if group is defined then we apply group members only filtering
                if (i_group)
                {
                    switch (u->GetTypeId())
                    {
                        case TYPEID_PLAYER:
                        {
                            if (i_group != static_cast<Player*>(u)->GetGroup())
                                return false;

                            break;
                        }
                        case TYPEID_UNIT:
                        {
                            Creature* creature = static_cast<Creature*>(u);

                            //the only other possible group members besides players are pets, we exclude anyone else
                            if (!creature->IsPet())
                                return false;

                            Group* group = nullptr;

                            if (Unit* owner = creature->GetOwner(nullptr, true))
                            {
                                if (owner->GetTypeId() == TYPEID_PLAYER)
                                {
                                    group = static_cast<Player*>(owner)->GetGroup();
                                }
                            }

                            if (i_group != group)
                                return false;

                            break;
                        }
                        default: return false;
                    }
                }

                return true;
            }
        private:
            Group const* i_group;
            Unit const* i_obj;
            SpellEntry const* i_spellInfo;
            float i_range;
    };

    class AnyUnitInObjectRangeCheck
    {
        public:
            AnyUnitInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                return u->isAlive() && i_obj->IsWithinDistInMap(u, i_range);
            }
        private:
            WorldObject const* i_obj;
            float i_range;
    };

    class AnyUnitFulfillingConditionInRangeCheck
    {
        public:
            AnyUnitFulfillingConditionInRangeCheck(WorldObject const* obj, std::function<bool(Unit*)> functor, float radius, DistanceCalculation type = DIST_CALC_COMBAT_REACH)
                    : i_obj(obj), i_functor(functor), i_range(radius), i_type(type) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                return i_functor(u) && i_obj->GetDistance(u, true, i_type) <= i_range;
            }
        private:
            WorldObject const* i_obj;
            std::function<bool(Unit*)> i_functor;
            float i_range;
            DistanceCalculation i_type;
    };

    // Success at unit in range, range update for next check (this can be use with UnitLastSearcher to find nearest unit)
    class NearestAttackableUnitInObjectRangeCheck
    {
        public:
            NearestAttackableUnitInObjectRangeCheck(Unit* source, Unit* owner, float range) : m_source(source), m_owner(owner), m_range(range) {}
            NearestAttackableUnitInObjectRangeCheck(NearestAttackableUnitInObjectRangeCheck const&) =  delete;

            Unit const& GetFocusObject() const { return *m_source; }

            bool operator()(Unit* currUnit)
            {
                if (currUnit->isAlive() && (m_source->IsAttackedBy(currUnit) || (m_owner && m_owner->IsAttackedBy(currUnit)) || m_source->IsEnemy(currUnit))
                    && m_source->CanAttack(currUnit)
                    && currUnit->IsVisibleForOrDetect(m_source, m_source, false)
                    && m_source->IsWithinDistInMap(currUnit, m_range))
                {
                    m_range = m_source->GetDistance(currUnit);        // use found unit range as new range limit for next check
                    return true;
                }

                return false;
            }

        private:
            Unit*       m_source;
            Unit*       m_owner;
            float       m_range;
    };

    class AnyAoETargetUnitInObjectRangeCheck
    {
        public:
            AnyAoETargetUnitInObjectRangeCheck(WorldObject const* obj, SpellEntry const* spellInfo, float range)
                : i_obj(obj), i_spellInfo(spellInfo), i_range(range)
            {
                i_targetForPlayer = i_obj->IsControlledByPlayer();
            }
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Unit* u)
            {
                // Check contains checks for: live, non-selectable, non-attackable flags, flight check and GM check, ignore totems
                if (u->GetTypeId() == TYPEID_UNIT && ((Creature*)u)->IsTotem())
                    return false;

                return i_obj->CanAttackSpell(u, i_spellInfo) && i_obj->IsWithinDistInMap(u, i_range);
            }

        private:
            WorldObject const* i_obj;
            SpellEntry const* i_spellInfo;
            float i_range;
            bool i_targetForPlayer;
    };

    // do attack at call of help to friendly crearture
    class CallOfHelpCreatureInRangeDo
    {
        public:
            CallOfHelpCreatureInRangeDo(Unit* funit, Unit* enemy, float range)
                : i_funit(funit), i_enemy(enemy), i_range(range)
            {}
            void operator()(Creature* u);

        private:
            Unit* const i_funit;
            Unit* const i_enemy;
            float i_range;
    };

    class AnyDeadUnitCheck
    {
        public:
            explicit AnyDeadUnitCheck(WorldObject const* fobj) : i_fobj(fobj) {}
            WorldObject const& GetFocusObject() const { return *i_fobj; }
            bool operator()(Unit* u) { return !u->isAlive(); }
        private:
            WorldObject const* i_fobj;
    };

    class AnyStealthedCheck
    {
        public:
            explicit AnyStealthedCheck(WorldObject const* fobj) : i_fobj(fobj) {}
            WorldObject const& GetFocusObject() const { return *i_fobj; }
            bool operator()(Unit* u) { return u->GetVisibility() == VISIBILITY_GROUP_STEALTH; }
        private:
            WorldObject const* i_fobj;
    };

    // Creature checks

    class AnyAssistCreatureInRangeCheck
    {
        public:
            AnyAssistCreatureInRangeCheck(Unit* funit, Unit* enemy, float range)
                : i_funit(funit), i_enemy(enemy), i_range(range)
            {
            }
            WorldObject const& GetFocusObject() const { return *i_funit; }
            bool operator()(Creature* u);

        private:
            Unit* const i_funit;
            Unit* const i_enemy;
            float i_range;
    };

    class NearestAssistCreatureInCreatureRangeCheck
    {
        public:
            NearestAssistCreatureInCreatureRangeCheck(Creature* obj, Unit* enemy, float range)
                : i_obj(obj), i_enemy(enemy), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Creature* u)
            {
                if (u == i_obj)
                    return false;
                if (!u->CanAssist(i_obj) || !u->CanAttack(i_enemy))
                    return false;

                if (!i_obj->IsWithinDistInMap(u, i_range))
                    return false;

                if (!i_obj->IsWithinLOSInMap(u))
                    return false;

                i_range = i_obj->GetDistance(u);            // use found unit range as new range limit for next check
                return true;
            }
            float GetLastRange() const { return i_range; }

            // prevent clone this object
            NearestAssistCreatureInCreatureRangeCheck(NearestAssistCreatureInCreatureRangeCheck const&);

        private:
            Creature* const i_obj;
            Unit* const i_enemy;
            float  i_range;
    };

    // Success at unit in range, range update for next check (this can be use with CreatureLastSearcher to find nearest creature)
    class NearestCreatureEntryWithLiveStateInObjectRangeCheck
    {
        public:
            NearestCreatureEntryWithLiveStateInObjectRangeCheck(WorldObject const& obj, uint32 entry, bool onlyAlive, bool onlyDead, float range, bool excludeSelf = false, bool is3D = true)
                : i_obj(obj), i_entry(entry), i_range(range * range), i_onlyAlive(onlyAlive), i_onlyDead(onlyDead), i_excludeSelf(excludeSelf), i_is3D(is3D), i_foundOutOfRange(false) {}
            WorldObject const& GetFocusObject() const { return i_obj; }
            bool operator()(Creature* u)
            {
                if (u->GetEntry() == i_entry && ((i_onlyAlive && u->isAlive()) || (i_onlyDead && u->IsCorpse()) || (!i_onlyAlive && !i_onlyDead)) && (!i_excludeSelf || (&i_obj != u)))
                {
                    float dist = i_obj.GetDistance(u, true, DIST_CALC_NONE);
                    if (dist < i_range)
                    {
                        i_range = dist; // use found unit range as new range limit for next check
                        return true;
                    }
                    i_foundOutOfRange = true;
                }
                return false;
            }
            float GetLastRange() const { return sqrt(i_range); }
        private:
            WorldObject const& i_obj;
            uint32 i_entry;
            float  i_range;
            bool   i_onlyAlive;
            bool   i_onlyDead;
            bool   i_excludeSelf;
            bool   i_is3D;
            bool   i_foundOutOfRange;

            // prevent clone this object
            NearestCreatureEntryWithLiveStateInObjectRangeCheck(NearestCreatureEntryWithLiveStateInObjectRangeCheck const&);
    };

    // Success at unit in range, range update for next check (this can be used with CreatureListSearcher to find creatures with given entry)
    class AllCreatureEntriesWithLiveStateInObjectRangeCheck
    {
        public:
            AllCreatureEntriesWithLiveStateInObjectRangeCheck(WorldObject const& obj, std::set<uint32>& entries, bool alive, float range, bool guids = false, bool excludeSelf = false, bool is3D = true)
                : i_obj(obj), i_entries(entries), i_range(range), i_guids(guids), i_alive(alive), i_excludeSelf(excludeSelf), i_is3D(is3D), i_foundOutOfRange(false) {}
            WorldObject const& GetFocusObject() const { return i_obj; }
            bool operator()(Creature* u)
            {
                if (i_entries.find(i_guids ? u->GetGUIDLow() : u->GetEntry()) != i_entries.end() && ((i_alive && u->isAlive()) || (!i_alive && u->IsCorpse())) && (!i_excludeSelf || (&i_obj != u)))
                {
                    if (i_obj.IsWithinCombatDistInMap(u, i_range, i_is3D))
                        return true;
                    i_foundOutOfRange = true;
                }
                return false;
            }
            bool FoundOutOfRange() const { return i_foundOutOfRange; }
        private:
            WorldObject const& i_obj;
            std::set<uint32>& i_entries;
            float  i_range;
            bool   i_guids;
            bool   i_alive;
            bool   i_excludeSelf;
            bool   i_is3D;
            bool   i_foundOutOfRange;

            // prevent clone this object
            AllCreatureEntriesWithLiveStateInObjectRangeCheck(AllCreatureEntriesWithLiveStateInObjectRangeCheck const&);
    };

    class AllCreaturesOfEntryInRangeCheck
    {
        public:
            AllCreaturesOfEntryInRangeCheck(const WorldObject* pObject, uint32 uiEntry, float fMaxRange) : m_pObject(pObject), m_uiEntry(uiEntry), m_fRange(fMaxRange) {}
            WorldObject const& GetFocusObject() const { return *m_pObject; }
            bool operator()(Unit* pUnit)
            {
                return pUnit->GetEntry() == m_uiEntry && m_pObject->IsWithinDist(pUnit, m_fRange, false);
            }

            // prevent clone this object
            AllCreaturesOfEntryInRangeCheck(AllCreaturesOfEntryInRangeCheck const&);

        private:
            const WorldObject* m_pObject;
            uint32 m_uiEntry;
            float m_fRange;
    };

    // Player checks and do

    class AnyPlayerInObjectRangeCheck
    {
        public:
            AnyPlayerInObjectRangeCheck(WorldObject const* obj, float range) : i_obj(obj), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Player* u)
            {
                return u->isAlive() && i_obj->IsWithinDistInMap(u, i_range);
            }
        private:
            WorldObject const* i_obj;
            float i_range;
    };

    class AnyPlayerInObjectRangeWithAuraCheck
    {
        public:
            AnyPlayerInObjectRangeWithAuraCheck(WorldObject const* obj, float range, uint32 spellId)
                : i_obj(obj), i_range(range), i_spellId(spellId) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Player* u)
            {
                return u->isAlive()
                       && i_obj->IsWithinDistInMap(u, i_range)
                       && u->HasAura(i_spellId);
            }
        private:
            WorldObject const* i_obj;
            float i_range;
            uint32 i_spellId;
    };

    class AnyPlayerInCapturePointRange
    {
        public:
            AnyPlayerInCapturePointRange(WorldObject const* obj, float range)
                : i_obj(obj), i_range(range) {}
            WorldObject const& GetFocusObject() const { return *i_obj; }
            bool operator()(Player* u)
            {
                return u->CanUseCapturePoint() &&
                       i_obj->IsWithinDistInMap(u, i_range);
            }
        private:
            WorldObject const* i_obj;
            float i_range;
    };

    // Prepare using Builder localized packets with caching and send to player
    template<class Builder>
    class LocalizedPacketDo
    {
        public:
            explicit LocalizedPacketDo(Builder& builder) : i_builder(builder) {}

            void operator()(Player* p);

        private:
            Builder& i_builder;
            std::vector<std::unique_ptr<WorldPacket>> i_data_cache;         // 0 = default, i => i-1 locale index
    };

    // Prepare using Builder localized packets with caching and send to player
    template<class Builder>
    class LocalizedPacketListDo
    {
        public:
            typedef std::vector<std::unique_ptr<WorldPacket>> WorldPacketList;
            explicit LocalizedPacketListDo(Builder& builder) : i_builder(builder) {}

            void operator()(Player* p);

        private:
            Builder& i_builder;
            std::vector<WorldPacketList> i_data_cache;
            // 0 = default, i => i-1 locale index
    };

#ifndef _MSC_VER
    template<> void PlayerVisitObjectsNotifier::Visit<Player>(PlayerMapType&);
    template<> void PlayerVisitObjectsNotifier::Visit<Creature>(CreatureMapType&);
    template<> void CreatureVisitObjectsNotifier::Visit<Player>(PlayerMapType&);
    template<> void CreatureVisitObjectsNotifier::Visit<Creature>(CreatureMapType&);
    template<> inline void DynamicObjectUpdater::Visit<Creature>(CreatureMapType&);
    template<> inline void DynamicObjectUpdater::Visit<Player>(PlayerMapType&);
#endif
}
#endif
