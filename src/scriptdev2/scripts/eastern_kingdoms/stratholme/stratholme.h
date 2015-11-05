/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
* This program is free software licensed under GPL version 2
* Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_STRATHOLME_H
#define DEF_STRATHOLME_H

enum
{
    MAX_ENCOUNTER               = 10,
    MAX_SILVERHAND              = 5,
    MAX_ZIGGURATS               = 3,

    TYPE_BARON_RUN              = 0,
    TYPE_BARONESS               = 1,
    TYPE_NERUB                  = TYPE_BARONESS + 1,        // Assert that these three TYPEs are in correct order.
    TYPE_PALLID                 = TYPE_BARONESS + 2,
    TYPE_RAMSTEIN               = 4,
    TYPE_BARON                  = 5,
    TYPE_BARTHILAS_RUN          = 6,
    TYPE_BLACK_GUARDS           = 7,
    TYPE_POSTMASTER             = 8,
    TYPE_TRUE_MASTERS           = 9,

    NPC_TIMMY_THE_CRUEL         = 10808,
    NPC_BARTHILAS               = 10435,
    NPC_BARONESS_ANASTARI       = 10436,
    NPC_NERUBENKAN              = 10437,
    NPC_MALEKI_THE_PALLID       = 10438,
    NPC_RAMSTEIN                = 10439,
    NPC_BARON                   = 10440,
    NPC_CRYSTAL                 = 10415,                    // Three ziggurat crystals
    NPC_THUZADIN_ACOLYTE        = 10399,                    // Acolytes in ziggurats
    NPC_ABOM_BILE               = 10416,
    NPC_ABOM_VENOM              = 10417,
    NPC_MINDLESS_UNDEAD         = 11030,                    // Zombies summoned after Ramstein
    NPC_BLACK_GUARD             = 10394,                    // Zombies summoned after Ramstein
    NPC_YSIDA                   = 16031,
    NPC_YSIDA_TRIGGER           = 16100,
    NPC_CRIMSON_INITIATE        = 10420,                    // A couple of them related to spawn Timmy
    NPC_CRIMSON_GALLANT         = 10424,
    NPC_CRIMSON_GUARDSMAN       = 10418,
    NPC_CRIMSON_CONJURER        = 10419,
    NPC_UNDEAD_POSTMAN          = 11142,
    NPC_GREGOR_THE_JUSTICIAR    = 17910,                    // related to quest "True Masters of the Light"
    NPC_CATHELA_THE_SEEKER      = 17911,
    NPC_NEMAS_THE_ARBITER       = 17912,
    NPC_AELMAR_THE_VANQUISHER   = 17913,
    NPC_VICAR_HYERONIMUS        = 17914,
    NPC_PALADIN_QUEST_CREDIT    = 17915,

    GO_SERVICE_ENTRANCE         = 175368,
    GO_GAUNTLET_GATE1           = 175357,
    GO_PORT_SLAUGHTER_GATE      = 175358,                   // Port used at the undeads event
    GO_ZIGGURAT_DOOR_1          = 175380,                   // Baroness
    GO_ZIGGURAT_DOOR_2          = 175379,                   // Nerub'enkan
    GO_ZIGGURAT_DOOR_3          = 175381,                   // Maleki
    GO_ZIGGURAT_DOOR_4          = 175405,                   // Ramstein
    GO_ZIGGURAT_DOOR_5          = 175796,                   // Baron
    GO_PORT_GAUNTLET            = 175374,                   // Port from gauntlet to slaugther
    GO_PORT_SLAUGTHER           = 175373,                   // Port at slaugther
    GO_PORT_ELDERS              = 175377,                   // Port at elders square
    GO_YSIDA_CAGE               = 181071,                   // Cage to open after baron event is done

    QUEST_DEAD_MAN_PLEA         = 8945,
    SPELL_BARON_ULTIMATUM       = 27861,
    SPELL_SUMMON_POSTMASTER     = 24627,

    SAY_ANNOUNCE_ZIGGURAT_1     = -1329004,
    SAY_ANNOUNCE_ZIGGURAT_2     = -1329005,
    SAY_ANNOUNCE_ZIGGURAT_3     = -1329006,
    SAY_ANNOUNCE_RIVENDARE      = -1329007,

    SAY_WARN_BARON              = -1329008,
    SAY_ANNOUNCE_RUN_START      = -1329009,
    SAY_ANNOUNCE_RUN_10_MIN     = -1329010,
    SAY_ANNOUNCE_RUN_5_MIN      = -1329011,
    SAY_ANNOUNCE_RUN_FAIL       = -1329012,
    SAY_ANNOUNCE_RAMSTEIN       = -1329013,
    SAY_UNDEAD_DEFEAT           = -1329014,
    SAY_EPILOGUE                = -1329015,
};

struct EventLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const EventLocation aStratholmeLocation[] =
{
    {3725.577f, -3599.484f, 142.367f},                      // Barthilas door run
    {4068.284f, -3535.678f, 122.771f, 2.50f},               // Barthilas tele
    {4032.643f, -3378.546f, 119.752f, 4.74f},               // Ramstein summon loc
    {4032.843f, -3390.246f, 119.732f},                      // Ramstein move loc
    {3969.357f, -3391.871f, 119.116f, 5.91f},               // Skeletons summon loc
    {4033.044f, -3431.031f, 119.055f},                      // Skeletons move loc
    {4032.602f, -3378.506f, 119.752f, 4.74f},               // Guards summon loc
    {4042.575f, -3337.929f, 115.059f}                       // Ysida move loc
};

static const EventLocation aTimmyLocation[] =
{
    {3696.851f, -3152.736f, 127.661f, 4.024f},              // Timmy spawn loc
    {3668.603f, -3183.314f, 126.215f}                       // Courtyard mobs sort point
};

struct ZigguratStore
{
    ObjectGuid m_doorGuid;
    ObjectGuid m_crystalGuid;
    GuidList m_lZigguratAcolyteGuid;
};

class instance_stratholme : public ScriptedInstance
{
    public:
        instance_stratholme(Map* pMap);
        ~instance_stratholme() {}

        void Initialize() override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature) override;

        void Update(uint32 uiDiff) override;

    protected:
        bool StartSlaugtherSquare();
        void DoSortZiggurats();
        void ThazudinAcolyteJustDied(Creature* pCreature);

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiBaronRunTimer;
        uint32 m_uiBarthilasRunTimer;
        uint32 m_uiMindlessSummonTimer;
        uint32 m_uiSlaugtherSquareTimer;

        uint32 m_uiYellCounter;
        uint32 m_uiMindlessCount;
        uint8 m_uiPostboxesUsed;
        uint8 m_uiSilverHandKilled;

        ZigguratStore m_zigguratStorage[MAX_ZIGGURATS];

        std::set<uint32> m_suiCrimsonLowGuids;
        GuidList m_luiCrystalGUIDs;
        GuidSet m_sAbomnationGUID;
        GuidList m_luiAcolyteGUIDs;
        GuidList m_luiUndeadGUIDs;
        GuidList m_luiGuardGUIDs;
};

#endif
