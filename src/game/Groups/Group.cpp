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
#include "Server/Opcodes.h"
#include "Server/WorldPacket.h"
#include "Server/WorldSession.h"
#include "Entities/Player.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "Entities/UpdateData.h"
#include "Entities/UpdateMask.h"
#include "Groups/Group.h"
#include "Tools/Formulas.h"
#include "Globals/ObjectAccessor.h"
#include "BattleGround/BattleGround.h"
#include "Maps/MapManager.h"
#include "Maps/MapPersistentStateMgr.h"
#include "Spells/SpellAuras.h"
#include "BattleGround/BattleGroundMgr.h"
#ifdef BUILD_DEPRECATED_PLAYERBOT
#include "PlayerBot/Base/PlayerbotMgr.h"
#endif

GroupMemberStatus GetGroupMemberStatus(const Player* member = nullptr)
{
    uint8 flags = MEMBER_STATUS_OFFLINE;
    if (member && member->GetSession() && !member->GetSession()->PlayerLogout())
    {
        flags |= MEMBER_STATUS_ONLINE;
        if (member->IsPvP())
            flags |= MEMBER_STATUS_PVP;
        if (member->IsDead())
            flags |= MEMBER_STATUS_DEAD;
        if (member->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
            flags |= MEMBER_STATUS_GHOST;
        if (member->IsPvPFreeForAll())
            flags |= MEMBER_STATUS_PVP_FFA;
        if (!member->IsInWorld())
            flags |= MEMBER_STATUS_ZONE_OUT;
        if (member->isAFK())
            flags |= MEMBER_STATUS_AFK;
        if (member->isDND())
            flags |= MEMBER_STATUS_DND;
    }
    return GroupMemberStatus(flags);
}

//===================================================
//============== Group ==============================
//===================================================

Group::Group() : m_Id(0), m_groupFlags(GROUP_FLAG_NORMAL),
    m_bgGroup(nullptr), m_lootMethod(FREE_FOR_ALL), m_lootThreshold(ITEM_QUALITY_UNCOMMON),
    m_subGroupsCounts(nullptr), m_scriptRef(this, NoopGroupDeleter()), m_leaderLastOnline(0), m_LFGAreaId(0)
{
}

Group::~Group()
{
    if (m_bgGroup)
    {
        DEBUG_LOG("Group::~Group: battleground group being deleted.");
        if (m_bgGroup->GetBgRaid(ALLIANCE) == this)
            m_bgGroup->SetBgRaid(ALLIANCE, nullptr);
        else if (m_bgGroup->GetBgRaid(HORDE) == this)
            m_bgGroup->SetBgRaid(HORDE, nullptr);
        else
            sLog.outError("Group::~Group: battleground group is not linked to the correct battleground.");
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some dungeon persistent state
    for (BoundInstancesMap::iterator itr2 = m_boundInstances.begin(); itr2 != m_boundInstances.end(); ++itr2)
        itr2->second.state->RemoveGroup(this);

    // Sub group counters clean up
    delete[] m_subGroupsCounts;
}

bool Group::Create(ObjectGuid guid, const char* name)
{
    m_leaderGuid = guid;
    m_leaderName = name;
    m_leaderLastOnline = time(nullptr);

    m_groupFlags  = IsBattleGroup() ? GROUP_FLAG_RAID : GROUP_FLAG_NORMAL;

    if (m_groupFlags == GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_masterLooterGuid = guid;
    m_currentLooterGuid = guid;                                             // used for round robin looter


    if (!IsBattleGroup())
    {
        m_Id = sObjectMgr.GenerateGroupId();

        Player* leader = sObjectMgr.GetPlayer(guid);

        Player::ConvertInstancesToGroup(leader, this, guid);

        // store group in database
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM `groups` WHERE groupId ='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId ='%u'", m_Id);

        CharacterDatabase.PExecute("INSERT INTO `groups`(groupId,leaderGuid,mainTank,mainAssistant,lootMethod,looterGuid,lootThreshold,icon1,icon2,icon3,icon4,icon5,icon6,icon7,icon8,isRaid) "
                                   "VALUES('%u','%u','%u','%u','%u','%u','%u','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','%u')",
                                   m_Id, m_leaderGuid.GetCounter(), m_mainTankGuid.GetCounter(), m_mainAssistantGuid.GetCounter(), uint32(m_lootMethod),
                                   m_masterLooterGuid.GetCounter(), uint32(m_lootThreshold),
                                   m_targetIcons[0].GetRawValue(), m_targetIcons[1].GetRawValue(),
                                   m_targetIcons[2].GetRawValue(), m_targetIcons[3].GetRawValue(),
                                   m_targetIcons[4].GetRawValue(), m_targetIcons[5].GetRawValue(),
                                   m_targetIcons[6].GetRawValue(), m_targetIcons[7].GetRawValue(),
                                   IsRaidGroup());
    }

    if (!AddMember(guid, name))
        return false;

    if (!IsBattleGroup())
        CharacterDatabase.CommitTransaction();

    _updateLeaderFlag();

    return true;
}

bool Group::LoadGroupFromDB(Field* fields)
{
    //                                          0         1              2           3           4              5      6      7      8      9      10     11     12     13      14          15
    // result = CharacterDatabase.Query("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, leaderGuid, groupId FROM `groups`");

    m_Id = fields[15].GetUInt32();
    m_leaderGuid = ObjectGuid(HIGHGUID_PLAYER, fields[14].GetUInt32());

    // group leader not exist
    if (!sObjectMgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_leaderLastOnline = time(nullptr);

    m_groupFlags  = fields[13].GetBool() ? GROUP_FLAG_RAID : GROUP_FLAG_NORMAL;

    if (m_groupFlags == GROUP_FLAG_RAID)
        _initRaidSubGroupsCounter();

    m_mainTankGuid = ObjectGuid(HIGHGUID_PLAYER, fields[0].GetUInt32());
    m_mainAssistantGuid = ObjectGuid(HIGHGUID_PLAYER, fields[1].GetUInt32());
    m_lootMethod = LootMethod(fields[2].GetUInt8());
    m_masterLooterGuid = ObjectGuid(HIGHGUID_PLAYER, fields[3].GetUInt32());
    m_lootThreshold = ItemQualities(fields[4].GetUInt16());

    for (int i = 0; i < TARGET_ICON_COUNT; ++i)
        m_targetIcons[i] = ObjectGuid(fields[5 + i].GetUInt64());

    m_currentLooterGuid = m_masterLooterGuid;
    return true;
}

bool Group::LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant)
{
    MemberSlot member;
    member.guid      = ObjectGuid(HIGHGUID_PLAYER, guidLow);

    // skip nonexistent member
    if (!sObjectMgr.GetPlayerNameByGUID(member.guid, member.name))
        return false;

    member.group     = subgroup;
    member.assistant = assistant;

    int32 lastMap = sObjectMgr.GetPlayerMapIdByGUID(member.guid);
    if (lastMap < 0)
    {
        sLog.outError("Group::LoadMemberFromDB> MapId is not found for %s.", member.guid.GetString().c_str());
        return false;
    }
    member.lastMap = uint32(lastMap);

    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subgroup);

    return true;
}

void Group::ConvertToRaid()
{
    m_groupFlags = GROUP_FLAG_RAID;

    _initRaidSubGroupsCounter();

    if (!IsBattleGroup())
        CharacterDatabase.PExecute("UPDATE `groups` SET isRaid = 1 WHERE groupId='%u'", m_Id);
    SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        if (Player* player = sObjectMgr.GetPlayer(citr->guid))
            player->UpdateForQuestWorldObjects();
}

bool Group::AddInvite(Player* player)
{
    if (!player || player->GetGroupInvite())
        return false;
    Group* group = player->GetGroup();
    if (group && group->IsBattleGroup())
        group = player->GetOriginalGroup();
    if (group)
        return false;

    RemoveInvite(player);

    m_invitees.insert(player);

    player->SetGroupInvite(this);

    return true;
}

bool Group::AddLeaderInvite(Player* player)
{
    if (!AddInvite(player))
        return false;

    _updateLeaderFlag(true);
    m_leaderGuid = player->GetObjectGuid();
    m_leaderName = player->GetName();
    _updateLeaderFlag();
    return true;
}

uint32 Group::RemoveInvite(Player* player)
{
    m_invitees.erase(player);

    player->SetGroupInvite(nullptr);
    return GetMembersCount();
}

void Group::RemoveAllInvites()
{
    for (auto m_invitee : m_invitees)
        m_invitee->SetGroupInvite(nullptr);

    m_invitees.clear();
}

Player* Group::GetInvited(ObjectGuid guid) const
{
    for (auto m_invitee : m_invitees)
        if (m_invitee->GetObjectGuid() == guid)
            return m_invitee;

    return nullptr;
}

Player* Group::GetInvited(const std::string& name) const
{
    for (auto m_invitee : m_invitees)
    {
        if (m_invitee->GetName() == name)
            return m_invitee;
    }
    return nullptr;
}

bool Group::AddMember(ObjectGuid guid, const char* name, uint8 joinMethod)
{
    if (!_addMember(guid, name))
        return false;

    SendUpdate();

    if (Player* player = sObjectMgr.GetPlayer(guid))
    {
        _updateMembersOnRosterChanged(player);

        if (!IsLeader(player->GetObjectGuid()) && !IsBattleGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN);
        }
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid membership
        if (IsRaidGroup())
            player->UpdateForQuestWorldObjects();

        // Broadcast new player group member fields to rest of the group
        UpdateData groupData;

        // Broadcast group members' fields to player
        for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            if (itr->getSource() == player)
                continue;

            if (Player* member = itr->getSource())
            {
                if (player->HasAtClient(member))
                {
                    UpdateMask updateMask;
                    updateMask.SetCount(member->GetValuesCount());
                    member->MarkUpdateFieldsWithFlagForUpdate(updateMask, UF_FLAG_GROUP_ONLY);
                    if (updateMask.HasData())
                        member->BuildValuesUpdateBlockForPlayer(groupData, updateMask, player);
                }

                if (member->HasAtClient(player))
                {
                    UpdateMask updateMask;
                    updateMask.SetCount(member->GetValuesCount());
                    member->MarkUpdateFieldsWithFlagForUpdate(updateMask, UF_FLAG_GROUP_ONLY);
                    if (updateMask.HasData())
                    {
                        UpdateData newData;
                        player->BuildValuesUpdateBlockForPlayer(newData, updateMask, member);

                        if (newData.HasData())
                        {
                            WorldPacket newDataPacket = newData.BuildPacket(0, false);
                            member->SendDirectMessage(newDataPacket);
                        }
                    }
                }
            }
        }

        if (groupData.HasData())
        {
            WorldPacket groupDataPacket = groupData.BuildPacket(0, false);
            player->SendDirectMessage(groupDataPacket);
        }

#ifdef ENABLE_PLAYERBOTS
        if (!IsLeader(player->GetObjectGuid()) && sWorld.GetLFGQueue().IsPlayerInQueue(player->GetObjectGuid()))
        {
            bool notifyPlayer = true; // show "you have left queue" message only if different dungeons
            if (IsInLFG())
            {
                LFGGroupQueueInfo grpLfgInfo;
                LFGPlayerQueueInfo plrLfgInfo;
                sWorld.GetLFGQueue().GetGroupQueueInfo(&grpLfgInfo, GetId());
                sWorld.GetLFGQueue().GetPlayerQueueInfo(&plrLfgInfo, player->GetObjectGuid());
                notifyPlayer = grpLfgInfo.areaId != plrLfgInfo.areaId;
            }
            sWorld.GetLFGQueue().RemovePlayerFromQueue(player->GetObjectGuid(), notifyPlayer ? PLAYER_CLIENT_LEAVE : PLAYER_SYSTEM_LEAVE);
        }
#endif

        if (IsInLFG())
        {
            if (joinMethod != GROUP_LFG)
            {
                sLFGMgr.UpdateGroup(this, true, guid);
            }
        }
    }

    return true;
}

