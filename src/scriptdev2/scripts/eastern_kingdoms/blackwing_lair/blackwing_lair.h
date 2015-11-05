/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_BLACKWING_LAIR
#define DEF_BLACKWING_LAIR

enum
{
    MAX_ENCOUNTER               = 8,

    TYPE_RAZORGORE              = 0,
    TYPE_VAELASTRASZ            = 1,
    TYPE_LASHLAYER              = 2,
    TYPE_FIREMAW                = 3,
    TYPE_EBONROC                = 4,
    TYPE_FLAMEGOR               = 5,
    TYPE_CHROMAGGUS             = 6,
    TYPE_NEFARIAN               = 7,

    DATA_DRAGON_EGG             = 1,                        // track the used eggs

    NPC_RAZORGORE               = 12435,
    NPC_VAELASTRASZ             = 13020,
    NPC_LASHLAYER               = 12017,
    NPC_FIREMAW                 = 11983,
    NPC_EBONROC                 = 14601,
    NPC_FLAMEGOR                = 11981,
    NPC_CHROMAGGUS              = 14020,
    NPC_NEFARIAN                = 11583,
    NPC_LORD_VICTOR_NEFARIUS    = 10162,
    NPC_BLACKWING_TECHNICIAN    = 13996,                    // Flees at Vael intro event

    // Razorgore event related
    NPC_GRETHOK_CONTROLLER      = 12557,
    NPC_BLACKWING_ORB_TRIGGER   = 14449,
    NPC_NEFARIANS_TROOPS        = 14459,
    NPC_MONSTER_GENERATOR       = 12434,
    NPC_BLACKWING_LEGIONNAIRE   = 12416,                    // one spawn per turn
    NPC_BLACKWING_MAGE          = 12420,                    // one spawn per turn
    NPC_DRAGONSPAWN             = 12422,                    // two spawns per turn

    GO_DOOR_RAZORGORE_ENTER     = 176964,
    GO_DOOR_RAZORGORE_EXIT      = 176965,
    GO_DOOR_NEFARIAN            = 176966,
    // GO_DOOR_CHROMAGGUS_ENTER  = 179115,
    // GO_DOOR_CHROMAGGUS_SIDE   = 179116,
    GO_DOOR_CHROMAGGUS_EXIT     = 179117,
    GO_DOOR_VAELASTRASZ         = 179364,
    GO_DOOR_LASHLAYER           = 179365,
    GO_ORB_OF_DOMINATION        = 177808,                   // trigger 19832 on Razorgore
    GO_BLACK_DRAGON_EGG         = 177807,
    GO_DRAKONID_BONES           = 179804,

    EMOTE_ORB_SHUT_OFF          = -1469035,
    EMOTE_TROOPS_FLEE           = -1469033,                 // emote by Nefarian's Troops npc

    MAX_EGGS_DEFENDERS          = 4,
};

// Coords used to spawn Nefarius at the throne
static const float aNefariusSpawnLoc[4] = { -7466.16f, -1040.80f, 412.053f, 2.14675f};

static const uint32 aRazorgoreSpawns[MAX_EGGS_DEFENDERS] = {NPC_BLACKWING_LEGIONNAIRE, NPC_BLACKWING_MAGE, NPC_DRAGONSPAWN, NPC_DRAGONSPAWN};

class instance_blackwing_lair : public ScriptedInstance
{
    public:
        instance_blackwing_lair(Map* pMap);
        ~instance_blackwing_lair() {}

        void Initialize() override;
        bool IsEncounterInProgress() const override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureDeath(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void SetData64(uint32 uiData, uint64 uiGuid) override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(uint32 uiDiff) override;

    protected:
        std::string m_strInstData;
        uint32 m_auiEncounter[MAX_ENCOUNTER];

        uint32 m_uiResetTimer;
        uint32 m_uiDefenseTimer;

        GuidList m_lTechnicianGuids;
        GuidList m_lDragonEggsGuids;
        GuidList m_lDrakonidBonesGuids;
        GuidList m_lDefendersGuids;
        GuidList m_lUsedEggsGuids;
        GuidVector m_vGeneratorGuids;
};

#endif
