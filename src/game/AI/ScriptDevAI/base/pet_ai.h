/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PET_H
#define SC_PET_H

// Using UnitAI for now. Might change later and use PetAI (need to export for dll first)
class ScriptedPetAI : public CreatureAI
{
    public:
        explicit ScriptedPetAI(Creature* creature);
        ~ScriptedPetAI() {}

        void AttackStart(Unit* /*who*/) override;

        void AttackedBy(Unit* /*attacker*/) override;

        void KilledUnit(Unit* /*victim*/) override {}

        void OwnerKilledUnit(Unit* /*victim*/) override {}

        void UpdateAI(const uint32 diff) override;

        void CombatStop() override;

        virtual void Reset() {}

        virtual void UpdatePetAI(const uint32 diff);      // while in combat

        virtual void UpdatePetOOCAI(const uint32 /*diff*/) {} // when not in combat

    protected:
        void ResetPetCombat();
};

#endif