uint32 Group::RemoveMember(ObjectGuid guid, uint8 method)
{
    Player* player = sObjectMgr.GetPlayer(guid);
#ifdef BUILD_DEPRECATED_PLAYERBOT
    // if master leaves group, all bots leave group
    if (player && player->GetPlayerbotMgr())
        player->GetPlayerbotMgr()->RemoveAllBotsFromGroup();
#endif

    _updateMembersOnRosterChanged(player);

    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if (GetMembersCount() > GetMembersMinCount())
    {
        bool leaderChanged = _removeMember(guid);
        bool leftGroup = false;

        if (player)
        {
            // quest related GO state dependent from raid membership
            if (IsRaidGroup())
                player->UpdateForQuestWorldObjects();

            WorldPacket data;

            if (method == GROUP_KICK)
            {
                data.Initialize(SMSG_GROUP_UNINVITE, 0);
                player->GetSession()->SendPacket(data);

                if (IsInLFG())
                {
                    // send a message to group and remove from queue
                    data.Initialize(SMSG_MEETINGSTONE_SETQUEUE, 5);
                    data << 0 << uint8(MEETINGSTONE_STATUS_PARTY_MEMBER_REMOVED_PARTY_REMOVED);
                    BroadcastPacket(data, true);
                    leftGroup = true;
                    sWorld.GetLFGQueue().GetMessager().AddMessage([groupId = GetId()](LFGQueue* queue)
                    {
                        queue->RemoveGroupFromQueue(groupId);
                    });

                    // want to make sure group is removed before player is added
                    // send a message and add a kicked player to lfg queue
                    player->GetSession()->SendMeetingstoneSetqueue(m_LFGAreaId, MEETINGSTONE_STATUS_LOOKING_FOR_NEW_PARTY_IN_QUEUE);
                    sLFGMgr.AddToQueue(player, m_LFGAreaId);
                }
            }

            if (method == GROUP_LEAVE && IsInLFG())
            {
                player->GetSession()->SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);

                if (!leaderChanged)
                {
                    data.Initialize(SMSG_MEETINGSTONE_SETQUEUE, 5);
                    data << m_LFGAreaId << uint8(MEETINGSTONE_STATUS_PARTY_MEMBER_LEFT_LFG);
                    BroadcastPacket(data, true);
                }
            }

            // we already removed player from group and in player->GetGroup() is his original group!
            if (Group* group = player->GetGroup())
            {
                group->SendUpdate();
            }
            else
            {
                data.Initialize(SMSG_GROUP_LIST, 24);
                data << uint64(0) << uint64(0) << uint64(0);
                player->GetSession()->SendPacket(data);
            }

            _homebindIfInstance(player);
        }

        if (leaderChanged)
        {
            leftGroup = true;

            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_leaderName.size() + 1));
            data << m_leaderName;
            BroadcastPacket(data, true);

            sWorld.GetLFGQueue().GetMessager().AddMessage([groupId = GetId()](LFGQueue* queue)
            {
                queue->RemoveGroupFromQueue(groupId);
            });
        }

        if (!leftGroup && IsInLFG())
            sLFGMgr.UpdateGroup(this, false, guid);

        SendUpdate();
    }
    // if group before remove <= 2 disband it
    else
        Disband(true);

    return m_memberSlots.size();
}

