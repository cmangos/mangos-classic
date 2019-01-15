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
SDName: Boss_Razorgore
SD%Complete: 95
SDComment: Threat management after Mind Control is released needs core support (boss should have aggro on its previous controller and its previous victim should have threat transfered from boss to controlling player)
SDCategory: Blackwing Lair
EndScriptData

*/

#include "AI/ScriptDevAI/include/precompiled.h"
#include "blackwing_lair.h"

enum
{
    SAY_EGGS_BROKEN_1           = -1469022,
    SAY_EGGS_BROKEN_2           = -1469023,
    SAY_EGGS_BROKEN_3           = -1469024,

    SPELL_POSSESS_VISUAL        = 23014,                    // visual effect and increase the damage taken
    SPELL_DESTROY_EGG           = 19873,

    SPELL_CLEAVE                = 19632,
    SPELL_WARSTOMP              = 24375,
    SPELL_FIREBALL_VOLLEY       = 22425,
    SPELL_CONFLAGRATION         = 23023,
};

struct boss_razorgoreAI : public ScriptedAI
{
    boss_razorgoreAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackwing_lair*)pCreature->GetInstanceData();
        Reset();
    }

    instance_blackwing_lair* m_pInstance;

    uint32 m_uiCleaveTimer;
    uint32 m_uiWarStompTimer;
    uint32 m_uiFireballVolleyTimer;
    uint32 m_uiConflagrationTimer;

    bool m_bEggsExploded;

    void Reset() override
    {
        m_bEggsExploded         = false;

        m_uiCleaveTimer         = urand(4000, 8000);
        m_uiWarStompTimer       = 30000;
        m_uiConflagrationTimer  = urand(10000, 15000);
        m_uiFireballVolleyTimer = urand(15000, 20000);
    }

    void DamageTaken(Unit* /*pDoneBy*/, uint32& uiDamage, DamageEffectType /*damagetype*/) override
    {
        if (uiDamage < m_creature->GetHealth())
            return;

        if (!m_pInstance)
            return;

        // Don't allow any accident
        if (m_bEggsExploded)
        {
            uiDamage = 0;
            return;
        }

        // Boss explodes everything and resets - this happens if not all eggs are destroyed
        if (m_pInstance->GetData(TYPE_RAZORGORE) == IN_PROGRESS)
        {
            uiDamage = 0;
            m_bEggsExploded = true;
            m_pInstance->SetData(TYPE_RAZORGORE, FAIL);
            DoScriptText(SAY_RAZORGORE_DEATH, m_creature);
            m_creature->CastSpell(m_creature, SPELL_FIREBALL, TRIGGERED_OLD_TRIGGERED);
            m_creature->ForcedDespawn(5000);
        }
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_RAZORGORE, FAIL);
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Cleave
        if (m_uiCleaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CLEAVE) == CAST_OK)
                m_uiCleaveTimer = urand(4000, 8000);
        }
        else
            m_uiCleaveTimer -= uiDiff;

        // War Stomp
        if (m_uiWarStompTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_WARSTOMP) == CAST_OK)
                m_uiWarStompTimer = 30000;
        }
        else
            m_uiWarStompTimer -= uiDiff;

        // Fireball Volley
        if (m_uiFireballVolleyTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_FIREBALL_VOLLEY) == CAST_OK)
                m_uiFireballVolleyTimer = urand(15000, 20000);
        }
        else
            m_uiFireballVolleyTimer -= uiDiff;

        // Conflagration
        if (m_uiConflagrationTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_CONFLAGRATION) == CAST_OK)
                m_uiConflagrationTimer = urand(15000, 25000);
        }
        else
            m_uiConflagrationTimer -= uiDiff;

        // Aura Check. If Razorgore's target is affected by conflagration, attack next one in aggro list not affected by the spell
        if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, SPELL_CONFLAGRATION, SELECT_FLAG_PLAYER | SELECT_FLAG_NOT_AURA))
        {
            // Target is not current victim, force select and attack it
            if (pTarget != m_creature->getVictim())
            {
                AttackStart(pTarget);
                m_creature->SetInFront(pTarget);
            }
            // Make sure our attack is ready
            if (m_creature->isAttackReady())
            {
                m_creature->AttackerStateUpdate(pTarget);
                m_creature->resetAttackTimer();
            }
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_razorgore(Creature* pCreature)
{
    return new boss_razorgoreAI(pCreature);
}

struct npc_blackwing_orbAI : public ScriptedAI
{
    npc_blackwing_orbAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_blackwing_lair*)pCreature->GetInstanceData();
        Reset();
    }

    instance_blackwing_lair* m_pInstance;

    uint32 m_uiIntroVisualTimer;

    void Reset() override
    {
        m_uiIntroVisualTimer = 4000;
    }

    void SpellHit(Unit* /* pCaster */, const SpellEntry* pSpell) override
    {
        // If hit by Razorgore's fireball: explodes everything in the room
        if (pSpell->Id == SPELL_FIREBALL)
        {
            if (Creature* pTemp = m_creature->SummonCreature(NPC_ORB_DOMINATION, m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ(), 0, TEMPSPAWN_TIMED_DESPAWN, 5 * IN_MILLISECONDS))
                DoScriptText(EMOTE_ORB_SHUT_OFF, pTemp);
            m_creature->CastSpell(m_creature, SPELL_EXPLODE_ORB, TRIGGERED_IGNORE_UNATTACKABLE_FLAG);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Set visual only on OOC timer
        if (m_uiIntroVisualTimer)
        {
            if (m_uiIntroVisualTimer <= uiDiff)
            {
                if (!m_pInstance)
                {
                    script_error_log("Instance Blackwing Lair: ERROR Failed to load instance data for this instace.");
                    return;
                }

                // If Razorgore is not respawned yet: wait
                if (Creature* pRazorgore = m_pInstance->GetSingleCreatureFromStorage(NPC_RAZORGORE))
                {
                    if (!(pRazorgore->isAlive()))
                    {
                        m_uiIntroVisualTimer = 2000;
                        return;
                    }
                }

                // If Grethok the Controller is here and spawned, start the visual, else wait for him
                if (Creature* pGrethok = GetClosestCreatureWithEntry(m_creature, NPC_GRETHOK_CONTROLLER, 2.0f))
                {
                    if (pGrethok->isAlive())
                    {
                        m_creature->CastSpell(m_creature, SPELL_POSSESS_VISUAL, TRIGGERED_OLD_TRIGGERED);
                        pGrethok->CastSpell(pGrethok, SPELL_CONTROL_ORB, TRIGGERED_OLD_TRIGGERED);
                        m_uiIntroVisualTimer = 0;
                    }
                }
                else
                    m_uiIntroVisualTimer = 2000;
            }
            else
                m_uiIntroVisualTimer -= uiDiff;
        }
    }
};

