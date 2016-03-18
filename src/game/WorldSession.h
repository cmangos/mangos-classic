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
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "AuctionHouseMgr.h"
#include "Item.h"
#include "WorldSocket.h"

#include <deque>
#include <mutex>

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

struct OpcodeHandler;

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

// class to deal with packet processing
// allows to determine if next packet is safe to be processed
class PacketFilter
{
    public:
        explicit PacketFilter(WorldSession* pSession) : m_pSession(pSession) {}
        virtual ~PacketFilter() {}

        virtual bool Process(WorldPacket* /*packet*/) { return true; }
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

        virtual bool Process(WorldPacket* packet) override;
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

        virtual bool Process(WorldPacket* packet) override;
};

/// Player session in the World
class MANGOS_DLL_SPEC WorldSession
{
        friend class CharacterHandler;

    public:
        WorldSession(uint32 id, WorldSocket* sock, AccountTypes sec, time_t mute_time, LocaleConstant locale);
        ~WorldSession();

        bool PlayerLoading() const { return m_playerLoading; }
        bool PlayerLogout() const { return m_playerLogout; }
        bool PlayerLogoutWithSave() const { return m_playerLogout && m_playerSave; }

        // marks this session as finalized in the socket which owns it.  this lets
        // the socket handling code know that the session can be safely deleted
        void Finalize() { m_Socket->FinalizeSession(); }

        void SizeError(WorldPacket const& packet, uint32 size) const;

        void SendPacket(WorldPacket const* packet);
        void SendNotification(const char* format, ...) ATTR_PRINTF(2, 3);
        void SendNotification(int32 string_id, ...);
        void SendPetNameInvalid(uint32 error, const std::string& name);
        void SendPartyResult(PartyOperation operation, const std::string& member, PartyResult res);
        void SendAreaTriggerMessage(const char* Text, ...) ATTR_PRINTF(2, 3);
        void SendTransferAborted(uint32 mapid, uint8 reason, uint8 arg = 0);
        void SendQueryTimeResponse();

        AccountTypes GetSecurity() const { return _security; }
        uint32 GetAccountId() const { return _accountId; }
        Player* GetPlayer() const { return _player; }
        char const* GetPlayerName() const;
        void SetSecurity(AccountTypes security) { _security = security; }
        const std::string &GetRemoteAddress() const { return m_Socket->GetRemoteAddress(); }
        void SetPlayer(Player* plr) { _player = plr; }

        /// Session in auth.queue currently
        void SetInQueue(bool state) { m_inQueue = state; }

        /// Is the user engaged in a log out process?
        bool isLogingOut() const { return _logoutTime || m_playerLogout; }

        /// Engage the logout process for the user
        void LogoutRequest(time_t requestTime)
        {
            _logoutTime = requestTime;
        }

        /// Is logout cooldown expired?
        bool ShouldLogOut(time_t currTime) const
        {
            return (_logoutTime > 0 && currTime >= _logoutTime + 20);
        }

        void LogoutPlayer(bool Save);
        void KickPlayer();

        void QueuePacket(WorldPacket* new_packet);

        bool Update(PacketFilter& updater);

        /// Handle the authentication waiting queue (to be completed)
        void SendAuthWaitQue(uint32 position);

        void SendNameQueryOpcode(Player* p);
        void SendNameQueryOpcodeFromDB(ObjectGuid guid);
        static void SendNameQueryOpcodeFromDBCallBack(QueryResult* result, uint32 accountId);

        void SendTrainerList(ObjectGuid guid);
        void SendTrainerList(ObjectGuid guid, const std::string& strTitle);

        void SendListInventory(ObjectGuid guid);
        bool CheckBanker(ObjectGuid guid);
        void SendShowBank(ObjectGuid guid);
        bool CheckMailBox(ObjectGuid guid);
        void SendTabardVendorActivate(ObjectGuid guid);
        void SendSpiritResurrect();
        void SendBindPoint(Creature* npc);
        void SendGMTicketGetTicket(uint32 status, GMTicket* ticket = nullptr);

        void SendAttackStop(Unit const* enemy);

        void SendBattlegGroundList(ObjectGuid guid, BattleGroundTypeId bgTypeId);

        void SendTradeStatus(const TradeStatusInfo& status);
        void SendUpdateTrade(bool trader_state = true);
        void SendCancelTrade();

        void SendPetitionQueryOpcode(ObjectGuid petitionguid);

