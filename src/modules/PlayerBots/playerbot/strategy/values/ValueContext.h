#pragma once

#include "ActiveSpellValue.h"
#include "NearestGameObjects.h"
#include "LogLevelValue.h"
#include "NearestNpcsValue.h"
#include "PossibleTargetsValue.h"
#include "NearestAdsValue.h"
#include "NearestCorpsesValue.h"
#include "PartyMemberWithoutAuraValue.h"
#include "PartyMemberToHeal.h"
#include "PartyMemberToResurrect.h"
#include "CurrentTargetValue.h"
#include "SelfTargetValue.h"
#include "MasterTargetValue.h"
#include "LineTargetValue.h"
#include "TankTargetValue.h"
#include "DpsTargetValue.h"
#include "CcTargetValue.h"
#include "CurrentCcTargetValue.h"
#include "PetTargetValue.h"
#include "GrindTargetValue.h"
#include "RtiTargetValue.h"
#include "PartyMemberToDispel.h"
#include "StatsValues.h"
#include "AttackerCountValues.h"
#include "PossibleAttackTargetsValue.h"
#include "AvailableLootValue.h"
#include "AlwaysLootListValue.h"
#include "LootStrategyValue.h"
#include "HasAvailableLootValue.h"
#include "LastMovementValue.h"
#include "DistanceValue.h"
#include "IsMovingValue.h"
#include "IsBehindValue.h"
#include "IsFacingValue.h"
#include "ItemCountValue.h"
#include "SpellIdValue.h"
#include "ItemForSpellValue.h"
#include "SpellCastUsefulValue.h"
#include "LastSpellCastValue.h"
#include "ChatValue.h"
#include "HasTotemValue.h"
#include "HaveAnyTotemValue.h"
#include "LeastHpTargetValue.h"
#include "AoeHealValues.h"
#include "AoeValues.h"
#include "RtiValue.h"
#include "PositionValue.h"
#include "ThreatValues.h"
#include "DuelTargetValue.h"
#include "InvalidTargetValue.h"
#include "EnemyPlayerValue.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "CollisionValue.h"
#include "CraftValues.h"
#include "LastSpellCastTimeValue.h"
#include "CombatStartTimeValue.h"
#include "ManaSaveLevelValue.h"
#include "LfgValues.h"
#include "PvpValues.h"
#include "EnemyHealerTargetValue.h"
#include "Formations.h"
#include "ItemUsageValue.h"
#include "LastSaidValue.h"
#include "NearestFriendlyPlayersValue.h"
#include "NearestNonBotPlayersValue.h"
#include "NewPlayerNearbyValue.h"
#include "OutfitListValue.h"
#include "PartyMemberWithoutItemValue.h"
#include "PossibleRpgTargetsValue.h"
#include "RandomBotUpdateValue.h"
#include "RangeValues.h"
#include "SkipSpellsListValue.h"
#include "SnareTargetValue.h"
#include "Stances.h"
#include "QuestValues.h"
#include "BudgetValues.h"
#include "MaintenanceValues.h"
#include "GroupValues.h"
#include "GuildValues.h"
#include "TradeValues.h"
#include "RpgValues.h"
#include "RTSCValues.h"
#include "VendorValues.h"
#include "TrainerValues.h"
#include "AttackersValue.h"
#include "WaitForAttackTimeValue.h"
#include "LastPotionUsedTimeValue.h"
#include "OperatorValues.h"
#include "EntryValues.h"
#include "GuidPositionValues.h"
#include "EngineValues.h"
#include "FreeMoveValues.h"
#include "HazardsValue.h"
#include "TalentSpecValue.h"
#include "MountValues.h"
#include "DeadValues.h"
#include "playerbot/strategy/druid/DruidValues.h"
#include "TravelValues.h"
#include "LootValues.h"
#include "GlyphValues.h"
#include "StuckValues.h"
#include "FishValues.h"
#include "RuneForgeValues.h"

