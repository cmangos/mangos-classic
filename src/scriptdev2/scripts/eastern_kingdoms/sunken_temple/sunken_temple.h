/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SUNKEN_TEMPLE_H
#define DEF_SUNKEN_TEMPLE_H

enum
{
    MAX_ENCOUNTER         = 5,
    MAX_STATUES           = 6,
    MAX_FLAMES            = 4,

    TYPE_ATALARION        = 0,
    TYPE_PROTECTORS       = 1,
    TYPE_JAMMALAN         = 2,
    TYPE_MALFURION        = 3,
    TYPE_AVATAR           = 4,

    NPC_ATALARION         = 8580,
    NPC_DREAMSCYTH        = 5721,
    NPC_WEAVER            = 5720,
    NPC_JAMMALAN          = 5710,
    NPC_AVATAR_OF_HAKKAR  = 8443,
    NPC_SHADE_OF_ERANIKUS = 5709,

    // Jammalain mini-bosses
    NPC_ZOLO              = 5712,
    NPC_GASHER            = 5713,
    NPC_LORO              = 5714,
    NPC_HUKKU             = 5715,
    NPC_ZULLOR            = 5716,
    NPC_MIJAN             = 5717,

    // Avatar of hakkar mobs
    NPC_SHADE_OF_HAKKAR   = 8440,                           // Shade of Hakkar appears when the event starts; will despawn when avatar of hakkar is summoned
    NPC_BLOODKEEPER       = 8438,                           // Spawned rarely and contains the hakkari blood -> used to extinguish the flames
    NPC_HAKKARI_MINION    = 8437,                           // Npc randomly spawned during the event = trash
    NPC_SUPPRESSOR        = 8497,                           // Npc summoned at one of the two doors and moves to the boss;

    NPC_MALFURION         = 15362,

    GO_ALTAR_OF_HAKKAR    = 148836,                         // Used in order to show the player the order of the statue activation
    GO_IDOL_OF_HAKKAR     = 148838,                         // Appears when atalarion is summoned; this was removed in 4.0.1

    GO_ATALAI_LIGHT       = 148883,                         // Green light, activates when the correct statue is chosen
    GO_ATALAI_LIGHT_BIG   = 148937,                         // Big light, used at the altar event

    GO_ATALAI_TRAP_1      = 177484,                         // Trapps triggered if the wrong statue is activated
    GO_ATALAI_TRAP_2      = 177485,                         // The traps are spawned in DB randomly around the statues (we don't know exactly which statue has which trap)
    GO_ATALAI_TRAP_3      = 148837,

    GO_ETERNAL_FLAME_1    = 148418,
    GO_ETERNAL_FLAME_2    = 148419,
    GO_ETERNAL_FLAME_3    = 148420,
    GO_ETERNAL_FLAME_4    = 148421,

    GO_EVIL_CIRCLE        = 148998,                         // Objects used at the avatar event. they are spawned when the event starts, and the mobs are summon atop of them
    GO_HAKKAR_DOOR_1      = 149432,                         // Combat doors
    GO_HAKKAR_DOOR_2      = 149433,

    GO_JAMMALAN_BARRIER   = 149431,

    // Event ids related to the statue activation
    EVENT_ID_STATUE_1     = 3094,
    EVENT_ID_STATUE_2     = 3095,
    EVENT_ID_STATUE_3     = 3097,
    EVENT_ID_STATUE_4     = 3098,
    EVENT_ID_STATUE_5     = 3099,
    EVENT_ID_STATUE_6     = 3100,

    SPELL_SUMMON_AVATAR   = 12639,                          // Cast by the shade of hakkar, updates entry to avatar
    SPELL_AVATAR_SUMMONED = 12948,

    SAY_JAMMALAN_INTRO    = -1109005,
    SAY_AVATAR_BRAZIER_1  = -1109006,
    SAY_AVATAR_BRAZIER_2  = -1109007,
    SAY_AVATAR_BRAZIER_3  = -1109008,
    SAY_AVATAR_BRAZIER_4  = -1109009,
    SAY_AVATAR_SPAWN      = -1109010,
};

// This is also the needed order for activation: S, N, SW, SE, NW, NE
static const uint32 m_aAtalaiStatueEvents[MAX_STATUES] = {EVENT_ID_STATUE_1, EVENT_ID_STATUE_2, EVENT_ID_STATUE_3, EVENT_ID_STATUE_4, EVENT_ID_STATUE_5, EVENT_ID_STATUE_6};

struct SummonLocations
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const SummonLocations aSunkenTempleLocation[] =
{
    { -466.5130f, 95.19820f, -189.646f, 0.0349f},           // Atalarion summon loc
    { -466.8673f, 272.31204f, -90.7441f, 3.5255f},          // Shade of hakkar summon loc
    { -660.5277f, -16.7117f, -90.8357f, 1.6055f}            // Malfurion summon loc
};

// Summon location for the suppressors
static const SummonLocations aHakkariDoorLocations[2] =
{
    { -420.629f, 276.682f, -90.827f, 0.0f},
    { -512.015f, 276.134f, -90.827f, 0.0f}
};

class instance_sunken_temple : public ScriptedInstance
{
    public:
        instance_sunken_temple(Map* pMap);
        ~instance_sunken_temple() {}

        void Initialize() override;

        void OnObjectCreate(GameObject* pGo) override;
        void OnCreatureCreate(Creature* pCreature) override;

        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void Update(uint32 uiDiff) override;

        bool ProcessStatueEvent(uint32 uiEventId);

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

    protected:
        void DoSpawnAtalarionIfCan();
        void DoUpdateFlamesFlags(bool bRestore);

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint8 m_uiProtectorsRemaining;                      // Jammalan door handling
        uint8 m_uiStatueCounter;                            // Atalarion Statue Event
        uint8 m_uiFlameCounter;                             // Avatar of Hakkar Event
        uint32 m_uiAvatarSummonTimer;
        uint32 m_uiSupressorTimer;
        bool m_bIsFirstHakkarWave;
        bool m_bCanSummonBloodkeeper;

        GuidList m_luiFlameGUIDs;
        GuidList m_luiBigLightGUIDs;
        GuidVector m_vuiCircleGUIDs;
};

#endif
