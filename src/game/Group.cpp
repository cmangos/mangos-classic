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
#include "Opcodes.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Player.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Group.h"
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "BattleGround/BattleGround.h"
#include "MapManager.h"
#include "MapPersistentStateMgr.h"

//===================================================
//============== Group ==============================
//===================================================

Group::Group() : m_Id(0), m_groupType(GROUPTYPE_NORMAL),
    m_bgGroup(nullptr), m_lootMethod(FREE_FOR_ALL), m_lootThreshold(ITEM_QUALITY_UNCOMMON),
    m_subGroupsCounts(nullptr)
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

    m_groupType  = isBGGroup() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_masterLooterGuid = guid;
    m_currentLooterGuid = guid;                                             // used for round robin looter


    if (!isBGGroup())
    {
        m_Id = sObjectMgr.GenerateGroupId();

        Player* leader = sObjectMgr.GetPlayer(guid);

        Player::ConvertInstancesToGroup(leader, this, guid);

        // store group in database
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId ='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId ='%u'", m_Id);

        CharacterDatabase.PExecute("INSERT INTO groups(groupId,leaderGuid,mainTank,mainAssistant,lootMethod,looterGuid,lootThreshold,icon1,icon2,icon3,icon4,icon5,icon6,icon7,icon8,isRaid) "
                                   "VALUES('%u','%u','%u','%u','%u','%u','%u','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','%u')",
                                   m_Id, m_leaderGuid.GetCounter(), m_mainTankGuid.GetCounter(), m_mainAssistantGuid.GetCounter(), uint32(m_lootMethod),
                                   m_masterLooterGuid.GetCounter(), uint32(m_lootThreshold),
                                   m_targetIcons[0].GetRawValue(), m_targetIcons[1].GetRawValue(),
                                   m_targetIcons[2].GetRawValue(), m_targetIcons[3].GetRawValue(),
                                   m_targetIcons[4].GetRawValue(), m_targetIcons[5].GetRawValue(),
                                   m_targetIcons[6].GetRawValue(), m_targetIcons[7].GetRawValue(),
                                   isRaidGroup());
    }

    if (!AddMember(guid, name))
        return false;

    if (!isBGGroup())
        CharacterDatabase.CommitTransaction();

    return true;
}

bool Group::LoadGroupFromDB(Field* fields)
{
    //                                          0         1              2           3           4              5      6      7      8      9      10     11     12     13      14          15
    // result = CharacterDatabase.Query("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, leaderGuid, groupId FROM groups");

    m_Id = fields[15].GetUInt32();
    m_leaderGuid = ObjectGuid(HIGHGUID_PLAYER, fields[14].GetUInt32());

    // group leader not exist
    if (!sObjectMgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_groupType  = fields[13].GetBool() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
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
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(subgroup);

    return true;
}

void Group::ConvertToRaid()
{
    m_groupType = GROUPTYPE_RAID;

    _initRaidSubGroupsCounter();

    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET isRaid = 1 WHERE groupId='%u'", m_Id);
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
    if (group && group->isBGGroup())
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

    m_leaderGuid = player->GetObjectGuid();
    m_leaderName = player->GetName();
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
    for (InvitesList::iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
        (*itr)->SetGroupInvite(nullptr);

    m_invitees.clear();
}

Player* Group::GetInvited(ObjectGuid guid) const
{
    for (InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
        if ((*itr)->GetObjectGuid() == guid)
            return (*itr);

    return nullptr;
}

Player* Group::GetInvited(const std::string& name) const
{
    for (InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if ((*itr)->GetName() == name)
            return (*itr);
    }
    return nullptr;
}

bool Group::AddMember(ObjectGuid guid, const char* name)
{
    if (!_addMember(guid, name))
        return false;

    SendUpdate();

    if (Player* player = sObjectMgr.GetPlayer(guid))
    {
        if (!IsLeader(player->GetObjectGuid()) && !isBGGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN);
        }
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid membership
        if (isRaidGroup())
            player->UpdateForQuestWorldObjects();
    }

    return true;
}

uint32 Group::RemoveMember(ObjectGuid guid, uint8 method)
{
    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if (GetMembersCount() > uint32(isBGGroup() ? 1 : 2))    // in BG group case allow 1 members group
    {
        bool leaderChanged = _removeMember(guid);

        if (Player* player = sObjectMgr.GetPlayer(guid))
        {
            // quest related GO state dependent from raid membership
            if (isRaidGroup())
                player->UpdateForQuestWorldObjects();

            WorldPacket data;

            if (method == 1)
            {
                data.Initialize(SMSG_GROUP_UNINVITE, 0);
                player->GetSession()->SendPacket(&data);
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
                player->GetSession()->SendPacket(&data);
            }

            _homebindIfInstance(player);
        }

        if (leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_memberSlots.front().name.size() + 1));
            data << m_memberSlots.front().name;
            BroadcastPacket(&data, true);
        }

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
    BroadcastPacket(&data, true);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player* player;

    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if (!player)
            continue;

        // we cannot call _removeMember because it would invalidate member iterator
        // if we are removing player from battleground raid
        if (isBGGroup())
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
        if (isRaidGroup())
            player->UpdateForQuestWorldObjects();

        if (!player->GetSession())
            continue;

        WorldPacket data;
        if (!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
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
            player->GetSession()->SendPacket(&data);
        }

        _homebindIfInstance(player);
    }
    m_memberSlots.clear();

    RemoveAllInvites();

    if (!isBGGroup())
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId='%u'", m_Id);
        CharacterDatabase.CommitTransaction();
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, nullptr);
    }

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
    BroadcastPacket(&data, true);
}

