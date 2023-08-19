/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_GUARDAI_H
#define SC_GUARDAI_H

#include "AI/ScriptDevAI/include/sc_creature.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum GuardAI
{
    GENERIC_CREATURE_COOLDOWN       = 5000,

    SAY_GUARD_SIL_AGGRO1            = -1000198,
    SAY_GUARD_SIL_AGGRO2            = -1000199,
    SAY_GUARD_SIL_AGGRO3            = -1000200,

    NPC_CENARION_INFANTRY           = 15184
};

struct guardAI : public CombatAI
{
    public:
        explicit guardAI(Creature* creature);
        ~guardAI() {}

        uint32 m_globalCooldown;                          // This variable acts like the global cooldown that players have (1.5 seconds)
        uint32 m_buffTimer;                               // This variable keeps track of buffs

        void Reset() override;

        void Aggro(Unit* who) override;

        void JustDied(Unit* /*killer*/) override;
        void JustRespawned() override;

        // Commonly used for guards in main cities
        void DoReplyToTextEmote(uint32 textEmote);
};

struct guardAI_orgrimmar : public guardAI
{
    guardAI_orgrimmar(Creature* creature) : guardAI(creature) {}

    void ReceiveEmote(Player* player, uint32 textEmote) override;
};

#endif
