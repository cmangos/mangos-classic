#pragma once

#include "HealthTriggers.h"
#include "GenericTriggers.h"
#include "LootTriggers.h"
#include "GenericTriggers.h"
#include "LfgTriggers.h"
#include "PvpTriggers.h"
#include "RpgTriggers.h"
#include "TravelTriggers.h"
#include "RtiTriggers.h"
#include "CureTriggers.h"
#include "GuildTriggers.h"
#include "StuckTriggers.h"
#include "BotStateTriggers.h"
#include "PullTriggers.h"
#include "OnyxiasLairDungeonTriggers.h"
#include "MoltenCoreDungeonTriggers.h"
#include "KarazhanDungeonTriggers.h"
#include "NaxxramasDungeonTriggers.h"
#include "GlyphTriggers.h"

namespace ai
{
    class TriggerContext : public NamedObjectContext<Trigger>
    {
    public:
        TriggerContext()
        {
            creators["return"] = [](PlayerbotAI* ai) { return new ReturnTrigger(ai); };
            creators["sit"] = [](PlayerbotAI* ai) { return new SitTrigger(ai); };
            creators["return to stay position"] = [](PlayerbotAI* ai) { return new ReturnToStayPositionTrigger(ai); };
            creators["return to pull position"] = [](PlayerbotAI* ai) { return new ReturnToPullPositionTrigger(ai); };
            creators["collision"] = [](PlayerbotAI* ai) { return new CollisionTrigger(ai); };

            creators["timer"] = [](PlayerbotAI* ai) { return new TimerTrigger(ai); };
            creators["random"] = [](PlayerbotAI* ai) { return new RandomTrigger(ai, "random", 20); };
            creators["seldom"] = [](PlayerbotAI* ai) { return new RandomTrigger(ai, "seldom", 300); };
            creators["often"] = [](PlayerbotAI* ai) { return new RandomTrigger(ai, "often", 5); };
            creators["very often"] = [](PlayerbotAI* ai) { return new TimeTrigger(ai, "very often", 5); };

            creators["target critical health"] = [](PlayerbotAI* ai) { return new TargetCriticalHealthTrigger(ai); };

            creators["critical health"] = [](PlayerbotAI* ai) { return new CriticalHealthTrigger(ai); };
            creators["low health"] = [](PlayerbotAI* ai) { return new LowHealthTrigger(ai); };
            creators["medium health"] = [](PlayerbotAI* ai) { return new MediumHealthTrigger(ai); };
            creators["almost full health"] = [](PlayerbotAI* ai) { return new AlmostFullHealthTrigger(ai); };

            creators["no mana"] = [](PlayerbotAI* ai) { return new NoManaTrigger(ai); };
            creators["low mana"] = [](PlayerbotAI* ai) { return new LowManaTrigger(ai); };
            creators["medium mana"] = [](PlayerbotAI* ai) { return new MediumManaTrigger(ai); };
            creators["high mana"] = [](PlayerbotAI* ai) { return new HighManaTrigger(ai); };
            creators["almost full mana"] = [](PlayerbotAI* ai) { return new AlmostFullManaTrigger(ai); };

            creators["party member critical health"] = [](PlayerbotAI* ai) { return new PartyMemberCriticalHealthTrigger(ai); };
            creators["party member low health"] = [](PlayerbotAI* ai) { return new PartyMemberLowHealthTrigger(ai); };
            creators["party member medium health"] = [](PlayerbotAI* ai) { return new PartyMemberMediumHealthTrigger(ai); };
            creators["party member almost full health"] = [](PlayerbotAI* ai) { return new PartyMemberAlmostFullHealthTrigger(ai); };

            creators["protect party member"] = [](PlayerbotAI* ai) { return new ProtectPartyMemberTrigger(ai); };

            creators["light rage available"] = [](PlayerbotAI* ai) { return new LightRageAvailableTrigger(ai); };
            creators["medium rage available"] = [](PlayerbotAI* ai) { return new MediumRageAvailableTrigger(ai); };
            creators["high rage available"] = [](PlayerbotAI* ai) { return new HighRageAvailableTrigger(ai); };

            creators["no energy available"] = [](PlayerbotAI* ai) { return new NoEnergyAvailableTrigger(ai); };
            creators["light energy available"] = [](PlayerbotAI* ai) { return new LightEnergyAvailableTrigger(ai); };
            creators["medium energy available"] = [](PlayerbotAI* ai) { return new MediumEnergyAvailableTrigger(ai); };
            creators["high energy available"] = [](PlayerbotAI* ai) { return new HighEnergyAvailableTrigger(ai); };

            creators["loot available"] = [](PlayerbotAI* ai) { return new LootAvailableTrigger(ai); };
            creators["no attackers"] = [](PlayerbotAI* ai) { return new NoAttackersTrigger(ai); };
            creators["no target"] = [](PlayerbotAI* ai) { return new NoTargetTrigger(ai); };
            creators["target in sight"] = [](PlayerbotAI* ai) { return new TargetInSightTrigger(ai); };
            creators["not dps target active"] = [](PlayerbotAI* ai) { return new NotDpsTargetActiveTrigger(ai); };
            creators["not dps aoe target active"] = [](PlayerbotAI* ai) { return new NotDpsAoeTargetActiveTrigger(ai); };
            creators["has nearest adds"] = [](PlayerbotAI* ai) { return new HasNearestAddsTrigger(ai); };
            creators["enemy player near"] = [](PlayerbotAI* ai) { return new EnemyPlayerNear(ai); };

            creators["combat start"] = [](PlayerbotAI* ai) { return new CombatStartTrigger(ai); };
            creators["combat end"] = [](PlayerbotAI* ai) { return new CombatEndTrigger(ai); };
            creators["death"] = [](PlayerbotAI* ai) { return new DeathTrigger(ai); };
            creators["resurrect"] = [](PlayerbotAI* ai) { return new ResurrectTrigger(ai); };

            creators["pull start"] = [](PlayerbotAI* ai) { return new PullStartTrigger(ai); };
            creators["pull end"] = [](PlayerbotAI* ai) { return new PullEndTrigger(ai); };

            creators["tank assist"] = [](PlayerbotAI* ai) { return new TankAssistTrigger(ai); };
            creators["lose aggro"] = [](PlayerbotAI* ai) { return new LoseAggroTrigger(ai); };
            creators["has aggro"] = [](PlayerbotAI* ai) { return new HasAggroTrigger(ai); };

            creators["ranged light aoe"] = [](PlayerbotAI* ai) { return new RangedLightAoeTrigger(ai); };
            creators["ranged medium aoe"] = [](PlayerbotAI* ai) { return new RangedMediumAoeTrigger(ai); };
            creators["ranged high aoe"] = [](PlayerbotAI* ai) { return new RangedHighAoeTrigger(ai); };
            creators["ranged very high aoe"] = [](PlayerbotAI* ai) { return new RangedVeryHighAoeTrigger(ai); };
            creators["melee light aoe"] = [](PlayerbotAI* ai) { return new RangedLightAoeTrigger(ai); };
            creators["melee medium aoe"] = [](PlayerbotAI* ai) { return new RangedMediumAoeTrigger(ai); };
            creators["melee high aoe"] = [](PlayerbotAI* ai) { return new RangedHighAoeTrigger(ai); };
            creators["melee very high aoe"] = [](PlayerbotAI* ai) { return new RangedVeryHighAoeTrigger(ai); };

            creators["has area debuff"] = [](PlayerbotAI* ai) { return new HasAreaDebuffTrigger(ai); };
            creators["has aura"] = [](PlayerbotAI* ai) { return new HasAuraTrigger(ai); };

            creators["enemy out of melee"] = [](PlayerbotAI* ai) { return new EnemyOutOfMeleeTrigger(ai); };
            creators["enemy out of spell"] = [](PlayerbotAI* ai) { return new EnemyOutOfSpellRangeTrigger(ai); };
            creators["enemy too close for spell"] = [](PlayerbotAI* ai) { return new EnemyTooCloseForSpellTrigger(ai); };
            creators["enemy too close for shoot"] = [](PlayerbotAI* ai) { return new EnemyTooCloseForShootTrigger(ai); };
            creators["enemy too close for melee"] = [](PlayerbotAI* ai) { return new EnemyTooCloseForMeleeTrigger(ai); };
            creators["enemy is close"] = [](PlayerbotAI* ai) { return new EnemyIsCloseTrigger(ai); };
            creators["party member to heal out of spell range"] = [](PlayerbotAI* ai) { return new PartyMemberToHealOutOfSpellRangeTrigger(ai); };
            creators["enemy fifteen yards"] = [](PlayerbotAI* ai) { return new EnemyInRangeTrigger(ai, "enemy fifteen yards", 15.0f); };
            creators["enemy ten yards"] = [](PlayerbotAI* ai) { return new EnemyInRangeTrigger(ai, "enemy ten yards", 10.0f); };
            creators["enemy five yards"] = [](PlayerbotAI* ai) { return new EnemyInRangeTrigger(ai, "enemy five yards", 5.0f); };
            creators["wait for attack safe distance"] = [](PlayerbotAI* ai) { return new WaitForAttackSafeDistanceTrigger(ai); };
            creators["enemy player ten yards"] = [](PlayerbotAI* ai) { return new EnemyInRangeTrigger(ai, "enemy player ten yards", 10.0f, true); };
            creators["combo points available"] = [](PlayerbotAI* ai) { return new ComboPointsAvailableTrigger(ai); };
            creators["multiple attackers"] = [](PlayerbotAI* ai) { return new MultipleAttackersTrigger(ai); };
            creators["high threat"] = [](PlayerbotAI* ai) { return new HighThreatTrigger(ai); };
            creators["medium threat"] = [](PlayerbotAI* ai) { return new MediumThreatTrigger(ai); };
            creators["some threat"] = [](PlayerbotAI* ai) { return new SomeThreatTrigger(ai); };
            creators["no threat"] = [](PlayerbotAI* ai) { return new NoThreatTrigger(ai); };
            creators["dead"] = [](PlayerbotAI* ai) { return new DeadTrigger(ai); };
            creators["corpse near"] = [](PlayerbotAI* ai) { return new CorpseNearTrigger(ai); };
            creators["party member dead"] = [](PlayerbotAI* ai) { return new PartyMemberDeadTrigger(ai); };
            creators["no pet"] = [](PlayerbotAI* ai) { return new NoPetTrigger(ai); };
            creators["has attackers"] = [](PlayerbotAI* ai) { return new HasAttackersTrigger(ai); };
            creators["no possible targets"] = [](PlayerbotAI* ai) { return new NoPossibleTargetsTrigger(ai); };
            creators["possible adds"] = [](PlayerbotAI* ai) { return new PossibleAddsTrigger(ai); };
            creators["panic"] = [](PlayerbotAI* ai) { return new PanicTrigger(ai); };
            creators["outnumbered"] = [](PlayerbotAI* ai) { return new OutNumberedTrigger(ai); };
            creators["behind target"] = [](PlayerbotAI* ai) { return new IsBehindTargetTrigger(ai); };
            creators["not behind target"] = [](PlayerbotAI* ai) { return new IsNotBehindTargetTrigger(ai); };
            creators["not facing target"] = [](PlayerbotAI* ai) { return new IsNotFacingTargetTrigger(ai); };
            creators["far from master"] = [](PlayerbotAI* ai) { return new FarFromMasterTrigger(ai); };
            creators["not near master"] = [](PlayerbotAI* ai) { return new NotNearMasterTrigger(ai); };
            creators["out of react range"] = [](PlayerbotAI* ai) { return new OutOfReactRangeTrigger(ai); };
            creators["update follow"] = [](PlayerbotAI* ai) { return new UpdateFollowTrigger(ai); };
            creators["stop follow"] = [](PlayerbotAI* ai) { return new StopFollowTrigger(ai); };
            creators["far from loot target"] = [](PlayerbotAI* ai) { return new FarFromCurrentLootTrigger(ai); };
            creators["can loot"] = [](PlayerbotAI* ai) { return new CanLootTrigger(ai); };
            creators["swimming"] = [](PlayerbotAI* ai) { return new IsSwimmingTrigger(ai); };
            creators["target changed"] = [](PlayerbotAI* ai) { return new TargetChangedTrigger(ai); };

            creators["critical aoe heal"] = [](PlayerbotAI* ai) { return new AoeHealTrigger(ai, "critical aoe heal", "critical", 2); };
            creators["low aoe heal"] = [](PlayerbotAI* ai) { return new AoeHealTrigger(ai, "low aoe heal", "low", 2); };
            creators["medium aoe heal"] = [](PlayerbotAI* ai) { return new AoeHealTrigger(ai, "medium aoe heal", "medium", 2); };
            creators["invalid target"] = [](PlayerbotAI* ai) { return new InvalidTargetTrigger(ai); };
            creators["lfg proposal active"] = [](PlayerbotAI* ai) { return new LfgProposalActiveTrigger(ai); };

            creators["unknown dungeon"] = [](PlayerbotAI* ai) { return new UnknownDungeonTrigger(ai); };

            creators["random bot update"] = [](PlayerbotAI* ai) { return new RandomBotUpdateTrigger(ai); };
            creators["no non bot players around"] = [](PlayerbotAI* ai) { return new NoNonBotPlayersAroundTrigger(ai); };
            creators["new player nearby"] = [](PlayerbotAI* ai) { return new NewPlayerNearbyTrigger(ai); };
            creators["no rpg target"] = [](PlayerbotAI* ai) { return new NoRpgTargetTrigger(ai); };
            creators["has rpg target"] = [](PlayerbotAI* ai) { return new HasRpgTargetTrigger(ai); };
            creators["far from rpg target"] = [](PlayerbotAI* ai) { return new FarFromRpgTargetTrigger(ai); };
            creators["near rpg target"] = [](PlayerbotAI* ai) { return new NearRpgTargetTrigger(ai); };
            creators["no rti target"] = [](PlayerbotAI* ai) { return new NoRtiTrigger(ai); };

            creators["give food"] = [](PlayerbotAI* ai) { return new GiveFoodTrigger(ai); };
            creators["give water"] = [](PlayerbotAI* ai) { return new GiveWaterTrigger(ai); };

            creators["bg waiting"] = [](PlayerbotAI* ai) { return new BgWaitingTrigger(ai); };
            creators["bg active"] = [](PlayerbotAI* ai) { return new BgActiveTrigger(ai); };
            creators["bg ended"] = [](PlayerbotAI* ai) { return new BgEndedTrigger(ai); };
            creators["bg invite active"] = [](PlayerbotAI* ai) { return new BgInviteActiveTrigger(ai); };
            creators["player has no flag"] = [](PlayerbotAI* ai) { return new PlayerHasNoFlag(ai); };
            creators["player has flag"] = [](PlayerbotAI* ai) { return new PlayerHasFlag(ai); };
            creators["team has flag"] = [](PlayerbotAI* ai) { return new TeamHasFlag(ai); };
            creators["enemy team has flag"] = [](PlayerbotAI* ai) { return new EnemyTeamHasFlag(ai); };
            creators["enemy flagcarrier near"] = [](PlayerbotAI* ai) { return new EnemyFlagCarrierNear(ai); };
            creators["in battleground"] = [](PlayerbotAI* ai) { return new PlayerIsInBattleground(ai); };
            creators["in battleground without flag"] = [](PlayerbotAI* ai) { return new PlayerIsInBattlegroundWithoutFlag(ai); };
            creators["wants in bg"] = [](PlayerbotAI* ai) { return new PlayerWantsInBattlegroundTrigger(ai); };
            creators["use trinket"] = [](PlayerbotAI* ai) { return new UseTrinketTrigger(ai); };
            creators["has blessing of salvation"] = [](PlayerbotAI* ai) { return new HasBlessingOfSalvationTrigger(ai); };
            creators["has greater blessing of salvation"] = [](PlayerbotAI* ai) { return new HasGreaterBlessingOfSalvationTrigger(ai); };
            creators["target of attacker"] = [](PlayerbotAI* ai) { return new TargetOfAttacker(ai); };
            creators["target of attacker close"] = [](PlayerbotAI* ai) { return new TargetOfAttackerClose(ai); };
            creators["target of fear cast"] = [](PlayerbotAI* ai) { return new TargetOfFearCastTrigger(ai); };
            creators["heal target full health"] = [](PlayerbotAI* ai) { return new HealTargetFullHealthTrigger(ai); };
            creators["dispel enrage"] = [](PlayerbotAI* ai) { return new DispelEnrageOnTargetTrigger(ai); };
            creators["at war"] = [](PlayerbotAI* ai) { return new AtWarTrigger(ai); };

            creators["mounted"] = [](PlayerbotAI* ai) { return new IsMountedTrigger(ai); };
            creators["rooted"] = [](PlayerbotAI* ai) { return new RootedTrigger(ai); };
            creators["party member rooted"] = [](PlayerbotAI* ai) { return new PartyMemberRootedTrigger(ai); };
            creators["feared"] = [](PlayerbotAI* ai) { return new FearedTrigger(ai); };
            creators["charmed"] = [](PlayerbotAI* ai) { return new CharmedTrigger(ai); };
            creators["stunned"] = [](PlayerbotAI* ai) { return new StunnedTrigger(ai); };

            // move to/enter dark portal if near
            creators["near dark portal"] = [](PlayerbotAI* ai) { return new NearDarkPortalTrigger(ai); };
            creators["at dark portal azeroth"] = [](PlayerbotAI* ai) { return new AtDarkPortalAzerothTrigger(ai); };
            creators["at dark portal outland"] = [](PlayerbotAI* ai) { return new AtDarkPortalOutlandTrigger(ai); };

            creators["vehicle near"] = [](PlayerbotAI* ai) { return new VehicleNearTrigger(ai); };
            creators["in vehicle"] = [](PlayerbotAI* ai) { return new InVehicleTrigger(ai); };

            creators["need world buff"] = [](PlayerbotAI* ai) { return new NeedWorldBuffTrigger(ai); };
            creators["falling"] = [](PlayerbotAI* ai) { return new IsFallingTrigger(ai); };
            creators["falling far"] = [](PlayerbotAI* ai) { return new IsFallingFarTrigger(ai); };
            creators["move stuck"] = [](PlayerbotAI* ai) { return new MoveStuckTrigger(ai); };
            creators["move long stuck"] = [](PlayerbotAI* ai) { return new MoveLongStuckTrigger(ai); };
            creators["combat stuck"] = [](PlayerbotAI* ai) { return new CombatStuckTrigger(ai); };
            creators["combat long stuck"] = [](PlayerbotAI* ai) { return new CombatLongStuckTrigger(ai); };
            creators["leader is afk"] = [](PlayerbotAI* ai) { return new LeaderIsAfkTrigger(ai); };

            creators["petition signed"] = [](PlayerbotAI* ai) { return new PetitionTurnInTrigger(ai); };
            creators["buy tabard"] = [](PlayerbotAI* ai) { return new BuyTabardTrigger(ai); };
            creators["leave large guild"] = [](PlayerbotAI* ai) { return new LeaveLargeGuildTrigger(ai); };
            creators["in pvp"] = [](PlayerbotAI* ai) { return new InPvpTrigger(ai); };
            creators["in pve"] = [](PlayerbotAI* ai) { return new InPveTrigger(ai); };
            creators["in raid fight"] = [](PlayerbotAI* ai) { return new InRaidFightTrigger(ai); };

            creators["val"] = [](PlayerbotAI* ai) { return new ValueTrigger(ai); };

            creators["rpg"] = [](PlayerbotAI* ai) { return new RpgTrigger(ai); };
            creators["rpg wander"] = [](PlayerbotAI* ai) { return new RpgWanderTrigger(ai); };
            creators["rpg taxi"] = [](PlayerbotAI* ai) { return new RpgTaxiTrigger(ai); };
            creators["rpg discover"] = [](PlayerbotAI* ai) { return new RpgDiscoverTrigger(ai); };
            creators["rpg start quest"] = [](PlayerbotAI* ai) { return new RpgStartQuestTrigger(ai); };
            creators["rpg end quest"] = [](PlayerbotAI* ai) { return new RpgEndQuestTrigger(ai); };
            creators["rpg repeat quest"] = [](PlayerbotAI* ai) { return new RpgRepeatQuestTrigger(ai); };
            creators["rpg buy"] = [](PlayerbotAI* ai) { return new RpgBuyTrigger(ai); };
            creators["rpg sell"] = [](PlayerbotAI* ai) { return new RpgSellTrigger(ai); };
            creators["rpg ah sell"] = [](PlayerbotAI* ai) { return new RpgAHSellTrigger(ai); };
            creators["rpg ah buy"] = [](PlayerbotAI* ai) { return new RpgAHBuyTrigger(ai); };
            creators["rpg get mail"] = [](PlayerbotAI* ai) { return new RpgGetMailTrigger(ai); };
            creators["rpg repair"] = [](PlayerbotAI* ai) { return new RpgRepairTrigger(ai); };
            creators["rpg train"] = [](PlayerbotAI* ai) { return new RpgTrainTrigger(ai); };
            creators["rpg heal"] = [](PlayerbotAI* ai) { return new RpgHealTrigger(ai); };
            creators["rpg home bind"] = [](PlayerbotAI* ai) { return new RpgHomeBindTrigger(ai); };
            creators["rpg queue bg"] = [](PlayerbotAI* ai) { return new RpgQueueBGTrigger(ai); };
            creators["rpg buy petition"] = [](PlayerbotAI* ai) { return new RpgBuyPetitionTrigger(ai); };
            creators["rpg use"] = [](PlayerbotAI* ai) { return new RpgUseTrigger(ai); };
            creators["rpg quest use"] = [](PlayerbotAI* ai) { return new RpgQuestUseTrigger(ai); };
            creators["rpg ai chat"] = [](PlayerbotAI* ai) { return new RpgAIChatTrigger(ai); };
            creators["rpg spell"] = [](PlayerbotAI* ai) { return new RpgSpellTrigger(ai); };
            creators["rpg spell click"] = [](PlayerbotAI* ai) { return new RpgSpellClickTrigger(ai); };
            creators["rpg craft"] = [](PlayerbotAI* ai) { return new RpgCraftTrigger(ai); };
            creators["rpg trade useful"] = [](PlayerbotAI* ai) { return new RpgTradeUsefulTrigger(ai); };
            creators["rpg enchant"] = [](PlayerbotAI* ai) { return new RpgEnchantTrigger(ai); };
            creators["rpg duel"] = [](PlayerbotAI* ai) { return new RpgDuelTrigger(ai); };
            creators["rpg item"] = [](PlayerbotAI* ai) { return new RpgItemTrigger(ai); };
            creators["rpg gossip talk"] = [](PlayerbotAI* ai) { return new RpgGossipTalkTrigger(ai); };

            creators["random jump"] = [](PlayerbotAI* ai) { return new RandomJumpTrigger(ai); };
            creators["rtsc jump active"] = [](PlayerbotAI* ai) { return new RtscJumpTrigger(ai); };

            creators["apply glyphs"] = [](PlayerbotAI* ai) { return new ApplyGlyphTrigger(ai); };

            // racials
            creators["berserking"] = [](PlayerbotAI* ai) { return new BerserkingTrigger(ai); };
            creators["blood fury"] = [](PlayerbotAI* ai) { return new BloodFuryTrigger(ai); };
            creators["cannibalize"] = [](PlayerbotAI* ai) { return new CannibalizeTrigger(ai); };
            creators["will of the forsaken"] = [](PlayerbotAI* ai) { return new WOtFTrigger(ai); };
            creators["stoneform"] = [](PlayerbotAI* ai) { return new StoneformTrigger(ai); };
            creators["shadowmeld"] = [](PlayerbotAI* ai) { return new ShadowmeldTrigger(ai); };
            creators["mana tap"] = [](PlayerbotAI* ai) { return new ManaTapTrigger(ai); };
            creators["arcane torrent"] = [](PlayerbotAI* ai) { return new ArcanetorrentTrigger(ai); };
            creators["war stomp"] = [](PlayerbotAI* ai) { return new WarStompTrigger(ai); };
            creators["perception"] = [](PlayerbotAI* ai) { return new PerceptionTrigger(ai); };

            // Dungeon Triggers
            creators["enter onyxia's lair"] = [](PlayerbotAI* ai) { return new OnyxiasLairEnterDungeonTrigger(ai); };
            creators["leave onyxia's lair"] = [](PlayerbotAI* ai) { return new OnyxiasLairLeaveDungeonTrigger(ai); };
            creators["enter molten core"] = [](PlayerbotAI* ai) { return new MoltenCoreEnterDungeonTrigger(ai); };
            creators["leave molten core"] = [](PlayerbotAI* ai) { return new MoltenCoreLeaveDungeonTrigger(ai); };
            creators["enter karazhan"] = [](PlayerbotAI* ai) { return new KarazhanEnterDungeonTrigger(ai); };
            creators["leave karazhan"] = [](PlayerbotAI* ai) { return new KarazhanLeaveDungeonTrigger(ai); };
            creators["enter naxxramas"] = [](PlayerbotAI* ai) { return new NaxxramasEnterDungeonTrigger(ai); };
            creators["leave naxxramas"] = [](PlayerbotAI* ai) { return new NaxxramasLeaveDungeonTrigger(ai); };

            // Dungeon Boss Triggers
            creators["start onyxia fight"] = [](PlayerbotAI* ai) { return new OnyxiaStartFightTrigger(ai); };
            creators["end onyxia fight"] = [](PlayerbotAI* ai) { return new OnyxiaEndFightTrigger(ai); };

            creators["start magmadar fight"] = [](PlayerbotAI* ai) { return new MagmadarStartFightTrigger(ai); };
            creators["end magmadar fight"] = [](PlayerbotAI* ai) { return new MagmadarEndFightTrigger(ai); };
            creators["magmadar lava bomb"] = [](PlayerbotAI* ai) { return new MagmadarLavaBombTrigger(ai); };
            creators["magmadar too close"] = [](PlayerbotAI* ai) { return new MagmadarTooCloseTrigger(ai); };

            creators["fire protection potion ready"] = [](PlayerbotAI* ai) { return new FireProtectionPotionReadyTrigger(ai); };

            creators["mc rune in sight"] = [](PlayerbotAI* ai) { return new MCRuneInSightTrigger(ai); };
            creators["mc rune close"] = [](PlayerbotAI* ai) { return new MCRuneCloseTrigger(ai); };

            creators["start netherspite fight"] = [](PlayerbotAI* ai) { return new NetherspiteStartFightTrigger(ai); };
            creators["end netherspite fight"] = [](PlayerbotAI* ai) { return new NetherspiteEndFightTrigger(ai); };
            creators["void zone too close"] = [](PlayerbotAI* ai) { return new VoidZoneTooCloseTrigger(ai); };
            creators["add nether portal - perseverence for tank"] = [](PlayerbotAI* ai) { return new NetherspiteBeamsCheatNeedRefreshTrigger(ai, 2); };
            creators["remove nether portal buffs from netherspite"] = [](PlayerbotAI* ai) { return new NetherspiteBeamsCheatNeedRefreshTrigger(ai); };
            creators["remove nether portal - dominance"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalDominanceTrigger(ai); };
            creators["remove nether portal - perseverence"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalPerseverenceTrigger(ai); };
            creators["remove nether portal - serenity"] = [](PlayerbotAI* ai) { return new RemoveNetherPortalSerenityTrigger(ai); };

            creators["start prince malchezaar fight"] = [](PlayerbotAI* ai) { return new PrinceMalchezaarStartFightTrigger(ai); };
            creators["end prince malchezaar fight"] = [](PlayerbotAI* ai) { return new PrinceMalchezaarEndFightTrigger(ai); };
            creators["netherspite infernal too close"] = [](PlayerbotAI* ai) { return new NetherspiteInfernalTooCloseTrigger(ai); };

            creators["start four horseman fight"] = [](PlayerbotAI* ai) { return new FourHorsemanStartFightTrigger(ai); };
            creators["end four horseman fight"] = [](PlayerbotAI* ai) { return new FourHorsemanEndFightTrigger(ai); };
        }
    };
};
