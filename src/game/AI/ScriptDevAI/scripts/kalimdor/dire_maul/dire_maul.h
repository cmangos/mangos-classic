/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_DM_H
#define DEF_DM_H

enum
{
    MAX_ENCOUNTER               = 19,
    MAX_GENERATORS              = 5,
    MAX_PORTALS                 = 19,

    // East
    TYPE_ALZZIN                 = 0,                        // Do not change - Handled with Acid
    TYPE_ZEVRIM                 = 1,
    TYPE_IRONBARK               = 2,

    // West
    TYPE_WARPWOOD               = 3,
    TYPE_IMMOLTHAR              = 4,
    TYPE_PRINCE                 = 5,
    TYPE_PYLON_1                = 6,
    TYPE_PYLON_2                = TYPE_PYLON_1 + 1,
    TYPE_PYLON_3                = TYPE_PYLON_1 + 2,
    TYPE_PYLON_4                = TYPE_PYLON_1 + 3,
    TYPE_PYLON_5                = TYPE_PYLON_1 + 4,

    // North
    TYPE_KING_GORDOK            = 11,
    TYPE_MOLDAR                 = 12,
    TYPE_FENGUS                 = 13,
    TYPE_SLIPKIK                = 14,
    TYPE_KROMCRUSH              = 15,
    TYPE_CHORUSH                = 16,
    TYPE_STOMPER_KREEG          = 17,

    // Other
    TYPE_DREADSTEED             = 18,

    // East
    GO_CRUMBLE_WALL             = 177220,
    GO_CORRUPT_VINE             = 179502,
    GO_FELVINE_SHARD            = 179559,
    GO_CONSERVATORY_DOOR        = 176907,                   // Opened by Ironbark the redeemed

    NPC_ZEVRIM_THORNHOOF        = 11490,
    NPC_OLD_IRONBARK            = 11491,
    NPC_IRONBARK_REDEEMED       = 14241,

    // West
    NPC_TENDRIS_WARPWOOD        = 11489,
    NPC_PRINCE_TORTHELDRIN      = 11486,
    NPC_IMMOLTHAR               = 11496,
    NPC_ARCANE_ABERRATION       = 11480,
    NPC_MANA_REMNANT            = 11483,
    NPC_HIGHBORNE_SUMMONER      = 11466,
    NPC_WARLOCK_DUMMY_INFERNAL  = 14501,
    NPC_JEEVEE                  = 14500,

    GO_PRINCES_CHEST            = 179545,
    GO_PRINCES_CHEST_AURA       = 179563,
    GO_CRYSTAL_GENERATOR_1      = 177259,
    GO_CRYSTAL_GENERATOR_2      = 177257,
    GO_CRYSTAL_GENERATOR_3      = 177258,
    GO_CRYSTAL_GENERATOR_4      = 179504,
    GO_CRYSTAL_GENERATOR_5      = 179505,
    GO_FORCEFIELD               = 179503,
    GO_WARPWOOD_DOOR            = 177221,
    GO_WEST_LIBRARY_DOOR        = 179550,

    GO_BELL_OF_DETHMOORA        = 179674,
    GO_DOOMSDAY_CANDLE          = 179673,
    GO_WHEEL_OF_BLACK_MARCH     = 179672,
    GO_DREADSTEED_PORTAL        = 179681,
    GO_WARLOCK_RITUAL_CIRCLE    = 179668,
    GO_MOUNT_QUEST_SYMBOL1      = 179669,
    GO_MOUNT_QUEST_SYMBOL2      = 179670,
    GO_MOUNT_QUEST_SYMBOL3      = 179671,
    GO_RITUAL_CANDLE_AURA       = 179688, // invis trap - true caster of 23226

    SPELL_BLACK_MARCH_BLESSING  = 23120,
    SPELL_RITUAL_CANDLE_AURA    = 23226,
    SPELL_RITUAL_BELL_AURA      = 23117,

    SPELL_TELEPORT              = 29216,

    NPC_XOROTHIAN_IMP           = 14482,
    NPC_DREAD_GUARD             = 14483,
    NPC_LORD_HELNURATH          = 14506,

    SAY_UNSUMMON_DEMON          = -1429005,

    // North
    NPC_GUARD_MOLDAR            = 14326,
    NPC_STOMPER_KREEG           = 14322,
    NPC_GUARD_FENGUS            = 14321,
    NPC_GUARD_SLIPKIK           = 14323,
    NPC_CAPTAIN_KROMCRUSH       = 14325,
    NPC_CHO_RUSH_THE_OBSERVER   = 14324,
    NPC_KING_GORDOK             = 11501,
    NPC_MIZZLE_THE_CRAFTY       = 14353,

    GO_KNOTS_CACHE              = 179501,
    GO_KNOTS_BALL_AND_CHAIN     = 179511,
    GO_GORDOK_TRIBUTE           = 179564,
    GO_NORTH_LIBRARY_DOOR       = 179549,

    SAY_FREE_IMMOLTHAR          = -1429000,
    SAY_KILL_IMMOLTHAR          = -1429001,
    SAY_IRONBARK_REDEEM         = -1429002,
    SAY_CHORUSH_KING_DEAD       = -1429003,
    SAY_SLIPKIK_TRAP            = -1429004,

    FACTION_HOSTILE             = 14,
    FACTION_FRIENDLY            = 35,
    FACTION_OGRE                = 45,

    SPELL_KING_OF_GORDOK        = 22799,
    SPELL_ICE_LOCK              = 22856,
    SPELL_GUARD_SLIPKIK_TRIGGER = 31770,
};

static const float afMizzleSpawnLoc[4] = {683.296f, 484.384f, 29.544f, 0.0174f};

class instance_dire_maul : public ScriptedInstance
{
    public:
        instance_dire_maul(Map* pMap);
        ~instance_dire_maul() {}

        void Initialize() override;

        void OnPlayerEnter(Player* pPlayer) override;
        void OnPlayerLeave(Player* pPlayer) override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureDeath(Creature* pCreature) override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        bool CheckConditionCriteriaMeet(Player const* pPlayer, uint32 uiInstanceConditionId, WorldObject const* pConditionSource, uint32 conditionSourceType) const override;

        void Update(const uint32 diff) override;
        GuidVector GetRitualSymbolGuids();
        void ProcessDreadsteedRitualStart();

    protected:
        bool CheckAllGeneratorsDestroyed();
        void ProcessForceFieldOpening();
        void SortPylonGuards();
        void PylonGuardJustDied(Creature* pCreature);

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        // East
        bool m_bWallDestroyed;
        GuidList m_lFelvineShardGUIDs;

        // West
        ObjectGuid m_aCrystalGeneratorGuid[MAX_GENERATORS];

        GuidList m_luiHighborneSummonerGUIDs;
        GuidList m_lGeneratorGuardGUIDs;
        GuidList m_lDreadsteedPortalsGUIDs;
        std::set<uint32> m_sSortedGeneratorGuards[MAX_GENERATORS];

        GuidVector m_lRitualSymbolGUIDs;

        // North
        bool m_bDoNorthBeforeWest;
};

#endif
