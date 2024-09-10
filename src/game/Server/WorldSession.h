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

/// \addtogroup u2w
/// @{
/// \file

#ifndef __WORLDSESSION_H
#define __WORLDSESSION_H

#include "Common.h"
#include "Globals/Locales.h"
#include "Globals/SharedDefines.h"
#include "Entities/ObjectGuid.h"
#include "AuctionHouse/AuctionHouseMgr.h"
#include "Entities/Item.h"
#include "Server/WorldSocket.h"
#include "Multithreading/Messager.h"
#include "BattleGround/BattleGroundDefines.h"

#include <atomic>
#include <map>
#include <deque>
#include <mutex>
#include <memory>

struct ItemPrototype;
struct AuctionEntry;
struct AuctionHouseEntry;
struct TradeStatusInfo;

class ObjectGuid;
class Creature;
class Item;
class Object;
class Player;
class Unit;
class WorldPacket;
class QueryResult;
class LoginQueryHolder;
class CharacterHandler;
class GMTicket;
class MovementInfo;
class WorldSession;
class SessionAnticheatInterface;

struct OpcodeHandler;

enum AccountDataType
{
    GLOBAL_CONFIG_CACHE             = 0,                    // 0x01 g
    PER_CHARACTER_CONFIG_CACHE      = 1,                    // 0x02 p
    GLOBAL_BINDINGS_CACHE           = 2,                    // 0x04 g
    PER_CHARACTER_BINDINGS_CACHE    = 3,                    // 0x08 p
    GLOBAL_MACROS_CACHE             = 4,                    // 0x10 g
    PER_CHARACTER_MACROS_CACHE      = 5,                    // 0x20 p
    PER_CHARACTER_LAYOUT_CACHE      = 6,                    // 0x40 p
    PER_CHARACTER_CHAT_CACHE        = 7,                    // 0x80 p
    NUM_ACCOUNT_DATA_TYPES          = 8
};

#define GLOBAL_CACHE_MASK           0x15
#define PER_CHARACTER_CACHE_MASK    0xEA

struct AccountData
{
    AccountData() : Time(0), Data("") {}

    time_t Time;
    std::string Data;
};

enum ClientOSType
{
    CLIENT_OS_UNKNOWN,
    CLIENT_OS_WIN,
    CLIENT_OS_MAC
};

enum ClientPlatformType
{
    CLIENT_PLATFORM_UNKNOWN,
    CLIENT_PLATFORM_X86,
    CLIENT_PLATFORM_PPC
};

enum PartyOperation
{
    PARTY_OP_INVITE = 0,
    PARTY_OP_LEAVE = 2,
};

enum PartyResult
{
    ERR_PARTY_RESULT_OK                 = 0,
    ERR_BAD_PLAYER_NAME_S               = 1,
    ERR_TARGET_NOT_IN_GROUP_S           = 2,
    ERR_GROUP_FULL                      = 3,
    ERR_ALREADY_IN_GROUP_S              = 4,
    ERR_NOT_IN_GROUP                    = 5,
    ERR_NOT_LEADER                      = 6,
    ERR_PLAYER_WRONG_FACTION            = 7,
    ERR_IGNORING_YOU_S                  = 8
};

enum TutorialDataState
{
    TUTORIALDATA_UNCHANGED = 0,
    TUTORIALDATA_CHANGED   = 1,
    TUTORIALDATA_NEW       = 2
};

enum WorldSessionState
{
    WORLD_SESSION_STATE_CREATED        = 0,
    WORLD_SESSION_STATE_CHAR_SELECTION = 1,
    WORLD_SESSION_STATE_READY          = 2,
    WORLD_SESSION_STATE_OFFLINE        = 3
};

enum TransferAbortReason
{
    TRANSFER_ABORT_NONE                         = 0x00,
    TRANSFER_ABORT_MAX_PLAYERS                  = 0x01,     // Transfer Aborted: instance is full
    TRANSFER_ABORT_NOT_FOUND                    = 0x02,     // Transfer Aborted: instance not found
    TRANSFER_ABORT_TOO_MANY_INSTANCES           = 0x03,     // You have entered too many instances recently.
    TRANSFER_ABORT_ZONE_IN_COMBAT               = 0x05,     // Unable to zone in while an encounter is in progress.
};

struct CharacterNameQueryResponse
{
    ObjectGuid          guid;                   // pc's guid
    std::string         name;                   // pc's name
    std::string         realm;                  // realm name (xrealm battlegrounds)
    uint32              race        = 0;        // pc's race
    uint32              gender      = 0;        // pc's gender
    uint32              classid     = 0;        // pc's class
};

enum AccountFlags
{
    ACCOUNT_FLAG_SHOW_ANTICHEAT = 0x01,
    ACCOUNT_FLAG_SILENCED       = 0x02,
    ACCOUNT_FLAG_SHOW_ANTISPAM  = 0x04,
    ACCOUNT_FLAG_HIDDEN         = 0x08,
};

// class to deal with packet processing
// allows to determine if next packet is safe to be processed
class PacketFilter
{
    public:
        explicit PacketFilter(WorldSession* pSession) : m_pSession(pSession) {}
        virtual ~PacketFilter() {}