void Group::ChangeLeader(ObjectGuid guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, slot->name.size() + 1);
    data << slot->name;
    BroadcastPacket(data, true);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player* player = sObjectMgr.GetPlayer(citr->guid);
        if (!player)
            continue;

        // we cannot call _removeMember because it would invalidate member iterator
        // if we are removing player from battleground raid
        if (IsBattleGroup())
            player->RemoveFromBattleGroundRaid();
        else
        {
            // we can remove player who is in battleground from his original group
            if (player->GetOriginalGroup() == this)
                player->SetOriginalGroup(nullptr);
            else
                player->SetGroup(nullptr);
        }

        // quest related GO state dependent from raid membership
        if (IsRaidGroup())
            player->UpdateForQuestWorldObjects();

        if (!player->GetSession())
            continue;

        WorldPacket data;
        if (!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(data);
        }

        // we already removed player from group and in player->GetGroup() is his original group, send update
        if (Group* group = player->GetGroup())
        {
            group->SendUpdate();
        }
        else
        {
            data.Initialize(SMSG_GROUP_LIST, 24);
            data << uint64(0) << uint64(0) << uint64(0);
            player->GetSession()->SendPacket(data);

            if (IsInLFG())
                player->GetSession()->SendMeetingstoneSetqueue(0, MEETINGSTONE_STATUS_NONE);
        }

        _homebindIfInstance(player);
    }

    if (IsInLFG())
    {
        sWorld.GetLFGQueue().GetMessager().AddMessage([groupId = GetId()](LFGQueue* queue)
        {
            queue->RemoveGroupFromQueue(groupId);
        });
    }

    m_memberSlots.clear();

    RemoveAllInvites();

    if (!IsBattleGroup())
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM `groups` WHERE groupId='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId='%u'", m_Id);
        CharacterDatabase.CommitTransaction();
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, nullptr);
    }

    _updateLeaderFlag(true);
    m_leaderGuid.Clear();
    m_leaderName.clear();
}

void Group::SetTargetIcon(uint8 id, ObjectGuid targetGuid)
{
    if (id >= TARGET_ICON_COUNT)
        return;

    // clean other icons
    if (targetGuid)
        for (int i = 0; i < TARGET_ICON_COUNT; ++i)
            if (m_targetIcons[i] == targetGuid)
                SetTargetIcon(i, ObjectGuid());

    m_targetIcons[id] = targetGuid;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1 + 1 + 8));
    data << uint8(0);                                       // set targets
    data << uint8(id);
    data << targetGuid;
    BroadcastPacket(data, true);
}

static void GetDataForXPAtKill_helper(Player* player, Unit const* victim, uint32& sum_level, Player*& member_with_max_level, Player*& not_gray_member_with_max_level)
{
    const uint32 level = player->GetLevel();

    sum_level += level;

    if (!member_with_max_level || member_with_max_level->GetLevel() < level)
        member_with_max_level = player;

    if (MaNGOS::XP::IsTrivialLevelDifference(level, victim->GetLevelForTarget(player)))
        return;

    if (!not_gray_member_with_max_level || not_gray_member_with_max_level->GetLevel() < level)
        not_gray_member_with_max_level = player;
}

void Group::GetDataForXPAtKill(Unit const* victim, uint32& count, uint32& sum_level, Player*& member_with_max_level, Player*& not_gray_member_with_max_level, Player* additional)
{
    for (GroupReference const* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member || !member->IsAlive())                  // only for alive
            continue;

        // will proccesed later
        if (member == additional)
            continue;

        if (!member->IsAtGroupRewardDistance(victim))       // at req. distance
            continue;

        ++count;
        GetDataForXPAtKill_helper(member, victim, sum_level, member_with_max_level, not_gray_member_with_max_level);
    }

    if (additional)
    {
        if (additional->IsAtGroupRewardDistance(victim))    // at req. distance
        {
            ++count;
            GetDataForXPAtKill_helper(additional, victim, sum_level, member_with_max_level, not_gray_member_with_max_level);
        }
    }
}

