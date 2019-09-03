/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Boss_Skeram
SD%Complete: 100
SDComment:
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "temple_of_ahnqiraj.h"

enum
{
    SAY_AGGRO                   = -1531000,
    SAY_SLAY                    = -1531001,
    SAY_DEATH                   = -1531002,

    SPELL_ARCANE_EXPLOSION      = 26192,
    SPELL_EARTH_SHOCK           = 26194,
    SPELL_TRUE_FULFILLMENT      = 785,
    SPELL_TELEPORT_1            = 4801,
    SPELL_TELEPORT_2            = 8195,
    SPELL_TELEPORT_3            = 20449,
    SPELL_INITIALIZE_IMAGES     = 794,
//    SPELL_INITIALIZE_IMAGE      = 3730,   // Triggers by spell 794 on each summoned image
    SPELL_SUMMON_IMAGES         = 747,
    SPELL_TELEPORT_IMAGE        = 26591,
};

struct boss_skeramAI : public ScriptedAI
{
    boss_skeramAI(Creature* creature) : ScriptedAI(creature)
    {
        m_instance = (ScriptedInstance*)creature->GetInstanceData();
        Reset();

        // Check this for images, because the Initialize spell hits the target only after aggro
        if (m_creature->IsTemporarySummon())
            m_isImage = true;
        else
            m_isImage = false;
    }

    ScriptedInstance* m_instance;

    uint32 m_arcaneExplosionTimer;
    uint32 m_fullFillmentTimer;
    uint32 m_blinkTimer;
    uint32 m_earthShockTimer;

    std::vector<uint32> m_teleports;
    uint8 m_teleportCounter;

    uint8 m_maxMeleeAllowed;

    float m_hpCheck;

    bool m_isImage;

    void Reset() override
    {
        m_arcaneExplosionTimer = urand(6, 12) * IN_MILLISECONDS;
        m_fullFillmentTimer    = 15 * IN_MILLISECONDS;
        m_blinkTimer           = urand(30, 45) * IN_MILLISECONDS;
        m_earthShockTimer      = 1200;

        m_hpCheck               = 75.0f;

        m_teleports              = { SPELL_TELEPORT_1, SPELL_TELEPORT_2, SPELL_TELEPORT_3 };
        m_teleportCounter        = 0;

        m_maxMeleeAllowed        = 0;

        if (m_creature->GetVisibility() != VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_ON);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (!m_isImage)
        {
            DoScriptText(SAY_DEATH, m_creature);

            if (m_instance)
                m_instance->SetData(TYPE_SKERAM, DONE);
        }
        // Else despawn to avoid looting
        else
            m_creature->ForcedDespawn(1);
    }

    void Aggro(Unit* /*who*/) override
    {
        // Prophet Skeram will only cast Arcane Explosion if a given number of players are in melee range
        // Initial value was 4+ but it was changed in patch 1.12 to be less dependant on raid
        // We assume value is number of players / 10 (raid of 40 people in Classic -> value of 4)
        m_maxMeleeAllowed = m_instance->instance->GetPlayers().getSize() / 10 + 1;

        if (m_isImage)
            return;

        DoScriptText(SAY_AGGRO, m_creature);

        if (m_instance)
            m_instance->SetData(TYPE_SKERAM, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_SKERAM, FAIL);

        if (m_isImage)
            m_creature->ForcedDespawn();
    }

    void JustSummoned(Creature* summoned) override
    {
        if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            summoned->AI()->AttackStart(target);
    }

    // Wrapper to handle the image teleport
    void DoTeleport(bool forced=false)
    {
        if (!m_instance)
            return;

        uint32 teleportSpellID = 0;
        if (forced)
        {
            if (m_isImage)
            {
                // Get an available teleport location from original boss
                if (Creature* prophet = m_instance->GetSingleCreatureFromStorage(NPC_SKERAM))
                {
                    if (boss_skeramAI* skeramAI = dynamic_cast<boss_skeramAI*>(prophet->AI()))
                        teleportSpellID = skeramAI->GetAvailableTeleport();
                }
            }
            else
                teleportSpellID = m_teleports[0];
        }
        else // Else, randomly blink to one of the three destinations
        {
            switch (urand(0, 2))
            {
                case 0: teleportSpellID = SPELL_TELEPORT_1; break;
                case 1: teleportSpellID = SPELL_TELEPORT_2; break;
                case 2: teleportSpellID = SPELL_TELEPORT_3; break;
            }
        }

        // Teleport, reset thread (and restore visibility if needed)
        DoCastSpellIfCan(m_creature, teleportSpellID);
        DoResetThreat();
        if (m_creature->GetVisibility() != VISIBILITY_ON)
            m_creature->SetVisibility(VISIBILITY_ON);
    }

