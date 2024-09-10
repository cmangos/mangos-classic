/*
 * This file is part of the CMaNGOS Project. See AUTHORS file for Copyright information
 *
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

#include "Common.h"
#include "Database/DatabaseEnv.h"
#include "Server/WorldSession.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Accounts/AccountMgr.h"
#include "Tools/PlayerDump.h"
#include "Spells/SpellMgr.h"
#include "Entities/Player.h"
#include "Entities/GameObject.h"
#include "Chat/Chat.h"
#include "Log/Log.h"
#include "Guilds/Guild.h"
#include "Guilds/GuildMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Maps/MapManager.h"
#include "Mails/MassMailMgr.h"
#include "DBScripts/ScriptMgr.h"
#include "Tools/Language.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Weather/Weather.h"
#include "MotionGenerators/PointMovementGenerator.h"
#include "MotionGenerators/PathFinder.h"
#include "MotionGenerators/TargetedMovementGenerator.h"
#include "SystemConfig.h"
#include "Config/Config.h"
#include "Mails/Mail.h"
#include "Util/Util.h"
#include "Entities/ItemEnchantmentMgr.h"
#include "BattleGround/BattleGroundMgr.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Maps/InstanceData.h"
#include "Server/DBCStores.h"
#include "AI/EventAI/CreatureEventAIMgr.h"
#include "Server/SQLStorages.h"
#include "Loot/LootMgr.h"
#include "World/WorldState.h"
#ifdef BUILD_METRICS
#include "Metric/Metric.h"
#endif
#include "Server/PacketLog.h"

#include "Globals/UnitCondition.h"
#include "Globals/CombatCondition.h"
#include "World/WorldStateExpression.h"

#include "MotionGenerators/MoveMap.h"

#ifdef BUILD_AHBOT
#include "AuctionHouseBot/AuctionHouseBot.h"

bool ChatHandler::HandleAHBotRebuildCommand(char* args)
{
    bool all = false;
    if (*args)
    {
        if (!ExtractLiteralArg(&args, "all"))
            return false;
        all = true;
    }

    sAuctionHouseBot.Rebuild(all);
    return true;
}

bool ChatHandler::HandleAHBotReloadCommand(char* /*args*/)
{
    if (sAuctionHouseBot.ReloadAllConfig())
    {
        SendSysMessage(LANG_AHBOT_RELOAD_OK);
        return true;
    }
    SendSysMessage(LANG_AHBOT_RELOAD_FAIL);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleAHBotStatusCommand(char* /*args*/)
{
    AuctionHouseBotStatusInfo statusInfo;
    sAuctionHouseBot.PrepareStatusInfos(statusInfo);

    if (!m_session)
    {
        SendSysMessage(LANG_AHBOT_STATUS_BAR_CONSOLE);
        SendSysMessage(LANG_AHBOT_STATUS_TITLE1_CONSOLE);
        SendSysMessage(LANG_AHBOT_STATUS_MIDBAR_CONSOLE);
    }
    else
        SendSysMessage(LANG_AHBOT_STATUS_TITLE1_CHAT);

    uint32 fmtId = m_session ? LANG_AHBOT_STATUS_FORMAT_CHAT : LANG_AHBOT_STATUS_FORMAT_CONSOLE;

    PSendSysMessage(fmtId, GetMangosString(LANG_AHBOT_STATUS_ITEM_COUNT),
                    statusInfo[AUCTION_HOUSE_ALLIANCE].ItemsCount,
                    statusInfo[AUCTION_HOUSE_HORDE].ItemsCount,
                    statusInfo[AUCTION_HOUSE_NEUTRAL].ItemsCount,
                    statusInfo[AUCTION_HOUSE_ALLIANCE].ItemsCount +
                    statusInfo[AUCTION_HOUSE_HORDE].ItemsCount +
                    statusInfo[AUCTION_HOUSE_NEUTRAL].ItemsCount);

    if (!m_session)
        SendSysMessage(LANG_AHBOT_STATUS_BAR_CONSOLE);

    return true;
}

bool ChatHandler::HandleAHBotItemCommand(char* args)
{
    // .ahbot item #itemid [$itemvalue [$addchance [$minstack [$maxstack]]]] [reset]
    char* cId = ExtractKeyFromLink(&args, "Hitem");
    if (!cId)
        return false;

    uint32 itemId = 0;
    if (!ExtractUInt32(&cId, itemId))                       // [name] manual form
    {
        std::string itemName = cId;
        WorldDatabase.escape_string(itemName);
        auto queryResult = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
        if (!queryResult)
        {
            PSendSysMessage(LANG_COMMAND_COULDNOTFIND, cId);
            SetSentErrorMessage(true);
            return false;
        }
        itemId = queryResult->Fetch()->GetUInt16();
    }
    ItemPrototype const* proto = ObjectMgr::GetItemPrototype(itemId);
    if (!proto)
    {
        PSendSysMessage(LANG_COMMAND_COULDNOTFIND, cId);
        return false;
    }

    AuctionHouseBotItemData itemData;
    bool reset = ExtractLiteralArg(&args, "reset") != nullptr;
    bool setItemData = true;
    if (!reset && !ExtractUInt32(&args, itemData.Value))
    {
        // only item id specified, show item data to player
        itemData = sAuctionHouseBot.GetItemData(itemId);
        setItemData = false;
    }
    else if (!reset && ExtractUInt32(&args, itemData.AddChance) && ExtractUInt32(&args, itemData.MinAmount))
        ExtractUInt32(&args, itemData.MaxAmount);
    if (setItemData)
        sAuctionHouseBot.SetItemData(itemId, itemData, reset);

    std::stringstream ss;
    ss << itemData.Value / 10000 << "g, " << itemData.Value / 100 % 100 << "s, " << itemData.Value % 100 << "c. ";
    if (itemData.MinAmount == 0)
        ss << "Item data is not overridden by user.";
    else if (itemData.AddChance == 0)
        ss << "Item will be added using normal sources.";
    else
        ss << "Add chance: " << itemData.AddChance << "%, Min/Max amount: " << itemData.MinAmount << "/" << itemData.MaxAmount;
    PSendSysMessage(LANG_ITEM_LIST_CHAT, itemId, itemId, proto->Name1, ss.str().c_str());
    return true;
}
#endif

// reload commands
bool ChatHandler::HandleReloadAllCommand(char* /*args*/)
{
    HandleReloadSkillFishingBaseLevelCommand((char*)"");

    HandleReloadAllAreaCommand((char*)"");
    HandleReloadAllEventAICommand((char*)"");
    HandleReloadAllLootCommand((char*)"");
    HandleReloadAllNpcCommand((char*)"");
    HandleReloadAllQuestCommand((char*)"");
    HandleReloadAllSpellCommand((char*)"");
    HandleReloadAllItemCommand((char*)"");
    HandleReloadAllGossipsCommand((char*)"");
    HandleReloadAllLocalesCommand((char*)"");

    HandleReloadCommandCommand((char*)"");
    HandleReloadReservedNameCommand((char*)"");
    HandleReloadMangosStringCommand((char*)"");
    HandleReloadGameTeleCommand((char*)"");
    HandleReloadBattleEventCommand((char*)"");
    return true;
}

bool ChatHandler::HandleReloadAllAreaCommand(char* /*args*/)
{
    // HandleReloadQuestAreaTriggersCommand((char*)""); -- reloaded in HandleReloadAllQuestCommand
    HandleReloadAreaTriggerTeleportCommand((char*)"");
    HandleReloadAreaTriggerTavernCommand((char*)"");
    HandleReloadGameGraveyardZoneCommand((char*)"");
    HandleReloadTaxiShortcuts((char*)"");
    return true;
}

bool ChatHandler::HandleReloadAllLootCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables...");
    LootIdSet ids_set;
    LoadLootTables(ids_set);
    SendGlobalSysMessage("DB tables `*_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllNpcCommand(char* args)
{
    if (*args != 'a')                                       // will be reloaded from all_gossips
        HandleReloadNpcGossipCommand((char*)"a");
    HandleReloadNpcTrainerCommand((char*)"a");
    HandleReloadNpcVendorCommand((char*)"a");
    HandleReloadPointsOfInterestCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadAllQuestCommand(char* /*args*/)
{
    HandleReloadQuestAreaTriggersCommand((char*)"a");
    HandleReloadQuestTemplateCommand((char*)"a");

    sLog.outString("Re-Loading Quests Relations...");
    sObjectMgr.LoadQuestRelations();
    SendGlobalSysMessage("DB tables `*_questrelation` and `*_involvedrelation` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllScriptsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Scripts...");
    HandleReloadDBScriptsOnCreatureDeathCommand((char*)"a");
    HandleReloadDBScriptsOnGoUseCommand((char*)"a");
    HandleReloadDBScriptsOnGossipCommand((char*)"a");
    HandleReloadDBScriptsOnEventCommand((char*)"a");
    HandleReloadDBScriptsOnQuestEndCommand((char*)"a");
    HandleReloadDBScriptsOnQuestStartCommand((char*)"a");
    HandleReloadDBScriptsOnSpellCommand((char*)"a");
    HandleReloadDBScriptsOnRelayCommand((char*)"a");
    SendGlobalSysMessage("DB tables `*_scripts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAllEventAICommand(char* /*args*/)
{
    HandleReloadEventAISummonsCommand((char*)"a");
    HandleReloadEventAIScriptsCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadAllSpellCommand(char* /*args*/)
{
    HandleReloadSpellAffectCommand((char*)"a");
    HandleReloadSpellAreaCommand((char*)"a");
    HandleReloadSpellChainCommand((char*)"a");
    HandleReloadSpellElixirCommand((char*)"a");
    HandleReloadSpellLearnSpellCommand((char*)"a");
    HandleReloadSpellProcEventCommand((char*)"a");
    HandleReloadSpellProcItemEnchantCommand((char*)"a");
    HandleReloadSpellScriptTargetCommand((char*)"a");
    HandleReloadSpellTargetPositionCommand((char*)"a");
    HandleReloadSpellThreatsCommand((char*)"a");
    HandleReloadSpellPetAurasCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadAllGossipsCommand(char* args)
{
    if (*args != 'a')                                       // already reload from all_scripts
        HandleReloadDBScriptsOnGossipCommand((char*)"a");
    HandleReloadGossipMenuCommand((char*)"a");
    HandleReloadNpcGossipCommand((char*)"a");
    HandleReloadPointsOfInterestCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadAllItemCommand(char* /*args*/)
{
    HandleReloadPageTextsCommand((char*)"a");
    HandleReloadItemEnchantementsCommand((char*)"a");
    HandleReloadItemRequiredTragetCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadAllLocalesCommand(char* /*args*/)
{
    HandleReloadLocalesCreatureCommand((char*)"a");
    HandleReloadLocalesGameobjectCommand((char*)"a");
    HandleReloadLocalesGossipMenuOptionCommand((char*)"a");
    HandleReloadLocalesItemCommand((char*)"a");
    HandleReloadLocalesNpcTextCommand((char*)"a");
    HandleReloadLocalesPageTextCommand((char*)"a");
    HandleReloadLocalesPointsOfInterestCommand((char*)"a");
    HandleReloadLocalesQuestCommand((char*)"a");
    return true;
}

bool ChatHandler::HandleReloadConfigCommand(char* /*args*/)
{
    sLog.outString("Re-Loading config settings...");
    sWorld.LoadConfigSettings(true);
    sMapMgr.InitializeVisibilityDistanceInfo();
#ifdef BUILD_METRICS
    metric::metric::instance().reload_config();
#endif
    PacketLog::instance()->Reinitialize();
    SendGlobalSysMessage("World config settings reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTavernCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Tavern Area Triggers...");
    sObjectMgr.LoadTavernAreaTriggers();
    SendGlobalSysMessage("DB table `areatrigger_tavern` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadAreaTriggerTeleportCommand(char* /*args*/)
{
    sLog.outString("Re-Loading AreaTrigger teleport definitions...");
    sObjectMgr.LoadAreaTriggerTeleports();
    SendGlobalSysMessage("DB table `areatrigger_teleport` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesAreaTriggerCommand(char* /*args*/)
{
    sLog.outString("Re-Loading AreaTrigger teleport locales definitions...");
    sObjectMgr.LoadAreatriggerLocales();
    SendGlobalSysMessage("DB table `locales_areatrigger_teleport` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCommandCommand(char* /*args*/)
{
    load_command_table = true;
    SendGlobalSysMessage("DB table `command` will be reloaded at next chat command use.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestRelationsCommand(char* /*args*/)
{
    sLog.outString("Loading Quests Relations... (`creature_questrelation`)");
    sObjectMgr.LoadCreatureQuestRelations();
    SendGlobalSysMessage("DB table `creature_questrelation` (creature quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreatureQuestInvRelationsCommand(char* /*args*/)
{
    sLog.outString("Loading Quests Relations... (`creature_involvedrelation`)");
    sObjectMgr.LoadCreatureInvolvedRelations();
    SendGlobalSysMessage("DB table `creature_involvedrelation` (creature quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadConditionsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `conditions`... ");
    sObjectMgr.LoadConditions();
    SendGlobalSysMessage("DB table `conditions` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadCreaturesStatsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading stats data...");
    sObjectMgr.LoadCreatureClassLvlStats();
    SendGlobalSysMessage("DB table `creature_template_classlevelstats` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGossipMenuCommand(char* /*args*/)
{
    sObjectMgr.LoadGossipMenus();
    SendGlobalSysMessage("DB tables `gossip_menu` and `gossip_menu_option` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestgiverGreetingCommand(char* /*args*/)
{
    sObjectMgr.LoadQuestgiverGreeting();
    SendGlobalSysMessage("DB table `questgiver_greeting` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestgiverGreetingLocalesCommand(char* /*args*/)
{
    sObjectMgr.LoadQuestgiverGreetingLocales();
    SendGlobalSysMessage("DB table `locales_questgiver_greeting` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadTrainerGreetingCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Trainer Greetings...");
    sObjectMgr.LoadTrainerGreetings();
    SendGlobalSysMessage("DB table `trainer_greeting` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesTrainerGreetingCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Trainer Greeting Locales...");
    sObjectMgr.LoadTrainerGreetingLocales();
    SendGlobalSysMessage("DB table `locales_trainer_greeting` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestRelationsCommand(char* /*args*/)
{
    sLog.outString("Loading Quests Relations... (`gameobject_questrelation`)");
    sObjectMgr.LoadGameobjectQuestRelations();
    SendGlobalSysMessage("DB table `gameobject_questrelation` (gameobject quest givers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadGOQuestInvRelationsCommand(char* /*args*/)
{
    sLog.outString("Loading Quests Relations... (`gameobject_involvedrelation`)");
    sObjectMgr.LoadGameobjectInvolvedRelations();
    SendGlobalSysMessage("DB table `gameobject_involvedrelation` (gameobject quest takers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestAreaTriggersCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Quest Area Triggers...");
    sObjectMgr.LoadQuestAreaTriggers();
    SendGlobalSysMessage("DB table `areatrigger_involvedrelation` (quest area triggers) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadQuestTemplateCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Quest Templates...");
    sObjectMgr.LoadQuests();
    SendGlobalSysMessage("DB table `quest_template` (quest definitions) reloaded.");

    /// dependent also from `gameobject` but this table not reloaded anyway
    sLog.outString("Re-Loading GameObjects for quests...");
    sObjectMgr.LoadGameObjectForQuests();
    SendGlobalSysMessage("Data GameObjects for quests reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesCreatureCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`creature_loot_template`)");
    LoadLootTemplates_Creature();
    LootTemplates_Creature.CheckLootRefs();
    SendGlobalSysMessage("DB table `creature_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesDisenchantCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`disenchant_loot_template`)");
    LoadLootTemplates_Disenchant();
    LootTemplates_Disenchant.CheckLootRefs();
    SendGlobalSysMessage("DB table `disenchant_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesFishingCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`fishing_loot_template`)");
    LoadLootTemplates_Fishing();
    LootTemplates_Fishing.CheckLootRefs();
    SendGlobalSysMessage("DB table `fishing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesGameobjectCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`gameobject_loot_template`)");
    LoadLootTemplates_Gameobject();
    LootTemplates_Gameobject.CheckLootRefs();
    SendGlobalSysMessage("DB table `gameobject_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesItemCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`item_loot_template`)");
    LoadLootTemplates_Item();
    LootTemplates_Item.CheckLootRefs();
    SendGlobalSysMessage("DB table `item_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesPickpocketingCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`pickpocketing_loot_template`)");
    LoadLootTemplates_Pickpocketing();
    LootTemplates_Pickpocketing.CheckLootRefs();
    SendGlobalSysMessage("DB table `pickpocketing_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesMailCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`mail_loot_template`)");
    LoadLootTemplates_Mail();
    LootTemplates_Mail.CheckLootRefs();
    SendGlobalSysMessage("DB table `mail_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesReferenceCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`reference_loot_template`)");
    LootIdSet ids_set;
    LoadLootTemplates_Reference(ids_set);
    CheckLootTemplates_Reference(ids_set);
    SendGlobalSysMessage("DB table `reference_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLootTemplatesSkinningCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Loot Tables... (`skinning_loot_template`)");
    LoadLootTemplates_Skinning();
    LootTemplates_Skinning.CheckLootRefs();
    SendGlobalSysMessage("DB table `skinning_loot_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadMangosStringCommand(char* /*args*/)
{
    sLog.outString("Re-Loading mangos_string Table!");
    sObjectMgr.LoadMangosStrings();
    SendGlobalSysMessage("DB table `mangos_string` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcGossipCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `npc_gossip` Table!");
    sObjectMgr.LoadNpcGossips();
    SendGlobalSysMessage("DB table `npc_gossip` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcTextCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `npc_text` Table!");
    sObjectMgr.LoadGossipText();
    SendGlobalSysMessage("DB table `npc_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcTrainerCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `npc_trainer_template` Table!");
    sObjectMgr.LoadTrainerTemplates();
    SendGlobalSysMessage("DB table `npc_trainer_template` reloaded.");

    sLog.outString("Re-Loading `npc_trainer` Table!");
    sObjectMgr.LoadTrainers();
    SendGlobalSysMessage("DB table `npc_trainer` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadNpcVendorCommand(char* /*args*/)
{
    // not safe reload vendor template tables independent...
    sLog.outString("Re-Loading `npc_vendor_template` Table!");
    sObjectMgr.LoadVendorTemplates();
    SendGlobalSysMessage("DB table `npc_vendor_template` reloaded.");

    sLog.outString("Re-Loading `npc_vendor` Table!");
    sObjectMgr.LoadVendors();
    SendGlobalSysMessage("DB table `npc_vendor` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadPointsOfInterestCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `points_of_interest` Table!");
    sObjectMgr.LoadPointsOfInterest();
    SendGlobalSysMessage("DB table `points_of_interest` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReservedNameCommand(char* /*args*/)
{
    sLog.outString("Loading ReservedNames... (`reserved_name`)");
    sObjectMgr.LoadReservedPlayersNames();
    SendGlobalSysMessage("DB table `reserved_name` (player reserved names) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReputationRewardRateCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `reputation_reward_rate` Table!");
    sObjectMgr.LoadReputationRewardRate();
    SendGlobalSysMessage("DB table `reputation_reward_rate` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadReputationSpilloverTemplateCommand(char* /*args*/)
{
    sLog.outString("Re-Loading `reputation_spillover_template` Table!");
    sObjectMgr.LoadReputationSpilloverTemplate();
    SendGlobalSysMessage("DB table `reputation_spillover_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSkillFishingBaseLevelCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Skill Fishing base level requirements...");
    sObjectMgr.LoadFishingBaseSkillLevel();
    SendGlobalSysMessage("DB table `skill_fishing_base_level` (fishing base level for zone/subzone) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAffectCommand(char* /*args*/)
{
    sLog.outString("Re-Loading SpellAffect definitions...");
    sSpellMgr.LoadSpellAffects();
    SendGlobalSysMessage("DB table `spell_affect` (spell mods apply requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellAreaCommand(char* /*args*/)
{
    sLog.outString("Re-Loading SpellArea Data...");
    sSpellMgr.LoadSpellAreas();
    SendGlobalSysMessage("DB table `spell_area` (spell dependences from area/quest/auras state) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellChainCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell Chain Data... ");
    sSpellMgr.LoadSpellChains();
    SendGlobalSysMessage("DB table `spell_chain` (spell ranks) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellElixirCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell Elixir types...");
    sSpellMgr.LoadSpellElixirs();
    SendGlobalSysMessage("DB table `spell_elixir` (spell elixir types) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellLearnSpellCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell Learn Spells...");
    sSpellMgr.LoadSpellLearnSpells();
    SendGlobalSysMessage("DB table `spell_learn_spell` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellProcEventCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell Proc Event conditions...");
    sSpellMgr.LoadSpellProcEvents();
    SendGlobalSysMessage("DB table `spell_proc_event` (spell proc trigger requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellProcItemEnchantCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell Proc Item Enchant...");
    sSpellMgr.LoadSpellProcItemEnchant();
    SendGlobalSysMessage("DB table `spell_proc_item_enchant` (item enchantment ppm) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellScriptTargetCommand(char* /*args*/)
{
    sLog.outString("Re-Loading SpellsScriptTarget...");
    sSpellMgr.LoadSpellScriptTarget();
    SendGlobalSysMessage("DB table `spell_script_target` (spell targets selection in case specific creature/GO requirements) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellTargetPositionCommand(char* /*args*/)
{
    sLog.outString("Re-Loading spell target destination coordinates...");
    sSpellMgr.LoadSpellTargetPositions();
    SendGlobalSysMessage("DB table `spell_target_position` (destination coordinates for spell targets) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellThreatsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Aggro Spells Definitions...");
    sSpellMgr.LoadSpellThreats();
    SendGlobalSysMessage("DB table `spell_threat` (spell aggro definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadStringIds(char* /*args*/)
{
    sLog.outString("Re-Loading String Id Definitions...");
    sScriptMgr.LoadStringIds();
    SendGlobalSysMessage("DB table `string_id` (string id definitions) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadTaxiShortcuts(char* /*args*/)
{
    sLog.outString("Re-Loading taxi flight shortcuts...");
    sObjectMgr.LoadTaxiShortcuts();
    SendGlobalSysMessage("DB table `taxi_shortcuts` (taxi flight shortcuts information) reloaded.");
    return true;
}

bool ChatHandler::HandleReloadSpellPetAurasCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Spell pet auras...");
    sSpellMgr.LoadSpellPetAuras();
    SendGlobalSysMessage("DB table `spell_pet_auras` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadPageTextsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Page Texts...");
    sObjectMgr.LoadPageTexts();
    SendGlobalSysMessage("DB table `page_texts` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemEnchantementsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Item Random Enchantments Table...");
    LoadRandomEnchantmentsTable();
    SendGlobalSysMessage("DB table `item_enchantment_template` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadItemRequiredTragetCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Item Required Targets Table...");
    sObjectMgr.LoadItemRequiredTarget();
    SendGlobalSysMessage("DB table `item_required_target` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadBattleEventCommand(char* /*args*/)
{
    sLog.outString("Re-Loading BattleGround Eventindexes...");
    sBattleGroundMgr.GetMessager().AddMessage([](BattleGroundMgr* mgr)
    {
        mgr->LoadBattleEventIndexes(true);
    });
    SendGlobalSysMessage("DB table `gameobject_battleground` and `creature_battleground` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadEventAISummonsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Summons from `creature_ai_summons`...");
    sEventAIMgr.LoadCreatureEventAI_Summons(true);
    SendGlobalSysMessage("DB table `creature_ai_summons` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadEventAIScriptsCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Scripts from `creature_ai_scripts`...");
    sEventAIMgr.LoadCreatureEventAI_Scripts();
    SendGlobalSysMessage("DB table `creature_ai_scripts` reloaded.");
    auto containerEntry = sEventAIMgr.GetCreatureEventEntryAIMap();
    auto containerGuid = sEventAIMgr.GetCreatureEventGuidAIMap();
    auto containerComputed = sEventAIMgr.GetEAIComputedDataMap();
    sMapMgr.DoForAllMaps([containerEntry, containerGuid, containerComputed](Map* map)
    {
        map->GetMessager().AddMessage([containerEntry, containerGuid, containerComputed](Map* map)
        {
            map->GetMapDataContainer().SetEventAIContainers(containerEntry, containerGuid, containerComputed);
        });
    });
    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnGossipCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_gossip`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_GOSSIP, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_gossip` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnSpellCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_spell`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_SPELL, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_spell` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnQuestStartCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_quest_start`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_QUEST_START, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_quest_start` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnQuestEndCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_quest_end`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_QUEST_END, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_quest_end` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnEventCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_event`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_EVENT, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_event` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnGoUseCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_go[_template]_use`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_GAMEOBJECT, true);
    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_GAMEOBJECT_TEMPLATE, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_go[_template]_use` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnCreatureDeathCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_creature_death`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_CREATURE_DEATH, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_creature_death` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadDBScriptsOnRelayCommand(char* args)
{
    if (*args != 'a')
        sLog.outString("Re-Loading Scripts from `dbscripts_on_relay`...");

    sScriptMgr.LoadScriptMap(SCRIPT_TYPE_RELAY, true);

    if (*args != 'a')
        SendGlobalSysMessage("DB table `dbscripts_on_relay` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameGraveyardZoneCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Graveyard-zone links...");

    sWorld.LoadGraveyardZones();

    SendGlobalSysMessage("DB table `game_graveyard_zone` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadGameTeleCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Game Tele coordinates...");

    sObjectMgr.LoadGameTele();

    SendGlobalSysMessage("DB table `game_tele` reloaded.");

    return true;
}

bool ChatHandler::HandleReloadLocalesCreatureCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Creature ...");
    sObjectMgr.LoadCreatureLocales();
    SendGlobalSysMessage("DB table `locales_creature` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGameobjectCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Gameobject ... ");
    sObjectMgr.LoadGameObjectLocales();
    SendGlobalSysMessage("DB table `locales_gameobject` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesGossipMenuOptionCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Gossip Menu Option ... ");
    sObjectMgr.LoadGossipMenuItemsLocales();
    SendGlobalSysMessage("DB table `locales_gossip_menu_option` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesItemCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Item ... ");
    sObjectMgr.LoadItemLocales();
    SendGlobalSysMessage("DB table `locales_item` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesNpcTextCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales NPC Text ... ");
    sObjectMgr.LoadGossipTextLocales();
    SendGlobalSysMessage("DB table `locales_npc_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesPageTextCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Page Text ... ");
    sObjectMgr.LoadPageTextLocales();
    SendGlobalSysMessage("DB table `locales_page_text` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesPointsOfInterestCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Points Of Interest ... ");
    sObjectMgr.LoadPointOfInterestLocales();
    SendGlobalSysMessage("DB table `locales_points_of_interest` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadLocalesQuestCommand(char* /*args*/)
{
    sLog.outString("Re-Loading Locales Quest ... ");
    sObjectMgr.LoadQuestLocales();
    SendGlobalSysMessage("DB table `locales_quest` reloaded.");
    return true;
}

bool ChatHandler::HandleReloadExpectedSpamRecords(char* /*args*/)
{
    sLog.outString("Reloading expected spam records...");
    sWorld.LoadSpamRecords(true);
    SendGlobalSysMessage("Reloaded expected spam records.");
    return true;
}

bool ChatHandler::HandleReloadCreatureCooldownsCommand(char* /*args*/)
{
    sLog.outString("Reloading creature cooldowns...");
    sObjectMgr.LoadCreatureCooldowns();
    SendGlobalSysMessage("Reloaded creature cooldowns.");
    return true;
}

bool ChatHandler::HandleReloadCreatureSpellLists(char* /*args*/)
{
    sLog.outString("Reloading creature spell lists...");
    auto conditionsAndExpressions = sObjectMgr.LoadConditionsAndExpressions();
    auto result = sObjectMgr.LoadCreatureSpellLists();
    sObjectMgr.LoadCreatureTemplateSpells(result);
    auto [unitConditions, worldstateExpressions, combatConditions] = conditionsAndExpressions;
    SendGlobalSysMessage("Reloaded creature spell lists.");
    if (result)
    {
        sMapMgr.DoForAllMaps([result](Map* map)
        {
            map->GetMessager().AddMessage([result](Map* map)
            {
                map->GetMapDataContainer().SetCreatureSpellListContainer(result);
            });
        });
    }
    return true;
}

bool ChatHandler::HandleReloadSpawnGroupsCommand(char* /*args*/)
{
    sLog.outString("Reloading spawn groups...");
    sObjectMgr.LoadSpawnGroups();
    SendGlobalSysMessage("Reloaded spawn groups.");
    return true;
}

bool ChatHandler::HandleLoadScriptsCommand(char* args)
{
    return *args != 0;
}

bool ChatHandler::HandleAccountSetGmLevelCommand(char* args)
{
    char* accountStr = ExtractOptNotLastArg(&args);

    std::string targetAccountName;
    Player* targetPlayer = nullptr;
    uint32 targetAccountId = ExtractAccountId(&accountStr, &targetAccountName, &targetPlayer);
    if (!targetAccountId)
        return false;

    /// only target player different from self allowed
    if (GetAccountId() == targetAccountId)
        return false;

    int32 gm;
    if (!ExtractInt32(&args, gm))
        return false;

    if (gm < SEC_PLAYER || gm > SEC_ADMINISTRATOR)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    /// can set security level only for target with less security and to less security that we have
    /// This will reject self apply by specify account name
    if (HasLowerSecurityAccount(nullptr, targetAccountId, true))
        return false;

    /// account can't set security to same or grater level, need more power GM or console
    AccountTypes plSecurity = GetAccessLevel();
    if (AccountTypes(gm) >= plSecurity)
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return false;
    }

    if (targetPlayer)
    {
        ChatHandler(targetPlayer).PSendSysMessage(LANG_YOURS_SECURITY_CHANGED, GetNameLink().c_str(), gm);
        targetPlayer->GetSession()->SetSecurity(AccountTypes(gm));
    }

    PSendSysMessage(LANG_YOU_CHANGE_SECURITY, targetAccountName.c_str(), gm);
    LoginDatabase.PExecute("UPDATE account SET gmlevel = '%i' WHERE id = '%u'", gm, targetAccountId);

    return true;
}

/// Set password for account
bool ChatHandler::HandleAccountSetPasswordCommand(char* args)
{
    ///- Get the command line arguments
    std::string account_name;
    uint32 targetAccountId = ExtractAccountId(&args, &account_name);
    if (!targetAccountId)
        return false;

    // allow or quoted string with possible spaces or literal without spaces
    char* szPassword1 = ExtractQuotedOrLiteralArg(&args);
    char* szPassword2 = ExtractQuotedOrLiteralArg(&args);
    if (!szPassword1 || !szPassword2)
        return false;

    /// can set password only for target with less security
    /// This is also reject self apply in fact
    if (HasLowerSecurityAccount(nullptr, targetAccountId, true))
        return false;

    if (strcmp(szPassword1, szPassword2))
    {
        SendSysMessage(LANG_NEW_PASSWORDS_NOT_MATCH);
        SetSentErrorMessage(true);
        return false;
    }

    AccountOpResult result = sAccountMgr.ChangePassword(targetAccountId, szPassword1);

    switch (result)
    {
        case AOR_OK:
            SendSysMessage(LANG_COMMAND_PASSWORD);
            break;
        case AOR_NAME_NOT_EXIST:
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
            SetSentErrorMessage(true);
            return false;
        case AOR_PASS_TOO_LONG:
            SendSysMessage(LANG_PASSWORD_TOO_LONG);
            SetSentErrorMessage(true);
            return false;
        default:
            SendSysMessage(LANG_COMMAND_NOTCHANGEPASSWORD);
            SetSentErrorMessage(true);
            return false;
    }

    // OK, but avoid normal report for hide passwords, but log use command for anyone
    char msg[100];
    snprintf(msg, 100, ".account set password %s *** ***", account_name.c_str());
    LogCommand(msg);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleMaxSkillCommand(char* /*args*/)
{
    Player* SelectedPlayer = getSelectedPlayer();
    if (!SelectedPlayer)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // each skills that have max skill value dependent from level seted to current level max skill value
    SelectedPlayer->UpdateSkillsForLevel(true);
    return true;
}

bool ChatHandler::HandleSetSkillCommand(char* args)
{
    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hskill:skill_id|h[name]|h|r
    char* skill_p = ExtractKeyFromLink(&args, "Hskill");
    if (!skill_p)
        return false;

    int32 skill;
    if (!ExtractInt32(&skill_p, skill))
        return false;

    int32 level;
    if (!ExtractInt32(&args, level))
        return false;

    int32 maxskill;
    if (!ExtractOptInt32(&args, maxskill, target->GetSkillMaxPure(skill)))
        return false;

    if (skill <= 0)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    SkillLineEntry const* sl = sSkillLineStore.LookupEntry(skill);
    if (!sl)
    {
        PSendSysMessage(LANG_INVALID_SKILL_ID, skill);
        SetSentErrorMessage(true);
        return false;
    }

    std::string tNameLink = GetNameLink(target);

    if (!target->GetSkillValue(skill))
    {
        PSendSysMessage(LANG_SET_SKILL_ERROR, tNameLink.c_str(), skill, sl->name[GetSessionDbcLocale()]);
        SetSentErrorMessage(true);
        return false;
    }

    if (level <= 0 || level > maxskill || maxskill <= 0)
        return false;

    target->SetSkill(skill, level, maxskill);
    PSendSysMessage(LANG_SET_SKILL, skill, sl->name[GetSessionDbcLocale()], tNameLink.c_str(), level, maxskill);

    return true;
}

bool ChatHandler::HandleUnLearnCommand(char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    uint32 spell_id = ExtractSpellIdFromLink(&args);
    if (!spell_id)
        return false;

    bool allRanks = ExtractLiteralArg(&args, "all") != nullptr;
    if (!allRanks && *args)                                 // can be fail also at syntax error
        return false;

    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (allRanks)
        spell_id = sSpellMgr.GetFirstSpellInChain(spell_id);

    if (target->HasSpell(spell_id))
        target->removeSpell(spell_id, false, !allRanks);
    else
        SendSysMessage(LANG_FORGET_SPELL);

    return true;
}

bool ChatHandler::HandleCooldownListCommand(char* /*args*/)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }


    target->PrintCooldownList(*this);
    return true;
}

bool ChatHandler::HandleCooldownClearCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    std::string tNameLink = "Unknown";
    if (target->GetTypeId() == TYPEID_PLAYER)
        tNameLink = GetNameLink(static_cast<Player*>(target));
    else
        tNameLink = target->GetName();

    if (!*args)
    {
        target->RemoveAllCooldowns();
        PSendSysMessage(LANG_REMOVEALL_COOLDOWN, tNameLink.c_str());
    }
    else
    {
        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spell_id = ExtractSpellIdFromLink(&args);
        if (!spell_id)
            return false;

        SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(spell_id);
        if (!spellEntry)
        {
            PSendSysMessage(LANG_UNKNOWN_SPELL, target == m_session->GetPlayer() ? GetMangosString(LANG_YOU) : tNameLink.c_str());
            SetSentErrorMessage(true);
            return false;
        }

        target->RemoveSpellCooldown(*spellEntry);
        PSendSysMessage(LANG_REMOVE_COOLDOWN, spell_id, target == m_session->GetPlayer() ? GetMangosString(LANG_YOU) : tNameLink.c_str());
    }
    return true;
}

bool ChatHandler::HandleCooldownClearClientSideCommand(char*)
{
    Player* target = getSelectedPlayer();
    if (!target)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    std::string tNameLink = GetNameLink(target);

    target->RemoveAllCooldowns(true);
    PSendSysMessage(LANG_REMOVEALL_COOLDOWN, tNameLink.c_str());
    return true;
}

bool ChatHandler::HandleLearnAllCommand(char* /*args*/)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    for (uint32 i = 0; i < sSpellTemplate.GetMaxEntry(); i++)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(i);
        if (!spellInfo)
            continue;

        for (uint32 j = 0; j < MAX_EFFECT_INDEX; j++)
        {
            if ((spellInfo->Effect[j] == SPELL_EFFECT_LEARN_SPELL) &&
                (spellInfo->EffectImplicitTargetA[j] == TARGET_NONE))
            {
                uint32 spellId = spellInfo->EffectTriggerSpell[j];
                SpellEntry const* newSpell = sSpellTemplate.LookupEntry<SpellEntry>(spellId);

                // skip broken spells
                if (!SpellMgr::IsSpellValid(newSpell, player, false))
                    continue;

                // skip spells with first rank learned as talent (and all talents then also)
                uint32 firstRankId = sSpellMgr.GetFirstSpellInChain(spellId);
                if (GetTalentSpellCost(firstRankId) > 0)
                    continue;
                
                if (!IsSpellHaveEffect(newSpell, SPELL_EFFECT_PROFICIENCY))
                {
                    // only class spells
                    if (!newSpell->SpellFamilyName)
                        continue;

                    // skip passives
                    if (newSpell->HasAttribute(SPELL_ATTR_PASSIVE) ||
                        newSpell->HasAttribute(SPELL_ATTR_DO_NOT_DISPLAY) ||
                        newSpell->HasAttribute(SPELL_ATTR_EX2_USE_SHAPESHIFT_BAR))
                        continue;
                }

                player->learnSpell(spellId, false);
            }
        }
    }

    SendSysMessage(LANG_COMMAND_LEARN_MANY_SPELLS);

    return true;
}

bool ChatHandler::HandleLearnAllGMCommand(char* /*args*/)
{
    static uint32 gmSpellList[] =
    {
        11,     // Frostbolt of Ages                1000 damage, no cooldown
        13,     // Swim Speed (TEST)
        26,     // Bind Self (TEST)                 sets hearthstone to current location
        47,     // Sprint (TEST)
        260,    // Charm (TEST)
        265,    // Area Death (TEST)
        530,    // Charm (Possess)
        1908,   // Uber Heal Over Time
        2583,   // Debug Spell Reflection (Debug)
        2650,   // Tame Pet (TEST)
        2653,   // Damage 100 (TEST)
        2654,   // Summon Tamed (TEST)
        5259,   // Disarm (TEST)
        5696,   // Charge (TEST)
        9454,   // Freeze                           permanent stun, useful for holding misbehaving players
        10032,  // Uber Stealth
        18800,  // Light Test
        18209,  // Test Grow
        18210,  // Test Shrink
        23452,  // Invisibility
        23775,  // Stun Forever
        24199,  // Knockback 35
        0
    };

    for (uint32 spell : gmSpellList)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer()))
        {
            PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
            continue;
        }

        m_session->GetPlayer()->learnSpell(spell, false);
    }

    SendSysMessage(LANG_LEARNING_GM_SKILLS);
    return true;
}

bool ChatHandler::HandleLearnAllMyClassCommand(char* /*args*/)
{
    HandleLearnAllMySpellsCommand((char*)"");
    HandleLearnAllMyTalentsCommand((char*)"");
    return true;
}

bool ChatHandler::HandleLearnAllMySpellsCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    ChrClassesEntry const* clsEntry = sChrClassesStore.LookupEntry(player->getClass());
    if (!clsEntry)
        return true;
    uint32 family = clsEntry->spellfamily;

    for (uint32 i = 0; i < sSkillLineAbilityStore.GetNumRows(); ++i)
    {
        SkillLineAbilityEntry const* entry = sSkillLineAbilityStore.LookupEntry(i);
        if (!entry)
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(entry->spellId);
        if (!spellInfo)
            continue;

        // skip server-side/triggered spells
        if (spellInfo->spellLevel == 0)
            continue;

        // skip wrong class/race skills
        if (!player->IsSpellFitByClassAndRace(spellInfo->Id))
            continue;

        // skip other spell families
        if (spellInfo->SpellFamilyName != family)
            continue;

        // skip spells with first rank learned as talent (and all talents then also)
        uint32 first_rank = sSpellMgr.GetFirstSpellInChain(spellInfo->Id);
        if (GetTalentSpellCost(first_rank) > 0)
            continue;

        // skip broken spells
        if (!SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;

        player->learnSpell(spellInfo->Id, false);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_SPELLS);
    return true;
}

bool ChatHandler::HandleLearnAllMyTalentsCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    uint32 classMask = player->getClassMask();

    for (uint32 i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo)
            continue;

        TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentInfo->TalentTab);
        if (!talentTabInfo)
            continue;

        if ((classMask & talentTabInfo->ClassMask) == 0)
            continue;

        // search highest talent rank
        uint32 spellid = 0;

        for (int rank = MAX_TALENT_RANK - 1; rank >= 0; --rank)
        {
            if (talentInfo->RankID[rank] != 0)
            {
                spellid = talentInfo->RankID[rank];
                break;
            }
        }

        if (!spellid)                                       // ??? none spells in talent
            continue;

        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
        if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player, false))
            continue;

        // learn highest rank of talent and learn all non-talent spell ranks (recursive by tree)
        player->learnSpellHighRank(spellid);
    }

    SendSysMessage(LANG_COMMAND_LEARN_CLASS_TALENTS);
    return true;
}

bool ChatHandler::HandleLearnAllLangCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();

    // skipping UNIVERSAL language (0)
    for (int i = 1; i < LANGUAGES_COUNT; ++i)
        player->learnSpell(lang_description[i].spell_id, false);

    SendSysMessage(LANG_COMMAND_LEARN_ALL_LANG);
    return true;
}

bool ChatHandler::HandleLearnAllDefaultCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    target->LearnDefaultSkills();
    target->learnDefaultSpells();
    target->learnQuestRewardedSpells();

    PSendSysMessage(LANG_COMMAND_LEARN_ALL_DEFAULT_AND_QUEST, GetNameLink(target).c_str());
    return true;
}

bool ChatHandler::HandleLearnCommand(char* args)
{
    Player* player = m_session->GetPlayer();
    Player* targetPlayer = getSelectedPlayer();

    if (!targetPlayer)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    bool allRanks = ExtractLiteralArg(&args, "all") != nullptr;
    if (!allRanks && *args)                                 // can be fail also at syntax error
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo || !SpellMgr::IsSpellValid(spellInfo, player))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
        SetSentErrorMessage(true);
        return false;
    }

    if (!allRanks && targetPlayer->HasSpell(spell))
    {
        if (targetPlayer == player)
            SendSysMessage(LANG_YOU_KNOWN_SPELL);
        else
            PSendSysMessage(LANG_TARGET_KNOWN_SPELL, targetPlayer->GetName());
        SetSentErrorMessage(true);
        return false;
    }

    if (allRanks)
        targetPlayer->learnSpellHighRank(spell);
    else
        targetPlayer->learnSpell(spell, false);

    return true;
}

bool ChatHandler::HandleAddItemCommand(char* args)
{
    char* cId = ExtractKeyFromLink(&args, "Hitem");
    if (!cId)
        return false;

    uint32 itemId = 0;
    if (!ExtractUInt32(&cId, itemId))                       // [name] manual form
    {
        std::string itemName = cId;
        WorldDatabase.escape_string(itemName);
        auto queryResult = WorldDatabase.PQuery("SELECT entry FROM item_template WHERE name = '%s'", itemName.c_str());
        if (!queryResult)
        {
            PSendSysMessage(LANG_COMMAND_COULDNOTFIND, cId);
            SetSentErrorMessage(true);
            return false;
        }
        itemId = queryResult->Fetch()->GetUInt16();
    }

    int32 count;
    if (!ExtractOptInt32(&args, count, 1))
        return false;

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if (!plTarget)
        plTarget = pl;

    DETAIL_LOG(GetMangosString(LANG_ADDITEM), itemId, count);

    ItemPrototype const* pProto = ObjectMgr::GetItemPrototype(itemId);
    if (!pProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, itemId);
        SetSentErrorMessage(true);
        return false;
    }

    // Subtract
    if (count < 0)
    {
        plTarget->DestroyItemCount(itemId, -count, true, true);
        PSendSysMessage(LANG_REMOVEITEM, itemId, -count, GetNameLink(plTarget).c_str());
        return true;
    }

    // Adding items
    uint32 noSpaceForCount = 0;

    // check space and find places
    ItemPosCountVec dest;
    uint8 msg = plTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, itemId, count, &noSpaceForCount);
    if (msg != EQUIP_ERR_OK)                                // convert to possible store amount
        count -= noSpaceForCount;

    if (count == 0 || dest.empty())                         // can't add any
    {
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);
        SetSentErrorMessage(true);
        return false;
    }

    Item* item = plTarget->StoreNewItem(dest, itemId, true, Item::GenerateItemRandomPropertyId(itemId));

    // remove binding (let GM give it to another player later)
    if (pl == plTarget)
        for (ItemPosCountVec::const_iterator itr = dest.begin(); itr != dest.end(); ++itr)
            if (Item* item1 = pl->GetItemByPos(itr->pos))
                item1->SetBinding(false);

    if (count > 0 && item)
    {
        pl->SendNewItem(item, count, false, true);
        if (pl != plTarget)
            plTarget->SendNewItem(item, count, true, false);
    }

    if (noSpaceForCount > 0)
        PSendSysMessage(LANG_ITEM_CANNOT_CREATE, itemId, noSpaceForCount);

    return true;
}

bool ChatHandler::HandleAddItemSetCommand(char* args)
{
    uint32 itemsetId;
    if (!ExtractUint32KeyFromLink(&args, "Hitemset", itemsetId))
        return false;

    // prevent generation all items with itemset field value '0'
    if (itemsetId == 0)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);
        SetSentErrorMessage(true);
        return false;
    }

    Player* pl = m_session->GetPlayer();
    Player* plTarget = getSelectedPlayer();
    if (!plTarget)
        plTarget = pl;

    DETAIL_LOG(GetMangosString(LANG_ADDITEMSET), itemsetId);

    bool found = false;
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const* pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
        if (!pProto)
            continue;

        if (pProto->ItemSet == itemsetId)
        {
            found = true;
            ItemPosCountVec dest;
            InventoryResult msg = plTarget->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, pProto->ItemId, 1);
            if (msg == EQUIP_ERR_OK)
            {
                Item* item = plTarget->StoreNewItem(dest, pProto->ItemId, true);

                // remove binding (let GM give it to another player later)
                if (pl == plTarget)
                    item->SetBinding(false);

                pl->SendNewItem(item, 1, false, true);
                if (pl != plTarget)
                    plTarget->SendNewItem(item, 1, true, false);
            }
            else
            {
                pl->SendEquipError(msg, nullptr, nullptr, pProto->ItemId);
                PSendSysMessage(LANG_ITEM_CANNOT_CREATE, pProto->ItemId, 1);
            }
        }
    }

    if (!found)
    {
        PSendSysMessage(LANG_NO_ITEMS_FROM_ITEMSET_FOUND, itemsetId);

        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleListItemCommand(char* args)
{
    uint32 item_id;
    if (!ExtractUint32KeyFromLink(&args, "Hitem", item_id))
        return false;

    if (!item_id)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    ItemPrototype const* itemProto = ObjectMgr::GetItemPrototype(item_id);
    if (!itemProto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 count;
    if (!ExtractOptUInt32(&args, count, 10))
        return false;

    // inventory case
    uint32 inv_count = 0;
    auto queryResult = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM character_inventory WHERE item_template='%u'", item_id);
    if (queryResult)
    {
        inv_count = (*queryResult)[0].GetUInt32();
    }

    queryResult = CharacterDatabase.PQuery(
                 //          0        1             2             3        4                  5
                 "SELECT ci.item, cibag.slot AS bag, ci.slot, ci.guid, characters.account,characters.name "
                 "FROM character_inventory AS ci LEFT JOIN character_inventory AS cibag ON (cibag.item=ci.bag),characters "
                 "WHERE ci.item_template='%u' AND ci.guid = characters.guid LIMIT %u ",
                 item_id, uint32(count));

    if (queryResult)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 item_guid = fields[0].GetUInt32();
            uint32 item_bag = fields[1].GetUInt32();
            uint32 item_slot = fields[2].GetUInt32();
            uint32 owner_guid = fields[3].GetUInt32();
            uint32 owner_acc = fields[4].GetUInt32();
            std::string owner_name = fields[5].GetCppString();

            char const* item_pos;
            if (Player::IsEquipmentPos(item_bag, item_slot))
                item_pos = "[equipped]";
            else if (Player::IsInventoryPos(item_bag, item_slot))
                item_pos = "[in inventory]";
            else if (Player::IsBankPos(item_bag, item_slot))
                item_pos = "[in bank]";
            else
                item_pos = "";

            PSendSysMessage(LANG_ITEMLIST_SLOT,
                            item_guid, owner_name.c_str(), owner_guid, owner_acc, item_pos);
        }
        while (queryResult->NextRow());

        uint32 res_count = uint32(queryResult->GetRowCount());

        if (count > res_count)
            count -= res_count;
        else if (count)
            count = 0;
    }

    // mail case
    uint32 mail_count = 0;
    queryResult = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM mail_items WHERE item_template='%u'", item_id);
    if (queryResult)
    {
        mail_count = (*queryResult)[0].GetUInt32();
    }

    if (count > 0)
    {
        queryResult = CharacterDatabase.PQuery(
                     //          0                     1            2              3               4            5               6
                     "SELECT mail_items.item_guid, mail.sender, mail.receiver, char_s.account, char_s.name, char_r.account, char_r.name "
                     "FROM mail,mail_items,characters as char_s,characters as char_r "
                     "WHERE mail_items.item_template='%u' AND char_s.guid = mail.sender AND char_r.guid = mail.receiver AND mail.id=mail_items.mail_id LIMIT %u",
                     item_id, uint32(count));
    }
    else
        queryResult.reset();

    if (queryResult)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 item_guid        = fields[0].GetUInt32();
            uint32 item_s           = fields[1].GetUInt32();
            uint32 item_r           = fields[2].GetUInt32();
            uint32 item_s_acc       = fields[3].GetUInt32();
            std::string item_s_name = fields[4].GetCppString();
            uint32 item_r_acc       = fields[5].GetUInt32();
            std::string item_r_name = fields[6].GetCppString();

            char const* item_pos = "[in mail]";

            PSendSysMessage(LANG_ITEMLIST_MAIL,
                            item_guid, item_s_name.c_str(), item_s, item_s_acc, item_r_name.c_str(), item_r, item_r_acc, item_pos);
        }
        while (queryResult->NextRow());

        uint32 res_count = uint32(queryResult->GetRowCount());

        if (count > res_count)
            count -= res_count;
        else if (count)
            count = 0;
    }

    // auction case
    uint32 auc_count = 0;
    queryResult = CharacterDatabase.PQuery("SELECT COUNT(item_template) FROM auction WHERE item_template='%u'", item_id);
    if (queryResult)
    {
        auc_count = (*queryResult)[0].GetUInt32();
    }

    if (count > 0)
    {
        queryResult = CharacterDatabase.PQuery(
                     //           0                      1                       2                   3
                     "SELECT  auction.itemguid, auction.itemowner, characters.account, characters.name "
                     "FROM auction,characters WHERE auction.item_template='%u' AND characters.guid = auction.itemowner LIMIT %u",
                     item_id, uint32(count));
    }
    else
        queryResult.reset();

    if (queryResult)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 item_guid       = fields[0].GetUInt32();
            uint32 owner           = fields[1].GetUInt32();
            uint32 owner_acc       = fields[2].GetUInt32();
            std::string owner_name = fields[3].GetCppString();

            char const* item_pos = "[in auction]";

            PSendSysMessage(LANG_ITEMLIST_AUCTION, item_guid, owner_name.c_str(), owner, owner_acc, item_pos);
        }
        while (queryResult->NextRow());
    }

    if (inv_count + mail_count + auc_count == 0)
    {
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage(LANG_COMMAND_LISTITEMMESSAGE, item_id, inv_count + mail_count + auc_count, inv_count, mail_count, auc_count);

    return true;
}

bool ChatHandler::HandleListObjectCommand(char* args)
{
    // number or [name] Shift-click form |color|Hgameobject_entry:go_id|h[name]|h|r
    uint32 go_id;
    if (!ExtractUint32KeyFromLink(&args, "Hgameobject_entry", go_id))
        return false;

    if (!go_id)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    GameObjectInfo const* gInfo = ObjectMgr::GetGameObjectInfo(go_id);
    if (!gInfo)
    {
        PSendSysMessage(LANG_COMMAND_LISTOBJINVALIDID, go_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session ? m_session->GetPlayer() : nullptr;

    static const uint32 MaxResult = 200; // guessed value for maximum result

    uint32 count = MaxResult;

    bool restrictZone = false;
    bool restrictArea = false;
    bool restrictMap = false;
    bool noRestriction = false;
    uint32 playerZoneId = 0;
    uint32 playerAreaId = 0;

    char* prevArgs = args;

    char const* za = ExtractLiteralArg(&args);
    if (za == nullptr || za[0] == 'w' || za[0] == 'W')
        noRestriction = true;
    else
    {
        if (za[0] == 'a' || za[0] == 'A') // area filter
        {
            restrictArea = true;
        }
        else if (za[0] == 'z' || za[0] == 'Z') // zone filter
        {
            restrictZone = true;
        }
        else if (za[0] == 'm' || za[0] == 'M') // map filter
        {
            restrictMap = true;
        }
        else
        {
            restrictZone = true;
            args = prevArgs;
        }
    }

    if (za != nullptr)
        ExtractOptUInt32(&args, count, MaxResult);

    if (player)
        player->GetTerrain()->GetZoneAndAreaId(playerZoneId, playerAreaId, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

    static const std::string QueryTablesName[] = {
        "gameobject",
        "gameobject_spawn_entry",
        "spawn_group_entry"
    };

    // temporary struct to hold gameobject data
    struct TempGobData
    {
        uint32 guid;
        float x;
        float y;
        float z;
        int mapid;
        float dist;
        uint32 originTable;
        bool operator < (const TempGobData& other) const { return dist < other.dist; }
    };

    std::set<TempGobData> tempData;
    std::unique_ptr<QueryResult> queryResult;
    uint32 counter = 0;
    uint32 worldCounter = 0;
    uint32 zoneCounter = 0;
    uint32 areaCounter = 0;
    uint32 mapCounter = 0;
    uint32 queryTableNameIndex = 0;

    // this lambda just fill tempData with request data (expect guid, position_x, position_y, position_z, map) in that order!
    // it will handle zone, area, map and sort result by distance from player
    auto AddPlayerData = [&]()
    {
        do
        {
            Field* fields = queryResult->Fetch();
            TempGobData data;
            data.guid = fields[0].GetUInt32();
            data.x = fields[1].GetFloat();
            data.y = fields[2].GetFloat();
            data.z = fields[3].GetFloat();
            data.mapid = fields[4].GetUInt16();
            data.originTable = queryTableNameIndex;
            data.dist = std::pow(data.x - player->GetPositionX(), 2) + std::pow(data.y - player->GetPositionY(), 2) + std::pow(data.z - player->GetPositionZ(), 2);

            uint32 objZoneId = 0;
            uint32 objAreaId = 0;
            player->GetTerrain()->GetZoneAndAreaId(objZoneId, objAreaId, data.x, data.y, data.z);
            if (player->GetMapId() == data.mapid)
            {
                if (objZoneId == playerZoneId)
                {
                    zoneCounter++;
                    if (restrictZone)
                        tempData.emplace(data);
                }

                if (objAreaId == playerAreaId)
                {
                    areaCounter++;
                    if (restrictArea)
                        tempData.emplace(data);
                }

                mapCounter++;
                if (restrictMap)
                    tempData.emplace(data);
            }

            if (noRestriction)
                tempData.emplace(data);

            worldCounter++;
        } while (queryResult->NextRow());
    };

    // this lambda just fill tempData with request data (expect guid, position_x, position_y, position_z, map) in that order!
    // handle only list of the object in the world as no session and command is pretty limited to add wanted map/zone/area id
    auto AddSimpleData = [&]()
    {
        do
        {
            Field* fields = queryResult->Fetch();
            TempGobData data;
            data.guid = fields[0].GetUInt32();
            data.x = fields[1].GetFloat();
            data.y = fields[2].GetFloat();
            data.z = fields[3].GetFloat();
            data.mapid = fields[4].GetUInt16();
            data.originTable = queryTableNameIndex;
            data.dist = counter++;

            worldCounter++;
            tempData.emplace(data);

        } while (queryResult->NextRow());
    };

    // query gameobject
    queryResult = WorldDatabase.PQuery(
        "SELECT guid, position_x, position_y, position_z, map "
        "FROM gameobject "
        "WHERE id = '%u'",
        go_id);

    if (queryResult)
        player ? AddPlayerData() : AddSimpleData();

    // query gameobject_spawn_entry
    queryTableNameIndex = 1;
    queryResult = WorldDatabase.PQuery(
        "SELECT a.guid, b.position_x, b.position_y, b.position_z, b.map "
        "FROM gameobject_spawn_entry a LEFT JOIN gameobject b ON a.guid = b.guid "
        "WHERE a.entry = '%u'",
        go_id);

    if (queryResult)
        player ? AddPlayerData() : AddSimpleData();

    // query spawn_group_entry
    queryTableNameIndex = 2;
    queryResult = WorldDatabase.PQuery(
        "SELECT d.guid, d.position_x, d.position_y, d.position_z, d.map "
        "FROM spawn_group_entry a LEFT JOIN spawn_group b ON a.Id = b.Id LEFT JOIN spawn_group_spawn c ON a.ID = c.Id LEFT JOIN gameobject d ON c.Guid = d.guid "
        "WHERE a.Entry = '%u' AND b.Type = 1",
        go_id);

    if (queryResult)
        player ? AddPlayerData() : AddSimpleData();

    // send result to client
    if (tempData.empty())
        PSendSysMessage("Object not found!");
    else
    {
        uint32 counter = 0;
        for (auto const& gob : tempData)
        {
            std::string info = " - From `" + QueryTablesName[gob.originTable];
            info += "`";
            info += PrepareStringNpcOrGoSpawnInformation<GameObject>(gob.guid);
            if (player)
                PSendSysMessage(LANG_GO_LIST_CHAT, gob.guid, info.c_str(),
                    gob.guid, gInfo->name, gob.x, gob.y, gob.z, gob.mapid);
            else
                PSendSysMessage(LANG_GO_LIST_CONSOLE, gob.guid, info.c_str(),
                    gInfo->name, gob.x, gob.y, gob.z, gob.mapid);
            ++counter;
            if (counter >= count)
                break;
        }
        if (player)
        {
            PSendSysMessage("%u object[%u] found in the world!", worldCounter, go_id);
            PSendSysMessage("Total in your map : %u", mapCounter);
            PSendSysMessage("Total in your zone : %u", zoneCounter);
            PSendSysMessage("Total in your area : %u", areaCounter);
        }
        else
            PSendSysMessage("%u object[%u] found in the world!", worldCounter, go_id);
    }
    return true;
}

bool ChatHandler::HandleListCreatureCommand(char* args)
{
    // number or [name] Shift-click form |color|Hcreature_entry:creature_id|h[name]|h|r
    uint32 cr_id;
    if (!ExtractUint32KeyFromLink(&args, "Hcreature_entry", cr_id))
        return false;

    if (!cr_id)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cr_id);
    if (!cInfo)
    {
        PSendSysMessage(LANG_COMMAND_INVALIDCREATUREID, cr_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session ? m_session->GetPlayer() : nullptr;

    static const uint32 MaxResult = 200; // guessed value for maximum result

    uint32 count = MaxResult;

    bool restrictZone = false;
    bool restrictArea = false;
    bool restrictMap = false;
    bool noRestriction = false;
    uint32 playerZoneId = 0;
    uint32 playerAreaId = 0;

    char* prevArgs = args;

    char const* za = ExtractLiteralArg(&args);
    if (za == nullptr || za[0] == 'w' || za[0] == 'W')
        noRestriction = true;
    else
    {
        if (za[0] == 'a' || za[0] == 'A') // area filter
        {
            restrictArea = true;
        }
        else if (za[0] == 'z' || za[0] == 'Z') // zone filter
        {
            restrictZone = true;
        }
        else if (za[0] == 'm' || za[0] == 'M') // map filter
        {
            restrictMap = true;
        }
        else
        {
            restrictZone = true;
            args = prevArgs;
        }
    }

    if (za != nullptr)
        ExtractOptUInt32(&args, count, MaxResult);

    if (player)
        player->GetTerrain()->GetZoneAndAreaId(playerZoneId, playerAreaId, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ());

    static const std::string QueryTablesName[] = {
        "creature",
        "creature_spawn_entry",
        "spawn_group_entry"
    };

    // temporary struct to hold creatures data
    struct TempCreatureData
    {
        uint32 guid;
        float x;
        float y;
        float z;
        int mapid;
        float dist;
        uint32 originTable;
        bool operator < (const TempCreatureData& other) const { return dist < other.dist; }
    };

    std::set<TempCreatureData> tempData;
    std::unique_ptr<QueryResult> result;
    uint32 counter = 0;
    uint32 worldCounter = 0;
    uint32 zoneCounter = 0;
    uint32 areaCounter = 0;
    uint32 mapCounter = 0;
    uint32 queryTableNameIndex = 0;

    // this lambda just fill tempData with request data (expect guid, position_x, position_y, position_z, map) in that order!
    // it will handle zone, area, map and sort result by distance from player
    auto AddPlayerData = [&]()
    {
        do
        {
            Field* fields = result->Fetch();
            TempCreatureData data;
            data.guid = fields[0].GetUInt32();
            data.x = fields[1].GetFloat();
            data.y = fields[2].GetFloat();
            data.z = fields[3].GetFloat();
            data.mapid = fields[4].GetUInt16();
            data.originTable = queryTableNameIndex;
            data.dist = std::pow(data.x - player->GetPositionX(), 2) + std::pow(data.y - player->GetPositionY(), 2) + std::pow(data.z - player->GetPositionZ(), 2);

            uint32 objZoneId = 0;
            uint32 objAreaId = 0;
            player->GetTerrain()->GetZoneAndAreaId(objZoneId, objAreaId, data.x, data.y, data.z);
            if (player->GetMapId() == data.mapid)
            {
                if (objZoneId == playerZoneId)
                {
                    zoneCounter++;
                    if (restrictZone)
                        tempData.emplace(data);
                }

                if (objAreaId == playerAreaId)
                {
                    areaCounter++;
                    if (restrictArea)
                        tempData.emplace(data);
                }

                mapCounter++;
                if (restrictMap)
                    tempData.emplace(data);
            }

            if (noRestriction)
                tempData.emplace(data);

            worldCounter++;
        } while (result->NextRow());
    };

    // this lambda just fill tempData with request data (expect guid, position_x, position_y, position_z, map) in that order!
    // handle only list of the object in the world as no session and command is pretty limited to add wanted map/zone/area id
    auto AddSimpleData = [&]()
    {
        do
        {
            Field* fields = result->Fetch();
            TempCreatureData data;
            data.guid = fields[0].GetUInt32();
            data.x = fields[1].GetFloat();
            data.y = fields[2].GetFloat();
            data.z = fields[3].GetFloat();
            data.mapid = fields[4].GetUInt16();
            data.originTable = queryTableNameIndex;
            data.dist = counter++;

            worldCounter++;
            tempData.emplace(data);

        } while (result->NextRow());
    };

    // query creature
    result = WorldDatabase.PQuery(
        "SELECT guid, position_x, position_y, position_z, map "
        "FROM creature "
        "WHERE id = '%u'",
        cr_id);

    if (result)
        player ? AddPlayerData() : AddSimpleData();

    // query gameobject_spawn_entry
    queryTableNameIndex = 1;
    result = WorldDatabase.PQuery(
        "SELECT a.guid, b.position_x, b.position_y, b.position_z, b.map "
        "FROM creature_spawn_entry a LEFT JOIN creature b ON a.guid = b.guid "
        "WHERE a.entry = '%u'",
        cr_id);

    if (result)
        player ? AddPlayerData() : AddSimpleData();

    // query spawn_group_entry
    queryTableNameIndex = 2;
    result = WorldDatabase.PQuery(
        "SELECT d.guid, d.position_x, d.position_y, d.position_z, d.map "
        "FROM spawn_group_entry a LEFT JOIN spawn_group b ON a.Id = b.Id LEFT JOIN spawn_group_spawn c ON a.ID = c.Id LEFT JOIN creature d ON c.Guid = d.guid "
        "WHERE a.Entry = '%u' AND b.Type = 0",
        cr_id);

    if (result)
        player ? AddPlayerData() : AddSimpleData();

    // send result to client
    if (tempData.empty())
        PSendSysMessage("Creature not found!");
    else
    {
        uint32 counter = 0;
        for (auto const& crData : tempData)
        {
            std::string info = " - From `" + QueryTablesName[crData.originTable];
            info += "`";
            info += PrepareStringNpcOrGoSpawnInformation<GameObject>(crData.guid);
            if (player)
                PSendSysMessage(LANG_CREATURE_LIST_CHAT, crData.guid, info.c_str(),
                    crData.guid, cInfo->Name, crData.x, crData.y, crData.z, crData.mapid);
            else
                PSendSysMessage(LANG_CREATURE_LIST_CONSOLE, crData.guid, info.c_str(),
                    cInfo->Name, crData.x, crData.y, crData.z, crData.mapid);
            ++counter;
            if (counter >= count)
                break;
        }
        if (player)
        {
            PSendSysMessage("%u creature[%u] found in the world!", worldCounter, cr_id);
            PSendSysMessage("Total in your map : %u", mapCounter);
            PSendSysMessage("Total in your zone : %u", zoneCounter);
            PSendSysMessage("Total in your area : %u", areaCounter);
        }
        else
            PSendSysMessage("%u creature[%u] found in the world!", worldCounter, cr_id);
    }
    return true;
}


void ChatHandler::ShowItemListHelper(uint32 itemId, int loc_idx, Player* target /*=nullptr*/)
{
    ItemPrototype const* itemProto = sItemStorage.LookupEntry<ItemPrototype >(itemId);
    if (!itemProto)
        return;

    std::string name = itemProto->Name1;
    sObjectMgr.GetItemLocaleStrings(itemProto->ItemId, loc_idx, &name);

    char const* usableStr = "";

    if (target)
    {
        if (target->CanUseItem(itemProto))
            usableStr = GetMangosString(LANG_COMMAND_ITEM_USABLE);
    }

    if (m_session)
        PSendSysMessage(LANG_ITEM_LIST_CHAT, itemId, itemId, name.c_str(), usableStr);
    else
        PSendSysMessage(LANG_ITEM_LIST_CONSOLE, itemId, name.c_str(), usableStr);
}

bool ChatHandler::HandleLookupItemCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    wstrToLower(wnamepart);

    Player* pl = m_session ? m_session->GetPlayer() : nullptr;

    uint32 counter = 0;

    // Search in `item_template`
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const* pProto = sItemStorage.LookupEntry<ItemPrototype >(id);
        if (!pProto)
            continue;

        int loc_idx = GetSessionDbLocaleIndex();

        std::string name;                                   // "" for let later only single time check default locale name directly
        sObjectMgr.GetItemLocaleStrings(id, loc_idx, &name);
        if ((name.empty() || !Utf8FitTo(name, wnamepart)) && !Utf8FitTo(pProto->Name1, wnamepart))
            continue;

        ShowItemListHelper(id, loc_idx, pl);
        ++counter;
    }

    if (counter == 0)
        SendSysMessage(LANG_COMMAND_NOITEMFOUND);

    return true;
}

bool ChatHandler::HandleLookupItemSetCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in ItemSet.dbc
    for (uint32 id = 0; id < sItemSetStore.GetNumRows(); ++id)
    {
        ItemSetEntry const* set = sItemSetStore.LookupEntry(id);
        if (set)
        {
            int loc = GetSessionDbcLocale();
            std::string name = set->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == GetSessionDbcLocale())
                        continue;

                    name = set->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send item set in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_ITEMSET_LIST_CHAT, id, id, name.c_str(), localeNames[loc]);
                else
                    PSendSysMessage(LANG_ITEMSET_LIST_CONSOLE, id, name.c_str(), localeNames[loc]);
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOITEMSETFOUND);
    return true;
}

bool ChatHandler::HandleLookupSkillCommand(char* args)
{
    if (!*args)
        return false;

    // can be nullptr in console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in SkillLine.dbc
    for (uint32 id = 0; id < sSkillLineStore.GetNumRows(); ++id)
    {
        SkillLineEntry const* skillInfo = sSkillLineStore.LookupEntry(id);
        if (skillInfo)
        {
            int loc = GetSessionDbcLocale();
            std::string name = skillInfo->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == GetSessionDbcLocale())
                        continue;

                    name = skillInfo->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                char valStr[50] = "";
                char const* knownStr = "";
                if (target && target->HasSkill(id))
                {
                    knownStr = GetMangosString(LANG_KNOWN);
                    uint32 curValue = target->GetSkillValuePure(id);
                    uint32 maxValue  = target->GetSkillMaxPure(id);
                    uint32 permValue = target->GetSkillBonusPermanent(id);
                    uint32 tempValue = target->GetSkillBonusTemporary(id);

                    char const* valFormat = GetMangosString(LANG_SKILL_VALUES);
                    snprintf(valStr, 50, valFormat, curValue, maxValue, permValue, tempValue);
                }

                // send skill in "id - [namedlink locale]" format
                if (m_session)
                    PSendSysMessage(LANG_SKILL_LIST_CHAT, id, id, name.c_str(), localeNames[loc], knownStr, valStr);
                else
                    PSendSysMessage(LANG_SKILL_LIST_CONSOLE, id, name.c_str(), localeNames[loc], knownStr, valStr);

                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSKILLFOUND);
    return true;
}

void ChatHandler::ShowSpellListHelper(Player* target, SpellEntry const* spellInfo, LocaleConstant loc)
{
    uint32 id = spellInfo->Id;

    bool known = target && target->HasSpell(id);
    bool learn = (spellInfo->Effect[EFFECT_INDEX_0] == SPELL_EFFECT_LEARN_SPELL);

    uint32 talentCost = GetTalentSpellCost(id);

    bool talent = (talentCost > 0);
    bool passive = IsPassiveSpell(spellInfo);
    bool active = target && target->HasAura(id);

    // unit32 used to prevent interpreting uint8 as char at output
    // find rank of learned spell for learning spell, or talent rank
    uint32 rank = talentCost ? talentCost : sSpellMgr.GetSpellRank(learn ? spellInfo->EffectTriggerSpell[EFFECT_INDEX_0] : id);

    // send spell in "id - [name, rank N] [talent] [passive] [learn] [known]" format
    std::ostringstream ss;
    if (m_session)
        ss << id << " - |cffffffff|Hspell:" << id << "|h[" << spellInfo->SpellName[loc];
    else
        ss << id << " - " << spellInfo->SpellName[loc];

    // include rank in link name
    if (rank)
        ss << GetMangosString(LANG_SPELL_RANK) << rank;

    if (m_session)
        ss << " " << localeNames[loc] << "]|h|r";
    else
        ss << " " << localeNames[loc];

    if (talent)
        ss << GetMangosString(LANG_TALENT);
    if (passive)
        ss << GetMangosString(LANG_PASSIVE);
    if (learn)
        ss << GetMangosString(LANG_LEARN);
    if (known)
        ss << GetMangosString(LANG_KNOWN);
    if (active)
        ss << GetMangosString(LANG_ACTIVE);

    SendSysMessage(ss.str().c_str());
}

bool ChatHandler::HandleLookupSpellCommand(char* args)
{
    if (!*args)
        return false;

    // can be nullptr at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in Spell.dbc
    for (uint32 id = 0; id < sSpellTemplate.GetMaxEntry(); ++id)
    {
        SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(id);
        if (spellInfo)
        {
            int loc = int(DEFAULT_LOCALE);
            std::string name = spellInfo->SpellName[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == GetSessionDbcLocale())
                        continue;

                    name = spellInfo->SpellName[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                ShowSpellListHelper(target, spellInfo, LocaleConstant(loc));
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOSPELLFOUND);
    return true;
}


void ChatHandler::ShowQuestListHelper(uint32 questId, int32 loc_idx, Player* target /*= nullptr*/)
{
    Quest const* qinfo = sObjectMgr.GetQuestTemplate(questId);
    if (!qinfo)
        return;

    std::string title = qinfo->GetTitle();
    sObjectMgr.GetQuestLocaleStrings(questId, loc_idx, &title);

    char const* statusStr = "";

    if (target)
    {
        QuestStatus status = target->GetQuestStatus(qinfo->GetQuestId());

        if (status == QUEST_STATUS_COMPLETE)
        {
            if (target->GetQuestRewardStatus(qinfo->GetQuestId()))
                statusStr = GetMangosString(LANG_COMMAND_QUEST_REWARDED);
            else
                statusStr = GetMangosString(LANG_COMMAND_QUEST_COMPLETE);
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
            statusStr = GetMangosString(LANG_COMMAND_QUEST_ACTIVE);
    }

    if (m_session)
        PSendSysMessage(LANG_QUEST_LIST_CHAT, qinfo->GetQuestId(), qinfo->GetQuestId(), qinfo->GetQuestLevel(), title.c_str(), statusStr);
    else
        PSendSysMessage(LANG_QUEST_LIST_CONSOLE, qinfo->GetQuestId(), title.c_str(), statusStr);
}

bool ChatHandler::HandleLookupQuestCommand(char* args)
{
    if (!*args)
        return false;

    // can be nullptr at console call
    Player* target = getSelectedPlayer();

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0 ;

    int loc_idx = GetSessionDbLocaleIndex();

    ObjectMgr::QuestMap const& qTemplates = sObjectMgr.GetQuestTemplates();
    for (const auto& qTemplate : qTemplates)
    {
        Quest* qinfo = qTemplate.second.get();

        std::string title;                                  // "" for avoid repeating check default locale
        sObjectMgr.GetQuestLocaleStrings(qinfo->GetQuestId(), loc_idx, &title);

        if ((title.empty() || !Utf8FitTo(title, wnamepart)) && !Utf8FitTo(qinfo->GetTitle(), wnamepart))
            continue;

        ShowQuestListHelper(qinfo->GetQuestId(), loc_idx, target);
        ++counter;
    }

    if (counter == 0)
        SendSysMessage(LANG_COMMAND_NOQUESTFOUND);

    return true;
}

bool ChatHandler::HandleLookupCreatureCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    for (uint32 id = 0; id < sCreatureStorage.GetMaxEntry(); ++id)
    {
        CreatureInfo const* cInfo = sCreatureStorage.LookupEntry<CreatureInfo> (id);
        if (!cInfo)
            continue;

        int loc_idx = GetSessionDbLocaleIndex();

        char const* name = "";                              // "" for avoid repeating check for default locale
        sObjectMgr.GetCreatureLocaleStrings(id, loc_idx, &name);
        if (!*name || !Utf8FitTo(name, wnamepart))
        {
            name = cInfo->Name;
            if (!Utf8FitTo(name, wnamepart))
                continue;
        }

        if (m_session)
            PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CHAT, id, id, name);
        else
            PSendSysMessage(LANG_CREATURE_ENTRY_LIST_CONSOLE, id, name);

        ++counter;
    }

    if (counter == 0)
        SendSysMessage(LANG_COMMAND_NOCREATUREFOUND);

    return true;
}

bool ChatHandler::HandleLookupObjectCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    // converting string that we try to find to lower case
    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    wstrToLower(wnamepart);

    uint32 counter = 0;

    for (SQLStorageBase::SQLSIterator<GameObjectInfo> itr = sGOStorage.getDataBegin<GameObjectInfo>(); itr < sGOStorage.getDataEnd<GameObjectInfo>(); ++itr)
    {
        int loc_idx = GetSessionDbLocaleIndex();
        if (loc_idx >= 0)
        {
            GameObjectLocale const* gl = sObjectMgr.GetGameObjectLocale(itr->id);
            if (gl)
            {
                if ((int32)gl->Name.size() > loc_idx && !gl->Name[loc_idx].empty())
                {
                    std::string name = gl->Name[loc_idx];

                    if (Utf8FitTo(name, wnamepart))
                    {
                        if (m_session)
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->id, itr->id, name.c_str());
                        else
                            PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->id, name.c_str());
                        ++counter;
                        continue;
                    }
                }
            }
        }

        std::string name = itr->name;
        if (name.empty())
            continue;

        if (Utf8FitTo(name, wnamepart))
        {
            if (m_session)
                PSendSysMessage(LANG_GO_ENTRY_LIST_CHAT, itr->id, itr->id, name.c_str());
            else
                PSendSysMessage(LANG_GO_ENTRY_LIST_CONSOLE, itr->id, name.c_str());
            ++counter;
        }
    }

    if (counter == 0)
        SendSysMessage(LANG_COMMAND_NOGAMEOBJECTFOUND);

    return true;
}

bool ChatHandler::HandleLookupTaxiNodeCommand(char* args)
{
    if (!*args)
        return false;

    std::string namepart = args;
    std::wstring wnamepart;

    if (!Utf8toWStr(namepart, wnamepart))
        return false;

    // converting string that we try to find to lower case
    wstrToLower(wnamepart);

    uint32 counter = 0;                                     // Counter for figure out that we found smth.

    // Search in TaxiNodes.dbc
    for (uint32 id = 0; id < sTaxiNodesStore.GetNumRows(); ++id)
    {
        TaxiNodesEntry const* nodeEntry = sTaxiNodesStore.LookupEntry(id);
        if (nodeEntry)
        {
            int loc = GetSessionDbcLocale();
            std::string name = nodeEntry->name[loc];
            if (name.empty())
                continue;

            if (!Utf8FitTo(name, wnamepart))
            {
                loc = 0;
                for (; loc < MAX_LOCALE; ++loc)
                {
                    if (loc == GetSessionDbcLocale())
                        continue;

                    name = nodeEntry->name[loc];
                    if (name.empty())
                        continue;

                    if (Utf8FitTo(name, wnamepart))
                        break;
                }
            }

            if (loc < MAX_LOCALE)
            {
                // send taxinode in "id - [name] (Map:m X:x Y:y Z:z)" format
                if (m_session)
                    PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CHAT, id, id, name.c_str(), localeNames[loc],
                                    nodeEntry->map_id, nodeEntry->x, nodeEntry->y, nodeEntry->z);
                else
                    PSendSysMessage(LANG_TAXINODE_ENTRY_LIST_CONSOLE, id, name.c_str(), localeNames[loc],
                                    nodeEntry->map_id, nodeEntry->x, nodeEntry->y, nodeEntry->z);
                ++counter;
            }
        }
    }
    if (counter == 0)                                       // if counter == 0 then we found nth
        SendSysMessage(LANG_COMMAND_NOTAXINODEFOUND);
    return true;
}

/** \brief GM command level 3 - Create a guild.
 *
 * This command allows a GM (level 3) to create a guild.
 *
 * The "args" parameter contains the name of the guild leader
 * and then the name of the guild.
 *
 */
bool ChatHandler::HandleGuildCreateCommand(char* args)
{
    // guildmaster name optional
    char* guildMasterStr = ExtractOptNotLastArg(&args);

    Player* target;
    if (!ExtractPlayerTarget(&guildMasterStr, &target))
        return false;

    char* guildStr = ExtractQuotedArg(&args);
    if (!guildStr)
        return false;

    std::string guildname = guildStr;

    if (target->GetGuildId())
    {
        SendSysMessage(LANG_PLAYER_IN_GUILD);
        return true;
    }

    Guild* guild = new Guild;
    if (!guild->Create(target, guildname))
    {
        delete guild;
        SendSysMessage(LANG_GUILD_NOT_CREATED);
        SetSentErrorMessage(true);
        return false;
    }

    sGuildMgr.AddGuild(guild);
    return true;
}

bool ChatHandler::HandleGuildInviteCommand(char* args)
{
    // player name optional
    char* nameStr = ExtractOptNotLastArg(&args);

    // if not guild name only (in "") then player name
    ObjectGuid target_guid;
    if (!ExtractPlayerTarget(&nameStr, nullptr, &target_guid))
        return false;

    char* guildStr = ExtractQuotedArg(&args);
    if (!guildStr)
        return false;

    std::string glName = guildStr;
    Guild* targetGuild = sGuildMgr.GetGuildByName(glName);
    if (!targetGuild)
        return false;

    // player's guild membership checked in AddMember before add
    if (!targetGuild->AddMember(target_guid, targetGuild->GetLowestRank()))
        return false;

    return true;
}

bool ChatHandler::HandleGuildUninviteCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    if (!ExtractPlayerTarget(&args, &target, &target_guid))
        return false;

    uint32 glId   = target ? target->GetGuildId() : Player::GetGuildIdFromDB(target_guid);
    if (!glId)
        return false;

    Guild* targetGuild = sGuildMgr.GetGuildById(glId);
    if (!targetGuild)
        return false;

    if (targetGuild->DelMember(target_guid))
        targetGuild->Disband();

    return true;
}

bool ChatHandler::HandleGuildRankCommand(char* args)
{
    char* nameStr = ExtractOptNotLastArg(&args);

    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&nameStr, &target, &target_guid, &target_name))
        return false;

    uint32 glId = target ? target->GetGuildId() : Player::GetGuildIdFromDB(target_guid);
    if (!glId)
        return false;

    Guild* targetGuild = sGuildMgr.GetGuildById(glId);
    if (!targetGuild)
        return false;

    uint32 newrank;
    if (!ExtractUInt32(&args, newrank))
        return false;

    if (newrank > targetGuild->GetLowestRank())
        return false;

    MemberSlot* slot = targetGuild->GetMemberSlot(target_guid);
    if (!slot)
        return false;

    if (newrank == GR_GUILDMASTER)
    {
        MemberSlot* leaderSlot = targetGuild->GetMemberSlot(targetGuild->GetLeaderGuid());
        if (!leaderSlot)
            return false;

        leaderSlot->ChangeRank(GR_OFFICER);
        targetGuild->SetLeader(target_guid);
    }
    else
        slot->ChangeRank(newrank);

    return true;
}

bool ChatHandler::HandleGuildDeleteCommand(char* args)
{
    if (!*args)
        return false;

    char* guildStr = ExtractQuotedArg(&args);
    if (!guildStr)
        return false;

    std::string gld = guildStr;

    Guild* targetGuild = sGuildMgr.GetGuildByName(gld);
    if (!targetGuild)
        return false;

    targetGuild->Disband();

    return true;
}

bool ChatHandler::HandleGetDistanceCommand(char* args)
{
    WorldObject* obj = nullptr;

    if (*args)
    {
        if (ObjectGuid guid = ExtractGuidFromLink(&args))
            obj = (WorldObject*)m_session->GetPlayer()->GetObjectByTypeMask(guid, TYPEMASK_CREATURE_OR_GAMEOBJECT);

        if (!obj)
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }
    }
    else
    {
        obj = getSelectedUnit();

        if (!obj)
        {
            SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }
    }

    Player* player = m_session->GetPlayer();

    PSendSysMessage("Raw distance: %.2f", sqrt(player->GetDistance(obj, true, DIST_CALC_NONE)));

    PSendSysMessage("P -> T Combat distance: %.2f", player->GetDistance(obj, true, DIST_CALC_COMBAT_REACH));
    PSendSysMessage("P -> T Bounding distance: %.2f", player->GetDistance(obj, true, DIST_CALC_BOUNDING_RADIUS));
    PSendSysMessage("T -> P Combat distance: %.2f", obj->GetDistance(player, true, DIST_CALC_COMBAT_REACH));
    PSendSysMessage("T -> P Bounding distance: %.2f", obj->GetDistance(player, true, DIST_CALC_BOUNDING_RADIUS));

    Unit* target = dynamic_cast<Unit*>(obj);
    PSendSysMessage("P -> T Attack distance: %.2f", player->GetAttackDistance(target));
    PSendSysMessage("P -> T Visible distance: %.2f", player->GetVisibilityData().GetStealthVisibilityDistance(target));
    PSendSysMessage("P -> T Visible distance (Alert): %.2f", player->GetVisibilityData().GetStealthVisibilityDistance(target, true));
    PSendSysMessage("T -> P Attack distance: %.2f", target->GetAttackDistance(player));
    PSendSysMessage("T -> P Visible distance: %.2f", target->GetVisibilityData().GetStealthVisibilityDistance(player));
    PSendSysMessage("T -> P Visible distance (Alert): %.2f", target->GetVisibilityData().GetStealthVisibilityDistance(player, true));

    return true;
}

bool ChatHandler::HandleGetLosCommand(char* /*args*/)
{
    Player* player = m_session->GetPlayer();
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        return false;
    }

    float x, y, z;
    target->GetPosition(x, y, z);
    z += target->GetCollisionHeight();
    bool normalLos = player->IsWithinLOS(x, y, z, false);
    bool m2Los = player->IsWithinLOS(x, y, z, true);
    PSendSysMessage("Los check: Normal: %s M2: %s", normalLos ? "true" : "false", m2Los ? "true" : "false");
    return true;
}

bool ChatHandler::HandleDieCommand(char* args)
{
    Player* player = m_session->GetPlayer();
    Unit* target = getSelectedUnit();

    if (!target || !player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (target->GetTypeId() == TYPEID_PLAYER)
    {
        if (HasLowerSecurity((Player*)target, ObjectGuid(), false))
            return false;
    }

    uint32 param;
    ExtractOptUInt32(&args, param, 0);
    if (param != 0)
    {
        if (target->IsAlive())
        {
            DamageEffectType damageType = DIRECT_DAMAGE;
            uint32 absorb = 0;
            uint32 damage = target->GetHealth();
            Unit::DealDamageMods(player, target, damage, &absorb, damageType);
            Unit::DealDamage(player, target, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
        }
    }
    else
    {
        if (target->IsAlive())
            Unit::DealDamage(player, target, target->GetHealth(), nullptr, INSTAKILL, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
    }

    return true;
}

bool ChatHandler::HandleDamageCommand(char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();
    Player* player = m_session->GetPlayer();

    if (!target || !player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!target->IsAlive())
        return true;

    int32 damage_int;
    if (!ExtractInt32(&args, damage_int))
        return false;

    if (damage_int <= 0)
        return true;

    uint32 damage = uint32(damage_int);

    // flat melee damage without resistance/etc reduction
    if (!*args)
    {
        uint32 absorb = 0;
        Unit::DealDamageMods(player, target, damage, &absorb, DIRECT_DAMAGE);
        Unit::DealDamage(player, target, damage, nullptr, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, nullptr, false);
        if (target != player)
            player->SendAttackStateUpdate(HITINFO_NORMALSWING2, target, SPELL_SCHOOL_MASK_NORMAL, damage, 0, 0, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    uint32 school;
    if (!ExtractUInt32(&args, school))
        return false;

    if (school >= MAX_SPELL_SCHOOL)
        return false;

    SpellSchoolMask schoolmask = GetSchoolMask(school);

    if (schoolmask & SPELL_SCHOOL_MASK_NORMAL)
        damage = Unit::CalcArmorReducedDamage(player, target, damage);

    // melee damage by specific school
    if (!*args)
    {
        uint32 absorb = 0;
        int32 resist = 0;

        target->CalculateDamageAbsorbAndResist(player, schoolmask, SPELL_DIRECT_DAMAGE, damage, &absorb, &resist);

        const uint32 bonus = (resist < 0 ? uint32(std::abs(resist)) : 0);
        damage += bonus;
        const uint32 malus = (resist > 0 ? (absorb + uint32(resist)) : absorb);

        if (damage <= malus)
            return true;

        damage -= malus;

        Unit::DealDamageMods(player, target, damage, &absorb, DIRECT_DAMAGE);
        Unit::DealDamage(player, target, damage, nullptr, DIRECT_DAMAGE, schoolmask, nullptr, false);
        player->SendAttackStateUpdate(HITINFO_NORMALSWING2, target, schoolmask, damage, absorb, resist, VICTIMSTATE_NORMAL, 0);
        return true;
    }

    // non-melee damage

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellid = ExtractSpellIdFromLink(&args);
    if (!spellid || !sSpellTemplate.LookupEntry<SpellEntry>(spellid))
        return false;

    player->SpellNonMeleeDamageLog(target, spellid, damage);
    return true;
}

bool ChatHandler::HandleReviveCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    if (!ExtractPlayerTarget(&args, &target, &target_guid))
        return false;

    if (target)
    {
        target->ResurrectPlayer(1.0f);
        target->SpawnCorpseBones();
    }
    else
        // will resurrected at login without corpse
        sObjectAccessor.ConvertCorpseForPlayer(target_guid);

    return true;
}

bool ChatHandler::HandleAuraCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellID = ExtractSpellIdFromLink(&args);

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spellID);
    if (!spellInfo)
        return false;

    if (!IsSpellAppliesAura(spellInfo, (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)) &&
            !IsSpellHaveEffect(spellInfo, SPELL_EFFECT_PERSISTENT_AREA_AURA))
    {
        PSendSysMessage(LANG_SPELL_NO_HAVE_AURAS, spellID);
        SetSentErrorMessage(true);
        return false;
    }

    SpellAuraHolder* holder = CreateSpellAuraHolder(spellInfo, target, m_session->GetPlayer());

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        uint8 eff = spellInfo->Effect[i];
        if (eff >= MAX_SPELL_EFFECTS)
            continue;
        if (IsAreaAuraEffect(eff)           ||
                eff == SPELL_EFFECT_APPLY_AURA  ||
                eff == SPELL_EFFECT_PERSISTENT_AREA_AURA)
        {
            int32 basePoints = spellInfo->CalculateSimpleValue(SpellEffectIndex(i));
            int32 damage = basePoints;
            Aura* aur = CreateAura(spellInfo, SpellEffectIndex(i), &damage, &basePoints, holder, target);
            holder->AddAura(aur, SpellEffectIndex(i));
        }
    }
    if (!target->AddSpellAuraHolder(holder))
        delete holder;
    else
        holder->SetState(SPELLAURAHOLDER_STATE_READY);

    return true;
}

bool ChatHandler::HandleUnAuraCommand(char* args)
{
    Unit* target = getSelectedUnit();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    std::string argstr = args;
    if (argstr == "all")
    {
        target->RemoveAllAuras();
        return true;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spellID = ExtractSpellIdFromLink(&args);
    if (!spellID)
        return false;

    target->RemoveAurasDueToSpell(spellID);

    return true;
}

bool ChatHandler::HandleLinkGraveCommand(char* args)
{
    uint32 g_id;
    if (!ExtractUInt32(&args, g_id))
        return false;

    char* teamStr = ExtractLiteralArg(&args);

    Team g_team;
    if (!teamStr)
        g_team = TEAM_BOTH_ALLOWED;
    else if (strncmp(teamStr, "horde", strlen(teamStr)) == 0)
        g_team = HORDE;
    else if (strncmp(teamStr, "alliance", strlen(teamStr)) == 0)
        g_team = ALLIANCE;
    else
        return false;

    WorldSafeLocsEntry const* graveyard = sWorldSafeLocsStore.LookupEntry<WorldSafeLocsEntry>(g_id);
    if (!graveyard)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDNOEXIST, g_id);
        SetSentErrorMessage(true);
        return false;
    }

    Player* player = m_session->GetPlayer();

    uint32 zoneId = player->GetZoneId();

    AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(zoneId);
    if (!areaEntry || areaEntry->zone != 0)
    {
        PSendSysMessage(LANG_COMMAND_GRAVEYARDWRONGZONE, g_id, zoneId);
        SetSentErrorMessage(true);
        return false;
    }

    if (player->GetMap()->GetGraveyardManager().AddGraveYardLink(g_id, zoneId, GRAVEYARD_AREALINK, g_team))
        PSendSysMessage(LANG_COMMAND_GRAVEYARDLINKED, g_id, zoneId);
    else
        PSendSysMessage(LANG_COMMAND_GRAVEYARDALRLINKED, g_id, zoneId);

    return true;
}

bool ChatHandler::HandleNearGraveCommand(char* args)
{
    Team g_team;

    size_t argslen = strlen(args);

    if (!*args)
        g_team = TEAM_BOTH_ALLOWED;
    else if (strncmp(args, "horde", argslen) == 0)
        g_team = HORDE;
    else if (strncmp(args, "alliance", argslen) == 0)
        g_team = ALLIANCE;
    else
        return false;

    Player* player = m_session->GetPlayer();
    uint32 zone_id = player->GetZoneId();
    uint32 area_id = player->GetAreaId();

    WorldSafeLocsEntry const* graveyard = player->GetMap()->GetGraveyardManager().GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), g_team);

    if (graveyard)
    {
        uint32 g_id = graveyard->ID;

        GraveYardData const* data = player->GetMap()->GetGraveyardManager().FindGraveYardData(g_id, area_id);
        if (!data || (g_team != TEAM_BOTH_ALLOWED && data->team != g_team && data->team != TEAM_BOTH_ALLOWED))
            data = player->GetMap()->GetGraveyardManager().FindGraveYardData(g_id, zone_id);

        if (!data)
        {
            PSendSysMessage(LANG_COMMAND_GRAVEYARDERROR, g_id);
            SetSentErrorMessage(true);
            return false;
        }

        std::string team_name;

        if (data->team == TEAM_BOTH_ALLOWED)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_ANY);
        else if (data->team == HORDE)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if (data->team == ALLIANCE)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_ALLIANCE);
        else                                                // Actually, this case cannot happen
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_NOTEAM);

        PSendSysMessage(LANG_COMMAND_GRAVEYARDNEAREST, g_id, team_name.c_str(), zone_id);
    }
    else
    {
        std::string team_name;

        if (g_team == TEAM_BOTH_ALLOWED)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_ANY);
        else if (g_team == HORDE)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_HORDE);
        else if (g_team == ALLIANCE)
            team_name = GetMangosString(LANG_COMMAND_GRAVEYARD_ALLIANCE);

        if (g_team == TEAM_BOTH_ALLOWED)
            PSendSysMessage(LANG_COMMAND_ZONENOGRAVEYARDS, zone_id);
        else
            PSendSysMessage(LANG_COMMAND_ZONENOGRAFACTION, zone_id, team_name.c_str());
    }

    return true;
}

//-----------------------Npc Commands-----------------------
bool ChatHandler::HandleNpcAllowMovementCommand(char* /*args*/)
{
    if (sWorld.getAllowMovement())
    {
        sWorld.SetAllowMovement(false);
        SendSysMessage(LANG_CREATURE_MOVE_DISABLED);
    }
    else
    {
        sWorld.SetAllowMovement(true);
        SendSysMessage(LANG_CREATURE_MOVE_ENABLED);
    }
    return true;
}

bool ChatHandler::HandleNpcChangeEntryCommand(char* args)
{
    if (!*args)
        return false;

    uint32 newEntryNum = atoi(args);
    if (!newEntryNum)
        return false;

    Unit* unit = getSelectedUnit();
    if (!unit || unit->GetTypeId() != TYPEID_UNIT)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }
    Creature* creature = (Creature*)unit;
    if (creature->UpdateEntry(newEntryNum))
        SendSysMessage(LANG_DONE);
    else
        SendSysMessage(LANG_ERROR);
    return true;
}

bool ChatHandler::HandleNpcInfoCommand(char* /*args*/)
{
    Creature* target = getSelectedCreature();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 faction = target->GetFaction();
    uint32 npcflags = target->GetUInt32Value(UNIT_NPC_FLAGS);
    uint32 displayid = target->GetDisplayId();
    uint32 nativeid = target->GetNativeDisplayId();
    uint32 Entry = target->GetEntry();
    CreatureInfo const* cInfo = target->GetCreatureInfo();

    time_t curRespawnDelay = target->GetRespawnTimeEx() - time(nullptr);
    if (curRespawnDelay < 0)
        curRespawnDelay = 0;
    std::string curRespawnDelayStr = secsToTimeString(curRespawnDelay, true);
    std::string defRespawnDelayStr = secsToTimeString(target->GetRespawnDelay(), true);
    std::string curCorpseDecayStr = secsToTimeString(std::chrono::system_clock::to_time_t(target->GetCorpseDecayTimer()), true);

    PSendSysMessage(LANG_NPCINFO_CHAR, target->GetGuidStr().c_str(), faction, npcflags, Entry, displayid, nativeid);

    PSendSysMessage(LANG_NPCINFO_LEVEL, target->GetLevel());
    PSendSysMessage(LANG_NPCINFO_HEALTH, target->GetCreateHealth(), target->GetMaxHealth(), target->GetHealth());
    PSendSysMessage(LANG_NPCINFO_FLAGS, target->GetUInt32Value(UNIT_FIELD_FLAGS), target->GetUInt32Value(UNIT_DYNAMIC_FLAGS), target->GetCreatureInfo()->ExtraFlags);
    PSendSysMessage(LANG_COMMAND_RAWPAWNTIMES, defRespawnDelayStr.c_str(), curRespawnDelayStr.c_str());
    PSendSysMessage("Corpse decay remaining time: %s", curCorpseDecayStr.c_str());
    PSendSysMessage(LANG_NPCINFO_LOOT,  cInfo->LootId, cInfo->PickpocketLootId, cInfo->SkinningLootId);
    PSendSysMessage(LANG_NPCINFO_DUNGEON_ID, target->GetInstanceId());
    PSendSysMessage(LANG_NPCINFO_POSITION, float(target->GetPositionX()), float(target->GetPositionY()), float(target->GetPositionZ()));
    PSendSysMessage("Combat timer: %u", target->GetCombatManager().GetCombatTimer());
    PSendSysMessage("Is in evade mode: %s", target->GetCombatManager().IsInEvadeMode() ? "true" : "false");

    PSendSysMessage("UNIT_FIELD_BYTES_0: %d : %d : %d : %d", target->GetByteValue(UNIT_FIELD_BYTES_0, 0), target->GetByteValue(UNIT_FIELD_BYTES_0, 1),
        target->GetByteValue(UNIT_FIELD_BYTES_0, 2), target->GetByteValue(UNIT_FIELD_BYTES_0, 3));

    PSendSysMessage("UNIT_FIELD_BYTES_1: %d : %d : %d : %d", target->GetByteValue(UNIT_FIELD_BYTES_1, 0), target->GetByteValue(UNIT_FIELD_BYTES_1, 1),
        target->GetByteValue(UNIT_FIELD_BYTES_1, 2), target->GetByteValue(UNIT_FIELD_BYTES_1, 3));

    PSendSysMessage("UNIT_FIELD_BYTES_2: %d : %d : %d : %d", target->GetByteValue(UNIT_FIELD_BYTES_2, 0), target->GetByteValue(UNIT_FIELD_BYTES_2, 1),
        target->GetByteValue(UNIT_FIELD_BYTES_2, 2), target->GetByteValue(UNIT_FIELD_BYTES_2, 3));

    auto& spellList = target->GetSpellList();
    PSendSysMessage("Spell Lists %s ID %u", spellList.Disabled ? "disabled" : "enabled", spellList.Id);

    PSendSysMessage("Combat Timer: %u Leashing disabled: %s", target->GetCombatManager().GetCombatTimer(), target->GetCombatManager().IsLeashingDisabled() ? "true" : "false");

    PSendSysMessage("Combat Script: %s", target->AI()->GetCombatScriptStatus() ? "true" : "false");
    PSendSysMessage("Movementflags: %u", target->m_movementInfo.moveFlags);

    if (CreatureGroup* group = target->GetCreatureGroup())
        PSendSysMessage("Creature group: %u", group->GetGroupEntry().Id);

    if (auto vector = sObjectMgr.GetAllRandomCreatureEntries(target->GetDbGuid()))
    {
        std::string output;
        for (uint32 entry : *vector)
            output += std::to_string(entry) + ",";
        PSendSysMessage("NPC is part of creature_spawn_entry: %s", output.data());
    }

    if ((npcflags & UNIT_NPC_FLAG_VENDOR))
    {
        SendSysMessage(LANG_NPCINFO_VENDOR);
    }
    if ((npcflags & UNIT_NPC_FLAG_TRAINER))
    {
        SendSysMessage(LANG_NPCINFO_TRAINER);
    }

    ShowNpcOrGoSpawnInformation<Creature>(target->GetDbGuid());
    return true;
}

bool ChatHandler::HandleNpcThreatCommand(char* /*args*/)
{
    Unit* target = getSelectedUnit();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // Showing threat for %s [Entry %u]
    PSendSysMessage(LANG_NPC_THREAT_SELECTED_CREATURE, target->GetName(), target->GetEntry());

    ThreatList const& tList = target->getThreatManager().getThreatList();
    for (auto itr : tList)
    {
        Unit* unit = itr->getTarget();

        if (unit)
            // Player |cffff0000%s|r [GUID: %u] has |cffff0000%f|r threat, taunt state %u and hostile state %u
            PSendSysMessage(LANG_NPC_THREAT_PLAYER, unit->GetName(), unit->GetGUIDLow(), target->getThreatManager().getThreat(unit), itr->GetTauntState(), itr->GetHostileState());
    }

    return true;
}

// play npc emote
bool ChatHandler::HandleNpcPlayEmoteCommand(char* args)
{
    uint32 emote = atoi(args);

    Creature* target = getSelectedCreature();
    if (!target)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->HandleEmote(emote);

    return true;
}

// TODO: NpcCommands that needs to be fixed :

bool ChatHandler::HandleNpcAddWeaponCommand(char* /*args*/)
{
    /*if (!*args)
    return false;

    ObjectGuid guid = m_session->GetPlayer()->GetSelectionGuid();
    if (guid.IsEmpty())
    {
        SendSysMessage(LANG_NO_SELECTION);
        return true;
    }

    Creature *pCreature = ObjectAccessor::GetCreature(*m_session->GetPlayer(), guid);

    if(!pCreature)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        return true;
    }

    char* pSlotID = strtok((char*)args, " ");
    if (!pSlotID)
        return false;

    char* pItemID = strtok(nullptr, " ");
    if (!pItemID)
        return false;

    uint32 ItemID = atoi(pItemID);
    uint32 SlotID = atoi(pSlotID);

    ItemPrototype* tmpItem = ObjectMgr::GetItemPrototype(ItemID);

    bool added = false;
    if(tmpItem)
    {
        switch(SlotID)
        {
            case 1:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY, ItemID);
                added = true;
                break;
            case 2:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_01, ItemID);
                added = true;
                break;
            case 3:
                pCreature->SetUInt32Value(UNIT_VIRTUAL_ITEM_SLOT_DISPLAY_02, ItemID);
                added = true;
                break;
            default:
                PSendSysMessage(LANG_ITEM_SLOT_NOT_EXIST,SlotID);
                added = false;
                break;
        }

        if(added)
            PSendSysMessage(LANG_ITEM_ADDED_TO_SLOT,ItemID,tmpItem->Name1,SlotID);
    }
    else
    {
        PSendSysMessage(LANG_ITEM_NOT_FOUND,ItemID);
        return true;
    }
    */
    return true;
}
//----------------------------------------------------------

bool ChatHandler::HandleExploreCheatCommand(char* args)
{
    if (!*args)
        return false;

    int flag = atoi(args);

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    if (flag != 0)
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_ALL, GetNameLink(chr).c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_ALL, GetNameLink().c_str());
    }
    else
    {
        PSendSysMessage(LANG_YOU_SET_EXPLORE_NOTHING, GetNameLink(chr).c_str());
        if (needReportToTarget(chr))
            ChatHandler(chr).PSendSysMessage(LANG_YOURS_EXPLORE_SET_NOTHING, GetNameLink().c_str());
    }

    for (uint8 i = 0; i < PLAYER_EXPLORED_ZONES_SIZE; ++i)
    {
        if (flag != 0)
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0xFFFFFFFF);
        }
        else
        {
            m_session->GetPlayer()->SetFlag(PLAYER_EXPLORED_ZONES_1 + i, 0);
        }
    }

    return true;
}

void ChatHandler::HandleCharacterLevel(Player* player, ObjectGuid player_guid, uint32 oldlevel, uint32 newlevel)
{
    if (player)
    {
        player->GiveLevel(newlevel);
        player->InitTalentForLevel();
        player->SetUInt32Value(PLAYER_XP, 0);

        if (needReportToTarget(player))
        {
            if (oldlevel == newlevel)
                ChatHandler(player).PSendSysMessage(LANG_YOURS_LEVEL_PROGRESS_RESET, GetNameLink().c_str());
            else if (oldlevel < newlevel)
                ChatHandler(player).PSendSysMessage(LANG_YOURS_LEVEL_UP, GetNameLink().c_str(), newlevel);
            else                                            // if(oldlevel > newlevel)
                ChatHandler(player).PSendSysMessage(LANG_YOURS_LEVEL_DOWN, GetNameLink().c_str(), newlevel);
        }
    }
    else
    {
        // update level and XP at level, all other will be updated at loading
        CharacterDatabase.PExecute("UPDATE characters SET level = '%u', xp = 0 WHERE guid = '%u'", newlevel, player_guid.GetCounter());
    }
}

bool ChatHandler::HandleCharacterLevelCommand(char* args)
{
    char* nameStr = ExtractOptNotLastArg(&args);

    int32 newlevel;
    bool nolevel = false;
    // exception opt second arg: .character level $name
    if (!ExtractInt32(&args, newlevel))
    {
        if (!nameStr)
        {
            nameStr = ExtractArg(&args);
            if (!nameStr)
                return false;

            nolevel = true;
        }
        else
            return false;
    }

    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&nameStr, &target, &target_guid, &target_name))
        return false;

    int32 oldlevel = target ? target->GetLevel() : Player::GetLevelFromDB(target_guid);
    if (nolevel)
        newlevel = oldlevel;

    if (newlevel < 1)
        return false;                                       // invalid level

    if (newlevel > STRONG_MAX_LEVEL)                        // hardcoded maximum level
        newlevel = STRONG_MAX_LEVEL;

    HandleCharacterLevel(target, target_guid, oldlevel, newlevel);

    if (!m_session || m_session->GetPlayer() != target)     // including player==nullptr
    {
        std::string nameLink = playerLink(target_name);
        PSendSysMessage(LANG_YOU_CHANGE_LVL, nameLink.c_str(), newlevel);
    }

    return true;
}

bool ChatHandler::HandleLevelUpCommand(char* args)
{
    int32 addlevel = 1;
    char* nameStr = nullptr;

    if (*args)
    {
        nameStr = ExtractOptNotLastArg(&args);

        // exception opt second arg: .levelup $name
        if (!ExtractInt32(&args, addlevel))
        {
            if (!nameStr)
                nameStr = ExtractArg(&args);
            else
                return false;
        }
    }

    ObjectGuid target_guid;
    std::string target_name;

    //add pet to levelup command
    if (m_session)
    {
        Pet* pet = getSelectedPet();
        Player* player = m_session->GetPlayer();
        if (pet && pet->GetOwner() && pet->GetOwner()->GetTypeId() == TYPEID_PLAYER)
        {
            if (pet->getPetType() == HUNTER_PET)
            {
                uint32 newPetLevel = pet->GetLevel() + addlevel;

                if (newPetLevel <= player->GetLevel())
                {
                    pet->GivePetLevel(newPetLevel);

                    std::string nameLink = petLink(pet->GetName());
                    PSendSysMessage(LANG_YOU_CHANGE_LVL, nameLink.c_str(), newPetLevel);
                    return true;
                }
            }

            return false;
        }
    }

    Player* target;
    if (!ExtractPlayerTarget(&nameStr, &target, &target_guid, &target_name))
        return false;

    int32 oldlevel = target ? target->GetLevel() : Player::GetLevelFromDB(target_guid);
    int32 newlevel = oldlevel + addlevel;

    if (newlevel < 1)
        newlevel = 1;

    if (newlevel > STRONG_MAX_LEVEL)                        // hardcoded maximum level
        newlevel = STRONG_MAX_LEVEL;

    HandleCharacterLevel(target, target_guid, oldlevel, newlevel);

    if (!m_session || m_session->GetPlayer() != target)     // including chr==nullptr
    {
        std::string nameLink = playerLink(target_name);
        PSendSysMessage(LANG_YOU_CHANGE_LVL, nameLink.c_str(), newlevel);
    }

    return true;
}

bool ChatHandler::HandleShowAreaCommand(char* args)
{
    if (!*args)
        return false;

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int area = GetAreaFlagByAreaID(atoi(args));
    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if (area < 0 || offset >= PLAYER_EXPLORED_ZONES_SIZE)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields | val));

    SendSysMessage(LANG_EXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleHideAreaCommand(char* args)
{
    if (!*args)
        return false;

    Player* chr = getSelectedPlayer();
    if (chr == nullptr)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    int area = GetAreaFlagByAreaID(atoi(args));
    int offset = area / 32;
    uint32 val = (uint32)(1 << (area % 32));

    if (area < 0 || offset >= PLAYER_EXPLORED_ZONES_SIZE)
    {
        SendSysMessage(LANG_BAD_VALUE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 currFields = chr->GetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset);
    chr->SetUInt32Value(PLAYER_EXPLORED_ZONES_1 + offset, (uint32)(currFields ^ val));

    SendSysMessage(LANG_UNEXPLORE_AREA);
    return true;
}

bool ChatHandler::HandleAuctionAllianceCommand(char* /*args*/)
{
    m_session->GetPlayer()->SetAuctionAccessMode(m_session->GetPlayer()->GetTeam() != ALLIANCE ? -1 : 0);
    m_session->SendAuctionHello(m_session->GetPlayer());
    return true;
}

bool ChatHandler::HandleAuctionHordeCommand(char* /*args*/)
{
    m_session->GetPlayer()->SetAuctionAccessMode(m_session->GetPlayer()->GetTeam() != HORDE ? -1 : 0);
    m_session->SendAuctionHello(m_session->GetPlayer());
    return true;
}

bool ChatHandler::HandleAuctionGoblinCommand(char* /*args*/)
{
    m_session->GetPlayer()->SetAuctionAccessMode(1);
    m_session->SendAuctionHello(m_session->GetPlayer());
    return true;
}

bool ChatHandler::HandleAuctionCommand(char* /*args*/)
{
    m_session->GetPlayer()->SetAuctionAccessMode(0);
    m_session->SendAuctionHello(m_session->GetPlayer());

    return true;
}

bool ChatHandler::HandleAuctionItemCommand(char* args)
{
    // format: (alliance|horde|goblin) item[:count] price [buyout] [short|long|verylong]
    char* typeStr = ExtractLiteralArg(&args);
    if (!typeStr)
        return false;

    uint32 houseid;
    if (strncmp(typeStr, "alliance", strlen(typeStr)) == 0)
        houseid = 1;
    else if (strncmp(typeStr, "horde", strlen(typeStr)) == 0)
        houseid = 6;
    else if (strncmp(typeStr, "goblin", strlen(typeStr)) == 0)
        houseid = 7;
    else
        return false;

    // parse item str
    char* itemStr = ExtractArg(&args);
    if (!itemStr)
        return false;

    uint32 item_id = 0;
    uint32 item_count = 1;
    if (sscanf(itemStr, "%u:%u", &item_id, &item_count) != 2)
        if (sscanf(itemStr, "%u", &item_id) != 1)
            return false;

    uint32 price;
    if (!ExtractUInt32(&args, price))
        return false;

    uint32 buyout;
    if (!ExtractOptUInt32(&args, buyout, 0))
        return false;

    uint32 etime = 4 * MIN_AUCTION_TIME;
    if (char* timeStr = ExtractLiteralArg(&args))
    {
        if (strncmp(timeStr, "short", strlen(timeStr)) == 0)
            etime = 1 * MIN_AUCTION_TIME;
        else if (strncmp(timeStr, "long", strlen(timeStr)) == 0)
            etime = 2 * MIN_AUCTION_TIME;
        else if (strncmp(timeStr, "verylong", strlen(timeStr)) == 0)
            etime = 4 * MIN_AUCTION_TIME;
        else
            return false;
    }

    AuctionHouseEntry const* auctionHouseEntry = sAuctionHouseStore.LookupEntry(houseid);
    AuctionHouseObject* auctionHouse = sAuctionMgr.GetAuctionsMap(auctionHouseEntry);

    if (!item_id)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    ItemPrototype const* item_proto = ObjectMgr::GetItemPrototype(item_id);
    if (!item_proto)
    {
        PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    if (item_count < 1 || (item_proto->MaxCount > 0 && item_count > uint32(item_proto->MaxCount)))
    {
        PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, item_count, item_id);
        SetSentErrorMessage(true);
        return false;
    }

    do
    {
        uint32 item_stack = item_count > item_proto->GetMaxStackSize() ? item_proto->GetMaxStackSize() : item_count;
        item_count -= item_stack;

        Item* newItem = Item::CreateItem(item_id, item_stack);
        MANGOS_ASSERT(newItem);

        auctionHouse->AddAuction(auctionHouseEntry, newItem, etime, price, buyout);
    }
    while (item_count);

    return true;
}

bool ChatHandler::HandleBankCommand(char* /*args*/)
{
    m_session->SendShowBank(m_session->GetPlayer()->GetObjectGuid());

    return true;
}

bool ChatHandler::HandleStableCommand(char* /*args*/)
{
    m_session->SendStablePet(m_session->GetPlayer()->GetObjectGuid());

    return true;
}

bool ChatHandler::HandleChangeWeatherCommand(char* args)
{
    // Weather is OFF
    if (!sWorld.getConfig(CONFIG_BOOL_WEATHER))
    {
        SendSysMessage(LANG_WEATHER_DISABLED);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 type;
    if (!ExtractUInt32(&args, type))
        return false;

    // see enum WeatherType
    if (!Weather::IsValidWeatherType(type))
        return false;

    float grade;
    if (!ExtractFloat(&args, grade))
        return false;

    // clamp grade from 0 to 1
    if (grade < 0.0f)
        grade = 0.0f;
    else if (grade > 1.0f)
        grade = 1.0f;

    Player* player = m_session->GetPlayer();
    uint32 zoneId = player->GetZoneId();
    if (!sWeatherMgr.GetWeatherChances(zoneId))
    {
        SendSysMessage(LANG_NO_WEATHER);
        SetSentErrorMessage(true);
    }
    player->GetMap()->SetWeather(zoneId, (WeatherType)type, grade, false);

    return true;
}

bool ChatHandler::HandleTeleAddCommand(char* args)
{
    if (!*args)
        return false;

    Player* player = m_session->GetPlayer();
    if (!player)
        return false;

    std::string name = args;

    if (sObjectMgr.GetGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TP_ALREADYEXIST);
        SetSentErrorMessage(true);
        return false;
    }

    GameTele tele;
    tele.position_x  = player->GetPositionX();
    tele.position_y  = player->GetPositionY();
    tele.position_z  = player->GetPositionZ();
    tele.orientation = player->GetOrientation();
    tele.mapId       = player->GetMapId();
    tele.name        = name;

    if (sObjectMgr.AddGameTele(tele))
    {
        SendSysMessage(LANG_COMMAND_TP_ADDED);
    }
    else
    {
        SendSysMessage(LANG_COMMAND_TP_ADDEDERR);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

bool ChatHandler::HandleTeleDelCommand(char* args)
{
    if (!*args)
        return false;

    std::string name = args;

    if (!sObjectMgr.DeleteGameTele(name))
    {
        SendSysMessage(LANG_COMMAND_TELE_NOTFOUND);
        SetSentErrorMessage(true);
        return false;
    }

    SendSysMessage(LANG_COMMAND_TP_DELETED);
    return true;
}

bool ChatHandler::HandleListAreaTriggerCommand(char* args)
{
    Player* player = m_session->GetPlayer();
    if (!player)
        return false;

    uint32 counter = 0;
    uint32 playerMap = player->GetMap()->GetId();

    if (player->GetMap()->IsContinent())
    {
        // Get areatriggers in the same area as the player
        uint32 playerArea = player->GetAreaId();
        AreaTableEntry const* areaEntry = GetAreaEntryByAreaID(playerArea);
        PSendSysMessage(LANG_AREATRIGGER_LIST, "area", areaEntry ? areaEntry->area_name[GetSessionDbcLocale()] : "<unknown>");

        for (uint32 id = 0; id < sAreaTriggerStore.GetNumRows(); ++id)
        {
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(id);
            if (!atEntry)
                continue;

            if (atEntry->mapid != playerMap)
                continue;

            uint32 areaTriggerArea = player->GetTerrain()->GetAreaId(atEntry->x, atEntry->y, atEntry->z);
            if (areaTriggerArea != playerArea)
                continue;

            ShowTriggerListHelper(atEntry);
            ++counter;
        }
    }
    else
    {
        // Get areatriggers in the same map as the player
        PSendSysMessage(LANG_AREATRIGGER_LIST, "map", player->GetMap()->GetMapName());

        for (uint32 id = 0; id < sAreaTriggerStore.GetNumRows(); ++id)
        {
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(id);
            if (!atEntry)
                continue;

            if (atEntry->mapid != playerMap)
                continue;

            ShowTriggerListHelper(atEntry);
            ++counter;
        }
    }

    if (counter == 0)
        SendSysMessage(LANG_COMMAND_NOTRIGGERFOUND);
    return true;
}

bool ChatHandler::HandleListAurasCommand(char* args)
{
    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 auraNameId;
    ExtractOptUInt32(&args, auraNameId, 0);
    if (auraNameId >= TOTAL_AURAS)
    {
        PSendSysMessage("Need to use aura name id below %u.", TOTAL_AURAS);
        SetSentErrorMessage(true);
        return false;
    }

    char const* talentStr = GetMangosString(LANG_TALENT);
    char const* passiveStr = GetMangosString(LANG_PASSIVE);

    if (!auraNameId)
    {
        Unit::SpellAuraHolderMap const& uAuras = unit->GetSpellAuraHolderMap();
        PSendSysMessage(LANG_COMMAND_TARGET_LISTAURAS, uAuras.size());
        for (Unit::SpellAuraHolderMap::const_iterator itr = uAuras.begin(); itr != uAuras.end(); ++itr)
        {
            bool talent = GetTalentSpellCost(itr->second->GetId()) > 0;

            SpellAuraHolder* holder = itr->second;
            char const* name = holder->GetSpellProto()->SpellName[GetSessionDbcLocale()];

            for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
            {
                Aura* aur = holder->GetAuraByEffectIndex(SpellEffectIndex(i));
                if (!aur)
                    continue;

                if (m_session)
                {
                    std::ostringstream ss_name;
                    ss_name << "|cffffffff|Hspell:" << itr->second->GetId() << "|h[" << name << "]|h|r";

                    PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, holder->GetId(), aur->GetEffIndex(),
                        aur->GetModifier()->m_auraname, aur->GetAuraDuration(), aur->GetAuraMaxDuration(),
                        ss_name.str().c_str(),
                        (holder->IsPassive() ? passiveStr : ""), (talent ? talentStr : ""),
                        holder->GetCasterGuid().GetString().c_str(), holder->GetStackAmount());
                }
                else
                {
                    PSendSysMessage(LANG_COMMAND_TARGET_AURADETAIL, holder->GetId(), aur->GetEffIndex(),
                        aur->GetModifier()->m_auraname, aur->GetAuraDuration(), aur->GetAuraMaxDuration(),
                        name,
                        (holder->IsPassive() ? passiveStr : ""), (talent ? talentStr : ""),
                        holder->GetCasterGuid().GetString().c_str(), holder->GetStackAmount());
                }
            }
        }
    }
    uint32 i = auraNameId ? auraNameId : 0;
    uint32 max = auraNameId ? auraNameId + 1 : TOTAL_AURAS;
    for (; i < max; ++i)
    {
        Unit::AuraList const& uAuraList = unit->GetAurasByType(AuraType(i));
        if (uAuraList.empty()) continue;
        PSendSysMessage(LANG_COMMAND_TARGET_LISTAURATYPE, uAuraList.size(), i);
        for (Unit::AuraList::const_iterator itr = uAuraList.begin(); itr != uAuraList.end(); ++itr)
        {
            bool talent = GetTalentSpellCost((*itr)->GetId()) > 0;

            char const* name = (*itr)->GetSpellProto()->SpellName[GetSessionDbcLocale()];

            if (m_session)
            {
                std::ostringstream ss_name;
                ss_name << "|cffffffff|Hspell:" << (*itr)->GetId() << "|h[" << name << "]|h|r";

                PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(),
                                ss_name.str().c_str(), ((*itr)->GetHolder()->IsPassive() ? passiveStr : ""), (talent ? talentStr : ""),
                                (*itr)->GetCasterGuid().GetString().c_str());
            }
            else
            {
                PSendSysMessage(LANG_COMMAND_TARGET_AURASIMPLE, (*itr)->GetId(), (*itr)->GetEffIndex(),
                                name, ((*itr)->GetHolder()->IsPassive() ? passiveStr : ""), (talent ? talentStr : ""),
                                (*itr)->GetCasterGuid().GetString().c_str());
            }
        }
    }
    return true;
}

bool ChatHandler::HandleListTalentsCommand(char* /*args*/)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    SendSysMessage(LANG_LIST_TALENTS_TITLE);
    uint32 count = 0;
    uint32 cost = 0;
    PlayerSpellMap const& uSpells = player->GetSpellMap();
    for (const auto& uSpell : uSpells)
    {
        if (uSpell.second.state == PLAYERSPELL_REMOVED || uSpell.second.disabled)
            continue;

        uint32 cost_itr = GetTalentSpellCost(uSpell.first);

        if (cost_itr == 0)
            continue;

        SpellEntry const* spellEntry = sSpellTemplate.LookupEntry<SpellEntry>(uSpell.first);
        if (!spellEntry)
            continue;

        ShowSpellListHelper(player, spellEntry, GetSessionDbcLocale());
        ++count;
        cost += cost_itr;
    }
    PSendSysMessage(LANG_LIST_TALENTS_COUNT, count, cost);

    return true;
}

bool ChatHandler::HandleResetHonorCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    target->ResetHonor();
    return true;
}

static bool HandleResetStatsOrLevelHelper(Player* player)
{
    ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->getClass());
    if (!cEntry)
    {
        sLog.outError("Class %u not found in DBC (Wrong DBC files?)", player->getClass());
        return false;
    }

    uint8 powertype = cEntry->powerType;

    // reset m_form if no aura
    if (!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
        player->SetShapeshiftForm(FORM_NONE);

    player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE);
    player->SetFloatValue(UNIT_FIELD_COMBATREACH, 1.5f);

    player->setFactionForRace(player->getRace());

    player->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_POWER_TYPE, powertype);

    // reset only if player not in some form;
    if (player->GetShapeshiftForm() == FORM_NONE)
        player->InitDisplayIds();

    // is it need, only in pre-2.x used and field byte removed later?
    if (powertype == POWER_RAGE || powertype == POWER_MANA)
        player->SetByteValue(UNIT_FIELD_BYTES_1, UNIT_BYTES_1_OFFSET_PET_LOYALTY, 0xEE);

    player->SetByteValue(UNIT_FIELD_BYTES_2, UNIT_BYTES_2_OFFSET_MISC_FLAGS, UNIT_BYTE2_FLAG_SUPPORTABLE | UNIT_BYTE2_FLAG_UNK5);

    player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    //-1 is default value
    player->SetInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, -1);

    // player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000 );
    return true;
}

