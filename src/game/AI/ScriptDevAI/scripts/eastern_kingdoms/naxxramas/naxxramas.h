/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_NAXXRAMAS_H
#define DEF_NAXXRAMAS_H

enum
{
    MAX_ENCOUNTER               = 16,

    // A few instance-script related texts
    SAY_THADDIUS_GREET          = -1533029,

    // Background screams in Instance if Thaddius still alive, four of them from 8873 to 8876
    SOUND_SCREAM1               = 8873,

    // Kel'Thuzad
    SAY_KELTHUZAD_CAT_DIED      = -1533089,
    SAY_KELTHUZAD_SHACKLES_1    = -1533106,
    SAY_KELTHUZAD_SHACKLES_2    = -1533108,
    // Kel'Thuzad's taunts after killing Wing Bosses
    SAY_KELTHUZAD_TAUNT1        = -1533090,
    SAY_KELTHUZAD_TAUNT2        = -1533091,
    SAY_KELTHUZAD_TAUNT3        = -1533092,
    SAY_KELTHUZAD_TAUNT4        = -1533093,
    EMOTE_FLEE                  = -1533159,
    // Dialogues with Lich King
    SAY_SAPP_DIALOG1            = -1533084,
    SAY_SAPP_DIALOG2_LICH       = -1533085,
    SAY_SAPP_DIALOG3            = -1533086,
    SAY_SAPP_DIALOG4_LICH       = -1533087,
    SAY_SAPP_DIALOG5            = -1533088,
    // Horsemen dialogue texts
    SAY_BLAU_TAUNT1             = -1533045,
    SAY_BLAU_TAUNT2             = -1533046,
    SAY_BLAU_TAUNT3             = -1533047,
    SAY_MORG_TAUNT1             = -1533071,
    SAY_MORG_TAUNT2             = -1533072,
    SAY_MORG_TAUNT3             = -1533073,
    SAY_KORT_TAUNT1             = -1533052,
    SAY_KORT_TAUNT2             = -1533053,
    SAY_KORT_TAUNT3             = -1533054,
    SAY_ZELI_TAUNT1             = -1533059,
    SAY_ZELI_TAUNT2             = -1533060,
    SAY_ZELI_TAUNT3             = -1533061,
    // Grand Widow Faerlina intro
    SAY_FAERLINA_INTRO          = -1533009,
    FOLLOWERS_STAND             = 1,
    FOLLOWERS_AURA              = 2,
    FOLLOWERS_KNEEL             = 3,

    TYPE_ANUB_REKHAN            = 0,
    TYPE_FAERLINA               = 1,
    TYPE_MAEXXNA                = 2,

    TYPE_NOTH                   = 3,
    TYPE_HEIGAN                 = 4,
    TYPE_LOATHEB                = 5,

    TYPE_RAZUVIOUS              = 6,
    TYPE_GOTHIK                 = 7,
    TYPE_FOUR_HORSEMEN          = 8,

    TYPE_PATCHWERK              = 9,
    TYPE_GROBBULUS              = 10,
    TYPE_GLUTH                  = 11,
    TYPE_THADDIUS               = 12,

    TYPE_SAPPHIRON              = 13,
    TYPE_KELTHUZAD              = 14,

    NPC_ANUB_REKHAN             = 15956,
    NPC_FAERLINA                = 15953,
    NPC_NAXXRAMAS_CULTIST       = 15980,
    NPC_NAXXRAMAS_ACOLYTE       = 15981,
    NPC_CORPSE_SCARAB           = 16698,

    NPC_ZOMBIE_CHOW             = 16360,
    NPC_GLUTH                   = 15932,
    NPC_THADDIUS                = 15928,
    NPC_STALAGG                 = 15929,
    NPC_FEUGEN                  = 15930,
    NPC_TESLA_COIL              = 16218,
    NPC_NAXXRAMAS_TRIGGER       = 16082,
    NPC_TOXIC_TUNNEL            = 16400,

    NPC_MOGRAINE                = 16062,
    NPC_ZELIEK                  = 16063,
    NPC_THANE                   = 16064,
    NPC_BLAUMEUX                = 16065,

    NPC_SPIRIT_OF_MOGRAINE      = 16775,
    NPC_SPIRIT_OF_BLAUMEUX      = 16776,
    NPC_SPIRIT_OF_ZELIREK       = 16777,
    NPC_SPIRIT_OF_KORTHAZZ      = 16778,

    NPC_SAPPHIRON               = 15989,
    NPC_KELTHUZAD               = 15990,
    NPC_THE_LICHKING            = 16980,
    NPC_MR_BIGGLESWORTH         = 16998,
    NPC_LIVING_POISON           = 16027,

