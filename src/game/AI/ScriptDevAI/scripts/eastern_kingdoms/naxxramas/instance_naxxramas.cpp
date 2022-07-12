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
SDName: Instance_Naxxramas
SD%Complete: 100
SDComment:
SDCategory: Naxxramas
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "naxxramas.h"

static const DialogueEntry naxxDialogue[] =
{
    {NPC_KELTHUZAD,         0,                10000},
    {SAY_SAPP_DIALOG1,      NPC_KELTHUZAD,    5000},
    {SAY_SAPP_DIALOG2_LICH, NPC_THE_LICHKING, 17000},
    {SAY_SAPP_DIALOG3,      NPC_KELTHUZAD,    6000},
    {SAY_SAPP_DIALOG4_LICH, NPC_THE_LICHKING, 8000},
    {SAY_SAPP_DIALOG5,      NPC_KELTHUZAD,    0},
    {NPC_THANE,             0,                10000},
    {SAY_KORT_TAUNT1,       NPC_THANE,        5000},
    {SAY_ZELI_TAUNT1,       NPC_ZELIEK,       6000},
    {SAY_BLAU_TAUNT1,       NPC_BLAUMEUX,     6000},
    {SAY_MORG_TAUNT1,       NPC_MOGRAINE,     7000},
    {SAY_BLAU_TAUNT2,       NPC_BLAUMEUX,     6000},
    {SAY_ZELI_TAUNT2,       NPC_ZELIEK,       5000},
    {SAY_KORT_TAUNT2,       NPC_THANE,        7000},
    {SAY_MORG_TAUNT2,       NPC_MOGRAINE,     0},
    {SAY_FAERLINA_INTRO,    NPC_FAERLINA,     10000},
    {FOLLOWERS_STAND,       0,                3000},
    {FOLLOWERS_AURA,        0,                30000},
    {FOLLOWERS_KNEEL,       0,                0},
    {0,                     0,                0}
};

instance_naxxramas::instance_naxxramas(Map* pMap) : ScriptedInstance(pMap),
    m_sapphironSpawnTimer(0),
    m_tauntTimer(0),
    m_horsemenKilled(0),
    m_horsemenTauntTimer(30 * MINUTE * IN_MILLISECONDS),
    m_livingPoisonTimer(0),
    m_despawnKTTriggerTimer(0),
    m_screamsTimer(2 * MINUTE * IN_MILLISECONDS),
    isFaerlinaIntroDone(false),
    m_shackledGuardians(0),
    m_checkGuardiansTimer(0),
    DialogueHelper(naxxDialogue)
{
    Initialize();
}

void instance_naxxramas::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    InitializeDialogueHelper(this);
}