        virtual bool Process(WorldPacket const& /*packet*/) const { return true; }
        virtual bool ProcessLogout() const { return true; }

    protected:
        WorldSession* const m_pSession;
};
// process only thread-safe packets in Map::Update()
class MapSessionFilter : public PacketFilter
{
    public:
        explicit MapSessionFilter(WorldSession* pSession) : PacketFilter(pSession) {}
        ~MapSessionFilter() {}

        virtual bool Process(WorldPacket const& packet) const override;
        // in Map::Update() we do not process player logout!
        virtual bool ProcessLogout() const override { return false; }
};

// class used to filer only thread-unsafe packets from queue
// in order to update only be used in World::UpdateSessions()
class WorldSessionFilter : public PacketFilter
{
    public:
        explicit WorldSessionFilter(WorldSession* pSession) : PacketFilter(pSession) {}
        ~WorldSessionFilter() {}

        virtual bool Process(WorldPacket const& packet) const override;
};

/// Player session in the World
class WorldSession
{
        friend class CharacterHandler;

    public:
        WorldSession(uint32 id, WorldSocket* sock, AccountTypes sec, time_t mute_time, LocaleConstant locale, std::string accountName, uint32 accountFlags);
        ~WorldSession();

        // Set this session have no attached socket but keep it alive for short period of time to permit a possible reconnection
        void SetOffline();
        void SetOnline();
        void SetInCharSelection();

        // Request set offline, close socket and put session offline
        bool RequestNewSocket(WorldSocket* socket);
        bool IsOffline() const { return m_sessionState == WORLD_SESSION_STATE_OFFLINE; }
        WorldSessionState GetState() const { return m_sessionState; }

        bool PlayerLoading() const { return m_playerLoading; }
        bool PlayerLogout() const { return m_playerLogout; }
        bool PlayerLogoutWithSave() const { return m_playerLogout && m_playerSave; }

        void SizeError(WorldPacket const& packet, uint32 size) const;

        void SendPacket(WorldPacket const& packet, bool forcedSend = false) const;
        void SendExpectedSpamRecords();
        void SendMotd(Player* currChar);
        void SendOfflineNameQueryResponses();
        void SendNotification(const char* format, ...) const ATTR_PRINTF(2, 3);
        void SendNotification(int32 string_id, ...) const;
        void SendPetNameInvalid() const;
        void SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res) const;
        void SendAreaTriggerMessage(const char* Text, ...) const ATTR_PRINTF(2, 3);
        void SendTransferAborted(TransferAbortReason reason) const;
        void SendQueryTimeResponse() const;

        bool IsInitialZoneUpdated() { return m_initialZoneUpdated; }

        AccountTypes GetSecurity() const { return _security; }
        uint32 GetAccountFlags() const { return m_accountFlags; }
        bool HasAccountFlag(uint32 flags) const { return (m_accountFlags & flags) != 0; }
        void AddAccountFlag(uint32 flags) { m_accountFlags |= flags; }
        void RemoveAccountFlag(uint32 flags) { m_accountFlags &= ~flags; }
        uint32 GetAccountId() const { return _accountId; }
        std::string const& GetAccountName() const { return m_accountName; }
        Player* GetPlayer() const { return _player; }
        char const* GetPlayerName() const;
        void SetSecurity(AccountTypes security) { _security = security; }
#if defined(BUILD_DEPRECATED_PLAYERBOT) || defined(ENABLE_PLAYERBOTS)
        // Players connected without socket are bot
        const std::string GetRemoteAddress() const { return m_socket ? m_socket->GetRemoteAddress() : "disconnected/bot"; }
#else
        const std::string GetRemoteAddress() const { return m_socket ? m_socket->GetRemoteAddress() : "disconnected"; }
#endif
        const std::string& GetLocalAddress() const { return m_localAddress; }

        void SetPlayer(Player* plr, uint32 playerGuid);

        void InitializeAnticheat(const BigNumber& K);
        void AssignAnticheat(std::unique_ptr<SessionAnticheatInterface>&& anticheat);
        SessionAnticheatInterface* GetAnticheat() const { return m_anticheat.get(); }

#if defined(BUILD_DEPRECATED_PLAYERBOT) || defined(ENABLE_PLAYERBOTS)
        void SetNoAnticheat();
