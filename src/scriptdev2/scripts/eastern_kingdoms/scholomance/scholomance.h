/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SCHOLOMANCE_H
#define DEF_SCHOLOMANCE_H

enum
{
    MAX_ENCOUNTER           = 10,
    MAX_EVENTS              = 6,
    MAX_GROUPS              = 4,
    MAX_NPC_PER_GROUP       = 4,

    TYPE_KIRTONOS           = 0,
    TYPE_RATTLEGORE         = 1,
    TYPE_RAS_FROSTWHISPER   = 2,
    TYPE_MALICIA            = 3,
    TYPE_THEOLEN            = 4,
    TYPE_POLKELT            = 5,
    TYPE_RAVENIAN           = 6,
    TYPE_ALEXEI_BAROV       = 7,
    TYPE_ILLUCIA_BAROV      = 8,
    TYPE_GANDLING           = 9,

    NPC_NECROFIEND          = 11551,
    NPC_RISEN_ABERRATION    = 10485,
    NPC_DISEASED_GHOUL      = 10495,
    NPC_REANIMATED_CORPSE   = 10481,

    NPC_KIRTONOS            = 10506,
    NPC_RATTLEGORE          = 11622,
    NPC_RAS_FROSTWHISPER    = 10508,
    NPC_THEOLEN_KRASTINOV   = 11261,
    NPC_LOREKEEPER_POLKELT  = 10901,
    NPC_RAVENIAN            = 10507,
    NPC_ILLUCIA_BAROV       = 10502,
    NPC_ALEXEI_BAROV        = 10504,
    NPC_INSTRUCTOR_MALICIA  = 10505,
    NPC_DARKMASTER_GANDLING = 1853,
    NPC_RISEN_GUARDIAN      = 11598,                        // summoned in random rooms by gandling

    GO_GATE_KIRTONOS        = 175570,
    GO_VIEWING_ROOM_DOOR    = 175167,                       // Must be opened in reload case
    GO_GATE_RAS             = 177370,
    GO_GATE_MALICIA         = 177375,
    GO_GATE_THEOLEN         = 177377,
    GO_GATE_POLKELT         = 177376,
    GO_GATE_RAVENIAN        = 177372,
    GO_GATE_BAROV           = 177373,
    GO_GATE_ILLUCIA         = 177371,
    GO_GATE_GANDLING        = 177374,

    // Because the shadow portal teleport coordinates are guesswork (taken from old script) these IDs might be randomized
    // TODO Syncronise with correct DB coordinates when they will be known
    EVENT_ID_POLKELT        = 5618,
    EVENT_ID_THEOLEN        = 5619,
    EVENT_ID_MALICIA        = 5620,
    EVENT_ID_ILLUCIA        = 5621,
    EVENT_ID_BAROV          = 5622,
    EVENT_ID_RAVENIAN       = 5623,

    SAY_GANDLING_SPAWN      = -1289000,
};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const SpawnLocation aGandlingSpawnLocs[1] =
{
    {180.771f, -5.4286f, 75.5702f, 1.29154f}
};

// Coordinates used to respawn the NPCs in the entrance room
// (right before the Viewing Room) on Rattlegore's death
static const SpawnLocation aEntranceRoomSpawnLocs[17] =
{
    {186.036f, 94.5f, 104.72f, 1.29154f},      // First corner
    {179.117f, 95.5166f, 104.81f, 1.29154f},
    {180.612f, 100.176f, 104.80f, 1.29154f},
    {185.926f, 100.079f, 104.80f, 1.29154f},
    {178.999f, 75.2952f, 104.72f, 1.29154f},   // Second corner
    {185.558f, 77.276f, 104.72f, 1.29154f},
    {187.556f, 70.4334f, 104.72f, 1.29154f},
    {180.51f, 82.3917f, 104.72f, 1.29154f},
    {212.915f, 70.6005f, 104.80f, 1.29154f},   // Third corner
    {221.199f, 77.0037f, 104.72f, 1.29154f},
    {214.381f, 76.233f, 104.80f, 1.29154f},
    {218.64f, 71.5957f, 104.72f, 1.29154f},
    {221.249f, 94.9361f, 104.72f, 1.29154f},   // Fourth corner
    {214.406f, 101.903f, 104.72f, 1.29154f},
    {217.521f, 95.4237f, 104.72f, 1.29154f},
    {223.296f, 105.101f, 104.72f, 1.29154f},
    {209.233f, 73.2819f, 104.80f, 1.29154f}    // patrolling necrofiend
};

struct GandlingEventData
{
    GandlingEventData() : m_bIsActive(false) {}
    bool m_bIsActive;
    ObjectGuid m_doorGuid;
    std::set<uint32> m_sAddGuids;
};

struct BoxVolume
{
    float m_fCornerX;
    float m_fCornerY;
    float m_fCenterZ;
    uint32 m_uiLength;
    uint32 m_uiWidth;
};

static const BoxVolume aEntranceRoom[] = {174.13f, 63.84f, 104.0f, 54, 44};

static const uint32 aGandlingEvents[MAX_EVENTS] = {EVENT_ID_POLKELT, EVENT_ID_THEOLEN, EVENT_ID_MALICIA, EVENT_ID_ILLUCIA, EVENT_ID_BAROV, EVENT_ID_RAVENIAN};

typedef std::map<uint32, GandlingEventData> GandlingEventMap;

class instance_scholomance : public ScriptedInstance
{
    public:
        instance_scholomance(Map* pMap);
        ~instance_scholomance() {}

        void Initialize() override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature) override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;
        void OnPlayerEnter(Player* pPlayer) override;

        void HandlePortalEvent(uint32 uiEventId, uint32 uiData);

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

    private:
        void DoSpawnGandlingIfCan(bool bByPlayerEnter);
        void DoRespawnEntranceRoom(Player* pSummoner);

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiGandlingEvent;
        GandlingEventMap m_mGandlingData;

        bool m_bIsRoomReset;
        GuidSet m_sEntranceRoomGuids;
};

#endif