void instance_naxxramas::JustDidDialogueStep(int32 entry)
{
    switch (entry)
    {
        case FOLLOWERS_STAND:
        {
            isFaerlinaIntroDone = true;
            for (auto& followerGuid : m_faerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->IsAlive() && !follower->IsInCombat())
                        follower->SetStandState(UNIT_STAND_STATE_STAND);
                }
            }
            break;
        }
        case FOLLOWERS_AURA:
        {
            for (auto& followerGuid : m_faerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->IsAlive() && !follower->IsInCombat())
                        follower->CastSpell(follower, SPELL_DARK_CHANNELING, TRIGGERED_OLD_TRIGGERED);
                }
            }
            break;
        }
        case FOLLOWERS_KNEEL:
        {
            for (auto& followerGuid : m_faerlinaFollowersList)
            {
                if (Creature* follower = instance->GetCreature(followerGuid))
                {
                    if (follower->IsAlive() && !follower->IsInCombat())
                    {
                        follower->RemoveAurasDueToSpell(SPELL_DARK_CHANNELING);
                        follower->SetStandState(UNIT_STAND_STATE_KNEEL);
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

void instance_naxxramas::OnPlayerEnter(Player* player)
{
    // Function only used to summon Sapphiron in case of server reload
    if (GetData(TYPE_SAPPHIRON) != SPECIAL)
        return;

    // Check if already summoned
    if (GetSingleCreatureFromStorage(NPC_SAPPHIRON, true))
        return;

    player->SummonCreature(NPC_SAPPHIRON, sapphironPositions[0], sapphironPositions[1], sapphironPositions[2], sapphironPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);
}

void instance_naxxramas::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_ANUB_REKHAN:
        case NPC_FAERLINA:
        case NPC_GLUTH:
        case NPC_THADDIUS:
        case NPC_STALAGG:
        case NPC_FEUGEN:
        case NPC_ZELIEK:
        case NPC_THANE:
        case NPC_BLAUMEUX:
        case NPC_MOGRAINE:
        case NPC_SPIRIT_OF_BLAUMEUX:
        case NPC_SPIRIT_OF_MOGRAINE:
        case NPC_SPIRIT_OF_KORTHAZZ:
        case NPC_SPIRIT_OF_ZELIREK:
        case NPC_GOTHIK:
        case NPC_SAPPHIRON:
        case NPC_KELTHUZAD:
        case NPC_THE_LICHKING:
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
        case NPC_NAXXRAMAS_TRIGGER:
        {
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            m_livingPoisonTimer = 5 * IN_MILLISECONDS;
            break;
        }
        case NPC_TOXIC_TUNNEL:
            creature->SetCanEnterCombat(false);
            break;
        case NPC_ZOMBIE_CHOW:
        {
            m_zombieChowList.push_back(creature->GetObjectGuid());
            break;
        }
        case NPC_NAXXRAMAS_CULTIST:
        case NPC_NAXXRAMAS_ACOLYTE:
        {
            m_faerlinaFollowersList.push_back(creature->GetObjectGuid());
            break;
        }
        case NPC_SUB_BOSS_TRIGGER:  m_gothikTriggerList.push_back(creature->GetObjectGuid()); break;
        case NPC_TESLA_COIL:        m_thaddiusTeslaCoilList.push_back(creature->GetObjectGuid()); break;
        case NPC_UNREL_TRAINEE:
        case NPC_UNREL_DEATH_KNIGHT:
        case NPC_UNREL_RIDER:
            m_unrelentingSideList.push_back(creature->GetObjectGuid());
            break;
        case NPC_SPECT_TRAINEE:
        case NPC_SPECT_DEATH_KNIGHT:
        case NPC_SPECT_RIDER:
        case NPC_SPECT_HORSE:
            m_spectralSideList.push_back(creature->GetObjectGuid());
            break;
        case NPC_SOUL_WEAVER:
        case NPC_UNSTOPPABLE_ABOM:
        case NPC_SOLDIER_FROZEN:
            if (creature->IsTemporarySummon())
            {
                if (creature->GetSpawnerGuid().IsCreature())
                {
                    if (Creature* summoner = creature->GetMap()->GetCreature(creature->GetSpawnerGuid()))
                    {
                        if (summoner->GetEntry() == NPC_KELTHUZAD)
                            return;
                    }
                }
            }
            break;
        case NPC_GUARDIAN:
            m_icrecrownGuardianList.push_back(creature->GetObjectGuid());
            break;
    }
}

void instance_naxxramas::OnObjectCreate(GameObject* gameObject)
{
    switch (gameObject->GetEntry())
    {
        // Arachnid Quarter
        case GO_ARAC_ANUB_DOOR:
            break;
        case GO_ARAC_ANUB_GATE:
            if (m_auiEncounter[TYPE_ANUB_REKHAN] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARAC_FAER_WEB:
            break;
        case GO_ARAC_FAER_DOOR:
            if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_ARAC_MAEX_INNER_DOOR:
            break;
        case GO_ARAC_MAEX_OUTER_DOOR:
            if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;

        // Plague Quarter
        case GO_PLAG_NOTH_ENTRY_DOOR:
            break;
        case GO_PLAG_NOTH_EXIT_DOOR:
        case GO_PLAG_HEIG_ENTRY_DOOR:
            if (m_auiEncounter[TYPE_NOTH] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_HEIG_EXIT_HALLWAY:
            if (m_auiEncounter[TYPE_HEIGAN] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_LOAT_DOOR:
            break;

        // Military Quarter
        case GO_MILI_GOTH_ENTRY_GATE:
            break;
        case GO_MILI_GOTH_EXIT_GATE:
            if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_MILI_GOTH_COMBAT_GATE:
            break;
        case GO_MILI_HORSEMEN_DOOR:
            if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CHEST_HORSEMEN_NORM:
            break;

        // Construct Quarter
        case GO_CONS_PATH_EXIT_DOOR:
            if (m_auiEncounter[TYPE_PATCHWERK] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_GLUT_EXIT_DOOR:
        case GO_CONS_THAD_DOOR:
            if (m_auiEncounter[TYPE_GLUTH] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_NOX_TESLA_FEUGEN:
        case GO_CONS_NOX_TESLA_STALAGG:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                gameObject->SetGoState(GO_STATE_READY);
            break;

        // Frostwyrm Lair
        case GO_KELTHUZAD_WATERFALL_DOOR:
            if (m_auiEncounter[TYPE_SAPPHIRON] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_KELTHUZAD_EXIT_DOOR:
        case GO_KELTHUZAD_WINDOW_1:
        case GO_KELTHUZAD_WINDOW_2:
        case GO_KELTHUZAD_WINDOW_3:
        case GO_KELTHUZAD_WINDOW_4:
            break;
        case GO_KELTHUZAD_TRIGGER:
            if (m_auiEncounter[TYPE_KELTHUZAD] != NOT_STARTED) // Only spawn the visual trigger for Kel'Thuzad when encounter is not started
                gameObject->SetLootState(GO_JUST_DEACTIVATED);
            break;

        // Eyes
        case GO_ARAC_EYE_RAMP:
        case GO_ARAC_EYE_BOSS:
            if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_PLAG_EYE_RAMP:
        case GO_PLAG_EYE_BOSS:
            if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_MILI_EYE_RAMP:
        case GO_MILI_EYE_BOSS:
            if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_CONS_EYE_RAMP:
        case GO_CONS_EYE_BOSS:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                gameObject->SetGoState(GO_STATE_ACTIVE);
            break;

        // Portals
        case GO_ARAC_PORTAL:
            if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_PLAG_PORTAL:
            if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_MILI_PORTAL:
            if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;
        case GO_CONS_PORTAL:
            if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                gameObject->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
            break;

        default:
            return;
    }
    m_goEntryGuidStore[gameObject->GetEntry()] = gameObject->GetObjectGuid();
}

void instance_naxxramas::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_MR_BIGGLESWORTH:
            if (m_auiEncounter[TYPE_KELTHUZAD] != DONE)
                DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_CAT_DIED, NPC_KELTHUZAD);
            break;
        case NPC_ZOMBIE_CHOW:
            creature->ForcedDespawn(2000);
            m_zombieChowList.remove(creature->GetObjectGuid());
            break;
        case NPC_UNREL_TRAINEE:
            if (Creature* anchor = GetClosestAnchorForGothik(creature, true))
                creature->CastSpell(anchor, SPELL_A_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, creature->GetObjectGuid());
            m_unrelentingSideList.remove(creature->GetObjectGuid());
            creature->ForcedDespawn(4000);
            break;
        case NPC_UNREL_DEATH_KNIGHT:
            if (Creature* anchor = GetClosestAnchorForGothik(creature, true))
                creature->CastSpell(anchor, SPELL_B_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, creature->GetObjectGuid());
            m_unrelentingSideList.remove(creature->GetObjectGuid());
            creature->ForcedDespawn(4000);
            break;
        case NPC_UNREL_RIDER:
            if (Creature* anchor = GetClosestAnchorForGothik(creature, true))
                creature->CastSpell(anchor, SPELL_C_TO_ANCHOR_1, TRIGGERED_OLD_TRIGGERED, nullptr, nullptr, creature->GetObjectGuid());
            m_unrelentingSideList.remove(creature->GetObjectGuid());
            creature->ForcedDespawn(4000);
            break;
        case NPC_SPECT_TRAINEE:
        case NPC_SPECT_DEATH_KNIGHT:
        case NPC_SPECT_RIDER:
        case NPC_SPECT_HORSE:
            m_spectralSideList.remove(creature->GetObjectGuid());
            creature->ForcedDespawn(4000);
            break;
        case NPC_SOLDIER_FROZEN:
        case NPC_UNSTOPPABLE_ABOM:
        case NPC_SOUL_WEAVER:
            creature->ForcedDespawn(2000);
            break;
        default:
            break;
    }
}

void instance_naxxramas::OnCreatureRespawn(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_SOUL_WEAVER:
        case NPC_UNSTOPPABLE_ABOM:
        case NPC_SOLDIER_FROZEN:
            if (!creature->IsTemporarySummon())
                break;
            [[fallthrough]];
        case NPC_ZOMBIE_CHOW:
        case NPC_GUARDIAN:
        case NPC_CORPSE_SCARAB:
            creature->SetInCombatWithZone();
            break;
    }
}

bool instance_naxxramas::IsEncounterInProgress() const
{
    for (uint8 i = 0; i <= TYPE_KELTHUZAD; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    }

    // Some Encounters use SPECIAL while in progress
    return m_auiEncounter[TYPE_GOTHIK] == SPECIAL;
}

void instance_naxxramas::SetData(uint32 type, uint32 data)
{
    switch (type)
    {
        case TYPE_ANUB_REKHAN:
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_ARAC_ANUB_DOOR);
            if (data == DONE)
                DoUseDoorOrButton(GO_ARAC_ANUB_GATE);
            break;
        case TYPE_FAERLINA:
            DoUseDoorOrButton(GO_ARAC_FAER_WEB);
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_ARAC_FAER_DOOR);
                DoUseDoorOrButton(GO_ARAC_MAEX_OUTER_DOOR);
            }
            m_auiEncounter[type] = data;
            break;
        case TYPE_MAEXXNA:
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_ARAC_MAEX_INNER_DOOR, data);
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_ARAC_EYE_RAMP);
                DoUseDoorOrButton(GO_ARAC_EYE_BOSS);
                DoRespawnGameObject(GO_ARAC_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_ARAC_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_tauntTimer = 5000;
            }
            break;
        case TYPE_NOTH:
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_PLAG_NOTH_ENTRY_DOOR);
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_PLAG_NOTH_EXIT_DOOR);
                DoUseDoorOrButton(GO_PLAG_HEIG_ENTRY_DOOR);
            }
            break;
        case TYPE_HEIGAN:
            m_auiEncounter[type] = data;
            // Open the entrance door on encounter win or failure (we specifically set the GOState to avoid issue in case encounter is reset before gate is closed in Heigan script)
            if (data == DONE || data == FAIL)
            {
                if (GameObject* door = GetSingleGameObjectFromStorage(GO_PLAG_HEIG_ENTRY_DOOR))
                    door->SetGoState(GO_STATE_ACTIVE);
            }
            if (data == DONE)
                DoUseDoorOrButton(GO_PLAG_HEIG_EXIT_HALLWAY);
            break;
        case TYPE_LOATHEB:
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_PLAG_LOAT_DOOR);
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_PLAG_EYE_RAMP);
                DoUseDoorOrButton(GO_PLAG_EYE_BOSS);
                DoRespawnGameObject(GO_PLAG_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_PLAG_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_tauntTimer = 5000;
            }
            break;
        case TYPE_RAZUVIOUS:
            m_auiEncounter[type] = data;
            break;
        case TYPE_GOTHIK:
            m_auiEncounter[type] = data;
            switch (data)
            {
                case IN_PROGRESS:
                    // Encounter begins: close the gate and start timer to summon unrelenting trainees
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    InitializeGothikTriggers();
                    break;
                case SPECIAL:
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    for (auto& spectralGuid : m_spectralSideList)
                    {
                        if (Creature* spectral = instance->GetCreature(spectralGuid))
                            spectral->CastSpell(spectral, SPELL_SPECTRAL_ASSAULT, TRIGGERED_OLD_TRIGGERED);
                    }
                    for (auto& unrelentingGuid : m_unrelentingSideList)
                    {
                        if (Creature* unrelenting = instance->GetCreature(unrelentingGuid))
                            unrelenting->CastSpell(unrelenting, SPELL_UNRELENTING_ASSAULT, TRIGGERED_OLD_TRIGGERED);
                    }
                    break;
                case FAIL:
                    if (m_auiEncounter[type] == IN_PROGRESS)
                        DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    break;
                case DONE:
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_EXIT_GATE);
                    DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);
                    // Open the central gate if Gothik is defeated before doing so
                    if (m_auiEncounter[type] == IN_PROGRESS)
                        DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    StartNextDialogueText(NPC_THANE);
                    break;
            }
            break;
        case TYPE_FOUR_HORSEMEN:
        {
            // Skip if already set
            if (m_auiEncounter[type] == data)
                return;
            if (data == SPECIAL)
            {
                ++m_horsemenKilled;

                if (m_horsemenKilled == 4)
                    SetData(TYPE_FOUR_HORSEMEN, DONE);

                // Don't store special data
                break;
            }
            if (data == FAIL)
            {
                m_horsemenKilled = 0;
                std::vector<uint32> entries = {NPC_BLAUMEUX, NPC_MOGRAINE, NPC_ZELIEK, NPC_THANE};
                for (uint32 entry : entries)
                {
                    if (Creature* creature = GetSingleCreatureFromStorage(entry))
                    {
                        if (creature->IsInCombat())
                            creature->AI()->EnterEvadeMode();
                    }
                }
            }
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);
            // Despawn Horsemen's spirit on FAIL or DONE
            std::vector<uint32> entries = {NPC_SPIRIT_OF_BLAUMEUX, NPC_SPIRIT_OF_MOGRAINE, NPC_SPIRIT_OF_KORTHAZZ, NPC_SPIRIT_OF_ZELIREK};
            for (uint32 entry : entries)
            {
                if (Creature* spirit = GetSingleCreatureFromStorage(entry))
                    spirit->ForcedDespawn();
            }
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_MILI_EYE_RAMP);
                DoUseDoorOrButton(GO_MILI_EYE_BOSS);
                DoRespawnGameObject(GO_MILI_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_MILI_PORTAL, GO_FLAG_NO_INTERACT, false);
                DoRespawnGameObject(GO_CHEST_HORSEMEN_NORM, 30 * MINUTE);
                m_tauntTimer = 5000;
            }
            break;
        }
        case TYPE_PATCHWERK:
            m_auiEncounter[type] = data;
            if (data == DONE)
                DoUseDoorOrButton(GO_CONS_PATH_EXIT_DOOR);
            break;
        case TYPE_GROBBULUS:
            m_auiEncounter[type] = data;
            break;
        case TYPE_GLUTH:
            m_auiEncounter[type] = data;
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_CONS_GLUT_EXIT_DOOR);
                DoUseDoorOrButton(GO_CONS_THAD_DOOR);
            }
            break;
        case TYPE_THADDIUS:
            // Only process real changes here
            if (m_auiEncounter[type] == data)
                return;

            m_auiEncounter[type] = data;
            if (data == FAIL)
            {
                // Reset stage for phase 1
                // Respawn: Stalagg, Feugen, their respective Tesla Coil NPCs and Tesla GOs
                if (Creature* stalagg = GetSingleCreatureFromStorage(NPC_STALAGG))
                {
                    stalagg->ForcedDespawn();
                    stalagg->Respawn();
                }

                if (Creature* feugen = GetSingleCreatureFromStorage(NPC_FEUGEN))
                {
                    feugen->ForcedDespawn();
                    feugen->Respawn();
                }

                for (auto& teslaGuid : m_thaddiusTeslaCoilList)
                {
                    if (Creature* teslaCoil = instance->GetCreature(teslaGuid))
                    {
                        teslaCoil->ForcedDespawn();
                        teslaCoil->Respawn();
                    }
                }
                if (GameObject* stalaggTesla = GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_STALAGG))
                    stalaggTesla->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* feugenTesla = GetSingleGameObjectFromStorage(GO_CONS_NOX_TESLA_FEUGEN))
                    feugenTesla->SetGoState(GO_STATE_ACTIVE);
            }
            if (data != SPECIAL)
                DoUseDoorOrButton(GO_CONS_THAD_DOOR, data);
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_CONS_EYE_RAMP);
                DoUseDoorOrButton(GO_CONS_EYE_BOSS);
                DoRespawnGameObject(GO_CONS_PORTAL, 30 * MINUTE);
                DoToggleGameObjectFlags(GO_CONS_PORTAL, GO_FLAG_NO_INTERACT, false);
                m_tauntTimer = 5000;
            }
            break;
        case TYPE_SAPPHIRON:
            m_auiEncounter[type] = data;
            if (data == DONE)
            {
                DoUseDoorOrButton(GO_KELTHUZAD_WATERFALL_DOOR);
                StartNextDialogueText(NPC_KELTHUZAD);
            }
            // Start Sapph summoning process
            if (data == SPECIAL)
                m_sapphironSpawnTimer = 22000;
            break;
        case TYPE_KELTHUZAD:
            m_auiEncounter[type] = data;
            DoUseDoorOrButton(GO_KELTHUZAD_EXIT_DOOR);
            if (data == IN_PROGRESS)
            {
                if (Creature* kelthuzad = GetSingleCreatureFromStorage(NPC_KELTHUZAD))
                {
                    if (kelthuzad->IsAlive())
                        kelthuzad->CastSpell(kelthuzad, SPELL_CHANNEL_VISUAL, TRIGGERED_OLD_TRIGGERED);
                }
                DoUseDoorOrButton(GO_KELTHUZAD_TRIGGER);
                m_despawnKTTriggerTimer = 5 * IN_MILLISECONDS;
            }
            if (data == FAIL)
            {
                if (GameObject* window = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_1))
                    window->ResetDoorOrButton();
                if (GameObject* window = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_2))
                    window->ResetDoorOrButton();
                if (GameObject* window = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_3))
                    window->ResetDoorOrButton();
                if (GameObject* window = GetSingleGameObjectFromStorage(GO_KELTHUZAD_WINDOW_4))
                    window->ResetDoorOrButton();
                if (GameObject* trigger = GetSingleGameObjectFromStorage(GO_KELTHUZAD_TRIGGER))
                {
                    trigger->SetRespawnTime(5);
                    trigger->Respawn();
                }
                // Clear everything related to Guardians of Icecrown
                m_icrecrownGuardianList.clear();
                m_checkGuardiansTimer = 0;
                m_shackledGuardians = 0;
            }
            if (data == DONE)
            {
                for (auto guardianGuid : m_icrecrownGuardianList)
                {
                    if (Creature* guardian = instance->GetCreature(guardianGuid))
                    {
                        if (guardian->AI())
                            guardian->AI()->EnterEvadeMode();
                        DoScriptText(EMOTE_FLEE, guardian);
                    }
                }
            }
            break;
    }

    if (data == DONE || (data == SPECIAL && type == TYPE_SAPPHIRON))
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                   << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                   << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                   << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                   << m_auiEncounter[12] << " " << m_auiEncounter[13] << " " << m_auiEncounter[14] << " " << m_auiEncounter[15];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_naxxramas::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
               >> m_auiEncounter[12] >> m_auiEncounter[13] >> m_auiEncounter[14] >> m_auiEncounter[15];

    for (uint32& i : m_auiEncounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_naxxramas::GetData(uint32 type) const
{
    if (type < MAX_ENCOUNTER)
        return m_auiEncounter[type];

    return 0;
}

void instance_naxxramas::Update(uint32 diff)
{
    // Handle the continuous spawning of Living Poison blobs in Patchwerk corridor
    if (m_livingPoisonTimer)
    {
        if (m_livingPoisonTimer <= diff)
        {
            if (Creature* trigger = GetSingleCreatureFromStorage(NPC_NAXXRAMAS_TRIGGER))
            {
                // Spawn 3 living poisons every 5 secs and make them cross the corridor and then despawn, for ever and ever
                for (uint8 i = 0; i < 3; i++)
                    if (Creature* poison = trigger->SummonCreature(NPC_LIVING_POISON, livingPoisonPositions[i].m_fX, livingPoisonPositions[i].m_fY, livingPoisonPositions[i].m_fZ, livingPoisonPositions[i].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                    {
                        poison->GetMotionMaster()->MovePoint(0, livingPoisonPositions[i + 3].m_fX, livingPoisonPositions[i + 3].m_fY, livingPoisonPositions[i + 3].m_fZ);
                        poison->ForcedDespawn(15000);
                    }
            }
            m_livingPoisonTimer = 5000;
        }
        else
            m_livingPoisonTimer -= diff;
    }

    if (m_despawnKTTriggerTimer)
    {
        if (m_despawnKTTriggerTimer < diff)
        {
            if (GameObject* trigger = GetSingleGameObjectFromStorage(GO_KELTHUZAD_TRIGGER))
            {
                trigger->ResetDoorOrButton();
                trigger->SetLootState(GO_JUST_DEACTIVATED);
                trigger->SetForcedDespawn();
            }
            m_despawnKTTriggerTimer = 0;
        }
        else
            m_despawnKTTriggerTimer -= diff;
    }

    if (m_screamsTimer && m_auiEncounter[TYPE_THADDIUS] != DONE)
    {
        if (m_screamsTimer <= diff)
        {
            if (Player* player = GetPlayerInMap())
                player->GetMap()->PlayDirectSoundToMap(SOUND_SCREAM1 + urand(0, 3));
            m_screamsTimer = (2 * MINUTE + urand(0, 30)) * IN_MILLISECONDS;
        }
        else
            m_screamsTimer -= diff;
    }

    if (m_tauntTimer)
    {
        if (m_tauntTimer <= diff)
        {
            DoTaunt();
            m_tauntTimer = 0;
        }
        else
            m_tauntTimer -= diff;
    }

    if (m_checkGuardiansTimer)
    {
        if (m_checkGuardiansTimer <= diff)
        {
            if (m_shackledGuardians > MAX_SHACKLES)
            {
                if (Creature* kelthuzad = GetSingleCreatureFromStorage(NPC_KELTHUZAD))
                {
                    DoScriptText((urand(0, 1) ? SAY_KELTHUZAD_SHACKLES_1: SAY_KELTHUZAD_SHACKLES_2), kelthuzad);
                    kelthuzad->CastSpell(nullptr, SPELL_CLEAR_ALL_SHACKLES, TRIGGERED_OLD_TRIGGERED);
                }
            }
            m_shackledGuardians = 0;
            m_checkGuardiansTimer = 2 * IN_MILLISECONDS;
        }
        else
            m_checkGuardiansTimer -= diff;
    }

    if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == NOT_STARTED)
    {
        if (m_horsemenTauntTimer <= diff)
        {
            uint32 horsemenEntry = 0;
            int32 textId = 0;
            switch (urand(0, 3))
            {
                case 0:
                    horsemenEntry = NPC_BLAUMEUX;
                    textId = SAY_BLAU_TAUNT3;
                    break;
                case 1:
                    horsemenEntry = NPC_THANE;
                    textId = SAY_KORT_TAUNT3;
                    break;
                case 2:
                    horsemenEntry = NPC_MOGRAINE;
                    textId = SAY_MORG_TAUNT3;
                    break;
                case 3:
                    horsemenEntry = NPC_ZELIEK;
                    textId = SAY_ZELI_TAUNT3;
                    break;
            }
            DoOrSimulateScriptTextForThisInstance(textId, horsemenEntry);
            m_horsemenTauntTimer = urand(30, 40) * MINUTE * IN_MILLISECONDS;
        }
        else
            m_horsemenTauntTimer -= diff;
    }

    if (m_sapphironSpawnTimer)
    {
        if (m_sapphironSpawnTimer <= diff)
        {
            if (Player* player = GetPlayerInMap())
                player->SummonCreature(NPC_SAPPHIRON, sapphironPositions[0], sapphironPositions[1], sapphironPositions[2], sapphironPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);

            m_sapphironSpawnTimer = 0;
        }
        else
            m_sapphironSpawnTimer -= diff;
    }

    DialogueUpdate(diff);
}

// Initialize all triggers used in Gothik the Harvester encounter by flagging them with their position in the room and what kind of NPC they will summon
void instance_naxxramas::InitializeGothikTriggers()
{
    Creature* gothik = GetSingleCreatureFromStorage(NPC_GOTHIK);

    if (!gothik)
        return;

    CreatureList summonList;

    for (auto triggerGuid : m_gothikTriggerList)
    {
        if (Creature* trigger = instance->GetCreature(triggerGuid))
        {
            GothTrigger gt;
            gt.isAnchorHigh = (trigger->GetPositionZ() >= (gothik->GetPositionZ() - 5.0f));
            gt.isRightSide = IsInRightSideGothikArea(trigger);
            gt.summonTypeFlag = 0x00;
            m_gothikTriggerMap[trigger->GetObjectGuid()] = gt;

            // Keep track of triggers that will be used as summon point
            if (!gt.isAnchorHigh && gt.isRightSide)
                summonList.push_back(trigger);
        }
    }

    if (!summonList.empty())
    {
        // Sort summoning trigger NPCS by distance from Gothik
        // and flag them regarding of what they will summon
        summonList.sort(ObjectDistanceOrder(gothik));
        uint8 index = 0;
        for (auto trigger : summonList)
        {
            switch (index)
            {
                // Closest and furthest: Unrelenting Knights and Trainees
                case 0:
                case 3:
                    m_gothikTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_TRAINEE | SUMMON_FLAG_KNIGHT;
                    break;
                // Middle: only Unrelenting Trainee
                case 1:
                    m_gothikTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_TRAINEE;
                    break;
                // Other middle: Unrelenting Rider
                case 2:
                    m_gothikTriggerMap[trigger->GetObjectGuid()].summonTypeFlag = SUMMON_FLAG_RIDER;
                    break;
                default:
                    break;
            }
            ++index;
        }
    }
    else
        script_error_log("No suitable summon trigger found for Gothik combat area. Set up failed.");
}

Creature* instance_naxxramas::GetClosestAnchorForGothik(Creature* source, bool rightSide)
{
    std::list<Creature*> outputList;

    for (auto& itr : m_gothikTriggerMap)
    {
        if (!itr.second.isAnchorHigh)
            continue;

        if (itr.second.isRightSide != rightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr.first))
            outputList.push_back(pCreature);
    }

    if (!outputList.empty())
    {
        outputList.sort(ObjectDistanceOrder(source));
        return outputList.front();
    }

    return nullptr;
}

void instance_naxxramas::GetGothikSummonPoints(CreatureList& outputList, bool rightSide)
{
    for (auto& itr : m_gothikTriggerMap)
    {
        if (itr.second.isAnchorHigh)
            continue;

        if (itr.second.isRightSide != rightSide)
            continue;

        if (Creature* pCreature = instance->GetCreature(itr.first))
            outputList.push_back(pCreature);
    }
}

// Right is right side from gothik (eastern), i.e. right is living and left is spectral
bool instance_naxxramas::IsInRightSideGothikArea(Unit* unit)
{
    if (GameObject* combatGate = GetSingleGameObjectFromStorage(GO_MILI_GOTH_COMBAT_GATE))
        return (combatGate->GetPositionY() >= unit->GetPositionY());

    script_error_log("left/right side check, Gothik combat area failed.");
    return true;
}

bool instance_naxxramas::IsSuitableTriggerForSummon(Unit* trigger, uint8 flag)
{
    return m_gothikTriggerMap[trigger->GetObjectGuid()].summonTypeFlag & flag;
}

void instance_naxxramas::DoTaunt()
{
    if (m_auiEncounter[TYPE_KELTHUZAD] != DONE)
    {
        uint8 wingsCleared = 0;

        if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
            ++wingsCleared;

        if (m_auiEncounter[TYPE_LOATHEB] == DONE)
            ++wingsCleared;

        if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
            ++wingsCleared;

        if (m_auiEncounter[TYPE_THADDIUS] == DONE)
            ++wingsCleared;

        switch (wingsCleared)
        {
            case 1: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT1, NPC_KELTHUZAD); break;
            case 2: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT2, NPC_KELTHUZAD); break;
            case 3: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT3, NPC_KELTHUZAD); break;
            case 4: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT4, NPC_KELTHUZAD); break;
            default:
                break;
        }
    }
}

