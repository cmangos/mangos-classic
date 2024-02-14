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
SDName: Boss_Mandokir
SD%Complete: 80
SDComment: test Threating Gaze. Script depends on ACID script for Vilebranch Speaker
SDCategory: Zul'Gurub
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "zulgurub.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    NPC_OHGAN           = 14988,
    NPC_CHAINED_SPIRIT  = 15117,                            // resing spirits

    SAY_AGGRO           = -1309015,
    SAY_DING_KILL       = -1309016,
    SAY_GRATS_JINDO     = -1309017,
    SAY_WATCH           = -1309018,
    SAY_WATCH_WHISPER   = -1309019,

    EMOTE_RAGE          = -1309024,

    SPELL_CHARGE        = 24408,
    SPELL_CHARGE_GAZE   = 24315,
    SPELL_INTIMIDATING_SHOUT          = 29321,
    SPELL_WHIRLWIND     = 13736,
    SPELL_MORTAL_STRIKE = 16856,
    SPELL_ENRAGE        = 23537,
    SPELL_THREATENING_GAZE = 24314,
    SPELL_SUMMON_PLAYER = 25104,
    SPELL_LEVEL_UP      = 24312,
    SPELL_OVERPOWER     = 24407,

    SPELL_SUMMON_BLOODLORDS_RAPTOR = 24349,

    // Ohgans Spells
    SPELL_SUNDERARMOR   = 24317,

    // Chained Spirit Spells
    SPELL_REVIVE        = 24341,

    SPELL_DESPAWN_CHAINED_SPIRITS = 24342,

    POINT_DOWNSTAIRS    = 1
};

struct SpawnLocations
{
    float fX, fY, fZ, fAng;
};

static SpawnLocations aSpirits[] =
{
    { -12150.9f, -1956.24f, 133.407f, 2.57835f},
    { -12157.1f, -1972.78f, 133.947f, 2.64903f},
    { -12172.3f, -1982.63f, 134.061f, 1.48664f},
    { -12194.0f, -1979.54f, 132.194f, 1.45916f},
    { -12211.3f, -1978.49f, 133.580f, 1.35705f},
    { -12228.4f, -1977.10f, 132.728f, 1.25495f},
    { -12250.0f, -1964.78f, 135.066f, 0.92901f},
    { -12264.0f, -1953.08f, 134.072f, 0.62663f},
    { -12289.0f, -1924.00f, 132.620f, 5.37829f},
    { -12267.3f, -1902.26f, 131.328f, 5.32724f},
    { -12255.3f, -1893.53f, 134.026f, 5.06413f},
    { -12229.9f, -1891.39f, 134.704f, 4.40047f},
    { -12215.9f, -1889.09f, 137.273f, 4.70285f},
    { -12200.5f, -1890.69f, 135.777f, 4.84422f},
    { -12186.0f, -1890.12f, 134.261f, 4.36513f},
    { -12246.3f, -1890.09f, 135.475f, 4.73427f},
    { -12170.7f, -1894.85f, 133.852f, 3.51690f},
    { -12279.0f, -1931.92f, 136.130f, 0.04151f},
    { -12266.1f, -1940.72f, 132.606f, 0.70910f}
};

enum MandokirActions
{
    MANDOKIR_ACTION_MAX,
};

struct boss_mandokirAI : public CombatAI
{
    boss_mandokirAI(Creature* creature) : CombatAI(creature, MANDOKIR_ACTION_MAX), m_instance(static_cast<ScriptedInstance*>(creature->GetInstanceData()))
    {

    }

    ScriptedInstance* m_instance;

    uint8 m_uiKillCount;

    float m_targetThreat;

    void Reset() override
    {
        CombatAI::Reset();

        m_uiKillCount           = 0;

        m_targetThreat         = 0.0f;
    }

    void Aggro(Unit* /*who*/) override
    {
        DoScriptText(SAY_AGGRO, m_creature);

        for (auto& aSpirit : aSpirits)
            m_creature->SummonCreature(NPC_CHAINED_SPIRIT, aSpirit.fX, aSpirit.fY, aSpirit.fZ, aSpirit.fAng, TEMPSPAWN_CORPSE_DESPAWN, 0);

        // At combat start Mandokir is mounted so we must unmount it first
        m_creature->Unmount();

        // And summon his raptor
        m_creature->CastSpell(nullptr, SPELL_SUMMON_BLOODLORDS_RAPTOR, TRIGGERED_NONE);

        if (m_instance)
            m_instance->SetData(TYPE_OHGAN, IN_PROGRESS);
    }

    void EnterEvadeMode() override
    {
        if (m_instance)
            m_instance->SetData(TYPE_OHGAN, FAIL);

        m_creature->CastSpell(nullptr, SPELL_DESPAWN_CHAINED_SPIRITS, TRIGGERED_OLD_TRIGGERED);

        CombatAI::EnterEvadeMode();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (m_instance)
            m_instance->SetData(TYPE_OHGAN, DONE);

        m_creature->CastSpell(nullptr, SPELL_DESPAWN_CHAINED_SPIRITS, TRIGGERED_OLD_TRIGGERED);
    }

