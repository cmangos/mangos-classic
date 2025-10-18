#pragma once

#include "MovementActions.h"
#include "BattleGround/BattleGround.h"
#include "BattleGround/BattleGroundMgr.h"
#include "BattleGround/BattleGroundWS.h"
#include "BattleGround/BattleGroundAB.h"
#ifndef MANGOSBOT_ZERO
#include "BattleGround/BattleGroundEY.h"
#endif
#ifdef MANGOSBOT_TWO
#include "BattleGround/BattleGroundIC.h"
#include "BattleGround/BattleGroundSA.h"
#endif
#include "CheckMountStateAction.h"

using namespace ai;

#define SPELL_CAPTURE_BANNER 21651

typedef void(*BattleBotWaypointFunc) ();

// from vmangos
struct BattleBotWaypoint
{
    BattleBotWaypoint(float x_, float y_, float z_, BattleBotWaypointFunc func) :
        x(x_), y(y_), z(z_), pFunc(func) {};
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    BattleBotWaypointFunc pFunc = nullptr;
};

typedef std::vector<BattleBotWaypoint> BattleBotPath;

extern std::vector<BattleBotPath*> const vPaths_WS;
extern std::vector<BattleBotPath*> const vPaths_AB;
extern std::vector<BattleBotPath*> const vPaths_AV;
extern std::vector<BattleBotPath*> const vPaths_EY;
extern std::vector<BattleBotPath*> const vPaths_IC;

class BGTactics : public MovementAction
{
public:
    BGTactics(PlayerbotAI* ai, std::string name = "bg tactics") : MovementAction(ai, name) {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "bg tactics"; }
        virtual std::string GetHelpDescription()
        {
            return "This action handles the bot's tactical movement in battlegrounds.\n"
                   "It includes pathfinding for objectives, flag handling, and strategic positioning.\n"
                   "Supports WSG, AB, AV, EY, and IC battlegrounds with specialized waypoints.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    virtual bool Execute(Event& event);
private:
    bool SelectAvObjectiveAlliance(WorldLocation& objectiveLocation);
    bool SelectAvObjectiveHorde(WorldLocation& objectiveLocation);
    bool moveToStart(bool force = false);
    bool selectObjective(bool reset = false);
    bool moveToObjective();
    bool selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths);
    bool moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse = false);
    bool startNewPathBegin(std::vector<BattleBotPath*> const& vPaths);
    bool startNewPathFree(std::vector<BattleBotPath*> const& vPaths);
    bool resetObjective();
    bool wsgPaths();
    bool wsgRoofJump();
    bool eotsJump();
    bool atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds);
    bool CheckFlagAv();
    bool flagTaken();
    bool teamFlagTaken();
    bool protectFC();
    bool useBuff();
    uint32 getDefendersCount(Position point, float range, bool combat = true);
    bool IsLockedInsideKeep();
};

class ArenaTactics : public MovementAction
{
public:
    ArenaTactics(PlayerbotAI* ai, std::string name = "arena tactics") : MovementAction(ai, name) {}

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "arena tactics"; }
        virtual std::string GetHelpDescription()
        {
            return "This action handles the bot's movement strategy in arenas.\n"
                   "Focuses on controlling the center of the arena and maintaining optimal positioning.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {}; }
#endif 
    virtual bool Execute(Event& event);
private:
    bool moveToCenter(BattleGround *bg);
};