bool ChatHandler::HandleResetLevelCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    if (!HandleResetStatsOrLevelHelper(target))
        return false;

    // set starting level
    uint32 start_level = sWorld.getConfig(CONFIG_UINT32_START_PLAYER_LEVEL);

    target->SetLevel(start_level);
    target->InitStatsForLevel(true);
    target->InitTaxiNodes();
    target->InitTalentForLevel();
    target->SetUInt32Value(PLAYER_XP, 0);

    // reset level for pet
    if (Pet* pet = target->GetPet())
        pet->SynchronizeLevelWithOwner();

    return true;
}

bool ChatHandler::HandleResetStatsCommand(char* args)
{
    Player* target;
    if (!ExtractPlayerTarget(&args, &target))
        return false;

    if (!HandleResetStatsOrLevelHelper(target))
        return false;

    target->InitStatsForLevel(true);
    target->InitTalentForLevel();

    return true;
}

bool ChatHandler::HandleResetSpellsCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    if (target)
    {
        target->resetSpells();

        ChatHandler(target).SendSysMessage(LANG_RESET_SPELLS);
        if (!m_session || m_session->GetPlayer() != target)
            PSendSysMessage(LANG_RESET_SPELLS_ONLINE, GetNameLink(target).c_str());
    }
    else
    {
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", uint32(AT_LOGIN_RESET_SPELLS), target_guid.GetCounter());
        PSendSysMessage(LANG_RESET_SPELLS_OFFLINE, target_name.c_str());
    }

    return true;
}