        // pet
        void SendPetNameQuery(ObjectGuid guid, uint32 petnumber);
        void SendStablePet(ObjectGuid guid);
        void SendStableResult(uint8 res);
        bool CheckStableMaster(ObjectGuid guid);

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
        void SendAuctionHello(Unit* unit);
        void SendAuctionCommandResult(AuctionEntry* auc, AuctionAction Action, AuctionError ErrorCode, InventoryResult invError = EQUIP_ERR_OK);
        void SendAuctionBidderNotification(AuctionEntry* auction, bool won);
        void SendAuctionOwnerNotification(AuctionEntry* auction, bool sold);
        void SendAuctionRemovedNotification(AuctionEntry* auction);
        static void SendAuctionOutbiddedMail(AuctionEntry* auction);
        void SendAuctionCancelledToBidderMail(AuctionEntry* auction);
        AuctionHouseEntry const* GetCheckedAuctionHouseForAuctioneer(ObjectGuid guid);

        // Item Enchantment
        void SendEnchantmentLog(ObjectGuid targetGuid, ObjectGuid casterGuid, uint32 itemId, uint32 spellId);
        void SendItemEnchantTimeUpdate(ObjectGuid playerGuid, ObjectGuid itemGuid, uint32 slot, uint32 duration);

        // Taxi
        void SendTaxiStatus(ObjectGuid guid);
        void SendTaxiMenu(Creature* unit);
        void SendDoFlight(uint32 mountDisplayId, uint32 path, uint32 pathNode = 0);
        bool SendLearnNewTaxiNode(Creature* unit);
        void SendActivateTaxiReply(ActivateTaxiReply reply);

        // Guild Team
        void SendGuildCommandResult(uint32 typecmd, const std::string& str, uint32 cmdresult);
        void SendPetitionShowList(ObjectGuid guid);
        void SendSaveGuildEmblem(uint32 msg);
        void SendBattleGroundJoinError(uint8 err);

        void BuildPartyMemberStatsChangedPacket(Player* player, WorldPacket* data);

        // Account mute time
        time_t m_muteTime;

        // Locales
        LocaleConstant GetSessionDbcLocale() const { return m_sessionDbcLocale; }
        int GetSessionDbLocaleIndex() const { return m_sessionDbLocaleIndex; }
        const char* GetMangosString(int32 entry) const;

        uint32 GetLatency() const { return m_latency; }
        void SetLatency(uint32 latency) { m_latency = latency; }
        void ResetClientTimeDelay() { m_clientTimeDelay = 0; }
        uint32 getDialogStatus(Player* pPlayer, Object* questgiver, uint32 defstatus);

        // Misc
        void SendKnockBack(float angle, float horizontalSpeed, float verticalSpeed);
        void SendPlaySpellVisual(ObjectGuid guid, uint32 spellArtKit);

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

        // played time
        void HandlePlayedTime(WorldPacket& recvPacket);

        // new
        void HandleMoveUnRootAck(WorldPacket& recvPacket);
        void HandleMoveRootAck(WorldPacket& recvPacket);

        // new inspect
        void HandleInspectOpcode(WorldPacket& recvPacket);

        // new party stats
        void HandleInspectHonorStatsOpcode(WorldPacket& recvPacket);

        void HandleMoveWaterWalkAck(WorldPacket& recvPacket);
        void HandleFeatherFallAck(WorldPacket& recv_data);

        void HandleMoveHoverAck(WorldPacket& recv_data);

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
        void HandleGMTicketGetTicketOpcode(WorldPacket& recvPacket);
        void HandleGMTicketCreateOpcode(WorldPacket& recvPacket);
        void HandleGMTicketSystemStatusOpcode(WorldPacket& recvPacket);

        void HandleGMTicketDeleteTicketOpcode(WorldPacket& recvPacket);
        void HandleGMTicketUpdateTextOpcode(WorldPacket& recvPacket);

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

        void HandleRequestRaidInfoOpcode(WorldPacket& recv_data);

        void HandleGroupInviteOpcode(WorldPacket& recvPacket);
        void HandleGroupAcceptOpcode(WorldPacket& recvPacket);
        void HandleGroupDeclineOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteOpcode(WorldPacket& recvPacket);
        void HandleGroupUninviteGuidOpcode(WorldPacket& recvPacket);
        void HandleGroupSetLeaderOpcode(WorldPacket& recvPacket);
        void HandleGroupDisbandOpcode(WorldPacket& recvPacket);
        void HandleOptOutOfLootOpcode(WorldPacket& recv_data);
        void HandleLootMethodOpcode(WorldPacket& recvPacket);
        void HandleLootRoll(WorldPacket& recv_data);
        void HandleRequestPartyMemberStatsOpcode(WorldPacket& recv_data);
        void HandleRaidTargetUpdateOpcode(WorldPacket& recv_data);
        void HandleRaidReadyCheckOpcode(WorldPacket& recv_data);
        void HandleRaidReadyCheckFinishedOpcode(WorldPacket& recv_data);
        void HandleGroupRaidConvertOpcode(WorldPacket& recv_data);
        void HandleGroupChangeSubGroupOpcode(WorldPacket& recv_data);
        void HandleGroupAssistantLeaderOpcode(WorldPacket& recv_data);
        void HandlePartyAssignmentOpcode(WorldPacket& recv_data);

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
        void HandleGuildEventLogQueryOpcode(WorldPacket& recvPacket);
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
        void HandleQuestgiverStatusMultipleQuery(WorldPacket& recvPacket);
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
        bool CanInteractWithQuestGiver(ObjectGuid guid, char const* descr);

