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
SDName: Boss_Gothik
SD%Complete: 95
SDComment: Prevent Gothik from turning and "in combat state" while on balcony
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"

enum
{
    SAY_SPEECH_1                = -1533040,
    SAY_SPEECH_2                = -1533140,
    SAY_SPEECH_3                = -1533141,
    SAY_SPEECH_4                = -1533142,

    SAY_KILL                    = -1533041,
    SAY_DEATH                   = -1533042,
    SAY_TELEPORT                = -1533043,

    EMOTE_TO_FRAY               = -1533138,
    EMOTE_GATE                  = -1533139,

    PHASE_SPEECH                = 0,
    PHASE_BALCONY               = 1,
    PHASE_STOP_SUMMONING        = 2,
    PHASE_TELEPORTING           = 3,
    PHASE_STOP_TELEPORTING      = 4,

    // Right is right side from gothik (eastern)
    SPELL_TELEPORT_RIGHT        = 28025,
    SPELL_TELEPORT_LEFT         = 28026,

    SPELL_HARVESTSOUL           = 28679,
    SPELL_SHADOWBOLT            = 29317,
};

enum eSpellDummy
{
    SPELL_A_TO_ANCHOR_1     = 27892,
    SPELL_B_TO_ANCHOR_1     = 27928,
    SPELL_C_TO_ANCHOR_1     = 27935,

    SPELL_A_TO_ANCHOR_2     = 27893,
    SPELL_B_TO_ANCHOR_2     = 27929,
    SPELL_C_TO_ANCHOR_2     = 27936,

    SPELL_A_TO_SKULL        = 27915,
    SPELL_B_TO_SKULL        = 27931,
    SPELL_C_TO_SKULL        = 27937
};