bool ChatHandler::HandleResetTalentsCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    if (target)
    {
        target->resetTalents(true);

        ChatHandler(target).SendSysMessage(LANG_RESET_TALENTS);
        if (!m_session || m_session->GetPlayer() != target)
            PSendSysMessage(LANG_RESET_TALENTS_ONLINE, GetNameLink(target).c_str());
        return true;
    }
    if (target_guid)
    {
        uint32 at_flags = AT_LOGIN_RESET_TALENTS;
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", at_flags, target_guid.GetCounter());
        std::string nameLink = playerLink(target_name);
        PSendSysMessage(LANG_RESET_TALENTS_OFFLINE, nameLink.c_str());
        return true;
    }

    SendSysMessage(LANG_NO_CHAR_SELECTED);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleResetTaxiNodesCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &target_guid, &target_name))
        return false;

    if (target)
    {
        target->InitTaxiNodes();

        ChatHandler(target).SendSysMessage("Your taxi nodes have been reset.");
        if (!m_session || m_session->GetPlayer() != target)
            PSendSysMessage("Taxi nodes of %s have been reset.", GetNameLink(target).c_str());
        return true;
    }
    if (target_guid)
    {
        uint32 at_flags = AT_LOGIN_RESET_TAXINODES;
        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", at_flags, target_guid.GetCounter());
        std::string nameLink = playerLink(target_name);
        PSendSysMessage("Taxi nodes of %s will be reset at next login.", nameLink.c_str());
        return true;
    }

    SendSysMessage(LANG_NO_CHAR_SELECTED);
    SetSentErrorMessage(true);
    return false;
}