        void HandleQuestgiverQuestAutoLaunch(WorldPacket& recvPacket);
        void HandlePushQuestToParty(WorldPacket& recvPacket);
        void HandleQuestPushResult(WorldPacket& recvPacket);

        bool processChatmessageFurtherAfterSecurityChecks(std::string&, uint32);
        void SendPlayerNotFoundNotice(const std::string& name);
        void SendWrongFactionNotice();
        void SendChatRestrictedNotice();
        void HandleMessagechatOpcode(WorldPacket& recvPacket);
        void HandleTextEmoteOpcode(WorldPacket& recvPacket);
        void HandleChatIgnoredOpcode(WorldPacket& recvPacket);

        void HandleReclaimCorpseOpcode(WorldPacket& recvPacket);
        void HandleCorpseQueryOpcode(WorldPacket& recvPacket);
        void HandleResurrectResponseOpcode(WorldPacket& recvPacket);
        void HandleSummonResponseOpcode(WorldPacket& recv_data);

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
        void HandleChannelDisplayListQueryOpcode(WorldPacket& recvPacket);
        void HandleGetChannelMemberCountOpcode(WorldPacket& recvPacket);
        void HandleSetChannelWatchOpcode(WorldPacket& recvPacket);

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

        void HandleTotemDestroyed(WorldPacket& recv_data);

        // BattleGround
        void HandleBattlemasterHelloOpcode(WorldPacket& recv_data);
        void HandleBattlemasterJoinOpcode(WorldPacket& recv_data);
        void HandleBattleGroundPlayerPositionsOpcode(WorldPacket& recv_data);
        void HandlePVPLogDataOpcode(WorldPacket& recv_data);
        void HandleBattlefieldStatusOpcode(WorldPacket& recv_data);
        void HandleBattleFieldPortOpcode(WorldPacket& recv_data);
        void HandleBattlefieldListOpcode(WorldPacket& recv_data);
        void HandleLeaveBattlefieldOpcode(WorldPacket& recv_data);

        void HandleWardenDataOpcode(WorldPacket& recv_data);
        void HandleWorldTeleportOpcode(WorldPacket& recv_data);
        void HandleMinimapPingOpcode(WorldPacket& recv_data);
        void HandleRandomRollOpcode(WorldPacket& recv_data);
        void HandleFarSightOpcode(WorldPacket& recv_data);
        void HandleWhoisOpcode(WorldPacket& recv_data);
        void HandleResetInstancesOpcode(WorldPacket& recv_data);

        void HandleAreaSpiritHealerQueryOpcode(WorldPacket& recv_data);
        void HandleAreaSpiritHealerQueueOpcode(WorldPacket& recv_data);
        void HandleCancelMountAuraOpcode(WorldPacket& recv_data);
        void HandleSelfResOpcode(WorldPacket& recv_data);
        void HandleRequestPetInfoOpcode(WorldPacket& recv_data);

        void HandleCancelTempEnchantmentOpcode(WorldPacket& recv_data);

        void HandleSetTaxiBenchmarkOpcode(WorldPacket& recv_data);

    private:
        // private trade methods
        void moveItems(Item* myItems[], Item* hisItems[]);
        bool VerifyMovementInfo(MovementInfo const& movementInfo, ObjectGuid const& guid) const;
        bool VerifyMovementInfo(MovementInfo const& movementInfo) const;
        void HandleMoverRelocation(MovementInfo& movementInfo);

        void ExecuteOpcode(OpcodeHandler const& opHandle, WorldPacket* packet);

        // logging helper
        void LogUnexpectedOpcode(WorldPacket* packet, const char* reason);
        void LogUnprocessedTail(WorldPacket* packet);

        Player * _player;
        WorldSocket * const m_Socket;                       // socket pointer is owned by the network thread which created 

        AccountTypes _security;
        uint32 _accountId;

        time_t _logoutTime;
        bool m_inQueue;                                     // session wait in auth.queue
        bool m_playerLoading;                               // code processed in LoginPlayer
        bool m_playerLogout;                                // code processed in LogoutPlayer
        bool m_playerRecentlyLogout;
        bool m_playerSave;                                  // code processed in LogoutPlayer with save request
        LocaleConstant m_sessionDbcLocale;
        int m_sessionDbLocaleIndex;
        uint32 m_latency;
        uint32 m_clientTimeDelay;
        uint32 m_Tutorials[8];
        TutorialDataState m_tutorialState;

        std::mutex m_recvQueueLock;
        std::deque<WorldPacket *> m_recvQueue;
};
#endif
/// @}
