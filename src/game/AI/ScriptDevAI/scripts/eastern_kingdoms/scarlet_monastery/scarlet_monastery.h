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
    AREATRIGGER_CATHEDRAL_ENTRANCE  = 4089, // used to trigger Corrupted Ashbringer event
    // these npcs are tracked for purposes of turning them friendly during the Corrupted Ashbringer event
    NPC_SORCERER    = 4294,
    NPC_MYRMIDON    = 4295,
    NPC_DEFENDER    = 4298,
    NPC_CHAPLAIN    = 4299,
    NPC_WIZARD      = 4300,
    NPC_CENTURION   = 4301,
    NPC_CHAMPION    = 4302,
    NPC_ABBOT       = 4303,
    NPC_MONK        = 4540,
    NPC_FAIRBANKS   = 4542,
};

class instance_scarlet_monastery : public ScriptedInstance
{
    public:
        instance_scarlet_monastery(Map* pMap);

        void Initialize() override;

        void OnCreatureCreate(Creature* pCreature) override;
        void OnCreatureDeath(Creature* pCreature) override;
        void OnObjectCreate(GameObject* pGo) override;
        void OnCreatureRespawn(Creature* creature) override;
        void OnObjectSpawn(GameObject* go) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiData) const override;

        bool DoHandleAreaTrigger(AreaTriggerEntry const* areaTrigger);

    private:
        GuidSet m_sAshbringerFriendlyGuids;
        uint32 m_auiEncounter[MAX_ENCOUNTER];
};

#endif