bool ChatHandler::HandleResetAllCommand(char* args)
{
    if (!*args)
        return false;

    std::string casename = args;

    AtLoginFlags atLogin;

    // Command specially created as single command to prevent using short case names
    if (casename == "spells")
    {
        atLogin = AT_LOGIN_RESET_SPELLS;
        sWorld.SendWorldText(LANG_RESETALL_SPELLS);
        if (!m_session)
            SendSysMessage(LANG_RESETALL_SPELLS);
    }
    else if (casename == "talents")
    {
        atLogin = AT_LOGIN_RESET_TALENTS;
        sWorld.SendWorldText(LANG_RESETALL_TALENTS);
        if (!m_session)
            SendSysMessage(LANG_RESETALL_TALENTS);
    }
    else
    {
        PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE, args);
        SetSentErrorMessage(true);
        return false;
    }

    CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'", atLogin, atLogin);
    HashMapHolder<Player>::MapType const& plist = sObjectAccessor.GetPlayers();
    for (const auto& itr : plist)
        itr.second->SetAtLoginFlag(atLogin);

    return true;
}

bool ChatHandler::HandleServerShutDownCancelCommand(char* /*args*/)
{
    sWorld.ShutdownCancel();
    return true;
}

bool ChatHandler::HandleServerShutDownCommand(char* args)
{
    uint32 delay;
    if (!ExtractUInt32(&args, delay))
        return false;

    uint32 exitcode;
    if (!ExtractOptUInt32(&args, exitcode, SHUTDOWN_EXIT_CODE))
        return false;

    // Exit code should be in range of 0-125, 126-255 is used
    // in many shells for their own return codes and code > 255
    // is not supported in many others
    if (exitcode > 125)
        return false;

    sWorld.ShutdownServ(delay, 0, exitcode);
    return true;
}

