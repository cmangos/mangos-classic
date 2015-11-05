/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_BRD_H
#define DEF_BRD_H

enum
{
    MAX_ENCOUNTER           = 13,
    MAX_RELIC_DOORS         = 12,
    MAX_DWARFS              = 7,
    MAX_DWARF_RUNES         = 7,
    MAX_CRONIES             = 3,

    TYPE_RING_OF_LAW        = 1,
    TYPE_VAULT              = 2,
    TYPE_ROCKNOT            = 3,
    TYPE_TOMB_OF_SEVEN      = 4,
    TYPE_LYCEUM             = 5,
    TYPE_IRON_HALL          = 6,
    TYPE_QUEST_JAIL_BREAK   = 7,
    TYPE_FLAMELASH          = 8,
    TYPE_HURLEY             = 9,
    TYPE_BRIDGE             = 10,
    TYPE_BAR                = 11,
    TYPE_PLUGGER            = 12,
    TYPE_NAGMARA            = 13,

    NPC_EMPEROR             = 9019,
    NPC_PRINCESS            = 8929,
    NPC_PRIESTESS           = 10076,
    NPC_HATEREL             = 9034,
    NPC_ANGERREL            = 9035,
    NPC_VILEREL             = 9036,
    NPC_GLOOMREL            = 9037,
    NPC_SEETHREL            = 9038,
    NPC_DOOMREL             = 9039,
    NPC_DOPEREL             = 9040,
    NPC_MAGMUS              = 9938,
    NPC_WATCHER_DOOMGRIP    = 9476,
    NPC_WARBRINGER_CONST    = 8905,                         // Four of them in Relict Vault are related to Doomgrip summon event
    NPC_LOREGRAIN           = 9024,
    NPC_FIREGUARD_DESTROYER = 8911,

    // Jail Break event related
    NPC_OGRABISI            = 9677,
    NPC_SHILL               = 9678,
    NPC_CREST               = 9680,
    NPC_JAZ                 = 9681,
    NPC_TOBIAS              = 9679,
    NPC_DUGHAL              = 9022,

    // Arena crowd
    NPC_ARENA_SPECTATOR     = 8916,
    NPC_SHADOWFORGE_PEASANT = 8896,
    NPC_SHADOWFORGE_CITIZEN = 8902,
    NPC_SHADOWFORGE_SENATOR = 8904,
    NPC_ANVILRAGE_SOLDIER   = 8893,
    NPC_ANVILRAGE_MEDIC     = 8894,
    NPC_ANVILRAGE_OFFICER   = 8895,

    // Spawned on Shadowforge bridge
    NPC_ANVILRAGE_GUARDMAN  = 8891,

    // Grim Guzzler bar events
    NPC_PHALANX             = 9502,
    NPC_GRIM_PATRON         = 9545,
    NPC_GUZZLING_PATRON     = 9547,
    NPC_HAMMERED_PATRON     = 9554,
    NPC_HURLEY_BLACKBREATH  = 9537,
    NPC_BLACKBREATH_CRONY   = 9541,
    NPC_PLUGGER_SPAZZRING   = 9499,
    NPC_PRIVATE_ROCKNOT     = 9503,
    NPC_MISTRESS_NAGMARA    = 9500,
    NPC_RIBBLY_SCREWSPIGGOT = 9543,

    GO_ARENA_1              = 161525,
    GO_ARENA_2              = 161522,
    GO_ARENA_3              = 161524,
    GO_ARENA_4              = 161523,

    GO_SHADOW_LOCK          = 161460,
    GO_SHADOW_MECHANISM     = 161461,
    GO_SHADOW_GIANT_DOOR    = 157923,
    GO_SHADOW_DUMMY         = 161516,
    GO_BAR_KEG_SHOT         = 170607,
    GO_BAR_KEG_TRAP         = 171941,
    GO_BAR_DOOR             = 170571,
    GO_TOMB_ENTER           = 170576,
    GO_TOMB_EXIT            = 170577,
    GO_LYCEUM               = 170558,
    GO_GOLEM_ROOM_N         = 170573,
    GO_GOLEM_ROOM_S         = 170574,
    GO_THRONE_ROOM          = 170575,

