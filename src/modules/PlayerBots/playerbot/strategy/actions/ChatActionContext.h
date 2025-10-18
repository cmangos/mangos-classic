#pragma once

#include "ListQuestsActions.h"
#include "StatsAction.h"
#include "LeaveGroupAction.h"
#include "TellReputationAction.h"
#include "LogLevelAction.h"
#include "TellLosAction.h"
#include "DropQuestAction.h"
#include "QueryQuestAction.h"
#include "QueryItemUsageAction.h"
#include "LootStrategyAction.h"
#include "AddLootAction.h"
#include "LootRollAction.h"
#include "ReleaseSpiritAction.h"
#include "TeleportAction.h"
#include "TaxiAction.h"
#include "RepairAllAction.h"
#include "UseItemAction.h"
#include "TellItemCountAction.h"
#include "RewardAction.h"
#include "BuyAction.h"
#include "SellAction.h"
#include "UnequipAction.h"
#include "EquipAction.h"
#include "KeepItemAction.h"
#include "TradeAction.h"
#include "ChangeTalentsAction.h"
#include "ListSpellsAction.h"
#include "ChangeStrategyAction.h"
#include "TrainerAction.h"
#include "ChangeChatAction.h"
#include "SetHomeAction.h"
#include "ResetAiAction.h"
#include "DestroyItemAction.h"
#include "BuffAction.h"
#include "AttackAction.h"
#include "HelpAction.h"
#include "GuildBankAction.h"
#include "ChatShortcutActions.h"
#include "GossipHelloAction.h"
#include "CastCustomSpellAction.h"
#include "InviteToGroupAction.h"
#include "TellCastFailedAction.h"
#include "RtiAction.h"
#include "ReviveFromCorpseAction.h"
#include "BankAction.h"
#include "PositionAction.h"
#include "TellTargetAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"
#include "SaveManaAction.h"
#include "playerbot/strategy/values/Formations.h"
#include "playerbot/strategy/values/Stances.h"
#include "CustomStrategyEditAction.h"
#include "AhAction.h"
#include "DebugAction.h"
#include "GoAction.h"
#include "MailAction.h"
#include "SendMailAction.h"
#include "ShareQuestAction.h"
#include "SkipSpellsListAction.h"
#include "CustomStrategyEditAction.h"
#include "FlagAction.h"
#include "HireAction.h"
#include "RangeAction.h"
#include "SetCraftAction.h"
#include "WtsAction.h"
#include "PassLeadershipToMasterAction.h"
#include "CheatAction.h"
#include "GuildManagementActions.h"
#include "RtscAction.h"
#include "BattleGroundJoinAction.h"
#include "MoveStyleAction.h"
#include "ValueActions.h"
#include "QuestRewardActions.h"
#include "ChooseTravelTargetAction.h"
#include "SkillAction.h"
#include "FactionAction.h"
#include "SetValueAction.h"
#include "GlyphAction.h"