bool ChatHandler::HandleServerRestartCommand(char* args)
{
    uint32 delay;
    if (!ExtractUInt32(&args, delay))
        return false;

    uint32 exitcode;
    if (!ExtractOptUInt32(&args, exitcode, RESTART_EXIT_CODE))
        return false;

    // Exit code should be in range of 0-125, 126-255 is used
    // in many shells for their own return codes and code > 255
    // is not supported in many others
    if (exitcode > 125)
        return false;

    sWorld.ShutdownServ(delay, SHUTDOWN_MASK_RESTART, exitcode);
    return true;
}

bool ChatHandler::HandleServerIdleRestartCommand(char* args)
{
    uint32 delay;
    if (!ExtractUInt32(&args, delay))
        return false;

    uint32 exitcode;
    if (!ExtractOptUInt32(&args, exitcode, RESTART_EXIT_CODE))
        return false;

    // Exit code should be in range of 0-125, 126-255 is used
    // in many shells for their own return codes and code > 255
    // is not supported in many others
    if (exitcode > 125)
        return false;

    sWorld.ShutdownServ(delay, SHUTDOWN_MASK_RESTART | SHUTDOWN_MASK_IDLE, exitcode);
    return true;
}

bool ChatHandler::HandleServerIdleShutDownCommand(char* args)
{
    uint32 delay;
    if (!ExtractUInt32(&args, delay))
        return false;

    uint32 exitcode;
    if (!ExtractOptUInt32(&args, exitcode, SHUTDOWN_EXIT_CODE))
        return false;

    // Exit code should be in range of 0-125, 126-255 is used
    // in many shells for their own return codes and code > 255
    // is not supported in many others
    if (exitcode > 125)
        return false;

    sWorld.ShutdownServ(delay, SHUTDOWN_MASK_IDLE, exitcode);
    return true;
}

