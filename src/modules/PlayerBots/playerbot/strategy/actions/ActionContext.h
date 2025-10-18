#pragma once

#include "GenericActions.h"
#include "EmoteAction.h"
#include "AddLootAction.h"
#include "LootAction.h"
#include "AddLootAction.h"
#include "LootRollAction.h"
#include "StayActions.h"
#include "FollowActions.h"
#include "ChangeStrategyAction.h"
#include "ChooseTargetActions.h"
#include "SuggestWhatToDoAction.h"
#include "PositionAction.h"
#include "AttackAction.h"
#include "CheckMailAction.h"
#include "CheckValuesAction.h"
#include "ChooseRpgTargetAction.h"
#include "ChooseTravelTargetAction.h"
#include "DelayAction.h"
#include "GiveItemAction.h"
#include "GreetAction.h"
#include "ImbueAction.h"
#include "MovementActions.h"
#include "MoveToRpgTargetAction.h"
#include "MoveToTravelTargetAction.h"
#include "OutfitAction.h"
#include "RevealGatheringItemAction.h"
#include "SayAction.h"
#include "OutfitAction.h"
#include "RandomBotUpdateAction.h"
#include "RemoveAuraAction.h"
#include "RpgAction.h"
#include "TravelAction.h"
#include "RtiAction.h"
#include "BattleGroundTactics.h"
#include "CheckMountStateAction.h"
#include "ChangeTalentsAction.h"
#include "AutoLearnSpellAction.h"
#include "XpGainAction.h"
#include "HonorGainAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "ReleaseSpiritAction.h"
#include "CombatActions.h"
#include "WorldBuffAction.h"
#include "CastCustomSpellAction.h"
#include "BattleGroundJoinAction.h"
#include "DestroyItemAction.h"
#include "ResetInstancesAction.h"
#include "BuyAction.h"
#include "GuildCreateActions.h"
#include "GuildManagementActions.h"
#include "GuildAcceptAction.h"
#include "RpgSubActions.h"
#include "VehicleActions.h"
#include "UseTrinketAction.h"
#include "BotStateActions.h"
#include "WaitForAttackAction.h"
#include "PullActions.h"
#include "ResetAiAction.h"
#include "ShareQuestAction.h"
#include "UpdateGearAction.h"
#include "SetAvoidAreaAction.h"
#include "GlyphAction.h"
#include "FishAction.h"
#include "AutoCompleteQuestAction.h"
#include "UnstuckAction.h"

#include "OnyxiasLairDungeonActions.h"
#include "MoltenCoreDungeonActions.h"
#include "KarazhanDungeonActions.h"
#include "NaxxramasDungeonActions.h"

