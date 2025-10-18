#pragma once
#include "MovementActions.h"

namespace ai
{
	class ReviveFromCorpseAction : public MovementAction 
    {
	public:
		ReviveFromCorpseAction(PlayerbotAI* ai) : MovementAction(ai, "revive from corpse") {}
        virtual bool Execute(Event& event);
    };

    class FindCorpseAction : public MovementAction 
    {
    public:
        FindCorpseAction(PlayerbotAI* ai) : MovementAction(ai, "find corpse") {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };

	class SpiritHealerAction : public MovementAction 
    {
	public:
	    SpiritHealerAction(PlayerbotAI* ai, std::string name = "spirit healer") : MovementAction(ai,name) {}
        virtual bool Execute(Event& event);
        virtual bool isUseful();
    };
}
