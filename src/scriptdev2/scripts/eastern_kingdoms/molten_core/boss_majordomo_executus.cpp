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
SDName: Boss_Majordomo_Executus
SD%Complete: 95
SDComment: Minor weaknesses
SDCategory: Molten Core
EndScriptData */

#include "precompiled.h"
#include "molten_core.h"
#include "TemporarySummon.h"

enum
{
    SAY_AGGRO               = -1409003,
    SAY_SLAY                = -1409005,
    SAY_SPECIAL             = -1409006,                     // Use unknown
    SAY_LAST_ADD            = -1409019,                     // When only one add remaining
    SAY_DEFEAT_1            = -1409007,
    SAY_DEFEAT_2            = -1409020,
    SAY_DEFEAT_3            = -1409021,

    SAY_SUMMON_0            = -1409023,
    SAY_SUMMON_1            = -1409024,
    SAY_SUMMON_MAJ          = -1409008,
    SAY_ARRIVAL1_RAG        = -1409009,
    SAY_ARRIVAL2_MAJ        = -1409010,
    SAY_ARRIVAL3_RAG        = -1409011,
    SAY_ARRIVAL4_MAJ        = -1409022,

    GOSSIP_ITEM_SUMMON_1    = -3409000,
    GOSSIP_ITEM_SUMMON_2    = -3409001,
    GOSSIP_ITEM_SUMMON_3    = -3409002,

    TEXT_ID_SUMMON_1        = 4995,
    TEXT_ID_SUMMON_2        = 5011,
    TEXT_ID_SUMMON_3        = 5012,

    SPELL_MAGIC_REFLECTION  = 20619,
    SPELL_DAMAGE_REFLECTION = 21075,
    SPELL_BLASTWAVE         = 20229,
    SPELL_AEGIS             = 20620,
    SPELL_TELEPORT          = 20618,

    SPELL_TELEPORT_SELF     = 19484,
    SPELL_SUMMON_RAGNAROS   = 19774,
    SPELL_ELEMENTAL_FIRE    = 19773,
    SPELL_RAGNA_EMERGE      = 20568,
};

