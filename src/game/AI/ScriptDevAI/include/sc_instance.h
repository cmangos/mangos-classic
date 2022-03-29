/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef SC_INSTANCE_H
#define SC_INSTANCE_H

#include "Maps/InstanceData.h"
#include "Maps/Map.h"
#include "AI/ScriptDevAI/ScriptDevAIMgr.h"

enum EncounterState
{
    NOT_STARTED   = 0,
    IN_PROGRESS   = 1,
    FAIL          = 2,
    DONE          = 3,
    SPECIAL       = 4
};

#define OUT_SAVE_INST_DATA             debug_log("SD2: Saving Instance Data for Instance %s (Map %d, Instance Id %d)", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_SAVE_INST_DATA_COMPLETE    debug_log("SD2: Saving Instance Data for Instance %s (Map %d, Instance Id %d) completed.", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA(a)          debug_log("SD2: Loading Instance Data for Instance %s (Map %d, Instance Id %d). Input is '%s'", instance->GetMapName(), instance->GetId(), instance->GetInstanceId(), a)
#define OUT_LOAD_INST_DATA_COMPLETE    debug_log("SD2: Instance Data Load for Instance %s (Map %d, Instance Id: %d) is complete.", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())
#define OUT_LOAD_INST_DATA_FAIL        script_error_log("Unable to load Instance Data for Instance %s (Map %d, Instance Id: %d).", instance->GetMapName(), instance->GetId(), instance->GetInstanceId())

class ScriptedInstance : public InstanceData
{
    public:
        typedef std::map<uint32, ObjectGuid> EntryGuidMap;
        typedef std::unordered_map<uint32, GuidVector> EntryGuidCollection;

        ScriptedInstance(Map* map) : InstanceData(map) {}
        ~ScriptedInstance() {}

        // Default accessor functions
        GameObject* GetSingleGameObjectFromStorage(uint32 entry, bool skipDebugLog = false) const;
        Creature* GetSingleCreatureFromStorage(uint32 entry, bool skipDebugLog = false) const;
        void GetCreatureGuidVectorFromStorage(uint32 entry, GuidVector& entryGuidVector, bool skipDebugLog = false) const;
        void GetGameObjectGuidVectorFromStorage(uint32 entry, GuidVector& entryGuidVector, bool skipDebugLog = false) const;

        // Change active state of doors or buttons
        void DoUseDoorOrButton(ObjectGuid guid, uint32 withRestoreTime = 0, bool useAlternativeState = false);
        void DoUseDoorOrButton(uint32 entry, uint32 withRestoreTime = 0, bool useAlternativeState = false);
        void DoUseOpenableObject(uint32 entry, bool open, uint32 withRestoreTime = 0, bool useAlternativeState = false);

        // Respawns a GO having negative spawntimesecs in gameobject-table
        void DoRespawnGameObject(ObjectGuid guid, uint32 timeToDespawn = MINUTE);
        void DoRespawnGameObject(uint32 entry, uint32 timeToDespawn = MINUTE);

        // Toggle the flags of a GO
        void DoToggleGameObjectFlags(ObjectGuid guid, uint32 GOflags, bool apply);
        void DoToggleGameObjectFlags(uint32 entry, uint32 GOflags, bool apply);

        // Sends world state update to all players in instance
        virtual void DoUpdateWorldState(uint32 stateId, uint32 stateData);

        // Get a Player from map
        Player* GetPlayerInMap(bool bOnlyAlive = false, bool bCanBeGamemaster = true) const;

        // Wrapper for simulating map-wide text in this instance. It is expected that the Creature is stored in m_npcEntryGuidStore if loaded.
        void DoOrSimulateScriptTextForThisInstance(int32 textEntry, uint32 creatureEntry)
        {
            // Prevent debug output in GetSingleCreatureFromStorage
            DoOrSimulateScriptTextForMap(textEntry, creatureEntry, instance, GetSingleCreatureFromStorage(creatureEntry, true));
        }

        void BanPlayersIfNoGm(const std::string& reason);

    protected:
        void DespawnGuids(GuidVector& spawns); // despawns all creature guids and clears contents
        void RespawnDbGuids(std::vector<uint32>& spawns, uint32 respawnDelay); // respawns all dbguid creatures

        // Storage for GO-Guids and NPC-Guids
        EntryGuidMap m_goEntryGuidStore;                   // Store unique GO-Guids by entry
        EntryGuidMap m_npcEntryGuidStore;                  // Store unique NPC-Guids by entry
        EntryGuidCollection m_npcEntryGuidCollection;      // Store all Guids by entry
        EntryGuidCollection m_goEntryGuidCollection;       // Store all Guids by entry
};

// Class for world maps (May need additional zone-wide functions later on)
class ScriptedMap : public ScriptedInstance
{
    public:
        ScriptedMap(Map* map) : ScriptedInstance(map) {}
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntry
{
    int32 textEntry;                                       ///< To be said text entry
    uint32 sayerEntry;                                    ///< Entry of the mob who should say
    uint32 timer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// A static const array of this structure must be handled to DialogueHelper
struct DialogueEntryTwoSide
{
    int32 textEntry;                                       ///< To be said text entry (first side)
    uint32 sayerEntry;                                    ///< Entry of the mob who should say (first side)
    int32 textEntryAlt;                                    ///< To be said text entry (second side)
    uint32 sayerEntryAlt;                                 ///< Entry of the mob who should say (second side)
    uint32 timer;                                         ///< Time delay until next text of array is said (0 stops)
};

/// Helper class handling a dialogue given as static const array of DialogueEntry or DialogueEntryTwoSide
class DialogueHelper
{
    public:
        // The array MUST be terminated by {0,0,0}
        DialogueHelper(DialogueEntry const* dialogueArray);
        // The array MUST be terminated by {0,0,0,0,0}
        DialogueHelper(DialogueEntryTwoSide const* dialogueTwoSideArray);
        virtual ~DialogueHelper() = default;

        /// Function to initialize the dialogue helper for instances. If not used with instances, GetSpeakerByEntry MUST be overwritten to obtain the speakers
        void InitializeDialogueHelper(ScriptedInstance* instance, bool canSimulateText = false) { m_instance = instance; m_canSimulate = canSimulateText; }
        /// Set if take first entries or second entries
        void SetDialogueSide(bool isFirstSide) { m_isFirstSide = isFirstSide; }

        void StartNextDialogueText(int32 textEntry);

        void DialogueUpdate(uint32 diff);

        // for use on death or respawn applicably
        void DisableDialogue() { m_timer = 0; }

    protected:
        /// Will be called when a dialogue step was done
        virtual void JustDidDialogueStep(int32 /*entry*/) {}
        /// Will be called to get a speaker, MUST be implemented if not used in instances
        virtual Creature* GetSpeakerByEntry(uint32 /*entry*/) { return nullptr; }

    private:
        void DoNextDialogueStep();

        ScriptedInstance* m_instance;

        DialogueEntry const* m_dialogueArray;
        DialogueEntry const* m_currentEntry;
        DialogueEntryTwoSide const* m_dialogueTwoSideArray;
        DialogueEntryTwoSide const* m_currentEntryTwoSide;

        uint32 m_timer;
        bool m_isFirstSide;
        bool m_canSimulate;
};

#endif