namespace ai
{
    class ValueContext : public NamedObjectContext<UntypedValue>
    {
    public:
        ValueContext()
        {
            creators["active spell"] = [](PlayerbotAI* ai) { return new ActiveSpellValue(ai); };
            creators["craft"] = [](PlayerbotAI* ai) { return new CraftValue(ai); };
            creators["collision"] = [](PlayerbotAI* ai) { return new CollisionValue(ai); };
            creators["skip spells list"] = [](PlayerbotAI* ai) { return new SkipSpellsListValue(ai); };
            creators["nearest game objects"] = [](PlayerbotAI* ai) { return new NearestGameObjects(ai); };
            creators["nearest game objects no los"] = [](PlayerbotAI* ai) { return new NearestGameObjects(ai, sPlayerbotAIConfig.sightDistance, LOS_IGNORE); };
            creators["nearest dynamic objects"] = [](PlayerbotAI* ai) { return new NearestDynamicObjects(ai); };
            creators["nearest dynamic objects no los"] = [](PlayerbotAI* ai) { return new NearestDynamicObjects(ai, sPlayerbotAIConfig.sightDistance, LOS_IGNORE); };
            creators["closest game objects static los"] = [](PlayerbotAI* ai) { return new NearestGameObjects(ai, INTERACTION_DISTANCE, LOS_STATIC); };
            creators["nearest npcs"] = [](PlayerbotAI* ai) { return new NearestNpcsValue(ai); };
            creators["nearest npcs no los"] = [](PlayerbotAI* ai) { return new NearestNpcsValue(ai, sPlayerbotAIConfig.sightDistance, true); };
            creators["nearest vehicles"] = [](PlayerbotAI* ai) { return new NearestVehiclesValue(ai); };
            creators["nearest friendly players"] = [](PlayerbotAI* ai) { return new NearestFriendlyPlayersValue(ai); };
            creators["closest friendly players"] = [](PlayerbotAI* ai) { return new NearestFriendlyPlayersValue(ai, INTERACTION_DISTANCE); };
            creators["possible targets"] = [](PlayerbotAI* ai) { return new PossibleTargetsValue(ai); };
            creators["possible targets no los"] = [](PlayerbotAI* ai) { return new PossibleTargetsValue(ai, "possible targets", sPlayerbotAIConfig.sightDistance, true); };
            creators["possible adds"] = [](PlayerbotAI* ai) { return new PossibleAddsValue(ai); };
            creators["all targets"] = [](PlayerbotAI* ai) { return new AllTargetsValue(ai); };
            creators["possible rpg targets"] = [](PlayerbotAI* ai) { return new PossibleRpgTargetsValue(ai); };
            creators["nearest adds"] = [](PlayerbotAI* ai) { return new NearestAddsValue(ai); };
            creators["nearest corpses"] = [](PlayerbotAI* ai) { return new NearestCorpsesValue(ai); };
            creators["nearest stealthed units"] = [](PlayerbotAI* ai) { return new NearestStealthedUnitsValue(ai); };
            creators["nearest stealthed unit"] = [](PlayerbotAI* ai) { return new NearestStealthedSingleUnitValue(ai); };
            creators["log level"] = [](PlayerbotAI* ai) { return new LogLevelValue(ai); };
            creators["friendly unit without aura"] = [](PlayerbotAI* ai) { return new FriendlyUnitWithoutAuraValue(ai); };
            creators["party member without aura"] = [](PlayerbotAI* ai) { return new PartyMemberWithoutAuraValue(ai); };
            creators["party tank without aura"] = [](PlayerbotAI* ai) { return new PartyTankWithoutAuraValue(ai); };
            creators["party member without my aura"] = [](PlayerbotAI* ai) { return new PartyMemberWithoutMyAuraValue(ai); };
            creators["attacker without aura"] = [](PlayerbotAI* ai) { return new AttackerWithoutAuraTargetValue(ai); };
            creators["party member to heal"] = [](PlayerbotAI* ai) { return new PartyMemberToHeal(ai); };
            creators["party member to resurrect"] = [](PlayerbotAI* ai) { return new PartyMemberToResurrect(ai); };
            creators["current target"] = [](PlayerbotAI* ai) { return new CurrentTargetValue(ai); };
            creators["self target"] = [](PlayerbotAI* ai) { return new SelfTargetValue(ai); };
            creators["master target"] = [](PlayerbotAI* ai) { return new MasterTargetValue(ai); };
            creators["line target"] = [](PlayerbotAI* ai) { return new LineTargetValue(ai); };
            creators["tank target"] = [](PlayerbotAI* ai) { return new TankTargetValue(ai); };
            creators["dps target"] = [](PlayerbotAI* ai) { return new DpsTargetValue(ai); };
            creators["dps aoe target"] = [](PlayerbotAI* ai) { return new DpsAoeTargetValue(ai); };
            creators["least hp target"] = [](PlayerbotAI* ai) { return new LeastHpTargetValue(ai); };
            creators["enemy player target"] = [](PlayerbotAI* ai) { return new EnemyPlayerValue(ai); };
            creators["enemy player targets"] = [](PlayerbotAI* ai) { return new EnemyPlayersValue(ai); };
            creators["has enemy player targets"] = [](PlayerbotAI* ai) { return new HasEnemyPlayersValue(ai); };
            creators["cc target"] = [](PlayerbotAI* ai) { return new CcTargetValue(ai); };
            creators["current cc target"] = [](PlayerbotAI* ai) { return new CurrentCcTargetValue(ai); };
            creators["pet target"] = [](PlayerbotAI* ai) { return new PetTargetValue(ai); };
            creators["old target"] = [](PlayerbotAI* ai) { return new CurrentTargetValue(ai); };
            creators["grind target"] = [](PlayerbotAI* ai) { return new GrindTargetValue(ai); };
            creators["rti target"] = [](PlayerbotAI* ai) { return new RtiTargetValue(ai); };
            creators["rti cc target"] = [](PlayerbotAI* ai) { return new RtiCcTargetValue(ai); };
            creators["duel target"] = [](PlayerbotAI* ai) { return new DuelTargetValue(ai); };
            creators["party member to dispel"] = [](PlayerbotAI* ai) { return new PartyMemberToDispel(ai); };
            creators["party member to protect"] = [](PlayerbotAI* ai) { return new PartyMemberToProtect(ai); };
            creators["party member to remove roots"] = [](PlayerbotAI* ai) { return new PartyMemberToRemoveRoots(ai); };
            creators["health"] = [](PlayerbotAI* ai) { return new HealthValue(ai); };
            creators["rage"] = [](PlayerbotAI* ai) { return new RageValue(ai); };
            creators["energy"] = [](PlayerbotAI* ai) { return new EnergyValue(ai); };
            creators["mana"] = [](PlayerbotAI* ai) { return new ManaValue(ai); };
            creators["combo"] = [](PlayerbotAI* ai) { return new ComboPointsValue(ai); };
            creators["dead"] = [](PlayerbotAI* ai) { return new IsDeadValue(ai); };
            creators["pet dead"] = [](PlayerbotAI* ai) { return new PetIsDeadValue(ai); };
            creators["pet happy"] = [](PlayerbotAI* ai) { return new PetIsHappyValue(ai); };
            creators["has mana"] = [](PlayerbotAI* ai) { return new HasManaValue(ai); };
            creators["attackers count"] = [](PlayerbotAI* ai) { return new AttackersCountValue(ai); };
            creators["possible attack targets count"] = [](PlayerbotAI* ai) { return new PossibleAttackTargetsCountValue(ai); };
            creators["my attacker count"] = [](PlayerbotAI* ai) { return new MyAttackerCountValue(ai); };
            creators["has aggro"] = [](PlayerbotAI* ai) { return new HasAggroValue(ai); };
            creators["has attackers"] = [](PlayerbotAI* ai) { return new HasAttackersValue(ai); };
            creators["has possible attack targets"] = [](PlayerbotAI* ai) { return new HasPossibleAttackTargetsValue(ai); };
            creators["mounted"] = [](PlayerbotAI* ai) { return new IsMountedValue(ai); };
            creators["mount skilltype"] = [](PlayerbotAI* ai) { return new MountSkillTypeValue(ai); };
            creators["available mount vendors"] = [](PlayerbotAI* ai) { return new AvailableMountVendors(ai); };
            creators["can train mount"] = [](PlayerbotAI* ai) { return new CanTrainMountValue(ai); };
            creators["can buy mount"] = [](PlayerbotAI* ai) { return new CanBuyMountValue(ai); };

            creators["can loot"] = [](PlayerbotAI* ai) { return new CanLootValue(ai); };
            creators["loot target"] = [](PlayerbotAI* ai) { return new LootTargetValue(ai); };
            creators["available loot"] = [](PlayerbotAI* ai) { return new AvailableLootValue(ai); };
            creators["has available loot"] = [](PlayerbotAI* ai) { return new HasAvailableLootValue(ai); };
            creators["stack space for item"] = [](PlayerbotAI* ai) { return new StackSpaceForItem(ai); };
            creators["should loot object"] = [](PlayerbotAI* ai) { return new ShouldLootObject(ai); };
            creators["always loot list"] = [](PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "always loot list"); };
            creators["skip loot list"] = [](PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "skip loot list"); };
            creators["skip go loot list"] = [](PlayerbotAI* ai) { return new AlwaysLootListValue(ai, "skip go loot list"); };
            creators["loot strategy"] = [](PlayerbotAI* ai) { return new LootStrategyValue(ai); };
            creators["active rolls"] = [](PlayerbotAI* ai) { return new ActiveRolls(ai); };
            creators["last movement"] = [](PlayerbotAI* ai) { return new LastMovementValue(ai); };
            creators["stay time"] = [](PlayerbotAI* ai) { return new StayTimeValue(ai); };
            creators["last taxi"] = [](PlayerbotAI* ai) { return new LastMovementValue(ai); };
            creators["last area trigger"] = [](PlayerbotAI* ai) { return new LastMovementValue(ai); };
            creators["distance"] = [](PlayerbotAI* ai) { return new DistanceValue(ai); };
            creators["moving"] = [](PlayerbotAI* ai) { return new IsMovingValue(ai); };
            creators["swimming"] = [](PlayerbotAI* ai) { return new IsSwimmingValue(ai); };
            creators["behind"] = [](PlayerbotAI* ai) { return new IsBehindValue(ai); };
            creators["facing"] = [](PlayerbotAI* ai) { return new IsFacingValue(ai); };

