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
    TYPE_AURIUS                 = 7,
    TYPE_BLACK_GUARDS           = 8,
    TYPE_POSTMASTER             = 9,

    NPC_TIMMY_THE_CRUEL         = 10808,
    NPC_BARTHILAS               = 10435,
    NPC_BARONESS_ANASTARI       = 10436,
    NPC_NERUBENKAN              = 10437,
    NPC_MALEKI_THE_PALLID       = 10438,
    NPC_RAMSTEIN                = 10439,
    NPC_BARON                   = 10440,
    NPC_BALNAZZAR               = 10813,
    NPC_CRYSTAL                 = 10415,                    // Three ziggurat crystals
    NPC_THUZADIN_ACOLYTE        = 10399,                    // Acolytes in ziggurats
    NPC_ABOM_BILE               = 10416,
    NPC_ABOM_VENOM              = 10417,
    NPC_MINDLESS_UNDEAD         = 11030,                    // Zombies summoned after Ramstein
    NPC_BLACK_GUARD             = 10394,                    // Zombies summoned after Ramstein
    NPC_YSIDA                   = 16031,
    // NPC_YSIDA_TRIGGER        = 16100,
    NPC_CRIMSON_INITIATE        = 10420,                    // A couple of them related to spawn Timmy
    NPC_CRIMSON_GALLANT         = 10424,
    NPC_CRIMSON_GUARDSMAN       = 10418,
    NPC_CRIMSON_CONJURER        = 10419,
    NPC_CRIMSON_MONK            = 11043,
    NPC_CRIMSON_SORCERER        = 10422,
    NPC_CRIMSON_BATTLE_MAGE     = 10425,
    NPC_AURIUS                  = 10917,
    NPC_UNDEAD_POSTMAN          = 11142,
    NPC_SKELETAL_GUARDIAN       = 10390,
    NPC_SKELETAL_BERSERKER      = 10391,

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
    QUEST_MEDALLION_FAITH       = 5122,

    SPELL_BARON_ULTIMATUM       = 27861,                    // Aura for player during the run (visual icon)
    SPELL_BARON_SOUL_DRAIN      = 27640,                    // Used by the Baron to kill Ysida
    SPELL_YSIDA_FREED           = 27773,                    // Argent Dawn extra-reputation reward on successful Baron run
    SPELL_SUMMON_POSTMASTER     = 24627,

    SAY_ANNOUNCE_ZIGGURAT_1     = -1329004,
    SAY_ANNOUNCE_ZIGGURAT_2     = -1329005,
    SAY_ANNOUNCE_ZIGGURAT_3     = -1329006,
    SAY_ANNOUNCE_RIVENDARE      = -1329007,

    SAY_WARN_BARON              = -1329008,
    SAY_ANNOUNCE_RUN_START      = -1329009,
    SAY_ANNOUNCE_RUN_10_MIN     = -1329010,
    SAY_ANNOUNCE_RUN_5_MIN      = -1329011,
    SAY_YSIDA_RUN_5_MIN         = -1329019,
    SAY_ANNOUNCE_RUN_FAIL       = -1329012,
    SAY_YSIDA_RUN_FAIL          = -1329020,
    SAY_ANNOUNCE_RAMSTEIN       = -1329013,
    SAY_UNDEAD_DEFEAT           = -1329014,
    SAY_EPILOGUE                = -1329015,

    SAY_AURIUS_AGGRO            = -1329021,
    SAY_AURIUS_DEATH            = -1329022,

    // Used in the Scarlet Bastion defense events
    YELL_BASTION_BARRICADE      = -1329023,
    YELL_BASTION_STAIRS         = -1329024,
    YELL_BASTION_ENTRANCE       = -1329025,
    YELL_BASTION_HALL_LIGHTS    = -1329026,
    YELL_BASTION_INNER_1        = -1329027,
    YELL_BASTION_INNER_2        = -1329028,
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
    {4044.78f,  -3333.68f,  115.53f,  4.15f},               // Ysida summon loc
    {4041.9f,   -3337.6f,   115.06f,  3.82f}                // Ysida move/death loc
};

// Scarlet Bastion defense events
enum
{
    BARRICADE           = 0,
    STAIRS              = 1,
    TIMMY               = 2,
    ENTRANCE            = 3,
    HALL_OF_LIGHTS      = 4,
    INNER_BASTION_1     = 5,
    INNER_BASTION_2     = 6,
    CRIMSON_THRONE      = 7,
    FIRST_BARRICADES    = 7,