    GO_SPECTRAL_CHALICE     = 164869,
    GO_CHEST_SEVEN          = 169243,
    GO_ARENA_SPOILS         = 181074,
    GO_SECRET_DOOR          = 174553,
    GO_SECRET_SAFE          = 161495,

    // Jail break event related
    GO_JAIL_DOOR_SUPPLY     = 170561,
    GO_JAIL_SUPPLY_CRATE    = 166872,

    GO_DWARFRUNE_A01        = 170578,
    GO_DWARFRUNE_B01        = 170579,
    GO_DWARFRUNE_C01        = 170580,
    GO_DWARFRUNE_D01        = 170581,
    GO_DWARFRUNE_E01        = 170582,
    GO_DWARFRUNE_F01        = 170583,
    GO_DWARFRUNE_G01        = 170584,

    QUEST_ROYAL_RESCUE      = 4003,                         // horde quest
    QUEST_FATE_KINGDOM      = 4362,                         // alliance quest

    SPELL_STONED            = 10255,                        // Aura of Warbringer Constructs in Relict Vault

    FACTION_DWARF_HOSTILE   = 754,                          // Hostile faction for the Tomb of the Seven dwarfs
    FACTION_ARENA_NEUTRAL   = 15,                           // Neutral faction for NPC in top of Arena after event complete
    FACTION_DARK_IRON       = 54,                           // Hostile faction for the Grim Guzzler
    FACTION_IRONFORGE       = 122,                          // Faction used by Phalanx when Plugger is killed (yes, friendly to Alliance, Hostile to Horde, very strange)

    // enum used to handle the various Grim Guzzler  bar patron's reaction
    // depending on the actions and events triggered by players
    PATRON_EMOTE            = 0,
    PATRON_PISSED           = 1,
    PATRON_HOSTILE          = 2,
    SAY_PISSED_PATRON_1     = -1230037,
    SAY_PISSED_PATRON_2     = -1230038,
    SAY_PISSED_PATRON_3     = -1230039,
    SAY_ROCKNOT_DESPAWN     = -1230047,
    YELL_PATROL_1           = -1230048,
    YELL_PATROL_2           = -1230049,
    SPELL_NAGMARA_VANISH    = 15341,
    
    // Emperor Dagran Thaurissan
    YELL_SENATOR_1          = -1230060,
    YELL_SENATOR_2          = -1230061,
    YELL_SENATOR_3          = -1230062,
    YELL_SENATOR_4          = -1230063
};

// Random emotes for Grim Guzzler patrons
static const uint32 aPatronsEmotes[] =
{
    EMOTE_ONESHOT_EXCLAMATION, EMOTE_ONESHOT_CHEER, EMOTE_ONESHOT_CHEER, EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_LAUGH
};

struct ArenaCylinder
{
    float m_fCenterX;
    float m_fCenterY;
    float m_fCenterZ;
    uint32 m_uiRadius;
    uint32 m_uiHeight;
};

static const ArenaCylinder aArenaCrowdVolume[] = {595.78f, -188.65f, -38.63f, 69, 10};

enum ArenaNPCs
{
    // Gladiators
    NPC_LEFTY               = 16049,
    NPC_ROTFANG             = 16050,
    NPC_SNOKH               = 16051,
    NPC_MALGEN              = 16052,
    NPC_KORV                = 16053,
    NPC_REZZNIK             = 16054,
    NPC_VAJASHNI            = 16055,
    NPC_VOLIDA              = 16058,
    NPC_THELDREN            = 16059,
    // Ring mobs
    NPC_WORM                = 8925,
    NPC_STINGER             = 8926,
    NPC_SCREECHER           = 8927,
    NPC_THUNDERSNOUT        = 8928,
    NPC_CREEPER             = 8933,
    NPC_BEETLE              = 8932,
    // Ring bosses
    NPC_GOROSH              = 9027,
    NPC_GRIZZLE             = 9028,
    NPC_EVISCERATOR         = 9029,
    NPC_OKTHOR              = 9030,
    NPC_ANUBSHIAH           = 9031,
    NPC_HEDRUM              = 9032
};