            creators["item count"] = [](PlayerbotAI* ai) { return new ItemCountValue(ai); };
            creators["inventory items"] = [](PlayerbotAI* ai) { return new InventoryItemValue(ai); };
            creators["inventory item ids"] = [](PlayerbotAI* ai) { return new InventoryItemIdValue(ai); };
            creators["trinkets on use"] = [](PlayerbotAI* ai) { return new EquipedUsableTrinketValue(ai); };

            creators["spell id"] = [](PlayerbotAI* ai) { return new SpellIdValue(ai); };
            creators["vehicle spell id"] = [](PlayerbotAI* ai) { return new VehicleSpellIdValue(ai); };
            creators["item for spell"] = [](PlayerbotAI* ai) { return new ItemForSpellValue(ai); };
            creators["spell cast useful"] = [](PlayerbotAI* ai) { return new SpellCastUsefulValue(ai); };
            creators["spell ready"] = [](PlayerbotAI* ai) { return new SpellReadyValue(ai); };
            creators["last spell cast"] = [](PlayerbotAI* ai) { return new LastSpellCastValue(ai); };
            creators["last spell cast time"] = [](PlayerbotAI* ai) { return new LastSpellCastTimeValue(ai); };
            creators["last potion used time"] = [](PlayerbotAI* ai) { return new LastPotionUsedTimeValue(ai); };
            creators["chat"] = [](PlayerbotAI* ai) { return new ChatValue(ai); };
            creators["has totem"] = [](PlayerbotAI* ai) { return new HasTotemValue(ai); };
            creators["have any totem"] = [](PlayerbotAI* ai) { return new HaveAnyTotemValue(ai); };

