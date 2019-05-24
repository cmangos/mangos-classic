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

#ifndef _TAXI_H
#define _TAXI_H

#include "Server/DBCStores.h"

#include <string>
#include <deque>

/////////////////////////////////////////////////
/// @file       Taxi.h
/// @date       May, 2018
/// @brief      Helper container includes for updated player taxi system.
/////////////////////////////////////////////////

class Player;

namespace Taxi
{
    // Aliases to help differentiate different numbers from each other
    typedef uint32 PathID;  // Flight path entry id
    typedef uint32 DestID;  // Destination entry id (taxi flight master stop as seen by the game internals)
    typedef uint32 Index;   // Index of a flight spline waypoint in a single flight route

    /// Route is a struct describing a single taxi flight path between 2 destinations
    struct Route
    {
        Route(PathID pathID, DestID pathStart, DestID pathEnd, Index nodeStart, Index nodeEnd, uint32 cost) :
            pathID(pathID), destStart(pathStart), destEnd(pathEnd), nodeStart(nodeStart), nodeEnd(nodeEnd), cost(cost) {}

        PathID pathID;      // Taxi flight path entry id
        DestID destStart;   // Start destination id for entry
        DestID destEnd;     // End destination for entry
        Index nodeStart;    // Real entry index start or a modified junction point for multi-route flights
        Index nodeEnd;      // Real enrty index end or a modified junction point for multi-route flights
        uint32 cost;        // Fully pre-calculated cost of this particular route (in non-persisting multi-route flights)
    };

    /// Roadmap is a sequence of Routes, the source for building a spline or a series of splines for the entire ride
    typedef std::deque<Route> Roadmap;
    /// Waypoint is an alias for TaxiPathNodeEntry, a part of the taxi flight spline
    typedef TaxiPathNodeEntry const* Waypoint;
    /// Map is a container which describes a full proto-spline made out of taxi node entries in a sequential order for a single game level
    typedef std::deque<Waypoint> Map;
    /// Atlas contains one or more Maps in a sequential order
    typedef std::deque<Map> Atlas;

    enum TrackerState
    {
        TRACKER_EMPTY    = 0, // Container is empty
        TRACKER_STAGING  = 1, // At least one route added, its possible to add more routes
        TRACKER_STANDBY  = 2, // Container is finalized: all routes and splines are calculated
        TRACKER_FLIGHT   = 3, // Container is tracking a taxi flight spline
        TRACKER_TRANSFER = 4, // Container is waiting for server to apply a new taxi flight spline after map change
    };

    /// This is a container representing player's current taxi ride's routes and destinations.
    class Tracker
    {
        public:
            explicit Tracker(Player& owner) :
                m_owner(owner), m_state(TRACKER_EMPTY), m_cost(0), m_displayId(0), m_location(nullptr), m_locationIndex(0), m_resumeIndex(0), m_debug(false) {}

            // Disable funny ctors
            Tracker(Tracker const&) = delete;
            void operator=(const Tracker&) = delete;
            Tracker(Tracker&&) = delete;
            Tracker& operator=(Tracker&&) = delete;

            /// Save contents: string serialization
            std::string Save();
            /// Load contents: string deserialization
            bool Load(std::string& string, DestID& destOrphan);

            TrackerState GetState() const { return m_state; }
            /// Notify the container to switch into certain state if applicable
            bool SetState(TrackerState state);

            /// Get current active route (a single taxi path)
            const Route& GetRoute() const { return m_routes.front(); }
            /// Get all pre-calculated routes
            const Roadmap& GetRoadmap() const { return m_routes; }
            /// Get the current spline for the map
            const Map& GetMap() const { return m_atlas.front(); }
            /// Get a collection of splines for the entire ride
            const Atlas& GetAtlas() const { return m_atlas; }

            /// Get cost of the current route
            uint32 GetCost() const { return (!m_routes.empty() ? m_routes.front().cost : 0); }
            /// Get total cost of the entire ride with all routes included
            uint32 GetCostTotal() const { return m_cost; }

            /// Get flying mount displayid for the entire ride (in effect until logout)
            uint32 GetMountDisplayId() const { return m_displayId; }

            /// Get the last tracked waypoint on the current taxi spline
            Waypoint GetCurrentWaypoint() const { return m_location; }
            /// Get the last tracked node position on the current taxi spline
            size_t GetCurrentWaypointIndex() const { return m_locationIndex; }
            /// Get the last known node we must resume from in the current taxi spline in the case of loading
            size_t GetResumeWaypointIndex() const { return m_resumeIndex; }

            /// Add a taxi flight path to the container in addition to the last one, returns true if input is correct
            bool AddRoute(TaxiPathEntry const* entry, float discountMulti = 0.0f, bool requireModel = true);
            /// Add a taxi flight path to the container by ID in addition to the last one, returns true if input is correct
            bool AddRoute(PathID pathID, float discountMulti = 0.0f, bool requireModel = true);
            /// Add a taxi flight path to the container by destinations in addition to the last one, returns true if input is correct
            bool AddRoute(DestID start, DestID end, float discountMulti = 0.0f, bool requireModel = true);
            /// Replace any existing container contents with a chain of flights defined by a vector of destinations, returns true if input is correct.
            bool AddRoutes(std::vector<DestID> const& destinations, float discountMulti = 0.0f, bool requireModel = true);
            /// Replace any existing container contents with a chain of flights defined by a deque of flight path ids, returns true if input is correct.
            bool AddRoutes(std::deque<PathID> const& paths, float discountMulti = 0.0f, bool requireModel = true);
            /// Finalize the container and prepare for the tracking of the flight. Used when all routes are added. It will be impossible to alter the container until the flight ends
            bool Prepare(Index nodeResume = 0);
            /// Clear the container by removing all contents. Can't be performed if taxi flight tracking is currently active, unless forced
            bool Clear(bool forced = false);

            /// Update tick: notify the container about arrival at the taxi spline node, advances internal node tracker and provides info about changes. Returns true when current route has changed
            bool UpdateRoute(const TaxiPathNodeEntry *entry, PathID& start, PathID& end);

            /// Try to join two routes together by determining junction points. Returns true on success.
            static bool Trim(Route& first, Route& second);

        private:
            Player& m_owner;
            TrackerState m_state;
            uint32 m_cost;
            uint32 m_displayId;
            Waypoint m_location;
            size_t m_locationIndex;
            size_t m_resumeIndex;
            Roadmap m_routes;
            Atlas m_atlas;

        public:
            bool m_debug;
    };
}

#endif