UnitAI* GetAI_npc_blackwing_orb(Creature* pCreature)
{
    return new npc_blackwing_orbAI(pCreature);
}

bool EffectDummyGameObj_go_black_dragon_egg(Unit* pCaster, uint32 uiSpellId, SpellEffectIndex uiEffIndex, GameObject* pGOTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId == SPELL_DESTROY_EGG && uiEffIndex == EFFECT_INDEX_1)
    {
        if (!pGOTarget->IsSpawned())
            return true;

        if (ScriptedInstance* pInstance = (ScriptedInstance*)pGOTarget->GetInstanceData())
        {
            if (urand(0, 1))
            {
                switch (urand(0, 2))
                {
                    case 0: DoScriptText(SAY_EGGS_BROKEN_1, pCaster); break;
                    case 1: DoScriptText(SAY_EGGS_BROKEN_2, pCaster); break;
                    case 2: DoScriptText(SAY_EGGS_BROKEN_3, pCaster); break;
                }
            }

            // Store the eggs which are destroyed, in order to count them for the second phase
            pInstance->SetData64(DATA_DRAGON_EGG, pGOTarget->GetObjectGuid());
        }

        return true;
    }

    return false;
}

void AddSC_boss_razorgore()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_razorgore";
    pNewScript->GetAI = &GetAI_boss_razorgore;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_blackwing_orb";
    pNewScript->GetAI = &GetAI_npc_blackwing_orb;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_black_dragon_egg";
    pNewScript->pEffectDummyGO = &EffectDummyGameObj_go_black_dragon_egg;
    pNewScript->RegisterSelf();
}