bool ChatHandler::HandleQuestAddCommand(char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .addquest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
    uint32 entry;
    if (!ExtractUint32KeyFromLink(&args, "Hquest", entry))
        return false;

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);
    if (!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // check item starting quest (it can work incorrectly if added without item in inventory)
    for (uint32 id = 0; id < sItemStorage.GetMaxEntry(); ++id)
    {
        ItemPrototype const* pProto = sItemStorage.LookupEntry<ItemPrototype>(id);
        if (!pProto)
            continue;

        if (pProto->StartQuest == entry)
        {
            PSendSysMessage(LANG_COMMAND_QUEST_STARTFROMITEM, entry, pProto->ItemId);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // ok, normal (creature/GO starting) quest
    if (player->CanAddQuest(pQuest, true) && !player->IsCurrentQuest(pQuest->GetQuestId()))
    {
        player->AddQuest(pQuest, nullptr);

        if (player->CanCompleteQuest(entry))
            player->CompleteQuest(entry);
    }

    if (!player->SatisfyQuestClass(pQuest, false))
        SendSysMessage("Warning: Added quest to player who does not fulfill class requirement. Might not work correctly.");
    if (!player->SatisfyQuestRace(pQuest, false))
        SendSysMessage("Warning: Added quest to player who does not fulfill race requirement. Might not work correctly.");

    return true;
}

bool ChatHandler::HandleQuestRemoveCommand(char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .removequest #entry'
    // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
    uint32 entry;
    if (!ExtractUint32KeyFromLink(&args, "Hquest", entry))
        return false;

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    if (!pQuest)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // remove all quest entries for 'entry' from quest log
    for (uint8 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 quest = player->GetQuestSlotQuestId(slot);
        if (quest == entry)
        {
            player->SetQuestSlot(slot, 0);

            // we ignore unequippable quest items in this case, its' still be equipped
            player->TakeQuestSourceItem(quest, false);
        }
    }

    // set quest status to not started (will updated in DB at next save)
    player->SetQuestStatus(entry, QUEST_STATUS_NONE);

    // reset rewarded for restart repeatable quest
    player->getQuestStatusMap()[entry].m_rewarded = false;

    SendSysMessage(LANG_COMMAND_QUEST_REMOVED);
    return true;
}

bool ChatHandler::HandleQuestCompleteCommand(char* args)
{
    Player* player = getSelectedPlayer();
    if (!player)
    {
        SendSysMessage(LANG_NO_CHAR_SELECTED);
        SetSentErrorMessage(true);
        return false;
    }

    // .quest complete #entry
    // number or [name] Shift-click form |color|Hquest:quest_id:quest_level|h[name]|h|r
    uint32 entry;
    if (!ExtractUint32KeyFromLink(&args, "Hquest", entry))
        return false;

    Quest const* pQuest = sObjectMgr.GetQuestTemplate(entry);

    // If player doesn't have the quest
    if (!pQuest || player->GetQuestStatus(entry) == QUEST_STATUS_NONE)
    {
        PSendSysMessage(LANG_COMMAND_QUEST_NOTFOUND, entry);
        SetSentErrorMessage(true);
        return false;
    }

    // Add quest items for quests that require items
    for (uint8 x = 0; x < QUEST_ITEM_OBJECTIVES_COUNT; ++x)
    {
        uint32 id = pQuest->ReqItemId[x];
        uint32 count = pQuest->ReqItemCount[x];
        if (!id || !count)
            continue;

        uint32 curItemCount = player->GetItemCount(id, true);

        ItemPosCountVec dest;
        uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, id, count - curItemCount);
        if (msg == EQUIP_ERR_OK)
        {
            Item* item = player->StoreNewItem(dest, id, true);
            player->SendNewItem(item, count - curItemCount, true, false);
        }
    }

    // All creature/GO slain/casted (not required, but otherwise it will display "Creature slain 0/10")
    for (uint8 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
    {
        int32 creature = pQuest->ReqCreatureOrGOId[i];
        uint32 creaturecount = pQuest->ReqCreatureOrGOCount[i];

        if (uint32 spell_id = pQuest->ReqSpell[i])
        {
            for (uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(creature, ObjectGuid((creature > 0 ? HIGHGUID_UNIT : HIGHGUID_GAMEOBJECT), uint32(std::abs(creature)), 1u), spell_id);
        }
        else if (creature > 0)
        {
            if (CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(creature))
                for (uint16 z = 0; z < creaturecount; ++z)
                    player->KilledMonster(cInfo, nullptr);
        }
        else if (creature < 0)
        {
            for (uint16 z = 0; z < creaturecount; ++z)
                player->CastedCreatureOrGO(-creature, ObjectGuid(), 0);
        }
    }

    // If the quest requires reputation to complete
    if (uint32 repFaction = pQuest->GetRepObjectiveFaction())
    {
        uint32 repValue = pQuest->GetRepObjectiveValue();
        uint32 curRep = player->GetReputationMgr().GetReputation(repFaction);
        if (curRep < repValue)
            if (FactionEntry const* factionEntry = sFactionStore.LookupEntry(repFaction))
                player->GetReputationMgr().SetReputation(factionEntry, repValue);
    }

    // If the quest requires money
    int32 ReqOrRewMoney = pQuest->GetRewOrReqMoney();
    if (ReqOrRewMoney < 0)
        player->ModifyMoney(-ReqOrRewMoney);

    player->CompleteQuest(entry);
    return true;
}

bool ChatHandler::HandleAddCharacterNoteCommand(char* args)
{
    Player* target;
    ObjectGuid playerGuid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &playerGuid, &target_name))
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = target ? target->GetSession()->GetAccountId() : sObjectMgr.GetPlayerAccountIdByGUID(playerGuid);

    std::string authorName = m_session ? m_session->GetPlayerName() : "Console";
    const char* reason = ExtractQuotedOrLiteralArg(&args);
    if (!reason)
        reason = "<no reason given>";

    sWorld.WarnAccount(account_id, authorName, reason, "NOTE");

    PSendSysMessage("Account #%u (character %s): a note has been added \"%s\"", account_id, target_name.c_str(), reason);
    return true;
}

bool ChatHandler::HandleWarnCharacterCommand(char* args)
{
    Player* target;
    ObjectGuid playerGuid;
    std::string target_name;
    if (!ExtractPlayerTarget(&args, &target, &playerGuid, &target_name))
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 account_id = target ? target->GetSession()->GetAccountId() : sObjectMgr.GetPlayerAccountIdByGUID(playerGuid);

    std::string authorName = m_session ? m_session->GetPlayerName() : "Console";
    const char* reason = ExtractQuotedOrLiteralArg(&args);
    if (!reason)
        reason = "<no reason given>";

    sWorld.WarnAccount(account_id, authorName, reason, "WARN");

    PSendSysMessage("Account #%u (character %s) has been warned for \"%s\"", account_id, target_name.c_str(), reason);
    return true;
}

bool ChatHandler::HandleBanAccountCommand(char* args)
{
    return HandleBanHelper(BAN_ACCOUNT, args);
}

bool ChatHandler::HandleBanCharacterCommand(char* args)
{
    return HandleBanHelper(BAN_CHARACTER, args);
}

bool ChatHandler::HandleBanIPCommand(char* args)
{
    return HandleBanHelper(BAN_IP, args);
}

bool ChatHandler::HandleBanHelper(BanMode mode, char* args)
{
    if (!*args)
        return false;

    char* cnameOrIP = ExtractArg(&args);
    if (!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    char* duration = ExtractArg(&args);                     // time string
    if (!duration)
        return false;

    uint32 duration_secs = TimeStringToSecs(duration);

    char* reason = ExtractArg(&args);
    if (!reason)
        return false;

    switch (mode)
    {
        case BAN_ACCOUNT:
            if (!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
            if (!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if (!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    switch (sWorld.BanAccount(mode, nameOrIP, duration_secs, reason, m_session ? m_session->GetPlayerName() : ""))
    {
        case BAN_SUCCESS:
            if (duration_secs > 0)
                PSendSysMessage(LANG_BAN_YOUBANNED, nameOrIP.c_str(), secsToTimeString(duration_secs, true).c_str(), reason);
            else
                PSendSysMessage(LANG_BAN_YOUPERMBANNED, nameOrIP.c_str(), reason);
            break;
        case BAN_SYNTAX_ERROR:
            return false;
        case BAN_NOTFOUND:
            switch (mode)
            {
                default:
                    PSendSysMessage(LANG_BAN_NOTFOUND, "account", nameOrIP.c_str());
                    break;
                case BAN_CHARACTER:
                    PSendSysMessage(LANG_BAN_NOTFOUND, "character", nameOrIP.c_str());
                    break;
            }
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleUnBanAccountCommand(char* args)
{
    return HandleUnBanHelper(BAN_ACCOUNT, args);
}

bool ChatHandler::HandleUnBanCharacterCommand(char* args)
{
    return HandleUnBanHelper(BAN_CHARACTER, args);
}

bool ChatHandler::HandleUnBanIPCommand(char* args)
{
    return HandleUnBanHelper(BAN_IP, args);
}

bool ChatHandler::HandleUnBanHelper(BanMode mode, char* args)
{
    if (!*args)
        return false;

    char* cnameOrIP = ExtractArg(&args);
    if (!cnameOrIP)
        return false;

    std::string nameOrIP = cnameOrIP;

    char* message = ExtractQuotedOrLiteralArg(&args);
    if (!message)
        return false;

    std::string unbanMessage(message);

    switch (mode)
    {
        case BAN_ACCOUNT:
            if (!AccountMgr::normalizeString(nameOrIP))
            {
                PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, nameOrIP.c_str());
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_CHARACTER:
            if (!normalizePlayerName(nameOrIP))
            {
                SendSysMessage(LANG_PLAYER_NOT_FOUND);
                SetSentErrorMessage(true);
                return false;
            }
            break;
        case BAN_IP:
            if (!IsIPAddress(nameOrIP.c_str()))
                return false;
            break;
    }

    std::string source = m_session ? m_session->GetPlayerName() : "CONSOLE";
    if (sWorld.RemoveBanAccount(mode, source, unbanMessage, nameOrIP))
        PSendSysMessage(LANG_UNBAN_UNBANNED, nameOrIP.c_str(), unbanMessage.c_str());
    else
        PSendSysMessage(LANG_UNBAN_ERROR, nameOrIP.c_str());

    return true;
}

bool ChatHandler::HandleBanInfoAccountCommand(char* args)
{
    if (!*args)
        return false;

    std::string account_name;
    uint32 accountid = ExtractAccountId(&args, &account_name);
    if (!accountid)
        return false;

    return HandleBanInfoHelper(accountid, account_name.c_str());
}

bool ChatHandler::HandleBanInfoCharacterCommand(char* args)
{
    Player* target;
    ObjectGuid target_guid;
    if (!ExtractPlayerTarget(&args, &target, &target_guid))
        return false;

    uint32 accountid = target ? target->GetSession()->GetAccountId() : sObjectMgr.GetPlayerAccountIdByGUID(target_guid);

    std::string accountname;
    if (!sAccountMgr.GetName(accountid, accountname))
    {
        PSendSysMessage(LANG_BANINFO_NOCHARACTER);
        return true;
    }

    return HandleBanInfoHelper(accountid, accountname.c_str());
}

bool ChatHandler::HandleBanInfoHelper(uint32 accountid, char const* accountname)
{
    auto queryResult = LoginDatabase.PQuery("SELECT " _FROM_UNIXTIME_("banned_at") ",expires_at-banned_at,active,expires_at,reason,banned_by,unbanned_at,unbanned_by "
                                            "FROM account_banned WHERE account_id = '%u' ORDER BY banned_at ASC", accountid);
    if (!queryResult)
    {
        PSendSysMessage(LANG_BANINFO_NOACCOUNTBAN, accountname);
        return true;
    }

    PSendSysMessage(LANG_BANINFO_BANHISTORY, accountname);
    do
    {
        Field* fields = queryResult->Fetch();

        time_t expiresAt = time_t(fields[3].GetUInt64());
        bool active = false;
        if (fields[2].GetBool() && (fields[1].GetUInt64() == (uint64)0 || expiresAt >= time(nullptr)))
            active = true;
        bool permanent = (fields[1].GetUInt64() == (uint64)0);
        std::string bantime = permanent ? GetMangosString(LANG_BANINFO_INFINITE) : secsToTimeString(fields[1].GetUInt64(), true);
        std::string unbannedBy = fields[7].IsNULL() ? "" : fields[7].GetString();
        std::string manuallyUnbanned = "";
        if (unbannedBy.empty())
            manuallyUnbanned = GetMangosString(LANG_BANINFO_YES);
        else
        {
            manuallyUnbanned = GetMangosString(LANG_BANINFO_NO);
            manuallyUnbanned += "," + secsToTimeString(fields[6].GetUInt64());
        }
        PSendSysMessage(LANG_BANINFO_HISTORYENTRY,
                        fields[0].GetString(), bantime.c_str(), active ? GetMangosString(LANG_BANINFO_YES) : GetMangosString(LANG_BANINFO_NO), fields[4].GetString(), fields[5].GetString());
    }
    while (queryResult->NextRow());

    return true;
}

bool ChatHandler::HandleBanInfoIPCommand(char* args)
{
    if (!*args)
        return false;

    char* cIP = ExtractQuotedOrLiteralArg(&args);
    if (!cIP)
        return false;

    if (!IsIPAddress(cIP))
        return false;

    std::string IP = cIP;

    LoginDatabase.escape_string(IP);
    auto queryResult = LoginDatabase.PQuery("SELECT ip, " _FROM_UNIXTIME_("banned_at") ", " _FROM_UNIXTIME_("expires_at") ", expires_at-" _UNIXTIME_ ", reason,banned_by,expires_at-banned_at"
                                            "FROM ip_banned WHERE ip = '%s'", IP.c_str());
    if (!queryResult)
    {
        PSendSysMessage(LANG_BANINFO_NOIP);
        return true;
    }

    Field* fields = queryResult->Fetch();
    bool permanent = !fields[6].GetUInt64();
    PSendSysMessage(LANG_BANINFO_IPENTRY,
                    fields[0].GetString(), fields[1].GetString(), permanent ? GetMangosString(LANG_BANINFO_NEVER) : fields[2].GetString(),
                    permanent ? GetMangosString(LANG_BANINFO_INFINITE) : secsToTimeString(fields[3].GetUInt64(), true).c_str(), fields[4].GetString(), fields[5].GetString());
    return true;
}

bool ChatHandler::HandleBanListCharacterCommand(char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE expires_at<=" _UNIXTIME_ " AND expires_at<>banned_at");

    char* cFilter = ExtractLiteralArg(&args);
    if (!cFilter)
        return false;

    std::string filter = cFilter;
    LoginDatabase.escape_string(filter);
    auto queryResult = CharacterDatabase.PQuery("SELECT account FROM characters WHERE name " _LIKE_ " " _CONCAT3_("'%%'", "'%s'", "'%%'"), filter.c_str());
    if (!queryResult)
    {
        PSendSysMessage(LANG_BANLIST_NOCHARACTER);
        return true;
    }

    return HandleBanListHelper(std::move(queryResult));
}

bool ChatHandler::HandleBanListAccountCommand(char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE expires_at<=" _UNIXTIME_ " AND expires_at<>banned_at");

    char* cFilter = ExtractLiteralArg(&args);
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.escape_string(filter);

    std::unique_ptr<QueryResult> queryResult;

    if (filter.empty())
    {
        queryResult = LoginDatabase.Query("SELECT account.id, username FROM account, account_banned"
                                          " WHERE account.id = account_banned.account_id AND active = 1 GROUP BY account.id");
    }
    else
    {
        queryResult = LoginDatabase.PQuery("SELECT account.id, username FROM account, account_banned"
                                           " WHERE account.id = account_banned.account_id AND active = 1 AND username " _LIKE_ " " _CONCAT3_("'%%'", "'%s'", "'%%'")" GROUP BY account.id",
                                           filter.c_str());
    }

    if (!queryResult)
    {
        PSendSysMessage(LANG_BANLIST_NOACCOUNT);
        return true;
    }

    return HandleBanListHelper(std::move(queryResult));
}

bool ChatHandler::HandleBanListHelper(std::unique_ptr<QueryResult> queryResult)
{
    PSendSysMessage(LANG_BANLIST_MATCHINGACCOUNT);

    // Chat short output
    if (m_session)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            uint32 accountid = fields[0].GetUInt32();

            auto banresult = LoginDatabase.PQuery("SELECT account.username FROM account,account_banned WHERE account_banned.account_id='%u' AND account_banned.account_id=account.id", accountid);
            if (banresult)
            {
                Field* fields2 = banresult->Fetch();
                PSendSysMessage("%s", fields2[0].GetString());
            }
        }
        while (queryResult->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_ACCOUNTS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_ACCOUNTS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field* fields = queryResult->Fetch();
            uint32 account_id = fields[0].GetUInt32();

            std::string account_name;

            // "account" case, name can be get in same query
            if (queryResult->GetFieldCount() > 1)
                account_name = fields[1].GetCppString();
            // "character" case, name need extract from another DB
            else
                sAccountMgr.GetName(account_id, account_name);

            // No SQL injection. id is uint32.
            auto banInfo = LoginDatabase.PQuery("SELECT banned_at,expires_at,banned_by,reason,unbanned_at,unbanned_by FROM account_banned WHERE account_id = %u ORDER BY expires_at", account_id);
            if (banInfo)
            {
                Field* fields2 = banInfo->Fetch();
                do
                {
                    time_t t_ban = fields2[0].GetUInt64();
                    tm* aTm_ban = localtime(&t_ban);

                    if (fields2[0].GetUInt64() == fields2[1].GetUInt64())
                    {
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|%-15.15s|%-15.15s|",
                                        account_name.c_str(), aTm_ban->tm_year % 100, aTm_ban->tm_mon + 1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                                        fields2[2].GetString(), fields2[3].GetString(), fields2[5].GetString(), secsToTimeString(fields2[4].GetUInt64()).data());
                    }
                    else
                    {
                        time_t t_unban = fields2[1].GetUInt64();
                        tm* aTm_unban = localtime(&t_unban);
                        PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|%-15.15s|%-15.15s|",
                                        account_name.c_str(), aTm_ban->tm_year % 100, aTm_ban->tm_mon + 1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                                        aTm_unban->tm_year % 100, aTm_unban->tm_mon + 1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                                        fields2[2].GetString(), fields2[3].GetString(), fields2[5].GetString(), secsToTimeString(fields2[4].GetUInt64()).data());
                    }
                }
                while (banInfo->NextRow());
            }
        }
        while (queryResult->NextRow());
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleBanListIPCommand(char* args)
{
    LoginDatabase.Execute("DELETE FROM ip_banned WHERE expires_at<=" _UNIXTIME_ " AND expires_at<>banned_at");

    char* cFilter = ExtractLiteralArg(&args);
    std::string filter = cFilter ? cFilter : "";
    LoginDatabase.escape_string(filter);

    std::unique_ptr<QueryResult> queryResult;

    if (filter.empty())
    {
        queryResult = LoginDatabase.Query("SELECT ip,banned_at,expires_at,banned_by,reason FROM ip_banned"
                                          " WHERE (banned_at=expires_at OR expires_at>" _UNIXTIME_ ")"
                                          " ORDER BY expires_at");
    }
    else
    {
        queryResult = LoginDatabase.PQuery("SELECT ip,banned_at,expires_at,banned_by,reason FROM ip_banned"
                                           " WHERE (banned_at=expires_at OR expires_at>" _UNIXTIME_ ") AND ip " _LIKE_ " " _CONCAT3_("'%%'", "'%s'", "'%%'")
                                           " ORDER BY expires_at", filter.c_str());
    }

    if (!queryResult)
    {
        PSendSysMessage(LANG_BANLIST_NOIP);
        return true;
    }

    PSendSysMessage(LANG_BANLIST_MATCHINGIP);
    // Chat short output
    if (m_session)
    {
        do
        {
            Field* fields = queryResult->Fetch();
            PSendSysMessage("%s", fields[0].GetString());
        }
        while (queryResult->NextRow());
    }
    // Console wide output
    else
    {
        SendSysMessage(LANG_BANLIST_IPS);
        SendSysMessage("===============================================================================");
        SendSysMessage(LANG_BANLIST_IPS_HEADER);
        do
        {
            SendSysMessage("-------------------------------------------------------------------------------");
            Field* fields = queryResult->Fetch();
            time_t t_ban = fields[1].GetUInt64();
            tm* aTm_ban = localtime(&t_ban);
            if (fields[1].GetUInt64() == fields[2].GetUInt64())
            {
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|   permanent  |%-15.15s|%-15.15s|",
                                fields[0].GetString(), aTm_ban->tm_year % 100, aTm_ban->tm_mon + 1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                                fields[3].GetString(), fields[4].GetString());
            }
            else
            {
                time_t t_unban = fields[2].GetUInt64();
                tm* aTm_unban = localtime(&t_unban);
                PSendSysMessage("|%-15.15s|%02d-%02d-%02d %02d:%02d|%02d-%02d-%02d %02d:%02d|%-15.15s|%-15.15s|",
                                fields[0].GetString(), aTm_ban->tm_year % 100, aTm_ban->tm_mon + 1, aTm_ban->tm_mday, aTm_ban->tm_hour, aTm_ban->tm_min,
                                aTm_unban->tm_year % 100, aTm_unban->tm_mon + 1, aTm_unban->tm_mday, aTm_unban->tm_hour, aTm_unban->tm_min,
                                fields[3].GetString(), fields[4].GetString());
            }
        }
        while (queryResult->NextRow());
        SendSysMessage("===============================================================================");
    }

    return true;
}

bool ChatHandler::HandleRespawnCommand(char* /*args*/)
{
    Player* pl = m_session->GetPlayer();

    // accept only explicitly selected target (not implicitly self targeting case)
    Unit* target = getSelectedUnit();
    if (pl->GetSelectionGuid() && target)
    {
        if (target->GetTypeId() != TYPEID_UNIT)
        {
            SendSysMessage(LANG_SELECT_CREATURE);
            SetSentErrorMessage(true);
            return false;
        }

        if (target->IsDead())
        {
            Creature* creature = static_cast<Creature*>(target);
            if (target->IsUsingNewSpawningSystem())
            {
                if (!creature->GetCreatureGroup())
                {
                    if (creature->GetMap()->GetMapDataContainer().GetSpawnGroupByGuid(creature->GetDbGuid(), TYPEID_UNIT))
                        target->GetMap()->GetPersistentState()->SaveCreatureRespawnTime(target->GetDbGuid(), time(nullptr));
                    else
                        target->GetMap()->GetSpawnManager().RespawnCreature(target->GetDbGuid(), 0);
                }
            }
            else
                creature->Respawn();
        }
        return true;
    }

    MaNGOS::RespawnDo u_do;
    MaNGOS::WorldObjectWorker<MaNGOS::RespawnDo> worker(u_do);
    Cell::VisitGridObjects(pl, worker, pl->GetVisibilityData().GetVisibilityDistance());

    pl->GetMap()->GetSpawnManager().RespawnSpawnGroupsInVicinity(pl->GetPosition(), pl->GetVisibilityData().GetVisibilityDistance());
    return true;
}

bool ChatHandler::HandleGMFlyCommand(char* args)
{
    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    Player* target = getSelectedPlayer();
    if (!target)
        target = m_session->GetPlayer();

    // [-ZERO] Need reimplement in another way
    {
        SendSysMessage(LANG_USE_BOL);
        return false;
    }
    target->SetCanFly(value);
    PSendSysMessage(LANG_COMMAND_FLYMODE_STATUS, GetNameLink(target).c_str(), args);
    return true;
}

bool ChatHandler::HandleGMUnkillableCommand(char* args)
{
    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }
    Player* target = m_session->GetPlayer();
    target->SetDeathPrevention(value);
    PSendSysMessage("GM Unkillability %s.", value ? "enabled" : "disabled");
    return true;
}

bool ChatHandler::HandlePDumpLoadCommand(char* args)
{
    char* file = ExtractQuotedOrLiteralArg(&args);
    if (!file)
        return false;

    std::string account_name;
    uint32 account_id = ExtractAccountId(&args, &account_name);
    if (!account_id)
        return false;

    char* name_str = ExtractLiteralArg(&args);

    uint32 lowguid = 0;
    std::string name;

    if (name_str)
    {
        name = name_str;
        // normalize the name if specified and check if it exists
        if (!normalizePlayerName(name))
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        if (ObjectMgr::CheckPlayerName(name, true) != CHAR_NAME_SUCCESS)
        {
            PSendSysMessage(LANG_INVALID_CHARACTER_NAME);
            SetSentErrorMessage(true);
            return false;
        }

        if (*args)
        {
            if (!ExtractUInt32(&args, lowguid))
                return false;

            if (!lowguid)
            {
                PSendSysMessage(LANG_INVALID_CHARACTER_GUID);
                SetSentErrorMessage(true);
                return false;
            }

            ObjectGuid guid = ObjectGuid(HIGHGUID_PLAYER, lowguid);

            if (sObjectMgr.GetPlayerAccountIdByGUID(guid))
            {
                PSendSysMessage(LANG_CHARACTER_GUID_IN_USE, lowguid);
                SetSentErrorMessage(true);
                return false;
            }
        }
    }

    switch (PlayerDumpReader::LoadDump(file, account_id, name, lowguid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_IMPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL, file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_FILE_BROKEN:
            PSendSysMessage(LANG_DUMP_BROKEN, file);
            SetSentErrorMessage(true);
            return false;
        case DUMP_TOO_MANY_CHARS:
            PSendSysMessage(LANG_ACCOUNT_CHARACTER_LIST_FULL, account_name.c_str(), account_id);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_IMPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandlePDumpWriteCommand(char* args)
{
    if (!*args)
        return false;

    char* file = ExtractQuotedOrLiteralArg(&args);
    if (!file)
        return false;

    char* p2 = ExtractLiteralArg(&args);

    uint32 lowguid;
    ObjectGuid guid;
    // character name can't start from number
    if (!ExtractUInt32(&p2, lowguid))
    {
        std::string name = ExtractPlayerNameFromLink(&p2);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        guid = sObjectMgr.GetPlayerGuidByName(name);
        if (!guid)
        {
            PSendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        lowguid = guid.GetCounter();
    }
    else
        guid = ObjectGuid(HIGHGUID_PLAYER, lowguid);

    if (!sObjectMgr.GetPlayerAccountIdByGUID(guid))
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    switch (PlayerDumpWriter().WriteDump(file, lowguid))
    {
        case DUMP_SUCCESS:
            PSendSysMessage(LANG_COMMAND_EXPORT_SUCCESS);
            break;
        case DUMP_FILE_OPEN_ERROR:
            PSendSysMessage(LANG_FILE_OPEN_FAIL, file);
            SetSentErrorMessage(true);
            return false;
        default:
            PSendSysMessage(LANG_COMMAND_EXPORT_FAILED);
            SetSentErrorMessage(true);
            return false;
    }

    return true;
}

bool ChatHandler::HandleMovegensCommand(char* /*args*/)
{
    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("%s movement generators stack:", unit->GetGuidStr().c_str());

    MotionMaster* mm = unit->GetMotionMaster();
    float x, y, z;
    mm->GetDestination(x, y, z);
    for (MotionMaster::const_iterator itr = mm->begin(); itr != mm->end(); ++itr)
    {
        switch ((*itr)->GetMovementGeneratorType())
        {
            case IDLE_MOTION_TYPE:          SendSysMessage(LANG_MOVEGENS_IDLE);          break;
            case RANDOM_MOTION_TYPE:        SendSysMessage(LANG_MOVEGENS_RANDOM);        break;
            case WAYPOINT_MOTION_TYPE:      SendSysMessage(LANG_MOVEGENS_WAYPOINT);      break;
            case CONFUSED_MOTION_TYPE:      SendSysMessage(LANG_MOVEGENS_CONFUSED);      break;

            case CHASE_MOTION_TYPE:
            {
                Unit* target = nullptr;
                float distance = 0.f;
                float angle = 0.f;
                ChaseMovementGenerator const* movegen = static_cast<ChaseMovementGenerator const*>(*itr);
                target = movegen->GetCurrentTarget();
                distance = movegen->GetOffset();
                angle = movegen->GetAngle();

                if (!target)
                    SendSysMessage(LANG_MOVEGENS_CHASE_NULL);
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    PSendSysMessage(LANG_MOVEGENS_CHASE_PLAYER, target->GetName(), target->GetGUIDLow(), distance, angle, ChaseModes[movegen->GetCurrentMode()]);
                else
                    PSendSysMessage(LANG_MOVEGENS_CHASE_CREATURE, target->GetName(), target->GetGUIDLow(), distance, angle, ChaseModes[movegen->GetCurrentMode()]);
                break;
            }
            case FOLLOW_MOTION_TYPE:
            {
                Unit* target = static_cast<FollowMovementGenerator const*>(*itr)->GetCurrentTarget();

                if (!target)
                    SendSysMessage(LANG_MOVEGENS_FOLLOW_NULL);
                else if (target->GetTypeId() == TYPEID_PLAYER)
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_PLAYER, target->GetName(), target->GetGUIDLow());
                else
                    PSendSysMessage(LANG_MOVEGENS_FOLLOW_CREATURE, target->GetName(), target->GetGUIDLow());
                break;
            }
            case HOME_MOTION_TYPE:
                if (unit->GetTypeId() == TYPEID_UNIT)
                {
                    PSendSysMessage(LANG_MOVEGENS_HOME_CREATURE, x, y, z);
                }
                else
                    SendSysMessage(LANG_MOVEGENS_HOME_PLAYER);
                break;
            case TAXI_MOTION_TYPE:     SendSysMessage(LANG_MOVEGENS_FLIGHT);  break;
            case POINT_MOTION_TYPE:
            {
                PSendSysMessage(LANG_MOVEGENS_POINT, x, y, z);
                break;
            }
            case FLEEING_MOTION_TYPE:  SendSysMessage(LANG_MOVEGENS_FEAR);    break;
            case DISTRACT_MOTION_TYPE: SendSysMessage(LANG_MOVEGENS_DISTRACT);  break;
            case EFFECT_MOTION_TYPE: SendSysMessage(LANG_MOVEGENS_EFFECT);  break;
            case FORMATION_MOTION_TYPE:
                PSendSysMessage("   Formation movement, following [%s]", unit->GetFormationSlot()->GetMaster()->GetGuidStr().c_str());
                break;
            default:
                PSendSysMessage(LANG_MOVEGENS_UNKNOWN, (*itr)->GetMovementGeneratorType());
                break;
        }
    }
    return true;
}

bool ChatHandler::HandleMovespeedShowCommand(char* /*args*/)
{
    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    PSendSysMessage("%s speeds:", unit->GetName());
    PSendSysMessage("Walk speed %f, Run speed %f, Swim speed %f", unit->GetSpeed(MOVE_WALK), unit->GetSpeed(MOVE_RUN), unit->GetSpeed(MOVE_SWIM));

    return true;
}

bool ChatHandler::HandleDebugMovement(char* args)
{
    Unit* unit = getSelectedUnit();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    bool value;
    if (!ExtractOnOff(&args, value))
    {
        SendSysMessage(LANG_USE_BOL);
        SetSentErrorMessage(true);
        return false;
    }

    unit->SetDebuggingMovement(value);
    PSendSysMessage("New value: %s", (value ? "true" : "false"));
    return true;
}

bool ChatHandler::HandlePrintMovement(char* args)
{
    Creature* unit = getSelectedCreature();
    if (!unit)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    MotionMaster* mm = unit->GetMotionMaster();
    if (mm->top()->GetMovementGeneratorType() == CHASE_MOTION_TYPE)
    {
        ChaseMovementGenerator* chaseM = static_cast<ChaseMovementGenerator*>(mm->top());
        auto data = chaseM->GetPrintout();
        PSendSysMessage("%s", data.first.data());
        sLog.outCustomLog("%s", data.second.data());
    }
    return true;
}

bool ChatHandler::HandleServerPLimitCommand(char* args)
{
    if (*args)
    {
        char* param = ExtractLiteralArg(&args);
        if (!param)
            return false;

        int32 val;
        if (param[0] =='p' || param[0] == 'P')
            val = -int32(SEC_PLAYER);
        else if (param[0] == 'm' || param[0] == 'M')
            val = -int32(SEC_MODERATOR);
        else if (param[0] == 'g' || param[0] == 'G')
            val = -int32(SEC_GAMEMASTER);
        else if (param[0] == 'a' || param[0] == 'A')
            val = -int32(SEC_ADMINISTRATOR);
        else if (param[0] == 'r' || param[0] == 'R')
            val = sConfig.GetIntDefault("PlayerLimit", DEFAULT_PLAYER_LIMIT);
        else if (ExtractInt32(&param, val))
        {
            if (val < -int32(SEC_ADMINISTRATOR))
                val = -int32(SEC_ADMINISTRATOR);
        }
        else
            return false;

        sWorld.SetPlayerLimit(val);

        // kick all low security level players
        if (sWorld.GetPlayerAmountLimit() > SEC_PLAYER)
            sWorld.KickAllLess(sWorld.GetPlayerSecurityLimit());
    }

    uint32 pLimit = sWorld.GetPlayerAmountLimit();
    AccountTypes allowedAccountType = sWorld.GetPlayerSecurityLimit();
    char const* secName;
    switch (allowedAccountType)
    {
        case SEC_PLAYER:        secName = "Player";        break;
        case SEC_MODERATOR:     secName = "Moderator";     break;
        case SEC_GAMEMASTER:    secName = "Gamemaster";    break;
        case SEC_ADMINISTRATOR: secName = "Administrator"; break;
        default:                secName = "<unknown>";     break;
    }

    PSendSysMessage("Player limits: amount %u, min. security level %s.", pLimit, secName);

    return true;
}

bool ChatHandler::HandleCastCommand(char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit(false);

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
        SetSentErrorMessage(true);
        return false;
    }

    bool triggered = ExtractLiteralArg(&args, "triggered") != nullptr;
    if (!triggered && *args)                                // can be fail also at syntax error
        return false;

    if (spellInfo->Targets && !target)
        target = m_session->GetPlayer();

    SpellCastResult result = m_session->GetPlayer()->CastSpell(target, spell, triggered ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
    if (result != SPELL_CAST_OK)
        PSendSysMessage("Spell resulted in fail %u", uint32(result));

    return true;
}

bool ChatHandler::HandleCastBackCommand(char* args)
{
    Creature* caster = getSelectedCreature();

    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    bool triggered = ExtractLiteralArg(&args, "triggered") != nullptr;
    if (!triggered && *args)                                // can be fail also at syntax error
        return false;

    caster->SetFacingToObject(m_session->GetPlayer());

    SpellCastResult result = caster->CastSpell(m_session->GetPlayer(), spell, triggered ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
    if (result != SPELL_CAST_OK)
        PSendSysMessage("Spell resulted in fail %u", uint32(result));

    return true;
}

bool ChatHandler::HandleCastDistCommand(char* args)
{
    if (!*args)
        return false;

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
        SetSentErrorMessage(true);
        return false;
    }

    float dist;
    if (!ExtractFloat(&args, dist))
        return false;

    bool triggered = ExtractLiteralArg(&args, "triggered") != nullptr;
    if (!triggered && *args)                                // can be fail also at syntax error
        return false;

    float x, y, z;
    m_session->GetPlayer()->GetClosePoint(x, y, z, dist);

    SpellCastResult result = m_session->GetPlayer()->CastSpell(x, y, z, spell, triggered ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
    if (result != SPELL_CAST_OK)
        PSendSysMessage("Spell resulted in fail %u", uint32(result));
    return true;
}

bool ChatHandler::HandleCastTargetCommand(char* args)
{
    Creature* caster = getSelectedCreature();

    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    if (!caster->GetVictim())
    {
        SendSysMessage(LANG_SELECTED_TARGET_NOT_HAVE_VICTIM);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell || !sSpellTemplate.LookupEntry<SpellEntry>(spell))
        return false;

    bool triggered = ExtractLiteralArg(&args, "triggered") != nullptr;
    if (!triggered && *args)                                // can be fail also at syntax error
        return false;

    caster->SetFacingToObject(m_session->GetPlayer());

    SpellCastResult result = caster->CastSpell(caster->GetVictim(), spell, triggered ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
    if (result != SPELL_CAST_OK)
        PSendSysMessage("Spell resulted in fail %u", uint32(result));

    return true;
}

/*
ComeToMe command REQUIRED for 3rd party scripting library to have access to PointMovementGenerator
Without this function 3rd party scripting library will get linking errors (unresolved external)
when attempting to use the PointMovementGenerator
*/
bool ChatHandler::HandleComeToMeCommand(char* /*args*/)
{
    Creature* caster = getSelectedCreature();

    if (!caster)
    {
        SendSysMessage(LANG_SELECT_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    Player* pl = m_session->GetPlayer();

    caster->GetMotionMaster()->MovePoint(0, pl->GetPositionX(), pl->GetPositionY(), pl->GetPositionZ());
    return true;
}

bool ChatHandler::HandleCastSelfCommand(char* args)
{
    if (!*args)
        return false;

    Unit* target = getSelectedUnit();

    if (!target)
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
    uint32 spell = ExtractSpellIdFromLink(&args);
    if (!spell)
        return false;

    SpellEntry const* spellInfo = sSpellTemplate.LookupEntry<SpellEntry>(spell);
    if (!spellInfo)
        return false;

    if (!SpellMgr::IsSpellValid(spellInfo, m_session->GetPlayer()))
    {
        PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spell);
        SetSentErrorMessage(true);
        return false;
    }

    bool triggered = ExtractLiteralArg(&args, "triggered") != nullptr;
    if (!triggered && *args)                                // can be fail also at syntax error
        return false;

    SpellCastResult result = target->CastSpell(target, spell, triggered ? TRIGGERED_OLD_TRIGGERED : TRIGGERED_NONE);
    if (result != SPELL_CAST_OK)
        PSendSysMessage("Spell resulted in fail %u", uint32(result));

    return true;
}

bool ChatHandler::HandleInstanceListBindsCommand(char* /*args*/)
{
    Player* player = getSelectedPlayer();
    if (!player) player = m_session->GetPlayer();
    uint32 counter = 0;

    Player::BoundInstancesMap& binds = player->GetBoundInstances();
    for (Player::BoundInstancesMap::const_iterator itr = binds.begin(); itr != binds.end(); ++itr)
    {
        DungeonPersistentState* state = itr->second.state;
        std::string timeleft = secsToTimeString(state->GetResetTime() - time(nullptr), true);
        if (const MapEntry* entry = sMapStore.LookupEntry(itr->first))
        {
            PSendSysMessage("map: %d (%s) inst: %d perm: %s canReset: %s TTR: %s",
                            itr->first, entry->name[GetSessionDbcLocale()], state->GetInstanceId(), itr->second.perm ? "yes" : "no",
                            state->CanReset() ? "yes" : "no", timeleft.c_str());
        }
        else
            PSendSysMessage("bound for a nonexistent map %u", itr->first);
        counter++;
    }

    PSendSysMessage("player binds: %d", counter);
    counter = 0;

    if (Group* group = player->GetGroup())
    {
        Group::BoundInstancesMap& binds = group->GetBoundInstances();
        for (Group::BoundInstancesMap::const_iterator itr = binds.begin(); itr != binds.end(); ++itr)
        {
            DungeonPersistentState* state = itr->second.state;
            std::string timeleft = secsToTimeString(state->GetResetTime() - time(nullptr), true);

            if (const MapEntry* entry = sMapStore.LookupEntry(itr->first))
            {
                PSendSysMessage("map: %d (%s) inst: %d perm: %s canReset: %s TTR: %s",
                                itr->first, entry->name[GetSessionDbcLocale()], state->GetInstanceId(), itr->second.perm ? "yes" : "no",
                                state->CanReset() ? "yes" : "no", timeleft.c_str());
            }
            else
                PSendSysMessage("bound for a nonexistent map %u", itr->first);
            counter++;
        }
    }
    PSendSysMessage("group binds: %d", counter);

    return true;
}

bool ChatHandler::HandleInstanceUnbindCommand(char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();
    if (!player)
        player = m_session->GetPlayer();
    uint32 counter = 0;
    uint32 mapid = 0;
    bool got_map = false;

    if (strncmp(args, "all", strlen(args)) != 0)
    {
        if (!isNumeric(args[0]))
            return false;

        got_map = true;
        mapid = atoi(args);
    }

    Player::BoundInstancesMap& binds = player->GetBoundInstances();
    for (Player::BoundInstancesMap::iterator itr = binds.begin(); itr != binds.end();)
    {
        if (got_map && mapid != itr->first)
        {
            ++itr;
            continue;
        }
        if (itr->first != player->GetMapId())
        {
            DungeonPersistentState* save = itr->second.state;
            std::string timeleft = secsToTimeString(save->GetResetTime() - time(nullptr), true);

            if (const MapEntry* entry = sMapStore.LookupEntry(itr->first))
            {
                PSendSysMessage("unbinding map: %d (%s) inst: %d perm: %s canReset: %s TTR: %s",
                                itr->first, entry->name[GetSessionDbcLocale()], save->GetInstanceId(), itr->second.perm ? "yes" : "no",
                                save->CanReset() ? "yes" : "no", timeleft.c_str());
            }
            else
                PSendSysMessage("bound for a nonexistent map %u", itr->first);
            player->UnbindInstance(itr);
            counter++;
        }
        else
            ++itr;
    }
    PSendSysMessage("instances unbound: %d", counter);

    return true;
}

bool ChatHandler::HandleInstanceStatsCommand(char* /*args*/)
{
    PSendSysMessage("instances loaded: %d", sMapMgr.GetNumInstances());
    PSendSysMessage("players in instances: %d", sMapMgr.GetNumPlayersInInstances());

    uint32 numSaves, numBoundPlayers, numBoundGroups;
    sMapPersistentStateMgr.GetStatistics(numSaves, numBoundPlayers, numBoundGroups);
    PSendSysMessage("instance saves: %d", numSaves);
    PSendSysMessage("players bound: %d", numBoundPlayers);
    PSendSysMessage("groups bound: %d", numBoundGroups);
    return true;
}

bool ChatHandler::HandleInstanceSaveDataCommand(char* /*args*/)
{
    Player* pl = m_session->GetPlayer();

    Map* map = pl->GetMap();

    InstanceData* iData = map->GetInstanceData();
    if (!iData)
    {
        PSendSysMessage("Map has no instance data.");
        SetSentErrorMessage(true);
        return false;
    }

    iData->SaveToDB();
    return true;
}

/// Display the list of GMs
bool ChatHandler::HandleGMListFullCommand(char* /*args*/)
{
    ///- Get the accounts with GM Level >0
    auto queryResult = LoginDatabase.Query("SELECT username,gmlevel FROM account WHERE gmlevel > 0");
    if (queryResult)
    {
        SendSysMessage(LANG_GMLIST);
        SendSysMessage("========================");
        SendSysMessage(LANG_GMLIST_HEADER);
        SendSysMessage("========================");

        ///- Circle through them. Display username and GM level
        do
        {
            Field* fields = queryResult->Fetch();
            PSendSysMessage("|%15s|%6s|", fields[0].GetString(), fields[1].GetString());
        }
        while (queryResult->NextRow());

        PSendSysMessage("========================");
    }
    else
        PSendSysMessage(LANG_GMLIST_EMPTY);
    return true;
}

/// Define the 'Message of the day' for the realm
bool ChatHandler::HandleServerSetMotdCommand(char* args)
{
    sWorld.SetMotd(args);
    PSendSysMessage(LANG_MOTD_NEW, args);
    return true;
}

bool ChatHandler::ShowPlayerListHelper(std::unique_ptr<QueryResult> queryResult, uint32* limit, bool title, bool error)
{
    if (!queryResult)
    {
        if (error)
        {
            PSendSysMessage(LANG_NO_PLAYERS_FOUND);
            SetSentErrorMessage(true);
        }
        return false;
    }

    if (!m_session && title)
    {
        SendSysMessage(LANG_CHARACTERS_LIST_BAR);
        SendSysMessage(LANG_CHARACTERS_LIST_HEADER);
        SendSysMessage(LANG_CHARACTERS_LIST_BAR);
    }

    if (queryResult)
    {
        ///- Circle through them. Display username and GM level
        do
        {
            // check limit
            if (limit)
            {
                if (*limit == 0)
                    break;
                --*limit;
            }

            Field* fields = queryResult->Fetch();
            uint32 guid      = fields[0].GetUInt32();
            std::string name = fields[1].GetCppString();
            uint8 race       = fields[2].GetUInt8();
            uint8 class_     = fields[3].GetUInt8();
            uint32 level     = fields[4].GetUInt32();

            ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(race);
            ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(class_);

            char const* race_name = raceEntry   ? raceEntry->name[GetSessionDbcLocale()] : "<?>";
            char const* class_name = classEntry ? classEntry->name[GetSessionDbcLocale()] : "<?>";

            if (!m_session)
                PSendSysMessage(LANG_CHARACTERS_LIST_LINE_CONSOLE, guid, name.c_str(), race_name, class_name, level);
            else
                PSendSysMessage(LANG_CHARACTERS_LIST_LINE_CHAT, guid, name.c_str(), name.c_str(), race_name, class_name, level);
        }
        while (queryResult->NextRow());
    }

    if (!m_session)
        SendSysMessage(LANG_CHARACTERS_LIST_BAR);

    return true;
}


/// Output list of character for account
bool ChatHandler::HandleAccountCharactersCommand(char* args)
{
    ///- Get the command line arguments
    std::string account_name;
    Player* target = nullptr;                                  // only for triggering use targeted player account
    uint32 account_id = ExtractAccountId(&args, &account_name, &target);
    if (!account_id)
        return false;

    ///- Get the characters for account id
    auto queryResult = CharacterDatabase.PQuery("SELECT guid, name, race, class, level FROM characters WHERE account = %u", account_id);

    return ShowPlayerListHelper(std::move(queryResult));
}

/// Set/Unset the expansion level for an account
bool ChatHandler::HandleAccountSetAddonCommand(char* args)
{
    ///- Get the command line arguments
    char* accountStr = ExtractOptNotLastArg(&args);

    std::string account_name;
    uint32 account_id = ExtractAccountId(&accountStr, &account_name);
    if (!account_id)
        return false;

    // Let set addon state only for lesser (strong) security level
    // or to self account
    if (GetAccountId() && GetAccountId() != account_id &&
            HasLowerSecurityAccount(nullptr, account_id, true))
        return false;

    uint32 lev;
    if (!ExtractUInt32(&args, lev))
        return false;

    // No SQL injection
    LoginDatabase.PExecute("UPDATE account SET expansion = '%u' WHERE id = '%u'", lev, account_id);
    PSendSysMessage(LANG_ACCOUNT_SETADDON, account_name.c_str(), account_id, lev);
    return true;
}

bool ChatHandler::HandleSendMailHelper(MailDraft& draft, char* args)
{
    // format: "subject text" "mail text"
    char* msgSubject = ExtractQuotedArg(&args);
    if (!msgSubject)
        return false;

    char* msgText = ExtractQuotedArg(&args);
    if (!msgText)
        return false;

    // msgSubject, msgText isn't NUL after prev. check
    draft.SetSubjectAndBody(msgSubject, msgText);

    return true;
}

bool ChatHandler::HandleSendMassMailCommand(char* args)
{
    // format: raceMask "subject text" "mail text"
    uint32 raceMask = 0;
    char const* name = nullptr;

    if (!ExtractRaceMask(&args, raceMask, &name))
        return false;

    // need dynamic object because it trasfered to mass mailer
    MailDraft* draft = new MailDraft;

    // fill mail
    if (!HandleSendMailHelper(*draft, args))
    {
        delete draft;
        return false;
    }

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    sMassMailMgr.AddMassMailTask(draft, sender, raceMask);

    PSendSysMessage(LANG_MAIL_SENT, name);
    return true;
}



bool ChatHandler::HandleSendItemsHelper(MailDraft& draft, char* args)
{
    // format: "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]
    char* msgSubject = ExtractQuotedArg(&args);
    if (!msgSubject)
        return false;

    char* msgText = ExtractQuotedArg(&args);
    if (!msgText)
        return false;

    // extract items
    typedef std::pair<uint32, uint32> ItemPair;
    typedef std::list< ItemPair > ItemPairs;
    ItemPairs items;

    // get from tail next item str
    while (char* itemStr = ExtractArg(&args))
    {
        // parse item str
        uint32 item_id = 0;
        uint32 item_count = 1;
        if (sscanf(itemStr, "%u:%u", &item_id, &item_count) != 2)
            if (sscanf(itemStr, "%u", &item_id) != 1)
                return false;

        if (!item_id)
        {
            PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            SetSentErrorMessage(true);
            return false;
        }

        ItemPrototype const* item_proto = ObjectMgr::GetItemPrototype(item_id);
        if (!item_proto)
        {
            PSendSysMessage(LANG_COMMAND_ITEMIDINVALID, item_id);
            SetSentErrorMessage(true);
            return false;
        }

        if (item_count < 1 || (item_proto->MaxCount > 0 && item_count > uint32(item_proto->MaxCount)))
        {
            PSendSysMessage(LANG_COMMAND_INVALID_ITEM_COUNT, item_count, item_id);
            SetSentErrorMessage(true);
            return false;
        }

        while (item_count > item_proto->GetMaxStackSize())
        {
            items.push_back(ItemPair(item_id, item_proto->GetMaxStackSize()));
            item_count -= item_proto->GetMaxStackSize();
        }

        items.push_back(ItemPair(item_id, item_count));

        if (items.size() > MAX_MAIL_ITEMS)
        {
            PSendSysMessage(LANG_COMMAND_MAIL_ITEMS_LIMIT, MAX_MAIL_ITEMS);
            SetSentErrorMessage(true);
            return false;
        }
    }

    // fill mail
    draft.SetSubjectAndBody(msgSubject, msgText);

    for (ItemPairs::const_iterator itr = items.begin(); itr != items.end(); ++itr)
    {
        if (Item* item = Item::CreateItem(itr->first, itr->second, m_session ? m_session->GetPlayer() : nullptr))
        {
            item->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted
            draft.AddItem(item);
        }
    }

    return true;
}

bool ChatHandler::HandleSendItemsCommand(char* args)
{
    // format: name "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]
    Player* receiver;
    ObjectGuid receiver_guid;
    std::string receiver_name;
    if (!ExtractPlayerTarget(&args, &receiver, &receiver_guid, &receiver_name))
        return false;

    MailDraft draft;

    // fill mail
    if (!HandleSendItemsHelper(draft, args))
        return false;

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    draft.SendMailTo(MailReceiver(receiver, receiver_guid), sender);

    std::string nameLink = playerLink(receiver_name);
    PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
    return true;
}

bool ChatHandler::HandleSendMassItemsCommand(char* args)
{
    // format: racemask "subject text" "mail text" item1[:count1] item2[:count2] ... item12[:count12]

    uint32 raceMask = 0;
    char const* name = nullptr;

    if (!ExtractRaceMask(&args, raceMask, &name))
        return false;

    // need dynamic object because it trasfered to mass mailer
    MailDraft* draft = new MailDraft;


    // fill mail
    if (!HandleSendItemsHelper(*draft, args))
    {
        delete draft;
        return false;
    }

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    sMassMailMgr.AddMassMailTask(draft, sender, raceMask);

    PSendSysMessage(LANG_MAIL_SENT, name);
    return true;
}

bool ChatHandler::HandleSendMoneyHelper(MailDraft& draft, char* args)
{
    /// format: "subject text" "mail text" money

    char* msgSubject = ExtractQuotedArg(&args);
    if (!msgSubject)
        return false;

    char* msgText = ExtractQuotedArg(&args);
    if (!msgText)
        return false;

    uint32 money;
    if (!ExtractUInt32(&args, money))
        return false;

    if (money <= 0)
        return false;

    // msgSubject, msgText isn't NUL after prev. check
    draft.SetSubjectAndBody(msgSubject, msgText).SetMoney(money);

    return true;
}

bool ChatHandler::HandleSendMoneyCommand(char* args)
{
    /// format: name "subject text" "mail text" money

    Player* receiver;
    ObjectGuid receiver_guid;
    std::string receiver_name;
    if (!ExtractPlayerTarget(&args, &receiver, &receiver_guid, &receiver_name))
        return false;

    MailDraft draft;

    // fill mail
    if (!HandleSendMoneyHelper(draft, args))
        return false;

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    draft.SendMailTo(MailReceiver(receiver, receiver_guid), sender);

    std::string nameLink = playerLink(receiver_name);
    PSendSysMessage(LANG_MAIL_SENT, nameLink.c_str());
    return true;
}

bool ChatHandler::HandleSendMassMoneyCommand(char* args)
{
    /// format: raceMask "subject text" "mail text" money

    uint32 raceMask = 0;
    char const* name = nullptr;

    if (!ExtractRaceMask(&args, raceMask, &name))
        return false;

    // need dynamic object because it trasfered to mass mailer
    MailDraft* draft = new MailDraft;

    // fill mail
    if (!HandleSendMoneyHelper(*draft, args))
    {
        delete draft;
        return false;
    }

    // from console show nonexistent sender
    MailSender sender(MAIL_NORMAL, m_session ? m_session->GetPlayer()->GetObjectGuid().GetCounter() : 0, MAIL_STATIONERY_GM);

    sMassMailMgr.AddMassMailTask(draft, sender, raceMask);

    PSendSysMessage(LANG_MAIL_SENT, name);
    return true;
}

/// Send a message to a player in game
bool ChatHandler::HandleSendMessageCommand(char* args)
{
    ///- Find the player
    Player* rPlayer;
    if (!ExtractPlayerTarget(&args, &rPlayer))
        return false;

    ///- message
    if (!*args)
        return false;

    WorldSession* rPlayerSession = rPlayer->GetSession();

    ///- Check that he is not logging out.
    if (rPlayerSession->isLogingOut())
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    ///- Send the message
    // Use SendAreaTriggerMessage for fastest delivery.
    rPlayerSession->SendAreaTriggerMessage("%s", args);
    rPlayerSession->SendAreaTriggerMessage("|cffff0000[Message from administrator]:|r");

    // Confirmation message
    std::string nameLink = GetNameLink(rPlayer);
    PSendSysMessage(LANG_SENDMESSAGE, nameLink.c_str(), args);
    return true;
}

bool ChatHandler::HandleModifyGenderCommand(char* args)
{
    if (!*args)
        return false;

    Player* player = getSelectedPlayer();

    if (!player)
    {
        PSendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    PlayerInfo const* info = sObjectMgr.GetPlayerInfo(player->getRace(), player->getClass());
    if (!info)
        return false;

    char* gender_str = args;
    int gender_len = strlen(gender_str);

    Gender gender;

    if (!strncmp(gender_str, "male", gender_len))           // MALE
    {
        if (player->getGender() == GENDER_MALE)
            return true;

        gender = GENDER_MALE;
    }
    else if (!strncmp(gender_str, "female", gender_len))    // FEMALE
    {
        if (player->getGender() == GENDER_FEMALE)
            return true;

        gender = GENDER_FEMALE;
    }
    else
    {
        SendSysMessage(LANG_MUST_MALE_OR_FEMALE);
        SetSentErrorMessage(true);
        return false;
    }

    // Set gender
    player->SetByteValue(UNIT_FIELD_BYTES_0, UNIT_BYTES_0_OFFSET_GENDER, gender);
    player->SetUInt16Value(PLAYER_BYTES_3, 0, uint16(gender) | (player->GetDrunkValue() & 0xFFFE));

    // Change display ID
    player->InitDisplayIds();

    char const* gender_full = gender ? "female" : "male";

    PSendSysMessage(LANG_YOU_CHANGE_GENDER, player->GetName(), gender_full);

    if (needReportToTarget(player))
        ChatHandler(player).PSendSysMessage(LANG_YOUR_GENDER_CHANGED, gender_full, GetNameLink().c_str());

    return true;
}

bool ChatHandler::HandleMmap(char* args)
{
    bool on;
    if (ExtractOnOff(&args, on))
    {
        if (on)
        {
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, true);
            SendSysMessage("WORLD: mmaps are now ENABLED (individual map settings still in effect)");
        }
        else
        {
            sWorld.setConfig(CONFIG_BOOL_MMAP_ENABLED, false);
            SendSysMessage("WORLD: mmaps are now DISABLED");
        }
        return true;
    }

    on = sWorld.getConfig(CONFIG_BOOL_MMAP_ENABLED);
    PSendSysMessage("mmaps are %sabled", on ? "en" : "dis");

    return true;
}

bool ChatHandler::HandleMmapTestArea(char* args)
{
    float radius = 40.0f;
    ExtractFloat(&args, radius);

    CreatureList creatureList;
    MaNGOS::AnyUnitInObjectRangeCheck go_check(m_session->GetPlayer(), radius);
    MaNGOS::CreatureListSearcher<MaNGOS::AnyUnitInObjectRangeCheck> go_search(creatureList, go_check);
    // Get Creatures
    Cell::VisitGridObjects(m_session->GetPlayer(), go_search, radius);

    if (!creatureList.empty())
    {
        PSendSysMessage("Found " SIZEFMTD " Creatures.", creatureList.size());

        uint32 paths = 0;
        uint32 uStartTime = WorldTimer::getMSTime();

        float gx, gy, gz;
        m_session->GetPlayer()->GetPosition(gx, gy, gz);
        for (auto& itr : creatureList)
        {
            PathFinder path(itr);
            path.calculate(gx, gy, gz);
            ++paths;
        }

        uint32 uPathLoadTime = WorldTimer::getMSTimeDiff(uStartTime, WorldTimer::getMSTime());
        PSendSysMessage("Generated %i paths in %i ms", paths, uPathLoadTime);
    }
    else
    {
        PSendSysMessage("No creatures in %f yard range.", radius);
    }

    return true;
}

// use ".mmap testheight 10" selecting any creature/player
bool ChatHandler::HandleMmapTestHeight(char* args)
{
    float radius = 0.0f;
    ExtractFloat(&args, radius);
    if (radius > 40.0f)
        radius = 40.0f;

    Unit* unit = getSelectedUnit();

    Player* player = m_session->GetPlayer();
    if (!unit)
        unit = player;

    if (unit->GetTypeId() == TYPEID_UNIT)
    {
        if (radius < 0.1f)
            radius = static_cast<Creature*>(unit)->GetRespawnRadius();
    }
    else
    {
        if (unit->GetTypeId() != TYPEID_PLAYER)
        {
            PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            return false;
        }
    }

    if (radius < 0.1f)
    {
        PSendSysMessage("Provided spawn radius for %s is too small. Using 5.0f instead.", unit->GetGuidStr().c_str());
        radius = 5.0f;
    }

    float gx, gy, gz;
    unit->GetPosition(gx, gy, gz);

    Creature* summoned = unit->SummonCreature(VISUAL_WAYPOINT, gx, gy, gz + 0.5f, 0, TEMPSPAWN_TIMED_DESPAWN, 20000);
    summoned->CastSpell(summoned, 8599, TRIGGERED_NONE);
    uint32 tries = 1;
    uint32 successes = 0;
    uint32 startTime = WorldTimer::getMSTime();
    for (; tries < 500; ++tries)
    {
        unit->GetPosition(gx, gy, gz);
        if (unit->GetMap()->GetReachableRandomPosition(unit, gx, gy, gz, radius))
        {
            unit->SummonCreature(VISUAL_WAYPOINT, gx, gy, gz, 0, TEMPSPAWN_TIMED_DESPAWN, 15000);
            ++successes;
            if (successes >= 100)
                break;
        }
    }
    uint32 genTime = WorldTimer::getMSTimeDiff(startTime, WorldTimer::getMSTime());
    PSendSysMessage("Generated %u valid points for %u try in %ums.", successes, tries, genTime);
    return true;
}

bool ChatHandler::HandleServerResetAllRaidCommand(char* /*args*/)
{
    PSendSysMessage("Global raid instances reset, all players in raid instances will be teleported to homebind!");
    sMapPersistentStateMgr.GetScheduler().ResetAllRaid();
    return true;
}

bool ChatHandler::HandleLinkAddCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    if (!player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 masterCounter;
    if (!ExtractUInt32(&args, masterCounter))
        return false;

    uint32 flags;
    if (!ExtractUInt32(&args, flags))
        return false;

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter))
    {
        Field* fields = queryResult->Fetch();
        uint32 flag = fields[0].GetUInt32();
        PSendSysMessage("Link already exists with flag = %u", flag);
    }
    else
    {
        WorldDatabase.PExecute("INSERT INTO creature_linking(guid,master_guid,flag) VALUES('%u','%u','%u')", player->GetSelectionGuid().GetCounter(), masterCounter, flags);
        PSendSysMessage("Created link for guid = %u , master_guid = %u and flags = %u", player->GetSelectionGuid().GetCounter(), masterCounter, flags);
    }

    return true;
}

bool ChatHandler::HandleLinkRemoveCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    if (!player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 masterCounter;
    if (!ExtractUInt32(&args, masterCounter))
        return false;

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter))
    {
        WorldDatabase.PExecute("DELETE FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter);
        PSendSysMessage("Deleted link for guid = %u and master_guid = %u", player->GetSelectionGuid().GetCounter(), masterCounter);
    }
    else
        SendSysMessage("Link does not exist.");

    return true;
}

bool ChatHandler::HandleLinkEditCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    if (!player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 masterCounter;
    if (!ExtractUInt32(&args, masterCounter))
        return false;

    uint32 flags;
    if (!ExtractUInt32(&args, flags))
        return false;

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter))
    {
        if (flags)
        {
            WorldDatabase.PExecute("UPDATE creature_linking SET flags = flags | '%u' WHERE guid = '%u' AND master_guid = '%u'", flags, player->GetSelectionGuid().GetCounter(), masterCounter);
            SendSysMessage("Flag added to link.");
        }
        else
        {
            WorldDatabase.PExecute("DELETE FROM creature_linking WHERE guid = '%u' AND master_guid = '%u')", player->GetSelectionGuid().GetCounter(), masterCounter);
            SendSysMessage("Link removed.");
        }
    }
    else
    {
        if (flags)
        {
            WorldDatabase.PExecute("INSERT INTO creature_linking(guid,master_guid,flags) VALUES('%u','%u','%u')", player->GetSelectionGuid().GetCounter(), masterCounter, flags);
            SendSysMessage("Link did not exist. Inserted link");
        }
        else
            SendSysMessage("Link does not exist.");
    }

    return true;
}

bool ChatHandler::HandleLinkToggleCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    if (!player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 masterCounter;
    if (!ExtractUInt32(&args, masterCounter))
        return false;

    uint32 flags;
    if (!ExtractUInt32(&args, flags))
        return false;

    uint32 toggle; // 0 add flags, 1 remove flags
    if (!ExtractUInt32(&args, toggle))
        return false;

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter))
    {
        if (toggle)
        {
            WorldDatabase.PExecute("UPDATE creature_linking SET flags = flags &~ '%u' WHERE guid = '%u' AND master_guid = '%u'", flags, player->GetSelectionGuid().GetCounter(), masterCounter);
            SendSysMessage("Flag removed from link.");
        }
        else
        {
            WorldDatabase.PExecute("UPDATE creature_linking SET flags = flags | '%u' WHERE guid = '%u' AND master_guid = '%u'", flags, player->GetSelectionGuid().GetCounter(), masterCounter);
            SendSysMessage("Flag added to link.");
        }
    }
    else
    {
        if (toggle)
            SendSysMessage("Link does not exist. No changes done.");
        else
        {
            WorldDatabase.PExecute("INSERT INTO creature_linking(guid,master_guid,flags) VALUES('%u','%u','%u')", player->GetSelectionGuid().GetCounter(), masterCounter, flags);
            SendSysMessage("Link did not exist, added.");
        }
    }

    return true;
}

bool ChatHandler::HandleLinkCheckCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    if (!player->GetSelectionGuid())
    {
        SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    uint32 masterCounter;
    if (!ExtractUInt32(&args, masterCounter))
        return false;

    bool found = false;

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", player->GetSelectionGuid().GetCounter(), masterCounter))
    {
        Field* fields = queryResult->Fetch();
        uint32 flags = fields[0].GetUInt32();
        PSendSysMessage("Link for guid = %u , master_guid = %u has flags = %u", player->GetSelectionGuid().GetCounter(), masterCounter, flags);
        found = true;
    }

    if (auto queryResult = WorldDatabase.PQuery("SELECT flag FROM creature_linking WHERE guid = '%u' AND master_guid = '%u'", masterCounter, player->GetSelectionGuid().GetCounter()))
    {
        Field* fields = queryResult->Fetch();
        uint32 flags = fields[0].GetUInt32();
        PSendSysMessage("Link for guid = %u , master_guid = %u has flags = %u", masterCounter, player->GetSelectionGuid().GetCounter(), flags);
        found = true;
    }

    if (!found)
        PSendSysMessage("Link for guids = %u , %u not found", masterCounter, player->GetSelectionGuid().GetCounter());

    return true;
}

bool ChatHandler::HandleVariablePrint(char* args)
{
    Player* player = GetSession()->GetPlayer();

    PSendSysMessage("%s", player->GetMap()->GetVariableManager().GetVariableList().data());
    return true;
}

bool ChatHandler::HandleWarEffortCommand(char* args)
{
    PSendSysMessage("%s", sWorldState.GetAQPrintout().data());
    return true;
}

bool ChatHandler::HandleWarEffortPhaseCommand(char* args)
{
    uint32 param;
    if (!ExtractUInt32(&args, param))
    {
        PSendSysMessage("%s", sWorldState.GetAQPrintout().data());
        return true;
    }
    sWorldState.HandleWarEffortPhaseTransition(param);
    return true;
}

bool ChatHandler::HandleWarEffortCounterCommand(char* args)
{
    uint32 index;
    if (!ExtractUInt32(&args, index) || index >= RESOURCE_MAX)
    {
        PSendSysMessage("Enter valid index for counter.");
        return true;
    }

    uint32 value;
    if (!ExtractUInt32(&args, value))
    {
        PSendSysMessage("Enter valid value for counter.");
        return true;
    }

    sWorldState.AddWarEffortProgress(AQResources(index), value);
    return true;
}

bool ChatHandler::HandleScourgeInvasionCommand(char* args)
{
    return true;
}

bool ChatHandler::HandleScourgeInvasionStateCommand(char* args)
{
    uint32 value;
    if (!ExtractUInt32(&args, value) || value >= SI_STATE_MAX)
    {
        PSendSysMessage("Enter valid value for state.");
        return true;
    }

    sWorldState.SetScourgeInvasionState(SIState(value));
    return true;
}

bool ChatHandler::HandleScourgeInvasionBattlesWonCommand(char* args)
{
    int32 value;
    if (!ExtractInt32(&args, value))
    {
        PSendSysMessage("Enter valid count for battles won.");
        return true;
    }

    sWorldState.AddBattlesWon(value);
    return true;
}

bool ChatHandler::HandleScourgeInvasionStartZone(char* args)
{
    int32 value;
    if (!ExtractInt32(&args, value))
    {
        PSendSysMessage("Enter valid SIZoneId (0-7).");
        return true;
    }

    sWorldState.StartZoneEvent(SIZoneIds(value));
    return true;
}

bool ChatHandler::HandleSetVariable(char* args)
{
    int32 variableId;
    if (!ExtractInt32(&args, variableId))
        return false;

    int32 value;
    if (!ExtractInt32(&args, value))
        return false;

    Player* player = GetSession()->GetPlayer();
    player->GetMap()->GetVariableManager().SetVariable(variableId, value);
    return true;
}

enum ModSpells
{
    // client spells
    SPELL_MOD_STRENGTH     = 13372,
    SPELL_MOD_AGILITY      = 13365,
    SPELL_MOD_STAMINA      = 13370,
    SPELL_MOD_INTELLECT    = 13366,
    SPELL_MOD_SPIRIT       = 13368,
    SPELL_MOD_SPELL_POWER  = 22747,

    // custom spells
    SPELL_MOD_ARMOR        = 15170,
    SPELL_MOD_RES_HOLY     = 15171,
    SPELL_MOD_RES_FIRE     = 15172,
    SPELL_MOD_RES_NATURE   = 15173,
    SPELL_MOD_RES_FROST    = 15174,
    SPELL_MOD_RES_SHADOW   = 15175,
    SPELL_MOD_RES_ARCANE   = 15176,
    SPELL_MOD_MELEE_AP     = 15177,
    SPELL_MOD_RANGE_AP     = 15178,
    SPELL_MOD_MELEE_CRIT   = 15179,
    SPELL_MOD_SPELL_CRIT   = 15180,
    SPELL_MOD_MELEE_HASTE  = 15181,
    SPELL_MOD_RANGE_HASTE  = 15182,
    SPELL_MOD_SPELL_HASTE  = 15183,
    SPELL_MOD_PARRY_CHANCE = 15184,
    SPELL_MOD_DODGE_CHANCE = 15185,
    SPELL_MOD_BLOCK_CHANCE = 15186,
};

bool ChatHandler::HandleResetModsCommand(char *args)
{
    Unit* target = getSelectedUnit();

    if (!target)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    target->RemoveAurasDueToSpell(SPELL_MOD_STRENGTH);
    target->RemoveAurasDueToSpell(SPELL_MOD_AGILITY);
    target->RemoveAurasDueToSpell(SPELL_MOD_STAMINA);
    target->RemoveAurasDueToSpell(SPELL_MOD_INTELLECT);
    target->RemoveAurasDueToSpell(SPELL_MOD_SPIRIT);
    target->RemoveAurasDueToSpell(SPELL_MOD_ARMOR);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_HOLY);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_FIRE);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_NATURE);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_FROST);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_SHADOW);
    target->RemoveAurasDueToSpell(SPELL_MOD_RES_ARCANE);
    target->RemoveAurasDueToSpell(SPELL_MOD_MELEE_AP);
    target->RemoveAurasDueToSpell(SPELL_MOD_RANGE_AP);
    target->RemoveAurasDueToSpell(SPELL_MOD_SPELL_POWER);
    target->RemoveAurasDueToSpell(SPELL_MOD_MELEE_CRIT);
    target->RemoveAurasDueToSpell(SPELL_MOD_SPELL_CRIT);
    target->RemoveAurasDueToSpell(SPELL_MOD_MELEE_HASTE);
    target->RemoveAurasDueToSpell(SPELL_MOD_RANGE_HASTE);
    target->RemoveAurasDueToSpell(SPELL_MOD_SPELL_HASTE);
    target->RemoveAurasDueToSpell(SPELL_MOD_PARRY_CHANCE);
    target->RemoveAurasDueToSpell(SPELL_MOD_DODGE_CHANCE);
    target->RemoveAurasDueToSpell(SPELL_MOD_BLOCK_CHANCE);

    PSendSysMessage("You removed all stat mods from %s.", target->GetName());

    return true;
}

