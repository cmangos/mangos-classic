/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_MOLTEN_CORE_H
#define DEF_MOLTEN_CORE_H

enum
{
    MAX_ENCOUNTER               = 10,

    TYPE_LUCIFRON               = 0,
    TYPE_MAGMADAR               = 1,
    TYPE_GEHENNAS               = 2,
    TYPE_GARR                   = 3,
    TYPE_SHAZZRAH               = 4,
    TYPE_GEDDON                 = 5,
    TYPE_GOLEMAGG               = 6,
    TYPE_SULFURON               = 7,
    TYPE_MAJORDOMO              = 8,
    TYPE_RAGNAROS               = 9,

    NPC_LUCIFRON                = 12118,
    NPC_MAGMADAR                = 11982,
    NPC_GEHENNAS                = 12259,
    NPC_GARR                    = 12057,
    NPC_SHAZZRAH                = 12264,
    NPC_GEDDON                  = 12056,
    NPC_GOLEMAGG                = 11988,
    NPC_SULFURON                = 12098,
    NPC_MAJORDOMO               = 12018,
    NPC_RAGNAROS                = 11502,

    // Adds
    // Used for respawn in case of wipe
    NPC_FLAMEWAKER_PROTECTOR    = 12119,                    // Lucifron
    NPC_FLAMEWAKER              = 11661,                    // Gehennas
    NPC_FIRESWORN               = 12099,                    // Garr
    NPC_CORE_RAGER              = 11672,                    // Golemagg
    NPC_FLAMEWAKER_PRIEST       = 11662,                    // Sulfuron
    NPC_FLAMEWAKER_HEALER       = 11663,                    // Majordomo
    NPC_FLAMEWAKER_ELITE        = 11664,                    // Majordomo

    GO_LAVA_STEAM               = 178107,
    GO_LAVA_SPLASH              = 178108,
    GO_CACHE_OF_THE_FIRE_LORD   = 179703,
    GO_RUNE_KRESS               = 176956,                   // Magmadar
    GO_RUNE_MOHN                = 176957,                   // Gehennas
    GO_RUNE_BLAZ                = 176955,                   // Garr
    GO_RUNE_MAZJ                = 176953,                   // Shazzrah
    GO_RUNE_ZETH                = 176952,                   // Geddon
    GO_RUNE_THERI               = 176954,                   // Golemagg
    GO_RUNE_KORO                = 176951,                   // Sulfuron
    GO_CIRCLE_MAGMADAR          = 178192,                   // Flames circle around each rune, removed when each boss is killed
    GO_CIRCLE_GEHENNAS          = 178193,
    GO_CIRCLE_GARR              = 178191,
    GO_CIRCLE_SHAZZRAH          = 178189,
    GO_CIRCLE_BARON_GEDDON      = 178188,
    GO_CIRCLE_GOLEMAGG          = 178190,
    GO_CIRCLE_SULFURON          = 178187,

    MAX_MOLTEN_RUNES            = 7,
    MAX_MAJORDOMO_ADDS          = 8,
    FACTION_MAJORDOMO_FRIENDLY  = 1080,
    SAY_MAJORDOMO_SPAWN         = -1409004,
};

struct sRuneEncounters
{
    uint32 m_uiRuneEntry, m_uiFlamesCircleEntry, m_uiType;
};

static const sRuneEncounters m_aMoltenCoreRunes[MAX_MOLTEN_RUNES] =
{
    {GO_RUNE_KRESS, GO_CIRCLE_MAGMADAR,     TYPE_MAGMADAR},
    {GO_RUNE_MOHN,  GO_CIRCLE_GEHENNAS,     TYPE_GEHENNAS},
    {GO_RUNE_BLAZ,  GO_CIRCLE_GARR,         TYPE_GARR},
    {GO_RUNE_MAZJ,  GO_CIRCLE_SHAZZRAH,     TYPE_SHAZZRAH},
    {GO_RUNE_ZETH,  GO_CIRCLE_BARON_GEDDON, TYPE_GEDDON},
    {GO_RUNE_THERI, GO_CIRCLE_GOLEMAGG,     TYPE_GOLEMAGG},
    {GO_RUNE_KORO,  GO_CIRCLE_SULFURON,     TYPE_SULFURON}
};

struct sSpawnLocation
{
    uint32 m_uiEntry;
    float m_fX, m_fY, m_fZ, m_fO;
};

static sSpawnLocation m_aMajordomoLocations[2] =
{
    {NPC_MAJORDOMO, 758.0892f, -1176.712f, -118.6403f, 3.124139f},  // Summon fight position
    {NPC_MAJORDOMO, 848.933f, -812.875f, -229.601f, 4.046f}     // Summon and teleport location (near Ragnaros)
};

class instance_molten_core : public ScriptedInstance
{
    public:
        instance_molten_core(Map* pMap);
        ~instance_molten_core() {}

        void Initialize() override;
        bool IsEncounterInProgress() const override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;
        void OnPlayerEnter(Player* pPlayer) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void ShowChatCommands(ChatHandler* handler) override;
        void ExecuteChatCommand(ChatHandler* handler, char* args) override;

    protected:
        void DoSpawnMajordomoIfCan(bool bByPlayerEnter);
        void SpawnMajordomo(Unit* summoner, bool initialSummon, uint8 summonPos);

        std::string m_strInstData;
        uint32 m_auiEncounter[MAX_ENCOUNTER];
};

#endif