    MAX_DEFENSE_POINTS  = 8
};

static const int ScarletEventYells[] =
{
    YELL_BASTION_BARRICADE, YELL_BASTION_STAIRS, 0, YELL_BASTION_ENTRANCE,
    YELL_BASTION_HALL_LIGHTS, YELL_BASTION_INNER_1, YELL_BASTION_INNER_2
};

static const EventLocation aDefensePoints[] =
{
    {3662.84f, -3175.15f, 126.559f},                        // Last barricade before the Scarlet Bastion (Courtyard)
    {3661.57f, -3157.80f, 128.945f},                        // Scarlet Bastion stairs (Courtyard)
    {3668.60f, -3183.31f, 126.215f},                        // Courtyard mobs sort point (Timmy)
    {3646.49f, -3072.84f, 134.207f},                        // Scarlet Bastion entrance corridor
    {3599.28f, -3107.91f, 134.204f},                        // Hall of Lights
    {3485.98f, -3087.02f, 135.080f},                        // Inner Bastion: first corridor
    {3436.74f, -3090.19f, 135.085f},                        // Inner Bastion: Second corridor
    {3661.89f, -3192.89f, 126.691f}                         // Barricades before the last one (Courtyard): in last position for conveniently iterate over the table
};

static const EventLocation aScarletGuards[] =               // Spawned when players cleared some parts of the Bastion
{
    {3598.213f, -3094.812f, 135.657f, 5.3425f},             // Hall of Lights
    {3602.198f, -3096.243f, 134.120f},
    {3598.152f, -3098.927f, 134.120f},
    {3432.967f, -3069.643f, 136.529f, 5.3425f},             // Inner Bastion
    {3441.795f, -3077.431f, 135.000f},
    {3437.445f, -3080.316f, 135.000f}
};

static const EventLocation aScourgeInvaders[] =             // Attack when players cleared some parts of the Bastion
{
    {3614.702f, -3187.642f, 131.406f, 4.024f},              // Timmy. TIMMYYYYYYY !!!
    {3647.36f, -3139.70f, 134.78f, 2.1962f},                // Entrance
    {3642.50f, -3106.24f, 314.18f, 0.5210f},
    {3547.85f, -3076.46f, 135.00f, 2.2269f},                // Inner Bastion
    {3512.21f, -3066.85f, 135.00f, 3.7136f},
    {3492.44f, -3077.72f, 135.00f, 2.1680f},                // Crimson Throne
    {3443.18f, -3083.90f, 135.01f, 2.1563f}
};

static const EventLocation aScarletLastStand[] =            // Positions remaining Scarlet defenders will move to if the position is lost
{
    {3658.43f, -3178.07f, 126.696f, 5.23599f},              // Last barricade
    {3665.62f, -3173.88f, 126.551f, 5.02655f},
    {3662.84f, -3175.15f, 126.559f, 5.11381f},
    {3656.91f, -3161.94f, 128.359f, 5.39307f},              // Bastion stairs
    {3664.29f, -3157.06f, 128.357f, 5.18363f},
    {3661.57f, -3157.80f, 128.945f, 5.23599f}
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

        void OnPlayerEnter(Player* pPlayer) override;

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

        void DoSpawnScarletGuards(uint8 uiStep, Player* pSummoner);
        void DoSpawnScourgeInvaders(uint8 uiStep, Player* pSummoner);
        void DoMoveBackDefenders(uint8 uiStep, Creature* pCreature);
        void DoScarletBastionDefense(uint8 uiStep, Creature* pCreature);

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiBaronRunTimer;
        uint32 m_uiBarthilasRunTimer;
        uint32 m_uiMindlessSummonTimer;
        uint32 m_uiSlaugtherSquareTimer;
        uint32 m_uiAuriusSummonTimer;

        uint32 m_uiYellCounter;
        uint32 m_uiMindlessCount;
        uint8 m_uiPostboxesUsed;

        ZigguratStore m_zigguratStorage[MAX_ZIGGURATS];

        GuidList m_suiCrimsonDefendersLowGuids[MAX_DEFENSE_POINTS];
        GuidList m_luiCrystalGUIDs;
        GuidSet m_sAbomnationGUID;
        GuidList m_luiAcolyteGUIDs;
        GuidList m_luiUndeadGUIDs;
        GuidList m_luiGuardGUIDs;
};

#endif