// Used in Gluth fight: move all spawned Zombie Chow towards Gluth to be devoured
bool instance_naxxramas::DoHandleEvent(uint32 eventId)
{
    switch (eventId)
    {
        case EVENT_ID_DECIMATE:
            if (Creature* gluth = GetSingleCreatureFromStorage(NPC_GLUTH))
            {
                for (auto& zombieGuid : m_zombieChowList)
                {
                    if (Creature* zombie = instance->GetCreature(zombieGuid))
                    {
                        if (zombie->IsAlive())
                        {
                            zombie->AI()->SetReactState(REACT_PASSIVE);
                            zombie->AttackStop();
                            zombie->SetTarget(nullptr);
                            zombie->AI()->DoResetThreat();
                            zombie->GetMotionMaster()->MovePoint(0, gluth->GetPositionX(), gluth->GetPositionY(), gluth->GetPositionZ());
                        }
                    }
                }
                return false;
            }
            return true;
        case EVENT_CLEAR_SHACKLES:
            m_shackledGuardians = 0;
            m_checkGuardiansTimer = 2 * IN_MILLISECONDS;    // Check every two seconds how many Guardians of Icecrown are shackled
            return true;
        case EVENT_GUARDIAN_SHACKLE:
            ++m_shackledGuardians;
            return true;
        default:
            break;
    }
    return false;
}

