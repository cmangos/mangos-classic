/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_ZULFARRAK_H
#define DEF_ZULFARRAK_H

enum
{
    MAX_ENCOUNTER                   = 9,

    TYPE_VELRATHA                   = 0,
    TYPE_GAHZRILLA                  = 1,
    TYPE_ANTUSUL                    = 2,
    TYPE_THEKA                      = 3,
    TYPE_ZUMRAH                     = 4,
    TYPE_NEKRUM                     = 5,
    TYPE_SEZZZIZ                    = 6,
    TYPE_CHIEF_SANDSCALP            = 7,
    TYPE_PYRAMID_EVENT              = 8,

    NPC_VELRATHA                    = 7795,
    NPC_GAHZRILLA                   = 7273,
    NPC_ANTUSUL                     = 8127,
    NPC_THEKA                       = 7272,
    NPC_ZUMRAH                      = 7271,
    NPC_NEKRUM                      = 7796,
    NPC_SEZZZIZ                     = 7275,
    NPC_CHIEF_SANDSCALP             = 7267,

    NPC_SERGEANT_BLY                = 7604,
    NPC_SANDFURY_SLAVE              = 7787,
    NPC_SANDFURY_DRUDGE             = 7788,
    NPC_SANDFURY_CRETIN             = 7789,
    NPC_SANDFURY_ACOLYTE            = 8876,
    NPC_SANDFURY_ZEALOT             = 8877,

    GO_SHALLOW_GRAVE                = 128403,
    GO_END_DOOR                     = 146084,

    // EVENT_ID_GONG_ZULFARRAK      = 2488,                 // go 141832
    // EVENT_ID_UNLOCKING           = 2609,                 // spell 10738

    AREATRIGGER_ANTUSUL             = 1447,
};

class instance_zulfarrak : public ScriptedInstance
{
    public:
        instance_zulfarrak(Map* pMap);
        ~instance_zulfarrak() {}

        void Initialize() override;

        void OnCreatureEnterCombat(Creature* pCreature) override;
        void OnCreatureEvade(Creature* pCreature);
        void OnCreatureDeath(Creature* pCreature) override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void GetShallowGravesGuidList(GuidList& lList) const { lList = m_lShallowGravesGuidList; }

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(const uint32 diff) override;

    protected:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        GuidList m_lShallowGravesGuidList;
        GuidList m_lPyramidTrollsGuidList;

        uint32 m_uiPyramidEventTimer;
};

#endif
