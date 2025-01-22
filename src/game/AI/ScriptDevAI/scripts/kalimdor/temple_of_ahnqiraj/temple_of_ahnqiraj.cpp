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
SDName: Instance_Temple_of_Ahnqiraj
SD%Complete: 80
SDComment: C'thun whisperings spells NYI.
SDCategory: Temple of Ahn'Qiraj
EndScriptData

*/

#include "AI/ScriptDevAI/include/sc_common.h"
#include "temple_of_ahnqiraj.h"

static const DialogueEntry aIntroDialogue[] =
{
    {EMOTE_EYE_INTRO,       NPC_MASTERS_EYE, 2000},
    {STAND_EMPERORS_INTRO,  0,               6000},
    {SAY_EMPERORS_INTRO_1,  NPC_VEKLOR,      6000},
    {SAY_EMPERORS_INTRO_2,  NPC_VEKNILASH,   8000},
    {SAY_EMPERORS_INTRO_3,  NPC_VEKLOR,      3000},
    {SAY_EMPERORS_INTRO_4,  NPC_VEKNILASH,   3000},
    {SAY_EMPERORS_INTRO_5,  NPC_VEKLOR,      1000},
    {SAY_EMPERORS_INTRO_6,  NPC_VEKNILASH,   0},
    {0, 0, 0}
};

instance_temple_of_ahnqiraj::instance_temple_of_ahnqiraj(Map* map) : ScriptedInstance(map), DialogueHelper(aIntroDialogue),
    m_uiBugTrioDeathCount(0),
    m_uiCthunWhisperTimer(90000),
    m_uiSkeramProphecyTimer(5 * MINUTE * IN_MILLISECONDS)
{
    Initialize();
};

void instance_temple_of_ahnqiraj::Initialize()
{
    memset(&m_encounter, 0, sizeof(m_encounter));

    InitializeDialogueHelper(this);
}

bool instance_temple_of_ahnqiraj::IsEncounterInProgress() const
{
    for (uint32 i : m_encounter)
    {
        if (i == IN_PROGRESS)
            return true;
    }

    return false;
}

void instance_temple_of_ahnqiraj::OnPlayerLeave(Player* player)
{
    // Remove any aura related to Temple of Ahn'Qiraj specific mounts
    for (uint32 aura : qiraji_mount_auras)
    {
        if (player->HasAura(aura))
        {
            player->RemoveAurasDueToSpell(aura);
            return;
        }
    }
}

void instance_temple_of_ahnqiraj::JustDidDialogueStep(int32 entry)
{
    switch (entry)
    {
        case EMOTE_EYE_INTRO:
        {
            // Despawn the Eye of C'Thun
            if (Creature* eye = GetSingleCreatureFromStorage(NPC_MASTERS_EYE))
                eye->ForcedDespawn(6000);
            break;
        }
        case STAND_EMPERORS_INTRO:
        {
            // Make the Twin Emperors stand
            if (Creature* veklor = GetSingleCreatureFromStorage(NPC_VEKLOR))
                veklor->SetStandState(UNIT_STAND_STATE_STAND);
            if (Creature* veknilash = GetSingleCreatureFromStorage(NPC_VEKNILASH))
                veknilash->SetStandState(UNIT_STAND_STATE_STAND);
            break;
        }
        case SAY_EMPERORS_INTRO_6:
            // Save the event for the rest of the instance life span
            SetData(TYPE_TWINS_INTRO, DONE);
            break;
        default:
            break;
    }
}

void instance_temple_of_ahnqiraj::DoHandleTempleAreaTrigger(uint32 triggerId, Player* player)
{
    if (triggerId == AREATRIGGER_TWIN_EMPERORS && GetData(TYPE_TWINS_INTRO) == NOT_STARTED)
    {
        // Make the Eye of C'Thun face the incoming player and start the dialogue
        StartNextDialogueText(EMOTE_EYE_INTRO);
        SetData(TYPE_TWINS_INTRO, IN_PROGRESS);
        if (Creature* eye = GetSingleCreatureFromStorage(NPC_MASTERS_EYE))
            eye->SetFacingToObject(player);
    }
    else if (triggerId == AREATRIGGER_SARTURA)
    {
        if (GetData(TYPE_SARTURA) == NOT_STARTED || GetData(TYPE_SARTURA) == FAIL)
        {
            if (Creature* sartura = GetSingleCreatureFromStorage(NPC_SARTURA))
                sartura->SetInCombatWithZone();
        }
    }
}