            creators["aoe heal"] = [](PlayerbotAI* ai) { return new AoeHealValue(ai); };

            creators["rti cc"] = [](PlayerbotAI* ai) { return new RtiCcValue(ai); };
            creators["rti"] = [](PlayerbotAI* ai) { return new RtiValue(ai); };
            creators["position"] = [](PlayerbotAI* ai) { return new PositionValue(ai); };
            creators["pos"] = [](PlayerbotAI* ai) { return new SinglePositionValue(ai); };
            creators["current position"] = [](PlayerbotAI* ai) { return new CurrentPositionValue(ai); };
            creators["master position"] = [](PlayerbotAI* ai) { return new MasterPositionValue(ai); };
            creators["custom position"] = [](PlayerbotAI* ai) { return new CustomPositionValue(ai); };
            creators["my threat"] = [](PlayerbotAI* ai) { return new MyThreatValue(ai); };
            creators["tank threat"] = [](PlayerbotAI* ai) { return new TankThreatValue(ai); };
            creators["threat"] = [](PlayerbotAI* ai) { return new ThreatValue(ai); };

            creators["incoming damage"] = [](PlayerbotAI* ai) { return new IncomingDamageValue(ai); };
            creators["balance"] = [](PlayerbotAI* ai) { return new BalancePercentValue(ai); };
            creators["possible attack targets"] = [](PlayerbotAI* ai) { return new PossibleAttackTargetsValue(ai); };
            creators["attackers"] = [](PlayerbotAI* ai) { return new AttackersValue(ai); };
            creators["attackers targeting me"] = [](PlayerbotAI* ai) { return new AttackersTargetingMeValue(ai); };
            creators["closest attacker targeting me"] = [](PlayerbotAI* ai) { return new ClosestAttackerTargetingMeTargetValue(ai); };
            creators["add hazard"] = [](PlayerbotAI* ai) { return new AddHazardValue(ai); };
            creators["stored hazards"] = [](PlayerbotAI* ai) { return new StoredHazardsValue(ai); };
            creators["hazards"] = [](PlayerbotAI* ai) { return new HazardsValue(ai); };
            creators["invalid target"] = [](PlayerbotAI* ai) { return new InvalidTargetValue(ai); };
            creators["mana save level"] = [](PlayerbotAI* ai) { return new ManaSaveLevelValue(ai); };
            creators["combat"] = [](PlayerbotAI* ai) { return new IsInCombatValue(ai); };
            creators["lfg proposal"] = [](PlayerbotAI* ai) { return new LfgProposalValue(ai); };
            creators["bag space"] = [](PlayerbotAI* ai) { return new BagSpaceValue(ai); };
            creators["durability"] = [](PlayerbotAI* ai) { return new DurabilityValue(ai); };
            creators["durability inventory"] = [](PlayerbotAI* ai) { return new DurabilityInventoryValue(ai); };
            creators["lowest durability"] = [](PlayerbotAI* ai) { return new LowestDurabilityValue(ai); };
            creators["max repair cost"] = [](PlayerbotAI* ai) { return new MaxGearRepairCostValue(ai); };
            creators["repair cost"] = [](PlayerbotAI* ai) { return new RepairCostValue(ai); };
            creators["min repair cost"] = [](PlayerbotAI* ai) { return new MinRepairCostValue(ai); };
            creators["train cost"] = [](PlayerbotAI* ai) { return new TrainCostValue(ai); };
            creators["enemy healer target"] = [](PlayerbotAI* ai) { return new EnemyHealerTargetValue(ai); };
            creators["snare target"] = [](PlayerbotAI* ai) { return new SnareTargetValue(ai); };
            creators["formation"] = [](PlayerbotAI* ai) { return new FormationValue(ai); };
            creators["formation position"] = [](PlayerbotAI* ai) { return new FormationPositionValue(ai); };
            creators["stance"] = [](PlayerbotAI* ai) { return new StanceValue(ai); };
            creators["item usage"] = [](PlayerbotAI* ai) { return new ItemUsageValue(ai); };
            creators["force item usage"] = [](PlayerbotAI* ai) { return new ForceItemUsageValue(ai); };
            creators["speed"] = [](PlayerbotAI* ai) { return new SpeedValue(ai); };
            creators["last said"] = [](PlayerbotAI* ai) { return new LastSaidValue(ai); };
            creators["last emote"] = [](PlayerbotAI* ai) { return new LastEmoteValue(ai); };

