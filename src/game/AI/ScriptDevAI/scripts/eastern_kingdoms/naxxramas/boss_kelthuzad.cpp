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
SDName: Boss_KelThuzad
SD%Complete: 99
SDComment: Phase 1 summoning timers and/or aura stacking are to be improved once Classic sniffs are available
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

enum
{
    SAY_SUMMON_MINIONS                  = -1533105,         // start of phase 1

    SAY_AGGRO1                          = -1533094,
    SAY_AGGRO2                          = -1533095,
    SAY_AGGRO3                          = -1533096,

    SAY_SLAY1                           = -1533097,
    SAY_SLAY2                           = -1533098,

    SAY_DEATH                           = -1533099,

    SAY_CHAIN1                          = -1533100,
    SAY_CHAIN2                          = -1533101,
    SAY_FROST_BLAST                     = -1533102,

    SAY_REQUEST_AID                     = -1533103,         // Start of phase 3
    SAY_ANSWER_REQUEST                  = -1533104,         // Lich King answer

    SAY_SPECIAL1_MANA_DET               = -1533106,
    SAY_SPECIAL3_MANA_DET               = -1533107,
    SAY_SPECIAL2_DISPELL                = -1533108,

    EMOTE_GUARDIAN                      = -1533134,         // At each guardian summon

    // Phase 1 spells
    SPELL_SUMMON_TYPE_A                 = 28421,            // Summon 1 Soldier of the Frozen Waste
    SPELL_SUMMON_TYPE_B                 = 28422,            // Summon 1 Unstoppable Abomination
    SPELL_SUMMON_TYPE_C                 = 28423,            // Summon 1 Soul Weaver

    SPELL_CHANNEL_VISUAL_EFFECT         = 29422,            // Handle the application of summoning auras

    // Call forth one of the add to the center of the room
    // Soldier of the Frozen Waste (triggers 28415)
    SPELL_SUMMON_PERIODIC_A_1           = 29410,            // Every 5 secs
    SPELL_SUMMON_PERIODIC_A_2           = 29391,            // Every 4 secs
    SPELL_SUMMON_PERIODIC_A_3           = 28425,            // Every 3 secs
    SPELL_SUMMON_PERIODIC_A_4           = 29392,            // Every 2 secs
    SPELL_SUMMON_PERIODIC_A_5           = 29409,            // Every 1 secs
    // Unstoppable Abomination (triggers 28416)
    SPELL_SUMMON_PERIODIC_B_1           = 28426,            // Every 30 secs
    SPELL_SUMMON_PERIODIC_B_2           = 29393,            // Every 25 secs
    SPELL_SUMMON_PERIODIC_B_3           = 29394,            // Every 20 secs
    SPELL_SUMMON_PERIODIC_B_4           = 29398,            // Every 15 secs
    SPELL_SUMMON_PERIODIC_B_5           = 29411,            // Every 10 secs
    // Soul Weaver (triggers 28417)
    SPELL_SUMMON_PERIODIC_C_1           = 29399,            // Every 40 secs
    SPELL_SUMMON_PERIODIC_C_2           = 29400,            // Every 35 secs
    SPELL_SUMMON_PERIODIC_C_3           = 28427,            // Every 30 secs
    SPELL_SUMMON_PERIODIC_C_4           = 29401,            // Every 20 secs
    SPELL_SUMMON_PERIODIC_C_5           = 29412,            // Every 15 secs

    MAX_SUMMON_TICKS                    = 5 * MINUTE + 10,  // Adds despawn after 5 min 10 secs
    MAX_CHANNEL_TICKS                   = 5 * MINUTE + 25,  // Start phase 2 after 5 min 25 secs

    NPC_WORLD_TRIGGER                   = 15384,            // Handle the summoning of the NPCs in each alcove

    // Phase 2 spells
    SPELL_FROST_BOLT                    = 28478,
    SPELL_FROST_BOLT_NOVA               = 28479,

    SPELL_CHAINS_OF_KELTHUZAD           = 28408,
    SPELL_CHAINS_OF_KELTHUZAD_TARGET    = 28410,

    SPELL_MANA_DETONATION               = 27819,
    SPELL_SHADOW_FISSURE                = 27810,
    SPELL_FROST_BLAST                   = 27808,
};

static const uint32 phaseOneAdds[] = {NPC_SOLDIER_FROZEN, NPC_UNSTOPPABLE_ABOM, NPC_SOUL_WEAVER};