InstanceData* GetInstanceData_instance_naxxramas(Map* pMap)
{
    return new instance_naxxramas(pMap);
}

struct Location3DPoint
{
    float x, y, z;
};

static const Location3DPoint gargoyleResetCoords = {2963.f, -3476.f, 297.6f};

enum
{
    SAY_GARGOYLE_NOISE      = -1533160, // %s emits a strange noise.

    SPELL_STONEFORM         = 29154,
    SPELL_STEALTH_DETECTION = 18950,
    SPELL_STONESKIN         = 28995,
    SPELL_ACID_VOLLEY       = 29325,
};

struct npc_stoneskin_gargoyleAI : public ScriptedAI
{
    npc_stoneskin_gargoyleAI(Creature* pCreature) : ScriptedAI(pCreature)
    {
        m_creature->GetCombatManager().SetLeashingCheck([&](Unit*, float x, float y, float z)
        {
            return x > gargoyleResetCoords.x && y > gargoyleResetCoords.y && z > gargoyleResetCoords.z;
        });
        Reset();
    }

    uint32 acidVolleyTimer;
    bool canCastVolley;

    void Reset() override
    {
        acidVolleyTimer = 4000;
        canCastVolley = false;
        TryStoneForm();

        DoCastSpellIfCan(m_creature, SPELL_STEALTH_DETECTION, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT);
    }

