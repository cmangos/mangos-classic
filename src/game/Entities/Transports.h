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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "Entities/GameObject.h"
#include "Movement/spline.h"

#include <map>
#include <set>

typedef Movement::Spline<double>                 TransportSpline;

struct KeyFrame
{
    explicit KeyFrame(TaxiPathNodeEntry const& _node) : Index(0), Node(&_node), InitialOrientation(0.0f),
        DistSinceStop(-1.0f), DistUntilStop(-1.0f), DistFromPrev(-1.0f), TimeFrom(0.0f), TimeTo(0.0f),
        Teleport(false), Update(false), ArriveTime(0), DepartureTime(0), Spline(nullptr), NextDistFromPrev(0.0f), NextArriveTime(0)
    {
    }

    uint32 Index;
    TaxiPathNodeEntry const* Node;
    float InitialOrientation;
    float DistSinceStop;
    float DistUntilStop;
    float DistFromPrev;
    float TimeFrom;
    float TimeTo;
    bool Teleport;
    bool Update;
    uint32 ArriveTime;
    uint32 DepartureTime;
    TransportSpline* Spline;

    // Data needed for next frame
    float NextDistFromPrev;
    uint32 NextArriveTime;

    bool IsTeleportFrame() const { return Teleport; }
    bool IsUpdateFrame() const { return Update; }
    bool IsStopFrame() const { return Node->actionFlag == 2; }
};

typedef std::vector<KeyFrame>  KeyFrameVec;

struct TransportTemplate
{
    TransportTemplate() : inInstance(false), pathTime(0), accelTime(0.0f), accelDist(0.0f), entry(0) { }
    ~TransportTemplate();

    std::set<uint32> mapsUsed;
    bool inInstance;
    uint32 pathTime;
    KeyFrameVec keyFrames;
    float accelTime;
    float accelDist;
    uint32 entry;
};

typedef std::set<WorldObject*> PassengerSet;

class GenericTransport : public GameObject
{
    public:
        bool AddPassenger(Unit* passenger);
        bool RemovePassenger(Unit* passenger);

        void UpdatePosition(float x, float y, float z, float o);
        void UpdatePassengerPosition(WorldObject* object);

        typedef std::set<Player*> PlayerSet;
        PassengerSet& GetPassengers() { return m_passengers; }

        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const
        {
            CalculatePassengerPosition(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const
        {
            CalculatePassengerOffset(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        static void CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO);
        static void CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO);

        virtual uint32 GetPathProgress() const = 0;
    protected:
        void UpdatePassengerPositions(PassengerSet& passengers);
    private:
        PassengerSet m_passengers;
};

class ElevatorTransport : public GenericTransport
{
    public:
        bool Create(uint32 guidlow, uint32 name_id, Map* map, float x, float y, float z, float ang,
            float rotation0 = 0.0f, float rotation1 = 0.0f, float rotation2 = 0.0f, float rotation3 = 0.0f, uint32 animprogress = GO_ANIMPROGRESS_DEFAULT, GOState go_state = GO_STATE_READY) override;
        void Update(const uint32 diff) override;

        uint32 GetPathProgress() const override;
    private:
        uint32 m_pathProgress;
        TransportAnimation const* m_animationInfo;
        uint32 m_currentSeg;
};

class Transport : public GenericTransport
{
    public:
        explicit Transport();

        bool Create(uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress);
        bool GenerateWaypoints(GameObjectInfo const* goinfo, std::set<uint32>& mapsUsed);
        void Update(const uint32 diff) override;

        uint32 GetPeriod() const { return m_period; }
        void SetPeriod(uint32 period) { m_period = period; }

        uint32 GetPathProgress() const override { return m_pathProgress; }

        KeyFrameVec& GetKeyFrames() { return m_keyFrames; }
    private:
        void TeleportTransport(uint32 newMapid, float x, float y, float z, float o);
        void UpdateForMap(Map const* targetMap);
        void MoveToNextWayPoint();                          // move m_next/m_cur to next points
        float CalculateSegmentPos(float perc);

        bool IsMoving() const { return m_isMoving; }
        void SetMoving(bool val) { m_isMoving = val; }

        ShortTimeTracker m_positionChangeTimer;
        bool m_isMoving;
        bool m_pendingStop;

        KeyFrameVec::const_iterator m_currentFrame;
        KeyFrameVec::const_iterator m_nextFrame;
        uint32 m_pathTime;
        uint32 m_pathProgress;

        uint32 m_period;

        KeyFrameVec m_keyFrames;
        TransportTemplate m_transportTemplate;
};
#endif