struct boss_majordomoAI : public ScriptedAI
{
    boss_majordomoAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_pInstance = (instance_molten_core*)pCreature->GetInstanceData();
        m_bHasEncounterFinished = false;
        Reset();
    }

    instance_molten_core* m_pInstance;

    uint32 m_uiMagicReflectionTimer;
    uint32 m_uiDamageReflectionTimer;
    uint32 m_uiBlastwaveTimer;
    uint32 m_uiTeleportTimer;
    uint32 m_uiAegisTimer;
    uint32 m_uiSpeechTimer;

    ObjectGuid m_ragnarosGuid;
    bool m_bHasEncounterFinished;
    uint8 m_uiAddsKilled;
    uint8 m_uiSpeech;
    GuidList m_luiMajordomoAddsGUIDs;

    void Reset() override
    {
        m_uiMagicReflectionTimer  = 30000;                  // Damage reflection first so we alternate
        m_uiDamageReflectionTimer = 15000;
        m_uiBlastwaveTimer = 10000;
        m_uiTeleportTimer = 20000;
        m_uiAegisTimer = 5000;
        m_uiSpeechTimer = 1000;

        m_uiAddsKilled = 0;
        m_uiSpeech = 0;
    }

    void KilledUnit(Unit* /*pVictim*/) override
    {
        if (urand(0, 4))
            return;

        DoScriptText(SAY_SLAY, m_creature);
    }

    void Aggro(Unit* pWho) override
    {
        if (pWho->GetTypeId() == TYPEID_UNIT && pWho->GetEntry() == NPC_RAGNAROS)
            return;

        DoScriptText(SAY_AGGRO, m_creature);

        if (m_pInstance)
            m_pInstance->SetData(TYPE_MAJORDOMO, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (!m_bHasEncounterFinished)                       // Normal reached home, FAIL
        {
            if (m_pInstance)
                m_pInstance->SetData(TYPE_MAJORDOMO, FAIL);
        }
        else                                                // Finished the encounter, DONE
        {
            // Exit combat
            m_creature->RemoveAllAurasOnEvade();
            m_creature->DeleteThreatList();
            m_creature->CombatStop(true);
            m_creature->SetLootRecipient(NULL);

            // Set friendly
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);
            m_creature->SetFactionTemporary(FACTION_MAJORDOMO_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN);

            // Reset orientation
            m_creature->SetFacingTo(m_aMajordomoLocations[0].m_fO);

            // Start his speech
            m_uiSpeechTimer = 1;                        // At next tick
            m_uiSpeech = 1;

            m_pInstance->SetData(TYPE_MAJORDOMO, DONE);
        }
    }

    void StartSummonEvent(Player* pPlayer)
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        // Prevent possible exploits with double summoning
        if (m_creature->GetMap()->GetCreature(m_ragnarosGuid))
            return;

        DoScriptText(SAY_SUMMON_0, m_creature, pPlayer);

        m_uiSpeechTimer = 5000;
        m_uiSpeech = 10;
    }

    void JustRespawned() override
    {
        // Encounter finished, need special treatment
        if (m_bHasEncounterFinished)
        {
            // This needs to be set to be able to resummon Ragnaros
            m_creature->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

            // Relocate here
            debug_log("SD2: boss_majordomo_executus: Relocate to Ragnaros' Lair on respawn");
            m_creature->GetMap()->CreatureRelocation(m_creature, m_aMajordomoLocations[1].m_fX, m_aMajordomoLocations[1].m_fY, m_aMajordomoLocations[1].m_fZ, m_aMajordomoLocations[1].m_fO);
            m_creature->SetActiveObjectState(false);
        }
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_FLAMEWAKER_HEALER || pSummoned->GetEntry() == NPC_FLAMEWAKER_ELITE)
        {
            m_luiMajordomoAddsGUIDs.push_back(pSummoned->GetObjectGuid());
            pSummoned->SetRespawnDelay(2 * HOUR);
        }
        else if (pSummoned->GetEntry() == NPC_RAGNAROS)
        {
            m_ragnarosGuid = pSummoned->GetObjectGuid();
            pSummoned->CastSpell(pSummoned, SPELL_RAGNA_EMERGE, false);
        }
    }

    void JustDied(Unit* pKiller) override
    {
        if (pKiller->GetTypeId() == TYPEID_UNIT && pKiller->GetEntry() == NPC_RAGNAROS)
            DoScriptText(SAY_ARRIVAL4_MAJ, m_creature);
    }

    void CorpseRemoved(uint32& uiRespawnDelay) override
    {
        uiRespawnDelay = urand(2 * HOUR, 3 * HOUR);

        if (m_bHasEncounterFinished)
        {
            // Needed for proper respawn handling
            debug_log("SD2: boss_majordomo_executus: Set active");
            m_creature->SetActiveObjectState(true);
        }
    }

    void SummonedCreatureJustDied(Creature* pSummoned) override
    {
        if (pSummoned->GetEntry() == NPC_FLAMEWAKER_HEALER || pSummoned->GetEntry() == NPC_FLAMEWAKER_ELITE)
        {
            m_uiAddsKilled += 1;

            // Yell if only one Add alive
            if (m_uiAddsKilled == m_luiMajordomoAddsGUIDs.size() - 1)
                DoScriptText(SAY_LAST_ADD, m_creature);

            // All adds are killed, retreat
            else if (m_uiAddsKilled == m_luiMajordomoAddsGUIDs.size())
            {
                m_bHasEncounterFinished = true;
                m_creature->GetMotionMaster()->MoveTargetedHome();
            }
        }
    }

    // Unsummon Majordomo adds
    void UnsummonMajordomoAdds()
    {
        for (GuidList::const_iterator itr = m_luiMajordomoAddsGUIDs.begin(); itr != m_luiMajordomoAddsGUIDs.end(); ++itr)
        {
            if (Creature* pAdd = m_creature->GetMap()->GetCreature(*itr))
                if (pAdd->IsTemporarySummon())
                    ((TemporarySummon*)pAdd)->UnSummon();
        }

        m_luiMajordomoAddsGUIDs.clear();
    }

    void DamageTaken(Unit* /*pDealer*/, uint32& uiDamage) override
    {
        if (uiDamage > m_creature->GetHealth())
        {
            uiDamage = 0;
            DoCastSpellIfCan(m_creature, SPELL_AEGIS, CAST_TRIGGERED);
        }
    }

    void UpdateAI(const uint32 uiDiff) override
    {
        // Handling of his combat-end speech and Ragnaros summoning
        if (m_uiSpeech)
        {
            if (m_uiSpeechTimer < uiDiff)
            {
                switch (m_uiSpeech)
                {
                        // Majordomo retreat event
                    case 1:
                        DoScriptText(SAY_DEFEAT_1, m_creature);
                        m_uiSpeechTimer = 7500;
                        ++m_uiSpeech;
                        break;
                    case 2:
                        DoScriptText(SAY_DEFEAT_2, m_creature);
                        m_uiSpeechTimer = 8000;
                        ++m_uiSpeech;
                        break;
                    case 3:
                        DoScriptText(SAY_DEFEAT_3, m_creature);
                        m_uiSpeechTimer = 21500;
                        ++m_uiSpeech;
                        break;
                    case 4:
                        DoCastSpellIfCan(m_creature, SPELL_TELEPORT_SELF);
                        // TODO - when should they be unsummoned?
                        // TODO - also unclear how this should be handled, as of range issues
                        m_uiSpeechTimer = 900;
                        ++m_uiSpeech;
                        break;
                    case 5:
                        // Majordomo is away now, remove his adds
                        UnsummonMajordomoAdds();
                        m_uiSpeech = 0;
                        break;

                        // Ragnaros Summon Event
                    case 10:
                        DoScriptText(SAY_SUMMON_1, m_creature);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 1000;
                        break;
                    case 11:
                        DoCastSpellIfCan(m_creature, SPELL_SUMMON_RAGNAROS);
                        // TODO - Move along, this expects to be handled with mmaps
                        m_creature->GetMotionMaster()->MovePoint(1, 831.079590f, -816.023193f, -229.023270f);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 7000;
                        break;
                    case 12:
                        // Reset orientation
                        if (GameObject* pLavaSteam = m_pInstance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                            m_creature->SetFacingToObject(pLavaSteam);
                        m_uiSpeechTimer = 4500;
                        ++m_uiSpeech;
                        break;
                    case 13:
                        DoScriptText(SAY_SUMMON_MAJ, m_creature);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 8000;
                        break;
                    case 14:
                        // Summon Ragnaros
                        if (m_pInstance)
                            if (GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                                m_creature->SummonCreature(NPC_RAGNAROS, pGo->GetPositionX(), pGo->GetPositionY(), pGo->GetPositionZ(), fmod(m_creature->GetOrientation() + M_PI, 2 * M_PI), TEMPSUMMON_TIMED_OOC_OR_DEAD_DESPAWN, 2 * HOUR * IN_MILLISECONDS);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 8700;
                        break;
                    case 15:
                        if (Creature* pRagnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                            DoScriptText(SAY_ARRIVAL1_RAG, pRagnaros);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 11700;
                        break;
                    case 16:
                        DoScriptText(SAY_ARRIVAL2_MAJ, m_creature);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 8700;
                        break;
                    case 17:
                        if (Creature* pRagnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                            DoScriptText(SAY_ARRIVAL3_RAG, pRagnaros);
                        ++m_uiSpeech;
                        m_uiSpeechTimer = 16500;
                        break;
                    case 18:
                        if (Creature* pRagnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                            pRagnaros->CastSpell(m_creature, SPELL_ELEMENTAL_FIRE, false);
                        // Rest of summoning speech is handled by Ragnaros, as Majordomo will be dead
                        m_uiSpeech = 0;
                        break;
                }
            }
            else
                m_uiSpeechTimer -= uiDiff;
        }

        // When encounter finished, no need to do anything anymore (important for moving home after victory)
        if (m_bHasEncounterFinished)
            return;

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        // Cast Ageis to heal self
        if (m_uiAegisTimer <= uiDiff)
            m_uiAegisTimer = 0;
        else
            m_uiAegisTimer -= uiDiff;

        if (m_creature->GetHealthPercent() < 90.0f && !m_uiAegisTimer)
        {
            DoCastSpellIfCan(m_creature, SPELL_AEGIS);
            m_uiAegisTimer = 10000;
        }

        // Magic Reflection Timer
        if (m_uiMagicReflectionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_MAGIC_REFLECTION) == CAST_OK)
                m_uiMagicReflectionTimer = 30000;
        }
        else
            m_uiMagicReflectionTimer -= uiDiff;

        // Damage Reflection Timer
        if (m_uiDamageReflectionTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_DAMAGE_REFLECTION) == CAST_OK)
                m_uiDamageReflectionTimer = 30000;
        }
        else
            m_uiDamageReflectionTimer -= uiDiff;

        // Teleports the target to the heated rock in the center of the area
        if (m_uiTeleportTimer < uiDiff)
        {
            if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 1))
            {
                if (DoCastSpellIfCan(pTarget, SPELL_TELEPORT) == CAST_OK)
                    m_uiTeleportTimer = 20000;
            }
        }
        else
            m_uiTeleportTimer -= uiDiff;

        // Blastwave Timer
        if (m_uiBlastwaveTimer < uiDiff)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_BLASTWAVE) == CAST_OK)
                m_uiBlastwaveTimer = 10000;
        }
        else
            m_uiBlastwaveTimer -= uiDiff;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_boss_majordomo(Creature* pCreature)
{
    return new boss_majordomoAI(pCreature);
}

