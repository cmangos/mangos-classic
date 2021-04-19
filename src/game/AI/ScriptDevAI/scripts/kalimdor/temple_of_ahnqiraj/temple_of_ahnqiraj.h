/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software licensed under GPL version 2
 * Please see the included DOCS/LICENSE.TXT for more information */

#ifndef DEF_TEMPLE_OF_AHNQIRAJ_H
#define DEF_TEMPLE_OF_AHNQIRAJ_H

enum
{
    MAX_ENCOUNTER               = 10,

    TYPE_SKERAM                 = 0,
    TYPE_BUG_TRIO               = 1,
    TYPE_SARTURA                = 2,
    TYPE_FANKRISS               = 3,
    TYPE_VISCIDUS               = 4,
    TYPE_HUHURAN                = 5,
    TYPE_TWINS                  = 6,
    TYPE_OURO                   = 7,
    TYPE_CTHUN                  = 8,
    TYPE_TWINS_INTRO            = 9,

    NPC_SKERAM                  = 15263,
    NPC_KRI                     = 15511,
    NPC_VEM                     = 15544,
    NPC_YAUJ                    = 15543,
    NPC_SARTURA                 = 15516,
    NPC_VEKLOR                  = 15276,
    NPC_VEKNILASH               = 15275,
    NPC_MASTERS_EYE             = 15963,
    NPC_OURO_SPAWNER            = 15957,
    NPC_VISCIDUS                = 15299,

    // C'Thun encounter
    NPC_EYE_OF_CTHUN            = 15589,
    NPC_CTHUN                   = 15727,
    NPC_CLAW_TENTACLE           = 15725,
    NPC_EYE_TENTACLE            = 15726,
    NPC_TENTACLE_PORTAL         = 15904,
    NPC_GIANT_CLAW_TENTACLE     = 15728,
    NPC_GIANT_EYE_TENTACLE      = 15334,
    NPC_EXIT_TRIGGER            = 15800,
    NPC_FLESH_TENTACLE          = 15802,
    NPC_GIANT_TENTACLE_PORTAL   = 15910,

    GO_SKERAM_GATE              = 180636,
    GO_TWINS_ENTER_DOOR         = 180634,
    GO_TWINS_EXIT_DOOR          = 180635,
    GO_SANDWORM_BASE            = 180795,

    EMOTE_EYE_INTRO             = -1531012,
    STAND_EMPERORS_INTRO        = 1,
    SAY_EMPERORS_INTRO_1        = -1531013,
    SAY_EMPERORS_INTRO_2        = -1531014,
    SAY_EMPERORS_INTRO_3        = -1531015,
    SAY_EMPERORS_INTRO_4        = -1531016,
    SAY_EMPERORS_INTRO_5        = -1531017,
    SAY_EMPERORS_INTRO_6        = -1531018,

    // Whispered on players around the map
    SAY_CTHUN_WHISPER_1         = -1531033,
    SAY_CTHUN_WHISPER_2         = -1531034,
    SAY_CTHUN_WHISPER_3         = -1531035,
    SAY_CTHUN_WHISPER_4         = -1531036,
    SAY_CTHUN_WHISPER_5         = -1531037,
    SAY_CTHUN_WHISPER_6         = -1531038,
    SAY_CTHUN_WHISPER_7         = -1531039,
    SAY_CTHUN_WHISPER_8         = -1531040,

    AREATRIGGER_TWIN_EMPERORS   = 4047,
    AREATRIGGER_SARTURA         = 4052,

    SPELL_SUMMON_PLAYER         = 20477,

    // Cast periodically on players around the instance
    SPELL_WHISPERINGS_CTHUN_1   = 26195,
    SPELL_WHISPERINGS_CTHUN_2   = 26197,
    SPELL_WHISPERINGS_CTHUN_3   = 26198,
    SPELL_WHISPERINGS_CTHUN_4   = 26258,
    SPELL_WHISPERINGS_CTHUN_5   = 26259,

    SPELL_SUMMON_PORTAL         = 26396,
    SPELL_SUMMON_GIANT_PORTAL   = 26477,

    NPC_POISON_CLOUD            = 15933,

    NPC_QIRAJI_SCARAB           = 15316,
    NPC_QIRAJI_SCORPION         = 15317,
};

// Prophecies yelled by Propher Skeram before he is engaged
static const uint32 sound_skeram_prophecy[] = { 8616, 8621, 8619, 8620, 8618 };

// Spells from Qiraji Resonating crystal (AQ40 specific mounts)
static const uint32 qiraji_mount_auras[] = { 25953, 26054, 26055, 26056 };

class instance_temple_of_ahnqiraj : public ScriptedInstance, private DialogueHelper
{
    public:
        instance_temple_of_ahnqiraj(Map* pMap);
        ~instance_temple_of_ahnqiraj() {}

        void Initialize() override;

        bool IsEncounterInProgress() const override;

        void OnCreatureCreate(Creature* creature) override;
        void OnCreatureDeath(Creature* creature) override;
        void OnObjectCreate(GameObject* pGo) override;
        void OnCreatureRespawn(Creature* creature) override;

        void OnPlayerLeave(Player* pPlayer) override;

        void SetData(uint32 uiType, uint32 uiData) override;
        uint32 GetData(uint32 uiType) const override;

        void DoHandleTempleAreaTrigger(uint32 triggerId, Player* player);

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override;

        void Update(uint32 uiDiff) override;

    private:
        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string m_strInstData;

        uint8 m_uiBugTrioDeathCount;
        uint32 m_uiCthunWhisperTimer;
        uint32 m_uiSkeramProphecyTimer;

        GuidVector m_bugTrioSpawns;

        void JustDidDialogueStep(int32 entry) override;
};

#endif
