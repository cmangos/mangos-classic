/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_RUINS_OF_AHNQIRAJ_H
#define DEF_RUINS_OF_AHNQIRAJ_H

enum
{
    MAX_ENCOUNTER               = 6,
    MAX_HELPERS                 = 5,

    TYPE_KURINNAXX              = 0,
    TYPE_RAJAXX                 = 1,
    TYPE_MOAM                   = 2,
    TYPE_BURU                   = 3,
    TYPE_AYAMISS                = 4,
    TYPE_OSSIRIAN               = 5,

    NPC_KURINNAXX               = 15348,
    NPC_MOAM                    = 15340,
    NPC_BURU                    = 15370,
    NPC_AYAMISS                 = 15369,
    NPC_OSSIRIAN                = 15339,
    NPC_GENERAL_ANDOROV         = 15471,                    // The general and the kaldorei are escorted for the rajaxx encounter
    NPC_KALDOREI_ELITE          = 15473,
    NPC_RAJAXX                  = 15341,                    // All of the following are used in the rajaxx encounter
    NPC_COLONEL_ZERRAN          = 15385,
    NPC_MAJOR_PAKKON            = 15388,
    NPC_MAJOR_YEGGETH           = 15386,
    NPC_CAPTAIN_XURREM          = 15390,
    NPC_CAPTAIN_DRENN           = 15389,
    NPC_CAPTAIN_TUUBID          = 15392,
    NPC_CAPTAIN_QEEZ            = 15391,
    NPC_QIRAJI_WARRIOR          = 15387,
    NPC_SWARMGUARD_NEEDLER      = 15344,

    MAX_ARMY_WAVES              = 7,

    GO_OSSIRIAN_CRYSTAL         = 180619,                   // Used in the ossirian encounter
    NPC_OSSIRIAN_TRIGGER        = 15590,                    // Triggers ossirian weakness

    SAY_OSSIRIAN_INTRO          = -1509022,                 // Yelled after Kurinnax dies

    // Rajaxx yells
    SAY_WAVE3                   = -1509005,
    SAY_WAVE4                   = -1509006,
    SAY_WAVE5                   = -1509007,
    SAY_WAVE6                   = -1509008,
    SAY_WAVE7                   = -1509009,
    SAY_INTRO                   = -1509010,
    SAY_DEAGGRO                 = -1509015,                 // on Rajaxx evade
    SAY_COMPLETE_QUEST          = -1509017,                 // Yell when realm complete quest 8743 for world event
};

struct SpawnLocation
{
    uint32 m_uiEntry;
    float m_fX, m_fY, m_fZ, m_fO;
};

// Spawn coords for Andorov and his team
static const SpawnLocation aAndorovSpawnLocs[MAX_HELPERS] =
{
    {NPC_GENERAL_ANDOROV, -8660.4f,  1510.29f, 32.449f,  2.2184f},
    {NPC_KALDOREI_ELITE,  -8655.84f, 1509.78f, 32.462f,  2.33341f},
    {NPC_KALDOREI_ELITE,  -8657.39f, 1506.28f, 32.418f,  2.33346f},
    {NPC_KALDOREI_ELITE,  -8660.96f, 1504.9f,  32.1567f, 2.33306f},
    {NPC_KALDOREI_ELITE,  -8664.45f, 1506.44f, 32.0944f, 2.33302f}
};

// Movement locations for Andorov
static const SpawnLocation aAndorovMoveLocs[] =
{
    {0, -8701.51f, 1561.80f, 32.092f},
    {0, -8718.66f, 1577.69f, 21.612f},
    {0, -8876.97f, 1651.96f, 21.57f, 5.52f},
    {0, -8882.15f, 1602.77f, 21.386f},
    {0, -8940.45f, 1550.69f, 21.616f},
};

struct SortingParameters
{
    uint32 m_uiEntry;
    int32 m_uiYellEntry;
    float m_fSearchDist;
};

static const SortingParameters aArmySortingParameters[MAX_ARMY_WAVES] =
{
    {NPC_CAPTAIN_QEEZ,   0,         20.0f},
    {NPC_CAPTAIN_TUUBID, 0,         22.0f},
    {NPC_CAPTAIN_DRENN,  SAY_WAVE3, 22.0f},
    {NPC_CAPTAIN_XURREM, SAY_WAVE4, 22.0f},
    {NPC_MAJOR_YEGGETH,  SAY_WAVE5, 20.0f},
    {NPC_MAJOR_PAKKON,   SAY_WAVE6, 21.0f},
    {NPC_COLONEL_ZERRAN, SAY_WAVE7, 17.0f},
};

class instance_ruins_of_ahnqiraj : public ScriptedInstance
{
    public:
        instance_ruins_of_ahnqiraj(Map* pMap);
        ~instance_ruins_of_ahnqiraj() {}

        void Initialize() override;

        // bool IsEncounterInProgress() const override;              // not active in AQ20

        void OnCreatureCreate(Creature* pCreature) override;
        void OnPlayerEnter(Player* pPlayer) override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void GetKaldoreiGuidList(GuidList& lList) { lList = m_lKaldoreiGuidList; }

        void Update(uint32 uiDiff) override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

    private:
        void DoSapwnAndorovIfCan();
        void DoSortArmyWaves();
        void DoSendNextArmyWave();

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        GuidList m_lKaldoreiGuidList;
        GuidSet m_sArmyWavesGuids[MAX_ARMY_WAVES];

        uint32 m_uiArmyDelayTimer;
        uint8 m_uiCurrentArmyWave;
};
#endif