struct boss_gothikAI : public ScriptedAI
{
    boss_gothikAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_naxxramas*)pCreature->GetInstanceData();
        SetCombatMovement(false);
        Reset();
    }

    instance_naxxramas* m_pInstance;

    GuidList m_lSummonedAddGuids;
    GuidList m_lTraineeSummonPosGuids;
    GuidList m_lDeathKnightSummonPosGuids;
    GuidList m_lRiderSummonPosGuids;

    uint8 m_uiPhase;
    uint8 m_uiSpeech;

    uint32 m_uiTraineeTimer;
    uint32 m_uiDeathKnightTimer;
    uint32 m_uiRiderTimer;
    uint32 m_uiTeleportTimer;
    uint32 m_uiShadowboltTimer;
    uint32 m_uiHarvestSoulTimer;
    uint32 m_uiPhaseTimer;
    uint32 m_uiControlZoneTimer;
    uint32 m_uiSpeechTimer;

    void Reset() override
    {
        // Remove immunity
        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);

        m_uiPhase = PHASE_SPEECH;
        m_uiSpeech = 1;

        m_uiTraineeTimer = 24 * IN_MILLISECONDS;
        m_uiDeathKnightTimer = 74 * IN_MILLISECONDS;
        m_uiRiderTimer = 134 * IN_MILLISECONDS;
        m_uiTeleportTimer = 20 * IN_MILLISECONDS;
        m_uiShadowboltTimer = 2 * IN_MILLISECONDS;
        m_uiHarvestSoulTimer = 2500;
        m_uiPhaseTimer = 4 * MINUTE * IN_MILLISECONDS + 7 * IN_MILLISECONDS; // last summon at 4:04, next would be 4:09
        m_uiControlZoneTimer = urand(120 * IN_MILLISECONDS, 150 * IN_MILLISECONDS);
        m_uiSpeechTimer = 1 * IN_MILLISECONDS;

        // Despawn Adds
        for (GuidList::const_iterator itr = m_lSummonedAddGuids.begin(); itr != m_lSummonedAddGuids.end(); itr++)
        {
            if (Creature* pCreature = m_creature->GetMap()->GetCreature(*itr))
                pCreature->ForcedDespawn();
        }

        m_lSummonedAddGuids.clear();
        m_lTraineeSummonPosGuids.clear();
        m_lDeathKnightSummonPosGuids.clear();
        m_lRiderSummonPosGuids.clear();
    }

    void Aggro(Unit* /*pWho*/) override
    {
        if (!m_pInstance)
            return;

        m_pInstance->SetData(TYPE_GOTHIK, IN_PROGRESS);

        // Make immune
        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);

        m_pInstance->SetGothTriggers();
        PrepareSummonPlaces();
    }

    bool IsCentralDoorClosed()
    {
        return m_pInstance && m_pInstance->GetData(TYPE_GOTHIK) != SPECIAL;
    }

    void ProcessCentralDoor()
    {
        if (IsCentralDoorClosed())
        {
            m_pInstance->SetData(TYPE_GOTHIK, SPECIAL);
            DoScriptText(EMOTE_GATE, m_creature);
        }
    }

    bool HasPlayersInLeftSide()
    {
        Map::PlayerList const& lPlayers = m_pInstance->instance->GetPlayers();

        if (lPlayers.isEmpty())
            return false;

        for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
        {
            if (Player* pPlayer = itr->getSource())
            {
                if (!m_pInstance->IsInRightSideGothArea(pPlayer) && pPlayer->isAlive())
                    return true;
            }
        }

        return false;
    }

    void KilledUnit(Unit* pVictim) override
    {
        if (pVictim->GetTypeId() == TYPEID_PLAYER)
            DoScriptText(SAY_KILL, m_creature);
    }

    void JustDied(Unit* /*pKiller*/) override
    {
        DoScriptText(SAY_DEATH, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_GOTHIK, DONE);
    }

    void JustReachedHome() override
    {
        if (m_pInstance)
            m_pInstance->SetData(TYPE_GOTHIK, FAIL);
    }

    void PrepareSummonPlaces()
    {
        std::list<Creature*> lSummonList;
        m_pInstance->GetGothSummonPointCreatures(lSummonList, true);

        if (lSummonList.empty())
            return;

        // Trainees and Rider
        uint8 index = 0;
        uint8 uiTraineeCount = 3;
        lSummonList.sort(ObjectDistanceOrder(m_creature));
        for (std::list<Creature*>::iterator itr = lSummonList.begin(); itr != lSummonList.end(); ++itr)
        {
            if (*itr)
            {
                if (uiTraineeCount == 0)
                    break;
                if (index == 1)
                    m_lRiderSummonPosGuids.push_back((*itr)->GetObjectGuid());
                else
                {
                    m_lTraineeSummonPosGuids.push_back((*itr)->GetObjectGuid());
                    --uiTraineeCount;
                }
                index++;
            }
        }

        // DeathKnights
        uint8 uiDeathKnightCount = 2;
        lSummonList.sort(ObjectDistanceOrderReversed(m_creature));
        for (std::list<Creature*>::iterator itr = lSummonList.begin(); itr != lSummonList.end(); ++itr)
        {
            if (*itr)
            {
                if (uiDeathKnightCount == 0)
                    break;
                m_lDeathKnightSummonPosGuids.push_back((*itr)->GetObjectGuid());
                --uiDeathKnightCount;
            }
        }
    }

    void SummonAdds(bool /*bRightSide*/, uint32 uiSummonEntry)
    {
        GuidList* plSummonPosGuids;
        switch (uiSummonEntry)
        {
            case NPC_UNREL_TRAINEE:      plSummonPosGuids = &m_lTraineeSummonPosGuids;     break;
            case NPC_UNREL_DEATH_KNIGHT: plSummonPosGuids = &m_lDeathKnightSummonPosGuids; break;
            case NPC_UNREL_RIDER:        plSummonPosGuids = &m_lRiderSummonPosGuids;       break;
            default:
                return;
        }
        if (plSummonPosGuids->empty())
            return;

        for (GuidList::iterator itr = plSummonPosGuids->begin(); itr != plSummonPosGuids->end(); ++itr)
        {
            if (Creature* pPos = m_creature->GetMap()->GetCreature(*itr))
                m_creature->SummonCreature(uiSummonEntry, pPos->GetPositionX(), pPos->GetPositionY(), pPos->GetPositionZ(), pPos->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        m_lSummonedAddGuids.push_back(pSummoned->GetObjectGuid());
        if (!IsCentralDoorClosed())
            pSummoned->SetInCombatWithZone();
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        m_lSummonedAddGuids.remove(pSummoned->GetObjectGuid());

        if (!m_pInstance)
            return;

        if (Creature* pAnchor = m_pInstance->GetClosestAnchorForGoth(pSummoned, true))
        {
            switch (pSummoned->GetEntry())
            {
                    // Wrong caster, it expected to be pSummoned.
                    // Mangos deletes the spell event at caster death, so for delayed spell like this
                    // it's just a workaround. Does not affect other than the visual though (+ spell takes longer to "travel")
                case NPC_UNREL_TRAINEE:         m_creature->CastSpell(pAnchor, SPELL_A_TO_ANCHOR_1, true, nullptr, nullptr, pSummoned->GetObjectGuid()); break;
                case NPC_UNREL_DEATH_KNIGHT:    m_creature->CastSpell(pAnchor, SPELL_B_TO_ANCHOR_1, true, nullptr, nullptr, pSummoned->GetObjectGuid()); break;
                case NPC_UNREL_RIDER:           m_creature->CastSpell(pAnchor, SPELL_C_TO_ANCHOR_1, true, nullptr, nullptr, pSummoned->GetObjectGuid()); break;
            }
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        switch (m_uiPhase)
        {
            case PHASE_SPEECH:
                if (m_uiSpeechTimer < uiDiff)
                {
                    switch (m_uiSpeech)
                    {
                        case 1: DoScriptText(SAY_SPEECH_1, m_creature); m_uiSpeechTimer = 4 * IN_MILLISECONDS; break;
                        case 2: DoScriptText(SAY_SPEECH_2, m_creature); m_uiSpeechTimer = 6 * IN_MILLISECONDS; break;
                        case 3: DoScriptText(SAY_SPEECH_3, m_creature); m_uiSpeechTimer = 5 * IN_MILLISECONDS; break;
                        case 4: DoScriptText(SAY_SPEECH_4, m_creature); m_uiPhase = PHASE_BALCONY; break;
                    }
                    m_uiSpeech++;
                }
                else
                    m_uiSpeechTimer -= uiDiff;

                // No break here

            case PHASE_BALCONY:                            // Do summoning
                if (m_uiTraineeTimer < uiDiff)
                {
                    SummonAdds(true, NPC_UNREL_TRAINEE);
                    m_uiTraineeTimer = 20 * IN_MILLISECONDS;
                }
                else
                    m_uiTraineeTimer -= uiDiff;
                if (m_uiDeathKnightTimer < uiDiff)
                {
                    SummonAdds(true, NPC_UNREL_DEATH_KNIGHT);
                    m_uiDeathKnightTimer = 25 * IN_MILLISECONDS;
                }
                else
                    m_uiDeathKnightTimer -= uiDiff;
                if (m_uiRiderTimer < uiDiff)
                {
                    SummonAdds(true, NPC_UNREL_RIDER);
                    m_uiRiderTimer = 30 * IN_MILLISECONDS;
                }
                else
                    m_uiRiderTimer -= uiDiff;

                if (m_uiPhaseTimer < uiDiff)
                {
                    m_uiPhase = PHASE_STOP_SUMMONING;
                    m_uiPhaseTimer = 27 * IN_MILLISECONDS;
                }
                else
                    m_uiPhaseTimer -= uiDiff;

                break;

            case PHASE_STOP_SUMMONING:
                if (m_uiPhaseTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_TELEPORT_RIGHT, CAST_TRIGGERED) == CAST_OK)
                    {
                        m_uiPhase = m_pInstance ? PHASE_TELEPORTING : PHASE_STOP_TELEPORTING;

                        DoScriptText(SAY_TELEPORT, m_creature);
                        DoScriptText(EMOTE_TO_FRAY, m_creature);

                        // Remove Immunity
                        m_creature->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, false);

                        DoResetThreat();
                        m_creature->SetInCombatWithZone();
                    }
                }
                else
                    m_uiPhaseTimer -= uiDiff;

                break;

            case PHASE_TELEPORTING:                         // Phase is only reached if m_pInstance is valid
                if (m_uiTeleportTimer < uiDiff)
                {
                    uint32 uiTeleportSpell = m_pInstance->IsInRightSideGothArea(m_creature) ? SPELL_TELEPORT_LEFT : SPELL_TELEPORT_RIGHT;
                    if (DoCastSpellIfCan(m_creature, uiTeleportSpell) == CAST_OK)
                    {
                        m_uiTeleportTimer = 20 * IN_MILLISECONDS;
                        m_uiShadowboltTimer = 2 * IN_MILLISECONDS;
                    }
                }
                else
                    m_uiTeleportTimer -= uiDiff;

                if (m_creature->GetHealthPercent() <= 30.0f)
                {
                    m_uiPhase = PHASE_STOP_TELEPORTING;
                    ProcessCentralDoor();
                    // as the doors now open, recheck whether mobs are standing around
                    m_uiControlZoneTimer = 1;
                }
                // no break here

            case PHASE_STOP_TELEPORTING:
                if (m_uiHarvestSoulTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_HARVESTSOUL) == CAST_OK)
                        m_uiHarvestSoulTimer = 15 * IN_MILLISECONDS;
                }
                else
                    m_uiHarvestSoulTimer -= uiDiff;

                if (m_uiShadowboltTimer)
                {
                    if (m_uiShadowboltTimer <= uiDiff)
                        m_uiShadowboltTimer = 0;
                    else
                        m_uiShadowboltTimer -= uiDiff;
                }
                // Shadowbold cooldown finished, cast when ready
                else if (!m_creature->IsNonMeleeSpellCasted(true))
                {
                    // Select valid target
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_TOPAGGRO, 0, SPELL_SHADOWBOLT, SELECT_FLAG_IN_LOS))
                        DoCastSpellIfCan(pTarget, SPELL_SHADOWBOLT);
                }

                break;
        }

        // Control Check, if Death zone empty
        if (m_uiControlZoneTimer)
        {
            if (m_uiControlZoneTimer <= uiDiff)
            {
                m_uiControlZoneTimer = 0;

                if (m_pInstance && !HasPlayersInLeftSide())
                {
                    ProcessCentralDoor();
                    for (GuidList::const_iterator itr = m_lSummonedAddGuids.begin(); itr != m_lSummonedAddGuids.end(); itr++)
                    {
                        if (Creature* pCreature = m_pInstance->instance->GetCreature(*itr))
                        {
                            if (!pCreature->isInCombat())
                                pCreature->SetInCombatWithZone();
                        }
                    }
                }
            }
            else
                m_uiControlZoneTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_boss_gothik(Creature* pCreature)
{
    return new boss_gothikAI(pCreature);
}

bool EffectDummyCreature_spell_anchor(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiEffIndex != EFFECT_INDEX_0 || pCreatureTarget->GetEntry() != NPC_SUB_BOSS_TRIGGER)
        return true;

    instance_naxxramas* pInstance = (instance_naxxramas*)pCreatureTarget->GetInstanceData();

    if (!pInstance)
        return true;

    switch (uiSpellId)
    {
        case SPELL_A_TO_ANCHOR_1:                           // trigger mobs at high right side
        case SPELL_B_TO_ANCHOR_1:
        case SPELL_C_TO_ANCHOR_1:
        {
            if (Creature* pAnchor2 = pInstance->GetClosestAnchorForGoth(pCreatureTarget, false))
            {
                uint32 uiTriggered = SPELL_A_TO_ANCHOR_2;

                if (uiSpellId == SPELL_B_TO_ANCHOR_1)
                    uiTriggered = SPELL_B_TO_ANCHOR_2;
                else if (uiSpellId == SPELL_C_TO_ANCHOR_1)
                    uiTriggered = SPELL_C_TO_ANCHOR_2;

                pCreatureTarget->CastSpell(pAnchor2, uiTriggered, true);
            }

            return true;
        }
        case SPELL_A_TO_ANCHOR_2:                           // trigger mobs at high left side
        case SPELL_B_TO_ANCHOR_2:
        case SPELL_C_TO_ANCHOR_2:
        {
            std::list<Creature*> lTargets;
            pInstance->GetGothSummonPointCreatures(lTargets, false);

            if (!lTargets.empty())
            {
                std::list<Creature*>::iterator itr = lTargets.begin();
                uint32 uiPosition = urand(0, lTargets.size() - 1);
                advance(itr, uiPosition);

                if (Creature* pTarget = (*itr))
                {
                    uint32 uiTriggered = SPELL_A_TO_SKULL;

                    if (uiSpellId == SPELL_B_TO_ANCHOR_2)
                        uiTriggered = SPELL_B_TO_SKULL;
                    else if (uiSpellId == SPELL_C_TO_ANCHOR_2)
                        uiTriggered = SPELL_C_TO_SKULL;

                    pCreatureTarget->CastSpell(pTarget, uiTriggered, true);
                }
            }
            return true;
        }
        case SPELL_A_TO_SKULL:                              // final destination trigger mob
        case SPELL_B_TO_SKULL:
        case SPELL_C_TO_SKULL:
        {
            if (Creature* pGoth = pInstance->GetSingleCreatureFromStorage(NPC_GOTHIK))
            {
                uint32 uiNpcEntry = NPC_SPECT_TRAINEE;

                if (uiSpellId == SPELL_B_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_DEATH_KNIGHT;
                else if (uiSpellId == SPELL_C_TO_SKULL)
                    uiNpcEntry = NPC_SPECT_RIDER;

                pGoth->SummonCreature(uiNpcEntry, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);

                if (uiNpcEntry == NPC_SPECT_RIDER)
                    pGoth->SummonCreature(NPC_SPECT_HORSE, pCreatureTarget->GetPositionX(), pCreatureTarget->GetPositionY(), pCreatureTarget->GetPositionZ(), pCreatureTarget->GetOrientation(), TEMPSUMMON_DEAD_DESPAWN, 0);
            }
            return true;
        }
    }

    return true;
};

void AddSC_boss_gothik()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_gothik";
    pNewScript->GetAI = &GetAI_boss_gothik;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "spell_anchor";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_anchor;
    pNewScript->RegisterSelf();
}