void Group::SendTargetIconList(WorldSession* session) const
{
    if (!session)
        return;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1 + TARGET_ICON_COUNT * 9));
    data << uint8(1);

    for (int i = 0; i < TARGET_ICON_COUNT; ++i)
    {
        if (!m_targetIcons[i])
            continue;

        data << uint8(i);
        data << m_targetIcons[i];
    }

    session->SendPacket(data);
}

void Group::SendUpdateTo(Player* player)
{
    if (player && player->GetGroup() == this)
    {
        WorldSession* session = player->GetSession();

        if (!session)
            return;

        member_citerator citr = _getMemberCSlot(player->GetObjectGuid());

        if (citr == m_memberSlots.end())
            return;

        // guess size
        WorldPacket data(SMSG_GROUP_LIST, (1 + 1 + 1 + 4 + GetMembersCount() * 20) + 8 + 1 + 8 + 1);
        data << (uint8)m_groupFlags;                         // group type
        data << (uint8)(citr->group | (citr->assistant ? 0x80 : 0)); // own flags (groupid | (assistant?0x80:0))

        data << uint32(GetMembersCount() - 1);
        for (member_citerator citr2 = m_memberSlots.begin(); citr2 != m_memberSlots.end(); ++citr2)
        {
            if (citr->guid == citr2->guid)
                continue;
            data << citr2->name;
            data << citr2->guid;
            data << uint8(GetGroupMemberStatus(sObjectMgr.GetPlayer(citr2->guid)));
            data << (uint8)(citr2->group | (citr2->assistant ? 0x80 : 0));
        }

        ObjectGuid masterLootGuid = (m_lootMethod == MASTER_LOOT) ? m_masterLooterGuid : ObjectGuid();
        data << m_leaderGuid;                               // leader guid
        if (GetMembersCount() - 1)
        {
            data << uint8(m_lootMethod);                    // loot method
            data << masterLootGuid;                         // master loot guid
            data << uint8(m_lootThreshold);                 // loot threshold
            data << uint8(0);                               // Heroic Mod Group - unused in vanilla
        }

        session->SendPacket(data);
    }
}

void Group::SendUpdate()
{
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        SendUpdateTo(sObjectMgr.GetPlayer(citr->guid));
}

void Group::UpdatePlayerOutOfRange(Player* pPlayer)
{
    if (!pPlayer || !pPlayer->IsInWorld())
        return;

    if (pPlayer->GetGroupUpdateFlag() == GROUP_UPDATE_FLAG_NONE)
        return;

    WorldPacket data;
    WorldSession::BuildPartyMemberStatsChangedPacket(pPlayer, data);

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* player = itr->getSource())
            if (player != pPlayer && !player->HasAtClient(pPlayer))
                player->GetSession()->SendPacket(data);
}

void Group::UpdatePlayerOnlineStatus(Player* player, bool online /*= true*/)
{
    if (!player)
        return;
    const ObjectGuid guid = player->GetObjectGuid();
    if (!IsMember(guid))
        return;

    SendUpdate();
    if (online)
    {
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);
    }
    else if (IsLeader(guid))
        m_leaderLastOnline = time(nullptr);
}

void Group::UpdateOfflineLeader(time_t time, uint32 delay)
{
    // Do not update BG groups, BGs take care of offliners
    if (IsBattleGroup())
        return;

    // Check leader presence
    if (const Player* leader = sObjectMgr.GetPlayer(m_leaderGuid))
    {
        // Consider loading a new map as being online as well until session finally times out
        if (leader->IsInWorld() || (leader->GetSession() && leader->IsBeingTeleportedFar()))
        {
            m_leaderLastOnline = time;
            return;
        }
    }

    // Check for delay
    if ((time - m_leaderLastOnline) < delay)
        return;

    _chooseLeader(true);
}

