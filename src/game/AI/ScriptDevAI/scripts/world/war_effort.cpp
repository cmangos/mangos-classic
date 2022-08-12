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

#include "AI/ScriptDevAI/include/sc_common.h"
#include "World/WorldState.h"
#include "AI/ScriptDevAI/scripts/kalimdor/world_kalimdor.h"
#include "AI/ScriptDevAI/base/CombatAI.h"
#include "Grids/GridNotifiers.h"
#include "Grids/GridNotifiersImpl.h"

enum Quests
{
    QUEST_ALLIANCE_COPPER_BARS_1    = 8492,
    QUEST_ALLIANCE_COPPER_BARS_2    = 8493,
    QUEST_HORDE_COPPER_BARS_1       = 8532,
    QUEST_HORDE_COPPER_BARS_2       = 8533,

    QUEST_ALLIANCE_PURPLE_LOTUS_1 = 8505,
    QUEST_ALLIANCE_PURPLE_LOTUS_2 = 8506,
    QUEST_HORDE_PURPLE_LOTUS_1 = 8582,
    QUEST_HORDE_PURPLE_LOTUS_2 = 8583,

    QUEST_HORDE_PEACEBLOOM_1 = 8549,
    QUEST_HORDE_PEACEBLOOM_2 = 8550,

    QUEST_ALLIANCE_LINEN_BANDAGE_1 = 8517,
    QUEST_ALLIANCE_LINEN_BANDAGE_2 = 8518,

    QUEST_ALLIANCE_RAINBOW_FIN_ALBACORE_1 = 8524,
    QUEST_ALLIANCE_RAINBOW_FIN_ALBACORE_2 = 8525,

    QUEST_ALLIANCE_LIGHT_LEATHER_1 = 8511,
    QUEST_ALLIANCE_LIGHT_LEATHER_2 = 8512,

    QUEST_HORDE_WOOL_BANDAGES_1 = 8604,
    QUEST_HORDE_WOOL_BANDAGES_2 = 8605,

    QUEST_ALLIANCE_STRANGLEKELP_1 = 8503,
    QUEST_ALLIANCE_STRANGLEKELP_2 = 8504,

    QUEST_ALLIANCE_MEDIUM_LEATHER_1 = 8513,
    QUEST_ALLIANCE_MEDIUM_LEATHER_2 = 8514,

    QUEST_HORDE_LEAN_WOLF_STEAKS_1 = 8611,
    QUEST_HORDE_LEAN_WOLF_STEAKS_2 = 8612,

    QUEST_HORDE_TIN_BARS_1 = 8542,
    QUEST_HORDE_TIN_BARS_2 = 8543,

    QUEST_ALLIANCE_SILK_BANDAGES_1 = 8520,
    QUEST_ALLIANCE_SILK_BANDAGES_2 = 8521,

    QUEST_ALLIANCE_IRON_BARS_1 = 8494,
    QUEST_ALLIANCE_IRON_BARS_2 = 8495,

    QUEST_ALLIANCE_ROAST_RAPTOR_1 = 8526,
    QUEST_ALLIANCE_ROAST_RAPTOR_2 = 8527,

    QUEST_HORDE_FIREBLOOM_1 = 8580,
    QUEST_HORDE_FIREBLOOM_2 = 8581,

    QUEST_HORDE_HEAVY_LEATHER_1 = 8588,
    QUEST_HORDE_HEAVY_LEATHER_2 = 8589,

    QUEST_ALLIANCE_SPOTTED_YELLOWTAIL_1 = 8528,
    QUEST_ALLIANCE_SPOTTED_YELLOWTAIL_2 = 8529,
    QUEST_HORDE_SPOTTED_YELLOWTAIL_1 = 8613,
    QUEST_HORDE_SPOTTED_YELLOWTAIL_2 = 8614,

    QUEST_ALLIANCE_THICK_LEATHER_1 = 8515,
    QUEST_ALLIANCE_THICK_LEATHER_2 = 8516,
    QUEST_HORDE_THICK_LEATHER_1 = 8590,
    QUEST_HORDE_THICK_LEATHER_2 = 8591,

