#pragma once

#include "playerbot/strategy/Action.h"
#include "playerbot/strategy/NamedObjectContext.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/TravelNode.h"
#include "MotionGenerators/WaypointMovementGenerator.h"
#include "playerbot/strategy/values/HazardsValue.h"

namespace ai
{
    class MovementAction : public Action
    {
    public:
        MovementAction(PlayerbotAI* ai, std::string name) : Action(ai, name) {}

    protected:
        bool ChaseTo(WorldObject *obj, float distance = 0.0f, float angle = 0.0f);
        bool MoveNear(uint32 mapId, float x, float y, float z, float distance = sPlayerbotAIConfig.contactDistance);
        bool FlyDirect(WorldPosition &startPosition,  WorldPosition &endPosition , WorldPosition& movePosition, TravelPath movePath, bool idle);

        inline bool MoveTo(const WorldLocation& location, bool idle = false, bool react = false, bool noPath = false, bool ignoreEnemyTargets = false)
        {
           return MoveTo(location.mapid, location.coord_x, location.coord_y, location.coord_z, idle, react, noPath, ignoreEnemyTargets);
        }

        bool MoveTo(uint32 mapId, float x, float y, float z, bool idle = false, bool react = false, bool noPath = false, bool ignoreEnemyTargets = false);
        bool MoveTo(Unit* target, float distance = 0.0f);
        bool MoveNear(WorldObject* target, float distance = sPlayerbotAIConfig.contactDistance);
        float GetFollowAngle();
        bool Follow(Unit* target, float distance = 0);
        bool Follow(Unit* target, float distance, float angle);
        float MoveDelay(float distance);
        bool FollowOnTransport(Unit* target);

        void WaitForReach(float distance);
        void WaitForReach(const Movement::PointsArray& path);

        bool IsMovingAllowed(Unit* target);
        bool IsMovingAllowed(uint32 mapId, float x, float y, float z);
        bool IsMovingAllowed();
        bool Flee(Unit *target);
        void ClearIdleState();
        void UpdateMovementState();

        bool isPossible() override;
        bool isUseful() override;

        void CreateWp(Player* wpOwner, float x, float y, float z, float o, uint32 entry, bool important = false);
        float GetAngle(const float x1, const float y1, const float x2, const float y2);

        // Used when this action is executed as a reaction
        bool ShouldReactionInterruptCast() const override { return true; }
        bool ShouldReactionInterruptMovement() const override { return true; }

    private:
        bool IsValidPosition(const WorldPosition& position, const WorldPosition& visibleFromPosition);
        bool IsHazardNearPosition(const WorldPosition& position, HazardPosition* outHazard = nullptr);
        bool GeneratePathAvoidingHazards(const WorldPosition& endPosition, bool generatePath, Movement::PointsArray& outPath);
    };

    class FleeAction : public MovementAction
    {
    public:
        FleeAction(PlayerbotAI* ai, float distance = sPlayerbotAIConfig.spellDistance) : MovementAction(ai, "flee"), distance(distance) {}
        virtual bool Execute(Event& event);

    private:
        float distance;
    };

    class FleeWithPetAction : public MovementAction
    {
    public:
        FleeWithPetAction(PlayerbotAI* ai) : MovementAction(ai, "flee with pet") {}
        virtual bool Execute(Event& event);
    };

    class RunAwayAction : public MovementAction
    {
    public:
        RunAwayAction(PlayerbotAI* ai) : MovementAction(ai, "runaway") {}
        virtual bool Execute(Event& event);
    };

    class MoveToLootAction : public MovementAction
    {
    public:
        MoveToLootAction(PlayerbotAI* ai) : MovementAction(ai, "move to loot") {}
        virtual bool Execute(Event& event);
    };

    class MoveOutOfEnemyContactAction : public MovementAction
    {
    public:
        MoveOutOfEnemyContactAction(PlayerbotAI* ai) : MovementAction(ai, "move out of enemy contact") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class SetFacingTargetAction : public Action
    {
    public:
        SetFacingTargetAction(PlayerbotAI* ai) : Action(ai, "set facing") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual bool isPossible();
    };

    class SetBehindTargetAction : public MovementAction
    {
    public:
        SetBehindTargetAction(PlayerbotAI* ai) : MovementAction(ai, "set behind") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
        virtual bool isPossible();
    };

    class MoveOutOfCollisionAction : public MovementAction
    {
    public:
        MoveOutOfCollisionAction(PlayerbotAI* ai) : MovementAction(ai, "move out of collision") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class MoveRandomAction : public MovementAction
    {
    public:
        MoveRandomAction(PlayerbotAI* ai) : MovementAction(ai, "move random") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

    class MoveToAction : public MovementAction, public Qualified
    {
    public:
        MoveToAction(PlayerbotAI* ai, std::string name = "move to") : MovementAction(ai, "name"), Qualified() {}
        virtual bool Execute(Event& event);
    };

    class JumpAction : public MovementAction, public Qualified
    {
    public:
        JumpAction(PlayerbotAI* ai) : MovementAction(ai, "jump"), Qualified() {}
        bool Execute(Event& event) override;
        bool isUseful() override;

        static WorldPosition CalculateJumpParameters(const WorldPosition& src, Unit* jumper, float angle, float vSpeed, float hSpeed, float &timeToLand, float &distanceToLand, float &maxHeight, bool &goodLanding, std::vector<WorldPosition> &path, float maxJumpHeight = sPlayerbotAIConfig.jumpHeightLimit);

    private:
        bool DoJump(const WorldPosition& dest, const WorldPosition& highestPoint, float angle, float vSpeed, float hSpeed, float timeToLand, float distanceToLand, float maxHeight, bool goodLanding, bool jumpInPlace, bool jumpBackward, bool showOnly);
        bool JumpTowards(const WorldPosition& src, const WorldPosition& dest, Unit* jumper, float jumpSpeed, bool preSetLanding = false);
        static float CalculateJumpTime(float srcZ, float destZ, float vSpeed, float hSpeed, float distance);
        static float CalculateJumpTime(float z_diff, float vSpeed, bool ascending);

        WorldPosition GetPossibleJumpStartFor(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possiblelanding, Unit* jumper, float &requiredSpeed, float distanceTo, float distanceFrom);
        WorldPosition GetPossibleJumpStartForInRange(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possiblelanding, Unit* jumper, float& requiredSpeed, float distanceTo, float distanceFrom);
        bool CanJumpTo(const WorldPosition& src, const WorldPosition& dest, WorldPosition& possiblelanding, float& jumpAngle, Unit* jumper, float jumpSpeed, float maxDistance = 10.0f);
        bool CanWalkTo(const WorldPosition& src, const WorldPosition& dest, Unit* jumper, float maxDistance = sPlayerbotAIConfig.sightDistance);
        bool IsJumpFasterThanWalking(const WorldPosition& src, const WorldPosition& dest, const WorldPosition& jumpLanding, Unit* jumper, float maxDistance = sPlayerbotAIConfig.sightDistance);

        static bool IsJumpSafe(const WorldPosition& src, const WorldPosition& dest, Unit* jumper);
        static bool CanLand(const WorldPosition& dest, Unit* jumper);
        static bool IsNotMagmaSlime(const WorldPosition& dest, Unit* jumper);
    };
}