// List which spell must be used at which point in phase 1 to summon a given NPC type
static const std::vector<std::vector<uint32>> soulWeaverSpellsTimers {
        {1,   SPELL_SUMMON_PERIODIC_C_1},   // 1 for 40 sec
        {42,  SPELL_SUMMON_PERIODIC_C_4},   // 1 for 20 sec
        {63,  SPELL_SUMMON_PERIODIC_C_3},   // 3 for 90 sec
        {154, SPELL_SUMMON_PERIODIC_C_5},   // 9 for 135 sec
        {290, 0}                            // Total: 14 Soul Weavers
};

static const std::vector<std::vector<uint32>> unstoppableAbominationSpellsTimers {
        {15,  SPELL_SUMMON_PERIODIC_B_1},   // 2 for 60 sec
        {76,  SPELL_SUMMON_PERIODIC_B_2},   // 2 for 50 sec
        {127, SPELL_SUMMON_PERIODIC_B_3},   // 5 for 100 sec
        {228, SPELL_SUMMON_PERIODIC_B_4},   // 3 for 45 sec
        {259, SPELL_SUMMON_PERIODIC_B_5},   // 2 for 20 sec
        {291, 0}                            // Total: 14 Unstoppable Abominations
};

static const std::vector<std::vector<uint32>> soldierFrozenWasteSpellsTimers {
        {7,   SPELL_SUMMON_PERIODIC_A_1},   // 13 for 65 secs
        {73,  SPELL_SUMMON_PERIODIC_A_2},   // 15 for 60 secs
        {134, SPELL_SUMMON_PERIODIC_A_3},   // 18 for 54 secs
        {189, SPELL_SUMMON_PERIODIC_A_4},   // 27 for 54 secs
        {244, SPELL_SUMMON_PERIODIC_A_5},   // 47 for 47 secs
        {292, 0}                            // Total: 120 Soldiers of the Frozen Wastes
};

// List all spells used to summon a given NPC type during phase 1
static const std::map<uint32, std::vector<uint32>> summoningSpells {
        {NPC_SOUL_WEAVER,      {SPELL_SUMMON_PERIODIC_C_1, SPELL_SUMMON_PERIODIC_C_2, SPELL_SUMMON_PERIODIC_C_3, SPELL_SUMMON_PERIODIC_C_4, SPELL_SUMMON_PERIODIC_C_5}},
        {NPC_UNSTOPPABLE_ABOM, {SPELL_SUMMON_PERIODIC_B_1, SPELL_SUMMON_PERIODIC_B_2, SPELL_SUMMON_PERIODIC_B_3, SPELL_SUMMON_PERIODIC_B_4, SPELL_SUMMON_PERIODIC_B_5}},
        {NPC_SOLDIER_FROZEN,   {SPELL_SUMMON_PERIODIC_A_1, SPELL_SUMMON_PERIODIC_A_2, SPELL_SUMMON_PERIODIC_A_3, SPELL_SUMMON_PERIODIC_A_4, SPELL_SUMMON_PERIODIC_A_5}}
};

enum Phase
{
    PHASE_NORMAL,
    PHASE_GUARDIANS,
};

struct boss_kelthuzadAI : public ScriptedAI
{
    boss_kelthuzadAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();