static void GetDataForXPAtKill_helper(Player* player, Unit const* victim, uint32& sum_level, Player*& member_with_max_level, Player*& not_gray_member_with_max_level)
{
    sum_level += player->getLevel();
    if (!member_with_max_level || member_with_max_level->getLevel() < player->getLevel())
        member_with_max_level = player;

    uint32 gray_level = MaNGOS::XP::GetGrayLevel(player->getLevel());
    if (victim->getLevel() > gray_level && (!not_gray_member_with_max_level
                                            || not_gray_member_with_max_level->getLevel() < player->getLevel()))
        not_gray_member_with_max_level = player;
}

void Group::GetDataForXPAtKill(Unit const* victim, uint32& count, uint32& sum_level, Player*& member_with_max_level, Player*& not_gray_member_with_max_level, Player* additional)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->getSource();
        if (!member || !member->isAlive())                  // only for alive
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

void Group::SendTargetIconList(WorldSession* session)
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

    session->SendPacket(&data);
}

void Group::SendUpdate()
{
    Player* player;

    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if (!player || !player->GetSession() || player->GetGroup() != this)
            continue;
        // guess size
        WorldPacket data(SMSG_GROUP_LIST, (1 + 1 + 1 + 4 + GetMembersCount() * 20) + 8 + 1 + 8 + 1);
        data << (uint8)m_groupType;                         // group type
        data << (uint8)(citr->group | (citr->assistant ? 0x80 : 0)); // own flags (groupid | (assistant?0x80:0))

        data << uint32(GetMembersCount() - 1);
        for (member_citerator citr2 = m_memberSlots.begin(); citr2 != m_memberSlots.end(); ++citr2)
        {
            if (citr->guid == citr2->guid)
                continue;
            Player* member = sObjectMgr.GetPlayer(citr2->guid);
            uint8 onlineState = (member) ? MEMBER_STATUS_ONLINE : MEMBER_STATUS_OFFLINE;
            onlineState = onlineState | ((isBGGroup()) ? MEMBER_STATUS_PVP : 0);

            data << citr2->name;
            data << citr2->guid;
            // online-state
            data << uint8(sObjectMgr.GetPlayer(citr2->guid) ? 1 : 0);
            data << (uint8)(citr2->group | (citr2->assistant ? 0x80 : 0));
        }

        ObjectGuid masterLootGuid = (m_lootMethod == MASTER_LOOT) ? m_masterLooterGuid : ObjectGuid();
        data << m_leaderGuid;                               // leader guid
        if (GetMembersCount() - 1)
        {
            data << uint8(m_lootMethod);                    // loot method
            data << masterLootGuid;                         // master loot guid
            data << uint8(m_lootThreshold);                 // loot threshold
        }
        player->GetSession()->SendPacket(&data);
    }
}

