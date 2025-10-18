#pragma once
#include "playerbot/strategy/Value.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/actions/GenericActions.h"

namespace ai
{
    class Formation : public AiNamedObject
    {
    public:
        Formation(PlayerbotAI* ai, std::string name) : AiNamedObject (ai, name) {}

    public:
        virtual std::string GetTargetName() { return ""; }
        virtual WorldLocation GetLocation() { return NullLocation; }
        virtual float GetAngle();
        virtual float GetOffset();
        virtual float GetMaxDistance();
        static WorldLocation NullLocation;
		static bool IsNullLocation(WorldLocation const& loc);

    protected:
        float GetFollowAngle();
    };

    class FollowFormation : public Formation
    {
    public:
        FollowFormation(PlayerbotAI* ai, std::string name) : Formation(ai, name) {}
        virtual WorldLocation GetLocation();
    };

    class MoveFormation : public Formation
    {
    public:
        MoveFormation(PlayerbotAI* ai, std::string name) : Formation(ai, name) {}

    protected:
        WorldLocation MoveLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation, float range);
        WorldLocation MoveSingleLine(std::vector<Player*> line, float diff, float cx, float cy, float cz, float orientation, float range);
    };

    class MoveAheadFormation : public MoveFormation
    {
    public:
        MoveAheadFormation(PlayerbotAI* ai, std::string name) : MoveFormation(ai, name) {}
        virtual WorldLocation GetLocation();
        virtual WorldLocation GetLocationInternal() { return NullLocation; }
    };

    class FormationValue : public ManualSetValue<Formation*>
	{
	public:
        FormationValue(PlayerbotAI* ai);
        ~FormationValue() { if (value) { delete value; value = NULL; } }
        virtual void Reset();
        virtual std::string Save();
        virtual bool Load(std::string value);
    };

    class FormationPositionValue : public CalculatedValue<WorldPosition>
    {
    public:
        FormationPositionValue(PlayerbotAI* ai, std::string name = "formation position") : CalculatedValue(ai, name) {}

        virtual WorldPosition Calculate() {
            Formation* formation = AI_VALUE(Formation*, "formation");

            WorldLocation loc = formation->GetLocation();
            if (Formation::IsNullLocation(loc) || loc.mapid == -1)
                return WorldLocation();

            return loc;
        }
    };

    class SetFormationAction : public ChatCommandAction
    {
    public:
        SetFormationAction(PlayerbotAI* ai) : ChatCommandAction(ai, "set formation") {}
        virtual bool Execute(Event& event) override;
    };
};