    QUEST_HORDE_MITHRIL_BARS_1 = 8545,
    QUEST_HORDE_MITHRIL_BARS_2 = 8546,

    QUEST_ALLIANCE_RUNECLOTH_BANDAGES_1 = 8522,
    QUEST_ALLIANCE_RUNECLOTH_BANDAGES_2 = 8523,
    QUEST_HORDE_RUNECLOTH_BANDAGES_1 = 8609,
    QUEST_HORDE_RUNECLOTH_BANDAGES_2 = 8610,

    QUEST_ALLIANCE_ARTHAS_TEARS_1 = 8509,
    QUEST_ALLIANCE_ARTHAS_TEARS_2 = 8510,

    QUEST_ALLIANCE_THORIUM_BARS_1 = 8499,
    QUEST_ALLIANCE_THORIUM_BARS_2 = 8500,

    QUEST_HORDE_BAKED_SALMON_1 = 8615,
    QUEST_HORDE_BAKED_SALMON_2 = 8616,

    QUEST_HORDE_RUGGED_LEATHER_1 = 8600,
    QUEST_HORDE_RUGGED_LEATHER_2 = 8601,

    QUEST_HORDE_MAGEWEAVE_BANDAGE_1 = 8607,
    QUEST_HORDE_MAGEWEAVE_BANDAGE_2 = 8608,

    QUEST_BANG_A_GONG               = 8743,

    SAY_BANG_GONG                   = 11427,
    SAY_QIRAJI_BREAK                = 11432,
};