void Group::UpdatePlayerOutOfRange(Player* pPlayer)
{
    if (!pPlayer || !pPlayer->IsInWorld())
        return;

    if (pPlayer->GetGroupUpdateFlag() == GROUP_UPDATE_FLAG_NONE)
        return;

    WorldPacket data;
    pPlayer->GetSession()->BuildPartyMemberStatsChangedPacket(pPlayer, &data);

    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
        if (Player* player = itr->getSource())
            if (player != pPlayer && !player->HaveAtClient(pPlayer))
                player->GetSession()->SendPacket(&data);
}

void Group::BroadcastPacket(WorldPacket* packet, bool ignorePlayersInBGRaid, int group, ObjectGuid ignore)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();
        if (!pl || (ignore && pl->GetObjectGuid() == ignore) || (ignorePlayersInBGRaid && pl->GetGroup() != this))
            continue;

        if (pl->GetSession() && (group == -1 || itr->getSubGroup() == group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket* packet)
{
    for (GroupReference* itr = GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* pl = itr->getSource();
        if (pl && pl->GetSession())
            if (IsLeader(pl->GetObjectGuid()) || IsAssistant(pl->GetObjectGuid()))
                pl->GetSession()->SendPacket(packet);
    }
}

void Group::OfflineReadyCheck()
{
    for (member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player* pl = sObjectMgr.GetPlayer(citr->guid);
        if (!pl || !pl->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
            data << citr->guid;
            data << uint8(0);
            BroadcastReadyCheck(&data);
        }
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

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(group);

    if (player)
    {
        player->SetGroupInvite(nullptr);
        // if player is in group and he is being added to BG raid group, then call SetBattleGroundRaid()
        if (player->GetGroup() && isBGGroup())
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
    }

    if (!isRaidGroup())                                     // reset targetIcons for non-raid-groups
    {
        for (int i = 0; i < TARGET_ICON_COUNT; ++i)
            m_targetIcons[i].Clear();
    }

    if (!isBGGroup())
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
        if (isBGGroup())
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

    if (!isBGGroup())
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid='%u'", guid.GetCounter());

    if (m_leaderGuid == guid)                               // leader was removed
    {
        if (GetMembersCount() > 0)
            _setLeader(m_memberSlots.front().guid);
        return true;
    }

    return false;
}

void Group::_setLeader(ObjectGuid guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if (slot == m_memberSlots.end())
        return;

    if (!isBGGroup())
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
        CharacterDatabase.PExecute("UPDATE groups SET leaderGuid='%u' WHERE groupId='%u'", slot_lowguid, m_Id);
        CharacterDatabase.CommitTransaction();
    }

    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
}

bool Group::_setMembersGroup(ObjectGuid guid, uint8 group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->group = group;

    SubGroupCounterIncrease(group);

    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET subgroup='%u' WHERE memberGuid='%u'", group, guid.GetCounter());

    return true;
}

bool Group::_setAssistantFlag(ObjectGuid guid, const bool& state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if (slot == m_memberSlots.end())
        return false;

    slot->assistant = state;
    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE group_member SET assistant='%u' WHERE memberGuid='%u'", (state == true) ? 1 : 0, guid.GetCounter());
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

    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainTank='%u' WHERE groupId='%u'", m_mainTankGuid.GetCounter(), m_Id);

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

    if (!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainAssistant='%u' WHERE groupId='%u'",
                                   m_mainAssistantGuid.GetCounter(), m_Id);

    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if (!member1 || !member2)
        return false;
    if (member1->GetGroup() != this || member2->GetGroup() != this)
        return false;
    else
        return member1->GetSubGroup() == member2->GetSubGroup();
}

// allows setting subgroup for offline members
void Group::ChangeMembersGroup(ObjectGuid guid, uint8 group)
{
    if (!isRaidGroup())
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
    if (!player || !isRaidGroup())
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

    BattleGroundBracketId bracket_id = reference->GetBattleGroundBracketIdFromLevel(bgTypeId);
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
        if (member->GetBattleGroundBracketIdFromLevel(bgTypeId) != bracket_id)
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
        if (pPlayer->getAttackers().size() && pPlayer->GetInstanceId() == instanceId)
            return true;
    }
    return false;
}