void Group::BroadcastPacket(WorldPacket const& packet, bool ignorePlayersInBGRaid, int group, ObjectGuid ignore) const
{
    for (GroupReference const* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();
        if (!pl || (ignore && pl->GetObjectGuid() == ignore) || (ignorePlayersInBGRaid && pl->GetGroup() != this))
            continue;

        if (pl->GetSession() && (group == -1 || itr->getSubGroup() == group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastPacketInRange(WorldObject const* who, WorldPacket const& packet, bool ignorePlayersInBGRaid, int group, ObjectGuid ignore) const
{
    for (auto itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();
        if (!pl || (ignore && pl->GetObjectGuid() == ignore) || (ignorePlayersInBGRaid && pl->GetGroup() != this))
            continue;

        if (!pl->IsAtGroupRewardDistance(who))
            continue;

        if (pl->GetSession() && (group == -1 || itr->getSubGroup() == group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket const& packet) const
{
    for (GroupReference const* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();
        if (pl && pl->GetSession())
            if (IsLeader(pl->GetObjectGuid()) || IsAssistant(pl->GetObjectGuid()))
                pl->GetSession()->SendPacket(packet);
    }
}

bool Group::_addMember(ObjectGuid guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; groupid < MAX_RAID_SUBGROUPS; ++groupid)
        {
            if (m_subGroupsCounts[groupid] < MAX_GROUP_SIZE)
            {
                groupFound = true;
                break;
            }
        }
        // We are raid group and no one slot is free
        if (!groupFound)
            return false;
    }

    return _addMember(guid, name, isAssistant, groupid);
}

bool Group::_addMember(ObjectGuid guid, const char* name, bool isAssistant, uint8 group)
{
    if (IsFull())
        return false;

    if (!guid)
        return false;

    Player* player = sObjectMgr.GetPlayer(guid, false);

    uint32 lastMap = 0;
    if (player && player->IsInWorld())
        lastMap = player->GetMapId();
    else if (player && player->IsBeingTeleported())
        lastMap = player->GetTeleportDest().mapid;

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    member.lastMap   = lastMap;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(group);

    if (player)
    {
        player->SetGroupInvite(nullptr);
        // if player is in group and he is being added to BG raid group, then call SetBattleGroundRaid()
        if (player->GetGroup() && IsBattleGroup())
            player->SetBattleGroundRaid(this, group);
        // if player is in bg raid and we are adding him to normal group, then call SetOriginalGroup()
        else if (player->GetGroup())
            player->SetOriginalGroup(this, group);
        // if player is not in group, then call set group
        else
            player->SetGroup(this, group);

        if (player->IsInWorld())
        {
            // if the same group invites the player back, cancel the homebind timer
            if (InstanceGroupBind* bind = GetBoundInstance(player->GetMapId()))
                if (bind->state->GetInstanceId() == player->GetInstanceId())
                    player->m_InstanceValid = true;
        }

        if (player->IsPvPFreeForAll())
        {
            player->ForceHealthAndPowerUpdate();
            for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
            {
                Player* groupMember = itr->getSource();
                if (groupMember && groupMember->GetSession())
                    groupMember->ForceHealthAndPowerUpdate();
            }
        }
    }

    if (!IsRaidGroup())                                     // reset targetIcons for non-raid-groups
    {
        for (auto& m_targetIcon : m_targetIcons)
            m_targetIcon.Clear();
    }

    if (!IsBattleGroup())
    {
        // insert into group table
        CharacterDatabase.PExecute("INSERT INTO group_member(groupId,memberGuid,assistant,subgroup) VALUES('%u','%u','%u','%u')",
                                   m_Id, member.guid.GetCounter(), ((member.assistant == 1) ? 1 : 0), member.group);
    }

    return true;
}

bool Group::_removeMember(ObjectGuid guid)
{
    Player* player = sObjectMgr.GetPlayer(guid);
    if (player)
    {
        // if we are removing player from battleground raid
        if (IsBattleGroup())
            player->RemoveFromBattleGroundRaid();
        else
        {
            // we can remove player who is in battleground from his original group
            if (player->GetOriginalGroup() == this)
                player->SetOriginalGroup(nullptr);
            else
                player->SetGroup(nullptr);
        }
    }

    member_witerator slot = _getMemberWSlot(guid);
    if (slot != m_memberSlots.end())
    {
        SubGroupCounterDecrease(slot->group);

        m_memberSlots.erase(slot);
    }

    if (!IsBattleGroup())
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid='%u'", guid.GetCounter());

    if (m_leaderGuid == guid)                               // leader was removed
    {
        _updateLeaderFlag(true);
        _chooseLeader();
        return true;
    }

    return false;
}

void Group::_chooseLeader(bool offline /*= false*/)
{
    if (GetMembersCount() < GetMembersMinCount())
        return;

    ObjectGuid first = ObjectGuid(); // First available: if no suitable canditates are found
    ObjectGuid chosen = ObjectGuid(); // Player matching prio creteria

    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        if (citr->guid == m_leaderGuid)
            continue;

        // Prioritize online players
        Player* player = sObjectMgr.GetPlayer(citr->guid);
        if (!player || !player->GetSession() || player->GetGroup() != this)
            continue;

        // Prioritize assistants for raids
        if (IsRaidGroup() && !citr->assistant)
        {
            if (first.IsEmpty())
                first = citr->guid;
            continue;
        }

        chosen = citr->guid;
        break;
    }

    if (chosen.IsEmpty())
        chosen = first;

    // If we are choosing a new leader due to inactivity, check if everyone is offline first
    if (offline && chosen.IsEmpty())
        return;

    // Still nobody online...
    if (chosen.IsEmpty())
        chosen = m_memberSlots.front().guid;

    // Do announce if we are choosing a new leader due to old one being offline
    return (offline ? ChangeLeader(chosen) : _setLeader(chosen));
}

void Group::_setLeader(ObjectGuid guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    if (!IsBattleGroup())
    {
        uint32 slot_lowguid = slot->guid.GetCounter();

        uint32 leader_lowguid = m_leaderGuid.GetCounter();

        // TODO: set a time limit to have this function run rarely cause it can be slow
        CharacterDatabase.BeginTransaction();

        // update the group's bound instances when changing leaders

        // remove all permanent binds from the group
        // in the DB also remove solo binds that will be replaced with permbinds
        // from the new leader
        CharacterDatabase.PExecute(
            "DELETE FROM group_instance WHERE leaderguid='%u' AND (permanent = 1 OR "
            "instance IN (SELECT instance FROM character_instance WHERE guid = '%u')"
            ")", leader_lowguid, slot_lowguid);

        Player* player = sObjectMgr.GetPlayer(slot->guid);

        if (player)
        {
            for (BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
            {
                if (itr->second.perm)
                {
                    itr->second.state->RemoveGroup(this);
                    m_boundInstances.erase(itr++);
                }
                else
                    ++itr;
            }
        }

        // update the group's solo binds to the new leader
        CharacterDatabase.PExecute("UPDATE group_instance SET leaderGuid='%u' WHERE leaderGuid = '%u'",
                                   slot_lowguid, leader_lowguid);

        // copy the permanent binds from the new leader to the group
        // overwriting the solo binds with permanent ones if necessary
        // in the DB those have been deleted already
        Player::ConvertInstancesToGroup(player, this, slot->guid);

        // update the group leader
        CharacterDatabase.PExecute("UPDATE `groups` SET leaderGuid='%u' WHERE groupId='%u'", slot_lowguid, m_Id);
        CharacterDatabase.CommitTransaction();
    }

    _updateLeaderFlag(true);
    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
    m_leaderLastOnline = time(nullptr);
    _updateLeaderFlag();
}

void Group::_updateLeaderFlag(bool remove /*= false*/) const
{
    if (Player* player = sObjectMgr.GetPlayer(m_leaderGuid))
        player->UpdateGroupLeaderFlag(remove);
}

void Group::_updateMembersOnRosterChanged(Player* changed)
{
    if (!changed || !changed->IsInWorld())
        return;

    // Fog of War: setting implies health obfuscation outside of groups, force update
    const bool fow = (sWorld.getConfig(CONFIG_UINT32_FOGOFWAR_HEALTH) == 0);
    // [XFACTION]: Prepare to alter fields if detected crossfaction group interaction
    const bool xfaction = sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP);

    if (!fow && !xfaction)
        return;

    auto update = [&fow, &xfaction] (Unit* pov, Unit* target)
    {
        if (fow)
        {
            auto forcehp = [] (Unit* u) { u->ForceValuesUpdateAtIndex(UNIT_FIELD_HEALTH); u->ForceValuesUpdateAtIndex(UNIT_FIELD_MAXHEALTH); };
            forcehp(target);
            target->CallForAllControlledUnits(forcehp, (CONTROLLED_MINIPET | CONTROLLED_PET | CONTROLLED_TOTEMS | CONTROLLED_GUARDIANS | CONTROLLED_CHARM));
        }

        // [XFACTION]: Prepare to alter object fields if detected crossfaction group interaction:
        if (xfaction && !pov->CanCooperate(target))
        {
            target->ForceValuesUpdateAtIndex(UNIT_FIELD_FACTIONTEMPLATE);

            if (target->GetTypeId() == TYPEID_PLAYER)
            {
                if (Corpse* corpse = static_cast<Player*>(target)->GetCorpse())
                    corpse->ForceValuesUpdateAtIndex(CORPSE_FIELD_BYTES_1);
            }
        }
    };

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        if (Player* member = itr->getSource())
        {
            if (member != changed && member->IsInWorld())
            {
                if (member->HasAtClient(changed))
                    update(member, changed);

                if (changed->HasAtClient(member))
                    update(changed, member);
            }
        }
    }
}

bool Group::_setMembersGroup(ObjectGuid guid, uint8 group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->group = group;

    SubGroupCounterIncrease(group);

    if (!IsBattleGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET subgroup='%u' WHERE memberGuid='%u'", group, guid.GetCounter());

    return true;
}

bool Group::_setAssistantFlag(ObjectGuid guid, const bool& state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->assistant = state;
    if (!IsBattleGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET assistant='%u' WHERE memberGuid='%u'", (state) ? 1 : 0, guid.GetCounter());
    return true;
}

bool Group::_setMainTank(ObjectGuid guid)
{
    if (m_mainTankGuid == guid)
        return false;

    if (guid)
    {
        member_citerator slot = _getMemberCSlot(guid);
        if (slot == m_memberSlots.end())
            return false;

        if (m_mainAssistantGuid == guid)
            _setMainAssistant(ObjectGuid());
    }

    m_mainTankGuid = guid;

    if (!IsBattleGroup())
        CharacterDatabase.PExecute("UPDATE `groups` SET mainTank='%u' WHERE groupId='%u'", m_mainTankGuid.GetCounter(), m_Id);

    return true;
}

bool Group::_setMainAssistant(ObjectGuid guid)
{
    if (m_mainAssistantGuid == guid)
        return false;

    if (guid)
    {
        member_witerator slot = _getMemberWSlot(guid);
        if (slot == m_memberSlots.end())
            return false;

        if (m_mainTankGuid == guid)
            _setMainTank(ObjectGuid());
    }

    m_mainAssistantGuid = guid;

    if (!IsBattleGroup())
        CharacterDatabase.PExecute("UPDATE `groups` SET mainAssistant='%u' WHERE groupId='%u'",
                                   m_mainAssistantGuid.GetCounter(), m_Id);

    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if (!member1 || !member2)
        return false;
    if (member1->GetGroup() != this || member2->GetGroup() != this)
        return false;
    return member1->GetSubGroup() == member2->GetSubGroup();
}

// allows setting subgroup for offline members
void Group::ChangeMembersGroup(ObjectGuid guid, uint8 group)
{
    if (!IsRaidGroup())
        return;

    Player* player = sObjectMgr.GetPlayer(guid);

    if (!player)
    {
        uint8 prevSubGroup = GetMemberGroup(guid);
        if (prevSubGroup == group)
            return;

        if (_setMembersGroup(guid, group))
        {
            SubGroupCounterDecrease(prevSubGroup);
            SendUpdate();
        }
    }
    else
        // This methods handles itself groupcounter decrease
        ChangeMembersGroup(player, group);
}

// only for online members
void Group::ChangeMembersGroup(Player* player, uint8 group)
{
    if (!player || !IsRaidGroup())
        return;

    uint8 prevSubGroup = player->GetSubGroup();
    if (prevSubGroup == group)
        return;

    if (_setMembersGroup(player->GetObjectGuid(), group))
    {
        if (player->GetGroup() == this)
            player->GetGroupRef().setSubGroup(group);
        // if player is in BG raid, it is possible that he is also in normal raid - and that normal raid is stored in m_originalGroup reference
        else
        {
            prevSubGroup = player->GetOriginalSubGroup();
            player->GetOriginalGroupRef().setSubGroup(group);
        }
        SubGroupCounterDecrease(prevSubGroup);

        SendUpdate();
    }
}

uint32 Group::CanJoinBattleGroundQueue(BattleGroundTypeId bgTypeId, BattleGroundQueueTypeId bgQueueTypeId, uint32 MinPlayerCount, uint32 MaxPlayerCount)
{
    // check for min / max count
    uint32 memberscount = GetMembersCount();
    if (memberscount < MinPlayerCount)
        return BG_JOIN_ERR_GROUP_NOT_ENOUGH;
    if (memberscount > MaxPlayerCount)
        return BG_JOIN_ERR_GROUP_TOO_MANY;

    // get a player as reference, to compare other players' stats to (queue id based on level, etc.)
    Player* reference = GetFirstMember()->getSource();
    // no reference found, can't join this way
    if (!reference)
        return BG_JOIN_ERR_OFFLINE_MEMBER;

    BattleGroundBracketId bracket_id = sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, reference->GetLevel());
    Team team = reference->GetTeam();

    // check every member of the group to be able to join
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->getSource();
        // offline member? don't let join
        if (!member)
            return BG_JOIN_ERR_OFFLINE_MEMBER;
        // don't allow cross-faction join as group
        if (member->GetTeam() != team)
            return BG_JOIN_ERR_MIXED_FACTION;
        // not in the same battleground level bracket, don't let join
        if (sBattleGroundMgr.GetBattleGroundBracketIdFromLevel(bgTypeId, member->GetLevel()) != bracket_id)
            return BG_JOIN_ERR_MIXED_LEVELS;
        // don't let join if someone from the group is already in that bg queue
        if (member->InBattleGroundQueueForBattleGroundQueueType(bgQueueTypeId))
            return BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE;
        // check for deserter debuff
        if (!member->CanJoinToBattleground())
            return BG_JOIN_ERR_GROUP_DESERTER;
        // check if member can join any more battleground queues
        if (!member->HasFreeBattleGroundQueueId())
            return BG_JOIN_ERR_ALL_QUEUES_USED;
    }
    return BG_JOIN_ERR_OK;
}

bool Group::InCombatToInstance(uint32 instanceId)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pPlayer = itr->getSource();
        if (!pPlayer->getAttackers().empty() && pPlayer->GetInstanceId() == instanceId)
            return true;
    }
    return false;
}

void Group::ResetInstances(InstanceResetMethod method, Player* SendMsgTo)
{
    if (IsBattleGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_GROUP_DISBAND

    typedef std::set<uint32> Uint32Set;
    Uint32Set mapsWithOfflinePlayer;                        // to store map of offline players
    Uint32Set mapsWithBeingTeleportedPlayer;                // to store map of offline players

    if (method != INSTANCE_RESET_GROUP_DISBAND)
    {
        // Store maps in which are offline members for instance reset check.
        for (member_citerator itr = m_memberSlots.begin(); itr != m_memberSlots.end(); ++itr)
        {
            Player* plr = ObjectAccessor::FindPlayer(itr->guid);
            if (!plr)
            {
                // add last map from offline player
                mapsWithOfflinePlayer.insert(itr->lastMap);
            }
            else
            {
                // add teleport destination map
                if (plr->IsBeingTeleported())
                    mapsWithBeingTeleportedPlayer.insert(plr->GetTeleportDest().mapid);
            }
        }
    }

    for (BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
    {
        DungeonPersistentState* state = itr->second.state;
        const MapEntry* entry = sMapStore.LookupEntry(itr->first);
        if (!entry || (!state->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND))
        {
            ++itr;
            continue;
        }

        if (method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if (entry->map_type == MAP_RAID)
            {
                ++itr;
                continue;
            }
        }

        // check if there are offline members on the map
        if (method != INSTANCE_RESET_GROUP_DISBAND && mapsWithOfflinePlayer.find(state->GetMapId()) != mapsWithOfflinePlayer.end())
        {
            if (SendMsgTo)
                SendMsgTo->SendResetInstanceFailed(1, state->GetMapId());
            ++itr;
            continue;
        }

        // check if there are teleporting members to the map
        if (method != INSTANCE_RESET_GROUP_DISBAND && mapsWithBeingTeleportedPlayer.find(state->GetMapId()) != mapsWithBeingTeleportedPlayer.end())
        {
            if (SendMsgTo)
                SendMsgTo->SendResetInstanceFailed(2, state->GetMapId());
            ++itr;
            continue;
        }

        bool isEmpty = true;
        // if the map is loaded, reset it if can
        if (entry->IsDungeon() && !(method == INSTANCE_RESET_GROUP_DISBAND && !state->CanReset()))
            if (Map* map = sMapMgr.FindMap(state->GetMapId(), state->GetInstanceId()))
                isEmpty = ((DungeonMap*)map)->Reset(method);

        if (SendMsgTo)
        {
            if (isEmpty)
                SendMsgTo->SendResetInstanceSuccess(state->GetMapId());
            else
                SendMsgTo->SendResetInstanceFailed(0, state->GetMapId());
        }

        if (isEmpty || method == INSTANCE_RESET_GROUP_DISBAND)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if (state->CanReset())
                state->DeleteFromDB();
            else
                CharacterDatabase.PExecute("DELETE FROM group_instance WHERE instance = '%u'", state->GetInstanceId());
            // i don't know for sure if hash_map iterators
            m_boundInstances.erase(itr);
            itr = m_boundInstances.begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            state->RemoveGroup(this);
        }
        else
            ++itr;
    }
}

InstanceGroupBind* Group::GetBoundInstance(uint32 mapid)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry)
        return nullptr;

    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    if (itr != m_boundInstances.end())
        return &itr->second;

    return nullptr;
}