namespace ai
{
    class ActionContext : public NamedObjectContext<Action>
    {
    public:
        ActionContext()
        {
            creators["mark rti"] = [](PlayerbotAI* ai) { return new MarkRtiAction(ai); };
            creators["set return position"] = [](PlayerbotAI* ai) { return new SetReturnPositionAction(ai); };
            creators["rpg"] = [](PlayerbotAI* ai) { return new RpgAction(ai); };
            creators["crpg"] = [](PlayerbotAI* ai) { return new CRpgAction(ai); };
            creators["choose rpg target"] = [](PlayerbotAI* ai) { return new ChooseRpgTargetAction(ai); };
            creators["move to rpg target"] = [](PlayerbotAI* ai) { return new MoveToRpgTargetAction(ai); };
			creators["travel"] = [](PlayerbotAI* ai) { return new TravelAction(ai); };
			creators["choose travel target"] = [](PlayerbotAI* ai) { return new ChooseTravelTargetAction(ai); };
            creators["choose group travel target"] = [](PlayerbotAI* ai) { return new ChooseGroupTravelTargetAction(ai); };
            creators["refresh travel target"] = [](PlayerbotAI* ai) { return new RefreshTravelTargetAction(ai); };
            creators["request travel target"] = [](PlayerbotAI* ai) { return new RequestTravelTargetAction(ai); };
            creators["request named travel target"] = [](PlayerbotAI* ai) { return new RequestNamedTravelTargetAction(ai); };
            creators["request quest travel target"] = [](PlayerbotAI* ai) { return new RequestQuestTravelTargetAction(ai); };
            creators["reset travel target"] = [](PlayerbotAI* ai) { return new ResetTargetAction(ai); };
            creators["move to travel target"] = [](PlayerbotAI* ai) { return new MoveToTravelTargetAction(ai); };
            creators["move out of collision"] = [](PlayerbotAI* ai) { return new MoveOutOfCollisionAction(ai); };
            creators["move random"] = [](PlayerbotAI* ai) { return new MoveRandomAction(ai); };
            creators["attack"] = [](PlayerbotAI* ai) { return new MeleeAction(ai); };
            creators["melee"] = [](PlayerbotAI* ai) { return new MeleeAction(ai); };
            creators["switch to melee"] = [](PlayerbotAI* ai) { return new SwitchToMeleeAction(ai); };
            creators["switch to ranged"] = [](PlayerbotAI* ai) { return new SwitchToRangedAction(ai); };
            creators["reach spell"] = [](PlayerbotAI* ai) { return new ReachSpellAction(ai); };
            creators["reach melee"] = [](PlayerbotAI* ai) { return new ReachMeleeAction(ai); };
            creators["reach pull"] = [](PlayerbotAI* ai) { return new ReachPullAction(ai); };
            creators["reach party member to heal"] = [](PlayerbotAI* ai) { return new ReachPartyMemberToHealAction(ai); };
            creators["flee"] = [](PlayerbotAI* ai) { return new FleeAction(ai); };
            creators["flee with pet"] = [](PlayerbotAI* ai) { return new FleeWithPetAction(ai); };
            creators["wait for attack keep safe distance"] = [](PlayerbotAI* ai) { return new WaitForAttackKeepSafeDistanceAction(ai); };
            creators["shoot"] = [](PlayerbotAI* ai) { return new CastShootAction(ai); };
            creators["whipper root tuber"] = [](PlayerbotAI* ai) { return new UseWhipperRootTuberAction(ai); };
            creators["healthstone"] = [](PlayerbotAI* ai) { return new UseHealthstoneAction(ai); };
            creators["healing potion"] = [](PlayerbotAI* ai) { return new UseHealingPotionAction(ai); };
            creators["mana potion"] = [](PlayerbotAI* ai) { return new UseManaPotionAction(ai); };
            creators["food"] = [](PlayerbotAI* ai) { return new EatAction(ai); };
            creators["drink"] = [](PlayerbotAI* ai) { return new DrinkAction(ai); };
            creators["tank assist"] = [](PlayerbotAI* ai) { return new TankAssistAction(ai); };
            creators["dps assist"] = [](PlayerbotAI* ai) { return new DpsAssistAction(ai); };
            creators["dps aoe"] = [](PlayerbotAI* ai) { return new DpsAoeAction(ai); };
            creators["attack rti target"] = [](PlayerbotAI* ai) { return new AttackRTITargetAction(ai); };
            creators["loot"] = [](PlayerbotAI* ai) { return new LootAction(ai); };
            creators["add loot"] = [](PlayerbotAI* ai) { return new AddLootAction(ai); };
            creators["add gathering loot"] = [](PlayerbotAI* ai) { return new AddGatheringLootAction(ai); };
            creators["add all loot"] = [](PlayerbotAI* ai) { return new AddAllLootAction(ai); };
            creators["release loot"] = [](PlayerbotAI* ai) { return new ReleaseLootAction(ai); };
            creators["auto loot roll"] = [](PlayerbotAI* ai) { return new AutoLootRollAction(ai); };
            creators["follow"] = [](PlayerbotAI* ai) { return new FollowAction(ai); };
            creators["stop follow"] = [](PlayerbotAI* ai) { return new StopFollowAction(ai); };
            creators["flee to master"] = [](PlayerbotAI* ai) { return new FleeToMasterAction(ai); };
            creators["runaway"] = [](PlayerbotAI* ai) { return new RunAwayAction(ai); };
            creators["stay"] = [](PlayerbotAI* ai) { return new StayAction(ai); };
            creators["sit"] = [](PlayerbotAI* ai) { return new SitAction(ai); };
            creators["attack anything"] = [](PlayerbotAI* ai) { return new AttackAnythingAction(ai); };
            creators["attack least hp target"] = [](PlayerbotAI* ai) { return new AttackLeastHpTargetAction(ai); };
            creators["attack enemy player"] = [](PlayerbotAI* ai) { return new AttackEnemyPlayerAction(ai); };
            creators["pull my target"] = [](PlayerbotAI* ai) { return new PullMyTargetAction(ai); };
            creators["pull rti target"] = [](PlayerbotAI* ai) { return new PullRTITargetAction(ai); };
            creators["pull start"] = [](PlayerbotAI* ai) { return new PullStartAction(ai); };
            creators["pull action"] = [](PlayerbotAI* ai) { return new PullAction(ai); };
            creators["return to pull position"] = [](PlayerbotAI* ai) { return new ReturnToPullPositionAction(ai); };
            creators["pull end"] = [](PlayerbotAI* ai) { return new PullEndAction(ai); };
            creators["emote"] = [](PlayerbotAI* ai) { return new EmoteAction(ai); };
            creators["talk"] = [](PlayerbotAI* ai) { return new TalkAction(ai); };
            creators["mount anim"] = [](PlayerbotAI* ai) { return new MountAnimAction(ai); };
            creators["suggest what to do"] = [](PlayerbotAI* ai) { return new SuggestWhatToDoAction(ai); };
            creators["suggest trade"] = [](PlayerbotAI* ai) { return new SuggestTradeAction(ai); };
            creators["return"] = [](PlayerbotAI* ai) { return new ReturnAction(ai); };
            creators["move to loot"] = [](PlayerbotAI* ai) { return new MoveToLootAction(ai); };
            creators["open loot"] = [](PlayerbotAI* ai) { return new OpenLootAction(ai); };
            creators["guard"] = [](PlayerbotAI* ai) { return new GuardAction(ai); };
            creators["return to stay position"] = [](PlayerbotAI* ai) { return new ReturnToStayPositionAction(ai); };
            creators["move out of enemy contact"] = [](PlayerbotAI* ai) { return new MoveOutOfEnemyContactAction(ai); };
            creators["set facing"] = [](PlayerbotAI* ai) { return new SetFacingTargetAction(ai); };
            creators["set behind"] = [](PlayerbotAI* ai) { return new SetBehindTargetAction(ai); };
            creators["attack duel opponent"] = [](PlayerbotAI* ai) { return new AttackDuelOpponentAction(ai); };
            creators["select new target"] = [](PlayerbotAI* ai) { return new SelectNewTargetAction(ai); };
            creators["check mail"] = [](PlayerbotAI* ai) { return new CheckMailAction(ai); };
            creators["say"] = [](PlayerbotAI* ai) { return new SayAction(ai); };
            creators["reveal gathering item"] = [](PlayerbotAI* ai) { return new RevealGatheringItemAction(ai); };
            creators["outfit"] = [](PlayerbotAI* ai) { return new OutfitAction(ai); };
            creators["random bot update"] = [](PlayerbotAI* ai) { return new RandomBotUpdateAction(ai); };
            creators["delay"] = [](PlayerbotAI* ai) { return new DelayAction(ai); };
            creators["greet"] = [](PlayerbotAI* ai) { return new GreetAction(ai); };
            creators["check values"] = [](PlayerbotAI* ai) { return new CheckValuesAction(ai); };
            creators["set avoid area"] = [](PlayerbotAI* ai) { return new SetAvoidAreaAction(ai); };
            creators["ra"] = [](PlayerbotAI* ai) { return new RemoveAuraAction(ai); };
            creators["remove blessing of salvation"] = [](PlayerbotAI* ai) { return new RemoveBlessingOfSalvationAction(ai); };
            creators["remove greater blessing of salvation"] = [](PlayerbotAI* ai) { return new RemoveGreaterBlessingOfSalvationAction(ai); };
            creators["apply stone"] = [](PlayerbotAI* ai) { return new ImbueWithStoneAction(ai); };
            creators["apply oil"] = [](PlayerbotAI* ai) { return new ImbueWithOilAction(ai); };
            creators["try emergency"] = [](PlayerbotAI* ai) { return new TryEmergencyAction(ai); };
            creators["give food"] = [](PlayerbotAI* ai) { return new GiveFoodAction(ai); };
            creators["give water"] = [](PlayerbotAI* ai) { return new GiveWaterAction(ai); };
            creators["mount"] = [](PlayerbotAI* ai) { return new CastSpellAction(ai, "mount"); };
            creators["auto talents"] = [](PlayerbotAI* ai) { return new AutoSetTalentsAction(ai); };
			creators["auto learn spell"] = [](PlayerbotAI* ai) { return new AutoLearnSpellAction(ai); };
            creators["auto share quest"] = [](PlayerbotAI* ai) { return new AutoShareQuestAction(ai); };
            creators["xp gain"] = [](PlayerbotAI* ai) { return new XpGainAction(ai); };
            creators["honor gain"] = [](PlayerbotAI* ai) { return new HonorGainAction(ai); };
            creators["invite nearby"] = [](PlayerbotAI* ai) { return new InviteNearbyToGroupAction(ai); };
            creators["invite guild"] = [](PlayerbotAI* ai) { return new InviteGuildToGroupAction(ai); };
            creators["leave far away"] = [](PlayerbotAI* ai) { return new LeaveFarAwayAction(ai); };
            creators["move to dark portal"] = [](PlayerbotAI* ai) { return new MoveToDarkPortalAction(ai); };
            creators["move from dark portal"] = [](PlayerbotAI* ai) { return new MoveFromDarkPortalAction(ai); };
            creators["use dark portal azeroth"] = [](PlayerbotAI* ai) { return new DarkPortalAzerothAction(ai); };
            creators["world buff"] = [](PlayerbotAI* ai) { return new WorldBuffAction(ai); };
            creators["hearthstone"] = [](PlayerbotAI* ai) { return new UseHearthStoneAction(ai); };
            creators["cast random spell"] = [](PlayerbotAI* ai) { return new CastRandomSpellAction(ai); };
            creators["free bg join"] = [](PlayerbotAI* ai) { return new FreeBGJoinAction(ai); };
            creators["use random recipe"] = [](PlayerbotAI* ai) { return new UseRandomRecipeAction(ai); };
            creators["open random item"] = [](PlayerbotAI* ai) { return new OpenRandomItemAction(ai); };
            creators["use random quest item"] = [](PlayerbotAI* ai) { return new UseRandomQuestItemAction(ai); };
            creators["craft random item"] = [](PlayerbotAI* ai) { return new CraftRandomItemAction(ai); };
            creators["smart destroy item"] = [](PlayerbotAI* ai) { return new SmartDestroyItemAction(ai); };
            creators["disenchant random item"] = [](PlayerbotAI* ai) { return new DisenchantRandomItemAction(ai); };
            creators["enchant random item"] = [](PlayerbotAI* ai) { return new EnchantRandomItemAction(ai); };
            creators["reset instances"] = [](PlayerbotAI* ai) { return new ResetInstancesAction(ai); };
            creators["reset raids"] = [](PlayerbotAI* ai) { return new ResetRaidsAction(ai); };
            creators["update gear"] = [](PlayerbotAI* ai) { return new UpdateGearAction(ai); };
            creators["buy petition"] = [](PlayerbotAI* ai) { return new BuyPetitionAction(ai); };
            creators["offer petition"] = [](PlayerbotAI* ai) { return new PetitionOfferAction(ai); };
            creators["offer petition nearby"] = [](PlayerbotAI* ai) { return new PetitionOfferNearbyAction(ai); };
            creators["turn in petition"] = [](PlayerbotAI* ai) { return new PetitionTurnInAction(ai); };
            creators["buy tabard"] = [](PlayerbotAI* ai) { return new BuyTabardAction(ai); };
            creators["guild manage nearby"] = [](PlayerbotAI* ai) { return new GuildManageNearbyAction(ai); };
            creators["use trinket"] = [](PlayerbotAI* ai) { return new UseTrinketAction(ai); };
            creators["unstuck"] = [](PlayerbotAI* ai) { return new UnstuckAction(ai); };
            creators["reset"] = [](PlayerbotAI* ai) { return new ResetAction(ai); };
            creators["interrupt current spell"] = [](PlayerbotAI* ai) { return new InterruptCurrentSpellAction(ai); };
            creators["initialize pet"] = [](PlayerbotAI* ai) { return new InitializePetAction(ai); };

            // item helpers
            creators["goblin sapper"] = [](PlayerbotAI* ai) { return new UseGoblinSapperChargeAction(ai); };
            creators["oil of immolation"] = [](PlayerbotAI* ai) { return new UseOilOfImmolationAction(ai); };
            creators["stoneshield potion"] = [](PlayerbotAI* ai) { return new UseStoneshieldPotionAction(ai); };
            creators["dark rune"] = [](PlayerbotAI* ai) { return new UseDarkRuneAction(ai); };
            creators["throw grenade"] = [](PlayerbotAI* ai) { return new ThrowGrenadeAction(ai); };
            creators["bg banner"] = [](PlayerbotAI* ai) { return new UseBgBannerAction(ai); };
            creators["use bandage"] = [](PlayerbotAI* ai) { return new UseBandageAction(ai); };
            creators["rocket boots"] = [](PlayerbotAI* ai) { return new UseRocketBootsAction(ai); };
            creators["fire protection potion"] = [](PlayerbotAI* ai) { return new UseFireProtectionPotionAction(ai); };
            creators["free action potion"] = [](PlayerbotAI* ai) { return new UseFreeActionPotionAction(ai); };

            // BG Tactics
            creators["bg tactics"] = [](PlayerbotAI* ai) { return new BGTactics(ai); };
            creators["bg move to start"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "move to start"); };
            creators["bg move to objective"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "move to objective"); };
            creators["bg select objective"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "select objective"); };
            creators["bg check objective"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "check objective"); };
            creators["bg attack fc"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "attack fc"); };
            creators["bg protect fc"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "protect fc"); };
            creators["bg use buff"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "use buff"); };
            creators["attack enemy flag carrier"] = [](PlayerbotAI* ai) { return new AttackEnemyFlagCarrierAction(ai); };
            creators["bg check flag"] = [](PlayerbotAI* ai) { return new BGTactics(ai, "check flag"); };

            // lightwell
            creators["use lightwell"] = [](PlayerbotAI* ai) { return new UseLightwellAction(ai); };

            // Vehicles
            creators["enter vehicle"] = [](PlayerbotAI* ai) { return new EnterVehicleAction(ai); };
            creators["leave vehicle"] = [](PlayerbotAI* ai) { return new LeaveVehicleAction(ai); };
            creators["hurl boulder"] = [](PlayerbotAI* ai) { return new CastHurlBoulderAction(ai); };
            creators["ram"] = [](PlayerbotAI* ai) { return new CastRamAction(ai); };
            creators["steam rush"] = [](PlayerbotAI* ai) { return new CastSteamRushAction(ai); };
            creators["steam blast"] = [](PlayerbotAI* ai) { return new CastSteamBlastAction(ai); };
            creators["napalm"] = [](PlayerbotAI* ai) { return new CastNapalmAction(ai); };
            creators["fire cannon"] = [](PlayerbotAI* ai) { return new CastFireCannonAction(ai); };
            creators["incendiary rocket"] = [](PlayerbotAI* ai) { return new CastIncendiaryRocketAction(ai); };
            creators["rocket blast"] = [](PlayerbotAI* ai) { return new CastRocketBlastAction(ai); };
            creators["blade salvo"] = [](PlayerbotAI* ai) { return new CastBladeSalvoAction(ai); };
            creators["glaive throw"] = [](PlayerbotAI* ai) { return new CastGlaiveThrowAction(ai); };

            // Quest vehicles
            creators["deliver stolen horse"] = [](PlayerbotAI* ai) { return new CastDeliverStolenHorseAction(ai); };
            creators["horsemans call"] = [](PlayerbotAI* ai) { return new CastHorsemansCallAction(ai); };

            creators["scarlet cannon"] = [](PlayerbotAI* ai) { return new CastScarletCannonAction(ai); };
            creators["electro - magnetic pulse"] = [](PlayerbotAI* ai) { return new CastElectroMagneticPulseAction(ai); };            
            creators["skeletal gryphon escape"] = [](PlayerbotAI* ai) { return new CastSkeletalGryphonEscapeAction(ai); };
            
            creators["frozen deathbolt"] = [](PlayerbotAI* ai) { return new CastFrozenDeathboltAction(ai); };
            creators["devour humanoid"] = [](PlayerbotAI* ai) { return new CastDevourHumanoidAction(ai); };            
            

            //Rpg
            creators["rpg stay"] = [](PlayerbotAI* ai) { return new RpgStayAction(ai); };
            creators["rpg work"] = [](PlayerbotAI* ai) { return new RpgWorkAction(ai); };
            creators["rpg emote"] = [](PlayerbotAI* ai) { return new RpgEmoteAction(ai); };
            creators["rpg cancel"] = [](PlayerbotAI* ai) { return new RpgCancelAction(ai); };
            creators["rpg taxi"] = [](PlayerbotAI* ai) { return new RpgTaxiAction(ai); };
            creators["rpg discover"] = [](PlayerbotAI* ai) { return new RpgDiscoverAction(ai); };
            creators["rpg start quest"] = [](PlayerbotAI* ai) { return new RpgStartQuestAction(ai); };
            creators["rpg end quest"] = [](PlayerbotAI* ai) { return new RpgEndQuestAction(ai); };
            creators["rpg buy"] = [](PlayerbotAI* ai) { return new RpgBuyAction(ai); };
            creators["rpg sell"] = [](PlayerbotAI* ai) { return new RpgSellAction(ai); };
            creators["rpg ah sell"] = [](PlayerbotAI* ai) { return new RpgAHSellAction(ai); };
            creators["rpg ah buy"] = [](PlayerbotAI* ai) { return new RpgAHBuyAction(ai); };
            creators["rpg get mail"] = [](PlayerbotAI* ai) { return new RpgGetMailAction(ai); };
            creators["rpg repair"] = [](PlayerbotAI* ai) { return new RpgRepairAction(ai); };
            creators["rpg train"] = [](PlayerbotAI* ai) { return new RpgTrainAction(ai); };
            creators["rpg heal"] = [](PlayerbotAI* ai) { return new RpgHealAction(ai); };
            creators["rpg home bind"] = [](PlayerbotAI* ai) { return new RpgHomeBindAction(ai); };
            creators["rpg queue bg"] = [](PlayerbotAI* ai) { return new RpgQueueBgAction(ai); };
            creators["rpg buy petition"] = [](PlayerbotAI* ai) { return new RpgBuyPetitionAction(ai); };
            creators["rpg use"] = [](PlayerbotAI* ai) { return new RpgUseAction(ai); };
            creators["rpg ai chat"] = [](PlayerbotAI* ai) { return new RpgAIChatAction(ai); };
            creators["rpg spell"] = [](PlayerbotAI* ai) { return new RpgSpellAction(ai); };
            creators["rpg spell click"] = [](PlayerbotAI* ai) { return new RpgSpellClickAction(ai); };
            creators["rpg craft"] = [](PlayerbotAI* ai) { return new RpgCraftAction(ai); };
            creators["rpg trade useful"] = [](PlayerbotAI* ai) { return new RpgTradeUsefulAction(ai); };
            creators["rpg enchant"] = [](PlayerbotAI* ai) { return new RpgEnchantAction(ai); };
            creators["rpg duel"] = [](PlayerbotAI* ai) { return new RpgDuelAction(ai); };
            creators["rpg item"] = [](PlayerbotAI* ai) { return new RpgItemAction(ai); };
            creators["rpg gossip talk"] = [](PlayerbotAI* ai) { return new RpgGossipTalkAction(ai); };

            creators["auto set glyph"] = [](PlayerbotAI* ai) { return new AutoSetGlyphAction(ai); };
            creators["auto complete quest"] = [](PlayerbotAI* ai) { return new AutoCompleteQuestAction(ai); };

            creators["move to fish"] = [](PlayerbotAI* ai) { return new MoveToFishAction(ai); };
            creators["fish"] = [](PlayerbotAI* ai) { return new FishAction(ai); };
            creators["use fishing bobber"] = [](PlayerbotAI* ai) { return new UseFishingBobberAction(ai); };

            // Bot States
            creators["set combat state"] = [](PlayerbotAI* ai) { return new SetCombatStateAction(ai); };
            creators["set non combat state"] = [](PlayerbotAI* ai) { return new SetNonCombatStateAction(ai); };
            creators["set dead state"] = [](PlayerbotAI* ai) { return new SetDeadStateAction(ai); };

            //racials
            creators["war stomp"] = [](PlayerbotAI* ai) { return new CastWarStompAction(ai); };
            creators["berserking"] = [](PlayerbotAI* ai) { return new CastBerserkingAction(ai); };
            creators["blood fury"] = [](PlayerbotAI* ai) { return new CastBloodFuryAction(ai); };
            creators["cannibalize"] = [](PlayerbotAI* ai) { return new CastCannibalizeAction(ai); };
            creators["escape artist"] = [](PlayerbotAI* ai) { return new CastEscapeArtistAction(ai); };
            creators["shadowmeld"] = [](PlayerbotAI* ai) { return new CastShadowmeldAction(ai); };
            creators["stoneform"] = [](PlayerbotAI* ai) { return new CastStoneformAction(ai); };
            creators["perception"] = [](PlayerbotAI* ai) { return new CastPerceptionAction(ai); };
            creators["will of the forsaken"] = [](PlayerbotAI* ai) { return new CastWillOfTheForsakenAction(ai); };