bool QuestRewarded_war_effort(Player* player, Creature* creature, Quest const* quest)
{
    uint32 itemCount = quest->ReqItemCount[0];
    AQResources resource;
    switch (quest->GetQuestId())
    {
        // common
        case QUEST_HORDE_COPPER_BARS_1:
        case QUEST_HORDE_COPPER_BARS_2: resource = AQ_COPPER_BAR_HORDE; break;
        case QUEST_ALLIANCE_COPPER_BARS_1:
        case QUEST_ALLIANCE_COPPER_BARS_2: resource = AQ_COPPER_BAR_ALLY; break;
        case QUEST_ALLIANCE_PURPLE_LOTUS_1:
        case QUEST_ALLIANCE_PURPLE_LOTUS_2: resource = AQ_PURPLE_LOTUS_ALLY; break;
        case QUEST_ALLIANCE_SPOTTED_YELLOWTAIL_1:
        case QUEST_ALLIANCE_SPOTTED_YELLOWTAIL_2: resource = AQ_SPOTTED_YELLOWTAIL_ALLY; break;
        case QUEST_HORDE_SPOTTED_YELLOWTAIL_1:
        case QUEST_HORDE_SPOTTED_YELLOWTAIL_2: resource = AQ_SPOTTED_YELLOWTAIL_HORDE; break;
        case QUEST_ALLIANCE_THICK_LEATHER_1:
        case QUEST_ALLIANCE_THICK_LEATHER_2: resource = AQ_THICK_LEATHER_ALLY; break;
        case QUEST_HORDE_THICK_LEATHER_1:
        case QUEST_HORDE_THICK_LEATHER_2: resource = AQ_THICK_LEATHER_HORDE; break;
        case QUEST_ALLIANCE_RUNECLOTH_BANDAGES_1:
        case QUEST_ALLIANCE_RUNECLOTH_BANDAGES_2: resource = AQ_RUNECLOTH_BANDAGE_ALLY; break;
        case QUEST_HORDE_RUNECLOTH_BANDAGES_1:
        case QUEST_HORDE_RUNECLOTH_BANDAGES_2: resource = AQ_RUNECLOTH_BANDAGE_HORDE; break;
        case QUEST_HORDE_PURPLE_LOTUS_1:
        case QUEST_HORDE_PURPLE_LOTUS_2: resource = AQ_PURPLE_LOTUS_HORDE; break;
        // horde
        case QUEST_HORDE_PEACEBLOOM_1:
        case QUEST_HORDE_PEACEBLOOM_2: resource = AQ_PEACEBLOOM; break;
        case QUEST_HORDE_WOOL_BANDAGES_1:
        case QUEST_HORDE_WOOL_BANDAGES_2: resource = AQ_WOOL_BANDAGE; break;
        case QUEST_HORDE_LEAN_WOLF_STEAKS_1:
        case QUEST_HORDE_LEAN_WOLF_STEAKS_2: resource = AQ_LEAN_WOLF_STEAK; break;
        case QUEST_HORDE_TIN_BARS_1:
        case QUEST_HORDE_TIN_BARS_2: resource = AQ_TIN_BAR; break;
        case QUEST_HORDE_FIREBLOOM_1:
        case QUEST_HORDE_FIREBLOOM_2: resource = AQ_FIREBLOOM; break;
        case QUEST_HORDE_HEAVY_LEATHER_1:
        case QUEST_HORDE_HEAVY_LEATHER_2: resource = AQ_HEAVY_LEATHER; break;
        case QUEST_HORDE_MITHRIL_BARS_1:
        case QUEST_HORDE_MITHRIL_BARS_2: resource = AQ_MITHRIL_BAR; break;
        case QUEST_HORDE_BAKED_SALMON_1:
        case QUEST_HORDE_BAKED_SALMON_2: resource = AQ_BAKED_SALMON; break;
        case QUEST_HORDE_RUGGED_LEATHER_1:
        case QUEST_HORDE_RUGGED_LEATHER_2: resource = AQ_RUGGED_LEATHER; break;
        case QUEST_HORDE_MAGEWEAVE_BANDAGE_1:
        case QUEST_HORDE_MAGEWEAVE_BANDAGE_2: resource = AQ_MAGEWEAVE_BANDAGE; break;
        // alliance
        case QUEST_ALLIANCE_LINEN_BANDAGE_1:
        case QUEST_ALLIANCE_LINEN_BANDAGE_2: resource = AQ_LINEN_BANDAGE; break;
        case QUEST_ALLIANCE_RAINBOW_FIN_ALBACORE_1:
        case QUEST_ALLIANCE_RAINBOW_FIN_ALBACORE_2: resource = AQ_RAINBOW_FIN_ALBACORE; break;
        case QUEST_ALLIANCE_LIGHT_LEATHER_1:
        case QUEST_ALLIANCE_LIGHT_LEATHER_2: resource = AQ_LIGHT_LEATHER; break;
        case QUEST_ALLIANCE_STRANGLEKELP_1:
        case QUEST_ALLIANCE_STRANGLEKELP_2: resource = AQ_STRANGLEKELP; break;
        case QUEST_ALLIANCE_MEDIUM_LEATHER_1:
        case QUEST_ALLIANCE_MEDIUM_LEATHER_2: resource = AQ_MEDIUM_LEATHER; break;
        case QUEST_ALLIANCE_SILK_BANDAGES_1:
        case QUEST_ALLIANCE_SILK_BANDAGES_2: resource = AQ_SILK_BANDAGE; break;
        case QUEST_ALLIANCE_IRON_BARS_1:
        case QUEST_ALLIANCE_IRON_BARS_2: resource = AQ_IRON_BAR; break;
        case QUEST_ALLIANCE_ROAST_RAPTOR_1:
        case QUEST_ALLIANCE_ROAST_RAPTOR_2: resource = AQ_ROAST_RAPTOR; break;
        case QUEST_ALLIANCE_ARTHAS_TEARS_1:
        case QUEST_ALLIANCE_ARTHAS_TEARS_2: resource = AQ_ARTHAS_TEARS; break;
        case QUEST_ALLIANCE_THORIUM_BARS_1:
        case QUEST_ALLIANCE_THORIUM_BARS_2: resource = AQ_THORIUM_BAR; break;
        default: return false;
    }

    sWorldState.AddWarEffortProgress(resource, itemCount);
    return true;
}

