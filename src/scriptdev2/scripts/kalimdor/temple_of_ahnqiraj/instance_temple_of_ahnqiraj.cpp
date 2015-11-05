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
EndScriptData */

#include "precompiled.h"
#include "temple_of_ahnqiraj.h"

static const DialogueEntry aIntroDialogue[] =
{
    {EMOTE_EYE_INTRO,       NPC_MASTERS_EYE, 7000},
    {SAY_EMPERORS_INTRO_1,  NPC_VEKLOR,      6000},
    {SAY_EMPERORS_INTRO_2,  NPC_VEKNILASH,   8000},
    {SAY_EMPERORS_INTRO_3,  NPC_VEKLOR,      3000},
    {SAY_EMPERORS_INTRO_4,  NPC_VEKNILASH,   3000},
    {SAY_EMPERORS_INTRO_5,  NPC_VEKLOR,      3000},
    {SAY_EMPERORS_INTRO_6,  NPC_VEKNILASH,   0},
    {0, 0, 0}
};

instance_temple_of_ahnqiraj::instance_temple_of_ahnqiraj(Map* pMap) : ScriptedInstance(pMap),
    m_uiBugTrioDeathCount(0),
    m_uiCthunWhisperTimer(90000),
    m_bIsEmperorsIntroDone(false),
    m_dialogueHelper(aIntroDialogue)
{
    Initialize();
};

void instance_temple_of_ahnqiraj::Initialize()
{
    memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

    m_dialogueHelper.InitializeDialogueHelper(this);
}

bool instance_temple_of_ahnqiraj::IsEncounterInProgress() const
{
    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            return true;
    }

    return false;
}

void instance_temple_of_ahnqiraj::DoHandleTempleAreaTrigger(uint32 uiTriggerId)
{
    if (uiTriggerId == AREATRIGGER_TWIN_EMPERORS && !m_bIsEmperorsIntroDone)
    {
        m_dialogueHelper.StartNextDialogueText(EMOTE_EYE_INTRO);
        // Note: there may be more related to this; The emperors should kneel before the Eye and they stand up after it despawns
        if (Creature* pEye = GetSingleCreatureFromStorage(NPC_MASTERS_EYE))
            pEye->ForcedDespawn(1000);
        m_bIsEmperorsIntroDone = true;
    }
    else if (uiTriggerId == AREATRIGGER_SARTURA)
    {
        if (GetData(TYPE_SARTURA) == NOT_STARTED || GetData(TYPE_SARTURA) == FAIL)
        {
            if (Creature* pSartura = GetSingleCreatureFromStorage(NPC_SARTURA))
                pSartura->SetInCombatWithZone();
        }
    }
}

void instance_temple_of_ahnqiraj::OnCreatureCreate(Creature* pCreature)
{
    switch (pCreature->GetEntry())
    {
        case NPC_SKERAM:
            // Don't store the summoned images guid
            if (GetData(TYPE_SKERAM) == IN_PROGRESS)
                break;
        case NPC_SARTURA:
        case NPC_VEKLOR:
        case NPC_VEKNILASH:
        case NPC_MASTERS_EYE:
        case NPC_OURO_SPAWNER:
        case NPC_CTHUN:
            m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
            break;
    }
}

void instance_temple_of_ahnqiraj::OnObjectCreate(GameObject* pGo)
{
    switch (pGo->GetEntry())
    {
        case GO_SKERAM_GATE:
            if (m_auiEncounter[TYPE_SKERAM] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_TWINS_ENTER_DOOR:
            break;
        case GO_TWINS_EXIT_DOOR:
            if (m_auiEncounter[TYPE_TWINS] == DONE)
                pGo->SetGoState(GO_STATE_ACTIVE);
            break;
        case GO_SANDWORM_BASE:
            break;

        default:
            return;
    }

    m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
}

void instance_temple_of_ahnqiraj::SetData(uint32 uiType, uint32 uiData)
{
    switch (uiType)
    {
        case TYPE_SKERAM:
            m_auiEncounter[uiType] = uiData;
            if (uiData == DONE)
                DoUseDoorOrButton(GO_SKERAM_GATE);
            break;
        case TYPE_BUG_TRIO:
            if (uiData == SPECIAL)
            {
                ++m_uiBugTrioDeathCount;
                if (m_uiBugTrioDeathCount == 2)
                    SetData(TYPE_BUG_TRIO, DONE);

                // don't store any special data
                break;
            }
            if (uiData == FAIL)
                m_uiBugTrioDeathCount = 0;
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_SARTURA:
        case TYPE_FANKRISS:
        case TYPE_VISCIDUS:
        case TYPE_HUHURAN:
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_TWINS:
            // Either of the twins can set data, so return to avoid double changing
            if (m_auiEncounter[uiType] ==  uiData)
                return;

            m_auiEncounter[uiType] = uiData;
            DoUseDoorOrButton(GO_TWINS_ENTER_DOOR);
            if (uiData == DONE)
                DoUseDoorOrButton(GO_TWINS_EXIT_DOOR);
            break;
        case TYPE_OURO:
            switch (uiData)
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
            m_auiEncounter[uiType] = uiData;
            break;
        case TYPE_CTHUN:
            m_auiEncounter[uiType] = uiData;
            break;
    }

    if (uiData == DONE)
    {
        OUT_SAVE_INST_DATA;

        std::ostringstream saveStream;
        saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " " << m_auiEncounter[3] << " "
                   << m_auiEncounter[4] << " " << m_auiEncounter[5] << " " << m_auiEncounter[6] << " " << m_auiEncounter[7] << " "
                   << m_auiEncounter[8];

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
    loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
               >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
               >> m_auiEncounter[8];

    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
    {
        if (m_auiEncounter[i] == IN_PROGRESS)
            m_auiEncounter[i] = NOT_STARTED;
    }

    OUT_LOAD_INST_DATA_COMPLETE;
}

uint32 instance_temple_of_ahnqiraj::GetData(uint32 uiType) const
{
    if (uiType < MAX_ENCOUNTER)
        return m_auiEncounter[uiType];

    return 0;
}

void instance_temple_of_ahnqiraj::Update(uint32 uiDiff)
{
    m_dialogueHelper.DialogueUpdate(uiDiff);

    if (GetData(TYPE_CTHUN) == IN_PROGRESS || GetData(TYPE_CTHUN) == DONE)
        return;

    if (m_uiCthunWhisperTimer < uiDiff)
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
        m_uiCthunWhisperTimer -= uiDiff;
}

InstanceData* GetInstanceData_instance_temple_of_ahnqiraj(Map* pMap)
{
    return new instance_temple_of_ahnqiraj(pMap);
}

bool AreaTrigger_at_temple_ahnqiraj(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (pAt->id == AREATRIGGER_TWIN_EMPERORS || pAt->id == AREATRIGGER_SARTURA)
    {
        if (pPlayer->isGameMaster() || !pPlayer->isAlive())
            return false;

        if (instance_temple_of_ahnqiraj* pInstance = (instance_temple_of_ahnqiraj*)pPlayer->GetInstanceData())
            pInstance->DoHandleTempleAreaTrigger(pAt->id);
    }

    return false;
}

void AddSC_instance_temple_of_ahnqiraj()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "instance_temple_of_ahnqiraj";
    pNewScript->GetInstanceData = &GetInstanceData_instance_temple_of_ahnqiraj;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_temple_ahnqiraj";
    pNewScript->pAreaTrigger = &AreaTrigger_at_temple_ahnqiraj;
    pNewScript->RegisterSelf();
}