void instance_temple_of_ahnqiraj::OnCreatureDeath(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_CLAW_TENTACLE:
        case NPC_EYE_TENTACLE:
            if (Creature* portal = GetClosestCreatureWithEntry(creature, NPC_TENTACLE_PORTAL, 5.0f))
                portal->ForcedDespawn();
        break;
        case NPC_GIANT_CLAW_TENTACLE:
        case NPC_GIANT_EYE_TENTACLE:
            if (Creature* portal = GetClosestCreatureWithEntry(creature, NPC_GIANT_TENTACLE_PORTAL, 5.0f))
                portal->ForcedDespawn();
        break;
    }
}

void instance_temple_of_ahnqiraj::OnCreatureCreate(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_VEKLOR:
        case NPC_VEKNILASH:
            if (GetData(TYPE_TWINS_INTRO) != DONE)
                creature->SetStandState(UNIT_STAND_STATE_KNEEL);
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
        case NPC_SKERAM:
            // Don't store the summoned images guid
            if (GetData(TYPE_SKERAM) == IN_PROGRESS)
                break;
        case NPC_KRI:
        case NPC_YAUJ:
        case NPC_VEM:
        case NPC_SARTURA:
        case NPC_MASTERS_EYE:
        case NPC_OURO_SPAWNER:
        case NPC_CTHUN:
        case NPC_VISCIDUS:
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
        case NPC_POISON_CLOUD:
            m_bugTrioSpawns.push_back(creature->GetObjectGuid());
            break;
        case NPC_EYE_OF_CTHUN:
            // Safeguard for C'Thun encounter in case of fight abruptly ended during phase 2
            if (GetData(TYPE_CTHUN) != DONE)
            {
                if (!creature->IsAlive())
                    creature->Respawn();
            }
            m_npcEntryGuidStore[creature->GetEntry()] = creature->GetObjectGuid();
            break;
    }
}