InstanceGroupBind* Group::BindToInstance(DungeonPersistentState* state, bool permanent, bool load)
{
    if (state && !IsBattleGroup())
    {
        InstanceGroupBind& bind = m_boundInstances[state->GetMapId()];
        if (bind.state)
        {
            // when a boss is killed or when copying the players's binds to the group
            if (permanent != bind.perm || state != bind.state)
                if (!load)
                    CharacterDatabase.PExecute("UPDATE group_instance SET instance = '%u', permanent = '%u' WHERE leaderGuid = '%u' AND instance = '%u'",
                                               state->GetInstanceId(), permanent, GetLeaderGuid().GetCounter(), bind.state->GetInstanceId());
        }
        else if (!load)
            CharacterDatabase.PExecute("INSERT INTO group_instance (leaderGuid, instance, permanent) VALUES ('%u', '%u', '%u')",
                                       GetLeaderGuid().GetCounter(), state->GetInstanceId(), permanent);

        if (bind.state != state)
        {
            if (bind.state)
                bind.state->RemoveGroup(this);
            state->AddGroup(this);
        }

        bind.state = state;
        bind.perm = permanent;
        if (!load)
            DEBUG_LOG("Group::BindToInstance: Group (Id: %d) is now bound to map %d, instance %d",
                      GetId(), state->GetMapId(), state->GetInstanceId());
        return &bind;
    }
    return nullptr;
}

