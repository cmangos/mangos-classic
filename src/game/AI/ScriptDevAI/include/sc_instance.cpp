/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#include "AI/ScriptDevAI/include/precompiled.h"

/**
   Function that uses a door or a button

   @param   guid The ObjectGuid of the Door/ Button that will be used
   @param   uiWithRestoreTime (in seconds) if == 0 autoCloseTime will be used (if not 0 by default in *_template)
   @param   bUseAlternativeState Use to alternative state
 */
void ScriptedInstance::DoUseDoorOrButton(ObjectGuid guid, uint32 withRestoreTime, bool useAlternativeState)
{
    if (!guid)
        return;

    if (GameObject* pGo = instance->GetGameObject(guid))
    {
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR || pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON)
        {
            if (pGo->GetLootState() == GO_READY)
                pGo->UseDoorOrButton(withRestoreTime, useAlternativeState);
            else if (pGo->GetLootState() == GO_ACTIVATED)
                pGo->ResetDoorOrButton();
        }
        else
            script_error_log("Script call DoUseDoorOrButton, but gameobject entry %u is type %u.", pGo->GetEntry(), pGo->GetGoType());
    }
}

/// Function that uses a door or button that is stored in m_goEntryGuidStore
void ScriptedInstance::DoUseDoorOrButton(uint32 entry, uint32 withRestoreTime /*= 0*/, bool useAlternativeState /*= false*/)
{
    EntryGuidMap::iterator find = m_goEntryGuidStore.find(entry);
    if (find != m_goEntryGuidStore.end())
        DoUseDoorOrButton(find->second, withRestoreTime, useAlternativeState);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded
        debug_log("SD2: Script call DoUseDoorOrButton(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", entry, instance->GetId());
}

/**
   Function that respawns a despawned GameObject with given time

   @param   guid The ObjectGuid of the GO that will be respawned
   @param   uiTimeToDespawn (in seconds) Despawn the GO after this time, default is a minute
 */