    void TryStoneForm()
    {
        if (m_creature->GetDefaultMovementType() == IDLE_MOTION_TYPE)
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STONEFORM, CAST_TRIGGERED | CAST_AURA_NOT_PRESENT) == CAST_OK)
            {
                m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
                m_creature->SetImmuneToPlayer(true);
            }
        }
    }

    void JustReachedHome() override
    {
        TryStoneForm();
    }

    void MoveInLineOfSight(Unit* pWho) override
    {
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE))
        {
            if (pWho->GetTypeId() == TYPEID_PLAYER
                && !m_creature->IsInCombat()
                && m_creature->IsWithinDistInMap(pWho, 17.0f)
                && !pWho->HasAuraType(SPELL_AURA_FEIGN_DEATH)
                && m_creature->IsWithinLOSInMap(pWho))
            {
                AttackStart(pWho);
            }
        }
        else
            ScriptedAI::MoveInLineOfSight(pWho);
    }

    void Aggro(Unit*) override
    {
        if (m_creature->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE | UNIT_FLAG_SPAWNING))
        {
            m_creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_UNINTERACTIBLE);
            m_creature->SetImmuneToPlayer(false);
        }

        // All Stoneskin Gargoyles cast Acid Volley but the first one encountered
        float respawnX, respawnY, respawnZ;
        m_creature->GetRespawnCoord(respawnX, respawnY, respawnZ);
        if (m_creature->GetDefaultMovementType() == IDLE_MOTION_TYPE || respawnZ < gargoyleResetCoords.z)
            canCastVolley = true;
    }

    void UpdateAI(uint32 const diff) override
    {
        if (!m_creature->SelectHostileTarget() || !m_creature->GetVictim())
            return;

        // Stoneskin at 30% HP
        if (m_creature->GetHealthPercent() < 30.0f && !m_creature->HasAura(SPELL_STONESKIN))
        {
            if (DoCastSpellIfCan(m_creature, SPELL_STONESKIN) == CAST_OK)
                DoScriptText(SAY_GARGOYLE_NOISE, m_creature);
        }

        // Acid Volley
        if (canCastVolley)
        {
            if (acidVolleyTimer < diff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_ACID_VOLLEY) == CAST_OK)
                    acidVolleyTimer = 8000;
            }
            else
                acidVolleyTimer -= diff;
        }

        DoMeleeAttackIfReady();
    }
};