namespace ai
{
    class ChatActionContext : public NamedObjectContext<Action>
    {
    public:
        ChatActionContext()
        {
            creators["range"] = [](PlayerbotAI* ai) { return new RangeAction(ai); };
            creators["stats"] = [](PlayerbotAI* ai) { return new StatsAction(ai); };
            creators["quests"] = [](PlayerbotAI* ai) { return new ListQuestsAction(ai); };
            creators["quest reward"] = [](PlayerbotAI* ai) { return new QuestRewardAction(ai); };
            creators["leave"] = [](PlayerbotAI* ai) { return new LeaveGroupAction(ai); };
            creators["reputation"] = [](PlayerbotAI* ai) { return new TellReputationAction(ai); };
            creators["log"] = [](PlayerbotAI* ai) { return new LogLevelAction(ai); };
            creators["los"] = [](PlayerbotAI* ai) { return new TellLosAction(ai); };
            creators["drop"] = [](PlayerbotAI* ai) { return new DropQuestAction(ai); };
            creators["clean quest log"] = [](PlayerbotAI* ai) { return new CleanQuestLogAction(ai); };
            creators["share"] = [](PlayerbotAI* ai) { return new ShareQuestAction(ai); };
            creators["query quest"] = [](PlayerbotAI* ai) { return new QueryQuestAction(ai); };
            creators["query item usage"] = [](PlayerbotAI* ai) { return new QueryItemUsageAction(ai); };
            creators["ll"] = [](PlayerbotAI* ai) { return new LootStrategyAction(ai); };
            creators["ss"] = [](PlayerbotAI* ai) { return new SkipSpellsListAction(ai); };
            creators["loot"] = [](PlayerbotAI* ai) { return new AddAllLootAction(ai); };
            creators["add all loot"] = [](PlayerbotAI* ai) { return new AddAllLootAction(ai); };
            creators["roll"] = [](PlayerbotAI* ai) { return new RollAction(ai); };
            creators["release"] = [](PlayerbotAI* ai) { return new ReleaseSpiritAction(ai); };
            creators["repop"] = [](PlayerbotAI* ai) { return new RepopAction(ai); };
            creators["teleport"] = [](PlayerbotAI* ai) { return new TeleportAction(ai); };
            creators["taxi"] = [](PlayerbotAI* ai) { return new TaxiAction(ai); };
            creators["repair"] = [](PlayerbotAI* ai) { return new RepairAllAction(ai); };
            creators["use"] = [](PlayerbotAI* ai) { return new UseAction(ai); };
            creators["item count"] = [](PlayerbotAI* ai) { return new TellItemCountAction(ai); };
            creators["equip"] = [](PlayerbotAI* ai) { return new EquipAction(ai); };
            creators["equip upgrades"] = [](PlayerbotAI* ai) { return new EquipUpgradesAction(ai); };
            creators["unequip"] = [](PlayerbotAI* ai) { return new UnequipAction(ai); };
            creators["keep"] = [](PlayerbotAI* ai) { return new KeepItemAction(ai); };
            creators["sell"] = [](PlayerbotAI* ai) { return new SellAction(ai); };
            creators["buy"] = [](PlayerbotAI* ai) { return new BuyAction(ai); };
            creators["buy back"] = [](PlayerbotAI* ai) { return new BuyBackAction(ai); };
            creators["reward"] = [](PlayerbotAI* ai) { return new RewardAction(ai); };
            creators["trade"] = [](PlayerbotAI* ai) { return new TradeAction(ai); };
            creators["talents"] = [](PlayerbotAI* ai) { return new ChangeTalentsAction(ai); };
            creators["spells"] = [](PlayerbotAI* ai) { return new ListSpellsAction(ai); };
            creators["co"] = [](PlayerbotAI* ai) { return new ChangeCombatStrategyAction(ai); };
            creators["nc"] = [](PlayerbotAI* ai) { return new ChangeNonCombatStrategyAction(ai); };
            creators["de"] = [](PlayerbotAI* ai) { return new ChangeDeadStrategyAction(ai); };
            creators["react"] = [](PlayerbotAI* ai) { return new ChangeReactionStrategyAction(ai); };
            creators["all"] = [](PlayerbotAI* ai) { return new ChangeAllStrategyAction(ai); };
            creators["trainer"] = [](PlayerbotAI* ai) { return new TrainerAction(ai); };
            creators["attack my target"] = [](PlayerbotAI* ai) { return new AttackMyTargetAction(ai); };
            creators["attack rti target"] = [](PlayerbotAI* ai) { return new AttackRTITargetAction(ai); };
            creators["chat"] = [](PlayerbotAI* ai) { return new ChangeChatAction(ai); };
            creators["home"] = [](PlayerbotAI* ai) { return new SetHomeAction(ai); };
            creators["destroy"] = [](PlayerbotAI* ai) { return new DestroyItemAction(ai); };
            creators["load ai"] = [](PlayerbotAI* ai) { return new LoadAiAction(ai); };
            creators["list ai"] = [](PlayerbotAI* ai) { return new ListAiAction(ai); };
            creators["save ai"] = [](PlayerbotAI* ai) { return new SaveAiAction(ai); };
            creators["reset ai"] = [](PlayerbotAI* ai) { return new ResetAiAction(ai, true); };
            creators["reset strats"] = [](PlayerbotAI* ai) { return new ResetStratsAction(ai); };
            creators["reset values"] = [](PlayerbotAI* ai) { return new ResetValuesAction(ai); };
            creators["reset ai soft"] = [](PlayerbotAI* ai) { return new ResetAiAction(ai, false); };
            creators["buff"] = [](PlayerbotAI* ai) { return new BuffAction(ai); };
            creators["help"] = [](PlayerbotAI* ai) { return new HelpAction(ai); };
            creators["gb"] = [](PlayerbotAI* ai) { return new GuildBankAction(ai); };
            creators["bank"] = [](PlayerbotAI* ai) { return new BankAction(ai); };
            creators["follow chat shortcut"] = [](PlayerbotAI* ai) { return new FollowChatShortcutAction(ai); };
            creators["stay chat shortcut"] = [](PlayerbotAI* ai) { return new StayChatShortcutAction(ai); };
            creators["guard chat shortcut"] = [](PlayerbotAI* ai) { return new GuardChatShortcutAction(ai); };
            creators["free chat shortcut"] = [](PlayerbotAI* ai) { return new FreeChatShortcutAction(ai); };
            creators["flee chat shortcut"] = [](PlayerbotAI* ai) { return new FleeChatShortcutAction(ai); };
            creators["runaway chat shortcut"] = [](PlayerbotAI* ai) { return new GoawayChatShortcutAction(ai); };
            creators["grind chat shortcut"] = [](PlayerbotAI* ai) { return new GrindChatShortcutAction(ai); };
            creators["tank attack chat shortcut"] = [](PlayerbotAI* ai) { return new TankAttackChatShortcutAction(ai); };
            creators["gossip hello"] = [](PlayerbotAI* ai) { return new GossipHelloAction(ai); };
            creators["cast"] = [](PlayerbotAI* ai) { return new CastCustomSpellAction(ai); };
            creators["cast custom nc spell"] = [](PlayerbotAI* ai) { return new CastCustomNcSpellAction(ai); };
            creators["invite"] = [](PlayerbotAI* ai) { return new InviteToGroupAction(ai); };
            creators["join"] = [](PlayerbotAI* ai) { return new JoinGroupAction(ai); };
            creators["lfg"] = [](PlayerbotAI* ai) { return new LfgAction(ai); };
            creators["spell"] = [](PlayerbotAI* ai) { return new TellSpellAction(ai); };
            creators["rti"] = [](PlayerbotAI* ai) { return new RtiAction(ai); };
            creators["spirit healer"] = [](PlayerbotAI* ai) { return new SpiritHealerAction(ai); };
            creators["position"] = [](PlayerbotAI* ai) { return new PositionAction(ai); };
            creators["tell target"] = [](PlayerbotAI* ai) { return new TellTargetAction(ai); };
            creators["summon"] = [](PlayerbotAI* ai) { return new SummonAction(ai); };
            creators["who"] = [](PlayerbotAI* ai) { return new WhoAction(ai); };
            creators["save mana"] = [](PlayerbotAI* ai) { return new SaveManaAction(ai); };
            creators["max dps chat shortcut"] = [](PlayerbotAI* ai) { return new MaxDpsChatShortcutAction(ai); };
            creators["tell possible attack targets"] = [](PlayerbotAI* ai) { return new TellPossibleAttackTargetsAction(ai); };
            creators["tell attackers"] = [](PlayerbotAI* ai) { return new TellAttackersAction(ai); };
            creators["formation"] = [](PlayerbotAI* ai) { return new SetFormationAction(ai); };
            creators["stance"] = [](PlayerbotAI* ai) { return new SetStanceAction(ai); };
            creators["sendmail"] = [](PlayerbotAI* ai) { return new SendMailAction(ai); };
            creators["mail"] = [](PlayerbotAI* ai) { return new MailAction(ai); };
            creators["go"] = [](PlayerbotAI* ai) { return new GoAction(ai); };
            creators["debug"] = [](PlayerbotAI* ai) { return new DebugAction(ai); };
            creators["cdebug"] = [](PlayerbotAI* ai) { return new DebugAction(ai); };
            creators["cs"] = [](PlayerbotAI* ai) { return new CustomStrategyEditAction(ai); };
            creators["wts"] = [](PlayerbotAI* ai) { return new WtsAction(ai); };
            creators["hire"] = [](PlayerbotAI* ai) { return new HireAction(ai); };
            creators["craft"] = [](PlayerbotAI* ai) { return new SetCraftAction(ai); };
            creators["flag"] = [](PlayerbotAI* ai) { return new FlagAction(ai); };
            creators["give leader"] = [](PlayerbotAI* ai) { return new GiveLeaderAction(ai); };
            creators["cheat"] = [](PlayerbotAI* ai) { return new CheatAction(ai); };
            creators["rtsc"] = [](PlayerbotAI* ai) { return new RTSCAction(ai); };
            creators["ah"] = [](PlayerbotAI* ai) { return new AhAction(ai); };
            creators["ah bid"] = [](PlayerbotAI* ai) { return new AhBidAction(ai); };
            creators["wait for attack time"] = [](PlayerbotAI* ai) { return new SetWaitForAttackTimeAction(ai); };
            creators["self resurrect"] = [](PlayerbotAI* ai) { return new SelfResurrectAction(ai); };
            creators["pet"] = [](PlayerbotAI* ai) { return new SetPetAction(ai); };

            creators["focus heal targets"] = [](PlayerbotAI* ai) { return new SetFocusHealTargetsAction(ai); };
            creators["follow target"] = [](PlayerbotAI* ai) { return new SetFollowTargetAction(ai); };
            creators["boost targets"] = [](PlayerbotAI* ai) { return new SetBoostTargetsAction(ai); };
            creators["revive targets"] = [](PlayerbotAI* ai) { return new SetReviveTargetsAction(ai); };
            creators["buff targets"] = [](PlayerbotAI* ai) { return new SetBuffTargetsAction(ai); };

            creators["guild invite"] = [](PlayerbotAI* ai) { return new GuildInviteAction(ai); };
            creators["guild join"] = [](PlayerbotAI* ai) { return new GuildJoinAction(ai); };
            creators["guild promote"] = [](PlayerbotAI* ai) { return new GuildPromoteAction(ai); };
            creators["guild demote"] = [](PlayerbotAI* ai) { return new GuildDemoteAction(ai); };
            creators["guild remove"] = [](PlayerbotAI* ai) { return new GuildRemoveAction(ai); };
            creators["guild leave"] = [](PlayerbotAI* ai) { return new GuildLeaveAction(ai); };
            creators["guild leader"] = [](PlayerbotAI* ai) { return new GuildLeaderAction(ai); };

            creators["bg free"] = [](PlayerbotAI* ai) { return new BGLeaveAction(ai); };
            creators["move style"] = [](PlayerbotAI* ai) { return new MoveStyleAction(ai); };

            creators["jump"] = [](PlayerbotAI* ai) { return new JumpAction(ai); };
            creators["doquest"] = [](PlayerbotAI* ai) { return new FocusTravelTargetAction(ai); };
            creators["skill"] = [](PlayerbotAI* ai) { return new SkillAction(ai); };
            creators["faction"] = [](PlayerbotAI* ai) { return new FactionAction(ai); };
            creators["set value"] = [](PlayerbotAI* ai) { return new SetValueAction(ai); };
            creators["glyph"] = [](PlayerbotAI* ai) { return new GlyphAction(ai); };
       }
    };
};
