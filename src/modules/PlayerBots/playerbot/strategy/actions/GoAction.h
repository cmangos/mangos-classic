#pragma once
#include "MovementActions.h"

namespace ai
{
    class TravelDestination;

	class GoAction : public MovementAction, Qualified
	{
	public:
		GoAction(PlayerbotAI* ai) : MovementAction(ai, "go"), Qualified() {}
		virtual bool Execute(Event& event) override;
        virtual bool isPossible() override { return true; }
        virtual bool isUseful() override { return true; }

#ifdef GenerateBotHelp
        virtual std::string GetHelpName() { return "go"; } //Must equal iternal name
        virtual std::string GetHelpDescription()
        {
            return "This will make the bot move to a specific location.\n"
                "Examples:\n"
                "go [game object] : Move to the specified game object.\n"
                "go <player or npc name> : Move to specified player or npc.\n"
                "go x;y : Move to gps location.\n"
                "go where : Ask the bot where it wants to go.\n"
                "go to <npc, zone or creature name> : Travel up to specified object\n or while following explain how to get there.\n"
                "go travel <npc, zone or creature name> : Set the bots current travel destination\n to specified object and travel (up to) that location.\n";
        }
        virtual std::vector<std::string> GetUsedActions() { return {}; }
        virtual std::vector<std::string> GetUsedValues() { return { "travel target" ,  "nearest npcs" ,  "nearest friendly players" , "position" }; }
#endif 

    private:
        bool TellWhereToGo(std::string& param, Player* requester) const;
        bool LeaderAlreadyTraveling(TravelDestination* dest) const;
        bool TellHowToGo(TravelDestination* dest, Player* requester) const;
        bool TravelTo(TravelDestination* dest, Player* requester) const;
        bool MoveToGo(std::string& param, Player* requester);
        bool MoveToUnit(std::string& param, Player* requester);
        bool MoveToGps(std::string& param, Player* requester);
        bool MoveToMapGps(std::string& param, Player* requester);
        bool MoveToPosition(std::string& param, Player* requester);

        void UpdateStrategyPosition(const WorldPosition& position);
	};
}
