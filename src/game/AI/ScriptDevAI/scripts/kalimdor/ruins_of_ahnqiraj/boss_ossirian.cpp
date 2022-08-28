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
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "ruins_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Globals/ObjectMgr.h"

enum
{
    SAY_SUPREME_1           = -1509018,
    SAY_SUPREME_2           = -1509019,
    SAY_SUPREME_3           = -1509020,
    SAY_RAND_INTRO_1        = -1509021,
    SAY_RAND_INTRO_2        = -1509023,
    SAY_RAND_INTRO_3        = -1509024,
    SAY_AGGRO               = -1509025,
    SAY_SLAY                = 11450,
    SAY_DEATH               = -1509027,

    SPELL_CURSE_OF_TONGUES  = 25195,
    SPELL_CYCLONE           = 25189,
    SPELL_STOMP             = 25188,
    SPELL_SUPREME           = 25176,
    SPELL_SUMMON_CRYSTAL    = 25192,
    SPELL_SAND_STORM        = 25160,                        // tornado spell
    SPELL_SUMMON            = 20477,                        // TODO NYI
    SPELL_DOUBLE_ATTACK     = 19818,

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
static const std::vector<uint32> aWeaknessSpell {SPELL_WEAKNESS_FIRE, SPELL_WEAKNESS_FROST, SPELL_WEAKNESS_NATURE, SPELL_WEAKNESS_ARCANE, SPELL_WEAKNESS_SHADOW};

enum OssirianActions
{
    OSSIRIAN_INITIAL_SPAWN,
    OSSIRIAN_SUPREME,
    OSSIRIAN_CYCLONE,
    OSSIRIAN_STOMP,
    OSSIRIAN_CURSE_OF_TONGUES,
    OSSIRIAN_SPEEDUP,
    OSSIRIAN_ACTION_MAX,
};

struct boss_ossirianAI : public CombatAI
{
    boss_ossirianAI(Creature* creature) :
        CombatAI(creature, OSSIRIAN_ACTION_MAX),
        m_instance(static_cast<instance_ruins_of_ahnqiraj*>(m_creature->GetInstanceData())),
        m_saidIntro(false),
        m_uiCrystalPosition(0)
    {
        AddCombatAction(OSSIRIAN_INITIAL_SPAWN, 10000u);
        AddCombatAction(OSSIRIAN_SUPREME, 45000u);
        AddCombatAction(OSSIRIAN_CYCLONE, 20000u);
        AddCombatAction(OSSIRIAN_STOMP, 30000u);
        AddCombatAction(OSSIRIAN_CURSE_OF_TONGUES, 30000u);
        AddCombatAction(OSSIRIAN_SPEEDUP, 10000u);
    }

    instance_ruins_of_ahnqiraj* m_instance;

    uint8 m_uiCrystalPosition;

    bool m_saidIntro;

    GuidVector m_spawnedCrystals;

