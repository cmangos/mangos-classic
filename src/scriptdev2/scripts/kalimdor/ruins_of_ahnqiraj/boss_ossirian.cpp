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
SDName: Boss_Ossirian
SD%Complete: 100%
SDComment:
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "precompiled.h"
#include "ruins_of_ahnqiraj.h"

enum
{
    SAY_SUPREME_1           = -1509018,
    SAY_SUPREME_2           = -1509019,
    SAY_SUPREME_3           = -1509020,
    SAY_RAND_INTRO_1        = -1509021,
    SAY_RAND_INTRO_2        = -1509023,
    SAY_RAND_INTRO_3        = -1509024,
    SAY_AGGRO               = -1509025,
    SAY_SLAY                = -1509026,
    SAY_DEATH               = -1509027,

    SPELL_SILENCE           = 25195,
    SPELL_CYCLONE           = 25189,
    SPELL_STOMP             = 25188,
    SPELL_SUPREME           = 25176,
    SPELL_SUMMON_CRYSTAL    = 25192,
    SPELL_SAND_STORM        = 25160,                        // tornado spell
    SPELL_SUMMON            = 20477,                        // TODO NYI

    MAX_CRYSTAL_POSITIONS   = 1,                            // TODO

    SPELL_WEAKNESS_FIRE     = 25177,
    SPELL_WEAKNESS_FROST    = 25178,
    SPELL_WEAKNESS_NATURE   = 25180,
    SPELL_WEAKNESS_ARCANE   = 25181,
    SPELL_WEAKNESS_SHADOW   = 25183,

    NPC_SAND_VORTEX         = 15428,                        // tornado npc

    ZONE_ID_RUINS_AQ        = 3429,
};

static const float aSandVortexSpawnPos[2][4] =
{
    { -9523.482f, 1880.435f, 85.645f, 5.08f},
    { -9321.39f,  1822.968f, 84.266f, 3.16f},
};

static const float aCrystalSpawnPos[3] = { -9355.75f, 1905.43f, 85.55f};
static const uint32 aWeaknessSpell[] = {SPELL_WEAKNESS_FIRE, SPELL_WEAKNESS_FROST, SPELL_WEAKNESS_NATURE, SPELL_WEAKNESS_ARCANE, SPELL_WEAKNESS_SHADOW};

struct boss_ossirianAI : public ScriptedAI
{