#ifndef MANGOSBOT_ZERO
            creators["mana tap"] = [](PlayerbotAI* ai) { return new CastManaTapAction(ai); };
            creators["arcane torrent"] = [](PlayerbotAI* ai) { return new CastArcaneTorrentAction(ai); };
            creators["gift of the naaru"] = [](PlayerbotAI* ai) { return new CastGiftOfTheNaaruAction(ai); };
#endif
#ifdef MANGOSBOT_TWO
            creators["every_man_for_himself"] = [](PlayerbotAI* ai) { return new CastEveryManforHimselfAction(ai); };
#endif

            creators["use id"] = [](PlayerbotAI* ai) { return new UseItemIdAction(ai); };
            creators["move to"] = [](PlayerbotAI* ai) { return new MoveToAction(ai); };

            // Dungeon Actions
            creators["enable onyxia's lair strategy"] = [](PlayerbotAI* ai) { return new OnyxiasLairEnableDungeonStrategyAction(ai); };
            creators["disable onyxia's lair strategy"] = [](PlayerbotAI* ai) { return new OnyxiasLairDisableDungeonStrategyAction(ai); };
            creators["enable molten core strategy"] = [](PlayerbotAI* ai) { return new MoltenCoreEnableDungeonStrategyAction(ai); };
            creators["disable molten core strategy"] = [](PlayerbotAI* ai) { return new MoltenCoreDisableDungeonStrategyAction(ai); };
            creators["enable karazhan strategy"] = [](PlayerbotAI* ai) { return new KarazhanEnableDungeonStrategyAction(ai); };
            creators["disable karazhan strategy"] = [](PlayerbotAI* ai) { return new KarazhanDisableDungeonStrategyAction(ai); };
            creators["enable naxxramas strategy"] = [](PlayerbotAI* ai) { return new NaxxramasEnableDungeonStrategyAction(ai); };
            creators["disable naxxramas strategy"] = [](PlayerbotAI* ai) { return new NaxxramasDisableDungeonStrategyAction(ai); };

            // Dungeon Boss Actions
            creators["enable onyxia fight strategy"] = [](PlayerbotAI* ai) { return new OnyxiaEnableFightStrategyAction(ai); };
            creators["disable onyxia fight strategy"] = [](PlayerbotAI* ai) { return new OnyxiaDisableFightStrategyAction(ai); };

            creators["enable magmadar fight strategy"] = [](PlayerbotAI* ai) { return new MagmadarEnableFightStrategyAction(ai); };
            creators["disable magmadar fight strategy"] = [](PlayerbotAI* ai) { return new MagmadarDisableFightStrategyAction(ai); };
            creators["move away from magmadar"] = [](PlayerbotAI* ai) { return new MagmadarMoveAwayAction(ai); };

            creators["move away from hazard"] = [](PlayerbotAI* ai) { return new MoveAwayFromHazard(ai); };
            creators["move to mc rune"] = [](PlayerbotAI* ai) { return new MoveToMCRuneAction(ai); };
            creators["douse mc rune aqual"] = [](PlayerbotAI* ai) { return new DouseMCRuneActionAqual(ai); };
            creators["douse mc rune eternal"] = [](PlayerbotAI* ai) { return new DouseMCRuneActionEternal(ai); };

            creators["enable netherspite fight strategy"] = [](PlayerbotAI* ai) { return new NetherspiteEnableFightStrategyAction(ai); };
            creators["disable netherspite fight strategy"] = [](PlayerbotAI* ai) { return new NetherspiteDisableFightStrategyAction(ai); };
            creators["move away from void zone"] = [](PlayerbotAI* ai) { return new VoidZoneMoveAwayAction(ai); };
            creators["add nether portal - perseverence for tank"] = [](PlayerbotAI* ai) { return new AddNetherPortalPerseverenceForTankAction(ai); };
            creators["remove nether portal buffs from netherspite"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalBuffsFromNetherspiteAction(ai); };
            creators["remove nether portal - perseverence"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalPerseverenceAction(ai); };
            creators["remove nether portal - serenity"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalSerenityAction(ai); };
            creators["remove nether portal - dominance"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalDominanceAction(ai); };

            creators["enable prince malchezaar fight strategy"] = [](PlayerbotAI* ai) { return new PrinceMalchezaarEnableFightStrategyAction(ai); };
            creators["disable prince malchezaar fight strategy"] = [](PlayerbotAI* ai) { return new PrinceMalchezaarDisableFightStrategyAction(ai); };
            creators["move away from netherspite infernal"] = [](PlayerbotAI* ai) { return new NetherspiteInfernalMoveAwayAction(ai); };

            creators["enable four horseman fight strategy"] = [](PlayerbotAI* ai) { return new FourHorsemanEnableFightStrategyAction(ai); };
            creators["disable four horseman fight strategy"] = [](PlayerbotAI* ai) { return new FourHorsemanDisableFightStrategyAction(ai); };
        }    
    };
};
