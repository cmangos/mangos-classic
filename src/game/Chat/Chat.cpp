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

#include "Chat/Chat.h"
#include "Tools/Language.h"
#include "Database/DatabaseEnv.h"
#include "WorldPacket.h"
#include "Server/WorldSession.h"
#include "Server/Opcodes.h"
#include "Log.h"
#include "World/World.h"
#include "Globals/ObjectMgr.h"
#include "Globals/ObjectAccessor.h"
#include "Entities/ObjectGuid.h"
#include "Entities/Player.h"
#include "Grids/GridNotifiersImpl.h"
#include "Grids/CellImpl.h"
#include "Accounts/AccountMgr.h"
#include "Spells/SpellMgr.h"
#include "Pools/PoolManager.h"
#include "GameEvents/GameEventMgr.h"

#include <cstdarg>

// Supported shift-links (client generated and server side)
// |color|Harea:area_id|h[name]|h|r
// |color|Hareatrigger:id|h[name]|h|r
// |color|Hareatrigger_target:id|h[name]|h|r
// |color|Hcreature:creature_guid|h[name]|h|r
// |color|Hcreature_entry:creature_id|h[name]|h|r
// |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r          - client, at shift click in recipes list dialog
// |color|Hgameevent:id|h[name]|h|r
// |color|Hgameobject:go_guid|h[name]|h|r
// |color|Hgameobject_entry:go_id|h[name]|h|r
// |color|Hitem:item_id:perm_ench_id:gem1:gem2:gem3:0:0:0:0|h[name]|h|r   - client, item icon shift click
// |color|Hitemset:itemset_id|h[name]|h|r
// |color|Hplayer:name|h[name]|h|r                                        - client, in some messages, at click copy only name instead link, so no way generate it in client string send to server
// |color|Hpool:pool_id|h[name]|h|r
// |color|Hquest:quest_id:quest_level|h[name]|h|r                         - client, quest list name shift-click
// |color|Hskill:skill_id|h[name]|h|r
// |color|Hspell:spell_id|h[name]|h|r                                     - client, spellbook spell icon shift-click
// |color|Htalent:talent_id,rank|h[name]|h|r                              - client, talent icon shift-click rank==-1 if shift-copy unlearned talent
// |color|Htaxinode:id|h[name]|h|r
// |color|Htele:id|h[name]|h|r

bool ChatHandler::load_command_table = true;