    boss_ossirianAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_ruins_of_ahnqiraj*)m_creature->GetInstanceData();
        m_bSaidIntro = false;
        Reset();
    }

    instance_ruins_of_ahnqiraj* m_pInstance;

    uint32 m_uiSupremeTimer;
    uint32 m_uiCycloneTimer;
    uint32 m_uiStompTimer;
    uint32 m_uiSilenceTimer;
    uint8 m_uiCrystalPosition;

    bool m_bSaidIntro;

    void Reset() override
    {
        m_uiCrystalPosition = 0;
        m_uiCycloneTimer = 20000;
        m_uiStompTimer   = 30000;
        m_uiSilenceTimer = 30000;
        m_uiSupremeTimer = 45000;
    }

    void Aggro(Unit* /*pWho*/) override
    {
        DoCastSpellIfCan(m_creature, SPELL_SUPREME, CAST_TRIGGERED);
        DoScriptText(SAY_AGGRO, m_creature);
        DoSpawnNextCrystal();

        for (uint8 i = 0; i < countof(aSandVortexSpawnPos); ++i)
            m_creature->SummonCreature(NPC_SAND_VORTEX, aSandVortexSpawnPos[i][0], aSandVortexSpawnPos[i][1], aSandVortexSpawnPos[i][2], aSandVortexSpawnPos[i][3], TEMPSUMMON_CORPSE_DESPAWN, 0);

        if (m_pInstance)
            m_pInstance->instance->SetWeather(ZONE_ID_RUINS_AQ, WEATHER_TYPE_STORM, 1.0f, true);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void DoSpawnNextCrystal()
    {
        if (!m_pInstance)
            return;

        Creature* pOssirianTrigger = nullptr;
        if (m_uiCrystalPosition == 0)
        {
            // Respawn static spawned crystal trigger
            pOssirianTrigger = m_pInstance->GetSingleCreatureFromStorage(NPC_OSSIRIAN_TRIGGER);
            if (pOssirianTrigger && !pOssirianTrigger->isAlive())
                pOssirianTrigger->Respawn();
        }
        else
        {
            // Summon a new crystal trigger at some position depending on m_uiCrystalPosition
            // Note: the summon points seem to be very random; requires additional research
            float fX, fY, fZ;
            m_creature->GetRandomPoint(aCrystalSpawnPos[0], aCrystalSpawnPos[1], aCrystalSpawnPos[2], 100.0f, fX, fY, fZ);
            m_creature->SummonCreature(NPC_OSSIRIAN_TRIGGER, fX, fY, fZ, 0, TEMPSUMMON_CORPSE_DESPAWN, 0);
        }
        if (!pOssirianTrigger)
            return;

        // Respawn GO near crystal trigger
        if (GameObject* pCrystal = GetClosestGameObjectWithEntry(pOssirianTrigger, GO_OSSIRIAN_CRYSTAL, 10.0f))
            m_pInstance->DoRespawnGameObject(pCrystal->GetObjectGuid(), 5 * MINUTE);

        // Increase position
        ++m_uiCrystalPosition %= MAX_CRYSTAL_POSITIONS;
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_OSSIRIAN_TRIGGER)
            pSummoned->CastSpell(pSummoned, SPELL_SUMMON_CRYSTAL, true);
        else if (pSummoned->GetEntry() == NPC_SAND_VORTEX)
        {
            // The movement of this isn't very clear - may require additional research
            pSummoned->CastSpell(pSummoned, SPELL_SAND_STORM, true);
            pSummoned->GetMotionMaster()->MoveRandomAroundPoint(aCrystalSpawnPos[0], aCrystalSpawnPos[1], aCrystalSpawnPos[2], 100.0f);
        }
    }

    void SpellHit(Unit* pCaster, const SpellEntry* pSpell) override
    {
        if (pCaster->GetTypeId() == TYPEID_UNIT && pCaster->GetEntry() == NPC_OSSIRIAN_TRIGGER)
        {
            // Check for proper spell id
            bool bIsWeaknessSpell = false;
            for (uint8 i = 0; i < countof(aWeaknessSpell); ++i)
            {
                if (pSpell->Id == aWeaknessSpell[i])
                {
                    bIsWeaknessSpell = true;
                    break;
                }
            }
            if (!bIsWeaknessSpell)
                return;

            m_creature->RemoveAurasDueToSpell(SPELL_SUPREME);
            m_uiSupremeTimer = 45000;

            ((Creature*)pCaster)->ForcedDespawn();
            DoSpawnNextCrystal();
        }
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        // TODO: Range guesswork
        if (!m_bSaidIntro && pWho->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(pWho, 75.0f, false))
        {
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_RAND_INTRO_1, m_creature); break;
                case 1: DoScriptText(SAY_RAND_INTRO_2, m_creature); break;
                case 2: DoScriptText(SAY_RAND_INTRO_3, m_creature); break;
            }
            m_bSaidIntro = true;
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Supreme
        if (m_uiSupremeTimer <= uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SUPREME, CAST_AURA_NOT_PRESENT) == CAST_OK)
            {
                switch (urand(0, 2))
                {
                    case 0: DoScriptText(SAY_SUPREME_1, m_creature); break;
                    case 1: DoScriptText(SAY_SUPREME_2, m_creature); break;
                    case 2: DoScriptText(SAY_SUPREME_3, m_creature); break;
                }
                m_uiSupremeTimer = 45000;
            }
            else
                m_uiSupremeTimer = 5000;
        }
        else
            m_uiSupremeTimer -= uiDiff;

        // Stomp
        if (m_uiStompTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STOMP) == CAST_OK)
                m_uiStompTimer = 30000;
        }
        else
            m_uiStompTimer -= uiDiff;

        // Cyclone
        if (m_uiCycloneTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CYCLONE) == CAST_OK)
                m_uiCycloneTimer = 20000;
        }
        else
            m_uiCycloneTimer -= uiDiff;

        // Silence
        if (m_uiSilenceTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_SILENCE) == CAST_OK)
                m_uiSilenceTimer = urand(20000, 30000);
        }
        else
            m_uiSilenceTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_ossirian(Creature* pCreature)
{
    return new boss_ossirianAI(pCreature);
}

// This is actually a hack for a server-side spell
bool GOUse_go_ossirian_crystal(Player* /*pPlayer*/, GameObject* pGo)
{
    if (Creature* pOssirianTrigger = GetClosestCreatureWithEntry(pGo, NPC_OSSIRIAN_TRIGGER, 10.0f))
        pOssirianTrigger->CastSpell(pOssirianTrigger, aWeaknessSpell[urand(0, 4)], false);

    return true;
}

void AddSC_boss_ossirian()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_ossirian";
    pNewScript->GetAI = &GetAI_boss_ossirian;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_ossirian_crystal";
    pNewScript->pGOUse = &GOUse_go_ossirian_crystal;
    pNewScript->RegisterSelf();
}
