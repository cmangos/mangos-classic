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
SD%Complete: 100
SDComment:
SDCategory: Molten Core
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "molten_core.h"
#include "Entities/TemporarySpawn.h"
#include "AI/ScriptDevAI/base/CombatAI.h"

enum
{
    // Majordomo Executus Encounter
    SAY_AGGRO                = -1409003,
    SAY_SLAY_1               = 9425,
    SAY_SLAY_SOUND           = 8036,
    SAY_LAST_ADD             = -1409019,                    // When only one add remaining
    SAY_DEFEAT_1             = -1409007,
    SAY_DEFEAT_2             = -1409020,
    SAY_DEFEAT_3             = -1409021,

    SPELL_MAGIC_REFLECTION   = 20619,
    SPELL_DAMAGE_REFLECTION  = 21075,
    SPELL_AEGIS              = 20620,
    SPELL_TELEPORT_RANDOM    = 20618,                       // Teleport random target
    SPELL_TELEPORT_TARGET    = 20534,                       // Teleport Victim
    SPELL_HATE_TO_ZERO       = 20538,                       // Threat reset after each teleport
    SPELL_IMMUNE_POLY        = 21087,                       // Cast onto Flamewaker Healers when half the adds are dead
    SPELL_SEPARATION_ANXIETY = 21094,                       // Aura cast on himself by Majordomo Executus, if adds move out of range, they will cast spell 21095 on themselves
    SPELL_ENCOURAGEMENT      = 21086,                       // Cast onto all remaining adds every time one is killed
    SPELL_CHAMPION           = 21090,                       // Cast onto the last remaining add

    // Ragnaros summoning event
    GOSSIP_ITEM_SUMMON_1     = -3409000,
    GOSSIP_ITEM_SUMMON_2     = -3409001,
    GOSSIP_ITEM_SUMMON_3     = -3409002,

    SAY_SUMMON_0             = -1409023,
    SAY_SUMMON_1             = -1409024,
    SAY_SUMMON_MAJ           = -1409008,
    SAY_ARRIVAL1_RAG         = -1409009,
    SAY_ARRIVAL2_MAJ         = -1409010,
    SAY_ARRIVAL3_RAG         = -1409011,
    SAY_ARRIVAL4_MAJ         = -1409022,

    TEXT_ID_SUMMON_1         = 4995,
    TEXT_ID_SUMMON_2         = 5011,
    TEXT_ID_SUMMON_3         = 5012,

    SPELL_TELEPORT_SELF      = 19484,
    SPELL_SUMMON_RAGNAROS    = 19774,
    SPELL_ELEMENTAL_FIRE     = 19773,
    SPELL_RAGNA_EMERGE       = 20568,
};

enum MajordomoActions
{
    MAJORDOMO_REFLECTION_SHIELD,
    MAJORDOMO_TELEPORT_RANDOM,
    MAJORDOMO_TELEPORT_TARGET,
    MAJORDOMO_AEGIS,
    MAJORDOMO_ACTION_MAX,
    MAJORDOMO_OUTRO,
};

struct boss_majordomoAI : public CombatAI
{
    boss_majordomoAI(Creature* creature) : CombatAI(creature, MAJORDOMO_ACTION_MAX), m_instance(static_cast<instance_molten_core*>(creature->GetInstanceData()))
    {
        m_bHasEncounterFinished = false;
        AddCombatAction(MAJORDOMO_REFLECTION_SHIELD, 15000u);
        AddCombatAction(MAJORDOMO_TELEPORT_RANDOM, 15000u);
        AddCombatAction(MAJORDOMO_TELEPORT_TARGET, 30000u);
        AddCombatAction(MAJORDOMO_AEGIS, 5000u);
        AddCustomAction(MAJORDOMO_OUTRO, true, [&]() { HandleOutro(); });
        SetDeathPrevention(true);
        AddOnKillText(SAY_SLAY_1);
        AddOnKillSound(SAY_SLAY_SOUND);
        Reset();
    }

    instance_molten_core* m_instance;