            creators["aoe count"] = [](PlayerbotAI* ai) { return new AoeCountValue(ai); };
            creators["aoe position"] = [](PlayerbotAI* ai) { return new AoePositionValue(ai); };
            creators["outfit list"] = [](PlayerbotAI* ai) { return new OutfitListValue(ai); };

            creators["random bot update"] = [](PlayerbotAI* ai) { return new RandomBotUpdateValue(ai); };
            creators["nearest non bot players"] = [](PlayerbotAI* ai) { return new NearestNonBotPlayersValue(ai); };
            creators["new player nearby"] = [](PlayerbotAI* ai) { return new NewPlayerNearbyValue(ai); };
            creators["already seen players"] = [](PlayerbotAI* ai) { return new AlreadySeenPlayersValue(ai); };
            creators["group"] = [](PlayerbotAI* ai) { return new IsInGroupValue(ai); };
            creators["range"] = [](PlayerbotAI* ai) { return new RangeValue(ai); };
            creators["inside target"] = [](PlayerbotAI* ai) { return new InsideTargetValue(ai); };
            creators["party member without item"] = [](PlayerbotAI* ai) { return new PartyMemberWithoutItemValue(ai); };
            creators["party member without food"] = [](PlayerbotAI* ai) { return new PartyMemberWithoutFoodValue(ai); };
            creators["party member without water"] = [](PlayerbotAI* ai) { return new PartyMemberWithoutWaterValue(ai); };
            creators["death count"] = [](PlayerbotAI* ai) { return new DeathCountValue(ai); };

            creators["rpg target"] = [](PlayerbotAI* ai) { return new RpgTargetValue(ai); };
            creators["ignore rpg target"] = [](PlayerbotAI* ai) { return new IgnoreRpgTargetValue(ai); };
            creators["next rpg action"] = [](PlayerbotAI* ai) { return new NextRpgActionValue(ai); };
            creators["talk target"] = [](PlayerbotAI* ai) { return new TalkTargetValue(ai); };
            creators["attack target"] = [](PlayerbotAI* ai) { return new AttackTargetValue(ai); };
            creators["pull target"] = [](PlayerbotAI* ai) { return new PullTargetValue(ai); };
            creators["follow target"] = [](PlayerbotAI* ai) { return new FollowTargetValue(ai); };
            creators["manual follow target"] = [](PlayerbotAI* ai) { return new ManualFollowTargetValue(ai); };
            creators["focus heal targets"] = [](PlayerbotAI* ai) { return new FocusHealTargetValue(ai); };
            creators["boost targets"] = [](PlayerbotAI* ai) { return new BoostTargetsValue(ai); };
            creators["revive targets"] = [](PlayerbotAI* ai) { return new ReviveTargetsValue(ai); };
            creators["buff targets"] = [](PlayerbotAI* ai) { return new BuffTargetsValue(ai); };

            creators["bg type"] = [](PlayerbotAI* ai) { return new BgTypeValue(ai); };
            creators["rpg bg type"] = [](PlayerbotAI* ai) { return new RpgBgTypeValue(ai); };
            creators["arena type"] = [](PlayerbotAI* ai) { return new ArenaTypeValue(ai); };
            creators["bg role"] = [](PlayerbotAI* ai) { return new BgRoleValue(ai); };
            creators["bg master"] = [](PlayerbotAI* ai) { return new BgMasterValue(ai); };
            creators["enemy flag carrier"] = [](PlayerbotAI* ai) { return new FlagCarrierValue(ai, false, true); };
            creators["team flag carrier"] = [](PlayerbotAI* ai) { return new FlagCarrierValue(ai, true, true); };