// dont ask me why - dont want to pollute header file with this for now - if we move it from bg to map it might be a good thing
class BattleGroundBroadcastBuilder
{
    public:
        BattleGroundBroadcastBuilder(BroadcastText const* bcd, ChatMsg msgtype, Creature const* source, Unit const* target)
            : i_msgtype(msgtype), i_source(source), i_bcd(bcd), i_target(target) {}
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            ChatHandler::BuildChatPacket(data, i_msgtype, i_bcd->GetText(loc_idx, i_source ? i_source->getGender() : GENDER_NONE).c_str(), i_bcd->languageId, CHAT_TAG_NONE, i_source ? i_source->GetObjectGuid() : ObjectGuid(), i_source ? i_source->GetName() : "", i_target ? i_target->GetObjectGuid() : ObjectGuid());
        }
    private:
        ChatMsg i_msgtype;
        Creature const* i_source;
        BroadcastText const* i_bcd;
        Unit const* i_target;
};

void SendBgNeutralToPlayer(int32 bcdEntry, Unit* target)
{
    BattleGroundBroadcastBuilder bg_builder(sObjectMgr.GetBroadcastText(bcdEntry), CHAT_MSG_BG_SYSTEM_NEUTRAL, nullptr, target);
    MaNGOS::LocalizedPacketDo<BattleGroundBroadcastBuilder> bg_do(bg_builder);
    target->GetMap()->ExecuteMapWorkerZone(ZONE_ID_SILITHUS, [&](Player* player)
    {
        bg_do(player);
    });
}

bool QuestRewarded_war_effort(Player* player, GameObject* go, Quest const* quest)
{
    if (quest->GetQuestId() == QUEST_BANG_A_GONG)
    {
        if (sWorldState.GetAqPhase() == PHASE_3_GONG_TIME)
        {
            SendBgNeutralToPlayer(SAY_BANG_GONG, player);
            SendBgNeutralToPlayer(SAY_QIRAJI_BREAK, player);
            if (InstanceData* data = go->GetInstanceData())
                data->SetData(TYPE_GONG_TIME, 0);
            sWorldState.HandleWarEffortPhaseTransition(PHASE_4_10_HOUR_WAR);
        }
    }
    return true;
}

struct SilithusBossAI : public CombatAI
{
    SilithusBossAI(Creature* creature) : CombatAI(creature, 0) {}

    void JustRespawned() override
    {
        CombatAI::JustRespawned();
        int32 textId = 0;
        switch (m_creature->GetEntry())
        {
            case NPC_COLOSSUS_OF_ASHI: textId = 11426; break;
            case NPC_COLOSSUS_OF_REGAL: textId = 11424; break;
            case NPC_COLOSSUS_OF_ZORA: textId = 11425; break;
            default: break;
        }
        DoBroadcastText(textId, m_creature);
    }

    void JustDied(Unit* /*killer*/) override
    {
        AQSilithusBoss boss;
        switch (m_creature->GetEntry())
        {
            default:
            case NPC_COLOSSUS_OF_ASHI: boss = AQ_SILITHUS_BOSS_ASHI; break;
            case NPC_COLOSSUS_OF_REGAL: boss = AQ_SILITHUS_BOSS_REGAL; break;
            case NPC_COLOSSUS_OF_ZORA: boss = AQ_SILITHUS_BOSS_ZORA; break;
        }
        sWorldState.SetSilithusBossKilled(boss);
    }
};

void AddSC_war_effort()
{
    Script* pNewScript = new Script;
    pNewScript->Name = "npc_war_effort";
    pNewScript->pQuestRewardedNPC = &QuestRewarded_war_effort;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "go_scarab_gong";
    pNewScript->pQuestRewardedGO = &QuestRewarded_war_effort;
    pNewScript->RegisterSelf();

    pNewScript = new Script;
    pNewScript->Name = "npc_silithus_boss";
    pNewScript->GetAI = &GetNewAIInstance<SilithusBossAI>;
    pNewScript->RegisterSelf();
}
