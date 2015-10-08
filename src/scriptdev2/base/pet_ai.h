/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_PET_H
#define SC_PET_H

// Using CreatureAI for now. Might change later and use PetAI (need to export for dll first)
class ScriptedPetAI : public CreatureAI
{
    public:
        explicit ScriptedPetAI(Creature* pCreature);
        ~ScriptedPetAI() {}

        void MoveInLineOfSight(Unit* /*pWho*/) override;

        void AttackStart(Unit* /*pWho*/) override;

        void AttackedBy(Unit* /*pAttacker*/) override;

        bool IsVisible(Unit* /*pWho*/) const override;

        void KilledUnit(Unit* /*pVictim*/) override {}

        void OwnerKilledUnit(Unit* /*pVictim*/) override {}

        void UpdateAI(const uint32 uiDiff) override;

        virtual void Reset() {}

        virtual void UpdatePetAI(const uint32 uiDiff);      // while in combat

        virtual void UpdatePetOOCAI(const uint32 /*uiDiff*/) {} // when not in combat

    protected:
        void ResetPetCombat();
};

#endif
