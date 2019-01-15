/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_SCARLETM_H
#define DEF_SCARLETM_H

enum
{
    MAX_ENCOUNTER                   = 2,

    TYPE_MOGRAINE_AND_WHITE_EVENT   = 1,
    TYPE_ASHBRINGER_EVENT           = 2,

    NPC_MOGRAINE                    = 3976,
    NPC_WHITEMANE                   = 3977,
    NPC_VORREL                      = 3981,
    NPC_INTERROGATOR_VISHAS         = 3983,

    GO_WHITEMANE_DOOR               = 104600,
    GO_CHAPEL_DOOR                  = 104591,

    SAY_TRIGGER_VORREL              = -1189015,

    ITEM_CORRUPTED_ASHRBRINGER      = 22691,
    SAY_ASHBRINGER_ENTRANCE         = -1189036,
};

class instance_scarlet_monastery : public ScriptedInstance
{
    public:
        instance_scarlet_monastery(Map* pMap);

        void Initialize() override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnCreatureDeath(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;

        void OnPlayerEnter(Player* pPlayer) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiData) const override;

    private:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
};

#endif
