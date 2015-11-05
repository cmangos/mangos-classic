/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SCHOLOMANCE_H
#define DEF_SCHOLOMANCE_H

enum
{
    MAX_ENCOUNTER           = 10,
    MAX_EVENTS              = 6,

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
    NPC_BONE_MINION         = 16119,                        // summoned in random rooms by gandling

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

struct GandlingEventData
{
    GandlingEventData() : m_bIsActive(false) {}
    bool m_bIsActive;
    ObjectGuid m_doorGuid;
    std::set<uint32> m_sAddGuids;
};

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

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiGandlingEvent;
        GandlingEventMap m_mGandlingData;
};

#endif