bool GossipHello_boss_majordomo(Player* pPlayer, Creature* pCreature)
{
    if (instance_molten_core* pInstance = (instance_molten_core*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_RAGNAROS) == NOT_STARTED || pInstance->GetData(TYPE_RAGNAROS) == FAIL)
        {
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            pPlayer->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_1, pCreature->GetObjectGuid());
        }
    }
    return true;
}

bool GossipSelect_boss_majordomo(Player* pPlayer, Creature* pCreature, uint32 /*sender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_2, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_3, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (boss_majordomoAI* pMajoAI = dynamic_cast<boss_majordomoAI*>(pCreature->AI()))
                pMajoAI->StartSummonEvent(pPlayer);
            break;
    }

    return true;
}

bool EffectDummyCreature_spell_boss_majordomo(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId != SPELL_TELEPORT_SELF || uiEffIndex != EFFECT_INDEX_0)
        return false;

    pCreatureTarget->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
    pCreatureTarget->NearTeleportTo(m_aMajordomoLocations[1].m_fX, m_aMajordomoLocations[1].m_fY, m_aMajordomoLocations[1].m_fZ, m_aMajordomoLocations[1].m_fO, true);
    // TODO - some visibility update?
    return true;
}

void AddSC_boss_majordomo()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "boss_majordomo";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_boss_majordomo;
    pNewScript->pGossipHello = &GossipHello_boss_majordomo;
    pNewScript->pGossipSelect = &GossipSelect_boss_majordomo;
    pNewScript->GetAI = &GetAI_boss_majordomo;
    pNewScript->RegisterSelf();
}
