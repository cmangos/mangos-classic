#pragma once
#include "GenericActions.h"

namespace ai
{
	class TrainerAction : public ChatCommandAction
    {
	public:
		TrainerAction(PlayerbotAI* ai) : ChatCommandAction(ai, "trainer") {}
        virtual bool Execute(Event& event) override;

    private:
        typedef void (TrainerAction::*TrainerSpellAction)(uint32, ObjectGuid trainerGuid, uint32 spellId, TrainerSpell const*, std::ostringstream& msg);
        void Iterate(Player* requester, Creature* creature, TrainerSpellAction action, SpellIds& spells);
        void Learn(uint32 cost, ObjectGuid trainerGuid, uint32 spellId, TrainerSpell const* tSpell, std::ostringstream& msg);
        void TellHeader(Player* requester, Creature* creature);
        void TellFooter(Player* requester, uint32 totalCost);
    };
}