void Group::UnbindInstance(uint32 mapid, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    if (itr != m_boundInstances.end())
    {
        if (!unload)
            CharacterDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u' AND instance = '%u'",
                                       GetLeaderGuid().GetCounter(), itr->second.state->GetInstanceId());
        itr->second.state->RemoveGroup(this);               // save can become invalid
        m_boundInstances.erase(itr);
    }
}

void Group::_homebindIfInstance(Player* player) const
{
    if (player && !player->IsGameMaster())
    {
        Map* map = player->GetMap();
        if (map->IsDungeon())
        {
            // leaving the group in an instance, the homebind timer is started
            // unless the player is permanently saved to the instance
            InstancePlayerBind* playerBind = player->GetBoundInstance(map->GetId());
            if (!playerBind || !playerBind->perm)
                player->m_InstanceValid = false;
        }
    }
}

static void RewardGroupAtKill_helper(Player* pGroupGuy, Unit* pVictim, uint32 count, bool PvP, float group_rate, uint32 sum_level, bool is_dungeon, Player* not_gray_member_with_max_level, Player* member_with_max_level, uint32 xp)
{
    // honor can be in PvP and !PvP (racial leader) cases (for alive)
    if (pGroupGuy->IsAlive())
        pGroupGuy->RewardHonor(pVictim, count);

    // xp and reputation only in !PvP case
    if (!PvP)
    {
        if (pVictim->GetTypeId() == TYPEID_UNIT)
        {
            Creature* creatureVictim = static_cast<Creature*>(pVictim);
            float rate = group_rate * float(pGroupGuy->GetLevel()) / sum_level;

            // if is in dungeon then all receive full reputation at kill
            // rewarded any alive/dead/near_corpse group member
            pGroupGuy->RewardReputation(creatureVictim, is_dungeon ? 1.0f : 1.0f / count);

            // XP updated only for alive group member
            if (pGroupGuy->IsAlive() && not_gray_member_with_max_level &&
                pGroupGuy->GetLevel() <= not_gray_member_with_max_level->GetLevel())
            {
                float itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? xp * rate : (xp * rate * 0.5f) + 1.0f;

                pGroupGuy->GiveXP((uint32)(std::round(itr_xp)), creatureVictim, group_rate);
                if (Pet* pet = pGroupGuy->GetPet())
                {
                    uint32 pet_xp = MaNGOS::XP::Gain(pet, creatureVictim);
                    itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? pet_xp * rate : (pet_xp * rate * 0.5f) + 1.0f;
                    pet->GivePetXP((uint32)(std::round(itr_xp * rate)));
                }
            }

            // quest objectives updated only for alive group member or dead but with not released body
            if (pGroupGuy->IsAlive() || !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
            {
                // normal creature (not pet/etc) can be only in !PvP case
                if (creatureVictim->GetTypeId() == TYPEID_UNIT)
                    if (CreatureInfo const* normalInfo = creatureVictim->GetCreatureInfo())
                        pGroupGuy->KilledMonster(normalInfo, creatureVictim);
            }
        }
    }
}

/** Provide rewards to group members at unit kill
 *
 * @param pVictim       Killed unit
 * @param player_tap    Player who tap unit if online, it can be group member or can be not if leaved after tap but before kill target
 *
 * Rewards received by group members and player_tap
 */
void Group::RewardGroupAtKill(Unit* pVictim, Player* player_tap)
{
    bool PvP = pVictim->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PLAYER_CONTROLLED);

    // prepare data for near group iteration (PvP and !PvP cases)
    uint32 count = 0;
    uint32 sum_level = 0;
    Player* member_with_max_level = nullptr;
    Player* not_gray_member_with_max_level = nullptr;

    GetDataForXPAtKill(pVictim, count, sum_level, member_with_max_level, not_gray_member_with_max_level, player_tap);

    if (member_with_max_level)
    {
        /// not get Xp in PvP or no not gray players in group
        uint32 xp = (PvP || !not_gray_member_with_max_level || pVictim->GetTypeId() != TYPEID_UNIT) ? 0 : MaNGOS::XP::Gain(not_gray_member_with_max_level, static_cast<Creature*>(pVictim));

        /// skip in check PvP case (for speed, not used)
        bool is_raid = PvP ? false : sMapStore.LookupEntry(pVictim->GetMapId())->IsRaid() && IsRaidGroup();
        bool is_dungeon = PvP ? false : sMapStore.LookupEntry(pVictim->GetMapId())->IsDungeon();
        float group_rate = MaNGOS::XP::xp_in_group_rate(count, is_raid);

        for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* pGroupGuy = itr->getSource();
            if (!pGroupGuy)
                continue;

            // will proccessed later
            if (pGroupGuy == player_tap)
                continue;

            if (!pGroupGuy->IsAtGroupRewardDistance(pVictim))
                continue;                               // member (alive or dead) or his corpse at req. distance

            RewardGroupAtKill_helper(pGroupGuy, pVictim, count, PvP, group_rate, sum_level, is_dungeon, not_gray_member_with_max_level, member_with_max_level, xp);
        }

        if (player_tap)
        {
            // member (alive or dead) or his corpse at req. distance
            if (player_tap->IsAtGroupRewardDistance(pVictim))
                RewardGroupAtKill_helper(player_tap, pVictim, count, PvP, group_rate, sum_level, is_dungeon, not_gray_member_with_max_level, member_with_max_level, xp);
        }
    }
}

