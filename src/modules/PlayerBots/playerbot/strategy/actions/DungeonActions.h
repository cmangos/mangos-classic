#pragma once
#include "MovementActions.h"
#include "playerbot/strategy/values/HazardsValue.h"

namespace ai
{
    class MoveAwayFromHazard : public MovementAction
    {
    public:
        MoveAwayFromHazard(PlayerbotAI* ai, std::string name = "move away from hazard") : MovementAction(ai, name) {}
        bool Execute(Event& event) override;
        bool isPossible() override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "move away from hazard"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot move away from hazardous areas in dungeons.\n"
                   "It identifies dangerous positions and navigates to a safer location.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"hazards"}; }
#endif 

    private:
        bool IsHazardNearby(const WorldPosition& point, const std::list<HazardPosition>& hazards) const;
    };

    class MoveAwayFromCreature : public MovementAction
    {
    public:
        MoveAwayFromCreature(PlayerbotAI* ai, std::string name, uint32 creatureID, float range) : MovementAction(ai, name), creatureID(creatureID), range(range) {}
        bool Execute(Event& event) override;
        bool isPossible() override;

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "move away from creature"; }
        virtual std::string GetHelpDescription()
        {
            return "This action makes the bot move away from a specific creature in dungeons.\n"
                   "It maintains a safe distance from the specified creature ID within a defined range.";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return {"hazards"}; }
#endif 

    private:
        bool IsValidPoint(const WorldPosition& point, const std::list<Creature*>& creatures, const std::list<HazardPosition>& hazards);
        bool HasCreaturesNearby(const WorldPosition& point, const std::list<Creature*>& creatures) const;
        bool IsHazardNearby(const WorldPosition& point, const std::list<HazardPosition>& hazards) const;

    private:
        uint32 creatureID;
        float range;
    };
}
