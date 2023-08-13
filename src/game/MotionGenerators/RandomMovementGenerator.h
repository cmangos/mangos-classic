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

#ifndef MANGOS_RANDOMMOTIONGENERATOR_H
#define MANGOS_RANDOMMOTIONGENERATOR_H

#include "MovementGenerator.h"
#include "Entities/ObjectGuid.h"

class PathFinder;

class AbstractRandomMovementGenerator : public MovementGenerator
{
    public:
        explicit AbstractRandomMovementGenerator(uint32 stateActive, uint32 stateMotion, uint32 delayMin, uint32 delayMax, uint32 movesMax = 1) :
            i_x(0.0f), i_y(0.0f), i_z(0.0f), i_radius(0.0f), i_verticalZ(0.0f), i_pathLength(0.0f), i_walk(true),
            i_nextMoveTimer(0), i_nextMoveCount(1), i_nextMoveCountMax(movesMax),
            i_nextMoveDelayMin(delayMin), i_nextMoveDelayMax(delayMax),
            i_stateActive(stateActive), i_stateMotion(stateMotion)
        {
        }

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;
        void Reset(Unit& owner) override;
        bool Update(Unit& owner, const uint32& diff) override;

    protected:
        virtual int32 _setLocation(Unit& owner);

        float i_x, i_y, i_z;
        float i_radius;
        float i_verticalZ;
        float i_pathLength;
        bool i_walk;

        std::unique_ptr<PathFinder> m_pathFinder;
        ShortTimeTracker i_nextMoveTimer;
        uint32 i_nextMoveCount, i_nextMoveCountMax;
        uint32 i_nextMoveDelayMin, i_nextMoveDelayMax;
        uint32 i_stateActive, i_stateMotion;
};

class ConfusedMovementGenerator : public AbstractRandomMovementGenerator
{
    public:
        explicit ConfusedMovementGenerator(Unit const& owner);
        ConfusedMovementGenerator(float x, float y, float z);

        MovementGeneratorType GetMovementGeneratorType() const override { return CONFUSED_MOTION_TYPE; }
};

class WanderMovementGenerator : public AbstractRandomMovementGenerator
{
    public:
        explicit WanderMovementGenerator(Creature const& npc);
        WanderMovementGenerator(float x, float y, float z, float radius, float verticalZ = 0.0f);

        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;

        void AddToRandomPauseTime(int32 waitTimeDiff, bool force);

        MovementGeneratorType GetMovementGeneratorType() const override { return RANDOM_MOTION_TYPE; }
};

class TimedWanderMovementGenerator : public WanderMovementGenerator
{
    public:
        explicit TimedWanderMovementGenerator(Creature const& npc, uint32 timer, float radius, float verticalZ = 0.0f);
        TimedWanderMovementGenerator(uint32 timer, float x, float y, float z, float radius, float verticalZ = 0.0f)
            : WanderMovementGenerator(x, y, z, radius, verticalZ), m_durationTimer(timer)
        {

        }

        bool Update(Unit& owner, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return TIMED_RANDOM_MOTION_TYPE; }

    private:
        ShortTimeTracker m_durationTimer;
};

class FleeingMovementGenerator : public AbstractRandomMovementGenerator
{
    public:
        explicit FleeingMovementGenerator(Unit const& source);

        virtual int32 _setLocation(Unit& owner) override;
        MovementGeneratorType GetMovementGeneratorType() const override { return FLEEING_MOTION_TYPE; }
};

class PanicMovementGenerator : public FleeingMovementGenerator
{
    public:
        PanicMovementGenerator(Unit const& source, uint32 time) : FleeingMovementGenerator(source), m_fleeingTimer(time) {}

        void Initialize(Unit& owner) override;
        void Finalize(Unit& owner) override;
        void Interrupt(Unit& owner) override;
        bool Update(Unit& owner, const uint32& diff) override;

        MovementGeneratorType GetMovementGeneratorType() const override { return TIMED_FLEEING_MOTION_TYPE; }

    private:
        ShortTimeTracker m_fleeingTimer;
};

#endif