#endif

        /// Session in auth.queue currently
        void SetInQueue(bool state) { m_inQueue = state; }

        /// Is the user engaged in a log out process?
        bool isLogingOut() const { return _logoutTime || m_playerLogout; }

        /// Engage the logout process for the user
        void LogoutRequest(time_t requestTime, bool saveToDB = true, bool kickSession = false)
        {
            _logoutTime = requestTime;
            m_playerSave = saveToDB;
            m_kickSession = kickSession;
        }

        void AfkStateChange(bool state);

        /// Is logout cooldown expired?
        bool ShouldLogOut(time_t currTime) const
        {
            return (_logoutTime > 0 && currTime >= _logoutTime + 20);
        }

        bool ShouldDisconnect(time_t currTime)
        {
            return (_logoutTime > 0 && currTime >= _logoutTime + 60);
        }

        bool ShouldAfkDisconnect(time_t currTime) const
        {
            return (m_afkTime > 0 && currTime >= m_afkTime + 15 * MINUTE);
        }

        void LogoutPlayer();
        void KickPlayer(bool save = false, bool inPlace = false); // inplace variable needed for shutdown

        void QueuePacket(std::unique_ptr<WorldPacket> new_packet);

        void DeleteMovementPackets();

        bool Update(uint32 diff);
        void UpdateMap(uint32 diff);

        /// Handle the authentication waiting queue (to be completed)
        void SendAuthWaitQue(uint32 position) const;

        void SendNameQueryResponse(CharacterNameQueryResponse& response) const;
        void SendNameQueryResponseFromDB(ObjectGuid guid) const;
        static void SendNameQueryResponseFromDBCallBack(QueryResult* result, uint32 accountId);

        void SendTrainerList(ObjectGuid guid) const;

        void SendListInventory(ObjectGuid guid) const;
        bool CheckBanker(ObjectGuid guid) const;
        void SendShowBank(ObjectGuid guid) const;
        bool CheckMailBox(ObjectGuid guid) const;
        void SendTabardVendorActivate(ObjectGuid guid) const;
        void SendSpiritResurrect() const;
        void SendBindPoint(Creature* npc) const;

        void SendGMTicketResult(uint32 opcode, uint32 result) const;
        void SendGMTicket(const GMTicket& ticket, time_t now = time(nullptr)) const;

        void SendBattleGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId) const;

        void SendTradeStatus(const TradeStatusInfo& status) const;
        void SendUpdateTrade(bool trader_state = true) const;
        void SendCancelTrade(TradeStatus status) const;

        void SendPetitionQueryOpcode(ObjectGuid petitionguid) const;

        // pet
        void SendPetNameQuery(ObjectGuid guid, uint32 petnumber) const;
        void SendStablePet(ObjectGuid guid) const;
        void SendStableResult(uint8 res) const;
        bool CheckStableMaster(ObjectGuid guid) const;

        // Account Data
        AccountData* GetAccountData(AccountDataType type) { return &m_accountData[type]; }
        void SetAccountData(AccountDataType type, time_t time_, const std::string& data);
        void SendAccountDataTimes();
        void LoadGlobalAccountData();
        void LoadAccountData(std::unique_ptr<QueryResult> queryResult, uint32 mask);
        void LoadTutorialsData();
        void SendTutorialsData();
        void SaveTutorialsData();
        uint32 GetTutorialInt(uint32 intId)
        {
            return m_Tutorials[intId];
        }

        void SetTutorialInt(uint32 intId, uint32 value)
        {
            if (m_Tutorials[intId] != value)
            {
                m_Tutorials[intId] = value;
                if (m_tutorialState == TUTORIALDATA_UNCHANGED)
                    m_tutorialState = TUTORIALDATA_CHANGED;
            }
        }

        // auction
        void SendAuctionHello(Unit* unit) const;
        void SendAuctionCommandResult(AuctionEntry* auc, AuctionAction Action, AuctionError ErrorCode, InventoryResult invError = EQUIP_ERR_OK) const;
        void SendAuctionBidderNotification(AuctionEntry* auction, bool won) const;
        void SendAuctionOwnerNotification(AuctionEntry* auction, bool sold) const;
        void SendAuctionRemovedNotification(AuctionEntry* auction) const;
        static void SendAuctionOutbiddedMail(AuctionEntry* auction);
        void SendAuctionCancelledToBidderMail(AuctionEntry* auction) const;
        AuctionHouseEntry const* GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid) const;

        // Item Enchantment
        void SendItemEnchantTimeUpdate(ObjectGuid playerGuid, ObjectGuid itemGuid, uint32 slot, uint32 duration) const;

        // Taxi
        void SendTaxiStatus(ObjectGuid guid) const;
        void SendTaxiMenu(Creature* unit) const;
        bool SendLearnNewTaxiNode(Creature* unit) const;
        void SendActivateTaxiReply(ActivateTaxiReply reply) const;

        // Guild Team
        void SendGuildCommandResult(uint32 typecmd, const std::string& str, uint32 cmdresult) const;
        void SendPetitionShowList(ObjectGuid guid) const;
        void SendSaveGuildEmblem(uint32 msg) const;
        void SendBattleGroundJoinError(uint8 err) const;

        static void BuildPartyMemberStatsChangedPacket(Player* player, WorldPacket& data);

        // Account mute time
        time_t m_muteTime;
        // Ticket squelch timer: prevent ticket spam
        ShortTimeTracker m_ticketSquelchTimer;

        // Locales
        LocaleConstant GetSessionDbcLocale() const { return m_sessionDbcLocale; }
        int GetSessionDbLocaleIndex() const { return m_sessionDbLocaleIndex; }
        const char* GetMangosString(int32 entry) const;

        uint32 GetLatency() const { return m_latency; }
        void SetLatency(uint32 latency) { m_latency = latency; }
        void ResetClientTimeDelay() { m_clientTimeDelay = 0; }
        uint32 getDialogStatus(const Player* pPlayer, const Object* questgiver, uint32 defstatus) const;
        ClientOSType GetOS() const { return m_clientOS; }
        void SetOS(ClientOSType os) { m_clientOS = os; }
        ClientPlatformType GetPlatform() const { return m_clientPlatform; }
        void SetPlatform(ClientPlatformType platform) { m_clientPlatform = platform; }
        uint32 GetGameBuild() const { return m_gameBuild; }
        void SetGameBuild(uint32 version) { m_gameBuild = version; }
        uint32 GetAccountMaxLevel() const { return m_accountMaxLevel; }
        void SetAccountMaxLevel(uint32 level) { m_accountMaxLevel = level; }
        uint32 GetOrderCounter() const { return m_orderCounter; }
        void IncrementOrderCounter() { ++m_orderCounter; }

        // Thread safety
        uint32 GetCurrentPlayerLevel() { return m_currentPlayerLevel; }
        void SetCurrentPlayerLevel(uint32 level) { m_currentPlayerLevel = level; }

        // Misc
        void SendKnockBack(Unit* who, float angle, float horizontalSpeed, float verticalSpeed);
        void SendPlaySpellVisual(ObjectGuid guid, uint32 spellArtKit) const;