            creators["home bind"] = [](PlayerbotAI* ai) { return new HomeBindValue(ai); };
            creators["last long move"] = [](PlayerbotAI* ai) { return new LastLongMoveValue(ai); };
            creators["graveyard"] = [](PlayerbotAI* ai) { return new GraveyardValue(ai); };
            creators["best graveyard"] = [](PlayerbotAI* ai) { return new BestGraveyardValue(ai); };
            creators["should spirit healer"] = [](PlayerbotAI* ai) { return new ShouldSpiritHealerValue(ai); };

            creators["bot roles"] = [](PlayerbotAI* ai) { return new BotRolesValue(ai); };
            creators["talent spec"] = [](PlayerbotAI* ai) { return new TalentSpecValue(ai); };

            creators["free quest log slots"] = [](PlayerbotAI* ai) { return new FreeQuestLogSlotValue(ai); };
            creators["dialog status"] = [](PlayerbotAI* ai) { return new DialogStatusValue(ai); };
            creators["dialog status quest"] = [](PlayerbotAI* ai) { return new DialogStatusQuestValue(ai); };
            creators["can accept quest npc"] = [](PlayerbotAI* ai) { return new CanAcceptQuestValue(ai); };
            creators["can accept quest low level npc"] = [](PlayerbotAI* ai) { return new CanAcceptQuestLowLevelValue(ai); };
            creators["can turn in quest npc"] = [](PlayerbotAI* ai) { return new CanTurnInQuestValue(ai); };
            creators["can repeat quest npc"] = [](PlayerbotAI* ai) { return new CanRepeatQuestValue(ai); };
            creators["need quest reward"] = [](PlayerbotAI* ai) { return new NeedQuestRewardValue(ai); };
            creators["need quest objective"] = [](PlayerbotAI* ai) { return new NeedQuestObjectiveValue(ai); };
            creators["need for quest"] = [](PlayerbotAI* ai) { return new NeedForQuestValue(ai); };
            creators["can use item on"] = [](PlayerbotAI* ai) { return new CanUseItemOn(ai); };
            creators["quest reward"] = [](PlayerbotAI* ai) { return new QuestRewardValue(ai); };
            creators["has nearby quest taker"] = [](PlayerbotAI* ai) { return new HasNearbyQuestTakerValue(ai); };

            creators["money needed for"] = [](PlayerbotAI* ai) { return new MoneyNeededForValue(ai); };
            creators["total money needed for"] = [](PlayerbotAI* ai) { return new TotalMoneyNeededForValue(ai); };
            creators["free money for"] = [](PlayerbotAI* ai) { return new FreeMoneyForValue(ai); };
            creators["has all money for"] = [](PlayerbotAI* ai) { return new HasAllMoneyForValue(ai); };
            creators["should get money"] = [](PlayerbotAI* ai) { return new ShouldGetMoneyValue(ai); };

            creators["free move center"] = [](PlayerbotAI* ai) { return new FreeMoveCenterValue(ai); };
            creators["free move range"] = [](PlayerbotAI* ai) { return new FreeMoveRangeValue(ai); };
            creators["can free move to"] = [](PlayerbotAI* ai) { return new CanFreeMoveToValue(ai); };
            creators["can free attack"] = [](PlayerbotAI* ai) { return new CanFreeAttackValue(ai); };
            creators["can free target"] = [](PlayerbotAI* ai) { return new CanFreeTargetValue(ai); };

            creators["can move around"] = [](PlayerbotAI* ai) { return new CanMoveAroundValue(ai); };
            creators["should home bind"] = [](PlayerbotAI* ai) { return new ShouldHomeBindValue(ai); };
            creators["should repair"] = [](PlayerbotAI* ai) { return new ShouldRepairValue(ai); };
            creators["can repair"] = [](PlayerbotAI* ai) { return new CanRepairValue(ai); };
            creators["should sell"] = [](PlayerbotAI* ai) { return new ShouldSellValue(ai); };
            creators["can sell"] = [](PlayerbotAI* ai) { return new CanSellValue(ai); };
            creators["can buy"] = [](PlayerbotAI* ai) { return new CanBuyValue(ai); };
            creators["should ah sell"] = [](PlayerbotAI* ai) { return new ShouldAHSellValue(ai); };
            creators["can ah sell"] = [](PlayerbotAI* ai) { return new CanAHSellValue(ai); };
            creators["can ah buy"] = [](PlayerbotAI* ai) { return new CanAHBuyValue(ai); };
            creators["can get mail"] = [](PlayerbotAI* ai) { return new CanGetMailValue(ai); };
            creators["should get mail"] = [](PlayerbotAI* ai) { return new ShouldGetMailValue(ai); };
            creators["can fight equal"] = [](PlayerbotAI* ai) { return new CanFightEqualValue(ai); };
            creators["can fight elite"] = [](PlayerbotAI* ai) { return new CanFightEliteValue(ai); };
            creators["can fight boss"] = [](PlayerbotAI* ai) { return new CanFightBossValue(ai); };
            creators["should drink"] = [](PlayerbotAI* ai) { return new ShouldDrinkValue(ai); };
            creators["should eat"] = [](PlayerbotAI* ai) { return new ShouldEatValue(ai); };
            creators["drink duration"] = [](PlayerbotAI* ai) { return new DrinkDurationValue(ai); };
            creators["eat duration"] = [](PlayerbotAI* ai) { return new EatDurationValue(ai); };

