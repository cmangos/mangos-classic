/* This file is part of the ScriptDev2 Project. See AUTHORS file for Copyright information
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* ScriptData
SDName: Blackrock_Depths
SD%Complete: 80
SDComment: Quest support: 4001, 4322, 4342, 7604, 9015.
SDCategory: Blackrock Depths
EndScriptData */

/* ContentData
go_shadowforge_brazier
go_relic_coffer_door
at_ring_of_law
npc_grimstone
npc_kharan_mighthammer
npc_marshal_windsor
npc_dughal_stormwing
npc_tobias_seecher
boss_doomrel
EndContentData */

#include "precompiled.h"
#include "blackrock_depths.h"
#include "escort_ai.h"

/*######
## go_shadowforge_brazier
######*/

bool GOUse_go_shadowforge_brazier(Player* /*pPlayer*/, GameObject* pGo)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_LYCEUM) == IN_PROGRESS)
            pInstance->SetData(TYPE_LYCEUM, DONE);
        else
            pInstance->SetData(TYPE_LYCEUM, IN_PROGRESS);
    }
    return false;
}

/*######
## go_relic_coffer_door
######*/

bool GOUse_go_relic_coffer_door(Player* /*pPlayer*/, GameObject* pGo)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData())
    {
        // check if the event is already done
        if (pInstance->GetData(TYPE_VAULT) != DONE && pInstance->GetData(TYPE_VAULT) != IN_PROGRESS)
            pInstance->SetData(TYPE_VAULT, SPECIAL);
    }

    return false;
}

/*######
## npc_grimstone
######*/

/* Notes about this event:
 * Visual: Npc Grimstone should use some visual spell when appear/ disappear / opening/ closing doors
 * Texts: The texts and their positions need confirmation
 * Event timer might also need adjustment
 * Quest-Event: This needs to be clearified - there is some suggestion, that Theldren&Adds also might come as first wave.
 */

enum
{
    SAY_START_1                     = -1230004,
    SAY_START_2                     = -1230005,
    SAY_OPEN_EAST_GATE              = -1230006,
    SAY_SUMMON_BOSS_1               = -1230007,
    SAY_SUMMON_BOSS_2               = -1230008,
    SAY_OPEN_NORTH_GATE             = -1230009,

    NPC_GRIMSTONE                   = 10096,
    DATA_BANNER_BEFORE_EVENT        = 5,

    // 4 or 6 in total? 1+2+1 / 2+2+2 / 3+3. Depending on this, code should be changed.
    MAX_MOB_AMOUNT                  = 4,
    MAX_THELDREN_ADDS               = 4,
    MAX_POSSIBLE_THELDREN_ADDS      = 8,

    SPELL_SUMMON_THELRIN_DND        = 27517,
    // Other spells used by Grimstone
    SPELL_ASHCROMBES_TELEPORT_A     = 15742,
    SPELL_ASHCROMBES_TELEPORT_B     = 6422,
    SPELL_ARENA_FLASH_A             = 15737,
    SPELL_ARENA_FLASH_B             = 15739,
    SPELL_ARENA_FLASH_C             = 15740,
    SPELL_ARENA_FLASH_D             = 15741,
    
    QUEST_THE_CHALLENGE             = 9015,
    NPC_THELDREN_QUEST_CREDIT       = 16166,
};

enum SpawnPosition
{
    POS_EAST                        = 0,
    POS_NORTH                       = 1,
    POS_GRIMSTONE                   = 2,
};

static const float aSpawnPositions[3][4] =
{
    {608.960f, -235.322f, -53.907f, 1.857f},                // Ring mob spawn position
    {644.300f, -175.989f, -53.739f, 3.418f},                // Ring boss spawn position
    {625.559f, -205.618f, -52.735f, 2.609f}                 // Grimstone spawn position
};

static const uint32 aGladiator[MAX_POSSIBLE_THELDREN_ADDS] = {NPC_LEFTY, NPC_ROTFANG, NPC_SNOKH, NPC_MALGEN, NPC_KORV, NPC_REZZNIK, NPC_VAJASHNI, NPC_VOLIDA};
static const uint32 aRingMob[] = {NPC_WORM, NPC_STINGER, NPC_SCREECHER, NPC_THUNDERSNOUT, NPC_CREEPER, NPC_BEETLE};
static const uint32 aRingBoss[] = {NPC_GOROSH, NPC_GRIZZLE, NPC_EVISCERATOR, NPC_OKTHOR, NPC_ANUBSHIAH, NPC_HEDRUM};

enum Phases
{
    PHASE_MOBS                      = 0,
    PHASE_BOSS                      = 2,
    PHASE_GLADIATORS                = 3,
};