#ifdef ENABLE_PLAYERBOTS
        void SendTeleportToObservers(float x, float y, float z, float orientation);
#endif

        void SendAuthOk() const;
        void SendAuthQueued() const;
        // opcodes handlers
        void Handle_NULL(WorldPacket& recvPacket);          // not used
        void Handle_EarlyProccess(WorldPacket& recvPacket); // just mark packets processed in WorldSocket::OnRead
        void Handle_ServerSide(WorldPacket& recvPacket);    // sever side only, can't be accepted from client
        void Handle_Deprecated(WorldPacket& recvPacket);    // never used anymore by client

        void HandleCharEnumOpcode(WorldPacket& recvPacket);
        void HandleCharDeleteOpcode(WorldPacket& recvPacket);
        void HandleCharCreateOpcode(WorldPacket& recvPacket);
        void HandlePlayerLoginOpcode(WorldPacket& recvPacket);
        void HandleCharEnum(QueryResult* result);
        void HandlePlayerLogin(LoginQueryHolder* holder);
        void HandlePlayerReconnect();

        // played time
        void HandlePlayedTime(WorldPacket& recvPacket);

        // new
        void HandleMoveRootAck(WorldPacket& recvPacket);

        // new inspect
        void HandleInspectOpcode(WorldPacket& recvPacket);

        // new party stats
        void HandleInspectHonorStatsOpcode(WorldPacket& recvPacket);

        void HandleMoveFlagChangeOpcode(WorldPacket& recvPacket);

        void HandleMountSpecialAnimOpcode(WorldPacket& recvdata);

        // character view
        void HandleShowingHelmOpcode(WorldPacket& recv_data);
        void HandleShowingCloakOpcode(WorldPacket& recv_data);

        // repair
        void HandleRepairItemOpcode(WorldPacket& recvPacket);

        // Knockback
        void HandleMoveKnockBackAck(WorldPacket& recvPacket);

        void HandleMoveTeleportAckOpcode(WorldPacket& recvPacket);
        void HandleForceSpeedChangeAckOpcodes(WorldPacket& recv_data);

        void HandleRepopRequestOpcode(WorldPacket& recvPacket);
        void HandleAutostoreLootItemOpcode(WorldPacket& recvPacket);
        void HandleLootMoneyOpcode(WorldPacket& recvPacket);
        void HandleLootOpcode(WorldPacket& recvPacket);
        void HandleLootReleaseOpcode(WorldPacket& recvPacket);
        void HandleLootMasterGiveOpcode(WorldPacket& recvPacket);
        void HandleWhoOpcode(WorldPacket& recvPacket);
        void HandleLogoutRequestOpcode(WorldPacket& recvPacket);
        void HandlePlayerLogoutOpcode(WorldPacket& recvPacket);
        void HandleLogoutCancelOpcode(WorldPacket& recvPacket);

        void HandleGMTicketSystemStatusOpcode(WorldPacket& recvPacket);
        void HandleGMTicketGetTicketOpcode(WorldPacket& recvPacket);
        void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketUpdateTextOpcode(WorldPacket& recvPacket);
        void HandleGMTicketDeleteTicketOpcode(WorldPacket& recvPacket);
        void HandleGMSurveySubmitOpcode(WorldPacket& recvPacket);

        void HandleTogglePvP(WorldPacket& recvPacket);

        void HandleZoneUpdateOpcode(WorldPacket& recvPacket);
        void HandleSetTargetOpcode(WorldPacket& recvPacket);
        void HandleSetSelectionOpcode(WorldPacket& recvPacket);
        void HandleStandStateChangeOpcode(WorldPacket& recvPacket);
        void HandleEmoteOpcode(WorldPacket& recvPacket);
        void HandleFriendListOpcode(WorldPacket& recvPacket);
        void HandleAddFriendOpcode(WorldPacket& recvPacket);
        static void HandleAddFriendOpcodeCallBack(QueryResult* result, uint32 accountId);
        void HandleDelFriendOpcode(WorldPacket& recvPacket);
        void HandleAddIgnoreOpcode(WorldPacket& recvPacket);
        static void HandleAddIgnoreOpcodeCallBack(QueryResult* result, uint32 accountId);
        void HandleDelIgnoreOpcode(WorldPacket& recvPacket);
        void HandleBugOpcode(WorldPacket& recvPacket);
        void HandleSetAmmoOpcode(WorldPacket& recvPacket);
        void HandleItemNameQueryOpcode(WorldPacket& recvPacket);

        void HandleAreaTriggerOpcode(WorldPacket& recvPacket);

        void HandleSetFactionAtWarOpcode(WorldPacket& recv_data);
        void HandleSetWatchedFactionOpcode(WorldPacket& recv_data);
        void HandleSetFactionInactiveOpcode(WorldPacket& recv_data);

        void HandleUpdateAccountData(WorldPacket& recvPacket);
        void HandleRequestAccountData(WorldPacket& recvPacket);
        void HandleSetActionButtonOpcode(WorldPacket& recvPacket);

        void HandleGameObjectUseOpcode(WorldPacket& recPacket);
        void HandleMeetingStoneJoinOpcode(WorldPacket& recPacket);
        void HandleMeetingStoneLeaveOpcode(WorldPacket& recPacket);
        void HandleMeetingStoneInfoOpcode(WorldPacket& recPacket);

        void HandleNameQueryOpcode(WorldPacket& recvPacket);

        void HandleQueryTimeOpcode(WorldPacket& recvPacket);

        void HandleCreatureQueryOpcode(WorldPacket& recvPacket);

        void HandleGameObjectQueryOpcode(WorldPacket& recvPacket);

        void HandleMoveWorldportAckOpcode(WorldPacket& recvPacket);
        void HandleMoveWorldportAckOpcode();                // for server-side calls

        void HandleMovementOpcodes(WorldPacket& recvPacket);
        void HandleSetActiveMoverOpcode(WorldPacket& recv_data);
        void HandleMoveNotActiveMoverOpcode(WorldPacket& recv_data);
        void HandleMoveTimeSkippedOpcode(WorldPacket& recv_data);

        bool ProcessMovementInfo(MovementInfo& movementInfo, Unit* mover, Player* plMover, WorldPacket& recv_data);

        void HandleRequestRaidInfoOpcode(WorldPacket& recv_data);

        void HandleGroupInviteOpcode(WorldPacket& recvPacket);
        void HandleGroupAcceptOpcode(WorldPacket& recvPacket);
        void HandleGroupDeclineOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        void HandleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        void HandleGroupDisbandOpcode(WorldPacket& recvPacket);
        void HandleLootMethodOpcode(WorldPacket& recvPacket);
        void HandleLootRoll(WorldPacket& recv_data);
        void HandleRequestPartyMemberStatsOpcode(WorldPacket& recv_data);
        void HandleRaidTargetUpdateOpcode(WorldPacket& recv_data);
        void HandleRaidReadyCheckOpcode(WorldPacket& recv_data);
        void HandleGroupRaidConvertOpcode(WorldPacket& recv_data);
        void HandleGroupChangeSubGroupOpcode(WorldPacket& recv_data);
        void HandleGroupSwapSubGroupOpcode(WorldPacket& recv_data);
        void HandleGroupAssistantLeaderOpcode(WorldPacket& recv_data);

        void HandlePetitionBuyOpcode(WorldPacket& recv_data);
        void HandlePetitionShowSignOpcode(WorldPacket& recv_data);
        void HandlePetitionQueryOpcode(WorldPacket& recv_data);
        void HandlePetitionRenameOpcode(WorldPacket& recv_data);
        void HandlePetitionSignOpcode(WorldPacket& recv_data);
        void HandlePetitionDeclineOpcode(WorldPacket& recv_data);
        void HandleOfferPetitionOpcode(WorldPacket& recv_data);
        void HandleTurnInPetitionOpcode(WorldPacket& recv_data);

        void HandleGuildQueryOpcode(WorldPacket& recvPacket);
        void HandleGuildCreateOpcode(WorldPacket& recvPacket);
        void HandleGuildInviteOpcode(WorldPacket& recvPacket);
        void HandleGuildRemoveOpcode(WorldPacket& recvPacket);
        void HandleGuildAcceptOpcode(WorldPacket& recvPacket);
        void HandleGuildDeclineOpcode(WorldPacket& recvPacket);
        void HandleGuildInfoOpcode(WorldPacket& recvPacket);
        void HandleGuildRosterOpcode(WorldPacket& recvPacket);
        void HandleGuildPromoteOpcode(WorldPacket& recvPacket);
        void HandleGuildDemoteOpcode(WorldPacket& recvPacket);
        void HandleGuildLeaveOpcode(WorldPacket& recvPacket);
        void HandleGuildDisbandOpcode(WorldPacket& recvPacket);
        void HandleGuildLeaderOpcode(WorldPacket& recvPacket);
        void HandleGuildMOTDOpcode(WorldPacket& recvPacket);
        void HandleGuildSetPublicNoteOpcode(WorldPacket& recvPacket);
        void HandleGuildSetOfficerNoteOpcode(WorldPacket& recvPacket);
        void HandleGuildRankOpcode(WorldPacket& recvPacket);
        void HandleGuildAddRankOpcode(WorldPacket& recvPacket);
        void HandleGuildDelRankOpcode(WorldPacket& recvPacket);
        void HandleGuildChangeInfoTextOpcode(WorldPacket& recvPacket);
        void HandleSaveGuildEmblemOpcode(WorldPacket& recvPacket);

        void HandleTaxiNodeStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleTaxiQueryAvailableNodes(WorldPacket& recvPacket);
        void HandleActivateTaxiOpcode(WorldPacket& recvPacket);
        void HandleActivateTaxiExpressOpcode(WorldPacket& recvPacket);
        void HandleMoveSplineDoneOpcode(WorldPacket& recvPacket);

        void HandleTabardVendorActivateOpcode(WorldPacket& recvPacket);
        void HandleBankerActivateOpcode(WorldPacket& recvPacket);
        void HandleBuyBankSlotOpcode(WorldPacket& recvPacket);
        void HandleTrainerListOpcode(WorldPacket& recvPacket);
        void HandleTrainerBuySpellOpcode(WorldPacket& recvPacket);

        void HandlePetitionShowListOpcode(WorldPacket& recvPacket);
        void HandleGossipHelloOpcode(WorldPacket& recvPacket);
        void HandleGossipSelectOptionOpcode(WorldPacket& recvPacket);
        void HandleSpiritHealerActivateOpcode(WorldPacket& recvPacket);
        void HandleNpcTextQueryOpcode(WorldPacket& recvPacket);
        void HandleBinderActivateOpcode(WorldPacket& recvPacket);
        void HandleListStabledPetsOpcode(WorldPacket& recvPacket);
        void HandleStablePet(WorldPacket& recvPacket);
        void HandleUnstablePet(WorldPacket& recvPacket);
        void HandleBuyStableSlot(WorldPacket& recvPacket);
        void HandleStableRevivePet(WorldPacket& recvPacket);
        void HandleStableSwapPet(WorldPacket& recvPacket);

        void HandleDuelAcceptedOpcode(WorldPacket& recvPacket);
        void HandleDuelCancelledOpcode(WorldPacket& recvPacket);

        void HandleAcceptTradeOpcode(WorldPacket& recvPacket);
        void HandleBeginTradeOpcode(WorldPacket& recvPacket);
        void HandleBusyTradeOpcode(WorldPacket& recvPacket);
        void HandleCancelTradeOpcode(WorldPacket& recvPacket);
        void HandleClearTradeItemOpcode(WorldPacket& recvPacket);
        void HandleIgnoreTradeOpcode(WorldPacket& recvPacket);
        void HandleInitiateTradeOpcode(WorldPacket& recvPacket);
        void HandleSetTradeGoldOpcode(WorldPacket& recvPacket);
        void HandleSetTradeItemOpcode(WorldPacket& recvPacket);
        void HandleUnacceptTradeOpcode(WorldPacket& recvPacket);

        void HandleAuctionHelloOpcode(WorldPacket& recvPacket);
        void HandleAuctionListItems(WorldPacket& recv_data);
        void HandleAuctionListBidderItems(WorldPacket& recv_data);
        void HandleAuctionSellItem(WorldPacket& recv_data);

        void HandleAuctionRemoveItem(WorldPacket& recv_data);
        void HandleAuctionListOwnerItems(WorldPacket& recv_data);
        void HandleAuctionPlaceBid(WorldPacket& recv_data);

        void HandleGetMailList(WorldPacket& recv_data);
        void HandleSendMail(WorldPacket& recv_data);
        void HandleMailTakeMoney(WorldPacket& recv_data);
        void HandleMailTakeItem(WorldPacket& recv_data);
        void HandleMailMarkAsRead(WorldPacket& recv_data);
        void HandleMailReturnToSender(WorldPacket& recv_data);
        void HandleMailDelete(WorldPacket& recv_data);
        void HandleItemTextQuery(WorldPacket& recv_data);
        void HandleMailCreateTextItem(WorldPacket& recv_data);
        void HandleQueryNextMailTime(WorldPacket& recv_data);
        void HandleCancelChanneling(WorldPacket& recv_data);

        void HandleSplitItemOpcode(WorldPacket& recvPacket);
        void HandleSwapInvItemOpcode(WorldPacket& recvPacket);
        void HandleDestroyItemOpcode(WorldPacket& recvPacket);
        void HandleAutoEquipItemOpcode(WorldPacket& recvPacket);
        void HandleItemQuerySingleOpcode(WorldPacket& recvPacket);
        void HandleSellItemOpcode(WorldPacket& recvPacket);
        void HandleBuyItemInSlotOpcode(WorldPacket& recvPacket);
        void HandleBuyItemOpcode(WorldPacket& recvPacket);
        void HandleListInventoryOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBagItemOpcode(WorldPacket& recvPacket);
        void HandleReadItemOpcode(WorldPacket& recvPacket);
        void HandleAutoEquipItemSlotOpcode(WorldPacket& recvPacket);
        void HandleSwapItem(WorldPacket& recvPacket);
        void HandleBuybackItem(WorldPacket& recvPacket);
        void HandleAutoBankItemOpcode(WorldPacket& recvPacket);
        void HandleAutoStoreBankItemOpcode(WorldPacket& recvPacket);
        void HandleWrapItemOpcode(WorldPacket& recvPacket);

        void HandleAttackSwingOpcode(WorldPacket& recvPacket);
        void HandleAttackStopOpcode(WorldPacket& recvPacket);
        void HandleSetSheathedOpcode(WorldPacket& recvPacket);

        void HandleUseItemOpcode(WorldPacket& recvPacket);
        void HandleOpenItemOpcode(WorldPacket& recvPacket);
        void HandleCastSpellOpcode(WorldPacket& recvPacket);
        void HandleCancelCastOpcode(WorldPacket& recvPacket);
        void HandleCancelAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelGrowthAuraOpcode(WorldPacket& recvPacket);
        void HandleCancelAutoRepeatSpellOpcode(WorldPacket& recvPacket);

        void HandleLearnTalentOpcode(WorldPacket& recvPacket);
        void HandleTalentWipeConfirmOpcode(WorldPacket& recvPacket);
        void HandleUnlearnSkillOpcode(WorldPacket& recvPacket);

        void HandleQuestgiverStatusQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverHelloOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverAcceptQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverQueryQuestOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverChooseRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverRequestRewardOpcode(WorldPacket& recvPacket);
        void HandleQuestQueryOpcode(WorldPacket& recvPacket);
        void HandleQuestgiverCancel(WorldPacket& recv_data);
        void HandleQuestLogSwapQuest(WorldPacket& recv_data);
        void HandleQuestLogRemoveQuest(WorldPacket& recv_data);
        void HandleQuestConfirmAccept(WorldPacket& recv_data);
        void HandleQuestgiverCompleteQuest(WorldPacket& recv_data);
        bool CanInteractWithQuestGiver(ObjectGuid guid, char const* descr) const;

        void HandleQuestgiverQuestAutoLaunch(WorldPacket& recvPacket);
        void HandlePushQuestToParty(WorldPacket& recvPacket);
        void HandleQuestPushResult(WorldPacket& recvPacket);

        bool CheckChatMessage(std::string&, bool addon = false);
        void SendPlayerNotFoundNotice(const std::string& name) const;
        void SendWrongFactionNotice() const;
        void SendChatRestrictedNotice() const;
        void HandleMessagechatOpcode(WorldPacket& recvPacket);
        void HandleTextEmoteOpcode(WorldPacket& recvPacket);
        void HandleChatIgnoredOpcode(WorldPacket& recvPacket);

        void HandleReclaimCorpseOpcode(WorldPacket& recvPacket);
        void HandleCorpseQueryOpcode(WorldPacket& recvPacket);
        void HandleResurrectResponseOpcode(WorldPacket& recvPacket);
        void HandleSummonResponseOpcode(WorldPacket& recv_data);

        bool CheckChatChannelNameAndPassword(std::string& name, std::string& pass);
        void HandleJoinChannelOpcode(WorldPacket& recvPacket);
        void HandleLeaveChannelOpcode(WorldPacket& recvPacket);
        void HandleChannelListOpcode(WorldPacket& recvPacket);
        void HandleChannelPasswordOpcode(WorldPacket& recvPacket);
        void HandleChannelSetOwnerOpcode(WorldPacket& recvPacket);
        void HandleChannelOwnerOpcode(WorldPacket& recvPacket);
        void HandleChannelModeratorOpcode(WorldPacket& recvPacket);
        void HandleChannelUnmoderatorOpcode(WorldPacket& recvPacket);
        void HandleChannelMuteOpcode(WorldPacket& recvPacket);
        void HandleChannelUnmuteOpcode(WorldPacket& recvPacket);
        void HandleChannelInviteOpcode(WorldPacket& recvPacket);
        void HandleChannelKickOpcode(WorldPacket& recvPacket);
        void HandleChannelBanOpcode(WorldPacket& recvPacket);
        void HandleChannelUnbanOpcode(WorldPacket& recvPacket);
        void HandleChannelAnnouncementsOpcode(WorldPacket& recvPacket);
        void HandleChannelModerateOpcode(WorldPacket& recvPacket);

        void HandleCompleteCinematic(WorldPacket& recvPacket);
        void HandleNextCinematicCamera(WorldPacket& recvPacket);

        void HandlePageQuerySkippedOpcode(WorldPacket& recvPacket);
        void HandlePageTextQueryOpcode(WorldPacket& recvPacket);

        void HandleTutorialFlagOpcode(WorldPacket& recv_data);
        void HandleTutorialClearOpcode(WorldPacket& recv_data);
        void HandleTutorialResetOpcode(WorldPacket& recv_data);

        // Pet
        void HandlePetAction(WorldPacket& recv_data);
        void HandlePetStopAttack(WorldPacket& recv_data);
        void HandlePetNameQueryOpcode(WorldPacket& recv_data);
        void HandlePetSetAction(WorldPacket& recv_data);
        void HandlePetAbandon(WorldPacket& recv_data);
        void HandlePetRename(WorldPacket& recv_data);
        void HandlePetCancelAuraOpcode(WorldPacket& recvPacket);
        void HandlePetUnlearnOpcode(WorldPacket& recvPacket);
        void HandlePetSpellAutocastOpcode(WorldPacket& recvPacket);
        void HandlePetCastSpellOpcode(WorldPacket& recvPacket);

        void HandleSetActionBarTogglesOpcode(WorldPacket& recv_data);

        void HandleCharRenameOpcode(WorldPacket& recv_data);
        static void HandleChangePlayerNameOpcodeCallBack(QueryResult* result, uint32 accountId, std::string newname);

        // BattleGround
        void HandleBattlemasterHelloOpcode(WorldPacket& recv_data);
        void HandleBattlemasterJoinOpcode(WorldPacket& recv_data);
        void HandleBattleGroundPlayerPositionsOpcode(WorldPacket& recv_data);
        void HandlePVPLogDataOpcode(WorldPacket& recv_data);
        void HandleBattlefieldStatusOpcode(WorldPacket& recv_data);
        void HandleBattlefieldPortOpcode(WorldPacket& recv_data);
        void HandleBattlefieldListOpcode(WorldPacket& recv_data);
        void HandleLeaveBattlefieldOpcode(WorldPacket& recv_data);

        void HandleWorldTeleportOpcode(WorldPacket& recv_data);
        void HandleMinimapPingOpcode(WorldPacket& recv_data);
        void HandleRandomRollOpcode(WorldPacket& recv_data);
        void HandleFarSightOpcode(WorldPacket& recv_data);
        void HandleWhoisOpcode(WorldPacket& recv_data);
        void HandleResetInstancesOpcode(WorldPacket& recv_data);

        void HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data);
        void HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data);
        void HandleSelfResOpcode(WorldPacket& recv_data);
        void HandleRequestPetInfoOpcode(WorldPacket& recv_data);

        // Meetingstone
        void SendMeetingstoneFailed(uint8 status);
        void SendMeetingstoneSetqueue(uint32 areaid, uint8 status);