    // Gothik
    NPC_GOTHIK                  = 16060,
    NPC_SUB_BOSS_TRIGGER        = 16137,                    // summon locations
    NPC_UNREL_TRAINEE           = 16124,
    NPC_UNREL_DEATH_KNIGHT      = 16125,
    NPC_UNREL_RIDER             = 16126,
    NPC_SPECT_TRAINEE           = 16127,
    NPC_SPECT_DEATH_KNIGHT      = 16148,
    NPC_SPECT_RIDER             = 16150,
    NPC_SPECT_HORSE             = 16149,

    // Kel'Thuzad
    NPC_SOLDIER_FROZEN          = 16427,
    NPC_UNSTOPPABLE_ABOM        = 16428,
    NPC_SOUL_WEAVER             = 16429,
    NPC_GUARDIAN                = 16441,

    // Arachnid Quarter
    GO_ARAC_ANUB_DOOR           = 181126,                   // encounter door
    GO_ARAC_ANUB_GATE           = 181195,                   // open after boss is dead
    GO_ARAC_FAER_WEB            = 181235,                   // encounter door
    GO_ARAC_FAER_DOOR           = 181167,                   // after faerlina, to outer ring
    GO_ARAC_MAEX_INNER_DOOR     = 181197,                   // encounter door
    GO_ARAC_MAEX_OUTER_DOOR     = 181209,                   // right before maex

    // Plague Quarter
    GO_PLAG_SLIME01_DOOR        = 181198,                   // not used
    GO_PLAG_SLIME02_DOOR        = 181199,                   // not used
    GO_PLAG_NOTH_ENTRY_DOOR     = 181200,                   // encounter door
    GO_PLAG_NOTH_EXIT_DOOR      = 181201,                   // exit, open when boss dead
    GO_PLAG_HEIG_ENTRY_DOOR     = 181202,
    GO_PLAG_HEIG_EXIT_DOOR      = 181203,                   // exit door - not used here
    GO_PLAG_HEIG_EXIT_HALLWAY   = 181496,                   // exit door, at the end of the hallway
    GO_PLAG_LOAT_DOOR           = 181241,                   // encounter door

    // Military Quarter
    GO_MILI_GOTH_ENTRY_GATE     = 181124,                   // used while encounter is in progress
    GO_MILI_GOTH_EXIT_GATE      = 181125,                   // exit, open at boss dead
    GO_MILI_GOTH_COMBAT_GATE    = 181170,                   // used while encounter is in progress
    GO_MILI_HORSEMEN_DOOR       = 181119,                   // encounter door

    GO_CHEST_HORSEMEN_NORM      = 181366,                   // four horsemen event, DoRespawnGameObject() when event == DONE

    // Construct Quarter
    GO_CONS_PATH_EXIT_DOOR      = 181123,
    GO_CONS_GLUT_EXIT_DOOR      = 181120,
    GO_CONS_THAD_DOOR           = 181121,                   // Thaddius enc door
    GO_CONS_NOX_TESLA_FEUGEN    = 181477,
    GO_CONS_NOX_TESLA_STALAGG   = 181478,

    // Frostwyrm Lair
    GO_KELTHUZAD_WATERFALL_DOOR = 181225,                   // exit, open after sapphiron is dead
    GO_KELTHUZAD_EXIT_DOOR      = 181228,
    GO_KELTHUZAD_WINDOW_1       = 181402,
    GO_KELTHUZAD_WINDOW_2       = 181403,
    GO_KELTHUZAD_WINDOW_3       = 181404,
    GO_KELTHUZAD_WINDOW_4       = 181405,
    GO_KELTHUZAD_TRIGGER        = 181444,

    // Eyes
    GO_ARAC_EYE_RAMP            = 181212,
    GO_PLAG_EYE_RAMP            = 181211,
    GO_MILI_EYE_RAMP            = 181210,
    GO_CONS_EYE_RAMP            = 181213,

    GO_ARAC_EYE_BOSS            = 181233,
    GO_PLAG_EYE_BOSS            = 181231,
    GO_MILI_EYE_BOSS            = 181230,
    GO_CONS_EYE_BOSS            = 181232,

    // Portals
    GO_ARAC_PORTAL              = 181575,
    GO_PLAG_PORTAL              = 181577,
    GO_MILI_PORTAL              = 181578,
    GO_CONS_PORTAL              = 181576,

    AREATRIGGER_FROSTWYRM       = 4120,                     // not needed here, but AT to be scripted
    AREATRIGGER_KELTHUZAD       = 4112,
    AREATRIGGER_GOTHIK          = 4116,
    AREATRIGGER_THADDIUS_DOOR   = 4113,
    AREATRIGGER_FROSTWYRM_TELE  = 4156,
    AREATRIGGER_FAERLINA_INTRO  = 4115,

    // Gothik summon steps
    STEP_TRAINEE                = 0,
    STEP_KNIGHT                 = 1,
    STEP_RIDER                  = 2,
    SPELL_SUMMON_TRAINEE        = 28007,                    // Triggers 27884 every 20s
    SPELL_SUMMON_KNIGHT         = 28009,                    // Triggers 28008 every 25s
    SPELL_SUMMON_MOUNTED_KNIGHT = 28011,                    // Triggers 28010 every 30s
    