    ObjectGuid m_ragnarosGuid;
    bool m_bHasEncounterFinished;
    uint8 m_addsKilled;
    uint8 m_speechStage;
    GuidList m_luiMajordomoAddsGUIDs;

    void Reset() override
    {
        m_addsKilled = 0;
        m_speechStage = 0;
    }

    void Aggro(Unit* who) override
    {
        if (who->GetTypeId() == TYPEID_UNIT && who->GetEntry() == NPC_RAGNAROS)
            return;

        DoScriptText(SAY_AGGRO, m_creature);
        // Majordomo Executus has a 100 yard aura to keep track of the distance of each of his adds, the Flamewaker Healers will enrage if moved out of it
        DoCastSpellIfCan(nullptr, SPELL_SEPARATION_ANXIETY, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        DoCastSpellIfCan(nullptr, SPELL_AEGIS, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);

        if (m_instance)
            m_instance->SetData(TYPE_MAJORDOMO, IN_PROGRESS);
    }

    void JustReachedHome() override
    {
        if (!m_bHasEncounterFinished)                       // Normal reached home, FAIL
        {
            if (m_instance)
                m_instance->SetData(TYPE_MAJORDOMO, FAIL);
        }
        else                                                // Finished the encounter, DONE
        {
            // Exit combat
            SetCombatScriptStatus(false);
            m_creature->RemoveAllAurasOnEvade();
            m_creature->CombatStop(true);
            m_creature->SetLootRecipient(nullptr);

            // Set friendly
            m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PLAYER);
            m_creature->SetFactionTemporary(FACTION_MAJORDOMO_FRIENDLY, TEMPFACTION_RESTORE_RESPAWN);

            // Reset orientation
            m_creature->SetFacingTo(m_aMajordomoLocations[0].m_fO);

            // Start his speech
            ResetTimer(MAJORDOMO_OUTRO, 0); // at next tick
            m_speechStage = 1;

            m_instance->SetData(TYPE_MAJORDOMO, DONE);
        }
    }

    void StartSummonEvent(Player* player)
    {
        m_creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        // Prevent possible exploits with double summoning
        if (m_creature->GetMap()->GetCreature(m_ragnarosGuid))
            return;

        DoScriptText(SAY_SUMMON_0, m_creature, player);

        ResetTimer(MAJORDOMO_OUTRO, 5000);
        m_speechStage = 10;
    }

    void JustRespawned() override
    {
        // Encounter finished, need special treatment
        if (m_bHasEncounterFinished)
        {
            // Relocate here
            debug_log("SD2: boss_majordomo_executus: Relocate to Ragnaros' Lair on respawn");
            m_creature->CastSpell(nullptr, SPELL_TELEPORT_SELF, TRIGGERED_OLD_TRIGGERED);
            m_creature->SetActiveObjectState(false);
        }
    }