void instance_temple_of_ahnqiraj::OnObjectCreate(GameObject* go)
{
    switch (go->GetEntry())
    {
        case GO_SKERAM_GATE:
            if (m_encounter[TYPE_SKERAM] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_TWINS_ENTER_DOOR:
            if (m_encounter[TYPE_HUHURAN] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_TWINS_EXIT_DOOR:
            if (m_encounter[TYPE_TWINS] == DONE)
                go->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_SANDWORM_BASE:
            break;

        default:
            return;
    }

    m_goEntryGuidStore[go->GetEntry()] = go->GetObjectGuid();
}

void instance_temple_of_ahnqiraj::OnCreatureRespawn(Creature* creature)
{
    switch (creature->GetEntry())
    {
        case NPC_QIRAJI_SCARAB:
        case NPC_QIRAJI_SCORPION:
            creature->SetCorpseDelay(5);
            break;
        case NPC_EXIT_TRIGGER:
            creature->SetInCombatWithZone(false);
            if (creature->AI())
                creature->AI()->SetCombatMovement(false);
            break;
    }   
}

void instance_temple_of_ahnqiraj::SetData(uint32 type, uint32 data)
{
    switch (type)
    {
        case TYPE_SKERAM:
            m_encounter[type] = data;
            if (data == DONE)
                DoUseDoorOrButton(GO_SKERAM_GATE);
            break;
        case TYPE_BUG_TRIO:
            if (data > SPECIAL)
            {
                ++m_uiBugTrioDeathCount;
                Creature* deadGuy = GetSingleCreatureFromStorage(data);
                // notify bugs on death to heal / remove invul
                if (Creature* vem = GetSingleCreatureFromStorage(NPC_VEM))
                    if (vem->IsAlive())
                        vem->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, deadGuy, vem, m_uiBugTrioDeathCount);
                if (Creature* yauj = GetSingleCreatureFromStorage(NPC_YAUJ))
                    if (yauj->IsAlive())
                        yauj->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, deadGuy, yauj, m_uiBugTrioDeathCount);
                if (Creature* kri = GetSingleCreatureFromStorage(NPC_KRI))
                    if (kri->IsAlive())
                        kri->AI()->SendAIEvent(AI_EVENT_CUSTOM_A, deadGuy, kri, m_uiBugTrioDeathCount);

                // don't store any special data
                break;
            }
            if (data == FAIL)
            {
                // Do not reset again the encounter if already set to failed
                if (GetData(TYPE_BUG_TRIO) == FAIL)
                    return;

                // On encounter failure, despawn Vem and Princess Yauj and make Lord Kri evade
                // the two other bosses will respawn when Lord Kri reaches home/respawns
                if (Creature* vem = GetSingleCreatureFromStorage(NPC_VEM))
                    vem->ForcedDespawn();
                if (Creature* yauj = GetSingleCreatureFromStorage(NPC_YAUJ))
                    yauj->ForcedDespawn();
                if (Creature* kri = GetSingleCreatureFromStorage(NPC_KRI))
                {
                    if (kri->IsAlive())
                        kri->AI()->EnterEvadeMode();
                    else
                        kri->Respawn();
                }
                for (auto guid : m_bugTrioSpawns)
                    if (Creature* spawn = instance->GetCreature(guid))
                        spawn->ForcedDespawn();
                m_bugTrioSpawns.clear();
            }
            if (data == IN_PROGRESS)
                m_uiBugTrioDeathCount = 0;
            m_encounter[type] = data;
            break;
        case TYPE_SARTURA:
        case TYPE_FANKRISS:
        case TYPE_VISCIDUS:
            m_encounter[type] = data;
            break;
        case TYPE_HUHURAN:
            m_encounter[type] = data;
            if (data == DONE)
                DoUseDoorOrButton(GO_TWINS_ENTER_DOOR);
            break;
        case TYPE_TWINS:
            // Either of the twins can set data, so return to avoid double changing
            if (m_encounter[type] ==  data)
                return;

            m_encounter[type] = data;
            DoUseDoorOrButton(GO_TWINS_ENTER_DOOR);
            if (data == DONE)
                DoUseDoorOrButton(GO_TWINS_EXIT_DOOR);
            break;
        case TYPE_OURO:
            switch (data)
            {
                case FAIL:
                    // Respawn the Ouro spawner on fail
                    if (Creature* pSpawner = GetSingleCreatureFromStorage(NPC_OURO_SPAWNER))
                        pSpawner->Respawn();
                // no break;
                case DONE:
                    // Despawn the sandworm base on Done or Fail
                    if (GameObject* pBase = GetSingleGameObjectFromStorage(GO_SANDWORM_BASE))
                        pBase->SetLootState(GO_JUST_DEACTIVATED);
                    break;
            }
            m_encounter[type] = data;
            break;
        case TYPE_CTHUN:
            if (data == FAIL)
            {
                // Respawn the Eye of C'Thun when failing in phase 2
                if (Creature* eyeOfCthun = GetSingleCreatureFromStorage(NPC_EYE_OF_CTHUN))
                {
                    if (eyeOfCthun->AI() && eyeOfCthun->IsInCombat())
                        eyeOfCthun->AI()->EnterEvadeMode();
                }
                else
                {
                    auto iter = m_npcEntryGuidStore.find(NPC_EYE_OF_CTHUN);
                    if (iter != m_npcEntryGuidStore.end())
                        instance->GetSpawnManager().RespawnCreature(iter->second.GetCounter(), 60);
                }
                    
                if (Creature* cthun = GetSingleCreatureFromStorage(NPC_CTHUN))
                    // Reset combat
                    if (cthun->AI() && cthun->IsInCombat())
                        cthun->AI()->EnterEvadeMode();
            }
            m_encounter[type] = data;
            break;
        case TYPE_TWINS_INTRO:
            m_encounter[type] = data;
            break;
    }

    if (data == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_encounter[0] << " " << m_encounter[1] << " " << m_encounter[2] << " " << m_encounter[3] << " "
                   << m_encounter[4] << " " << m_encounter[5] << " " << m_encounter[6] << " " << m_encounter[7] << " "
                   << m_encounter[8] << " " << m_encounter[9];

        m_strInstData = saveStream.str();

        SaveToDB();
        OUT_SAVE_INST_DATA_COMPLETE;
    }
}