bool ChatHandler::ModifyStatCommandHelper(char* args, char const* statName, uint32 spellId)
{
    if (!*args)
        return false;
    
    Unit* target = getSelectedUnit();

    if (!target)
    {
        PSendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
        SetSentErrorMessage(true);
        return false;
    }

    int32 amount;
    if (!ExtractInt32(&args, amount))
        return false;

    if (!amount)
    {
        target->RemoveAurasDueToSpell(spellId);
        return true;
    }

    target->RemoveAurasDueToSpell(spellId);
    target->CastCustomSpell(target, spellId, &amount, &amount, nullptr, TRIGGERED_OLD_TRIGGERED);

    if (spellId == SPELL_MOD_STAMINA)
        target->SetHealthPercent(100);
    else if (spellId == SPELL_MOD_INTELLECT)
        target->SetPower(POWER_MANA, target->GetMaxPower(POWER_MANA));

    PSendSysMessage("You changed %s of %s to %i.", statName, target->GetName(), amount);

    return true;
}

bool ChatHandler::HandleModifyStrengthCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Strength", SPELL_MOD_STRENGTH);
}

bool ChatHandler::HandleModifyAgilityCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Agility", SPELL_MOD_AGILITY);
}

bool ChatHandler::HandleModifyStaminaCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Stamina", SPELL_MOD_STAMINA);
}