            creators["vendor has useful item"] = [](PlayerbotAI* ai) { return new VendorHasUsefulItemValue(ai); };
            creators["craft spells"] = [](PlayerbotAI* ai) { return new CraftSpellsValue(ai); };
            creators["enchant spells"] = [](PlayerbotAI* ai) { return new EnchantSpellsValue(ai); };
            creators["has reagents for"] = [](PlayerbotAI* ai) { return new HasReagentsForValue(ai); };
            creators["can craft spell"] = [](PlayerbotAI* ai) { return new CanCraftSpellValue(ai); };
            creators["should craft spell"] = [](PlayerbotAI* ai) { return new ShouldCraftSpellValue(ai); };

            creators["group members"] = [](PlayerbotAI* ai) { return new GroupMembersValue(ai); };
            creators["following party"] = [](PlayerbotAI* ai) { return new IsFollowingPartyValue(ai); };
            creators["near leader"] = [](PlayerbotAI* ai) { return new IsNearLeaderValue(ai); };
            creators["and"] = [](PlayerbotAI* ai) { return new BoolAndValue(ai); };
            creators["or"] = [](PlayerbotAI* ai) { return new BoolOrValue(ai); };
            creators["not"] = [](PlayerbotAI* ai) { return new NotValue(ai); };
            creators["gt32"] = [](PlayerbotAI* ai) { return new GT32Value(ai); };
            creators["manual bool"] = [](PlayerbotAI* ai) { return new BoolManualSetValue(ai); };
            creators["manual int"] = [](PlayerbotAI* ai) { return new IntManualSetValue(ai); };
            creators["manual saved int"] = [](PlayerbotAI* ai) { return new IntManualSetSavedValue(ai); };
            creators["manual string"] = [](PlayerbotAI* ai) { return new StringManualSetValue(ai); };
            creators["manual saved string"] = [](PlayerbotAI* ai) { return new StringManualSetSavedValue(ai); };
            creators["manual time"] = [](PlayerbotAI* ai) { return new TimeManualSetValue(ai); };
            creators["group count"] = [](PlayerbotAI* ai) { return new GroupBoolCountValue(ai); };
            creators["group and"] = [](PlayerbotAI* ai) { return new GroupBoolANDValue(ai); };
            creators["group or"] = [](PlayerbotAI* ai) { return new GroupBoolORValue(ai); };
            creators["group ready"] = [](PlayerbotAI* ai) { return new GroupReadyValue(ai); };

            creators["petition signs"] = [](PlayerbotAI* ai) { return new PetitionSignsValue(ai); };
            creators["can hand in petition"] = [](PlayerbotAI* ai) { return new CanHandInPetitionValue(ai); };
            creators["can buy tabard"] = [](PlayerbotAI* ai) { return new CanBuyTabard(ai); };
            

            creators["experience"] = [](PlayerbotAI* ai) { return new ExperienceValue(ai); };
            creators["honor"] = [](PlayerbotAI* ai) { return new HonorValue(ai); };

            creators["entry loot usage"] = [](PlayerbotAI* ai) { return new EntryLootUsageValue(ai); };
            creators["has upgrade"] = [](PlayerbotAI* ai) { return new HasUpgradeValue(ai); };
            creators["items useful to give"] = [](PlayerbotAI* ai) { return new ItemsUsefulToGiveValue(ai); };
            creators["items useful to enchant"] = [](PlayerbotAI* ai) { return new ItemsUsefulToEnchantValue(ai); };

            creators["see spell location"] = [](PlayerbotAI* ai) { return new SeeSpellLocationValue(ai); };
            creators["RTSC selected"] = [](PlayerbotAI* ai) { return new RTSCSelectedValue(ai); };
            creators["RTSC next spell action"] = [](PlayerbotAI* ai) { return new RTSCNextSpellActionValue(ai); };
            creators["RTSC saved location"] = [](PlayerbotAI* ai) { return new RTSCSavedLocationValue(ai); };

            creators["trainable spells"] = [](PlayerbotAI* ai) { return new TrainableSpellsValue(ai); };
            creators["available trainers"] = [](PlayerbotAI* ai) { return new AvailableTrainersValue(ai); };
            creators["mount list"] = [](PlayerbotAI* ai) { return new MountListValue(ai); };
            creators["current mount speed"] = [](PlayerbotAI* ai) { return new CurrentMountSpeedValue(ai); };
            creators["max mount speed"] = [](PlayerbotAI* ai) { return new MaxMountSpeedValue(ai); };

