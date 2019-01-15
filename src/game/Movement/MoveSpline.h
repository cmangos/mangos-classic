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

#ifndef MANGOSSERVER_MOVEPLINE_H
#define MANGOSSERVER_MOVEPLINE_H

#include "spline.h"
#include "MoveSplineInitArgs.h"

namespace Movement
{
    extern float gravity;

    enum MonsterMoveType
    {
        MonsterMoveNormal = 0,
        MonsterMoveStop = 1,
        MonsterMoveFacingSpot = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle = 4
    };

    struct Location : public Vector3
    {
        Location() : orientation(0) {}
        Location(float x, float y, float z, float o) : Vector3(x, y, z), orientation(o) {}
        Location(const Vector3& v) : Vector3(v), orientation(0) {}
        Location(const Vector3& v, float o) : Vector3(v), orientation(o) {}

        float orientation;
    };

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
    class MoveSpline
    {
        public:
            typedef Spline<int32> MySpline;
            enum UpdateResult
            {
                Result_None         = 0x01,
                Result_Arrived      = 0x02,
                Result_NextCycle    = 0x04,
                Result_NextSegment  = 0x08,
            };
            friend class PacketBuilder;
        protected:
            MySpline        spline;

            FacingInfo      facing;

            uint32          m_Id;

            MoveSplineFlag  splineflags;

            uint32          time_passed;
            int32           point_Idx;
            int32           point_Idx_offset;

            void init_spline(const MoveSplineInitArgs& args);
        protected:

            const MySpline::ControlArray& getPath() const { return spline.getPoints();}
            void computeFallElevation(float& el) const;

            UpdateResult _updateState(uint32& ms_time_diff);
            uint32 next_timestamp() const { return spline.length(point_Idx + 1);}
            uint32 segment_time_elapsed() const { return next_timestamp() - time_passed;}
            uint32 timeElapsed() const { return Duration() - time_passed;}
            uint32 timePassed() const { return time_passed;}

        public:
            const MySpline& _Spline() const { return spline;}
            int32 _currentSplineIdx() const { return point_Idx;}
            void _Finalize();
            void _Interrupt() { splineflags.done = true;}

        public:

            void Initialize(const MoveSplineInitArgs&);
            bool Initialized() const { return !spline.empty();}

            explicit MoveSpline();

            template<class UpdateHandler>
            void updateState(uint32 difftime, UpdateHandler& handler)
            {
                MANGOS_ASSERT(Initialized());
                do
                    handler(_updateState(difftime));
                while (difftime > 0);
            }

            void updateState(uint32 difftime)
            {
                MANGOS_ASSERT(Initialized());
                do _updateState(difftime);
                while (difftime > 0);
            }

            Location ComputePosition() const;

            uint32 GetId() const { return m_Id;}
            bool Finalized() const { return splineflags.done; }
            bool isCyclic() const { return splineflags.cyclic;}
            const Vector3 FinalDestination() const { return Initialized() ? spline.getPoint(spline.last()) : Vector3();}
            const Vector3 CurrentDestination() const { return Initialized() ? spline.getPoint(point_Idx + 1) : Vector3();}
            int32 currentPathIdx() const;

            uint32 Duration() const { return spline.length();}

            std::string ToString() const;
    };
}
#endif // MANGOSSERVER_MOVEPLINE_H
