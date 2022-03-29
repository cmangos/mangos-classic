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
SDName: Boss_Fankriss
SD%Complete: 100
SDComment: sound not implemented
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Spells/Scripts/SpellScript.h"

enum
{
    SOUND_SENTENCE_YOU      = 8588,
    SOUND_SERVE_TO          = 8589,
    SOUND_LAWS              = 8590,
    SOUND_TRESPASS          = 8591,
    SOUND_WILL_BE           = 8592,

    SPELL_MORTAL_WOUND      = 25646,
    SPELL_ENTANGLE_1        = 720,
    SPELL_ENTANGLE_2        = 731,
    SPELL_ENTANGLE_3        = 1121,
    SPELL_SUMMON_WORM_1     = 518,
    SPELL_SUMMON_WORM_2     = 25831,
    SPELL_SUMMON_WORM_3     = 25832,

    SPELL_SPAWN_VEKNISS_HATCHLING_1 = 26630,
    SPELL_SPAWN_VEKNISS_HATCHLING_2 = 26631,
    SPELL_SPAWN_VEKNISS_HATCHLING_3 = 26632,

    NPC_SPAWN_FANKRISS      = 15630,
};

static const uint32 aEntangleSpells[3] = { SPELL_ENTANGLE_1, SPELL_ENTANGLE_2, SPELL_ENTANGLE_3 };
static const uint32 aSpawnHatchlingsSpells[3] = { SPELL_SPAWN_VEKNISS_HATCHLING_1, SPELL_SPAWN_VEKNISS_HATCHLING_2, SPELL_SPAWN_VEKNISS_HATCHLING_3 };

enum FankrissActions
{
    FANKRISS_MORTAL_WOUND,
    FANKRISS_SUMMON_WORM,
    FANKRISS_ENTANGLE,
    FANKRISS_ACTION_MAX,
};

struct boss_fankrissAI : public CombatAI
{
    boss_fankrissAI(Creature* creature) : CombatAI(creature, FANKRISS_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {
        AddCombatAction(FANKRISS_MORTAL_WOUND, 10000, 15000);
        AddCombatAction(FANKRISS_SUMMON_WORM, 30000, 50000);
        AddCombatAction(FANKRISS_ENTANGLE, 15000, 20000);
    }

    ScriptedInstance* m_instance;

    bool m_isInitialEntangleDone;
    uint8 m_wormsSpawnPerWave;

    std::vector<uint32> m_summonWormSpells;

    ObjectGuid m_EntangleTargetGuid;
    GuidVector m_spawns;

    void Reset() override
    {
        CombatAI::Reset();
        m_EntangleTargetGuid.Clear();
        DespawnGuids(m_spawns);
        m_isInitialEntangleDone = false;
        m_wormsSpawnPerWave = urand(1, 3);  // One to three Spawn of Fankriss are spawned for each wave
        m_summonWormSpells = { SPELL_SUMMON_WORM_1, SPELL_SUMMON_WORM_2, SPELL_SUMMON_WORM_3 };
    }

    void Aggro(Unit* /*who*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_FANKRISS, IN_PROGRESS);

        std::shuffle(m_summonWormSpells.begin(), m_summonWormSpells.end(), *GetRandomGenerator());
    }

    void JustReachedHome() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_FANKRISS, FAIL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_FANKRISS, DONE);
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_SPAWN_FANKRISS)
        {
            if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                summoned->AI()->AttackStart(target);
        }
        else
            summoned->SetInCombatWithZone();
        m_spawns.push_back(summoned->GetObjectGuid());
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        summoned->ForcedDespawn(2000);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case FANKRISS_MORTAL_WOUND:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_MORTAL_WOUND) == CAST_OK)
                    ResetCombatAction(action, urand(7000, 14000));
                break;
            }
            case FANKRISS_SUMMON_WORM:
            {
                if (DoCastSpellIfCan(nullptr, m_summonWormSpells[m_wormsSpawnPerWave - 1]) == CAST_OK)
                {
                    --m_wormsSpawnPerWave;
                    // Initialise the next wave now that all Spawn of Fankriss for the current wave were spawned
                    if (!m_wormsSpawnPerWave)
                    {
                        // Determine how many Spawn of Fankriss will be spawned in the next wave
                        m_wormsSpawnPerWave = urand(1, 3);
                        // Randomise the summoning spells so each NPC spawns in a different location
                        std::shuffle(m_summonWormSpells.begin(), m_summonWormSpells.end(), *GetRandomGenerator());
                        ResetCombatAction(action, urand(22000, 70000));
                    }
                    else    // Summon another Spawn of Fankriss in a few seconds
                        ResetCombatAction(action, urand(5000, 7000));
                }
                break;
            }
            case FANKRISS_ENTANGLE:
            {
                uint32 entangleIndex = urand(0, 2);
                // Teleport and untangle one random player into one of the three alcoves
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER | SELECT_FLAG_SKIP_TANK))
                {
                    if (DoCastSpellIfCan(target, aEntangleSpells[entangleIndex]) == CAST_OK)
                    {
                        m_EntangleTargetGuid = target->GetObjectGuid();
                        ResetCombatAction(action, urand(25000, 55000));
                    }
                }

                // At combat start, Vekniss Hatchlings are summoned in all three alcoves
                if (!m_isInitialEntangleDone)
                {
                    DoCastSpellIfCan(nullptr, aSpawnHatchlingsSpells[(entangleIndex + 1) % 3], CAST_TRIGGERED);
                    DoCastSpellIfCan(nullptr, aSpawnHatchlingsSpells[(entangleIndex + 2) % 3], CAST_TRIGGERED);
                    m_isInitialEntangleDone = true;
                }
                break;
            }
        }
    }
};

struct EntangleFankriss : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
        {
            uint32 spellId = 0;
            switch (spell->m_spellInfo->Id)
            {
                default:
                case SPELL_ENTANGLE_1: spellId = SPELL_SPAWN_VEKNISS_HATCHLING_1; break;
                case SPELL_ENTANGLE_2: spellId = SPELL_SPAWN_VEKNISS_HATCHLING_2; break;
                case SPELL_ENTANGLE_3: spellId = SPELL_SPAWN_VEKNISS_HATCHLING_3; break;
            }
            spell->GetCaster()->CastSpell(nullptr, spellId, TRIGGERED_OLD_TRIGGERED);
        }
    }
};

void AddSC_boss_fankriss()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_fankriss";
    pNewScript->GetAI = &GetNewAIInstance<boss_fankrissAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<EntangleFankriss>("spell_entangle_fankriss");
}
