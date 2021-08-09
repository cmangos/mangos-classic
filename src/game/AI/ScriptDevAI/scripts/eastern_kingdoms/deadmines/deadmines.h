/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_DEADMINES_H
#define DEF_DEADMINES_H

enum
{
    MAX_ENCOUNTER           = 4,

    TYPE_RHAHKZOR           = 0,
    TYPE_SNEED              = 1,
    TYPE_GILNID             = 2,
    TYPE_IRON_CLAD_DOOR     = 3,

    INST_SAY_ALARM1         = -1036000,
    INST_SAY_ALARM2         = -1036001,

    GO_FACTORY_DOOR         = 13965,                        // rhahk'zor
    GO_MAST_ROOM_DOOR       = 16400,                        // sneed
    GO_FOUNDRY_DOOR         = 16399,                        // gilnid
    GO_HEAVY_DOOR_1         = 17153,                        // to sneed
    GO_HEAVY_DOOR_2         = 17154,                        // to gilnid
    GO_IRON_CLAD_DOOR       = 16397,
    GO_DEFIAS_CANNON        = 16398,
    GO_SMITE_CHEST          = 144111,                       // use to get correct location of mr.smites equipment changes
    GO_MYSTERIOUS_CHEST     = 180024,                       // used for quest 7938; spawns in the instance only if one of the players has the quest

    NPC_RHAHKZOR            = 644,
    NPC_SNEED               = 643,
    NPC_GILNID              = 1763,
    NPC_MR_SMITE            = 646,
    NPC_PIRATE              = 657,
    NPC_SQUALLSHAPER        = 1732,

    QUEST_FORTUNE_AWAITS    = 7938,

    GUID_PREFIX = 3600000,
};

class instance_deadmines : public ScriptedInstance
{
    public:
        instance_deadmines(Map* pMap);

        void Initialize() override;

        void OnPlayerEnter(Player* pPlayer) override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void OnCreatureDeath(Creature* pCreature) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(const uint32 diff) override;

        void SpawnFirstDeadminesPatrol();
        void SpawnSecondDeadminesPatrol();
        void SpawnThirdDeadminesPatrol();
        bool m_firstEnter;

    private:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint32 m_uiIronDoorTimer;
        uint32 m_uiDoorStep;
};

#endif