bool AreaTrigger_at_ring_of_law(Player* pPlayer, AreaTriggerEntry const* pAt)
{
    if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pPlayer->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_RING_OF_LAW) == IN_PROGRESS || pInstance->GetData(TYPE_RING_OF_LAW) == DONE || pInstance->GetData(TYPE_RING_OF_LAW) == SPECIAL)
            return false;

        if (pPlayer->isGameMaster())
            return false;

        pInstance->SetData(TYPE_RING_OF_LAW, pInstance->GetData(TYPE_RING_OF_LAW) == DATA_BANNER_BEFORE_EVENT ? SPECIAL : IN_PROGRESS);

        pPlayer->SummonCreature(NPC_GRIMSTONE, aSpawnPositions[POS_GRIMSTONE][0], aSpawnPositions[POS_GRIMSTONE][1], aSpawnPositions[POS_GRIMSTONE][2], aSpawnPositions[POS_GRIMSTONE][3], TEMPSUMMON_DEAD_DESPAWN, 0);
        pInstance->SetArenaCenterCoords(pAt->x, pAt->y, pAt->z);

        return false;
    }
    return false;
}

/*######
## npc_grimstone
######*/

struct npc_grimstoneAI : public npc_escortAI
{
    npc_grimstoneAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData();
        m_uiMobSpawnId = urand(0, 5);
        // Select MAX_THELDREN_ADDS(4) random adds for Theldren encounter
        uint8 uiCount = 0;
        for (uint8 i = 0; i < MAX_POSSIBLE_THELDREN_ADDS && uiCount < MAX_THELDREN_ADDS; ++i)
        {
            if (urand(0, 1) || i >= MAX_POSSIBLE_THELDREN_ADDS - MAX_THELDREN_ADDS + uiCount)
            {
                m_uiGladiatorId[uiCount] = aGladiator[i];
                ++uiCount;
            }
        }