    SPELL_SPECTRAL_ASSAULT      = 28781,
    SPELL_UNRELENTING_ASSAULT   = 29874,

    EVENT_ID_DECIMATE           = 10495,
    EVENT_CLEAR_SHACKLES        = 10536,
    EVENT_GUARDIAN_SHACKLE      = 10537,

    SPELL_DARK_CHANNELING       = 21157,
    SPELL_CHANNEL_VISUAL        = 29423,                    // Periodically trigger 29422
    SPELL_CLEAR_ALL_SHACKLES    = 29910,                    // Cast by Kel'Thuzad if more than three Guardians of Icecrown are controlled

    MAX_SHACKLES                = 3,                        // How many Guardians of Icecrown can be crowed control without Kel'Thuzad dispelling the shackles

    SPELL_EXPLODE               = 28433,                    // Used by Living Poison blobs when players come in range
};

enum GothikSpellDummy
{
    SPELL_A_TO_ANCHOR_1         = 27892,
    SPELL_B_TO_ANCHOR_1         = 27928,
    SPELL_C_TO_ANCHOR_1         = 27935,

    SPELL_A_TO_ANCHOR_2         = 27893,
    SPELL_B_TO_ANCHOR_2         = 27929,
    SPELL_C_TO_ANCHOR_2         = 27936,

    SPELL_A_TO_SKULL            = 27915,
    SPELL_B_TO_SKULL            = 27931,
    SPELL_C_TO_SKULL            = 27937
};

enum GothikSummonFlag {
    SUMMON_FLAG_TRAINEE         = 0x01,
    SUMMON_FLAG_KNIGHT          = 0x02,
    SUMMON_FLAG_RIDER           = 0x04
};

struct GothTrigger
{
    bool isRightSide;
    bool isAnchorHigh;
    int summonTypeFlag;
};

static const float sapphironPositions[4] = {3521.48f, -5234.87f, 137.626f, 4.53329f};

struct SpawnLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
};

// Used in Construct Quarter for the deadly blobs continuously spawning in Patchwerk corridor
static const SpawnLocation livingPoisonPositions[6] =
{
    {3128.692f, -3119.211f, 293.346f, 4.725505f},
    {3154.432f, -3125.669f, 293.408f, 4.456693f},
    {3175.614f, -3134.716f, 293.282f, 4.244928f},
    {3128.709f, -3157.404f, 293.3238f, 4.725505f},
    {3145.881f, -3158.563f, 293.3216f, 4.456693f},
    {3157.736f, -3164.859f, 293.2874f, 4.244928f},
};

class instance_naxxramas : public ScriptedInstance, private DialogueHelper
{
    public:
        instance_naxxramas(Map* pMap);
        ~instance_naxxramas() {}

        void Initialize() override;

        bool IsEncounterInProgress() const override;

        void OnPlayerEnter(Player* player) override;
        void OnCreatureCreate(Creature* creature) override;
        void OnObjectCreate(GameObject* gameObject) override;

        void OnCreatureDeath(Creature* creature) override;

        void OnCreatureRespawn(Creature* creature) override;

        void SetData(uint32 type, uint32 data) override;
        uint32 GetData(uint32 type) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(const uint32 diff) override;

        // Gothik
        void InitializeGothikTriggers();
        bool IsSuitableTriggerForSummon(Unit* trigger, uint8 flag);
        Creature* GetClosestAnchorForGothik(Creature* source, bool rightSide);
        void GetGothikSummonPoints(CreatureList& lList, bool rightSide);
        bool IsInRightSideGothikArea(Unit* pUnit);

        // Kel'Thuzad
        void DoTaunt();

        bool DoHandleEvent(uint32 eventId);

        bool DoHandleAreaTrigger(AreaTriggerEntry const* areaTrigger);

    protected:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        GuidList m_thaddiusTeslaCoilList;
        GuidList m_gothikTriggerList;
        GuidList m_zombieChowList;
        GuidList m_faerlinaFollowersList;
        GuidList m_unrelentingSideList;
        GuidList m_spectralSideList;
        GuidList m_icrecrownGuardianList;

        std::unordered_map<ObjectGuid, GothTrigger> m_gothikTriggerMap;

        uint32 m_sapphironSpawnTimer;
        uint32 m_tauntTimer;
        uint8 m_horsemenKilled;
        uint32 m_livingPoisonTimer;
        uint32 m_screamsTimer;
        uint32 m_horsemenTauntTimer;
        uint32 m_despawnKTTriggerTimer;
        uint32 m_checkGuardiansTimer;
        uint32 m_shackledGuardians;

        bool isFaerlinaIntroDone;

        void JustDidDialogueStep(int32 entry) override;
};

#endif