static const uint32 aArenaNPCs[] =
{
    // Gladiators
    NPC_LEFTY, NPC_ROTFANG, NPC_SNOKH, NPC_MALGEN, NPC_KORV, NPC_REZZNIK, NPC_VAJASHNI, NPC_VOLIDA, NPC_THELDREN,
    // Ring mobs
    NPC_WORM, NPC_STINGER, NPC_SCREECHER, NPC_THUNDERSNOUT, NPC_CREEPER, NPC_BEETLE,
    // Ring bosses
    NPC_GOROSH, NPC_GRIZZLE, NPC_EVISCERATOR, NPC_OKTHOR, NPC_ANUBSHIAH, NPC_HEDRUM
};

// Used to summon Watcher Doomgrip
static const float aVaultPositions[4] = {821.905f, -338.382f, -50.134f, 3.78736f};

// Used to summon Hurley Blackbreath
static const float aHurleyPositions[4] = {856.0867f, -149.7469f, -49.6719f, 0.05949629f};

// Used to summon the patrol in Grim Guzzler
static const float aBarPatrolPositions[2][4] = {
    {872.7059f, -232.5491f, -43.7525f, 2.069044f},
    {865.5645f, -219.7471f, -43.7033f, 2.033881f}
};

static const uint32 aBarPatrolId[3] = {NPC_FIREGUARD_DESTROYER, NPC_ANVILRAGE_OFFICER, NPC_ANVILRAGE_OFFICER};

// Tomb of the Seven dwarfs
static const uint32 aTombDwarfes[MAX_DWARFS] = {NPC_ANGERREL, NPC_SEETHREL, NPC_DOPEREL, NPC_GLOOMREL, NPC_VILEREL, NPC_HATEREL, NPC_DOOMREL};

class instance_blackrock_depths : public ScriptedInstance
{
    public:
        instance_blackrock_depths(Map* pMap);
        ~instance_blackrock_depths() {}

        void Initialize() override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureDeath(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature);
        void OnObjectCreate(GameObject* pGo) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(uint32 uiDiff) override;

        // Arena Event
        void SetArenaCenterCoords(float fX, float fY, float fZ) { m_fArenaCenterX = fX; m_fArenaCenterY = fY; m_fArenaCenterZ = fZ; }
        void GetArenaCenterCoords(float& fX, float& fY, float& fZ) { fX = m_fArenaCenterX; fY = m_fArenaCenterY; fZ = m_fArenaCenterZ; }
        void GetArenaCrowdGuid(GuidSet& sCrowdSet) { sCrowdSet = m_sArenaCrowdNpcGuids; }

        // Bar events
        void SetBarDoorIsOpen() { m_bIsBarDoorOpen = true; }
        void GetBarDoorIsOpen(bool& bIsOpen) { bIsOpen = m_bIsBarDoorOpen; }
        void HandleBarPatrons(uint8 uiEventType);
        void HandleBarPatrol(uint8 uiStep);

    private:
        void DoCallNextDwarf();
        bool CanReplacePrincess();

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        // Grim Guzzler bar events
        bool  m_bIsBarDoorOpen;
        uint32 m_uiBarAleCount;
        uint32 m_uiPatronEmoteTimer;
        uint8 m_uiBrokenKegs;
        uint32 m_uiPatrolTimer;
        uint8 m_uiStolenAles;
        uint32 m_uiDagranTimer;

        uint8 m_uiCofferDoorsOpened;

        uint8 m_uiDwarfRound;
        uint32 m_uiDwarfFightTimer;

        float m_fArenaCenterX, m_fArenaCenterY, m_fArenaCenterZ;

        GuidSet m_sVaultNpcGuids;
        GuidSet m_sArenaCrowdNpcGuids;
        GuidSet m_sBarPatronNpcGuids;
        GuidSet m_sBarPatrolGuids;
};

#endif