        m_uiGuardiansCountMax = 4;
        Reset();
    }

    instance_naxxramas* m_pInstance;

    uint32 m_uiGuardiansCount;
    uint32 m_uiGuardiansCountMax;
    uint32 m_uiGuardiansTimer;
    uint32 m_uiLichKingAnswerTimer;
    uint32 m_uiFrostBoltTimer;
    uint32 m_uiFrostBoltNovaTimer;
    uint32 m_uiChainsTimer;
    uint32 m_uiManaDetonationTimer;
    uint32 m_uiShadowFissureTimer;
    uint32 m_uiFrostBlastTimer;

    uint8 m_uiPhase;
    uint32 m_summonTicks;

    GuidSet m_lIntroMobsSet;

    CreatureList m_summoningTriggers;

    void Reset() override
    {
        m_uiFrostBoltTimer       = urand(1, 60) * IN_MILLISECONDS;    // It won't be more than a minute without cast it
        m_uiFrostBoltNovaTimer   = 15 * IN_MILLISECONDS;              // Cast every 15 seconds
        m_uiChainsTimer          = urand(60, 120) * IN_MILLISECONDS;  // Cast every 60 - 120 seconds
        m_uiManaDetonationTimer  = 20 * IN_MILLISECONDS;              // Seems to cast about every 20 seconds
        m_uiShadowFissureTimer   = urand(10, 12) * IN_MILLISECONDS;   // Cast every 10 - 12 seconds
        m_uiFrostBlastTimer      = urand(30, 40) * IN_MILLISECONDS;   // Cast every 30 - 40 seconds
        m_uiGuardiansTimer       = 5 * IN_MILLISECONDS;               // 5 seconds for summoning each Guardian of Icecrown in phase 3
        m_uiLichKingAnswerTimer  = 4 * IN_MILLISECONDS;
        m_uiGuardiansCount       = 0;
        m_summonTicks = 0;

        m_uiPhase                = PHASE_NORMAL;

        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        m_creature->SetImmuneToPlayer(true);
        SetMeleeEnabled(false);
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() != TYPEID_PLAYER)
            return;

        if (urand(0, 1))
            DoScriptText(urand(0, 1) ? SAY_SLAY1 : SAY_SLAY2, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_KELTHUZAD, DONE);
    }

    void EnterEvadeMode() override
    {
        DespawnIntroCreatures();

        m_creature->InterruptNonMeleeSpells(false);

        if (m_creature->HasAura(SPELL_CHANNEL_VISUAL))
        {
            m_creature->RemoveAurasDueToSpell(SPELL_CHANNEL_VISUAL);
            m_creature->RemoveAurasDueToSpell(SPELL_CHANNEL_VISUAL_EFFECT);
        }

        if (m_pInstance)
            m_pInstance->SetData(TYPE_KELTHUZAD, FAIL);

        ScriptedAI::EnterEvadeMode();
    }

    void SpellHit(Unit* /*caster*/, const SpellEntry* spell) override
    {
        // Phase 1 start
        if (spell->Id == SPELL_CHANNEL_VISUAL)
        {
            DoScriptText(SAY_SUMMON_MINIONS, m_creature);

            // Get all summoning trigger NPCs
            m_summoningTriggers.clear();
            GetCreatureListWithEntryInGrid(m_summoningTriggers, m_creature, NPC_WORLD_TRIGGER, 100.0f);
        }
        // Phase 1 periodic update every 1 second (everything in phase 1 is handled here rather than in UpdateAI())
        else if (spell->Id == SPELL_CHANNEL_VISUAL_EFFECT)
        {
            ++m_summonTicks;
            switch (m_summonTicks)
            {
                case 4:
                    SummonIntroCreatures(NPC_SOLDIER_FROZEN, 9);
                    break;
                case 9:
                    SummonIntroCreatures(NPC_UNSTOPPABLE_ABOM, 3);
                    break;
                case 12:
                    SummonIntroCreatures(NPC_SOUL_WEAVER, 1);
                    break;
                case MAX_SUMMON_TICKS:
                    DespawnIntroCreatures();    // Will leave those in combat for some reason
                    break;
                case MAX_CHANNEL_TICKS:
                    StartPhase2();
                    break;
                default:
                    UpdateSummoning();
                    // Kel'Thuzad does not enter combat until phase 2, so we check every second if there are still players alive and force him to evade otherwise
                    if (!m_pInstance->GetPlayerInMap(true, false))
                        EnterEvadeMode();
                    break;
            }
        }
    }

    // Remove all periodic auras on Kel'Thuzad used to summon a given NPC type
    void StopAllSummoningForNPC(uint32 entry)
    {
        auto iter = summoningSpells.begin();
        while (iter != summoningSpells.end())
        {
            if (iter->first == entry)
                for (auto aura : iter->second)
                    m_creature->RemoveAurasDueToSpell(aura);
            ++iter;
        }
    }

    // Called every tick (1 second) to check if the periodic summoning auras on Kel'Thuzad need to be updated or removed
    void UpdateSummoning()
    {
        for (auto add : phaseOneAdds)
        {
            uint32 summoningAura = 0;
            std::vector<std::vector<uint32>> spellTimers;
            switch (add)
            {
                case NPC_SOUL_WEAVER:
                    spellTimers = soulWeaverSpellsTimers;
                    break;
                case NPC_UNSTOPPABLE_ABOM:
                    spellTimers = unstoppableAbominationSpellsTimers;
                    break;
                case NPC_SOLDIER_FROZEN:
                    spellTimers = soldierFrozenWasteSpellsTimers;
                    break;
                default:
                    return;
            }

            // Iterate the summoning auras timeline for the current add as long as the tick is in range
            // then exit with the last valid summoning aura
            for (const auto& spellTimer : spellTimers)
            {
                if (spellTimer[0] <= m_summonTicks)
                    summoningAura = spellTimer[1];
                else
                    break;
            }

            // Do not update aura for current add type if summoning aura is already present
            if (m_creature->HasAura(summoningAura))
                continue;

            // Clear all summoning auras for current add before applying the new one (especially needed in case new one is NULL)
            StopAllSummoningForNPC(add);
            DoCastSpellIfCan(m_creature, summoningAura, CAST_TRIGGERED);
        }
    }

    void Aggro(Unit* /*who*/) override
    {
        switch (urand(0, 2))
        {
            case 0:
                DoScriptText(SAY_AGGRO1, m_creature);
                break;
            case 1:
                DoScriptText(SAY_AGGRO2, m_creature);
                break;
            case 2:
                DoScriptText(SAY_AGGRO3, m_creature);
                break;
        }
    }

    void DespawnIntroCreatures()
    {
        if (m_pInstance)
        {
            for (auto itr : m_lIntroMobsSet)
            {
                if (Creature* pCreature = m_pInstance->instance->GetCreature(itr))
                {
                    if (pCreature->isAlive() && !pCreature->isInCombat())
                        pCreature->ForcedDespawn();
                }
            }
        }

        m_lIntroMobsSet.clear();
    }

    // Summon three type of adds in each of the surrounding alcoves
    bool SummonIntroCreatures(uint32 entry, uint8 count)
    {
        if (!m_pInstance)
            return false;

        float fNewX, fNewY, fNewZ;

        // Spawn all the adds for phase 1 from each of the trigger NPCs
        for (auto& trigger : m_summoningTriggers)
        {
            // "How many NPCs per type" is stored in a vector: {npc_entry:number_of_npcs}
            for (uint8 i = 0; i < count; ++i)
            {
                m_creature->GetRandomPoint(trigger->GetPositionX(), trigger->GetPositionY(), trigger->GetPositionZ(), 12.0f, fNewX, fNewY, fNewZ);
                if (Creature* summoned = m_creature->SummonCreature(entry, fNewX, fNewY, fNewZ, 0.0f, TEMPSPAWN_CORPSE_DESPAWN, 5 * MINUTE * IN_MILLISECONDS))
                {
                    if (summoned->AI())
                        summoned->AI()->SetReactState(REACT_PASSIVE);   // Intro mobs only attack if engaged or hostile target in range
                }
            }
        }
        return true;
    }

    void JustSummoned(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_SOLDIER_FROZEN:
            case NPC_UNSTOPPABLE_ABOM:
            case NPC_SOUL_WEAVER:
                m_lIntroMobsSet.insert(pSummoned->GetObjectGuid());
                break;
            default:
                break;
        }
    }

    // Every time an add dies, randomly summon the same type on one of the seven corners
    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        switch (pSummoned->GetEntry())
        {
            case NPC_SOLDIER_FROZEN:
            case NPC_SOUL_WEAVER:
            case NPC_UNSTOPPABLE_ABOM:
                m_lIntroMobsSet.erase(pSummoned->GetObjectGuid());
                break;
            default:
                break;
        }
    }

    void StartPhase2()
    {
        // Clear all summoning auras
        m_creature->InterruptNonMeleeSpells(false);

        // Make attackable and engage a target
        m_creature->SetImmuneToPlayer(false);
        m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        SetMeleeEnabled(true);
        SetReactState(REACT_AGGRESSIVE);
        m_creature->SetInCombatWithZone();
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        if (m_uiFrostBoltTimer < uiDiff)
        {
           if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FROST_BOLT) == CAST_OK)
               m_uiFrostBoltTimer = urand(1, 60) * IN_MILLISECONDS;
        }
        else
           m_uiFrostBoltTimer -= uiDiff;

        if (m_uiFrostBoltNovaTimer < uiDiff)
        {
          if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FROST_BOLT_NOVA) == CAST_OK)
              m_uiFrostBoltNovaTimer = 15 * IN_MILLISECONDS;
        }
        else
          m_uiFrostBoltNovaTimer -= uiDiff;

        if (m_uiManaDetonationTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, SPELL_MANA_DETONATION, SELECT_FLAG_PLAYER | SELECT_FLAG_POWER_MANA))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_MANA_DETONATION) == CAST_OK)
                {
                    if (urand(0, 1))
                        DoScriptText(SAY_SPECIAL1_MANA_DET, m_creature);

                    m_uiManaDetonationTimer = 20 * IN_MILLISECONDS;
                }
            }
        }
        else
            m_uiManaDetonationTimer -= uiDiff;

        if (m_uiShadowFissureTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_SHADOW_FISSURE) == CAST_OK)
                {
                    if (urand(0, 1))
                        DoScriptText(SAY_SPECIAL3_MANA_DET, m_creature);

                    m_uiShadowFissureTimer = urand(10, 12) * IN_MILLISECONDS;
                }
            }
        }
        else
            m_uiShadowFissureTimer -= uiDiff;

        if (m_uiFrostBlastTimer < uiDiff)
        {
           if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_FROST_BLAST) == CAST_OK)
           {
              if (urand(0, 1))
                   DoScriptText(SAY_FROST_BLAST, m_creature);

                   m_uiFrostBlastTimer = urand(30, 40) * IN_MILLISECONDS;
           }
        }
        else
           m_uiFrostBlastTimer -= uiDiff;

        if (m_uiChainsTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_CHAINS_OF_KELTHUZAD) == CAST_OK)
            {
                DoScriptText(urand(0, 1) ? SAY_CHAIN1 : SAY_CHAIN2, m_creature);

                m_uiChainsTimer = urand(60, 120) * IN_MILLISECONDS;
            }
        }
        else
            m_uiChainsTimer -= uiDiff;

        if (m_uiPhase == PHASE_NORMAL)
        {
            if (m_creature->GetHealthPercent() < 40.0f)
            {
                m_uiPhase = PHASE_GUARDIANS;
                DoScriptText(SAY_REQUEST_AID, m_creature);
            }
        }
        if (m_uiPhase == PHASE_GUARDIANS && m_uiGuardiansCount < m_uiGuardiansCountMax)
        {
            if (m_uiGuardiansTimer < uiDiff)
            {
            // Summon a Guardian of Icecrown in a random alcove
            //   SummonMob(NPC_GUARDIAN);
               m_uiGuardiansTimer = 5 * IN_MILLISECONDS;
           }
           else
               m_uiGuardiansTimer -= uiDiff;

           if (m_uiLichKingAnswerTimer && m_pInstance)
           {
               if (m_uiLichKingAnswerTimer <= uiDiff)
               {
                   if (Creature* pLichKing = m_pInstance->GetSingleCreatureFromStorage(NPC_THE_LICHKING))
                       DoScriptText(SAY_ANSWER_REQUEST, pLichKing);

                   m_pInstance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_1);
                   m_pInstance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_2);
                   m_pInstance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_3);
                   m_pInstance->DoUseDoorOrButton(GO_KELTHUZAD_WINDOW_4);

                   m_uiLichKingAnswerTimer = 0;
               }
               else
                   m_uiLichKingAnswerTimer -= uiDiff;
           }
        }

        DoMeleeAttackIfReady();
    }
};