#define MOVEMENT_PACKET_TIME_DELAY 0

        // Warden
        void HandleWardenDataOpcode(WorldPacket& recv_data);

        // Movement
        void SynchronizeMovement(MovementInfo& movementInfo);

        std::deque<uint32> GetOutOpcodeHistory();
        std::deque<uint32> GetIncOpcodeHistory();

#ifdef ENABLE_PLAYERBOTS
        void HandleBotPackets();
#endif

        Messager<WorldSession>& GetMessager() { return m_messager; }

        void SetPacketLogging(bool state);

    private:
        // private trade methods
        void moveItems(Item* myItems[], Item* hisItems[]);
        bool VerifyMovementInfo(MovementInfo const& movementInfo, Unit* mover, bool unroot) const;
        void HandleMoverRelocation(MovementInfo& movementInfo);

        void ExecuteOpcode(OpcodeHandler const& opHandle, WorldPacket& packet);

        // logging helper
        void LogUnexpectedOpcode(WorldPacket const& packet, const char* reason) const;
        void LogUnprocessedTail(WorldPacket const& packet) const;

        void ProcessByteBufferException(WorldPacket const& packet);

        uint32 m_GUIDLow;                                   // set logged or recently logout player (while m_playerRecentlyLogout set)
        Player* _player;
        std::shared_ptr<WorldSocket> m_socket;              // socket pointer is owned by the network thread which created it
        std::shared_ptr<WorldSocket> m_requestSocket;       // a new socket for this session is requested (double connection)
        std::string m_localAddress;
        WorldSessionState m_sessionState;                   // this session state

        AccountTypes _security;
        uint32 _accountId;
        std::string m_accountName;
        uint32 m_accountFlags;

        // anticheat
        ClientOSType m_clientOS;
        ClientPlatformType m_clientPlatform;
        uint32 m_gameBuild;
        uint32 m_accountMaxLevel;
        uint32 m_orderCounter;
        uint32 m_lastAnticheatUpdate;
        std::unique_ptr<SessionAnticheatInterface> m_anticheat;

        time_t _logoutTime;                                 // when logout will be processed after a logout request
        time_t m_kickTime;
        time_t m_afkTime;
        bool m_playerSave;                                  // should we have to save the player after logout request
        bool m_inQueue;                                     // session wait in auth.queue
        bool m_playerLoading;                               // code processed in LoginPlayer
        bool m_kickSession;

        // True when the player is in the process of logging out (WorldSession::LogoutPlayer is currently executing)
        bool m_playerLogout;
        bool m_playerRecentlyLogout;
        LocaleConstant m_sessionDbcLocale;
        int m_sessionDbLocaleIndex;
        std::atomic<uint32> m_latency;
        AccountData m_accountData[NUM_ACCOUNT_DATA_TYPES];
        uint32 m_clientTimeDelay;
        uint32 m_Tutorials[8];
        TutorialDataState m_tutorialState;

        std::set<ObjectGuid> m_offlineNameQueries; // for name queries made when not logged in (character selection screen)
        std::deque<CharacterNameQueryResponse> m_offlineNameResponses; // for responses to name queries made when not logged in

        bool m_initialZoneUpdated = false;

        // Thread safety mechanisms
        std::mutex m_recvQueueLock;
        std::mutex m_recvQueueMapLock;
        std::deque<std::unique_ptr<WorldPacket>> m_recvQueue;
        std::deque<std::unique_ptr<WorldPacket>> m_recvQueueMap;

        Messager<WorldSession> m_messager;

        std::atomic<uint32> m_currentPlayerLevel;
};
#endif
/// @}