bool ChatHandler::HandleModifyIntellectCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Intellect", SPELL_MOD_INTELLECT);
}

bool ChatHandler::HandleModifySpiritCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Spirit", SPELL_MOD_SPIRIT);
}

bool ChatHandler::HandleModifyArmorCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Armor", SPELL_MOD_ARMOR);
}

bool ChatHandler::HandleModifyHolyCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Holy Resistance", SPELL_MOD_RES_HOLY);
}

bool ChatHandler::HandleModifyFireCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Fire Resistance", SPELL_MOD_RES_FIRE);
}

bool ChatHandler::HandleModifyNatureCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Nature Resistance", SPELL_MOD_RES_NATURE);
}

bool ChatHandler::HandleModifyFrostCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Frost Resistance", SPELL_MOD_RES_FROST);
}

bool ChatHandler::HandleModifyShadowCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Shadow Resistance", SPELL_MOD_RES_SHADOW);
}

bool ChatHandler::HandleModifyArcaneCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Arcane Resistance", SPELL_MOD_RES_ARCANE);
}

bool ChatHandler::HandleModifyMeleeApCommand(char *args)
{
   
    return ModifyStatCommandHelper(args, "Melee Attack Power", SPELL_MOD_MELEE_AP);
}

bool ChatHandler::HandleModifyRangedApCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Ranged Attack Power", SPELL_MOD_RANGE_AP);
}

bool ChatHandler::HandleModifySpellPowerCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Spell Power", SPELL_MOD_SPELL_POWER);
}

bool ChatHandler::HandleModifyMeleeCritCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Melee Crit", SPELL_MOD_MELEE_CRIT);
}

bool ChatHandler::HandleModifySpellCritCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Spell Crit", SPELL_MOD_SPELL_CRIT);
}

bool ChatHandler::HandleModifyMeleeHasteCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Melee Haste", SPELL_MOD_MELEE_HASTE);
}

bool ChatHandler::HandleModifyRangedHasteCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Ranged Haste", SPELL_MOD_RANGE_HASTE);
}

bool ChatHandler::HandleModifySpellHasteCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Spell Haste", SPELL_MOD_SPELL_HASTE);
}

bool ChatHandler::HandleModifyBlockCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Block Chance", SPELL_MOD_BLOCK_CHANCE);
}

bool ChatHandler::HandleModifyDodgeCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Dodge Chance", SPELL_MOD_DODGE_CHANCE);
}

bool ChatHandler::HandleModifyParryCommand(char *args)
{
    return ModifyStatCommandHelper(args, "Parry Chance", SPELL_MOD_PARRY_CHANCE);
}

bool ChatHandler::HandleGoNextCommand(char* args)
{
    Player* player = m_session->GetPlayer();

    uint32 lastDbguid; uint32 gameobjectUint;
    bool gameobject;
    if (!ExtractUInt32(&args, lastDbguid) || !ExtractUInt32(&args, gameobjectUint))
    {
        std::tie(lastDbguid, gameobject) = player->GetLastData();
        ++lastDbguid;
    }
    else
    {
        gameobject = gameobjectUint;
        player->SetLastData(lastDbguid, gameobject);
    }

    uint32 mapId;
    float x, y, z;
    if (gameobject)
    {
        uint32 maxDbGuid = sObjectMgr.GetMaxGoDbGuid();
        GameObjectData const* goData = nullptr;
        uint32 i;
        for (i = lastDbguid; i < maxDbGuid; ++i)
        {
            goData = sObjectMgr.GetGOData(i);
            if (goData)
                break;
        }

        if (goData)
        {
            player->SetLastData(i, true);
            mapId = goData->mapid;
            x = goData->posX;
            y = goData->posY;
            z = goData->posZ;
            PSendSysMessage("Teleporting to gameobject dbGuid %u", i);
        }
        else
            return false;
    }
    else
    {
        uint32 maxDbGuid = sObjectMgr.GetMaxCreatureDbGuid();
        CreatureData const* creatureData = nullptr;
        uint32 i;
        for (i = lastDbguid; i < maxDbGuid; ++i)
        {
            creatureData = sObjectMgr.GetCreatureData(i);
            if (creatureData)
                break;
        }

        if (creatureData)
        {
            player->SetLastData(i, false);
            mapId = creatureData->mapid;
            x = creatureData->posX;
            y = creatureData->posY;
            z = creatureData->posZ;
            PSendSysMessage("Teleporting to creature dbGuid %u", i);
        }
        else
            return false;
    }

    return HandleGoHelper(player, mapId, x, y, &z);
}