/*###################
#   npc_living_poison
###################*/

struct npc_living_poisonAI : public ScriptedAI
{
    npc_living_poisonAI(Creature* creature) : ScriptedAI(creature) { Reset(); }

    void Reset() override
    {
        SetMeleeEnabled(false);
    }

    // Any time a player comes close to the Living Poison, it will explode and kill itself while doing heavy AoE damage to the player
    void MoveInLineOfSight(Unit* who) override
    {
        if (m_creature->GetDistance2d(who->GetPositionX(), who->GetPositionY(), DIST_CALC_BOUNDING_RADIUS) > 4.0f)
            return;

        DoCastSpellIfCan(m_creature, SPELL_EXPLODE, CAST_TRIGGERED);
    }

    void AttackStart(Unit* /*who*/) override {}
};

bool instance_naxxramas::DoHandleAreaTrigger(AreaTriggerEntry const* areaTrigger)
{
    if (areaTrigger->id == AREATRIGGER_KELTHUZAD)
    {
        if (GetData(TYPE_KELTHUZAD) == NOT_STARTED || GetData(TYPE_KELTHUZAD) == FAIL)
            SetData(TYPE_KELTHUZAD, IN_PROGRESS);
    }

    if (areaTrigger->id == AREATRIGGER_FAERLINA_INTRO)
    {
        if (GetData(TYPE_FAERLINA) != NOT_STARTED)
            return false;
        if (!isFaerlinaIntroDone)
            StartNextDialogueText(SAY_FAERLINA_INTRO);
    }

    if (areaTrigger->id == AREATRIGGER_THADDIUS_DOOR)
    {
        if (GetData(TYPE_THADDIUS) == NOT_STARTED)
        {
            if (Creature* thaddius = GetSingleCreatureFromStorage(NPC_THADDIUS))
            {
                SetData(TYPE_THADDIUS, SPECIAL);
                DoScriptText(SAY_THADDIUS_GREET, thaddius);
            }
        }
    }

    if (areaTrigger->id == AREATRIGGER_FROSTWYRM_TELE)
    {
        // Area trigger handles teleport in DB. Here we only need to check if all the end wing encounters are done
        if (GetData(TYPE_THADDIUS) != DONE || GetData(TYPE_LOATHEB) != DONE || GetData(TYPE_MAEXXNA) != DONE ||
                GetData(TYPE_FOUR_HORSEMEN) != DONE)
            return true;
    }

    return false;
}

