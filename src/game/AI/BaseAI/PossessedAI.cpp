#include "AI/BaseAI/PossessedAI.h"
#include "Entities/Creature.h"

PossessedAI::PossessedAI(Creature* creature) : UnitAI(static_cast<Unit*>(creature))
{
}

CreatureSpellList const& PossessedAI::GetSpellList() const
{
    static CreatureSpellList list; // will never be actually used
    return list;
}