    uint32 GetAvailableTeleport()
    {
        // Only original boss can give teleport location
        if (m_isImage)
            return 0;

        if (m_teleportCounter < m_teleports.size())
            return m_teleports[m_teleportCounter++];

        return 0;
    }

    void UpdateAI(const uint32 diff) override
    {
        // Return since we have no target
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Arcane Explosion is done if more than a set number of people are in melee range
        if (m_arcaneExplosionTimer < diff)
        {
            PlayerList meleePlayerList;
            float meleeRange = m_creature->GetCombinedCombatReach(m_creature->getVictim(), true);
            GetPlayerListWithEntryInWorld(meleePlayerList, m_creature, meleeRange);
            if (meleePlayerList.size() >= m_maxMeleeAllowed)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ARCANE_EXPLOSION) == CAST_OK)
                   m_arcaneExplosionTimer = urand(8, 18) * IN_MILLISECONDS;
            }
            else    // Recheck in 1 second
                m_arcaneExplosionTimer = 1 * IN_MILLISECONDS;
        }
        else
            m_arcaneExplosionTimer -= diff;

        // True Fullfilment
        if (m_fullFillmentTimer < diff)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(target, SPELL_TRUE_FULFILLMENT) == CAST_OK)
                    m_fullFillmentTimer = urand(20, 30) * IN_MILLISECONDS;
            }
        }
        else
            m_fullFillmentTimer -= diff;

        // Blink_Timer
        if (m_blinkTimer < diff)
        {
            DoTeleport();
            m_blinkTimer = urand(10, 30) * IN_MILLISECONDS;
        }
        else
            m_blinkTimer -= diff;

        // Earth Shock is cast every 1.2s on the victim if Skeram can't reach them or they are not auto attacking him
        if (m_earthShockTimer < diff)
        {
            if (!m_creature->CanReachWithMeleeAttack(m_creature->getVictim()) || !m_creature->getVictim()->hasUnitState(UNIT_STAT_MELEE_ATTACKING))
            {
                if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_EARTH_SHOCK) == CAST_OK)
                    m_earthShockTimer = 1200;
            }
        }
        else
            m_earthShockTimer -= diff;

        // Summon images at 75%, 50% and 25%
        if (!m_isImage && m_creature->GetHealthPercent() < m_hpCheck)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUMMON_IMAGES) == CAST_OK)
            {
                m_hpCheck -= 25.0f;
                std::random_shuffle(m_teleports.begin(), m_teleports.end());    // Shuffle the teleport spells to ensure that boss and images have a different location assigned randomly
                m_teleportCounter = 1;
                // Teleport shortly after the images are summoned and set invisible to clear the selection (Workaround alert!!!)
                DoCastSpellIfCan(m_creature, SPELL_INITIALIZE_IMAGES, CAST_TRIGGERED);
                m_creature->SetVisibility(VISIBILITY_OFF);
                m_blinkTimer = urand(30, 40) * IN_MILLISECONDS;                 // Set same blink timer than the summoned images
                DoTeleport(true);                                               // Force teleport (the images will do the same)
            }
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_skeram(Creature* creature)
{
    return new boss_skeramAI(creature);
}

bool EffectDummyCreature_prophet_skeram(Unit* /*caster*/, uint32 spellId, SpellEffectIndex effIndex, Creature* creatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    // always check spellid and effectindex
    if (spellId == SPELL_TELEPORT_IMAGE && effIndex == EFFECT_INDEX_0)
    {
        // check for target == caster first
        if (instance_temple_of_ahnqiraj* instance = (instance_temple_of_ahnqiraj*)creatureTarget->GetInstanceData())
        {
            if (Creature* prophet = instance->GetSingleCreatureFromStorage(NPC_SKERAM))
            {
                if (prophet == creatureTarget)
                    return false;
            }
        }

        if (boss_skeramAI* skeramAI = dynamic_cast<boss_skeramAI*>(creatureTarget->AI()))
            skeramAI->DoTeleport(true);
    }

    return false;
}

void AddSC_boss_skeram()
{
    Script* newScript = new Script;
    newScript->Name = "boss_skeram";
    newScript->GetAI = &GetAI_boss_skeram;
    newScript->pEffectDummyNPC = &EffectDummyCreature_prophet_skeram;
    newScript->RegisterSelf();
}
