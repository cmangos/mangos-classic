/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_BLACKROCK_SPIRE_H
#define DEF_BLACKROCK_SPIRE_H

enum
{
    MAX_ENCOUNTER               = 6,
    MAX_ROOMS                   = 7,

    TYPE_ROOM_EVENT             = 0,
    TYPE_EMBERSEER              = 1,
    TYPE_FLAMEWREATH            = 2,                        // Only summon once per instance
    TYPE_STADIUM                = 3,
    TYPE_DRAKKISATH             = 4,
    TYPE_VALTHALAK              = 5,                        // Only summon once per instance

    NPC_SCARSHIELD_INFILTRATOR  = 10299,
    NPC_BLACKHAND_SUMMONER      = 9818,
    NPC_BLACKHAND_VETERAN       = 9819,
    NPC_PYROGUARD_EMBERSEER     = 9816,
    NPC_SOLAKAR_FLAMEWREATH     = 10264,
    NPC_BLACKHAND_INCARCERATOR  = 10316,
    NPC_BLACKHAND_ELITE         = 10317,
    NPC_LORD_VICTOR_NEFARIUS    = 10162,
    NPC_REND_BLACKHAND          = 10429,
    NPC_GYTH                    = 10339,
    NPC_THE_BEAST               = 10430,
    NPC_DRAKKISATH              = 10363,
    NPC_CHROMATIC_WHELP         = 10442,                    // related to Gyth arena event
    NPC_CHROMATIC_DRAGON        = 10447,
    NPC_BLACKHAND_HANDLER       = 10742,

    // Doors
    GO_EMBERSEER_IN             = 175244,
    GO_DOORS                    = 175705,
    GO_EMBERSEER_OUT            = 175153,
    GO_FATHER_FLAME             = 175245,
    GO_GYTH_ENTRY_DOOR          = 164726,
    GO_GYTH_COMBAT_DOOR         = 175185,
    GO_GYTH_EXIT_DOOR           = 175186,
    GO_DRAKKISATH_DOOR_1        = 175946,
    GO_DRAKKISATH_DOOR_2        = 175947,

    // upper spire entrance
    GO_DRAGONSPINE              = 164725,
    GO_BRAZIER_1                = 175528,
    GO_BRAZIER_2                = 175529,
    GO_BRAZIER_3                = 175530,
    GO_BRAZIER_4                = 175531,
    GO_BRAZIER_5                = 175532,
    GO_BRAZIER_6                = 175533,

    GO_ROOM_7_RUNE              = 175194,
    GO_ROOM_3_RUNE              = 175195,
    GO_ROOM_6_RUNE              = 175196,
    GO_ROOM_1_RUNE              = 175197,
    GO_ROOM_5_RUNE              = 175198,
    GO_ROOM_2_RUNE              = 175199,
    GO_ROOM_4_RUNE              = 175200,

    GO_ROOKERY_EGG              = 175124,

    GO_EMBERSEER_RUNE_1         = 175266,
    GO_EMBERSEER_RUNE_2         = 175267,
    GO_EMBERSEER_RUNE_3         = 175268,
    GO_EMBERSEER_RUNE_4         = 175269,
    GO_EMBERSEER_RUNE_5         = 175270,
    GO_EMBERSEER_RUNE_6         = 175271,
    GO_EMBERSEER_RUNE_7         = 175272,

    ITEM_SEAL_OF_ASCENSION      = 12344,

    MAX_STADIUM_WAVES           = 7,
    MAX_STADIUM_MOBS_PER_WAVE   = 5,

    FACTION_BLACK_DRAGON        = 103
};

static const Position aStadiumLocs[7] =
{
    {210.00f, -420.30f, 110.94f, 3.14f},                    // dragons summon location
    {210.14f, -397.54f, 111.1f,  0.f},                      // Gyth summon location
    {163.62f, -420.33f, 110.47f, 0.f},                      // center of the stadium location (for movement)
    {164.63f, -444.04f, 121.97f, 3.22f},                    // Lord Nefarius summon position
    {161.01f, -443.73f, 121.97f, 6.26f},                    // Rend summon position
    {164.64f, -443.30f, 121.97f, 1.61f},                    // Nefarius move position
    {165.74f, -466.46f, 116.80f, 0.f},                      // Rend move position
};

static const Position aBeastLocs[3] =
{
    {98.09f, -563.45f, 109.86f, 2.37f},                     // Summon points
    {99.81f, -561.47f, 109.24f, 2.43f},
    {96.92f, -560.98f, 110.18f, 2.43f},
};