ChatCommand* ChatHandler::getCommandTable()
{
    static ChatCommand accountSetCommandTable[] =
    {
        { "addon",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountSetAddonCommand,     "", nullptr },
        { "gmlevel",        SEC_CONSOLE,        true,  &ChatHandler::HandleAccountSetGmLevelCommand,   "", nullptr },
        { "password",       SEC_CONSOLE,        true,  &ChatHandler::HandleAccountSetPasswordCommand,  "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand accountCommandTable[] =
    {
        { "characters",     SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAccountCharactersCommand,   "", nullptr },
        { "create",         SEC_CONSOLE,        true,  &ChatHandler::HandleAccountCreateCommand,       "", nullptr },
        { "delete",         SEC_CONSOLE,        true,  &ChatHandler::HandleAccountDeleteCommand,       "", nullptr },
        { "onlinelist",     SEC_CONSOLE,        true,  &ChatHandler::HandleAccountOnlineListCommand,   "", nullptr },
        { "lock",           SEC_PLAYER,         true,  &ChatHandler::HandleAccountLockCommand,         "", nullptr },
        { "set",            SEC_ADMINISTRATOR,  true,  nullptr,                                        "", accountSetCommandTable },
        { "password",       SEC_PLAYER,         true,  &ChatHandler::HandleAccountPasswordCommand,     "", nullptr },
        { "",               SEC_PLAYER,         true,  &ChatHandler::HandleAccountCommand,             "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

#ifdef BUILD_AHBOT
    static ChatCommand ahbotCommandTable[] =
    {
        { "rebuild",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAHBotRebuildCommand,        "", nullptr },
        { "reload",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAHBotReloadCommand,         "", nullptr },
        { "status",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAHBotStatusCommand,         "", nullptr },
        { "item",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAHBotItemCommand,           "", nullptr },
        { nullptr,          0,                  true,  nullptr,                                        "", nullptr }
    };
#endif

    static ChatCommand auctionCommandTable[] =
    {
        { "alliance",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionAllianceCommand,     "", nullptr },
        { "goblin",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionGoblinCommand,       "", nullptr },
        { "horde",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionHordeCommand,        "", nullptr },
        { "item",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAuctionItemCommand,         "", nullptr },
        { "",               SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAuctionCommand,             "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand banCommandTable[] =
    {
        { "account",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanAccountCommand,          "", nullptr },
        { "character",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanCharacterCommand,        "", nullptr },
        { "ip",             SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanIPCommand,               "", nullptr },
        { "note",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleAddCharacterNoteCommand,    "", nullptr },
        { "warn",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleWarnCharacterCommand,       "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand baninfoCommandTable[] =
    {
        { "account",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoAccountCommand,      "", nullptr },
        { "character",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoCharacterCommand,    "", nullptr },
        { "ip",             SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanInfoIPCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand banlistCommandTable[] =
    {
        { "account",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListAccountCommand,      "", nullptr },
        { "character",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListCharacterCommand,    "", nullptr },
        { "ip",             SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleBanListIPCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand castCommandTable[] =
    {
        { "back",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCastBackCommand,            "", nullptr },
        { "dist",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCastDistCommand,            "", nullptr },
        { "self",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCastSelfCommand,            "", nullptr },
        { "target",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCastTargetCommand,          "", nullptr },
        { "",               SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCastCommand,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand characterDeletedCommandTable[] =
    {
        { "delete",         SEC_CONSOLE,        true,  &ChatHandler::HandleCharacterDeletedDeleteCommand, "", nullptr },
        { "list",           SEC_CONSOLE,        true,  &ChatHandler::HandleCharacterDeletedListCommand, "", nullptr },
        { "restore",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleCharacterDeletedRestoreCommand, "", nullptr },
        { "old",            SEC_CONSOLE,        true,  &ChatHandler::HandleCharacterDeletedOldCommand, "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand characterCommandTable[] =
    {
        { "deleted",        SEC_GAMEMASTER,     true,  nullptr,                                           "", characterDeletedCommandTable},
        { "erase",          SEC_CONSOLE,        true,  &ChatHandler::HandleCharacterEraseCommand,      "", nullptr },
        { "level",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleCharacterLevelCommand,      "", nullptr },
        { "rename",         SEC_GAMEMASTER,     true,  &ChatHandler::HandleCharacterRenameCommand,     "", nullptr },
        { "reputation",     SEC_GAMEMASTER,     true,  &ChatHandler::HandleCharacterReputationCommand, "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand channelCommandTable[] =
    {
        { "list",           SEC_MODERATOR,      false, &ChatHandler::HandleChannelListCommand,              "", nullptr },
        { "static",         SEC_MODERATOR,      false, &ChatHandler::HandleChannelStaticCommand,            "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand cooldownCommandTable[] =
    {
        { "list",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCooldownListCommand,             "", nullptr },
        { "clear",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCooldownClearCommand,            "", nullptr },
        { "clearclientside", SEC_ADMINISTRATOR,  false, &ChatHandler::HandleCooldownClearClientSideCommand,  "", nullptr },
        { nullptr,             0,                  false, nullptr,                                          "", nullptr }
    };

    static ChatCommand debugPlayCommandTable[] =
    {
        { "cinematic",      SEC_MODERATOR,      false, &ChatHandler::HandleDebugPlayCinematicCommand,       "", nullptr },
        { "sound",          SEC_MODERATOR,      false, &ChatHandler::HandleDebugPlaySoundCommand,           "", nullptr },
        { "music",          SEC_MODERATOR,      false, &ChatHandler::HandleDebugPlayMusicCommand,           "", nullptr },
        { "petdismiss",     SEC_MODERATOR,      false, &ChatHandler::HandleDebugPetDismissSound,            "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand debugSendCommandTable[] =
    {
        { "buyerror",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendBuyErrorCommand,        "", nullptr },
        { "channelnotify",  SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendChannelNotifyCommand,   "", nullptr },
        { "chatmmessage",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendChatMsgCommand,         "", nullptr },
        { "equiperror",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendEquipErrorCommand,      "", nullptr },
        { "opcode",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendOpcodeCommand,          "", nullptr },
        { "poi",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendPoiCommand,             "", nullptr },
        { "qfailedresult",  SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendQuestFailedMsgCommand,  "", nullptr },
        { "qpartymsg",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendQuestPartyMsgCommand,   "", nullptr },
        { "qinvalidmsg",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendQuestInvalidMsgCommand, "", nullptr },
        { "sellerror",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendSellErrorCommand,       "", nullptr },
        { "spellfail",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendSpellFailCommand,       "", nullptr },
        { "spellVisual",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSpellVisual,                "", nullptr },
        { "worldstate",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSendWorldState,             "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand debugScriptCommandTable[] =
    {
        { "help",           SEC_GAMEMASTER,     false, &ChatHandler::HandleSD2HelpCommand,                  "", nullptr },
        { "command",        SEC_GAMEMASTER,     false, &ChatHandler::HandleSD2ScriptCommand,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand debugVisibilityCommandTable[] =
    {
        { "atclient",       SEC_MODERATOR,      false, &ChatHandler::HandleDebugHaveAtClientCommand,        "", nullptr },
        { "isvisible",      SEC_MODERATOR,      false, &ChatHandler::HandleDebugIsVisibleCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand debugPerformanceCommandTable[] =
    {
        { "tempspawn",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleShowTemporarySpawnList,          "", nullptr },
        { "gridsloaded",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGridsLoadedCount,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand debugCommandTable[] =
    {
        { "anim",           SEC_GAMEMASTER,     false, &ChatHandler::HandleDebugAnimCommand,                "", nullptr },
        { "bg",             SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugBattlegroundCommand,        "", nullptr },
        { "getitemstate",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetItemStateCommand,        "", nullptr },
        { "lootrecipient",  SEC_GAMEMASTER,     false, &ChatHandler::HandleDebugGetLootRecipientCommand,    "", nullptr },
        { "getitemvalue",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetItemValueCommand,        "", nullptr },
        { "getvalue",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugGetValueCommand,            "", nullptr },
        { "moditemvalue",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugModItemValueCommand,        "", nullptr },
        { "modvalue",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugModValueCommand,            "", nullptr },
        { "play",           SEC_MODERATOR,      false, nullptr,                                             "", debugPlayCommandTable },
        { "send",           SEC_ADMINISTRATOR,  false, nullptr,                                             "", debugSendCommandTable },
        { "setaurastate",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetAuraStateCommand,        "", nullptr },
        { "setitemvalue",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetItemValueCommand,        "", nullptr },
        { "script",         SEC_GAMEMASTER,     false, nullptr,                                             "", debugScriptCommandTable },
        { "setvalue",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSetValueCommand,            "", nullptr },
        { "spellcheck",     SEC_CONSOLE,        true,  &ChatHandler::HandleDebugSpellCheckCommand,          "", nullptr },
        { "spellcoefs",     SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleDebugSpellCoefsCommand,          "", nullptr },
        { "spellmods",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugSpellModsCommand,           "", nullptr },
        { "taxi",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugTaxiCommand,                "", nullptr },
        { "uws",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugUpdateWorldStateCommand,    "", nullptr },
        { "waypoint",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugWaypoint,                   "", nullptr },
        { "byte",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugByteFields,                 "", nullptr },
        { "moveflag",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugMoveflags,                  "", nullptr },
        { "visibility",     SEC_MODERATOR,      false, nullptr,                                             "", debugVisibilityCommandTable },
        { "perf",           SEC_ADMINISTRATOR,  false, nullptr,                                             "", debugPerformanceCommandTable },
        { "lootdropstats",  SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugLootDropStats,              "", nullptr },
        { "utf8overflow",   SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleDebugOverflowCommand,            "", nullptr },
        { "chatfreeze",     SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleDebugChatFreezeCommand,          "", nullptr },
        { "opcodehistory",  SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleDebugPacketHistory,              "", nullptr },
        { "debugflags",     SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleDebugObjectFlags,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                             "", nullptr }
    };

    static ChatCommand eventCommandTable[] =
    {
        { "list",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleEventListCommand,           "", nullptr },
        { "start",          SEC_GAMEMASTER,     true,  &ChatHandler::HandleEventStartCommand,          "", nullptr },
        { "stop",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleEventStopCommand,           "", nullptr },
        { "",               SEC_GAMEMASTER,     true,  &ChatHandler::HandleEventInfoCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand gmCommandTable[] =
    {
        { "chat",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGMChatCommand,              "", nullptr },
        { "fly",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGMFlyCommand,               "", nullptr },
        { "ingame",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleGMListIngameCommand,        "", nullptr },
        { "list",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleGMListFullCommand,          "", nullptr },
        { "mountup",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGMMountUpCommand,           "", nullptr },
        { "visible",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGMVisibleCommand,           "", nullptr },
        { "setview",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleSetViewCommand,             "", nullptr },
        { "",               SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGMCommand,                  "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand goCommandTable[] =
    {
        { "creature",       SEC_MODERATOR,      false, &ChatHandler::HandleGoCreatureCommand,          "", nullptr },
        { "graveyard",      SEC_MODERATOR,      false, &ChatHandler::HandleGoGraveyardCommand,         "", nullptr },
        { "grid",           SEC_MODERATOR,      false, &ChatHandler::HandleGoGridCommand,              "", nullptr },
        { "object",         SEC_MODERATOR,      false, &ChatHandler::HandleGoObjectCommand,            "", nullptr },
        { "taxinode",       SEC_MODERATOR,      false, &ChatHandler::HandleGoTaxinodeCommand,          "", nullptr },
        { "trigger",        SEC_MODERATOR,      false, &ChatHandler::HandleGoTriggerCommand,           "", nullptr },
        { "zonexy",         SEC_MODERATOR,      false, &ChatHandler::HandleGoZoneXYCommand,            "", nullptr },
        { "xy",             SEC_MODERATOR,      false, &ChatHandler::HandleGoXYCommand,                "", nullptr },
        { "xyz",            SEC_MODERATOR,      false, &ChatHandler::HandleGoXYZCommand,               "", nullptr },
        { "",               SEC_MODERATOR,      false, &ChatHandler::HandleGoCommand,                  "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand gobjectCommandTable[] =
    {
        { "add",            SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectAddCommand,       "", nullptr },
        { "delete",         SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectDeleteCommand,    "", nullptr },
        { "move",           SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectMoveCommand,      "", nullptr },
        { "near",           SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectNearCommand,      "", nullptr },
        { "nearspawned",    SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectNearSpawnedCommand, "", nullptr },
        { "target",         SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectTargetCommand,    "", nullptr },
        { "turn",           SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectTurnCommand,      "", nullptr },
        { "activate",       SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectActivateCommand,  "", nullptr },
        { "despawn",        SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectForcedDespawnCommand, "", nullptr },
        { "respawn",        SEC_GAMEMASTER,     false, &ChatHandler::HandleGameObjectRespawnCommand,   "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand guildCommandTable[] =
    {
        { "create",         SEC_GAMEMASTER,     true,  &ChatHandler::HandleGuildCreateCommand,         "", nullptr },
        { "delete",         SEC_GAMEMASTER,     true,  &ChatHandler::HandleGuildDeleteCommand,         "", nullptr },
        { "invite",         SEC_GAMEMASTER,     true,  &ChatHandler::HandleGuildInviteCommand,         "", nullptr },
        { "uninvite",       SEC_GAMEMASTER,     true,  &ChatHandler::HandleGuildUninviteCommand,       "", nullptr },
        { "rank",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleGuildRankCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand honorCommandTable[] =
    {
        { "add",            SEC_GAMEMASTER,     false, &ChatHandler::HandleHonorAddCommand,            "", nullptr },
        { "addkill",        SEC_GAMEMASTER,     false, &ChatHandler::HandleHonorAddKillCommand,        "", nullptr },
        { "show",           SEC_GAMEMASTER,     false, &ChatHandler::HandleHonorShow,                  "", nullptr },
        { "update",         SEC_GAMEMASTER,     false, &ChatHandler::HandleHonorUpdateCommand,         "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand instanceCommandTable[] =
    {
        { "listbinds",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceListBindsCommand,   "", nullptr },
        { "unbind",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceUnbindCommand,      "", nullptr },
        { "stats",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleInstanceStatsCommand,       "", nullptr },
        { "savedata",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleInstanceSaveDataCommand,    "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand learnCommandTable[] =
    {
        { "all",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllCommand,            "", nullptr },
        { "all_gm",         SEC_GAMEMASTER,     false, &ChatHandler::HandleLearnAllGMCommand,          "", nullptr },
        { "all_crafts",     SEC_GAMEMASTER,     false, &ChatHandler::HandleLearnAllCraftsCommand,      "", nullptr },
        { "all_default",    SEC_MODERATOR,      false, &ChatHandler::HandleLearnAllDefaultCommand,     "", nullptr },
        { "all_lang",       SEC_MODERATOR,      false, &ChatHandler::HandleLearnAllLangCommand,        "", nullptr },
        { "all_myclass",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMyClassCommand,     "", nullptr },
        { "all_myspells",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMySpellsCommand,    "", nullptr },
        { "all_mytalents",  SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLearnAllMyTalentsCommand,   "", nullptr },
        { "all_recipes",    SEC_GAMEMASTER,     false, &ChatHandler::HandleLearnAllRecipesCommand,     "", nullptr },
        { "",               SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLearnCommand,               "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand linkCommandTable[] =
    {
        { "add",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkAddCommand,             "", nullptr },
        { "remove",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkRemoveCommand,          "", nullptr },
        { "edit",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkEditCommand,            "", nullptr },
        { "toggle",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkToggleCommand,          "", nullptr },
        { "check",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkCheckCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand listCommandTable[] =
    {
        { "auras",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleListAurasCommand,           "", nullptr },
        { "creature",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleListCreatureCommand,        "", nullptr },
        { "item",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleListItemCommand,            "", nullptr },
        { "object",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleListObjectCommand,          "", nullptr },
        { "talents",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleListTalentsCommand,         "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand lookupAccountCommandTable[] =
    {
        { "email",          SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountEmailCommand,  "", nullptr },
        { "ip",             SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountIpCommand,     "", nullptr },
        { "name",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupAccountNameCommand,   "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand lookupPlayerCommandTable[] =
    {
        { "account",        SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerAccountCommand, "", nullptr },
        { "email",          SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerEmailCommand,   "", nullptr },
        { "ip",             SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupPlayerIpCommand,      "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand lookupCommandTable[] =
    {
        { "account",        SEC_GAMEMASTER,     true,  nullptr,                                        "", lookupAccountCommandTable },
        { "area",           SEC_MODERATOR,      true,  &ChatHandler::HandleLookupAreaCommand,          "", nullptr },
        { "creature",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupCreatureCommand,      "", nullptr },
        { "event",          SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupEventCommand,         "", nullptr },
        { "faction",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupFactionCommand,       "", nullptr },
        { "item",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupItemCommand,          "", nullptr },
        { "itemset",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupItemSetCommand,       "", nullptr },
        { "object",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupObjectCommand,        "", nullptr },
        { "quest",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupQuestCommand,         "", nullptr },
        { "player",         SEC_GAMEMASTER,     true,  nullptr,                                        "", lookupPlayerCommandTable },
        { "pool",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleLookupPoolCommand,          "", nullptr },
        { "skill",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupSkillCommand,         "", nullptr },
        { "spell",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupSpellCommand,         "", nullptr },
        { "taxinode",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLookupTaxiNodeCommand,      "", nullptr },
        { "tele",           SEC_MODERATOR,      true,  &ChatHandler::HandleLookupTeleCommand,          "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand mmapCommandTable[] =
    {
        { "path",           SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapPathCommand,            "", nullptr },
        { "loc",            SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapLocCommand,             "", nullptr },
        { "loadedtiles",    SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapLoadedTilesCommand,     "", nullptr },
        { "stats",          SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapStatsCommand,           "", nullptr },
        { "testarea",       SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapTestArea,               "", nullptr },
        { "testheight",     SEC_GAMEMASTER,     false, &ChatHandler::HandleMmapTestHeight,             "", nullptr },
        { "",               SEC_ADMINISTRATOR,  false, &ChatHandler::HandleMmap,                       "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand modifyCommandTable[] =
    {
        { "hp",             SEC_MODERATOR,      false, &ChatHandler::HandleModifyHPCommand,            "", nullptr },
        { "mana",           SEC_MODERATOR,      false, &ChatHandler::HandleModifyManaCommand,          "", nullptr },
        { "rage",           SEC_MODERATOR,      false, &ChatHandler::HandleModifyRageCommand,          "", nullptr },
        { "energy",         SEC_MODERATOR,      false, &ChatHandler::HandleModifyEnergyCommand,        "", nullptr },
        { "money",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyMoneyCommand,         "", nullptr },
        { "speed",          SEC_MODERATOR,      false, &ChatHandler::HandleModifySpeedCommand,         "", nullptr },
        { "swim",           SEC_MODERATOR,      false, &ChatHandler::HandleModifySwimCommand,          "", nullptr },
        { "scale",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyScaleCommand,         "", nullptr },
        { "bwalk",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyBWalkCommand,         "", nullptr },
        { "aspeed",         SEC_MODERATOR,      false, &ChatHandler::HandleModifyASpeedCommand,        "", nullptr },
        { "faction",        SEC_MODERATOR,      false, &ChatHandler::HandleModifyFactionCommand,       "", nullptr },
        { "tp",             SEC_MODERATOR,      false, &ChatHandler::HandleModifyTalentCommand,        "", nullptr },
        { "mount",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyMountCommand,         "", nullptr },
        { "honor",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyHonorCommand,         "", nullptr },
        { "rep",            SEC_GAMEMASTER,     false, &ChatHandler::HandleModifyRepCommand,           "", nullptr },
        { "drunk",          SEC_MODERATOR,      false, &ChatHandler::HandleModifyDrunkCommand,         "", nullptr },
        { "standstate",     SEC_GAMEMASTER,     false, &ChatHandler::HandleModifyStandStateCommand,    "", nullptr },
        { "morph",          SEC_GAMEMASTER,     false, &ChatHandler::HandleModifyMorphCommand,         "", nullptr },
        { "gender",         SEC_GAMEMASTER,     false, &ChatHandler::HandleModifyGenderCommand,        "", nullptr },
        { "strength",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyStrengthCommand,      "", nullptr },
        { "agility",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyAgilityCommand,       "", nullptr },
        { "stamina",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyStaminaCommand,       "", nullptr },
        { "intellect",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyIntellectCommand,     "", nullptr },
        { "spirit",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifySpiritCommand,        "", nullptr },
        { "armor",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyArmorCommand,         "", nullptr },
        { "holyresist",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyHolyCommand,          "", nullptr },
        { "fireresist",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyFireCommand,          "", nullptr },
        { "natureresist",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyNatureCommand,        "", nullptr },
        { "frostresist",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyFrostCommand,         "", nullptr },
        { "shadowresist",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyShadowCommand,        "", nullptr },
        { "arcaneresist",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyArcaneCommand,        "", nullptr },
        { "ap",             SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyMeleeApCommand,       "", nullptr },
        { "rangeap",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyRangedApCommand,      "", nullptr },
        { "spellpower",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifySpellPowerCommand,    "", nullptr },
        { "crit",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyMeleeCritCommand,     "", nullptr },
        { "spellcrit",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifySpellCritCommand,     "", nullptr },
        { "meleehaste",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyMeleeHasteCommand,    "", nullptr },
        { "rangehaste",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyRangedHasteCommand,   "", nullptr },
        { "spellhaste",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifySpellHasteCommand,    "", nullptr },
        { "block",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyBlockCommand,         "", nullptr },
        { "dodge",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyDodgeCommand,         "", nullptr },
        { "parry",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleModifyParryCommand,         "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand npcCommandTable[] =
    {
        { "add",            SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcAddCommand,              "", nullptr },
        { "additem",        SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcAddVendorItemCommand,    "", nullptr },
        { "aiinfo",         SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcAIInfoCommand,           "", nullptr },
        { "allowmove",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcAllowMovementCommand,    "", nullptr },
        { "changeentry",    SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcChangeEntryCommand,      "", nullptr },
        { "changelevel",    SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcChangeLevelCommand,      "", nullptr },
        { "delete",         SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcDeleteCommand,           "", nullptr },
        { "delitem",        SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcDelVendorItemCommand,    "", nullptr },
        { "factionid",      SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcFactionIdCommand,        "", nullptr },
        { "flag",           SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcFlagCommand,             "", nullptr },
        { "follow",         SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcFollowCommand,           "", nullptr },
        { "info",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcInfoCommand,             "", nullptr },
        { "threat",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcThreatCommand,           "", nullptr },
        { "move",           SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcMoveCommand,             "", nullptr },
        { "playemote",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcPlayEmoteCommand,        "", nullptr },
        { "setmodel",       SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSetModelCommand,         "", nullptr },
        { "setmovetype",    SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSetMoveTypeCommand,      "", nullptr },
        { "spawndist",      SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSpawnDistCommand,        "", nullptr },
        { "spawntime",      SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSpawnTimeCommand,        "", nullptr },
        { "say",            SEC_MODERATOR,      false, &ChatHandler::HandleNpcSayCommand,              "", nullptr },
        { "textemote",      SEC_MODERATOR,      false, &ChatHandler::HandleNpcTextEmoteCommand,        "", nullptr },
        { "unfollow",       SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcUnFollowCommand,         "", nullptr },
        { "whisper",        SEC_MODERATOR,      false, &ChatHandler::HandleNpcWhisperCommand,          "", nullptr },
        { "yell",           SEC_MODERATOR,      false, &ChatHandler::HandleNpcYellCommand,             "", nullptr },
        { "tame",           SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcTameCommand,             "", nullptr },
        { "setdeathstate",  SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSetDeathStateCommand,    "", nullptr },
        { "showloot",       SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcShowLootCommand,         "", nullptr },
        { "tempspawn",      SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcTempSpawn,               "", nullptr },

        //{ TODO: fix or remove this commands
        { "addweapon",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNpcAddWeaponCommand,        "", nullptr },
        { "name",           SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcNameCommand,             "", nullptr },
        { "subname",        SEC_GAMEMASTER,     false, &ChatHandler::HandleNpcSubNameCommand,          "", nullptr },
        //}

        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand pdumpCommandTable[] =
    {
        { "load",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandlePDumpLoadCommand,           "", nullptr },
        { "write",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandlePDumpWriteCommand,          "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand petCommandTable[] =
    {
        { "levelloyalty",   SEC_GAMEMASTER,     true,  &ChatHandler::HandlePetLevelLoyaltyCommand,   "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand poolCommandTable[] =
    {
        { "list",           SEC_GAMEMASTER,     false, &ChatHandler::HandlePoolListCommand,            "", nullptr },
        { "spawns",         SEC_GAMEMASTER,     false, &ChatHandler::HandlePoolSpawnsCommand,          "", nullptr },
        { "",               SEC_GAMEMASTER,     true,  &ChatHandler::HandlePoolInfoCommand,            "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand questCommandTable[] =
    {
        { "add",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleQuestAddCommand,            "", nullptr },
        { "complete",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleQuestCompleteCommand,       "", nullptr },
        { "remove",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleQuestRemoveCommand,         "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand reloadCommandTable[] =
    {
        { "all",            SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllCommand,           "", nullptr },
        { "all_area",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllAreaCommand,       "", nullptr },
        { "all_eventai",    SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllEventAICommand,    "", nullptr },
        { "all_gossips",    SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllGossipsCommand,    "", nullptr },
        { "all_item",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllItemCommand,       "", nullptr },
        { "all_locales",    SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllLocalesCommand,    "", nullptr },
        { "all_loot",       SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllLootCommand,       "", nullptr },
        { "all_npc",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllNpcCommand,        "", nullptr },
        { "all_quest",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllQuestCommand,      "", nullptr },
        { "all_scripts",    SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllScriptsCommand,    "", nullptr },
        { "all_spell",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadAllSpellCommand,      "", nullptr },

        { "config",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReloadConfigCommand,        "", nullptr },

        { "areatrigger_involvedrelation", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestAreaTriggersCommand,       "", nullptr },
        { "areatrigger_tavern",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadAreaTriggerTavernCommand,       "", nullptr },
        { "areatrigger_teleport",        SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadAreaTriggerTeleportCommand,     "", nullptr },
        { "command",                     SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCommandCommand,                 "", nullptr },
        { "conditions",                  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadConditionsCommand,              "", nullptr },
        { "creature_ai_scripts",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAIScriptsCommand,          "", nullptr },
        { "creature_ai_summons",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAISummonsCommand,          "", nullptr },
        { "creature_ai_texts",           SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadEventAITextsCommand,            "", nullptr },
        { "creature_battleground",       SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadBattleEventCommand,             "", nullptr },
        { "creature_involvedrelation",   SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCreatureQuestInvRelationsCommand, "", nullptr },
        { "creature_loot_template",      SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesCreatureCommand,   "", nullptr },
        { "creature_questrelation",      SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadCreatureQuestRelationsCommand,  "", nullptr },
        { "creature_template_classlevelstats", SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadCreaturesStatsCommand,     "", nullptr },
        { "dbscript_string",             SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDbScriptStringCommand,          "", nullptr },
        { "dbscripts_on_creature_death", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnCreatureDeathCommand, "", nullptr },
        { "dbscripts_on_event",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnEventCommand,        "", nullptr },
        { "dbscripts_on_gossip",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnGossipCommand,       "", nullptr },
        { "dbscripts_on_go_use",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnGoUseCommand,        "", nullptr },
        { "dbscripts_on_quest_end",      SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnQuestEndCommand,     "", nullptr },
        { "dbscripts_on_quest_start",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnQuestStartCommand,   "", nullptr },
        { "dbscripts_on_spell",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnSpellCommand,        "", nullptr },
        { "dbscripts_on_relay",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadDBScriptsOnRelayCommand,        "", nullptr },
        { "disenchant_loot_template",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesDisenchantCommand, "", nullptr },
        { "fishing_loot_template",       SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesFishingCommand,    "", nullptr },
        { "game_graveyard_zone",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGameGraveyardZoneCommand,       "", nullptr },
        { "game_tele",                   SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGameTeleCommand,                "", nullptr },
        { "gameobject_involvedrelation", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGOQuestInvRelationsCommand,     "", nullptr },
        { "gameobject_loot_template",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesGameobjectCommand, "", nullptr },
        { "gameobject_questrelation",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGOQuestRelationsCommand,        "", nullptr },
        { "gameobject_battleground",     SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadBattleEventCommand,             "", nullptr },
        { "gossip_menu",                 SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGossipMenuCommand,              "", nullptr },
        { "gossip_menu_option",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadGossipMenuCommand,              "", nullptr },
        { "item_enchantment_template",   SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadItemEnchantementsCommand,       "", nullptr },
        { "item_loot_template",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesItemCommand,       "", nullptr },
        { "item_required_target",        SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadItemRequiredTragetCommand,      "", nullptr },
        { "locales_creature",            SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesCreatureCommand,         "", nullptr },
        { "locales_gameobject",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesGameobjectCommand,       "", nullptr },
        { "locales_gossip_menu_option",  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesGossipMenuOptionCommand, "", nullptr },
        { "locales_item",                SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesItemCommand,             "", nullptr },
        { "locales_npc_text",            SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesNpcTextCommand,          "", nullptr },
        { "locales_page_text",           SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesPageTextCommand,         "", nullptr },
        { "locales_points_of_interest",  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesPointsOfInterestCommand, "", nullptr },
        { "locales_quest",               SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesQuestCommand,            "", nullptr },
        { "locales_questgiver_greeting", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestgiverGreetingLocalesCommand, "", nullptr },
        { "locales_trainer_greeting",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesTrainerGreetingCommand,  "", nullptr },
        { "locales_areatrigger_teleport", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLocalesAreaTriggerCommand,     "", nullptr },
        { "mail_loot_template",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesMailCommand,       "", nullptr },
        { "mangos_string",               SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadMangosStringCommand,            "", nullptr },
        { "npc_gossip",                  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcGossipCommand,               "", nullptr },
        { "npc_text",                    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcTextCommand,                 "", nullptr },
        { "npc_trainer",                 SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcTrainerCommand,              "", nullptr },
        { "npc_vendor",                  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadNpcVendorCommand,               "", nullptr },
        { "page_text",                   SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadPageTextsCommand,               "", nullptr },
        { "pickpocketing_loot_template", SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesPickpocketingCommand, "", nullptr},
        { "points_of_interest",          SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadPointsOfInterestCommand,        "", nullptr },
        { "questgiver_greeting",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestgiverGreetingCommand,      "", nullptr },
        { "quest_template",              SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadQuestTemplateCommand,           "", nullptr },
        { "reference_loot_template",     SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesReferenceCommand,  "", nullptr },
        { "reserved_name",               SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadReservedNameCommand,            "", nullptr },
        { "reputation_reward_rate",      SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadReputationRewardRateCommand,    "", nullptr },
        { "reputation_spillover_template", SEC_ADMINISTRATOR, true, &ChatHandler::HandleReloadReputationSpilloverTemplateCommand, "", nullptr },
        { "skill_fishing_base_level",    SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSkillFishingBaseLevelCommand,   "", nullptr },
        { "skinning_loot_template",      SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadLootTemplatesSkinningCommand,   "", nullptr },
        { "spam_records",                SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadExpectedSpamRecords,            "", nullptr },
        { "spell_affect",                SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellAffectCommand,             "", nullptr },
        { "spell_area",                  SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellAreaCommand,               "", nullptr },
        { "spell_bonus_data",            SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellBonusesCommand,            "", nullptr },
        { "spell_chain",                 SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellChainCommand,              "", nullptr },
        { "spell_elixir",                SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellElixirCommand,             "", nullptr },
        { "spell_learn_spell",           SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellLearnSpellCommand,         "", nullptr },
        { "spell_pet_auras",             SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellPetAurasCommand,           "", nullptr },
        { "spell_proc_event",            SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellProcEventCommand,          "", nullptr },
        { "spell_proc_item_enchant",     SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellProcItemEnchantCommand,    "", nullptr },
        { "spell_script_target",         SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellScriptTargetCommand,       "", nullptr },
        { "spell_target_position",       SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellTargetPositionCommand,     "", nullptr },
        { "spell_threats",               SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadSpellThreatsCommand,            "", nullptr },
        { "taxi_shortcuts",              SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadTaxiShortcuts,              "", nullptr },
        { "trainer_greeting",            SEC_ADMINISTRATOR, true,  &ChatHandler::HandleReloadTrainerGreetingCommand,         "", nullptr },

        { nullptr,                       0,                 false, nullptr,                                                  "", nullptr }
    };

    static ChatCommand resetCommandTable[] =
    {
        { "honor",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetHonorCommand,          "", nullptr },
        { "level",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetLevelCommand,          "", nullptr },
        { "spells",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetSpellsCommand,         "", nullptr },
        { "stats",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetStatsCommand,          "", nullptr },
        { "talents",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetTalentsCommand,        "", nullptr },
        { "taxinodes",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetTaxiNodesCommand,      "", nullptr },
        { "mods",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetModsCommand,           "", nullptr },
        { "all",            SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleResetAllCommand,            "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand sendMassCommandTable[] =
    {
        { "items",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassItemsCommand,       "", nullptr },
        { "mail",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassMailCommand,        "", nullptr },
        { "money",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMassMoneyCommand,       "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand sendCommandTable[] =
    {
        { "mass",           SEC_ADMINISTRATOR,  true,  nullptr,                                        "", sendMassCommandTable },
        { "items",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendItemsCommand,           "", nullptr },
        { "mail",           SEC_MODERATOR,      true,  &ChatHandler::HandleSendMailCommand,            "", nullptr },
        { "message",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMessageCommand,         "", nullptr },
        { "money",          SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleSendMoneyCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand movementCommandTable[] =
    {
        { "movegens",       SEC_GAMEMASTER,     false, &ChatHandler::HandleMovegensCommand,            "", nullptr },
        { "cometome",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleComeToMeCommand,            "", nullptr },
        { "movespeed",      SEC_GAMEMASTER,     false, &ChatHandler::HandleMovespeedShowCommand,       "", nullptr },
        { "debug",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDebugMovement,              "", nullptr },
        { "print",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandlePrintMovement,              "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand combatCommandTable[] =
    {
        { "stop",           SEC_GAMEMASTER,     false, &ChatHandler::HandleCombatStopCommand,          "", nullptr },
        { "list",           SEC_PLAYER,         false, &ChatHandler::HandleCombatListCommand,          "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverIdleRestartCommandTable[] =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand, "", nullptr },
        { "",               SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerIdleRestartCommand,   "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverIdleShutdownCommandTable[] =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand, "", nullptr },
        { "",               SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerIdleShutDownCommand,  "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverRestartCommandTable[] =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand, "", nullptr },
        { "",               SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerRestartCommand,       "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverShutdownCommandTable[] =
    {
        { "cancel",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCancelCommand, "", nullptr },
        { "",               SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerShutDownCommand,      "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverLogCommandTable[] =
    {
        { "filter",         SEC_CONSOLE,        true,  &ChatHandler::HandleServerLogFilterCommand,     "", nullptr },
        { "level",          SEC_CONSOLE,        true,  &ChatHandler::HandleServerLogLevelCommand,      "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverSetCommandTable[] =
    {
        { "motd",           SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerSetMotdCommand,       "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand serverCommandTable[] =
    {
        { "corpses",        SEC_GAMEMASTER,     true,  &ChatHandler::HandleServerCorpsesCommand,       "", nullptr },
        { "exit",           SEC_CONSOLE,        true,  &ChatHandler::HandleServerExitCommand,          "", nullptr },
        { "idlerestart",    SEC_ADMINISTRATOR,  true,  nullptr,                                        "", serverIdleRestartCommandTable },
        { "idleshutdown",   SEC_ADMINISTRATOR,  true,  nullptr,                                        "", serverIdleShutdownCommandTable },
        { "info",           SEC_PLAYER,         true,  &ChatHandler::HandleServerInfoCommand,          "", nullptr },
        { "log",            SEC_CONSOLE,        true,  nullptr,                                        "", serverLogCommandTable },
        { "motd",           SEC_PLAYER,         true,  &ChatHandler::HandleServerMotdCommand,          "", nullptr },
        { "plimit",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerPLimitCommand,        "", nullptr },
        { "resetallraid",   SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleServerResetAllRaidCommand,  "", nullptr },
        { "restart",        SEC_ADMINISTRATOR,  true,  nullptr,                                        "", serverRestartCommandTable },
        { "shutdown",       SEC_ADMINISTRATOR,  true,  nullptr,                                        "", serverShutdownCommandTable },
        { "set",            SEC_ADMINISTRATOR,  true,  nullptr,                                        "", serverSetCommandTable },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand teleCommandTable[] =
    {
        { "add",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleTeleAddCommand,             "", nullptr },
        { "del",            SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleTeleDelCommand,             "", nullptr },
        { "name",           SEC_MODERATOR,      true,  &ChatHandler::HandleTeleNameCommand,            "", nullptr },
        { "group",          SEC_MODERATOR,      false, &ChatHandler::HandleTeleGroupCommand,           "", nullptr },
        { "",               SEC_MODERATOR,      false, &ChatHandler::HandleTeleCommand,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand triggerCommandTable[] =
    {
        { "active",         SEC_GAMEMASTER,     false, &ChatHandler::HandleTriggerActiveCommand,       "", nullptr },
        { "near",           SEC_GAMEMASTER,     false, &ChatHandler::HandleTriggerNearCommand,         "", nullptr },
        { "",               SEC_GAMEMASTER,     true,  &ChatHandler::HandleTriggerCommand,             "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand unbanCommandTable[] =
    {
        { "account",        SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanAccountCommand,      "", nullptr },
        { "character",      SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanCharacterCommand,    "", nullptr },
        { "ip",             SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleUnBanIPCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand wpCommandTable[] =
    {
        { "show",           SEC_GAMEMASTER,     false, &ChatHandler::HandleWpShowCommand,              "", nullptr },
        { "add",            SEC_GAMEMASTER,     false, &ChatHandler::HandleWpAddCommand,               "", nullptr },
        { "modify",         SEC_GAMEMASTER,     false, &ChatHandler::HandleWpModifyCommand,            "", nullptr },
        { "export",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleWpExportCommand,            "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand worldStateTable[] =
    {
        { "wareffort",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleWarEffortCommand,           "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand testingCommandTable[] =
    {
        { "bags",           SEC_GAMEMASTER,     false, &ChatHandler::HandleBagsCommand,                "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand ticketCommandTable[] =
    {
        { "discard",        SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketDiscardCommand,       "", nullptr },
        { "escalate",       SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketEscalateCommand,      "", nullptr },
        { "go",             SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketGoCommand,            "", nullptr },
        { "goname",         SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketGoNameCommand,        "", nullptr },
        { "note",           SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketNoteCommand,          "", nullptr },
        { "read",           SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketReadCommand,          "", nullptr },
        { "resolve",        SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketResolveCommand,       "", nullptr },
        { "sort",           SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketSortCommand,          "", nullptr },
        { "whisper",        SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketWhisperCommand,       "", nullptr },
        { "",               SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketCommand,              "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand ticketsCommandTable[] =
    {
        { "list",           SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketsListCommand,         "", nullptr },
        { "queue",          SEC_ADMINISTRATOR,  false, &ChatHandler::HandleTicketsQueueCommand,        "", nullptr },
        { "",               SEC_GAMEMASTER,     false, &ChatHandler::HandleTicketsCommand,             "", nullptr },
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    static ChatCommand battlegroundCommandTable[] =
    {
        { "start",         SEC_GAMEMASTER,     false,  &ChatHandler::HandleBattlegroundStartCommand,   "", nullptr },
        { "stop",          SEC_GAMEMASTER,     false,  &ChatHandler::HandleBattlegroundStopCommand,    "", nullptr },
        { nullptr,         0,                  false,  nullptr,                                        "", nullptr }
    };

    static ChatCommand commandTable[] =
    {
        { "account",        SEC_PLAYER,         true,  nullptr,                                           "", accountCommandTable  },
        { "auction",        SEC_ADMINISTRATOR,  false, nullptr,                                           "", auctionCommandTable  },
#ifdef BUILD_AHBOT
        { "ahbot",          SEC_ADMINISTRATOR,  true,  nullptr,                                           "", ahbotCommandTable    },
#endif
        { "cast",           SEC_ADMINISTRATOR,  false, nullptr,                                           "", castCommandTable     },
        { "character",      SEC_GAMEMASTER,     true,  nullptr,                                           "", characterCommandTable},
        { "channel",        SEC_MODERATOR,      false, nullptr,                                           "", channelCommandTable  },
        { "debug",          SEC_MODERATOR,      true,  nullptr,                                           "", debugCommandTable    },
        { "event",          SEC_GAMEMASTER,     false, nullptr,                                           "", eventCommandTable    },
        { "gm",             SEC_PLAYER,         true,  nullptr,                                           "", gmCommandTable       },
        { "honor",          SEC_GAMEMASTER,     false, nullptr,                                           "", honorCommandTable    },
        { "go",             SEC_MODERATOR,      false, nullptr,                                           "", goCommandTable       },
        { "gobject",        SEC_GAMEMASTER,     false, nullptr,                                           "", gobjectCommandTable  },
        { "guild",          SEC_GAMEMASTER,     true,  nullptr,                                           "", guildCommandTable    },
        { "instance",       SEC_ADMINISTRATOR,  true,  nullptr,                                           "", instanceCommandTable },
        { "learn",          SEC_MODERATOR,      false, nullptr,                                           "", learnCommandTable    },
        { "link",           SEC_ADMINISTRATOR,  false, nullptr,                                           "", linkCommandTable     },
        { "list",           SEC_ADMINISTRATOR,  true,  nullptr,                                           "", listCommandTable     },
        { "lookup",         SEC_MODERATOR,      true,  nullptr,                                           "", lookupCommandTable   },
        { "modify",         SEC_MODERATOR,      false, nullptr,                                           "", modifyCommandTable   },
        { "npc",            SEC_MODERATOR,      false, nullptr,                                           "", npcCommandTable      },
        { "pet",            SEC_GAMEMASTER,     true,  nullptr,                                           "", petCommandTable      },
        { "pool",           SEC_GAMEMASTER,     true,  nullptr,                                           "", poolCommandTable     },
        { "pdump",          SEC_ADMINISTRATOR,  true,  nullptr,                                           "", pdumpCommandTable    },
        { "quest",          SEC_ADMINISTRATOR,  false, nullptr,                                           "", questCommandTable    },
        { "reload",         SEC_ADMINISTRATOR,  true,  nullptr,                                           "", reloadCommandTable   },
        { "reset",          SEC_ADMINISTRATOR,  true,  nullptr,                                           "", resetCommandTable    },
        { "server",         SEC_PLAYER,         true,  nullptr,                                           "", serverCommandTable   },
        { "tele",           SEC_MODERATOR,      true,  nullptr,                                           "", teleCommandTable     },
        { "trigger",        SEC_GAMEMASTER,     false, nullptr,                                           "", triggerCommandTable  },
        { "wp",             SEC_GAMEMASTER,     false, nullptr,                                           "", wpCommandTable       },
        { "bg",             SEC_GAMEMASTER,     false, nullptr,                                           "", battlegroundCommandTable },

        { "aura",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAuraCommand,                "", nullptr },
        { "unaura",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleUnAuraCommand,              "", nullptr },
        { "announce",       SEC_MODERATOR,      true,  &ChatHandler::HandleAnnounceCommand,            "", nullptr },
        { "notify",         SEC_MODERATOR,      true,  &ChatHandler::HandleNotifyCommand,              "", nullptr },
        { "goname",         SEC_MODERATOR,      false, &ChatHandler::HandleGonameCommand,              "", nullptr },
        { "namego",         SEC_MODERATOR,      false, &ChatHandler::HandleNamegoCommand,              "", nullptr },
        { "groupgo",        SEC_MODERATOR,      false, &ChatHandler::HandleGroupgoCommand,             "", nullptr },
        { "commands",       SEC_PLAYER,         true,  &ChatHandler::HandleCommandsCommand,            "", nullptr },
        { "demorph",        SEC_GAMEMASTER,     false, &ChatHandler::HandleDeMorphCommand,             "", nullptr },
        { "die",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDieCommand,                 "", nullptr },
        { "revive",         SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleReviveCommand,              "", nullptr },
        { "dismount",       SEC_PLAYER,         false, &ChatHandler::HandleDismountCommand,            "", nullptr },
        { "gps",            SEC_MODERATOR,      false, &ChatHandler::HandleGPSCommand,                 "", nullptr },
        { "guid",           SEC_GAMEMASTER,     false, &ChatHandler::HandleGUIDCommand,                "", nullptr },
        { "help",           SEC_PLAYER,         true,  &ChatHandler::HandleHelpCommand,                "", nullptr },
        { "itemmove",       SEC_GAMEMASTER,     false, &ChatHandler::HandleItemMoveCommand,            "", nullptr },
        { "cooldown",       SEC_ADMINISTRATOR,  false, nullptr,                                        "", cooldownCommandTable },
        { "unlearn",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleUnLearnCommand,             "", nullptr },
        { "distance",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGetDistanceCommand,         "", nullptr },
        { "los",            SEC_ADMINISTRATOR,  false, &ChatHandler::HandleGetLosCommand,              "", nullptr },
        { "recall",         SEC_MODERATOR,      false, &ChatHandler::HandleRecallCommand,              "", nullptr },
        { "save",           SEC_PLAYER,         false, &ChatHandler::HandleSaveCommand,                "", nullptr },
        { "saveall",        SEC_MODERATOR,      true,  &ChatHandler::HandleSaveAllCommand,             "", nullptr },
        { "kick",           SEC_GAMEMASTER,     true,  &ChatHandler::HandleKickPlayerCommand,          "", nullptr },
        { "ban",            SEC_GAMEMASTER,     true,  nullptr,                                        "", banCommandTable      },
        { "unban",          SEC_ADMINISTRATOR,  true,  nullptr,                                        "", unbanCommandTable    },
        { "baninfo",        SEC_ADMINISTRATOR,  false, nullptr,                                        "", baninfoCommandTable  },
        { "banlist",        SEC_ADMINISTRATOR,  true,  nullptr,                                        "", banlistCommandTable  },
        { "start",          SEC_PLAYER,         false, &ChatHandler::HandleStartCommand,               "", nullptr },
        { "taxicheat",      SEC_MODERATOR,      false, &ChatHandler::HandleTaxiCheatCommand,           "", nullptr },
        { "linkgrave",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLinkGraveCommand,           "", nullptr },
        { "neargrave",      SEC_ADMINISTRATOR,  false, &ChatHandler::HandleNearGraveCommand,           "", nullptr },
        { "explorecheat",   SEC_ADMINISTRATOR,  false, &ChatHandler::HandleExploreCheatCommand,        "", nullptr },
        { "levelup",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleLevelUpCommand,             "", nullptr },
        { "showarea",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleShowAreaCommand,            "", nullptr },
        { "hidearea",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleHideAreaCommand,            "", nullptr },
        { "additem",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAddItemCommand,             "", nullptr },
        { "additemset",     SEC_ADMINISTRATOR,  false, &ChatHandler::HandleAddItemSetCommand,          "", nullptr },
        { "bank",           SEC_ADMINISTRATOR,  false, &ChatHandler::HandleBankCommand,                "", nullptr },
        { "wchange",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleChangeWeatherCommand,       "", nullptr },
        { "testing",        SEC_GAMEMASTER,     false, nullptr,                                        "", testingCommandTable },
        { "ticket",         SEC_GAMEMASTER,     false, nullptr,                                        "", ticketCommandTable   },
        { "tickets",        SEC_GAMEMASTER,     false, nullptr,                                        "", ticketsCommandTable  },
        { "maxskill",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleMaxSkillCommand,            "", nullptr },
        { "setskill",       SEC_ADMINISTRATOR,  false, &ChatHandler::HandleSetSkillCommand,            "", nullptr },
        { "whispers",       SEC_MODERATOR,      false, &ChatHandler::HandleWhispersCommand,            "", nullptr },
        { "pinfo",          SEC_GAMEMASTER,     true,  &ChatHandler::HandlePInfoCommand,               "", nullptr },
        { "respawn",        SEC_ADMINISTRATOR,  false, &ChatHandler::HandleRespawnCommand,             "", nullptr },
        { "send",           SEC_MODERATOR,      true,  nullptr,                                        "", sendCommandTable     },
        { "loadscripts",    SEC_ADMINISTRATOR,  true,  &ChatHandler::HandleLoadScriptsCommand,         "", nullptr },
        { "mute",           SEC_MODERATOR,      true,  &ChatHandler::HandleMuteCommand,                "", nullptr },
        { "unmute",         SEC_MODERATOR,      true,  &ChatHandler::HandleUnmuteCommand,              "", nullptr },
        { "movement",       SEC_PLAYER,         true,  nullptr,                                        "", movementCommandTable },
        { "damage",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleDamageCommand,              "", nullptr },
        { "combat",         SEC_PLAYER,         false, nullptr,                                        "", combatCommandTable },
        { "repairitems",    SEC_GAMEMASTER,     true,  &ChatHandler::HandleRepairitemsCommand,         "", nullptr },
        { "stable",         SEC_ADMINISTRATOR,  false, &ChatHandler::HandleStableCommand,              "", nullptr },
        { "waterwalk",      SEC_GAMEMASTER,     false, &ChatHandler::HandleWaterwalkCommand,           "", nullptr },
        { "quit",           SEC_CONSOLE,        true,  &ChatHandler::HandleQuitCommand,                "", nullptr },
        { "mmap",           SEC_GAMEMASTER,     false, nullptr,                                        "", mmapCommandTable },
#ifdef BUILD_PLAYERBOT
        { "bot",            SEC_PLAYER,         false, &ChatHandler::HandlePlayerbotCommand,           "", nullptr },
#endif
        { nullptr,          0,                  false, nullptr,                                        "", nullptr }
    };

    if (load_command_table)
    {
        load_command_table = false;

        // check hardcoded part integrity
        CheckIntegrity(commandTable, nullptr);

        QueryResult* result = WorldDatabase.Query("SELECT name,security,help FROM command");
        if (result)
        {
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].GetCppString();

                SetDataForCommandInTable(commandTable, name.c_str(), fields[1].GetUInt16(), fields[2].GetCppString());
            }
            while (result->NextRow());
            delete result;
        }
    }

    return commandTable;
}

ChatHandler::ChatHandler(WorldSession* session) : m_session(session), sentErrorMessage(false)
{}

ChatHandler::ChatHandler(Player* player) : m_session(player->GetSession()), sentErrorMessage(false) {}

ChatHandler::~ChatHandler() {}

const char* ChatHandler::GetMangosString(int32 entry) const
{
    return m_session->GetMangosString(entry);
}

const char* ChatHandler::GetOnOffStr(bool value) const
{
    return value ?  GetMangosString(LANG_ON) : GetMangosString(LANG_OFF);
}

uint32 ChatHandler::GetAccountId() const
{
    return m_session->GetAccountId();
}

AccountTypes ChatHandler::GetAccessLevel() const
{
    return m_session->GetSecurity();
}

bool ChatHandler::isAvailable(ChatCommand const& cmd) const
{
    // check security level only for simple  command (without child commands)
    return GetAccessLevel() >= (AccountTypes)cmd.SecurityLevel;
}

std::string ChatHandler::GetNameLink() const
{
    return GetNameLink(m_session->GetPlayer());
}

bool ChatHandler::HasLowerSecurity(Player* target, ObjectGuid guid, bool strong)
{
    WorldSession* target_session = nullptr;
    uint32 target_account = 0;

    if (target)
        target_session = target->GetSession();
    else
        target_account = sObjectMgr.GetPlayerAccountIdByGUID(guid);

    if (!target_session && !target_account)
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return true;
    }

    return HasLowerSecurityAccount(target_session, target_account, strong);
}

bool ChatHandler::HasLowerSecurityAccount(WorldSession* target, uint32 target_account, bool strong)
{
    AccountTypes target_sec;

    // ignore only for non-players for non strong checks (when allow apply command at least to same sec level)
    if (GetAccessLevel() > SEC_PLAYER && !strong && !sWorld.getConfig(CONFIG_BOOL_GM_LOWER_SECURITY))
        return false;

    if (target)
        target_sec = target->GetSecurity();
    else if (target_account)
        target_sec = sAccountMgr.GetSecurity(target_account);
    else
        return true;                                        // caller must report error for (target==nullptr && target_account==0)

    if (GetAccessLevel() < target_sec || (strong && GetAccessLevel() <= target_sec))
    {
        SendSysMessage(LANG_YOURS_SECURITY_IS_LOW);
        SetSentErrorMessage(true);
        return true;
    }

    return false;
}

bool ChatHandler::hasStringAbbr(const char* name, const char* part) const
{
    // non "" command
    if (*name)
    {
        // "" part from non-"" command
        if (!*part)
            return false;

        for (;;)
        {
            if (!*part)
                return true;
            if (!*name)
                return false;
            if (tolower(*name) != tolower(*part))
                return false;
            ++name; ++part;
        }
    }
    // allow with any for ""

    return true;
}

void ChatHandler::SendSysMessage(const char* str)
{
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;

    while (char* line = LineFromMessage(pos))
    {
        ChatHandler::BuildChatPacket(data, CHAT_MSG_SYSTEM, line);
        m_session->SendPacket(data);
    }

    delete[] buf;
}

void ChatHandler::SendGlobalSysMessage(const char* str) const
{
    // Chat output
    WorldPacket data;

    // need copy to prevent corruption by strtok call in LineFromMessage original string
    char* buf = mangos_strdup(str);
    char* pos = buf;
    ObjectGuid guid = m_session ? m_session->GetPlayer()->GetObjectGuid() : ObjectGuid();

    while (char* line = LineFromMessage(pos))
    {
        ChatHandler::BuildChatPacket(data, CHAT_MSG_SYSTEM, line);
        sWorld.SendGlobalMessage(data);
    }

    delete[] buf;
}

void ChatHandler::SendSysMessage(int32 entry)
{
    SendSysMessage(GetMangosString(entry));
}

void ChatHandler::PSendSysMessage(int32 entry, ...)
{
    const char* format = GetMangosString(entry);
    va_list ap;
    char str [2048];
    va_start(ap, entry);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::PSendSysMessage(const char* format, ...)
{
    va_list ap;
    char str [2048];
    va_start(ap, format);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);
    SendSysMessage(str);
}

void ChatHandler::CheckIntegrity(ChatCommand* table, ChatCommand* parentCommand) const
{
    for (uint32 i = 0; table[i].Name != nullptr; ++i)
    {
        ChatCommand* command = &table[i];

        if (parentCommand && command->SecurityLevel < parentCommand->SecurityLevel)
            sLog.outError("Subcommand '%s' of command '%s' have less access level (%u) that parent (%u)",
                          command->Name, parentCommand->Name, command->SecurityLevel, parentCommand->SecurityLevel);

        if (!parentCommand && command->Name[0] == '\0')
            sLog.outError("Subcommand '' at top level");

        if (command->ChildCommands)
        {
            if (command->Handler)
            {
                if (parentCommand)
                    sLog.outError("Subcommand '%s' of command '%s' have handler and subcommands in same time, must be used '' subcommand for handler instead.",
                                  command->Name, parentCommand->Name);
                else
                    sLog.outError("First level command '%s' have handler and subcommands in same time, must be used '' subcommand for handler instead.",
                                  command->Name);
            }

            if (parentCommand && command->Name[0] == '\0')
                sLog.outError("Subcommand '' of command '%s' have subcommands", parentCommand->Name);

            CheckIntegrity(command->ChildCommands, command);
        }
        else if (!command->Handler)
        {
            if (parentCommand)
                sLog.outError("Subcommand '%s' of command '%s' not have handler and subcommands in same time. Must have some from its!",
                              command->Name, parentCommand->Name);
            else
                sLog.outError("First level command '%s' not have handler and subcommands in same time. Must have some from its!",
                              command->Name);
        }
    }
}

/**
 * Search (sub)command for command line available for chat handler access level
 *
 * @param text  Command line string that will parsed for (sub)command search
 *
 * @return Pointer to found command structure or nullptr if appropriate command not found
 */
ChatCommand const* ChatHandler::FindCommand(char const* text)
{
    ChatCommand* command = nullptr;
    char const* textPtr = text;

    return FindCommand(getCommandTable(), textPtr, command) == CHAT_COMMAND_OK ? command : nullptr;
}

/**
 * Search (sub)command for command line available for chat handler access level with options and fail case additional info
 *
 * @param table         Pointer to command C-style array first level command where will be searched
 * @param text          Command line string that will parsed for (sub)command search,
 *                      it modified at return from function and pointed to not parsed tail
 * @param command       At success this is found command, at other cases this is last found parent command
 *                      before subcommand search fail
 * @param parentCommand Output arg for optional return parent command for command arg.
 * @param cmdNamePtr    Output arg for optional return last parsed command name.
 * @param allAvailable  Optional arg (with false default value) control use command access level checks while command search.
 * @param exactlyName   Optional arg (with false default value) control use exactly name in checks while command search.
 *
 * @return one from enum value of ChatCommandSearchResult. Output args return values highly dependent from this return result:
 *
 *      CHAT_COMMAND_OK       - Command found!
 *                              text point to non parsed tail with possible command specific data, command store found command pointer,
 *                              parentCommand have parent of found command or nullptr if command found in table array directly
 *                              cmdNamePtr store found command name in original form from command line
 *      CHAT_COMMAND_UNKNOWN  - Command not found in table directly
 *                              text only skip possible whitespaces,
 *                              command is nullptr
 *                              parentCommand is nullptr
 *                              cmdNamePtr store command name that not found as it extracted from command line
 *      CHAT_COMMAND_UNKNOWN_SUBCOMMAND - Subcommand not found in some deed subcomand lists
 *                              text point to non parsed tail including not found command name in command line,
 *                              command store last found parent command if any
 *                              parentCommand have parent of command in command arg or nullptr
 *                              cmdNamePtr store command name that not found as it extracted from command line
 */
ChatCommandSearchResult ChatHandler::FindCommand(ChatCommand* table, char const*& text, ChatCommand*& command, ChatCommand** parentCommand /*= nullptr*/, std::string* cmdNamePtr /*= nullptr*/, bool allAvailable /*= false*/, bool exactlyName /*= false*/) const
{
    std::string cmd;

    // skip whitespaces
    while (*text != ' ' && *text != '\0')
    {
        cmd += *text;
        ++text;
    }

    while (*text == ' ') ++text;

    // search first level command in table
    for (uint32 i = 0; table[i].Name != nullptr; ++i)
    {
        if (exactlyName)
        {
            size_t len = strlen(table[i].Name);
            if (strncmp(table[i].Name, cmd.c_str(), len + 1) != 0)
                continue;
        }
        else
        {
            if (!hasStringAbbr(table[i].Name, cmd.c_str()))
                continue;
        }
        // select subcommand from child commands list
        if (table[i].ChildCommands != nullptr)
        {
            char const* oldchildtext = text;
            ChatCommand* parentSubcommand = nullptr;
            ChatCommandSearchResult res = FindCommand(table[i].ChildCommands, text, command, &parentSubcommand, cmdNamePtr, allAvailable, exactlyName);

            switch (res)
            {
                case CHAT_COMMAND_OK:
                {
                    // if subcommand success search not return parent command, then this parent command is owner of child commands
                    if (parentCommand)
                        *parentCommand = parentSubcommand ? parentSubcommand : &table[i];

                    // Name == "" is special case: restore original command text for next level "" (where parentSubcommand==nullptr)
                    if (command->Name[0] == '\0' && !parentSubcommand)
                        text = oldchildtext;

                    return CHAT_COMMAND_OK;
                }
                case CHAT_COMMAND_UNKNOWN:
                {
                    // command not found directly in child command list, return child command list owner
                    command = &table[i];
                    if (parentCommand)
                        *parentCommand = nullptr;              // we don't known parent of table list at this point

                    text = oldchildtext;                    // restore text to stated just after parse found parent command
                    return CHAT_COMMAND_UNKNOWN_SUBCOMMAND; // we not found subcommand for table[i]
                }
                case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
                default:
                {
                    // some deep subcommand not found, if this second level subcommand then parentCommand can be nullptr, use known value for it
                    if (parentCommand)
                        *parentCommand = parentSubcommand ? parentSubcommand : &table[i];
                    return res;
                }
            }
        }

        // must be available (not checked for subcommands case because parent command expected have most low access that all subcommands always
        if (!allAvailable && !isAvailable(table[i]))
            continue;

        // must be have handler is explicitly selected
        if (!table[i].Handler)
            continue;

        // command found directly in to table
        command = &table[i];

        // unknown table owner at this point
        if (parentCommand)
            *parentCommand = nullptr;

        if (cmdNamePtr)
            *cmdNamePtr = cmd;

        return CHAT_COMMAND_OK;
    }

    // command not found in table directly
    command = nullptr;

    // unknown table owner at this point
    if (parentCommand)
        *parentCommand = nullptr;

    if (cmdNamePtr)
        *cmdNamePtr = cmd;

    return CHAT_COMMAND_UNKNOWN;
}

/**
 * Execute (sub)command available for chat handler access level with options in command line string
 *
 * @param text  Command line string that will parsed for (sub)command search and command specific data
 *
 * Command output and errors in command execution will send to chat handler.
 */
void ChatHandler::ExecuteCommand(const char* text)
{
    std::string fullcmd = text;                             // original `text` can't be used. It content destroyed in command code processing.

    ChatCommand* command = nullptr;
    ChatCommand* parentCommand = nullptr;

    ChatCommandSearchResult res = FindCommand(getCommandTable(), text, command, &parentCommand);

    switch (res)
    {
        case CHAT_COMMAND_OK:
        {
            SetSentErrorMessage(false);
            if ((this->*(command->Handler))((char*)text))   // text content destroyed at call
            {
                if (command->SecurityLevel > SEC_PLAYER)
                    LogCommand(fullcmd.c_str());
            }
            // some commands have custom error messages. Don't send the default one in these cases.
            else if (!HasSentErrorMessage())
            {
                if (!command->Help.empty())
                    SendSysMessage(command->Help.c_str());
                else
                    SendSysMessage(LANG_CMD_SYNTAX);

                if (ChatCommand* showCommand = (command->Name[0] == '\0' && parentCommand ? parentCommand : command))
                    if (ChatCommand* childs = showCommand->ChildCommands)
                        ShowHelpForSubCommands(childs, showCommand->Name);

                SetSentErrorMessage(true);
            }
            break;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
        {
            SendSysMessage(LANG_NO_SUBCMD);
            ShowHelpForCommand(command->ChildCommands, text);
            SetSentErrorMessage(true);
            break;
        }
        case CHAT_COMMAND_UNKNOWN:
        {
            SendSysMessage(LANG_NO_CMD);
            SetSentErrorMessage(true);
            break;
        }
    }
}

/**
 * Function find appropriate command and update command security level and help text
 *
 * @param commandTable  Table for first level command search
 * @param text          Command line string that will parsed for (sub)command search
 * @param security      New security level for command
 * @param help          New help text  for command
 *
 * @return true if command has been found, and false in other case
 *
 * All problems found while command search and updated output as to DB errors log
 */
bool ChatHandler::SetDataForCommandInTable(ChatCommand* commandTable, const char* text, uint32 security, std::string const& help)
{
    std::string fullcommand = text;                         // original `text` can't be used. It content destroyed in command code processing.

    ChatCommand* command = nullptr;
    std::string cmdName;

    ChatCommandSearchResult res = FindCommand(commandTable, text, command, nullptr, &cmdName, true, true);

    switch (res)
    {
        case CHAT_COMMAND_OK:
        {
            if (command->SecurityLevel != security)
                DETAIL_LOG("Table `command` overwrite for command '%s' default security (%u) by %u",
                           fullcommand.c_str(), command->SecurityLevel, security);

            command->SecurityLevel = security;
            command->Help          = help;
            return true;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
        {
            // command have subcommands, but not '' subcommand and then any data in `command` useless for it.
            if (cmdName.empty())
                sLog.outErrorDb("Table `command` have command '%s' that only used with some subcommand selection, it can't have help or overwritten access level, skip.", cmdName.c_str());
            else
                sLog.outErrorDb("Table `command` have unexpected subcommand '%s' in command '%s', skip.", cmdName.c_str(), fullcommand.c_str());
            return false;
        }
        case CHAT_COMMAND_UNKNOWN:
        {
            sLog.outErrorDb("Table `command` have nonexistent command '%s', skip.", cmdName.c_str());
            return false;
        }
    }

    return false;
}

bool ChatHandler::ParseCommands(const char* text)
{
    MANGOS_ASSERT(text);
    MANGOS_ASSERT(*text);

    /// chat case (.command or !command format)
    if (m_session)
    {
        if (m_session->GetSecurity() == SEC_PLAYER && !sWorld.getConfig(CONFIG_BOOL_PLAYER_COMMANDS))
            return false;

        if (text[0] != '!' && text[0] != '.')
            return false;

        /// ignore single . and ! in line
        if (strlen(text) < 2)
            return false;
    }

    /// ignore messages staring from many dots.
    if ((text[0] == '.' && text[1] == '.') || (text[0] == '!' && text[1] == '!'))
        return false;

    /// skip first . or ! (in console allowed use command with . and ! and without its)
    if (text[0] == '!' || text[0] == '.')
        ++text;

    ExecuteCommand(text);

    return true;
}

bool ChatHandler::ShowHelpForSubCommands(ChatCommand* table, char const* cmd)
{
    std::string list;
    for (uint32 i = 0; table[i].Name != nullptr; ++i)
    {
        // must be available (ignore handler existence for show command with possible available subcommands
        if (!isAvailable(table[i]))
            continue;

        if (m_session)
            list += "\n    ";
        else
            list += "\n\r    ";

        list += table[i].Name;

        if (table[i].ChildCommands)
            list += " ...";
    }

    if (list.empty())
        return false;

    if (table == getCommandTable())
    {
        SendSysMessage(LANG_AVIABLE_CMD);
        SendSysMessage(list.c_str());
    }
    else
    {
        PSendSysMessage(LANG_SUBCMDS_LIST, cmd);
        SendSysMessage(list.c_str());
    }
    return true;
}

bool ChatHandler::ShowHelpForCommand(ChatCommand* table, const char* cmd)
{
    char const* oldCmd = cmd;
    ChatCommand* command = nullptr;
    ChatCommand* parentCommand = nullptr;

    ChatCommand* showCommand = nullptr;
    ChatCommand* childCommands = nullptr;

    ChatCommandSearchResult res = FindCommand(table, cmd, command, &parentCommand);

    switch (res)
    {
        case CHAT_COMMAND_OK:
        {
            // for "" subcommand use parent command if any for subcommands list output
            if (command->Name[0] == '\0' && parentCommand)
            {
                showCommand = parentCommand;
                cmd = "";
            }
            else
                showCommand = command;

            childCommands = showCommand->ChildCommands;
            break;
        }
        case CHAT_COMMAND_UNKNOWN_SUBCOMMAND:
            showCommand = command;
            childCommands = showCommand->ChildCommands;
            break;
        case CHAT_COMMAND_UNKNOWN:
            // not show command list at error in first level command find fail
            childCommands = table != getCommandTable() || oldCmd[0] == '\0' ? table : nullptr;
            command = nullptr;
            break;
    }

    if (command && !command->Help.empty())
        SendSysMessage(command->Help.c_str());

    if (childCommands)
        if (ShowHelpForSubCommands(childCommands, showCommand ? showCommand->Name : ""))
            return true;

    if (command && command->Help.empty())
        SendSysMessage(LANG_NO_HELP_CMD);

    return command || childCommands;
}

bool ChatHandler::HasEscapeSequences(const char* message)
{
    while (*message)
    {
        // find next pipe command
        message = strchr(message, '|');

        if (!message)
            return false;

        ++message;

        char commandChar = *message;

        // ignore escaped pipe
        if (commandChar != '|')
            return true;

        ++message;
    }
    return false;
}

bool ChatHandler::CheckEscapeSequences(const char* message)
{
    /*

    valid examples:
    |cffa335ee|Hitem:812:0:0:0:0:0:0:0:70|h[Glowing Brightwood Staff]|h|r
    |cff808080|Hquest:2278:47|h[The Platinum Discs]|h|r
    |cff4e96f7|Htalent:2232:-1|h[Taste for Blood]|h|r
    |cff71d5ff|Hspell:21563|h[Command]|h|r
    |cffffd000|Henchant:3919|h[Engineering: Rough Dynamite]|h|r

    | will be escaped to ||
    */

    const char validSequence[6] = "cHhhr";
    const char* validSequenceIterator = validSequence;

    // more simple checks
    if (sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_SEVERITY) < 3)
    {
        const std::string validCommands = "cHhr|";

        while (*message)
        {
            // find next pipe command
            message = strchr(message, '|');

            if (!message)
                return true;

            ++message;
            char commandChar = *message;
            if (validCommands.find(commandChar) == std::string::npos)
                return false;

            ++message;
            // validate sequence
            if (sWorld.getConfig(CONFIG_UINT32_CHAT_STRICT_LINK_CHECKING_SEVERITY) == 2)
            {
                if (commandChar == *validSequenceIterator)
                {
                    if (validSequenceIterator == validSequence + 4)
                        validSequenceIterator = validSequence;
                    else
                        ++validSequenceIterator;
                }
                else if (commandChar != '|')
                    return false;
            }
        }
        return true;
    }

    std::istringstream reader(message);
    char buffer[256];

    uint32 color = 0;

    ItemPrototype const* linkedItem = nullptr;
    Quest const* linkedQuest = nullptr;
    SpellEntry const* linkedSpell = nullptr;
    ItemRandomPropertiesEntry const* itemProperty = nullptr;

    while (!reader.eof())
    {
        if (validSequence == validSequenceIterator)
        {
            linkedItem = nullptr;
            linkedQuest = nullptr;
            linkedSpell = nullptr;
            itemProperty = nullptr;

            reader.ignore(255, '|');
        }
        else if (reader.get() != '|')
        {
            DEBUG_LOG("ChatHandler::isValidChatMessage sequence aborted unexpectedly");
            return false;
        }

        // pipe has always to be followed by at least one char
        if (reader.peek() == '\0')
        {
            DEBUG_LOG("ChatHandler::isValidChatMessage pipe followed by \\0");
            return false;
        }

        // no further pipe commands
        if (reader.eof())
            break;

        char commandChar;
        reader >> std::noskipws >> commandChar;

        // | in normal messages is escaped by ||
        if (commandChar != '|')
        {
            if (commandChar == *validSequenceIterator)
            {
                if (validSequenceIterator == validSequence + 4)
                    validSequenceIterator = validSequence;
                else
                    ++validSequenceIterator;
            }
            else
            {
                DEBUG_LOG("ChatHandler::isValidChatMessage invalid sequence, expected %c but got %c", *validSequenceIterator, commandChar);
                return false;
            }
        }
        else if (validSequence != validSequenceIterator)
        {
            // no escaped pipes in sequences
            DEBUG_LOG("ChatHandler::isValidChatMessage got escaped pipe in sequence");
            return false;
        }

        switch (commandChar)
        {
            case 'c':
                color = 0;
                // validate color, expect 8 hex chars
                for (int i = 0; i < 8; ++i)
                {
                    char c;
                    reader >> c;
                    if (!c)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage got \\0 while reading color in |c command");
                        return false;
                    }

                    color <<= 4;
                    // check for hex char
                    if (c >= '0' && c <= '9')
                    {
                        color |= c - '0';
                        continue;
                    }
                    if (c >= 'a' && c <= 'f')
                    {
                        color |= 10 + c - 'a';
                        continue;
                    }
                    DEBUG_LOG("ChatHandler::isValidChatMessage got non hex char '%c' while reading color", c);
                    return false;
                }
                break;
            case 'H':
                // read chars up to colon  = link type
                reader.getline(buffer, 256, ':');
                if (reader.eof())                           // : must be
                    return false;

                if (strcmp(buffer, "item") == 0)
                {
                    // read item entry
                    reader.getline(buffer, 256, ':');
                    if (reader.eof())                       // : must be
                        return false;

                    linkedItem = ObjectMgr::GetItemPrototype(atoi(buffer));
                    if (!linkedItem)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage got invalid itemID %u in |item command", atoi(buffer));
                        return false;
                    }

                    if (color != ItemQualityColors[linkedItem->Quality])
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage linked item has color %u, but user claims %u", ItemQualityColors[linkedItem->Quality],
                                  color);
                        return false;
                    }

                    // the itementry is followed by several integers which describe an instance of this item

                    // position relative after itemEntry
                    const uint8 randomPropertyPosition = 6;

                    int32 propertyId = 0;
                    bool negativeNumber = false;
                    char c = '\0';
                    for (uint8 i = 0; i < randomPropertyPosition; ++i)
                    {
                        propertyId = 0;
                        negativeNumber = false;
                        while ((c = reader.get()) != ':')
                        {
                            if (c >= '0' && c <= '9')
                            {
                                propertyId *= 10;
                                propertyId += c - '0';
                            }
                            else if (c == '-')
                                negativeNumber = true;
                            else
                                return false;
                        }
                    }
                    if (negativeNumber)
                        propertyId *= -1;

                    if (propertyId > 0)
                    {
                        itemProperty = sItemRandomPropertiesStore.LookupEntry(propertyId);
                        if (!itemProperty)
                            return false;
                    }

                    // ignore other integers
                    while ((c >= '0' && c <= '9') || c == ':')
                    {
                        reader.ignore(1);
                        c = reader.peek();
                    }
                }
                else if (strcmp(buffer, "quest") == 0)
                {
                    // no color check for questlinks, each client will adapt it anyway
                    uint32 questid = 0;
                    // read questid
                    char c = reader.peek();
                    while (c >= '0' && c <= '9')
                    {
                        reader.ignore(1);
                        questid *= 10;
                        questid += c - '0';
                        c = reader.peek();
                    }

                    linkedQuest = sObjectMgr.GetQuestTemplate(questid);

                    if (!linkedQuest)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage Questtemplate %u not found", questid);
                        return false;
                    }

                    if (c != ':')
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage Invalid quest link structure");
                        return false;
                    }

                    reader.ignore(1);
                    c = reader.peek();
                    // level
                    uint32 questlevel = 0;
                    while (c >= '0' && c <= '9')
                    {
                        reader.ignore(1);
                        questlevel *= 10;
                        questlevel += c - '0';
                        c = reader.peek();
                    }

                    if (questlevel >= STRONG_MAX_LEVEL)
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage Quest level %u too big", questlevel);
                        return false;
                    }

                    if (c != '|')
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage Invalid quest link structure");
                        return false;
                    }
                }
                else if (strcmp(buffer, "talent") == 0)
                {
                    // talent links are always supposed to be blue
                    if (color != CHAT_LINK_COLOR_TALENT)
                        return false;

                    // read talent entry
                    reader.getline(buffer, 256, ':');
                    if (reader.eof())                       // : must be
                        return false;

                    TalentEntry const* talentInfo = sTalentStore.LookupEntry(atoi(buffer));
                    if (!talentInfo)
                        return false;

                    linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(talentInfo->RankID[0]);
                    if (!linkedSpell)
                        return false;

                    char c = reader.peek();
                    // skillpoints? whatever, drop it
                    while (c != '|' && c != '\0')
                    {
                        reader.ignore(1);
                        c = reader.peek();
                    }
                }
                else if (strcmp(buffer, "spell") == 0)
                {
                    if (color != CHAT_LINK_COLOR_SPELL)
                        return false;

                    uint32 spellid = 0;
                    // read spell entry
                    char c = reader.peek();
                    while (c >= '0' && c <= '9')
                    {
                        reader.ignore(1);
                        spellid *= 10;
                        spellid += c - '0';
                        c = reader.peek();
                    }
                    linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                    if (!linkedSpell)
                        return false;
                }
                else if (strcmp(buffer, "enchant") == 0)
                {
                    if (color != CHAT_LINK_COLOR_ENCHANT)
                        return false;

                    uint32 spellid = 0;
                    // read spell entry
                    char c = reader.peek();
                    while (c >= '0' && c <= '9')
                    {
                        reader.ignore(1);
                        spellid *= 10;
                        spellid += c - '0';
                        c = reader.peek();
                    }
                    linkedSpell = sSpellTemplate.LookupEntry<SpellEntry>(spellid);
                    if (!linkedSpell)
                        return false;
                }
                else
                {
                    DEBUG_LOG("ChatHandler::isValidChatMessage user sent unsupported link type '%s'", buffer);
                    return false;
                }
                break;
            case 'h':
                // if h is next element in sequence, this one must contain the linked text :)
                if (*validSequenceIterator == 'h')
                {
                    // links start with '['
                    if (reader.get() != '[')
                    {
                        DEBUG_LOG("ChatHandler::isValidChatMessage link caption doesn't start with '['");
                        return false;
                    }
                    reader.getline(buffer, 256, ']');
                    if (reader.eof())                       // ] must be
                        return false;

                    // verify the link name
                    if (linkedSpell)
                    {
                        // spells with that flag have a prefix of "$PROFESSION: "
                        if (linkedSpell->HasAttribute(SPELL_ATTR_TRADESPELL))
                        {
                            // lookup skillid
                            SkillLineAbilityMapBounds bounds = sSpellMgr.GetSkillLineAbilityMapBoundsBySpellId(linkedSpell->Id);
                            if (bounds.first == bounds.second)
                            {
                                return false;
                            }

                            SkillLineAbilityEntry const* skillInfo = bounds.first->second;

                            if (!skillInfo)
                            {
                                return false;
                            }

                            SkillLineEntry const* skillLine = sSkillLineStore.LookupEntry(skillInfo->skillId);
                            if (!skillLine)
                            {
                                return false;
                            }

                            for (uint8 i = 0; i < MAX_LOCALE; ++i)
                            {
                                uint32 skillLineNameLength = strlen(skillLine->name[i]);
                                if (skillLineNameLength > 0 && strncmp(skillLine->name[i], buffer, skillLineNameLength) == 0)
                                {
                                    // found the prefix, remove it to perform spellname validation below
                                    // -2 = strlen(": ")
                                    uint32 spellNameLength = strlen(buffer) - skillLineNameLength - 2;
                                    memmove(buffer, buffer + skillLineNameLength + 2, spellNameLength + 1);
                                }
                            }
                        }
                        bool foundName = false;
                        for (uint8 i = 0; i < MAX_LOCALE; ++i)
                        {
                            if (*linkedSpell->SpellName[i] && strcmp(linkedSpell->SpellName[i], buffer) == 0)
                            {
                                foundName = true;
                                break;
                            }
                        }
                        if (!foundName)
                            return false;
                    }
                    else if (linkedQuest)
                    {
                        if (linkedQuest->GetTitle() != buffer)
                        {
                            QuestLocale const* ql = sObjectMgr.GetQuestLocale(linkedQuest->GetQuestId());

                            if (!ql)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage default questname didn't match and there is no locale");
                                return false;
                            }

                            bool foundName = false;
                            for (const auto& i : ql->Title)
                            {
                                if (i == buffer)
                                {
                                    foundName = true;
                                    break;
                                }
                            }
                            if (!foundName)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage no quest locale title matched");
                                return false;
                            }
                        }
                    }
                    else if (linkedItem)
                    {
                        std::string expectedName = std::string(linkedItem->Name1);

                        if (expectedName != buffer)
                        {
                            ItemLocale const* il = sObjectMgr.GetItemLocale(linkedItem->ItemId);

                            bool foundName = false;
                            for (uint8 i = LOCALE_koKR; i < MAX_LOCALE; ++i)
                            {
                                int8 dbIndex = sObjectMgr.GetStorageLocaleIndexFor(LocaleConstant(i));
                                if (dbIndex == -1 || il == nullptr || (size_t)dbIndex >= il->Name.size())
                                    // using strange database/client combinations can lead to this case
                                    expectedName = linkedItem->Name1;
                                else
                                    expectedName = il->Name[dbIndex];

                                if (expectedName == buffer)
                                {
                                    foundName = true;
                                    break;
                                }
                            }
                            if (!foundName)
                            {
                                DEBUG_LOG("ChatHandler::isValidChatMessage linked item name wasn't found in any localization");
                                return false;
                            }
                        }
                    }
                    // that place should never be reached - if nothing linked has been set in |H
                    // it will return false before
                    else
                        return false;
                }
                break;
            case 'r':
            case '|':
                // no further payload
                break;
            default:
                DEBUG_LOG("ChatHandler::isValidChatMessage got invalid command |%c", commandChar);
                return false;
        }
    }

    // check if every opened sequence was also closed properly
    if (validSequence != validSequenceIterator)
        DEBUG_LOG("ChatHandler::isValidChatMessage EOF in active sequence");

    return validSequence == validSequenceIterator;
}

Player* ChatHandler::getSelectedPlayer() const
{
    if (!m_session)
        return nullptr;

    ObjectGuid guid  = m_session->GetPlayer()->GetSelectionGuid();

    if (!guid)
        return m_session->GetPlayer();

    return sObjectMgr.GetPlayer(guid);
}

Unit* ChatHandler::getSelectedUnit(bool self) const
{
    if (!m_session)
        return nullptr;

    ObjectGuid guid = m_session->GetPlayer()->GetSelectionGuid();

    if (!guid)
        return self ? m_session->GetPlayer() : nullptr;

    // can be selected player at another map
    return ObjectAccessor::GetUnit(*m_session->GetPlayer(), guid);
}

Creature* ChatHandler::getSelectedCreature() const
{
    if (!m_session)
        return nullptr;

    return m_session->GetPlayer()->GetMap()->GetAnyTypeCreature(m_session->GetPlayer()->GetSelectionGuid());
}

Pet* ChatHandler::getSelectedPet() const
{
    if (!m_session)
        return nullptr;

    return m_session->GetPlayer()->GetMap()->GetPet(m_session->GetPlayer()->GetSelectionGuid());
}

/**
 * Function skip all whitespaces in args string
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *             allowed nullptr string pointer stored in *args
 */
void ChatHandler::SkipWhiteSpaces(char** args)
{
    if (!*args)
        return;

    while (isWhiteSpace(**args))
        ++(*args);
}

/**
 * Function extract to val arg signed integer value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractInt32(char** args, int32& val) const
{
    if (!*args || !** args)
        return false;

    char* tail = *args;

    long valRaw = strtol(*args, &tail, 10);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    if (valRaw < std::numeric_limits<int32>::min() || valRaw > std::numeric_limits<int32>::max())
        return false;

    // value successfully extracted
    val = int32(valRaw);
    *args = tail;
    return true;
}

/**
 * Function extract to val arg optional signed integer value or use default value. Fail if extracted not signed integer.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptInt32(char** args, int32& val, int32 defVal) const
{
    if (!*args || !** args)
    {
        val = defVal;
        return true;
    }

    return ExtractInt32(args, val);
}

/**
 * Function extract to val arg unsigned integer value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @param base set used base for extracted value format (10 for decimal, 16 for hex, etc), 0 let auto select by system internal function
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractUInt32Base(char** args, uint32& val, uint32 base) const
{
    if (!*args || !** args)
        return false;

    char* tail = *args;

    unsigned long valRaw = strtoul(*args, &tail, base);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    if (valRaw > std::numeric_limits<uint32>::max())
        return false;

    // value successfully extracted
    val = uint32(valRaw);
    *args = tail;

    SkipWhiteSpaces(args);
    return true;
}

/**
 * Function extract to val arg optional unsigned integer value or use default value. Fail if extracted not unsigned integer.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptUInt32(char** args, uint32& val, uint32 defVal) const
{
    if (!*args || !** args)
    {
        val = defVal;
        return true;
    }

    return ExtractUInt32(args, val);
}

/**
 * Function extract to val arg float value or fail
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true if value extraction successful
 */
bool  ChatHandler::ExtractFloat(char** args, float& val)
{
    if (!*args || !** args)
        return false;

    char* tail = *args;

    double valRaw = strtod(*args, &tail);

    if (tail != *args && isWhiteSpace(*tail))
        *(tail++) = '\0';
    else if (tail && *tail)                                 // some not whitespace symbol
        return false;                                       // args not modified and can be re-parsed

    // value successfully extracted
    val = float(valRaw);
    *args = tail;

    SkipWhiteSpaces(args);
    return true;
}

/**
 * Function extract to val arg optional float value or use default value. Fail if extracted not float.
 *
 * @param args    variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val     return extracted value if function success, in fail case original value unmodified
 * @param defVal  default value used if no data for extraction in args
 * @return        true if value extraction successful
 */
bool  ChatHandler::ExtractOptFloat(char** args, float& val, float defVal)
{
    if (!*args || !** args)
    {
        val = defVal;
        return true;
    }

    return ExtractFloat(args, val);
}

/**
 * Function extract name-like string (from non-numeric or special symbol until whitespace)
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param lit  optional explicit literal requirement. function fail if literal is not starting substring of lit.
 *             Note: function in same way fail if no any literal or literal not fit in this case. Need additional check for select specific fail case
 * @return     name/number-like string without whitespaces, or nullptr if args empty or not appropriate content.
 */
char* ChatHandler::ExtractLiteralArg(char** args, char const* lit /*= nullptr*/)
{
    if (!*args || !** args)
        return nullptr;

    char* head = *args;

    // reject quoted string or link (|-started text)
    switch (head[0])
    {
        // reject quoted string
        case '[': case '\'': case '"':
            return nullptr;
        // reject link (|-started text)
        case '|':
            // client replace all | by || in raw text
            if (head[1] != '|')
                return nullptr;
            ++head;                                         // skip one |
            break;
        default: break;
    }

    if (lit)
    {
        int l = strlen(lit);

        int largs = 0;
        while (head[largs] && !isWhiteSpace(head[largs]))
            ++largs;

        if (largs < l)
            l = largs;

        int diff = strncmp(head, lit, l);

        if (diff != 0)
            return nullptr;

        if (head[l] && !isWhiteSpace(head[l]))
            return nullptr;

        char* arg = head;

        if (head[l])
        {
            head[l] = '\0';

            head += l + 1;

            *args = head;
        }
        else
            *args = head + l;

        SkipWhiteSpaces(args);
        return arg;
    }

    char* name = strtok(head, " ");

    char* tail = strtok(nullptr, "");

    *args = tail ? tail : (char*)"";                        // *args don't must be nullptr

    SkipWhiteSpaces(args);

    return name;
}

/**
 * Function extract quote-like string (any characters guarded by some special character, in our cases ['")
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     quote-like string, or nullptr if args empty or not appropriate content.
 */
char* ChatHandler::ExtractQuotedArg(char** args, bool asis /*= false*/)
{
    if (!*args || !** args)
        return nullptr;

    if (**args != '\'' &&**  args != '"' &&**  args != '[')
        return nullptr;

    char guard = (*args)[0];

    if (guard == '[')
        guard = ']';

    char* tail = (*args) + 1;                               // start scan after first quote symbol
    char* head = asis ? *args : tail;                       // start arg

    while (*tail && *tail != guard)
        ++tail;

    if (!*tail || (tail[1] && !isWhiteSpace(tail[1])))      // fail
        return nullptr;

    if (!tail[1])                                           // quote is last char in string
    {
        if (!asis)
            *tail = '\0';
    }
    else                                                    // quote isn't last char
    {
        if (asis)
            ++tail;

        *tail = '\0';
    }

    *args = tail + 1;

    SkipWhiteSpaces(args);

    return head;
}

/**
 * Function extract quote-like string or literal if quote not detected
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     quote/literal string, or nullptr if args empty or not appropriate content.
 */
char* ChatHandler::ExtractQuotedOrLiteralArg(char** args, bool asis /*= false*/)
{
    char* arg = ExtractQuotedArg(args, asis);
    if (!arg)
        arg = ExtractLiteralArg(args);
    return arg;
}

/**
 * Function extract on/off literals as boolean values
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param val  return extracted value if function success, in fail case original value unmodified
 * @return     true at success
 */
bool ChatHandler::ExtractOnOff(char** args, bool& value)
{
    char* arg = ExtractLiteralArg(args);
    if (!arg)
        return false;

    std::string lowercase(arg);
    std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), [](char c) { return std::tolower(c); });

    if (strncmp(lowercase.c_str(), "on", 3) == 0)
        value = true;
    else if (strncmp(lowercase.c_str(), "off", 4) == 0)
        value = false;
    else
        return false;

    return true;
}

/**
 * Function extract shift-link-like string (any characters guarded by | and |h|r with some additional internal structure check)
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *
 * @param linkTypes  optional nullptr-terminated array of link types, shift-link must fit one from link type from array if provided or extraction fail
 *
 * @param found_idx  if not nullptr then at return index in linkTypes that fit shift-link type, if extraction fail then non modified
 *
 * @param keyPair    if not nullptr then pointer to 2-elements array for return start and end pointer for found key
 *                   if extraction fail then non modified
 *
 * @param somethingPair then pointer to 2-elements array for return start and end pointer if found.
 *                   if not nullptr then shift-link must have data field, if extraction fail then non modified
 *
 * @return     shift-link-like string, or nullptr if args empty or not appropriate content.
 */
char* ChatHandler::ExtractLinkArg(char** args, char const* const* linkTypes /*= nullptr*/, int* foundIdx /*= nullptr*/, char** keyPair /*= nullptr*/, char** somethingPair /*= nullptr*/)
{
    if (!*args || !** args)
        return nullptr;

    // skip if not linked started or encoded single | (doubled by client)
    if ((*args)[0] != '|' || (*args)[1] == '|')
        return nullptr;

    // |color|Hlinktype:key:data...|h[name]|h|r

    char* head = *args;

    // [name] Shift-click form |color|linkType:key|h[name]|h|r
    // or
    // [name] Shift-click form |color|linkType:key:something1:...:somethingN|h[name]|h|r
    // or
    // [name] Shift-click form |linkType:key|h[name]|h|r

    // |color|Hlinktype:key:data...|h[name]|h|r

    char* tail = (*args) + 1;                               // skip |

    if (*tail != 'H')                                       // skip color part, some links can not have color part
    {
        while (*tail && *tail != '|')
            ++tail;

        if (!*tail)
            return nullptr;

        // |Hlinktype:key:data...|h[name]|h|r

        ++tail;                                             // skip |
    }

    // Hlinktype:key:data...|h[name]|h|r

    if (*tail != 'H')
        return nullptr;

    int linktype_idx = 0;

    if (linkTypes)                                          // check link type if provided
    {
        // check linktypes (its include H in name)
        for (; linkTypes[linktype_idx]; ++linktype_idx)
        {
            // exactly string with follow : or |
            int l = strlen(linkTypes[linktype_idx]);
            if (strncmp(tail, linkTypes[linktype_idx], l) == 0 &&
                    (tail[l] == ':' || tail[l] == '|'))
                break;
        }

        // is search fail?
        if (!linkTypes[linktype_idx])                       // nullptr terminator in last element
            return nullptr;

        tail += strlen(linkTypes[linktype_idx]);            // skip linktype string

        // :key:data...|h[name]|h|r

        if (*tail != ':')
            return nullptr;
    }
    else
    {
        while (*tail && *tail != ':')                       // skip linktype string
            ++tail;

        if (!*tail)
            return nullptr;
    }

    ++tail;

    // key:data...|h[name]|h|r
    char* keyStart = tail;                                  // remember key start for return

    while (*tail && *tail != '|' && *tail != ':')
        ++tail;

    if (!*tail)
        return nullptr;

    char* keyEnd = tail;                                    // remember key end for truncate

    // |h[name]|h|r or :something...|h[name]|h|r

    char* somethingStart = tail + 1;
    char* somethingEnd   = tail + 1;                        // will updated later if need

    if (*tail == ':' && somethingPair)                      // optional data extraction
    {
        // :something...|h[name]|h|r
        ++tail;

        // something|h[name]|h|r or something:something2...|h[name]|h|r

        while (*tail && *tail != '|' && *tail != ':')
            ++tail;

        if (!*tail)
            return nullptr;

        somethingEnd = tail;                                // remember data end for truncate
    }

    // |h[name]|h|r or :something2...|h[name]|h|r

    while (*tail && (*tail != '|' || *(tail + 1) != 'h'))   // skip ... part if exist
        ++tail;

    if (!*tail)
        return nullptr;

    // |h[name]|h|r

    tail += 2;                                              // skip |h

    // [name]|h|r
    if (!*tail || *tail != '[')
        return nullptr;

    while (*tail && (*tail != ']' || *(tail + 1) != '|'))   // skip name part
        ++tail;

    tail += 2;                                              // skip ]|

    // h|r
    if (!*tail || *tail != 'h'  || *(tail + 1) != '|')
        return nullptr;

    tail += 2;                                              // skip h|

    // r
    if (!*tail || *tail != 'r' || (*(tail + 1) && !isWhiteSpace(*(tail + 1))))
        return nullptr;

    ++tail;                                                 // skip r

    // success

    if (*tail)                                              // truncate all link string
        *(tail++) = '\0';

    if (foundIdx)
        *foundIdx = linktype_idx;

    if (keyPair)
    {
        keyPair[0] = keyStart;
        keyPair[1] = keyEnd;
    }

    if (somethingPair)
    {
        somethingPair[0] = somethingStart;
        somethingPair[1] = somethingEnd;
    }

    *args = tail;

    SkipWhiteSpaces(args);

    return head;
}

/**
 * Function extract name/number/quote/shift-link-like string
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 * @param asis control save quote string wrappers
 * @return     extracted arg string, or nullptr if args empty or not appropriate content.
 */
char* ChatHandler::ExtractArg(char** args, bool asis /*= false*/)
{
    if (!*args || !** args)
        return nullptr;

    char* arg = ExtractQuotedOrLiteralArg(args, asis);
    if (!arg)
        arg = ExtractLinkArg(args);

    return arg;
}

/**
 * Function extract name/quote/number/shift-link-like string, and return it if args have more non-whitespace data
 *
 * @param args variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *             if args have only single arg then args still pointing to this arg (unmodified pointer)
 * @return     extracted string, or nullptr if args empty or not appropriate content or have single arg totally.
 */
char* ChatHandler::ExtractOptNotLastArg(char** args)
{
    char* arg = ExtractArg(args, true);

    // have more data
    if (*args &&**  args)
        return arg;

    // optional name not found
    *args = arg ? arg : (char*)"";                          // *args don't must be nullptr

    return nullptr;
}

/**
 * Function extract data from shift-link "|color|LINKTYPE:RETURN:SOMETHING1|h[name]|h|r if linkType == LINKTYPE
 * It also extract literal/quote if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkType   shift-link must fit by link type to this arg value or extraction fail
 *
 * @param something1 if not nullptr then shift-link must have data field and it returned into this arg
 *                   if extraction fail then non modified
 *
 * @return           extracted key, or nullptr if args empty or not appropriate content or not fit to linkType.
 */
char* ChatHandler::ExtractKeyFromLink(char** text, char const* linkType, char** something1 /*= nullptr*/)
{
    char const* linkTypes[2];
    linkTypes[0] = linkType;
    linkTypes[1] = nullptr;

    int foundIdx;

    return ExtractKeyFromLink(text, linkTypes, &foundIdx, something1);
}

/**
 * Function extract data from shift-link "|color|LINKTYPE:RETURN:SOMETHING1|h[name]|h|r if LINKTYPE in linkTypes array
 * It also extract literal/quote if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkTypes  nullptr-terminated array of link types, shift-link must fit one from link type from array or extraction fail
 *
 * @param found_idx  if not nullptr then at return index in linkTypes that fit shift-link type, for non-link case return -1
 *                   if extraction fail then non modified
 *
 * @param something1 if not nullptr then shift-link must have data field and it returned into this arg
 *                   if extraction fail then non modified
 *
 * @return           extracted key, or nullptr if args empty or not appropriate content or not fit to linkType.
 */
char* ChatHandler::ExtractKeyFromLink(char** text, char const* const* linkTypes, int* found_idx, char** something1 /*= nullptr*/)
{
    // skip empty
    if (!*text || !** text)
        return nullptr;

    // return non link case
    char* arg = ExtractQuotedOrLiteralArg(text);
    if (arg)
    {
        if (found_idx)
            *found_idx = -1;                                // special index case

        return arg;
    }

    char* keyPair[2];
    char* somethingPair[2];

    arg = ExtractLinkArg(text, linkTypes, found_idx, keyPair, something1 ? somethingPair : nullptr);
    if (!arg)
        return nullptr;

    *keyPair[1] = '\0';                                     // truncate key string

    if (something1)
    {
        *somethingPair[1] = '\0';                           // truncate data string
        *something1 = somethingPair[0];
    }

    return keyPair[0];
}

/**
 * Function extract uint32 key from shift-link "|color|LINKTYPE:RETURN|h[name]|h|r if linkType == LINKTYPE
 * It also extract direct number if not shift-link in args
 *
 * @param args       variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *                   if args have sift link with linkType != LINKTYPE then args still pointing to this arg (unmodified pointer)
 *
 * @param linkType   shift-link must fit by link type to this arg value or extraction fail
 *
 * @param value      store result value at success return, not modified at fail
 *
 * @return           true if extraction succesful
 */
bool ChatHandler::ExtractUint32KeyFromLink(char** text, char const* linkType, uint32& value)
{
    char* arg = ExtractKeyFromLink(text, linkType);
    if (!arg)
        return false;

    return ExtractUInt32(&arg, value);
}

GameObject* ChatHandler::GetGameObjectWithGuid(uint32 lowguid, uint32 entry) const
{
    if (!m_session)
        return nullptr;

    Player* pl = m_session->GetPlayer();

    return pl->GetMap()->GetGameObject(ObjectGuid(HIGHGUID_GAMEOBJECT, entry, lowguid));
}

enum SpellLinkType
{
    SPELL_LINK_RAW     = -1,                                // non-link case
    SPELL_LINK_SPELL   = 0,
    SPELL_LINK_TALENT  = 1,
    SPELL_LINK_ENCHANT = 2,
};

static char const* const spellKeys[] =
{
    "Hspell",                                               // normal spell
    "Htalent",                                              // talent spell
    "Henchant",                                             // enchanting recipe spell
    nullptr
};

uint32 ChatHandler::ExtractSpellIdFromLink(char** text)
{
    // number or [name] Shift-click form |color|Henchant:recipe_spell_id|h[prof_name: recipe_name]|h|r
    // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r
    // number or [name] Shift-click form |color|Htalent:talent_id,rank|h[name]|h|r
    int type;
    char* param1_str = nullptr;
    char* idS = ExtractKeyFromLink(text, spellKeys, &type, &param1_str);
    if (!idS)
        return 0;

    uint32 id;
    if (!ExtractUInt32(&idS, id))
        return 0;

    switch (type)
    {
        case SPELL_LINK_RAW:
        case SPELL_LINK_SPELL:
        case SPELL_LINK_ENCHANT:
            return id;
        case SPELL_LINK_TALENT:
        {
            // talent
            TalentEntry const* talentEntry = sTalentStore.LookupEntry(id);
            if (!talentEntry)
                return 0;

            int32 rank;
            if (!ExtractInt32(&param1_str, rank))
                return 0;

            if (rank < 0)                                   // unlearned talent have in shift-link field -1 as rank
                rank = 0;

            return rank < MAX_TALENT_RANK ? talentEntry->RankID[rank] : 0;
        }
    }

    // unknown type?
    return 0;
}

GameTele const* ChatHandler::ExtractGameTeleFromLink(char** text)
{
    // id, or string, or [name] Shift-click form |color|Htele:id|h[name]|h|r
    char* cId = ExtractKeyFromLink(text, "Htele");
    if (!cId)
        return nullptr;

    // id case (explicit or from shift link)
    uint32 id;
    if (ExtractUInt32(&cId, id))
        return sObjectMgr.GetGameTele(id);
    return sObjectMgr.GetGameTele(cId);
}

enum GuidLinkType
{
    GUID_LINK_RAW        = -1,                              // non-link case
    GUID_LINK_PLAYER     = 0,
    GUID_LINK_CREATURE   = 1,
    GUID_LINK_GAMEOBJECT = 2,
};

static char const* const guidKeys[] =
{
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    nullptr
};

ObjectGuid ChatHandler::ExtractGuidFromLink(char** text)
{
    int type = 0;

    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hplayer:name|h[name]|h|r
    char* idS = ExtractKeyFromLink(text, guidKeys, &type);
    if (!idS)
        return ObjectGuid();

    switch (type)
    {
        case GUID_LINK_RAW:
        case GUID_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return ObjectGuid();

            if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
                return player->GetObjectGuid();

            return sObjectMgr.GetPlayerGuidByName(name);
        }
        case GUID_LINK_CREATURE:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return ObjectGuid();

            if (CreatureData const* data = sObjectMgr.GetCreatureData(lowguid))
                return data->GetObjectGuid(lowguid);
            return ObjectGuid();
        }
        case GUID_LINK_GAMEOBJECT:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return ObjectGuid();

            if (GameObjectData const* data = sObjectMgr.GetGOData(lowguid))
                return ObjectGuid(HIGHGUID_GAMEOBJECT, data->id, lowguid);
            return ObjectGuid();
        }
    }

    // unknown type?
    return ObjectGuid();
}

enum LocationLinkType
{
    LOCATION_LINK_RAW               = -1,                   // non-link case
    LOCATION_LINK_PLAYER            = 0,
    LOCATION_LINK_TELE              = 1,
    LOCATION_LINK_TAXINODE          = 2,
    LOCATION_LINK_CREATURE          = 3,
    LOCATION_LINK_GAMEOBJECT        = 4,
    LOCATION_LINK_CREATURE_ENTRY    = 5,
    LOCATION_LINK_GAMEOBJECT_ENTRY  = 6,
    LOCATION_LINK_AREATRIGGER       = 7,
    LOCATION_LINK_AREATRIGGER_TARGET = 8,
};

static char const* const locationKeys[] =
{
    "Htele",
    "Htaxinode",
    "Hplayer",
    "Hcreature",
    "Hgameobject",
    "Hcreature_entry",
    "Hgameobject_entry",
    "Hareatrigger",
    "Hareatrigger_target",
    nullptr
};

bool ChatHandler::ExtractLocationFromLink(char** text, uint32& mapid, float& x, float& y, float& z)
{
    int type = 0;

    // |color|Hplayer:name|h[name]|h|r
    // |color|Htele:id|h[name]|h|r
    // |color|Htaxinode:id|h[name]|h|r
    // |color|Hcreature:creature_guid|h[name]|h|r
    // |color|Hgameobject:go_guid|h[name]|h|r
    // |color|Hcreature_entry:creature_id|h[name]|h|r
    // |color|Hgameobject_entry:go_id|h[name]|h|r
    // |color|Hareatrigger:id|h[name]|h|r
    // |color|Hareatrigger_target:id|h[name]|h|r
    char* idS = ExtractKeyFromLink(text, locationKeys, &type);
    if (!idS)
        return false;

    switch (type)
    {
        case LOCATION_LINK_RAW:
        case LOCATION_LINK_PLAYER:
        {
            std::string name = idS;
            if (!normalizePlayerName(name))
                return false;

            if (Player* player = sObjectMgr.GetPlayer(name.c_str()))
            {
                mapid = player->GetMapId();
                x = player->GetPositionX();
                y = player->GetPositionY();
                z = player->GetPositionZ();
                return true;
            }

            if (ObjectGuid guid = sObjectMgr.GetPlayerGuidByName(name))
            {
                // to point where player stay (if loaded)
                float o;
                bool in_flight;
                return Player::LoadPositionFromDB(guid, mapid, x, y, z, o, in_flight);
            }

            return false;
        }
        case LOCATION_LINK_TELE:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            GameTele const* tele = sObjectMgr.GetGameTele(id);
            if (!tele)
                return false;
            mapid = tele->mapId;
            x = tele->position_x;
            y = tele->position_y;
            z = tele->position_z;
            return true;
        }
        case LOCATION_LINK_TAXINODE:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(id);
            if (!node)
                return false;
            mapid = node->map_id;
            x = node->x;
            y = node->y;
            z = node->z;
            return true;
        }
        case LOCATION_LINK_CREATURE:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return false;

            if (CreatureData const* data = sObjectMgr.GetCreatureData(lowguid))
            {
                mapid = data->mapid;
                x = data->posX;
                y = data->posY;
                z = data->posZ;
                return true;
            }
            return false;
        }
        case LOCATION_LINK_GAMEOBJECT:
        {
            uint32 lowguid;
            if (!ExtractUInt32(&idS, lowguid))
                return false;

            if (GameObjectData const* data = sObjectMgr.GetGOData(lowguid))
            {
                mapid = data->mapid;
                x = data->posX;
                y = data->posY;
                z = data->posZ;
                return true;
            }
            return false;
        }
        case LOCATION_LINK_CREATURE_ENTRY:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (ObjectMgr::GetCreatureTemplate(id))
            {
                FindCreatureData worker(id, m_session ? m_session->GetPlayer() : nullptr);

                sObjectMgr.DoCreatureData(worker);

                if (CreatureDataPair const* dataPair = worker.GetResult())
                {
                    mapid = dataPair->second.mapid;
                    x = dataPair->second.posX;
                    y = dataPair->second.posY;
                    z = dataPair->second.posZ;
                    return true;
                }
                return false;
            }
            return false;
        }
        case LOCATION_LINK_GAMEOBJECT_ENTRY:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (ObjectMgr::GetGameObjectInfo(id))
            {
                FindGOData worker(id, m_session ? m_session->GetPlayer() : nullptr);

                sObjectMgr.DoGOData(worker);

                if (GameObjectDataPair const* dataPair = worker.GetResult())
                {
                    mapid = dataPair->second.mapid;
                    x = dataPair->second.posX;
                    y = dataPair->second.posY;
                    z = dataPair->second.posZ;
                    return true;
                }
                return false;
            }
            return false;
        }
        case LOCATION_LINK_AREATRIGGER:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(id);
            if (!atEntry)
            {
                PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, id);
                SetSentErrorMessage(true);
                return false;
            }

            mapid = atEntry->mapid;
            x = atEntry->x;
            y = atEntry->y;
            z = atEntry->z;
            return true;
        }
        case LOCATION_LINK_AREATRIGGER_TARGET:
        {
            uint32 id;
            if (!ExtractUInt32(&idS, id))
                return false;

            if (!sAreaTriggerStore.LookupEntry(id))
            {
                PSendSysMessage(LANG_COMMAND_GOAREATRNOTFOUND, id);
                SetSentErrorMessage(true);
                return false;
            }

            AreaTrigger const* at = sObjectMgr.GetAreaTrigger(id);
            if (!at)
            {
                PSendSysMessage(LANG_AREATRIGER_NOT_HAS_TARGET, id);
                SetSentErrorMessage(true);
                return false;
            }

            mapid = at->target_mapId;
            x = at->target_X;
            y = at->target_Y;
            z = at->target_Z;
            return true;
        }
    }

    // unknown type?
    return false;
}

std::string ChatHandler::ExtractPlayerNameFromLink(char** text)
{
    // |color|Hplayer:name|h[name]|h|r
    char* name_str = ExtractKeyFromLink(text, "Hplayer");
    if (!name_str)
        return "";

    std::string name = name_str;
    if (!normalizePlayerName(name))
        return "";

    return name;
}

/**
 * Function extract at least one from request player data (pointer/guid/name) from args name/shift-link or selected player if no args
 *
 * @param args        variable pointer to non parsed args string, updated at function call to new position (with skipped white spaces)
 *
 * @param player      optional arg   One from 3 optional args must be provided at least (or more).
 * @param player_guid optional arg   For function success only one from provided args need get result
 * @param player_name optional arg   But if early arg get value then all later args will have its (if requested)
 *                                   if player_guid requested and not found then name also will not found
 *                                   So at success can be returned 2 cases: (player/guid/name) or (guid/name)
 *
 * @return           true if extraction successful
 */
bool ChatHandler::ExtractPlayerTarget(char** args, Player** player /*= nullptr*/, ObjectGuid* player_guid /*= nullptr*/, std::string* player_name /*= nullptr*/)
{
    if (*args &&**  args)
    {
        std::string name = ExtractPlayerNameFromLink(args);
        if (name.empty())
        {
            SendSysMessage(LANG_PLAYER_NOT_FOUND);
            SetSentErrorMessage(true);
            return false;
        }

        Player* pl = sObjectMgr.GetPlayer(name.c_str());

        // if allowed player pointer
        if (player)
            *player = pl;

        // if need guid value from DB (in name case for check player existence)
        ObjectGuid guid = !pl && (player_guid || player_name) ? sObjectMgr.GetPlayerGuidByName(name) : ObjectGuid();

        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetObjectGuid() : guid;

        if (player_name)
            *player_name = pl || guid ? name : "";
    }
    else
    {
        Player* pl = getSelectedPlayer();
        // if allowed player pointer
        if (player)
            *player = pl;
        // if allowed player guid (if no then only online players allowed)
        if (player_guid)
            *player_guid = pl ? pl->GetObjectGuid() : ObjectGuid();

        if (player_name)
            *player_name = pl ? pl->GetName() : "";
    }

    // some from req. data must be provided (note: name is empty if player not exist)
    if ((!player || !*player) && (!player_guid || !*player_guid) && (!player_name || player_name->empty()))
    {
        SendSysMessage(LANG_PLAYER_NOT_FOUND);
        SetSentErrorMessage(true);
        return false;
    }

    return true;
}

uint32 ChatHandler::ExtractAccountId(char** args, std::string* accountName /*= nullptr*/, Player** targetIfNullArg /*= nullptr*/)
{
    uint32 account_id = 0;

    ///- Get the account name from the command line
    char* account_str = ExtractLiteralArg(args);

    if (!account_str)
    {
        if (!targetIfNullArg)
            return 0;

        /// only target player different from self allowed (if targetPlayer!=nullptr then not console)
        Player* targetPlayer = getSelectedPlayer();
        if (!targetPlayer)
            return 0;

        account_id = targetPlayer->GetSession()->GetAccountId();

        if (accountName)
            sAccountMgr.GetName(account_id, *accountName);

        if (targetIfNullArg)
            *targetIfNullArg = targetPlayer;

        return account_id;
    }

    std::string account_name;

    if (ExtractUInt32(&account_str, account_id))
    {
        if (!sAccountMgr.GetName(account_id, account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_str);
            SetSentErrorMessage(true);
            return 0;
        }
    }
    else
    {
        account_name = account_str;
        if (!AccountMgr::normalizeString(account_name))
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
            SetSentErrorMessage(true);
            return 0;
        }

        account_id = sAccountMgr.GetId(account_name);
        if (!account_id)
        {
            PSendSysMessage(LANG_ACCOUNT_NOT_EXIST, account_name.c_str());
            SetSentErrorMessage(true);
            return 0;
        }
    }

    if (accountName)
        *accountName = account_name;

    if (targetIfNullArg)
        *targetIfNullArg = nullptr;

    return account_id;
}

struct RaceMaskName
{
    char const* literal;
    uint32 raceMask;
};

static RaceMaskName const raceMaskNames[] =
{
    // races
    { "human", (1 << (RACE_HUMAN - 1))   },
    { "orc", (1 << (RACE_ORC - 1))     },
    { "dwarf", (1 << (RACE_DWARF - 1))   },
    { "nightelf", (1 << (RACE_NIGHTELF - 1))},
    { "undead", (1 << (RACE_UNDEAD - 1))  },
    { "tauren", (1 << (RACE_TAUREN - 1))  },
    { "gnome", (1 << (RACE_GNOME - 1))   },
    { "troll", (1 << (RACE_TROLL - 1))   },

    // masks
    { "alliance", RACEMASK_ALLIANCE },
    { "horde",    RACEMASK_HORDE },
    { "all", RACEMASK_ALL_PLAYABLE },

    // terminator
    { nullptr, 0 }
};

bool ChatHandler::ExtractRaceMask(char** text, uint32& raceMask, char const** maskName /*=nullptr*/)
{
    if (ExtractUInt32(text, raceMask))
    {
        if (maskName)
            *maskName = "custom mask";
    }
    else
    {
        for (RaceMaskName const* itr = raceMaskNames; itr->literal; ++itr)
        {
            if (ExtractLiteralArg(text, itr->literal))
            {
                raceMask = itr->raceMask;

                if (maskName)
                    *maskName = itr->literal;
                break;
            }
        }

        if (!raceMask)
            return false;
    }

    return true;
}

std::string ChatHandler::GetNameLink(Player* chr) const
{
    return playerLink(chr->GetName());
}

bool ChatHandler::needReportToTarget(Player* chr) const
{
    Player* pl = m_session->GetPlayer();
    return pl != chr && pl->IsVisibleGloballyFor(chr);
}

LocaleConstant ChatHandler::GetSessionDbcLocale() const
{
    return m_session->GetSessionDbcLocale();
}

int ChatHandler::GetSessionDbLocaleIndex() const
{
    return m_session->GetSessionDbLocaleIndex();
}

const char* CliHandler::GetMangosString(int32 entry) const
{
    return sObjectMgr.GetMangosStringForDbcLocale(entry);
}

uint32 CliHandler::GetAccountId() const
{
    return m_accountId;
}

AccountTypes CliHandler::GetAccessLevel() const
{
    return m_loginAccessLevel;
}

bool CliHandler::isAvailable(ChatCommand const& cmd) const
{
    // skip non-console commands in console case
    if (!cmd.AllowConsole)
        return false;

    // normal case
    return GetAccessLevel() >= (AccountTypes)cmd.SecurityLevel;
}

void CliHandler::SendSysMessage(const char* str)
{
    m_print(str);
    m_print("\r\n");
}

std::string CliHandler::GetNameLink() const
{
    return GetMangosString(LANG_CONSOLE_COMMAND);
}

bool CliHandler::needReportToTarget(Player* /*chr*/) const
{
    return true;
}

LocaleConstant CliHandler::GetSessionDbcLocale() const
{
    return sWorld.GetDefaultDbcLocale();
}

int CliHandler::GetSessionDbLocaleIndex() const
{
    return sObjectMgr.GetDbc2StorageLocaleIndex();
}

// Check/ Output if a NPC or GO (by guid) is part of a pool or game event
template <typename T>
void ChatHandler::ShowNpcOrGoSpawnInformation(uint32 guid)
{
    if (uint16 pool_id = sPoolMgr.IsPartOfAPool<T>(guid))
    {
        uint16 top_pool_id = sPoolMgr.IsPartOfTopPool<Pool>(pool_id);
        if (!top_pool_id || top_pool_id == pool_id)
            PSendSysMessage(LANG_NPC_GO_INFO_POOL, uint32(pool_id));
        else
            PSendSysMessage(LANG_NPC_GO_INFO_TOP_POOL, uint32(pool_id), uint32(top_pool_id));

        if (int16 event_id = sGameEventMgr.GetGameEventId<Pool>(top_pool_id))
        {
            GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
            GameEventData const& eventData = events[std::abs(event_id)];

            if (event_id > 0)
                PSendSysMessage(LANG_NPC_GO_INFO_POOL_GAME_EVENT_S, uint32(top_pool_id), uint32(std::abs(event_id)), eventData.description.c_str());
            else
                PSendSysMessage(LANG_NPC_GO_INFO_POOL_GAME_EVENT_D, uint32(top_pool_id), uint32(std::abs(event_id)), eventData.description.c_str());
        }
    }
    else if (int16 event_id = sGameEventMgr.GetGameEventId<T>(guid))
    {
        GameEventMgr::GameEventDataMap const& events = sGameEventMgr.GetEventMap();
        GameEventData const& eventData = events[std::abs(event_id)];

        if (event_id > 0)
            PSendSysMessage(LANG_NPC_GO_INFO_GAME_EVENT_S, uint32(std::abs(event_id)), eventData.description.c_str());
        else
            PSendSysMessage(LANG_NPC_GO_INFO_GAME_EVENT_D, uint32(std::abs(event_id)), eventData.description.c_str());
    }
}

// Prepare ShortString for a NPC or GO (by guid) with pool or game event IDs
template <typename T>
std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation(uint32 guid) const
{
    std::string str;
    if (uint16 pool_id = sPoolMgr.IsPartOfAPool<T>(guid))
    {
        uint16 top_pool_id = sPoolMgr.IsPartOfTopPool<T>(guid);
        if (int16 event_id = sGameEventMgr.GetGameEventId<Pool>(top_pool_id))
        {
            char buffer[100];
            const char* format = GetMangosString(LANG_NPC_GO_INFO_POOL_EVENT_STRING);
            sprintf(buffer, format, pool_id, event_id);
            str = buffer;
        }
        else
        {
            char buffer[100];
            const char* format = GetMangosString(LANG_NPC_GO_INFO_POOL_STRING);
            sprintf(buffer, format, pool_id);
            str = buffer;
        }
    }
    else if (int16 event_id = sGameEventMgr.GetGameEventId<T>(guid))
    {
        char buffer[100];
        const char* format = GetMangosString(LANG_NPC_GO_INFO_EVENT_STRING);
        sprintf(buffer, format, event_id);
        str = buffer;
    }

    return str;
}

void ChatHandler::LogCommand(char const* fullcmd) const
{
    // chat case
    if (m_session)
    {
        Player* p = m_session->GetPlayer();
        ObjectGuid sel_guid = p->GetSelectionGuid();
        sLog.outCommand(GetAccountId(), "Command: %s [Player: %s (Account: %u) X: %f Y: %f Z: %f Map: %u Selected: %s]",
                        fullcmd, p->GetName(), GetAccountId(), p->GetPositionX(), p->GetPositionY(), p->GetPositionZ(), p->GetMapId(),
                        sel_guid.GetString().c_str());
    }
    else                                        // 0 account -> console
    {
        sLog.outCommand(GetAccountId(), "Command: %s [Account: %u from %s]",
                        fullcmd, GetAccountId(), GetAccountId() ? "RA-connection" : "Console");
    }
}

void ChatHandler::BuildChatPacket(WorldPacket& data, ChatMsg msgtype, char const* message, Language language /*= LANG_UNIVERSAL*/, ChatTagFlags chatTag /*= CHAT_TAG_NONE*/,
                                  ObjectGuid const& senderGuid /*= ObjectGuid()*/, char const* senderName /*= nullptr*/,
                                  ObjectGuid const& targetGuid /*= ObjectGuid()*/, char const* targetName /*= nullptr*/,
                                  char const* channelName /*= nullptr*/, uint8 playerRank /*= 0*/)
{
    data.Initialize(SMSG_MESSAGECHAT);
    data << uint8(msgtype);
    data << uint32(language);

    switch (msgtype)
    {
        case CHAT_MSG_MONSTER_WHISPER:
        case CHAT_MSG_RAID_BOSS_WHISPER:
        case CHAT_MSG_RAID_BOSS_EMOTE:
        case CHAT_MSG_MONSTER_EMOTE:
            MANGOS_ASSERT(senderName);
            data << uint32(strlen(senderName) + 1);
            data << senderName;
            data << ObjectGuid(targetGuid);
            break;

        case CHAT_MSG_SAY:
        case CHAT_MSG_PARTY:
        case CHAT_MSG_YELL:
            data << ObjectGuid(senderGuid);
            data << ObjectGuid(senderGuid);
            break;

        case CHAT_MSG_MONSTER_SAY:
        case CHAT_MSG_MONSTER_YELL:
            MANGOS_ASSERT(senderName);
            data << ObjectGuid(senderGuid);
            data << uint32(strlen(senderName) + 1);
            data << senderName;
            data << ObjectGuid(targetGuid);
            break;

        case CHAT_MSG_CHANNEL:
            MANGOS_ASSERT(channelName);
            data << channelName;
            data << uint32(playerRank);
            data << ObjectGuid(senderGuid);
            break;

        default:
            data << ObjectGuid(senderGuid);
            break;
    }

    MANGOS_ASSERT(message);
    data << uint32(strlen(message) + 1);
    data << message;
    data << uint8(chatTag);
}

// Instantiate template for helper function
template void ChatHandler::ShowNpcOrGoSpawnInformation<Creature>(uint32 guid);
template void ChatHandler::ShowNpcOrGoSpawnInformation<GameObject>(uint32 guid);

template std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation<Creature>(uint32 guid) const;
template std::string ChatHandler::PrepareStringNpcOrGoSpawnInformation<GameObject>(uint32 guid) const;