bool AreaTrigger_at_naxxramas(Player* player, AreaTriggerEntry const* areaTrigger)
{
    if (player->IsGameMaster() || !player->IsAlive())
        return false;

    if (auto* instance = (instance_naxxramas*)player->GetInstanceData())
        return instance->DoHandleAreaTrigger(areaTrigger);

    return false;
}

bool ProcessEventId_naxxramas(uint32 eventId, Object* source, Object* /*target*/, bool /*isStart*/)
{
    if (auto* instance = (instance_naxxramas*)((Creature*)source)->GetInstanceData())
        return instance->DoHandleEvent(eventId);

    return false;
}

void AddSC_instance_naxxramas()
{
    Script* newScript = new Script;
    newScript->Name = "instance_naxxramas";
    newScript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_stoneskin_gargoyle";
    newScript->GetAI = &GetNewAIInstance<npc_stoneskin_gargoyleAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "npc_living_poison";
    newScript->GetAI = &GetNewAIInstance<npc_living_poisonAI>;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "at_naxxramas";
    newScript->pAreaTrigger = &AreaTrigger_at_naxxramas;
    newScript->RegisterSelf();

    newScript = new Script;
    newScript->Name = "event_naxxramas";
    newScript->pProcessEventId = &ProcessEventId_naxxramas;
    newScript->RegisterSelf();
}