void Group::ResetInstances(InstanceResetMethod method, Player* SendMsgTo)
{
    if (isBGGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_GROUP_DISBAND

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

        bool isEmpty = true;
        // if the map is loaded, reset it
        if (Map* map = sMapMgr.FindMap(state->GetMapId(), state->GetInstanceId()))
            if (map->IsDungeon() && !(method == INSTANCE_RESET_GROUP_DISBAND && !state->CanReset()))
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
    else
        return nullptr;
}

InstanceGroupBind* Group::BindToInstance(DungeonPersistentState* state, bool permanent, bool load)
{
    if (state && !isBGGroup())
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
    else
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

void Group::_homebindIfInstance(Player* player)
{
    if (player && !player->isGameMaster())
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
    if (pGroupGuy->isAlive())
        pGroupGuy->RewardHonor(pVictim, count);

    // xp and reputation only in !PvP case
    if (!PvP)
    {
        float rate = group_rate * float(pGroupGuy->getLevel()) / sum_level;

        // if is in dungeon then all receive full reputation at kill
        // rewarded any alive/dead/near_corpse group member
        pGroupGuy->RewardReputation(pVictim, is_dungeon ? 1.0f : rate);

        // XP updated only for alive group member
        if (pGroupGuy->isAlive() && not_gray_member_with_max_level &&
                pGroupGuy->getLevel() <= not_gray_member_with_max_level->getLevel())
        {
            uint32 itr_xp = (member_with_max_level == not_gray_member_with_max_level) ? uint32(xp * rate) : uint32((xp * rate / 2) + 1);

            pGroupGuy->GiveXP(itr_xp, pVictim);
            if (Pet* pet = pGroupGuy->GetPet())
                // TODO: Pets need to get exp based on their level diff to the target, not the owners.
                // the whole RewardGroupAtKill needs a rewrite to match up with this anyways:
                // http://wowwiki.wikia.com/wiki/Formulas:Mob_XP?oldid=228414
                pet->GivePetXP(itr_xp / 2);
        }

        // quest objectives updated only for alive group member or dead but with not released body
        if (pGroupGuy->isAlive() || !pGroupGuy->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
        {
            // normal creature (not pet/etc) can be only in !PvP case
            if (pVictim->GetTypeId() == TYPEID_UNIT)
                pGroupGuy->KilledMonster(((Creature*)pVictim)->GetCreatureInfo(), pVictim->GetObjectGuid());
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
    bool PvP = pVictim->isCharmedOwnedByPlayerOrPlayer();

    // prepare data for near group iteration (PvP and !PvP cases)
    uint32 count = 0;
    uint32 sum_level = 0;
    Player* member_with_max_level = nullptr;
    Player* not_gray_member_with_max_level = nullptr;

    GetDataForXPAtKill(pVictim, count, sum_level, member_with_max_level, not_gray_member_with_max_level, player_tap);

    if (member_with_max_level)
    {
        /// not get Xp in PvP or no not gray players in group
        uint32 xp = (PvP || !not_gray_member_with_max_level) ? 0 : MaNGOS::XP::Gain(not_gray_member_with_max_level, pVictim);

        /// skip in check PvP case (for speed, not used)
        bool is_raid = PvP ? false : sMapStore.LookupEntry(pVictim->GetMapId())->IsRaid() && isRaidGroup();
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