UnitAI* GetAI_boss_kelthuzad(Creature* pCreature)
{
    return new boss_kelthuzadAI(pCreature);
}

// Summon one add (which type depends on spell)
struct TriggerKTAdd :
        public SpellScript
{
    void OnEffectExecute(Spell* spell, SpellEffectIndex /* effIdx */) const override
    {
        if (Unit* unitTarget = spell->GetUnitTarget())
        {
            switch (spell->m_spellInfo->Id)
            {
                case 28415:
                    unitTarget->CastSpell(unitTarget, SPELL_SUMMON_TYPE_A, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetCaster()->GetObjectGuid(), nullptr);
                    break;
                case 28416:
                    unitTarget->CastSpell(unitTarget, SPELL_SUMMON_TYPE_B, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetCaster()->GetObjectGuid(), nullptr);
                    break;
                case 28417:
                    unitTarget->CastSpell(unitTarget, SPELL_SUMMON_TYPE_C, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, spell->GetCaster()->GetObjectGuid(), nullptr);
                    break;
                default:
                    break;
            }
        }
    }
};

void AddSC_boss_kelthuzad()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_kelthuzad";
    pNewScript->GetAI = &GetAI_boss_kelthuzad;
    pNewScript->RegisterSelf();

    RegisterSpellScript<TriggerKTAdd>("spell_trigger_KT_add");
}
