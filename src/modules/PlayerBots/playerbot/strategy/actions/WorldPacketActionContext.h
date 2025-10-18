#pragma once

#include "AcceptInvitationAction.h"
#include "PassLeadershipToMasterAction.h"
#include "TellMasterAction.h"
#include "TalkToQuestGiverAction.h"
#include "AcceptQuestAction.h"
#include "LootRollAction.h"
#include "ReviveFromCorpseAction.h"
#include "AcceptResurrectAction.h"
#include "UseMeetingStoneAction.h"
#include "AreaTriggerAction.h"
#include "CheckMountStateAction.h"
#include "RememberTaxiAction.h"
#include "TradeStatusAction.h"
#include "InventoryChangeFailureAction.h"
#include "LootAction.h"
#include "QuestAction.h"
#include "LeaveGroupAction.h"
#include "TellCastFailedAction.h"
#include "AcceptDuelAction.h"
#include "ReadyCheckAction.h"
#include "LfgActions.h"
#include "SecurityCheckAction.h"
#include "GuildAcceptAction.h"
#include "AcceptBattleGroundInvitationAction.h"
#include "PetitionSignAction.h"
#include "BattleGroundJoinAction.h"
#include "SeeSpellAction.h"
#include "ArenaTeamActions.h"

namespace ai
{
    class WorldPacketActionContext : public NamedObjectContext<Action>
    {
    public:
        WorldPacketActionContext()
        {   
            creators["lfg join"] = [](PlayerbotAI* ai) { return new LfgJoinAction(ai); };
            creators["lfg accept"] = [](PlayerbotAI* ai) { return new LfgAcceptAction(ai); };
            creators["lfg role check"] = [](PlayerbotAI* ai) { return new LfgRoleCheckAction(ai); };
            creators["lfg leave"] = [](PlayerbotAI* ai) { return new LfgLeaveAction(ai); };
            creators["lfg teleport"] = [](PlayerbotAI* ai) { return new LfgTeleportAction(ai); };
            creators["bg status check"] = [](PlayerbotAI* ai) { return new BGStatusCheckAction(ai); };
            creators["bg status"] = [](PlayerbotAI* ai) { return new BGStatusAction(ai); };
            creators["bg join"] = [](PlayerbotAI* ai) { return new BGJoinAction(ai); };
            creators["bg leave"] = [](PlayerbotAI* ai) { return new BGLeaveAction(ai); };
            creators["arena tactics"] = [](PlayerbotAI* ai) { return new ArenaTactics(ai); };
            creators["accept invitation"] = [](PlayerbotAI* ai) { return new AcceptInvitationAction(ai); };
            creators["give leader in dungeon"] = [](PlayerbotAI* ai) { return new GiveLeaderAction(ai, "I don't know this dungeon, lead the way!"); };
            creators["leader"] = [](PlayerbotAI* ai) { return new PassLeadershipToMasterAction(ai); };
            creators["tell not enough money"] = [](PlayerbotAI* ai) { return new TellMasterAction(ai, "Not enough money"); };
            creators["tell not enough reputation"] = [](PlayerbotAI* ai) { return new TellMasterAction(ai, "Not enough reputation"); };
            creators["tell cannot equip"] = [](PlayerbotAI* ai) { return new InventoryChangeFailureAction(ai); };
            creators["talk to quest giver"] = [](PlayerbotAI* ai) { return new TalkToQuestGiverAction(ai); };
            creators["accept quest"] = [](PlayerbotAI* ai) { return new AcceptQuestAction(ai); };
            creators["confirm quest"] = [](PlayerbotAI* ai) { return new ConfirmQuestAction(ai); };
            creators["quest details"] = [](PlayerbotAI* ai) { return new QuestDetailsAction(ai); };
            creators["accept all quests"] = [](PlayerbotAI* ai) { return new AcceptAllQuestsAction(ai); };
            creators["accept quest share"] = [](PlayerbotAI* ai) { return new AcceptQuestShareAction(ai); };
            creators["loot start roll"] = [](PlayerbotAI* ai) { return (QueryItemUsageAction*)new LootStartRollAction(ai); };
            creators["loot roll"] = [](PlayerbotAI* ai) { return (QueryItemUsageAction*)new LootRollAction(ai); };
            creators["revive from corpse"] = [](PlayerbotAI* ai) { return new ReviveFromCorpseAction(ai); };
            creators["find corpse"] = [](PlayerbotAI* ai) { return new FindCorpseAction(ai); };
            creators["auto release"] = [](PlayerbotAI* ai) { return new AutoReleaseSpiritAction(ai); };
            creators["accept resurrect"] = [](PlayerbotAI* ai) { return new AcceptResurrectAction(ai); };
            creators["use meeting stone"] = [](PlayerbotAI* ai) { return new UseMeetingStoneAction(ai); };
            creators["accept summon"] = [](PlayerbotAI* ai) { return new AcceptSummonAction(ai); };
            creators["area trigger"] = [](PlayerbotAI* ai) { return new AreaTriggerAction(ai); };
            creators["reach area trigger"] = [](PlayerbotAI* ai) { return new ReachAreaTriggerAction(ai); };
            creators["check mount state"] = [](PlayerbotAI* ai) { return new CheckMountStateAction(ai); };
            creators["remember taxi"] = [](PlayerbotAI* ai) { return new RememberTaxiAction(ai); };
            creators["accept trade"] = [](PlayerbotAI* ai) { return new TradeStatusAction(ai); };
            creators["store loot"] = [](PlayerbotAI* ai) { return new StoreLootAction(ai); };
            creators["quest update add kill"] = [](PlayerbotAI* ai) { return new QuestUpdateAddKillAction(ai); };
            creators["quest update add item"] = [](PlayerbotAI* ai) { return new QuestUpdateAddItemAction(ai); };
            creators["quest update failed"] = [](PlayerbotAI* ai) { return new QuestUpdateFailedAction(ai); };
            creators["quest update failed timer"] = [](PlayerbotAI* ai) { return new QuestUpdateFailedTimerAction(ai); };
            creators["quest update complete"] = [](PlayerbotAI* ai) { return new QuestUpdateCompleteAction(ai); };
            creators["party command"] = [](PlayerbotAI* ai) { return new PartyCommandAction(ai); };
            creators["tell cast failed"] = [](PlayerbotAI* ai) { return new TellCastFailedAction(ai); };
            creators["accept duel"] = [](PlayerbotAI* ai) { return new AcceptDuelAction(ai); };
            creators["ready check"] = [](PlayerbotAI* ai) { return new ReadyCheckAction(ai); };
            creators["ready check finished"] = [](PlayerbotAI* ai) { return new FinishReadyCheckAction(ai); };
            creators["uninvite"] = [](PlayerbotAI* ai) { return new UninviteAction(ai); };
            creators["security check"] = [](PlayerbotAI* ai) { return new SecurityCheckAction(ai); };
            creators["guild accept"] = [](PlayerbotAI* ai) { return new GuildAcceptAction(ai); };
            creators["inventory change failure"] = [](PlayerbotAI* ai) { return new InventoryChangeFailureAction(ai); };
            creators["petition sign"] = [](PlayerbotAI* ai) { return new PetitionSignAction(ai); };
            creators["see spell"] = [](PlayerbotAI* ai) { return new SeeSpellAction(ai); };
            creators["arena team accept"] = [](PlayerbotAI* ai) { return new ArenaTeamAcceptAction(ai); };
        }
    };
};