/*********************************************************/
/***                   LFG SYSTEM                      ***/
/*********************************************************/

void Group::CalculateLFGRoles(LFGGroupQueueInfo& data)
{
    uint32 m_initRoles = (PLAYER_ROLE_TANK | PLAYER_ROLE_DAMAGE | PLAYER_ROLE_HEALER);
    uint32 dpsCount = 0;

    static std::array<LfgRoles, 3> PotentialRoles =
    {
        PLAYER_ROLE_TANK,
        PLAYER_ROLE_HEALER,
        PLAYER_ROLE_DAMAGE
    };

    std::list<ObjectGuid> processed;

    for (const auto& citr : GetMemberSlots())
    {
        Classes playerClass = (Classes)sObjectMgr.GetPlayerClassByGUID(citr.guid);
        LfgRoles lfgRole;
        Player* member = sObjectMgr.GetPlayer(citr.guid);
        // if enabled and player is online, calculate role based on most used talent tree
        if (member && sWorld.getConfig(CONFIG_BOOL_LFG_MATCHMAKING))
            lfgRole = LFGMgr::CalculateTalentRoles(member);
        else
            lfgRole = LFGMgr::CalculateRoles(playerClass);

        for (LfgRoles role : PotentialRoles)
        {
            // We can't fulfill this role as our class, skip it
            if (!(lfgRole & role))
                continue;

            if ((role & m_initRoles) == role)
            {
                // If we occupy the slot, don't test other roles
                if (FillPremadeLFG(citr.guid, playerClass, role, m_initRoles, dpsCount, processed))
                    break;
            }
        }
    }

    data.availableRoles = (LfgRoles)m_initRoles;
    data.dpsCount = dpsCount;
    data.playerCount = GetMembersCount();
}

bool Group::FillPremadeLFG(ObjectGuid const& plrGuid, Classes playerClass, LfgRoles requiredRole, uint32& InitRoles,
    uint32& DpsCount, std::list<ObjectGuid>& processed)
{
    // We grant the role unless someone else in the group has higher priority for it
    LfgRolePriority priority = LFGMgr::GetPriority(playerClass, requiredRole);

    for (const auto& citr : GetMemberSlots())
    {
        if (plrGuid == citr.guid)
            continue;

        // Player is already allocated
        if (std::find(std::begin(processed), std::end(processed), citr.guid) != processed.end())
            continue;

        Classes memberClass = (Classes)sObjectMgr.GetPlayerClassByGUID(citr.guid);

        // Someone else has higher prio
        if (priority < LFGMgr::GetPriority(memberClass, requiredRole))
            return false;
    }

    switch (requiredRole)
    {
    case PLAYER_ROLE_TANK:
    {
        InitRoles &= ~PLAYER_ROLE_TANK;
        break;
    }
    case PLAYER_ROLE_HEALER:
    {
        InitRoles &= ~PLAYER_ROLE_HEALER;
        break;
    }
    case PLAYER_ROLE_DAMAGE:
    {
        if (DpsCount < LFGMgr::GetMaximumDPSSlots())
        {
            ++DpsCount;

            if (DpsCount >= LFGMgr::GetMaximumDPSSlots())
                InitRoles &= ~PLAYER_ROLE_DAMAGE;
        }
        break;
    }
    default:
        break;
    }
    processed.push_back(plrGuid);

    return true;
}