void instance_temple_of_ahnqiraj::Load(const char* chrIn)
{
    if (!chrIn)
    {
        OUT_LOAD_INST_DATA_FAIL;
        return;
    }

    OUT_LOAD_INST_DATA(chrIn);

    std::istringstream loadStream(chrIn);
    loadStream >> m_encounter[0] >> m_encounter[1] >> m_encounter[2] >> m_encounter[3]
               >> m_encounter[4] >> m_encounter[5] >> m_encounter[6] >> m_encounter[7]
               >> m_encounter[8] >> m_encounter[9];

    for (uint32& i : m_encounter)
    {
        if (i == IN_PROGRESS)
            i = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_temple_of_ahnqiraj::GetData(uint32 type) const
{
    if (type < MAX_ENCOUNTER)
        return m_encounter[type];

    return 0;
}

void instance_temple_of_ahnqiraj::Update(uint32 diff)
{
    DialogueUpdate(diff);

    if (GetData(TYPE_SKERAM) == NOT_STARTED)
    {
        if (m_uiSkeramProphecyTimer < diff)
        {
            if (Player* player = GetPlayerInMap())
                player->GetMap()->PlayDirectSoundToMap(sound_skeram_prophecy[urand(0, 4)]);
            m_uiSkeramProphecyTimer = urand(3, 4) * MINUTE * IN_MILLISECONDS;   // Timer is guesswork
        }
        else
            m_uiSkeramProphecyTimer -= diff;
    }

    if (GetData(TYPE_CTHUN) == IN_PROGRESS || GetData(TYPE_CTHUN) == DONE)
        return;

    if (m_uiCthunWhisperTimer < diff)
    {
        if (Player* pPlayer = GetPlayerInMap())
        {
            if (Creature* pCthun = GetSingleCreatureFromStorage(NPC_CTHUN))
            {
                // ToDo: also cast the C'thun Whispering charm spell - requires additional research
                switch (urand(0, 7))
                {
                    case 0: DoScriptText(SAY_CTHUN_WHISPER_1, pCthun, pPlayer); break;
                    case 1: DoScriptText(SAY_CTHUN_WHISPER_2, pCthun, pPlayer); break;
                    case 2: DoScriptText(SAY_CTHUN_WHISPER_3, pCthun, pPlayer); break;
                    case 3: DoScriptText(SAY_CTHUN_WHISPER_4, pCthun, pPlayer); break;
                    case 4: DoScriptText(SAY_CTHUN_WHISPER_5, pCthun, pPlayer); break;
                    case 5: DoScriptText(SAY_CTHUN_WHISPER_6, pCthun, pPlayer); break;
                    case 6: DoScriptText(SAY_CTHUN_WHISPER_7, pCthun, pPlayer); break;
                    case 7: DoScriptText(SAY_CTHUN_WHISPER_8, pCthun, pPlayer); break;
                }
            }
        }
        m_uiCthunWhisperTimer = urand(1.5 * MINUTE * IN_MILLISECONDS, 5 * MINUTE * IN_MILLISECONDS);
    }
    else
        m_uiCthunWhisperTimer -= diff;
}

bool AreaTrigger_at_temple_ahnqiraj(Player* player, AreaTriggerEntry const* at)
{
    if (at->id == AREATRIGGER_TWIN_EMPERORS || at->id == AREATRIGGER_SARTURA)
    {
        if (player->IsGameMaster() || !player->IsAlive())
            return false;

        if (instance_temple_of_ahnqiraj* instance = dynamic_cast<instance_temple_of_ahnqiraj*>(player->GetInstanceData()))
            instance->DoHandleTempleAreaTrigger(at->id, player);
    }

    return false;
}

bool ProcessEventId_event_reset_cthun(uint32 /*eventId*/, Object* source, Object* /*target*/, bool /*isStart*/)
{
    if (source->IsCreature())
    {
        if (instance_temple_of_ahnqiraj* instance = dynamic_cast<instance_temple_of_ahnqiraj*>(static_cast<Creature*>(source)->GetInstanceData()))
        {
            instance->SetData(TYPE_CTHUN, FAIL);
            return true;
        }
    }
    return false;
}

void AddSC_instance_temple_of_ahnqiraj()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "instance_temple_of_ahnqiraj";
    pNewScript->GetInstanceData = &GetNewInstanceScript<instance_temple_of_ahnqiraj>;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_temple_ahnqiraj";
    pNewScript->pAreaTrigger = &AreaTrigger_at_temple_ahnqiraj;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "event_cthun_reset";
    pNewScript->pProcessEventId = &ProcessEventId_event_reset_cthun;
    pNewScript->RegisterSelf();
}