    void Reset() override
    {
        CombatAI::Reset();
        m_uiCrystalPosition = 0;

        m_creature->UpdateSpeed(MOVE_RUN, false, 1.0f);

        DoCastSpellIfCan(nullptr, SPELL_DOUBLE_ATTACK, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
        DoCastSpellIfCan(nullptr, SPELL_SUPREME, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        DespawnGuids(m_spawnedCrystals);
        RespawnFirstCrystal();
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        for (auto aSandVortexSpawnPo : aSandVortexSpawnPos)
            m_creature->SummonCreature(NPC_SAND_VORTEX, aSandVortexSpawnPo[0], aSandVortexSpawnPo[1], aSandVortexSpawnPo[2], aSandVortexSpawnPo[3], TEMPSPAWN_CORPSE_DESPAWN, 0);

        if (m_instance)
            m_instance->instance->SetWeather(ZONE_ID_RUINS_AQ, WEATHER_TYPE_STORM, 1.0f, true);
    }

    void JustDied(Unit* /*killer*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        DoScriptText(SAY_SLAY, m_creature);
    }

    void DoSpawnNextCrystal(uint32 spawnCount)
    {
        if (!m_instance)
            return;

        GuidVector vector;
        m_instance->GetCreatureGuidVectorFromStorage(NPC_OSSIRIAN_TRIGGER, vector);
        if (vector.size() < 2) // wrong db data
            return;

        vector.erase(vector.begin()); // remove first
        std::shuffle(vector.begin(), vector.end(), *GetRandomGenerator());

        uint32 spawned = 0;
        for (uint32 i = 0; i < vector.size() - 1; ++i) // try to find at least one
        {
            // iterate from random roll until either one (should always occur) is found or we run out of crystals
            if (Creature* creature = m_creature->GetMap()->GetCreature(vector[i]))
            {
                if (!creature->IsAlive())
                {
                    creature->Respawn();
                    ++spawned;
                    if (spawned >= spawnCount)
                        break;
                }
            }
        }
    }

    void RespawnFirstCrystal()
    {
        GuidVector vector;
        m_instance->GetCreatureGuidVectorFromStorage(NPC_OSSIRIAN_TRIGGER, vector);
        if (vector.size() < 2) // wrong db data
            return;

        if (Creature* creature = m_creature->GetMap()->GetCreature(vector[0]))
            if (!creature->IsAlive())
                creature->Respawn();
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            Creature* crystal = static_cast<Creature*>(invoker);
            CreatureData const* data = sObjectMgr.GetCreatureData(crystal->GetGUIDLow());
            if (data->spawntimesecsmin == 0 && m_instance->GetData(TYPE_OSSIRIAN) != IN_PROGRESS)
            {
                if (GameObject* crystalGo = GetClosestGameObjectWithEntry(crystal, GO_OSSIRIAN_CRYSTAL, 5.0f))
                {
                    crystalGo->SetLootState(GO_JUST_DEACTIVATED);
                    crystalGo->SetForcedDespawn();
                }
                crystal->SetRespawnDelay(7200);
                crystal->ForcedDespawn();
            }
            else
            {
                crystal->CastSpell(nullptr, SPELL_SUMMON_CRYSTAL, TRIGGERED_OLD_TRIGGERED);
                m_spawnedCrystals.push_back(crystal->GetObjectGuid());
            }
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SAND_VORTEX)
        {
            // The movement of this isn't very clear - may require additional research
            summoned->CastSpell(summoned, SPELL_SAND_STORM, TRIGGERED_OLD_TRIGGERED);
            summoned->GetMotionMaster()->MoveRandomAroundPoint(aCrystalSpawnPos[0], aCrystalSpawnPos[1], aCrystalSpawnPos[2], 100.0f);
        }
    }

    void SpellHit(Unit* caster, const SpellEntry* spellInfo) override
    {
        if (caster->GetTypeId() == TYPEID_UNIT && caster->GetEntry() == NPC_OSSIRIAN_TRIGGER)
        {
            // Check for proper spell id
            if (std::find(aWeaknessSpell.begin(), aWeaknessSpell.end(), spellInfo->Id) == aWeaknessSpell.end())
                return;

            m_creature->RemoveAurasDueToSpell(SPELL_SUPREME);
            ResetCombatAction(OSSIRIAN_SUPREME, 45000);

            DoSpawnNextCrystal(1);
            // despawn go
            if (GameObject* crystal = GetClosestGameObjectWithEntry(caster, GO_OSSIRIAN_CRYSTAL, 5.0f))
            {
                crystal->SetLootState(GO_JUST_DEACTIVATED);
                crystal->SetForcedDespawn();
            }
            static_cast<Creature*>(caster)->SetRespawnDelay(7200);
            static_cast<Creature*>(caster)->ForcedDespawn(500);
        }
    }

    void MoveInLineOfSight(Unit* who) override
    {
        // TODO: Range guesswork
        if (!m_saidIntro && who->GetTypeId() == TYPEID_PLAYER && m_creature->IsWithinDistInMap(who, 75.0f, false))
        {
            switch (urand(0, 2))
            {
                case 0: DoScriptText(SAY_RAND_INTRO_1, m_creature); break;
                case 1: DoScriptText(SAY_RAND_INTRO_2, m_creature); break;
                case 2: DoScriptText(SAY_RAND_INTRO_3, m_creature); break;
            }
            m_saidIntro = true;
        }
        CombatAI::MoveInLineOfSight(who);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case OSSIRIAN_INITIAL_SPAWN:
            {
                DoSpawnNextCrystal(5);
                DisableCombatAction(action);
                break;
            }
            case OSSIRIAN_SUPREME:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_SUPREME, CAST_AURA_NOT_PRESENT) == CAST_OK)
                {
                    switch (urand(0, 2))
                    {
                        case 0: DoScriptText(SAY_SUPREME_1, m_creature); break;
                        case 1: DoScriptText(SAY_SUPREME_2, m_creature); break;
                        case 2: DoScriptText(SAY_SUPREME_3, m_creature); break;
                    }
                    ResetCombatAction(action, 45000);
                }
                break;
            }
            case OSSIRIAN_CYCLONE:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_CYCLONE) == CAST_OK)
                    ResetCombatAction(action, 20000);
                break;
            }
            case OSSIRIAN_STOMP:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_STOMP) == CAST_OK)
                    ResetCombatAction(action, 30000);
                break;
            }
            case OSSIRIAN_CURSE_OF_TONGUES:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_CURSE_OF_TONGUES) == CAST_OK)
                    ResetCombatAction(action, urand(20000, 30000));
                break;
            }
            case OSSIRIAN_SPEEDUP:
            {
                m_creature->UpdateSpeed(MOVE_RUN, false, 2.2f);
                DisableCombatAction(action);
                break;
            }
        }
    }
};

// This is actually a hack for a server-side spell
bool GOUse_go_ossirian_crystal(Player* /*player*/, GameObject* go)
{
    go->SendGameObjectCustomAnim(go->GetObjectGuid());
    if (Creature* pOssirianTrigger = GetClosestCreatureWithEntry(go, NPC_OSSIRIAN_TRIGGER, 10.0f))
        pOssirianTrigger->CastSpell(nullptr, aWeaknessSpell[urand(0, aWeaknessSpell.size() - 1)], TRIGGERED_NONE);

    return true;
}

void AddSC_boss_ossirian()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_ossirian";
    pNewScript->GetAI = &GetNewAIInstance<boss_ossirianAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_ossirian_crystal";
    pNewScript->pGOUse = &GOUse_go_ossirian_crystal;
    pNewScript->RegisterSelf();
}