            creators["has area debuff"] = [](PlayerbotAI* ai) { return new HasAreaDebuffValue(ai); };
            creators["combat start time"] = [](PlayerbotAI* ai) { return new CombatStartTimeValue(ai); };
            creators["wait for attack time"] = [](PlayerbotAI* ai) { return new WaitForAttackTimeValue(ai); };

            creators["mc runes"] = [](PlayerbotAI* ai) { return new MCRunesValue(ai); };
            creators["gos"] = [](PlayerbotAI* ai) { return new GameObjectsValue(ai); };
            creators["entry filter"] = [](PlayerbotAI* ai) { return new EntryFilterValue(ai); };
            creators["guid filter"] = [](PlayerbotAI* ai) { return new GuidFilterValue(ai); };
            creators["range filter"] = [](PlayerbotAI* ai) { return new RangeFilterValue(ai); };
            creators["go usable filter"] = [](PlayerbotAI* ai) { return new GoUsableFilterValue(ai); };
            creators["go trapped filter"] = [](PlayerbotAI* ai) { return new GoTrappedFilterValue(ai); };
            creators["gos in sight"] = [](PlayerbotAI* ai) { return new GosInSightValue(ai); };
            creators["gos close"] = [](PlayerbotAI* ai) { return new GoSCloseValue(ai); };
            creators["has object"] = [](PlayerbotAI* ai) { return new HasObjectValue(ai); };

            creators["action possible"] = [](PlayerbotAI* ai) { return new ActionPossibleValue(ai); };
            creators["action useful"] = [](PlayerbotAI* ai) { return new ActionUsefulValue(ai); };
            creators["trigger active"] = [](PlayerbotAI* ai) { return new TriggerActiveValue(ai); };
            creators["has strategy"] = [](PlayerbotAI* ai) { return new HasStrategyValue(ai); };

            creators["party tank without lifebloom"] = [](PlayerbotAI* ai) { return new PartyTankWithoutLifebloomValue(ai); };
            creators["move style"] = [](PlayerbotAI* ai) { return new MoveStyleValue(ai); };
            creators["available glyphs"] = [](PlayerbotAI* ai) { return new AvailableGlyphsValue(ai); };
            creators["wanted glyphs"] = [](PlayerbotAI* ai) { return new WantedGlyphsValue(ai); };
            creators["equiped glyphs"] = [](PlayerbotAI* ai) { return new EquipedGlyphsValue(ai); };
            creators["glyph is upgrade"] = [](PlayerbotAI* ai) { return new GlyphIsUpgradeValue(ai); };

            //Travel
            creators["focus travel target"] = [](PlayerbotAI* ai) { return new FocusTravelTargetValue(ai); };
            creators["has focus travel target"] = [](PlayerbotAI* ai) { return new HasFocusTravelTargetValue(ai); };

            creators["travel target"] = [](PlayerbotAI* ai) { return new TravelTargetValue(ai); };
            creators["leader travel target"] = [](PlayerbotAI* ai) { return new LeaderTravelTargetValue(ai); };

            creators["travel target active"] = [](PlayerbotAI* ai) { return new TravelTargetActiveValue(ai); };            
            creators["travel target traveling"] = [](PlayerbotAI* ai) { return new TravelTargetTravelingValue(ai); };
            creators["travel target working"] = [](PlayerbotAI* ai) { return new TravelTargetWorkingValue(ai); };
            
            creators["future travel destinations"] = [](PlayerbotAI* ai) { return new FutureTravelDestinationsValue(ai); };
            creators["no active travel destinations"] = [](PlayerbotAI* ai) { return new NoActiveTravelDestinationsValue(ai); };
            creators["need travel purpose"] = [](PlayerbotAI* ai) { return new NeedTravelPurposeValue(ai); };
            creators["should travel named"] = [](PlayerbotAI* ai) { return new ShouldTravelNamedValue(ai); };
            creators["in overworld"] = [](PlayerbotAI* ai) { return new InOverworldValue(ai); };
            creators["quest stage active"] = [](PlayerbotAI* ai) { return new QuestStageActiveValue(ai); };

            creators["can fish"] = [](PlayerbotAI* ai) { return new CanFishValue(ai); };
            creators["can open fishing dobber"] = [](PlayerbotAI* ai) { return new CanOpenFishingDobberValue(ai); };
            creators["done fishing"] = [](PlayerbotAI* ai) { return new DoneFishingValue(ai); };

            //Stuck
            creators["time since last change"] = [](PlayerbotAI* ai) { return new TimeSinceLastChangeValue(ai); };
            creators["distance moved since"] = [](PlayerbotAI* ai) { return new DistanceMovedSinceValue(ai); };

            creators["runeforge spells"] = [](PlayerbotAI* ai) { return new RuneForgeSpellsValue(ai); };
            creators["best runeforge spell"] = [](PlayerbotAI* ai) { return new BestRuneForgeSpellValue(ai); };
            creators["should runeforge"] = [](PlayerbotAI* ai) { return new ShouldRuneForgeValue(ai); };
        };
    };
}