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

#ifndef _MAP_WORKERS_H_INCLUDED
#define _MAP_WORKERS_H_INCLUDED

#include "Grids/Cell.h"
#include "Grids/GridNotifiersImpl.h"
#include "MapUpdater.h"
#include "MotionGenerators/MovementGenerator.h"
#include "Entities/Object.h"
#include "Platform/Define.h"

class Worker
{
    public:
        Worker(MapUpdater& updater) : m_updater(updater) {}
        virtual ~Worker() = default;
        virtual void execute() {};

    protected:
        MapUpdater& GetWorker() { return m_updater; }

    private:
        MapUpdater& m_updater;
};

class MapUpdateWorker : public Worker
{
    public:
        MapUpdateWorker(Map& map, uint32 diff, MapUpdater& updater) :
            Worker(updater), m_map(map), m_diff(diff)
        {}

        void execute() override
        {
            m_map.Update(m_diff);
            GetWorker().update_finished();
        }

    private:
        Map& m_map;
        uint32 m_diff;
};

class GridCrawler : public Worker
{
    public:
        GridCrawler(Map& map, std::vector<Cell> &cells, uint32 diff, MapUpdater& updater) :
            Worker(updater), m_map(map), m_cells(cells), m_diff(diff)
        {}

        void execute() override
        {
            WorldObjectUnSet objToUpdate;
            MaNGOS::ObjectUpdater obj_updater(objToUpdate, m_diff);
            TypeContainerVisitor<MaNGOS::ObjectUpdater, GridTypeMapContainer  > grid_object_update(obj_updater);    // For creature
            TypeContainerVisitor<MaNGOS::ObjectUpdater, WorldTypeMapContainer > world_object_update(obj_updater);   // For pets

            for (auto &cell : m_cells)
            {
                m_map.Visit(cell, grid_object_update);
                m_map.Visit(cell, world_object_update);
            }

            GetWorker().update_finished();
        }

    private:
        Map& m_map;
        std::vector<Cell> &m_cells;
        uint32 m_diff;
};


class ObjectUpdateWorker : public Worker
{
    public:
        ObjectUpdateWorker(std::unordered_set<WorldObject*>& objects, uint32 diff, MapUpdater& updater) :
            Worker(updater), m_objects(objects), m_diff(diff)
        {}

        void execute() override
        {
            for (WorldObject* const &object : m_objects)
                object->Update(m_diff);

            GetWorker().update_finished();
        }

    private:
        std::unordered_set<WorldObject*>& m_objects;
        uint32 m_diff;
};

#endif //_MAP_WORKERS_H_INCLUDED