        Reset();
    }

    instance_blackrock_depths* m_pInstance;

    uint8 m_uiEventPhase;
    uint32 m_uiEventTimer;

    uint8 m_uiMobSpawnId;
    uint8 m_uiMobDeadCount;

    Phases m_uiPhase;

    uint32 m_uiGladiatorId[MAX_THELDREN_ADDS];

    GuidList m_lSummonedGUIDList;
    GuidSet m_lArenaCrowd;

    void Reset() override
    {
        m_creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        m_uiEventTimer    = 1000;
        m_uiEventPhase    = 0;
        m_uiMobDeadCount  = 0;

        m_uiPhase = PHASE_MOBS;
    }

    void JustSummoned(Creature* pSummoned) override
    {
        if (!m_pInstance)
            return;

        // Ring mob or boss summoned
        float fX, fY, fZ;
        float fcX, fcY, fcZ;
        m_pInstance->GetArenaCenterCoords(fX, fY, fZ);
        m_creature->GetRandomPoint(fX, fY, fZ, 10.0f, fcX, fcY, fcZ);
        pSummoned->GetMotionMaster()->MovePoint(1, fcX, fcY, fcZ);

        m_lSummonedGUIDList.push_back(pSummoned->GetObjectGuid());
    }

    void DoChallengeQuestCredit()
    {
        Map::PlayerList const& PlayerList = m_creature->GetMap()->GetPlayers();

        for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        {
            Player* pPlayer = itr->getSource();
            if (pPlayer && pPlayer->GetQuestStatus(QUEST_THE_CHALLENGE) == QUEST_STATUS_INCOMPLETE)
                pPlayer->KilledMonsterCredit(NPC_THELDREN_QUEST_CREDIT);
        }
    }

    void SummonedCreatureJustDied(Creature* /*pSummoned*/) override
    {
        ++m_uiMobDeadCount;

        switch (m_uiPhase)
        {
            case PHASE_MOBS:                                // Ring mob killed
                if (m_uiMobDeadCount == MAX_MOB_AMOUNT)
                {
                    m_uiEventTimer = 5000;
                    m_uiMobDeadCount = 0;
                }
                break;
            case PHASE_BOSS:                                // Ring boss killed
                // One Boss
                if (m_uiMobDeadCount == 1)
                {
                    m_uiEventTimer = 5000;
                    m_uiMobDeadCount = 0;
                }
                break;
            case PHASE_GLADIATORS:                          // Theldren and his band killed
                // Adds + Theldren
                if (m_uiMobDeadCount == MAX_THELDREN_ADDS + 1)
                {
                    m_uiEventTimer = 5000;
                    m_uiMobDeadCount = 0;
                    DoChallengeQuestCredit();
                }
                break;
        }
    }

    void SummonRingMob(uint32 uiEntry, SpawnPosition uiPosition)
    {
        float fX, fY, fZ;
        m_creature->GetRandomPoint(aSpawnPositions[uiPosition][0], aSpawnPositions[uiPosition][1], aSpawnPositions[uiPosition][2], 2.0f, fX, fY, fZ);
        m_creature->SummonCreature(uiEntry, fX, fY, fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 0:                                         // Middle reached first time
                DoScriptText(SAY_START_1, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 1:                                         // Reached wall again
                DoScriptText(SAY_OPEN_EAST_GATE, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 2:                                         // walking along the wall, while door opened
                SetEscortPaused(true);
                break;
            case 3:                                         // Middle reached second time
                DoScriptText(SAY_SUMMON_BOSS_1, m_creature);
                break;
            case 4:                                         // Reached North Gate
                DoScriptText(SAY_OPEN_NORTH_GATE, m_creature);
                SetEscortPaused(true);
                m_uiEventTimer = 5000;
                break;
            case 5:
                if (m_pInstance)
                {
                    m_pInstance->SetData(TYPE_RING_OF_LAW, DONE);
                    debug_log("SD2: npc_grimstone: event reached end and set complete.");
                }
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_pInstance)
            return;

        if (m_pInstance->GetData(TYPE_RING_OF_LAW) == FAIL)
        {
            // Reset Doors
            if (m_uiEventPhase >= 10)                       // North Gate is opened
            {
                m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
            }
            else if (m_uiEventPhase >= 4)                   // East Gate is opened
            {
                m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
            }

            // Despawn Summoned Mobs
            for (GuidList::const_iterator itr = m_lSummonedGUIDList.begin(); itr != m_lSummonedGUIDList.end(); ++itr)
            {
                if (Creature* pSummoned = m_creature->GetMap()->GetCreature(*itr))
                    pSummoned->ForcedDespawn();
            }
            m_lSummonedGUIDList.clear();

            // Despawn NPC
            m_creature->ForcedDespawn();
            return;
        }

        if (m_uiEventTimer)
        {
            if (m_uiEventTimer <= uiDiff)
            {
                switch (m_uiEventPhase)
                {
                    case 0:
                        // Shortly after spawn, start walking
                        m_creature->CastSpell(m_creature, SPELL_ASHCROMBES_TELEPORT_A, true);
                        DoScriptText(SAY_START_2, m_creature);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                        // Some of the NPCs in the crowd do cheer emote at event start
                        // we randomly select 25% of the NPCs to do this
                        m_pInstance->GetArenaCrowdGuid(m_lArenaCrowd);
                        for (GuidSet::const_iterator itr = m_lArenaCrowd.begin(); itr != m_lArenaCrowd.end(); ++itr)
                        {
                            if (Creature* pSpectator = m_creature->GetMap()->GetCreature(*itr))
                            {
                                if (urand(0, 1) < 0.25)
                                    pSpectator->HandleEmote(EMOTE_ONESHOT_CHEER);
                            }
                        }
                        Start(false);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 1:
                        // Start walking towards wall
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 2:
                        m_uiEventTimer = 2000;
                        break;
                    case 3:
                        // Open East Gate
                        m_creature->CastSpell(m_creature, SPELL_ARENA_FLASH_A, true);
                        m_creature->CastSpell(m_creature, SPELL_ARENA_FLASH_B, true);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                        m_uiEventTimer = 3000;
                        break;
                    case 4:
                        // timer for teleport out spell which has 2000 ms cast time
                        m_creature->CastSpell(m_creature, SPELL_ASHCROMBES_TELEPORT_B, true);
                        m_uiEventTimer = 2500;
                        break;
                    case 5:
                        m_creature->SetVisibility(VISIBILITY_OFF);
                        SetEscortPaused(false);
                        // Summon Ring Mob(s)
                        SummonRingMob(aRingMob[m_uiMobSpawnId], POS_EAST);
                        m_uiEventTimer = 8000;
                        break;
                    case 6:
                        // Summon Ring Mob(s)
                        SummonRingMob(aRingMob[m_uiMobSpawnId], POS_EAST);
                        SummonRingMob(aRingMob[m_uiMobSpawnId], POS_EAST);
                        m_uiEventTimer = 8000;
                        break;
                    case 7:
                        // Summon Ring Mob(s)
                        SummonRingMob(aRingMob[m_uiMobSpawnId], POS_EAST);
                        m_uiEventTimer = 0;
                        break;
                    case 8:
                        // Summoned Mobs are dead, continue event
                        DoScriptText(SAY_SUMMON_BOSS_2, m_creature);
                        m_creature->SetVisibility(VISIBILITY_ON);
                        m_creature->CastSpell(m_creature, SPELL_ASHCROMBES_TELEPORT_A, true);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_1);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                    case 9:
                        // Open North Gate
                        m_creature->CastSpell(m_creature, SPELL_ARENA_FLASH_C, true);
                        m_creature->CastSpell(m_creature, SPELL_ARENA_FLASH_D, true);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                        m_uiEventTimer = 5000;
                        break;
                    case 10:
                        // timer for teleport out spell which has 2000 ms cast time
                        m_creature->CastSpell(m_creature, SPELL_ASHCROMBES_TELEPORT_B, true);
                        m_uiEventTimer = 2500;
                        break;
                    case 11:
                        // Summon Boss
                        m_creature->SetVisibility(VISIBILITY_OFF);
                        // If banner summoned after start, then summon Thelden after the creatures are dead
                        if (m_pInstance->GetData(TYPE_RING_OF_LAW) == SPECIAL && m_uiPhase == PHASE_MOBS)
                        {
                            m_uiPhase = PHASE_GLADIATORS;
                            SummonRingMob(NPC_THELDREN, POS_NORTH);
                            for (uint8 i = 0; i < MAX_THELDREN_ADDS; ++i)
                                SummonRingMob(m_uiGladiatorId[i], POS_NORTH);
                        }
                        else
                        {
                            m_uiPhase = PHASE_BOSS;
                            SummonRingMob(aRingBoss[urand(0, 5)], POS_NORTH);
                        }
                        m_uiEventTimer = 0;
                        break;
                    case 12:
                        // Boss dead
                        m_lSummonedGUIDList.clear();
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_2);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_3);
                        m_pInstance->DoUseDoorOrButton(GO_ARENA_4);
                        SetEscortPaused(false);
                        m_uiEventTimer = 0;
                        break;
                }
                ++m_uiEventPhase;
            }
            else
                m_uiEventTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_grimstone(Creature* pCreature)
{
    return new npc_grimstoneAI(pCreature);
}

bool EffectDummyCreature_spell_banner_of_provocation(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Creature* pCreatureTarget, ObjectGuid /*originalCasterGuid*/)
{
    if (uiSpellId == SPELL_SUMMON_THELRIN_DND && uiEffIndex != EFFECT_INDEX_0)
    {
        instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreatureTarget->GetInstanceData();
        if (pInstance && pInstance->GetData(TYPE_RING_OF_LAW) != DONE && pInstance->GetData(TYPE_RING_OF_LAW) != SPECIAL)
            pInstance->SetData(TYPE_RING_OF_LAW, pInstance->GetData(TYPE_RING_OF_LAW) == IN_PROGRESS ? uint32(SPECIAL) : uint32(DATA_BANNER_BEFORE_EVENT));

        return true;
    }
    return false;
}

/*######
## npc_kharan_mighthammer
######*/
enum
{
    QUEST_WHAT_IS_GOING_ON = 4001,
    QUEST_KHARANS_TALE     = 4342
};

#define GOSSIP_ITEM_KHARAN_1    "I need to know where the princess are, Kharan!"
#define GOSSIP_ITEM_KHARAN_2    "All is not lost, Kharan!"

#define GOSSIP_ITEM_KHARAN_3    "Gor'shak is my friend, you can trust me."
#define GOSSIP_ITEM_KHARAN_4    "Not enough, you need to tell me more."
#define GOSSIP_ITEM_KHARAN_5    "So what happened?"
#define GOSSIP_ITEM_KHARAN_6    "Continue..."
#define GOSSIP_ITEM_KHARAN_7    "So you suspect that someone on the inside was involved? That they were tipped off?"
#define GOSSIP_ITEM_KHARAN_8    "Continue with your story please."
#define GOSSIP_ITEM_KHARAN_9    "Indeed."
#define GOSSIP_ITEM_KHARAN_10   "The door is open, Kharan. You are a free man."

bool GossipHello_npc_kharan_mighthammer(Player* pPlayer, Creature* pCreature)
{
    if (pCreature->isQuestGiver())
        pPlayer->PrepareQuestMenu(pCreature->GetObjectGuid());

    if (pPlayer->GetQuestStatus(QUEST_WHAT_IS_GOING_ON) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    if (pPlayer->GetQuestStatus(QUEST_KHARANS_TALE) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

    if (pPlayer->GetTeam() == HORDE)
        pPlayer->SEND_GOSSIP_MENU(2473, pCreature->GetObjectGuid());
    else
        pPlayer->SEND_GOSSIP_MENU(2474, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_kharan_mighthammer(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            pPlayer->SEND_GOSSIP_MENU(2475, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+2:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            pPlayer->SEND_GOSSIP_MENU(2476, pCreature->GetObjectGuid());
            break;

        case GOSSIP_ACTION_INFO_DEF+3:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            pPlayer->SEND_GOSSIP_MENU(2477, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+4:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            pPlayer->SEND_GOSSIP_MENU(2478, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+5:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
            pPlayer->SEND_GOSSIP_MENU(2479, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+6:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_8, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            pPlayer->SEND_GOSSIP_MENU(2480, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+7:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_9, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 8);
            pPlayer->SEND_GOSSIP_MENU(2481, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+8:
            pPlayer->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_10, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);
            pPlayer->SEND_GOSSIP_MENU(2482, pCreature->GetObjectGuid());
            break;
        case GOSSIP_ACTION_INFO_DEF+9:
            pPlayer->CLOSE_GOSSIP_MENU();
            if (pPlayer->GetTeam() == HORDE)
                pPlayer->AreaExploredOrEventHappens(QUEST_WHAT_IS_GOING_ON);
            else
                pPlayer->AreaExploredOrEventHappens(QUEST_KHARANS_TALE);
            break;
    }
    return true;
}

/*######
## npc_rocknot
######*/

enum
{
    SAY_GOT_BEER       = -1230000,

    SPELL_DRUNKEN_RAGE = 14872,

    QUEST_ALE          = 4295
};

struct npc_rocknotAI : public npc_escortAI
{
    npc_rocknotAI(Creature* pCreature) : npc_escortAI(pCreature)
    {
        m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
        Reset();
    }

    ScriptedInstance* m_pInstance;

    uint32 m_uiBreakKegTimer;
    uint32 m_uiBreakDoorTimer;

    void Reset() override
    {
        if (HasEscortState(STATE_ESCORT_ESCORTING))
            return;

        m_uiBreakKegTimer  = 0;
        m_uiBreakDoorTimer = 0;
    }

    void DoGo(uint32 id, uint32 state)
    {
        if (GameObject* pGo = m_pInstance->GetSingleGameObjectFromStorage(id))
            pGo->SetGoState(GOState(state));
    }

    void WaypointReached(uint32 uiPointId) override
    {
        if (!m_pInstance)
            return;

        switch (uiPointId)
        {
            case 1:
                m_creature->HandleEmote(EMOTE_ONESHOT_KICK);
                break;
            case 2:
                m_creature->HandleEmote(EMOTE_ONESHOT_ATTACKUNARMED);
                break;
            case 3:
                m_creature->HandleEmote(EMOTE_ONESHOT_ATTACKUNARMED);
                break;
            case 4:
                m_creature->HandleEmote(EMOTE_ONESHOT_KICK);
                break;
            case 5:
                m_creature->HandleEmote(EMOTE_ONESHOT_KICK);
                m_uiBreakKegTimer = 2000;
                break;
        }
    }

    void UpdateEscortAI(const uint32 uiDiff) override
    {
        if (!m_pInstance)
            return;

        if (m_uiBreakKegTimer)
        {
            if (m_uiBreakKegTimer <= uiDiff)
            {
                DoGo(GO_BAR_KEG_SHOT, 0);
                m_uiBreakKegTimer = 0;
                m_uiBreakDoorTimer = 1000;
            }
            else
                m_uiBreakKegTimer -= uiDiff;
        }

        if (m_uiBreakDoorTimer)
        {
            if (m_uiBreakDoorTimer <= uiDiff)
            {
                DoGo(GO_BAR_DOOR, 2);
                DoGo(GO_BAR_KEG_TRAP, 0);                   // doesn't work very well, leaving code here for future
                // spell by trap has effect61, this indicate the bar go hostile

                if (Creature* pTmp = m_pInstance->GetSingleCreatureFromStorage(NPC_PHALANX))
                    pTmp->SetFactionTemporary(14, TEMPFACTION_NONE);

                // for later, this event(s) has alot more to it.
                // optionally, DONE can trigger bar to go hostile.
                m_pInstance->SetData(TYPE_BAR, DONE);

                m_uiBreakDoorTimer = 0;
            }
            else
                m_uiBreakDoorTimer -= uiDiff;
        }
    }
};

CreatureAI* GetAI_npc_rocknot(Creature* pCreature)
{
    return new npc_rocknotAI(pCreature);
}

bool QuestRewarded_npc_rocknot(Player* /*pPlayer*/, Creature* pCreature, Quest const* pQuest)
{
    ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData();

    if (!pInstance)
        return true;

    if (pInstance->GetData(TYPE_BAR) == DONE || pInstance->GetData(TYPE_BAR) == SPECIAL)
        return true;

    if (pQuest->GetQuestId() == QUEST_ALE)
    {
        if (pInstance->GetData(TYPE_BAR) != IN_PROGRESS)
            pInstance->SetData(TYPE_BAR, IN_PROGRESS);

        pInstance->SetData(TYPE_BAR, SPECIAL);

        // keep track of amount in instance script, returns SPECIAL if amount ok and event in progress
        if (pInstance->GetData(TYPE_BAR) == SPECIAL)
        {
            DoScriptText(SAY_GOT_BEER, pCreature);
            pCreature->CastSpell(pCreature, SPELL_DRUNKEN_RAGE, false);

            if (npc_rocknotAI* pEscortAI = dynamic_cast<npc_rocknotAI*>(pCreature->AI()))
                pEscortAI->Start(false, NULL, NULL, true);
        }
    }

    return true;
}

/*######
## npc_marshal_windsor
######*/

enum
{
    // Windsor texts
    SAY_WINDSOR_AGGRO1          = -1230011,
    SAY_WINDSOR_AGGRO2          = -1230012,
    SAY_WINDSOR_AGGRO3          = -1230013,
    SAY_WINDSOR_START           = -1230014,
    SAY_WINDSOR_CELL_DUGHAL_1   = -1230015,
    SAY_WINDSOR_CELL_DUGHAL_3   = -1230016,
    SAY_WINDSOR_EQUIPMENT_1     = -1230017,
    SAY_WINDSOR_EQUIPMENT_2     = -1230018,
    SAY_WINDSOR_EQUIPMENT_3     = -1230019,
    SAY_WINDSOR_EQUIPMENT_4     = -1230020,
    SAY_WINDSOR_CELL_JAZ_1      = -1230021,
    SAY_WINDSOR_CELL_JAZ_2      = -1230022,
    SAY_WINDSOR_CELL_SHILL_1    = -1230023,
    SAY_WINDSOR_CELL_SHILL_2    = -1230024,
    SAY_WINDSOR_CELL_SHILL_3    = -1230025,
    SAY_WINDSOR_CELL_CREST_1    = -1230026,
    SAY_WINDSOR_CELL_CREST_2    = -1230027,
    SAY_WINDSOR_CELL_TOBIAS_1   = -1230028,
    SAY_WINDSOR_CELL_TOBIAS_2   = -1230029,
    SAY_WINDSOR_FREE_1          = -1230030,
    SAY_WINDSOR_FREE_2          = -1230031,

    // Additional gossips
    SAY_DUGHAL_FREE             = -1230010,
    GOSSIP_ID_DUGHAL            = -3230000,
    GOSSIP_TEXT_ID_DUGHAL       = 2846,

    SAY_TOBIAS_FREE_1           = -1230032,
    SAY_TOBIAS_FREE_2           = -1230033,
    GOSSIP_ID_TOBIAS            = -3230001,
    GOSSIP_TEXT_ID_TOBIAS       = 2847,

    NPC_REGINALD_WINDSOR        = 9682,

    QUEST_JAIL_BREAK            = 4322
};

struct npc_marshal_windsorAI : public npc_escortAI
{
    npc_marshal_windsorAI(Creature* m_creature) : npc_escortAI(m_creature)
    {
        m_pInstance = (instance_blackrock_depths*)m_creature->GetInstanceData();
        Reset();
    }

    instance_blackrock_depths* m_pInstance;

    uint8 m_uiEventPhase;

    void Reset() override
    {
        if (!HasEscortState(STATE_ESCORT_ESCORTING))
            m_uiEventPhase = 0;
    }

    void Aggro(Unit* pWho) override
    {
        switch (urand(0, 2))
        {
            case 0: DoScriptText(SAY_WINDSOR_AGGRO1, m_creature, pWho); break;
            case 1: DoScriptText(SAY_WINDSOR_AGGRO2, m_creature); break;
            case 2: DoScriptText(SAY_WINDSOR_AGGRO3, m_creature, pWho); break;
        }
    }

    void WaypointReached(uint32 uiPointId) override
    {
        switch (uiPointId)
        {
            case 1:
                if (m_pInstance)
                    m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, IN_PROGRESS);

                DoScriptText(SAY_WINDSOR_START, m_creature);
                break;
            case 7:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_WINDSOR_CELL_DUGHAL_1, m_creature, pPlayer);
                if (m_pInstance)
                {
                    if (Creature* pDughal = m_pInstance->GetSingleCreatureFromStorage(NPC_DUGHAL))
                    {
                        pDughal->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        m_creature->SetFacingToObject(pDughal);
                    }
                }
                ++m_uiEventPhase;
                SetEscortPaused(true);
                break;
            case 9:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_WINDSOR_CELL_DUGHAL_3, m_creature, pPlayer);
                break;
            case 14:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_WINDSOR_EQUIPMENT_1, m_creature, pPlayer);
                break;
            case 15:
                // ToDo: fix this emote!
                // m_creature->HandleEmoteCommand(EMOTE_ONESHOT_USESTANDING);
                break;
            case 16:
                if (m_pInstance)
                    m_pInstance->DoUseDoorOrButton(GO_JAIL_DOOR_SUPPLY);
                break;
            case 18:
                DoScriptText(SAY_WINDSOR_EQUIPMENT_2, m_creature);
                break;
            case 19:
                // ToDo: fix this emote!
                // m_creature->HandleEmoteCommand(EMOTE_ONESHOT_USESTANDING);
                break;
            case 20:
                if (m_pInstance)
                    m_pInstance->DoUseDoorOrButton(GO_JAIL_SUPPLY_CRATE);
                break;
            case 21:
                m_creature->UpdateEntry(NPC_REGINALD_WINDSOR);
                break;
            case 22:
                if (Player* pPlayer = GetPlayerForEscort())
                {
                    DoScriptText(SAY_WINDSOR_EQUIPMENT_3, m_creature, pPlayer);
                    m_creature->SetFacingToObject(pPlayer);
                }
                break;
            case 23:
                DoScriptText(SAY_WINDSOR_EQUIPMENT_4, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    m_creature->SetFacingToObject(pPlayer);
                break;
            case 30:
                if (m_pInstance)
                {
                    if (Creature* pJaz = m_pInstance->GetSingleCreatureFromStorage(NPC_JAZ))
                        m_creature->SetFacingToObject(pJaz);
                }
                DoScriptText(SAY_WINDSOR_CELL_JAZ_1, m_creature);
                ++m_uiEventPhase;
                SetEscortPaused(true);
                break;
            case 32:
                DoScriptText(SAY_WINDSOR_CELL_JAZ_2, m_creature);
                break;
            case 35:
                if (m_pInstance)
                {
                    if (Creature* pShill = m_pInstance->GetSingleCreatureFromStorage(NPC_SHILL))
                        m_creature->SetFacingToObject(pShill);
                }
                DoScriptText(SAY_WINDSOR_CELL_SHILL_1, m_creature);
                ++m_uiEventPhase;
                SetEscortPaused(true);
                break;
            case 37:
                DoScriptText(SAY_WINDSOR_CELL_SHILL_2, m_creature);
                break;
            case 38:
                DoScriptText(SAY_WINDSOR_CELL_SHILL_3, m_creature);
                break;
            case 45:
                if (m_pInstance)
                {
                    if (Creature* pCrest = m_pInstance->GetSingleCreatureFromStorage(NPC_CREST))
                        m_creature->SetFacingToObject(pCrest);
                }
                DoScriptText(SAY_WINDSOR_CELL_CREST_1, m_creature);
                ++m_uiEventPhase;
                SetEscortPaused(true);
                break;
            case 47:
                DoScriptText(SAY_WINDSOR_CELL_CREST_2, m_creature);
                break;
            case 49:
                DoScriptText(SAY_WINDSOR_CELL_TOBIAS_1, m_creature);
                if (m_pInstance)
                {
                    if (Creature* pTobias = m_pInstance->GetSingleCreatureFromStorage(NPC_TOBIAS))
                    {
                        pTobias->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        m_creature->SetFacingToObject(pTobias);
                    }
                }
                ++m_uiEventPhase;
                SetEscortPaused(true);
                break;
            case 51:
                if (Player* pPlayer = GetPlayerForEscort())
                    DoScriptText(SAY_WINDSOR_CELL_TOBIAS_2, m_creature, pPlayer);
                break;
            case 57:
                DoScriptText(SAY_WINDSOR_FREE_1, m_creature);
                if (Player* pPlayer = GetPlayerForEscort())
                    m_creature->SetFacingToObject(pPlayer);
                break;
            case 58:
                DoScriptText(SAY_WINDSOR_FREE_2, m_creature);
                if (m_pInstance)
                    m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, DONE);

                if (Player* pPlayer = GetPlayerForEscort())
                    pPlayer->GroupEventHappens(QUEST_JAIL_BREAK, m_creature);
                break;
        }
    }

    void UpdateEscortAI(const uint32 /*uiDiff*/) override
    {
        // Handle escort resume events
        if (m_pInstance && m_pInstance->GetData(TYPE_QUEST_JAIL_BREAK) == SPECIAL)
        {
            switch (m_uiEventPhase)
            {
                case 1:                     // Dughal
                case 3:                     // Ograbisi
                case 4:                     // Crest
                case 5:                     // Shill
                case 6:                     // Tobias
                    SetEscortPaused(false);
                    break;
                case 2:                     // Jaz
                    ++m_uiEventPhase;
                    break;
            }

            m_pInstance->SetData(TYPE_QUEST_JAIL_BREAK, IN_PROGRESS);
        }

        if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

CreatureAI* GetAI_npc_marshal_windsor(Creature* pCreature)
{
    return new npc_marshal_windsorAI(pCreature);
}

bool QuestAccept_npc_marshal_windsor(Player* pPlayer, Creature* pCreature, const Quest* pQuest)
{
    if (pQuest->GetQuestId() == QUEST_JAIL_BREAK)
    {
        pCreature->SetFactionTemporary(FACTION_ESCORT_A_NEUTRAL_ACTIVE, TEMPFACTION_RESTORE_RESPAWN);

        if (npc_marshal_windsorAI* pEscortAI = dynamic_cast<npc_marshal_windsorAI*>(pCreature->AI()))
            pEscortAI->Start(false, pPlayer, pQuest);

        return true;
    }

    return false;
}

/*######
## npc_dughal_stormwing
######*/

bool GossipHello_npc_dughal_stormwing(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ID_DUGHAL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_DUGHAL, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_npc_dughal_stormwing(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        // Set instance data in order to allow the quest to continue
        if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
            pInstance->SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);

        DoScriptText(SAY_DUGHAL_FREE, pCreature, pPlayer);
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        pCreature->SetWalk(false);
        pCreature->GetMotionMaster()->MoveWaypoint();

        pPlayer->CLOSE_GOSSIP_MENU();
    }

    return true;
}

/*######
## npc_tobias_seecher
######*/

bool GossipHello_npc_tobias_seecher(Player* pPlayer, Creature* pCreature)
{
    if (pPlayer->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE)
        pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ID_TOBIAS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_TOBIAS, pCreature->GetObjectGuid());

    return true;
}

bool GossipSelect_npc_tobias_seecher(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
    {
        // Set instance data in order to allow the quest to continue
        if (instance_blackrock_depths* pInstance = (instance_blackrock_depths*)pCreature->GetInstanceData())
            pInstance->SetData(TYPE_QUEST_JAIL_BREAK, SPECIAL);

        DoScriptText(urand(0, 1) ? SAY_TOBIAS_FREE_1 : SAY_TOBIAS_FREE_2, pCreature);
        pCreature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);

        pCreature->SetWalk(false);
        pCreature->GetMotionMaster()->MoveWaypoint();

        pPlayer->CLOSE_GOSSIP_MENU();
    }

    return true;
}

/*######
## boss_doomrel
######*/

enum
{
    SAY_DOOMREL_START_EVENT     = -1230003,
    GOSSIP_ITEM_CHALLENGE       = -3230002,
    GOSSIP_TEXT_ID_CHALLENGE    = 2601,
};

bool GossipHello_boss_doomrel(Player* pPlayer, Creature* pCreature)
{
    if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
    {
        if (pInstance->GetData(TYPE_TOMB_OF_SEVEN) == NOT_STARTED || pInstance->GetData(TYPE_TOMB_OF_SEVEN) == FAIL)
            pPlayer->ADD_GOSSIP_ITEM_ID(GOSSIP_ICON_CHAT, GOSSIP_ITEM_CHALLENGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
    }

    pPlayer->SEND_GOSSIP_MENU(GOSSIP_TEXT_ID_CHALLENGE, pCreature->GetObjectGuid());
    return true;
}

bool GossipSelect_boss_doomrel(Player* pPlayer, Creature* pCreature, uint32 /*uiSender*/, uint32 uiAction)
{
    switch (uiAction)
    {
        case GOSSIP_ACTION_INFO_DEF+1:
            pPlayer->CLOSE_GOSSIP_MENU();
            DoScriptText(SAY_DOOMREL_START_EVENT, pCreature);
            // start event
            if (ScriptedInstance* pInstance = (ScriptedInstance*)pCreature->GetInstanceData())
                pInstance->SetData(TYPE_TOMB_OF_SEVEN, IN_PROGRESS);

            break;
    }
    return true;
}

void AddSC_blackrock_depths()
{
    Script* pNewScript;

    pNewScript = new Script;
    pNewScript->Name = "go_shadowforge_brazier";
    pNewScript->pGOUse = &GOUse_go_shadowforge_brazier;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_relic_coffer_door";
    pNewScript->pGOUse = &GOUse_go_relic_coffer_door;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "at_ring_of_law";
    pNewScript->pAreaTrigger = &AreaTrigger_at_ring_of_law;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_grimstone";
    pNewScript->GetAI = &GetAI_npc_grimstone;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_theldren_trigger";
    pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_banner_of_provocation;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_kharan_mighthammer";
    pNewScript->pGossipHello =  &GossipHello_npc_kharan_mighthammer;
    pNewScript->pGossipSelect = &GossipSelect_npc_kharan_mighthammer;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_rocknot";
    pNewScript->GetAI = &GetAI_npc_rocknot;
    pNewScript->pQuestRewardedNPC = &QuestRewarded_npc_rocknot;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_marshal_windsor";
    pNewScript->GetAI = &GetAI_npc_marshal_windsor;
    pNewScript->pQuestAcceptNPC = &QuestAccept_npc_marshal_windsor;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_tobias_seecher";
    pNewScript->pGossipHello =  &GossipHello_npc_tobias_seecher;
    pNewScript->pGossipSelect = &GossipSelect_npc_tobias_seecher;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_dughal_stormwing";
    pNewScript->pGossipHello =  &GossipHello_npc_dughal_stormwing;
    pNewScript->pGossipSelect = &GossipSelect_npc_dughal_stormwing;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "boss_doomrel";
    pNewScript->pGossipHello = &GossipHello_boss_doomrel;
    pNewScript->pGossipSelect = &GossipSelect_boss_doomrel;
    pNewScript->RegisterSelf();
}