static const uint32 aStadiumSpectators[12] =
{
    NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_ELITE, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN,
    NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_ELITE, NPC_BLACKHAND_VETERAN, NPC_BLACKHAND_VETERAN
};

static const Position aSpectatorsSpawnLocs[12] =
{
    {163.3209f, -340.9818f, 111.0216f, 4.818223f},
    {164.2471f, -339.0313f, 111.0368f, 1.413717f},
    {161.124f, -339.5178f, 111.0381f, 3.001966f},
    {162.5045f, -337.8101f, 111.0367f, 4.13643f},
    {160.9896f, -337.7715f, 111.0368f, 1.117011f},
    {161.8347f, -335.7923f, 111.0352f, 2.286381f},
    {113.9726f, -366.0805f, 116.9195f, 6.252025f},
    {112.7245f, -368.9635f, 116.9307f, 4.677482f},
    {110.5757f, -368.2123f, 116.9278f, 4.310963f},
    {109.3343f, -366.4785f, 116.9261f, 2.740167f},
    {110.1331f, -363.9824f, 116.9272f, 0.5235988f},
    {111.9971f, -363.0948f, 116.929f, 5.951573f},
};

static const Position aSpectatorsTargetLocs[12] =
{
    {160.619f, -395.826f, 121.9752f, -1.502597f},
    {162.1428f, -395.1175f, 121.9751f, -1.67753f},
    {158.6822f, -395.7097f, 121.9753f, -1.787977f},
    {164.384f, -395.3787f, 121.9751f, -1.502597f},
    {156.9669f, -395.2188f, 121.9752f, -1.678662f},
    {166.2515f, -395.0366f, 121.975f, -1.791467f},
    {143.814f, -396.7092f, 121.9753f, -1.40136f},
    {145.3893f, -396.1959f, 121.9752f, -1.419479f},
    {142.1598f, -396.0284f, 121.9752f, -1.661444f},
    {147.7274f, -396.3042f, 121.9753f, -1.40136f},
    {139.9446f, -396.7277f, 121.9753f, -1.428414f},
    {149.3754f, -395.7497f, 121.9753f, -1.714769f},
};

// Stadium event description
static const uint32 aStadiumEventNpcs[MAX_STADIUM_WAVES][5] =
{
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER, 0},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
    {NPC_CHROMATIC_WHELP, NPC_CHROMATIC_WHELP, NPC_CHROMATIC_DRAGON, NPC_CHROMATIC_DRAGON, NPC_BLACKHAND_HANDLER},
};

class instance_blackrock_spire : public ScriptedInstance, private DialogueHelper
{
    public:
        instance_blackrock_spire(Map* pMap);
        ~instance_blackrock_spire() {}

        void Initialize() override;

        void OnObjectCreate(GameObject* pGo) override;
        void OnCreatureCreate(Creature* pCreature) override;

        void OnCreatureDeath(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature) override;
        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureDespawn(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void DoUseEmberseerRunes(bool bReset = false);
        void DoProcessEmberseerEvent();

        void DoOpenUpperDoorIfCan(Player* pPlayer);
        void DoSortRoomEventMobs();
        void GetIncarceratorGUIDList(GuidList& lList) const { lList = m_lIncarceratorGUIDList; }

        void StartflamewreathEventIfCan();

        void Update(const uint32 diff) override;

        bool m_bBeastIntroDone;
        bool m_bBeastOutOfLair;

    private:
        void JustDidDialogueStep(int32 iEntry) override;
        void DoSendNextStadiumWave();
        void DoSendNextFlamewreathWave();

        bool m_bUpperDoorOpened;

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiDragonspineDoorTimer;
        uint32 m_uiDragonspineGoCount;
        uint32 m_uiFlamewreathEventTimer;
        uint32 m_uiFlamewreathWaveCount;
        uint32 m_uiStadiumEventTimer;
        uint8 m_uiStadiumWaves;
        uint8 m_uiStadiumMobsAlive;

        ObjectGuid m_aRoomRuneGuid[MAX_ROOMS];
        GuidList m_alRoomEventMobGUIDSorted[MAX_ROOMS];
        GuidList m_lRoomEventMobGUIDList;
        GuidList m_lIncarceratorGUIDList;
        GuidList m_lEmberseerRunesGUIDList;
        GuidList m_lStadiumSpectatorsGUIDList;
};

#endif