    void SpellHitTarget(Unit* /*target*/, const SpellEntry* spellInfo) override
    {
        if (spellInfo->Id == SPELL_CHARGE)
            DoResetThreat();
    }

    void ReceiveAIEvent(AIEventType eventType, Unit* /*sender*/, Unit* invoker, uint32 /*miscValue*/) override
    {
        if (eventType == AI_EVENT_CUSTOM_A)
        {
            DoScriptText(SAY_WATCH, m_creature, invoker);
            DoScriptText(SAY_WATCH_WHISPER, m_creature, invoker);
            m_targetThreat = m_creature->getThreatManager().getThreat(invoker);
        }
        else if (eventType == AI_EVENT_CUSTOM_B)
        {
            Player* watchTarget = dynamic_cast<Player*>(invoker);

            // If threat is higher that previously saved, mandokir will act
            if (watchTarget && watchTarget->IsAlive() && m_creature->getThreatManager().getThreat(watchTarget) > m_targetThreat)
            {
                if (!m_creature->IsWithinLOSInMap(watchTarget))
                    m_creature->CastSpell(watchTarget, SPELL_SUMMON_PLAYER, TRIGGERED_OLD_TRIGGERED);

                m_creature->CastSpell(watchTarget, SPELL_CHARGE_GAZE, TRIGGERED_OLD_TRIGGERED);
            }
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
        {
            ++m_uiKillCount;

            if (m_uiKillCount == 3)
            {
                DoScriptText(SAY_DING_KILL, m_creature);

                if (m_instance)
                {
                    if (Creature* pJindo = m_instance->GetSingleCreatureFromStorage(NPC_JINDO))
                    {
                        if (pJindo->IsAlive())
                            DoScriptText(SAY_GRATS_JINDO, pJindo);
                    }
                }

                DoCastSpellIfCan(nullptr, SPELL_LEVEL_UP, CAST_TRIGGERED);
                m_uiKillCount = 0;
            }

            if (m_creature->IsInCombat())
            {
                if (Creature* spirit = GetClosestCreatureWithEntry(victim, NPC_CHAINED_SPIRIT, 50.0f))
                    spirit->CastSpell(victim, SPELL_REVIVE, TRIGGERED_NONE);
            }
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_OHGAN)
        {
            if (m_creature->GetVictim())
                summoned->AI()->AttackStart(m_creature->GetVictim());
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_OHGAN)
        {
            DoCastSpellIfCan(nullptr, SPELL_ENRAGE, CAST_TRIGGERED);
            DoScriptText(EMOTE_RAGE, m_creature);
        }
    }

    void MovementInform(uint32 moveType, uint32 pointId) override
    {
        if (moveType != POINT_MOTION_TYPE || !m_instance)
            return;

        if (pointId == POINT_DOWNSTAIRS)
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            m_creature->SetInCombatWithZone();
        }
    }
};

// Ohgan
struct mob_ohganAI : public CombatAI
{
    mob_ohganAI(Creature* creature) : CombatAI(creature, 1)
    {
        AddCombatAction(0, 5000u);
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->IsPlayer())
        {
            if (m_creature->IsInCombat())
            {
                if (Creature* spirit = GetClosestCreatureWithEntry(pVictim, NPC_CHAINED_SPIRIT, 50.0f))
                    spirit->CastSpell(pVictim, SPELL_REVIVE, TRIGGERED_NONE);
            }
        }
    }

    void ExecuteAction(uint32 action) override
    {
        if (action == 0)
        {
            if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_SUNDERARMOR) == CAST_OK)
                ResetCombatAction(action, urand(10000, 15000));
        }
    }
};

struct ThreateningGaze : public AuraScript
{
    void OnApply(Aura* aura, bool apply) const override
    {
        if (Unit* caster = aura->GetCaster())
            if (caster->AI())
                caster->AI()->ReceiveAIEvent((apply ? AI_EVENT_CUSTOM_A : AI_EVENT_CUSTOM_B), aura->GetTarget(), aura->GetTarget(), 0);
    }
};

struct DespawnChainedSpirits : public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex effIdx) const override
    {
        if (effIdx == EFFECT_INDEX_0)
            if (Unit* target = spell->GetUnitTarget())
                if (target->GetEntry() == NPC_CHAINED_SPIRIT && target->IsCreature())
                    static_cast<Creature*>(target)->ForcedDespawn();
    }
};

void AddSC_boss_mandokir()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_mandokir";
    pNewScript->GetAI = &GetNewAIInstance<boss_mandokirAI>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "mob_ohgan";
    pNewScript->GetAI = &GetNewAIInstance<mob_ohganAI>;
    pNewScript->RegisterSelf();

    RegisterSpellScript<ThreateningGaze>("spell_threatening_gaze");
    RegisterSpellScript<DespawnChainedSpirits>("spell_despawn_chained_spirits");
}