    void JustSummoned(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_FLAMEWAKER_HEALER || summoned->GetEntry() == NPC_FLAMEWAKER_ELITE)
        {
            m_luiMajordomoAddsGUIDs.push_back(summoned->GetObjectGuid());
            summoned->SetRespawnDelay(2 * HOUR);
        }
        else if (summoned->GetEntry() == NPC_RAGNAROS)
        {
            m_ragnarosGuid = summoned->GetObjectGuid();
            summoned->CastSpell(summoned, SPELL_RAGNA_EMERGE, TRIGGERED_NONE);
            if (GameObject* lavaSplash = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_SPLASH))
                lavaSplash->SetLootState(GO_JUST_DEACTIVATED);
            if (GameObject* lavaSteam = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                lavaSteam->SetLootState(GO_JUST_DEACTIVATED);
        }
    }

    void CorpseRemoved(uint32& respawnDelay) override
    {
        respawnDelay = urand(2 * HOUR, 3 * HOUR);

        if (m_bHasEncounterFinished)
        {
            // Needed for proper respawn handling
            debug_log("SD2: boss_majordomo_executus: Set active");
            m_creature->SetActiveObjectState(true);
        }
    }

    void SummonedCreatureJustDied(Creature* summoned) override
    {
        if (summoned->GetEntry() == NPC_FLAMEWAKER_HEALER || summoned->GetEntry() == NPC_FLAMEWAKER_ELITE)
        {
            m_addsKilled += 1;

            // Yell if only one add is alive and buff it
            if (m_addsKilled == m_luiMajordomoAddsGUIDs.size() - 1)
            {
                DoScriptText(SAY_LAST_ADD, m_creature);
                DoCastSpellIfCan(nullptr, SPELL_CHAMPION);
            }
            // All adds are killed, retreat
            else if (m_addsKilled == m_luiMajordomoAddsGUIDs.size())
            {
                m_bHasEncounterFinished = true;
                m_creature->GetMotionMaster()->MoveTargetedHome();
                SetCombatScriptStatus(true);
                return;
            }

            // If 4 adds (half of them) are dead, make all remaining healers immune to polymorph via aura
            if (m_addsKilled >= MAX_MAJORDOMO_ADDS / 2)
                DoCastSpellIfCan(nullptr, SPELL_IMMUNE_POLY);

            // Buff the remaining adds
            DoCastSpellIfCan(nullptr, SPELL_ENCOURAGEMENT);
        }
    }

    // Unsummon Majordomo adds
    void UnsummonMajordomoAdds()
    {
        for (GuidList::const_iterator itr = m_luiMajordomoAddsGUIDs.begin(); itr != m_luiMajordomoAddsGUIDs.end(); ++itr)
        {
            if (Creature* add = m_creature->GetMap()->GetCreature(*itr))
                if (add->IsTemporarySummon())
                    static_cast<TemporarySpawn*>(add)->UnSummon();
        }

        m_luiMajordomoAddsGUIDs.clear();
    }

    void HandleOutro()
    {
        uint32 timer = 0;
        // Handling of his combat-end speech and Ragnaros summoning
        switch (m_speechStage)
        {
            // Majordomo retreat event
            case 1:
                DoScriptText(SAY_DEFEAT_1, m_creature);
                timer = 7500;
                ++m_speechStage;
                break;
            case 2:
                DoScriptText(SAY_DEFEAT_2, m_creature);
                timer = 8000;
                ++m_speechStage;
                break;
            case 3:
                DoScriptText(SAY_DEFEAT_3, m_creature);
                timer = 21500;
                ++m_speechStage;
                break;
            case 4:
                DoCastSpellIfCan(m_creature, SPELL_TELEPORT_SELF);
                // TODO - when should they be unsummoned?
                // TODO - also unclear how this should be handled, as of range issues
                timer = 900;
                ++m_speechStage;
                break;
            case 5:
                // Majordomo is away now, remove his adds
                UnsummonMajordomoAdds();
                m_speechStage = 0;
                break;

                // Ragnaros Summon Event
            case 10:
                DoScriptText(SAY_SUMMON_1, m_creature);
                if (GameObject* lavaSplash = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_SPLASH))
                {
                    lavaSplash->SetRespawnTime(900);
                    lavaSplash->Refresh();
                }
                if (GameObject* lavaSteam = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                {
                    lavaSteam->SetRespawnTime(900);
                    lavaSteam->Refresh();
                }
                ++m_speechStage;
                timer = 1000;
                break;
            case 11:
                DoCastSpellIfCan(nullptr, SPELL_SUMMON_RAGNAROS);
                // TODO - Move along, this expects to be handled with mmaps
                m_creature->GetMotionMaster()->MovePoint(1, 830.9636f, -814.7055f, -228.9733f);
                ++m_speechStage;
                timer = 11500;
                break;
            case 12:
                // Reset orientation
                if (GameObject* pLavaSteam = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                    m_creature->SetFacingToObject(pLavaSteam);
                DoScriptText(SAY_SUMMON_MAJ, m_creature);
                ++m_speechStage;
                timer = 8000;
                break;
            case 13:
                // Summon Ragnaros and make sure it faces Majordomo Executus
                if (m_instance)
                    if (GameObject* pGo = m_instance->GetSingleGameObjectFromStorage(GO_LAVA_STEAM))
                        m_creature->SummonCreature(NPC_RAGNAROS, 838.3082f, -831.4665f, -232.1853f, 2.199115f, TEMPSPAWN_TIMED_OOC_OR_DEAD_DESPAWN, 2 * HOUR * IN_MILLISECONDS);
                ++m_speechStage;
                timer = 8700;
                break;
            case 14:
                if (Creature* ragnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                {
                    ragnaros->HandleEmote(EMOTE_ONESHOT_ROAR);
                    DoScriptText(SAY_ARRIVAL1_RAG, ragnaros);
                }
                ++m_speechStage;
                timer = 11700;
                break;
            case 15:
                DoScriptText(SAY_ARRIVAL2_MAJ, m_creature);
                ++m_speechStage;
                timer = 8700;
                break;
            case 16:
                if (Creature* ragnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                    DoScriptText(SAY_ARRIVAL3_RAG, ragnaros);
                ++m_speechStage;
                timer = 16500;
                break;
            case 17:
                if (Creature* ragnaros = m_creature->GetMap()->GetCreature(m_ragnarosGuid))
                    ragnaros->CastSpell(m_creature, SPELL_ELEMENTAL_FIRE, TRIGGERED_NONE);
                // Rest of summoning speech is handled by Ragnaros, as Majordomo will be dead
                m_speechStage = 0;
                break;
        }
        if (timer)
            ResetTimer(MAJORDOMO_OUTRO, timer);
    }

    void ExecuteAction(uint32 action) override
    {
        switch (action)
        {
            case MAJORDOMO_REFLECTION_SHIELD:
            {
                if (DoCastSpellIfCan(nullptr, (urand(0, 1) ? SPELL_DAMAGE_REFLECTION : SPELL_MAGIC_REFLECTION)) == CAST_OK)
                    ResetCombatAction(action, 30000);
                break;
            }
            case MAJORDOMO_TELEPORT_RANDOM:
            {
                if (DoCastSpellIfCan(m_creature->GetVictim(), SPELL_TELEPORT_TARGET) == CAST_OK)
                {
                    DoCastSpellIfCan(nullptr, SPELL_HATE_TO_ZERO);
                    ResetCombatAction(action, urand(25000, 30000));
                }
                break;
            }
            case MAJORDOMO_TELEPORT_TARGET:
            {
                if (Unit* target = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                {
                    if (DoCastSpellIfCan(target, SPELL_TELEPORT_RANDOM) == CAST_OK)
                    {
                        DoCastSpellIfCan(nullptr, SPELL_HATE_TO_ZERO);
                        ResetCombatAction(action, urand(25000, 30000));
                    }
                }
                break;
            }
            case MAJORDOMO_AEGIS:
            {
                if (DoCastSpellIfCan(nullptr, SPELL_AEGIS, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT) == CAST_OK)
                    ResetCombatAction(action, urand(25000, 30000));
                break;
            }
        }
    }
};

UnitAI* GetAI_boss_majordomo(Creature* creature)
{
    return new boss_majordomoAI(creature);
}

bool GossipHello_boss_majordomo(Player* player, Creature* creature)
{
    if (instance_molten_core* pInstance = (instance_molten_core*)creature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_RAGNAROS) == NOT_STARTED || pInstance->GetData(TYPE_RAGNAROS) == FAIL)
        {
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_1, creature->GetObjectGuid());
        }
    }
    return true;
}

bool GossipSelect_boss_majordomo(Player* player, Creature* creature, uint32 /*sender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF + 1:
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            player->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_2, creature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 2:
            player->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_SUMMON_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            player->SEND_GOSSIP_MENU(TEXT_ID_SUMMON_3, creature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF + 3:
            player->CLOSE_GOSSIP_MENU();
            if (boss_majordomoAI* pMajoAI = dynamic_cast<boss_majordomoAI*>(creature->AI()))
                pMajoAI->StartSummonEvent(player);
            break;
    }

    return true;
}

void AddSC_boss_majordomo()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "boss_majordomo";
    pNewScript->pGossipHello = &GossipHello_boss_majordomo;
    pNewScript->pGossipSelect = &GossipSelect_boss_majordomo;
    pNewScript->GetAI = &GetAI_boss_majordomo;
    pNewScript->RegisterSelf();
}