void ScriptedInstance::DoRespawnGameObject(ObjectGuid guid, uint32 timeToDespawn)
{
    if (!guid)
        return;

    if (GameObject* pGo = instance->GetGameObject(guid))
    {
        // not expect any of these should ever be handled
        if (pGo->GetGoType() == GAMEOBJECT_TYPE_FISHINGNODE || pGo->GetGoType() == GAMEOBJECT_TYPE_DOOR ||
                pGo->GetGoType() == GAMEOBJECT_TYPE_BUTTON || pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
            return;

        if (pGo->IsSpawned())
            return;

        pGo->SetRespawnTime(timeToDespawn);
        pGo->Refresh();
    }
}

/// Function that uses a door or button that is stored in m_goEntryGuidStore
void ScriptedInstance::DoToggleGameObjectFlags(uint32 entry, uint32 GOflags, bool apply)
{
    EntryGuidMap::iterator find = m_goEntryGuidStore.find(entry);
    if (find != m_goEntryGuidStore.end())
        DoToggleGameObjectFlags(find->second, GOflags, apply);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded
        debug_log("SD2: Script call ToogleTameObjectFlags (by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", entry, instance->GetId());
}

/**
   Function that toggles the GO-flags of a GameObject

   @param   guid The ObjectGuid of the GO that will be respawned
   @param   uiGOflags Which GO-flags to toggle
   @param   bApply should the GO-flags be applied or removed?
 */
void ScriptedInstance::DoToggleGameObjectFlags(ObjectGuid guid, uint32 GOflags, bool apply)
{
    if (!guid)
        return;

    if (GameObject* pGo = instance->GetGameObject(guid))
    {
        if (apply)
            pGo->SetFlag(GAMEOBJECT_FLAGS, GOflags);
        else
            pGo->RemoveFlag(GAMEOBJECT_FLAGS, GOflags);
    }
}

/// Function that respawns a despawned GO that is stored in m_goEntryGuidStore
void ScriptedInstance::DoRespawnGameObject(uint32 entry, uint32 timeToDespawn)
{
    EntryGuidMap::iterator find = m_goEntryGuidStore.find(entry);
    if (find != m_goEntryGuidStore.end())
        DoRespawnGameObject(find->second, timeToDespawn);
    else
        // Output log, possible reason is not added GO to storage, or not yet loaded;
        debug_log("SD2: Script call DoRespawnGameObject(by Entry), but no gameobject of entry %u was created yet, or it was not stored by script for map %u.", entry, instance->GetId());
}

/**
   Helper function to update a world state for all players in the map

   @param   uiStateId The WorldState that will be set for all players in the map
   @param   uiStateData The Value to which the State will be set to
 */
void ScriptedInstance::DoUpdateWorldState(uint32 stateId, uint32 stateData)
{
    Map::PlayerList const& lPlayers = instance->GetPlayers();

    if (!lPlayers.isEmpty())
    {
        for (const auto& lPlayer : lPlayers)
        {
            if (Player* pPlayer = lPlayer.getSource())
                pPlayer->SendUpdateWorldState(stateId, stateData);
        }
    }
    else
        debug_log("SD2: DoUpdateWorldState attempt send data but no players in map.");
}

/// Get the first found Player* (with requested properties) in the map. Can return nullptr.
Player* ScriptedInstance::GetPlayerInMap(bool bOnlyAlive /*=false*/, bool bCanBeGamemaster /*=true*/) const
{
    Map::PlayerList const& lPlayers = instance->GetPlayers();

    for (const auto& lPlayer : lPlayers)
    {
        Player* pPlayer = lPlayer.getSource();
        if (pPlayer && (!bOnlyAlive || pPlayer->isAlive()) && (bCanBeGamemaster || !pPlayer->isGameMaster()))
            return pPlayer;
    }

    return nullptr;
}

/// Returns a pointer to a loaded GameObject that was stored in m_goEntryGuidStore. Can return nullptr
GameObject* ScriptedInstance::GetSingleGameObjectFromStorage(uint32 entry)
{
    EntryGuidMap::iterator find = m_goEntryGuidStore.find(entry);
    if (find != m_goEntryGuidStore.end())
        return instance->GetGameObject(find->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    script_error_log("Script requested gameobject with entry %u, but no gameobject of this entry was created yet, or it was not stored by script for map %u.", entry, instance->GetId());

    return nullptr;
}

/// Returns a pointer to a loaded Creature that was stored in m_goEntryGuidStore. Can return nullptr
Creature* ScriptedInstance::GetSingleCreatureFromStorage(uint32 entry, bool skipDebugLog /*=false*/)
{
    EntryGuidMap::iterator find = m_npcEntryGuidStore.find(entry);
    if (find != m_npcEntryGuidStore.end())
        return instance->GetCreature(find->second);

    // Output log, possible reason is not added GO to map, or not yet loaded;
    if (!skipDebugLog)
        script_error_log("Script requested creature with entry %u, but no npc of this entry was created yet, or it was not stored by script for map %u.", entry, instance->GetId());

    return nullptr;
}

void ScriptedInstance::GetCreatureGuidVectorFromStorage(uint32 entry, GuidVector& entryGuidVector, bool /*skipDebugLog*/)
{
    auto iter = m_npcEntryGuidCollection.find(entry);
    if (iter != m_npcEntryGuidCollection.end())
        entryGuidVector = (*iter).second;
}

void ScriptedInstance::GetGameObjectGuidVectorFromStorage(uint32 entry, GuidVector& entryGuidVector, bool /*skipDebugLog*/)
{
    auto iter = m_goEntryGuidCollection.find(entry);
    if (iter != m_goEntryGuidCollection.end())
        entryGuidVector = (*iter).second;
}

/**
   Constructor for DialogueHelper

   @param   pDialogueArray The static const array of DialogueEntry holding the information about the dialogue. This array MUST be terminated by {0,0,0}
 */
DialogueHelper::DialogueHelper(DialogueEntry const* dialogueArray) :
    m_instance(nullptr),
    m_dialogueArray(dialogueArray),
    m_currentEntry(nullptr),
    m_dialogueTwoSideArray(nullptr),
    m_currentEntryTwoSide(nullptr),
    m_timer(0),
    m_isFirstSide(true),
    m_canSimulate(false)
{}

/**
   Constructor for DialogueHelper (Two Sides)

   @param   pDialogueTwoSideArray The static const array of DialogueEntryTwoSide holding the information about the dialogue. This array MUST be terminated by {0,0,0,0,0}
 */
DialogueHelper::DialogueHelper(DialogueEntryTwoSide const* dialogueTwoSideArray) :
    m_instance(nullptr),
    m_dialogueArray(nullptr),
    m_currentEntry(nullptr),
    m_dialogueTwoSideArray(dialogueTwoSideArray),
    m_currentEntryTwoSide(nullptr),
    m_timer(0),
    m_isFirstSide(true),
    m_canSimulate(false)
{}

/**
   Function to start a (part of a) dialogue

   @param   iTextEntry The TextEntry of the dialogue that will be started (must be always the entry of first side)
 */
void DialogueHelper::StartNextDialogueText(int32 textEntry)
{
    // Find iTextEntry
    bool found = false;

    if (m_dialogueArray)                                   // One Side
    {
        for (DialogueEntry const* entry = m_dialogueArray; entry->textEntry; ++entry)
        {
            if (entry->textEntry == textEntry)
            {
                m_currentEntry = entry;
                found = true;
                break;
            }
        }
    }
    else                                                    // Two Sides
    {
        for (DialogueEntryTwoSide const* entry = m_dialogueTwoSideArray; entry->textEntry; ++entry)
        {
            if (entry->textEntry == textEntry)
            {
                m_currentEntryTwoSide = entry;
                found = true;
                break;
            }
        }
    }

    if (!found)
    {
        script_error_log("Script call DialogueHelper::StartNextDialogueText, but textEntry %i is not in provided dialogue (on map id %u)", textEntry, m_instance ? m_instance->instance->GetId() : 0);
        return;
    }

    DoNextDialogueStep();
}

/// Internal helper function to do the actual say of a DialogueEntry
void DialogueHelper::DoNextDialogueStep()
{
    // Last Dialogue Entry done?
    if ((m_currentEntry && !m_currentEntry->textEntry) || (m_currentEntryTwoSide && !m_currentEntryTwoSide->textEntry))
    {
        m_timer = 0;
        return;
    }

    // Get Text, SpeakerEntry and Timer
    int32 iTextEntry = 0;
    uint32 uiSpeakerEntry = 0;

    if (m_dialogueArray)                               // One Side
    {
        uiSpeakerEntry = m_currentEntry->sayerEntry;
        iTextEntry = m_currentEntry->textEntry;

        m_timer = m_currentEntry->timer;
    }
    else                                                // Two Sides
    {
        // Second Entries can be 0, if they are the entry from first side will be taken
        uiSpeakerEntry = !m_isFirstSide && m_currentEntryTwoSide->sayerEntryAlt ? m_currentEntryTwoSide->sayerEntryAlt : m_currentEntryTwoSide->sayerEntry;
        iTextEntry = !m_isFirstSide && m_currentEntryTwoSide->textEntryAlt ? m_currentEntryTwoSide->textEntryAlt : m_currentEntryTwoSide->textEntry;

        m_timer = m_currentEntryTwoSide->timer;
    }

    // Simulate Case
    if (uiSpeakerEntry && iTextEntry < 0)
    {
        // Use Speaker if directly provided
        Creature* pSpeaker = GetSpeakerByEntry(uiSpeakerEntry);
        if (m_instance && !pSpeaker)                       // Get Speaker from instance
        {
            if (m_canSimulate)                             // Simulate case
                m_instance->DoOrSimulateScriptTextForThisInstance(iTextEntry, uiSpeakerEntry);
            else
                pSpeaker = m_instance->GetSingleCreatureFromStorage(uiSpeakerEntry);
        }

        if (pSpeaker)
            DoScriptText(iTextEntry, pSpeaker);
    }

    JustDidDialogueStep(m_dialogueArray ?  m_currentEntry->textEntry : m_currentEntryTwoSide->textEntry);

    // Increment position
    if (m_dialogueArray)
        ++m_currentEntry;
    else
        ++m_currentEntryTwoSide;
}

/// Call this function within any DialogueUpdate method. This is required for saying next steps in a dialogue
void DialogueHelper::DialogueUpdate(uint32 diff)
{
    if (m_timer)
    {
        if (m_timer <= diff)
            DoNextDialogueStep();
        else
            m_timer -= diff;
    }
}
