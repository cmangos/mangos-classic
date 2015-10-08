/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_RFD_H
#define DEF_RFD_H

enum
{
    MAX_ENCOUNTER               = 1,
    MAX_WAVES                   = 3,
    MAX_COUNT_POS               = 2,

    TYPE_TUTEN_KASH             = 0,

    NPC_TOMB_FIEND              = 7349,
    NPC_TOMB_REAVER             = 7351,
    NPC_TUTEN_KASH              = 7355,

    GO_GONG                     = 148917,
};

struct Locations
{
    float m_fX, m_fY, m_fZ, m_fO;
};

static const Locations aSpawnLocations[MAX_COUNT_POS] =
{
    {2484.83f, 811.11f, 43.40f, 1.67f},                 // Right corridor
    {2546.03f, 902.77f, 47.16f, 5.04f},                 // Left corridor
};

struct SummonInformation
{
    uint32 m_uiNpcEntry;
    uint8 m_uiNPCperWave;
};

static const SummonInformation aWaveSummonInformation[] =
{
    {NPC_TOMB_FIEND,  8},
    {NPC_TOMB_REAVER, 4},
    {NPC_TUTEN_KASH,  1}
};

class instance_razorfen_downs : public ScriptedInstance
{
    public:
        instance_razorfen_downs(Map* pMap);
        ~instance_razorfen_downs() {}

        void Initialize() override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;
        void OnCreatureDeath(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void DoSpawnWaveIfCan(GameObject* pGo);

    protected:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint8 m_uiWaveCounter;

        GuidList m_lSpawnedMobsList;
};

#endif
