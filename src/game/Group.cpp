/*
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 * Copyright (C) 2009-2010 MaNGOSZero <http://github.com/mangoszero/mangoszero/>
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
#include "World.h"
#include "ObjectMgr.h"
#include "ObjectGuid.h"
#include "Group.h"
#include "Formulas.h"
#include "ObjectAccessor.h"
#include "BattleGround.h"
#include "MapManager.h"
#include "InstanceSaveMgr.h"
#include "MapInstanced.h"
#include "Util.h"

Group::Group() : m_Id(0), m_leaderGuid(0), m_mainTank(0), m_mainAssistant(0),  m_groupType(GROUPTYPE_NORMAL),
    m_bgGroup(NULL), m_lootMethod(FREE_FOR_ALL), m_looterGuid(0), m_lootThreshold(ITEM_QUALITY_UNCOMMON),
    m_subGroupsCounts(NULL)
{
    for (int i = 0; i < TARGETICONCOUNT; ++i)
        m_targetIcons[i] = 0;
}

Group::~Group()
{
    if(m_bgGroup)
    {
        sLog.outDebug("Group::~Group: battleground group being deleted.");
        if(m_bgGroup->GetBgRaid(ALLIANCE) == this) m_bgGroup->SetBgRaid(ALLIANCE, NULL);
        else if(m_bgGroup->GetBgRaid(HORDE) == this) m_bgGroup->SetBgRaid(HORDE, NULL);
        else sLog.outError("Group::~Group: battleground group is not linked to the correct battleground.");
    }
    Rolls::iterator itr;
    while(!RollId.empty())
    {
        itr = RollId.begin();
        Roll *r = *itr;
        RollId.erase(itr);
        delete(r);
    }

    // it is undefined whether objectmgr (which stores the groups) or instancesavemgr
    // will be unloaded first so we must be prepared for both cases
    // this may unload some instance saves
    for(BoundInstancesMap::iterator itr2 = m_boundInstances.begin(); itr2 != m_boundInstances.end(); ++itr2)
        itr2->second.save->RemoveGroup(this);

    // Sub group counters clean up
    if (m_subGroupsCounts)
        delete[] m_subGroupsCounts;
}

bool Group::Create(const uint64 &guid, const char * name)
{
    m_leaderGuid = guid;
    m_leaderName = name;

    m_groupType  = isBGGroup() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_lootMethod = GROUP_LOOT;
    m_lootThreshold = ITEM_QUALITY_UNCOMMON;
    m_looterGuid = guid;

    if(!isBGGroup())
    {
        m_Id = sObjectMgr.GenerateGroupId();

        Player *leader = sObjectMgr.GetPlayer(guid);

        Player::ConvertInstancesToGroup(leader, this, guid);

        // store group in database
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId ='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId ='%u'", m_Id);

        CharacterDatabase.PExecute("INSERT INTO groups(groupId,leaderGuid,mainTank,mainAssistant,lootMethod,looterGuid,lootThreshold,icon1,icon2,icon3,icon4,icon5,icon6,icon7,icon8,isRaid) "
            "VALUES('%u','%u','%u','%u','%u','%u','%u','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','" UI64FMTD "','%u')",
            m_Id, GUID_LOPART(m_leaderGuid), GUID_LOPART(m_mainTank), GUID_LOPART(m_mainAssistant), uint32(m_lootMethod),
            GUID_LOPART(m_looterGuid), uint32(m_lootThreshold), m_targetIcons[0], m_targetIcons[1], m_targetIcons[2], m_targetIcons[3], m_targetIcons[4], m_targetIcons[5], m_targetIcons[6], m_targetIcons[7], isRaidGroup());
    }

    if(!AddMember(guid, name))
        return false;

    if(!isBGGroup())
        CharacterDatabase.CommitTransaction();

    return true;
}

bool Group::LoadGroupFromDB(Field* fields)
{
    //                                          0         1              2           3           4              5      6      7      8      9      10     11     12     13      14          15
    // result = CharacterDatabase.Query("SELECT mainTank, mainAssistant, lootMethod, looterGuid, lootThreshold, icon1, icon2, icon3, icon4, icon5, icon6, icon7, icon8, isRaid, leaderGuid, groupId FROM groups");

    m_Id = fields[15].GetUInt32();
    m_leaderGuid = MAKE_NEW_GUID(fields[14].GetUInt32(),0,HIGHGUID_PLAYER);

    // group leader not exist
    if(!sObjectMgr.GetPlayerNameByGUID(m_leaderGuid, m_leaderName))
        return false;

    m_groupType  = fields[13].GetBool() ? GROUPTYPE_RAID : GROUPTYPE_NORMAL;

    if (m_groupType == GROUPTYPE_RAID)
        _initRaidSubGroupsCounter();

    m_mainTank = fields[0].GetUInt64();
    m_mainAssistant = fields[1].GetUInt64();
    m_lootMethod = (LootMethod)fields[2].GetUInt8();
    m_looterGuid = MAKE_NEW_GUID(fields[3].GetUInt32(), 0, HIGHGUID_PLAYER);
    m_lootThreshold = (ItemQualities)fields[4].GetUInt16();

    for(int i=0; i<TARGETICONCOUNT; ++i)
        m_targetIcons[i] = fields[5+i].GetUInt64();

    return true;
}

bool Group::LoadMemberFromDB(uint32 guidLow, uint8 subgroup, bool assistant)
{
    MemberSlot member;
    member.guid      = MAKE_NEW_GUID(guidLow, 0, HIGHGUID_PLAYER);

    // skip non-existed member
    if(!sObjectMgr.GetPlayerNameByGUID(member.guid, member.name))
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

    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET isRaid = 1 WHERE groupId='%u'", m_Id);
    SendUpdate();

    // update quest related GO states (quest activity dependent from raid membership)
    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
        if(Player* player = sObjectMgr.GetPlayer(citr->guid))
            player->UpdateForQuestsGO();
}

bool Group::AddInvite(Player *player)
{
    if(!player || player->GetGroupInvite() || player->GetGroup())
        return false;

    RemoveInvite(player);

    m_invitees.insert(player);

    player->SetGroupInvite(this);

    return true;
}

bool Group::AddLeaderInvite(Player *player)
{
    if(!AddInvite(player))
        return false;

    m_leaderGuid = player->GetGUID();
    m_leaderName = player->GetName();
    return true;
}

uint32 Group::RemoveInvite(Player *player)
{
    m_invitees.erase(player);

    player->SetGroupInvite(NULL);
    return GetMembersCount();
}

void Group::RemoveAllInvites()
{
    for(InvitesList::iterator itr=m_invitees.begin(); itr!=m_invitees.end(); ++itr)
        (*itr)->SetGroupInvite(NULL);

    m_invitees.clear();
}

Player* Group::GetInvited(const uint64& guid) const
{
    for(InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if((*itr)->GetGUID() == guid)
            return (*itr);
    }
    return NULL;
}

Player* Group::GetInvited(const std::string& name) const
{
    for(InvitesList::const_iterator itr = m_invitees.begin(); itr != m_invitees.end(); ++itr)
    {
        if((*itr)->GetName() == name)
            return (*itr);
    }
    return NULL;
}

bool Group::AddMember(const uint64 &guid, const char* name)
{
    if(!_addMember(guid, name))
        return false;
    SendUpdate();

    Player *player = sObjectMgr.GetPlayer(guid);
    if(player)
    {
        if(!IsLeader(player->GetGUID()) && !isBGGroup())
        {
            // reset the new member's instances, unless he is currently in one of them
            // including raid instances that they are not permanently bound to!
            player->ResetInstances(INSTANCE_RESET_GROUP_JOIN);
        }
        player->SetGroupUpdateFlag(GROUP_UPDATE_FULL);
        UpdatePlayerOutOfRange(player);

        // quest related GO state dependent from raid memebership
        if(isRaidGroup())
            player->UpdateForQuestsGO();
    }

    return true;
}

uint32 Group::RemoveMember(const uint64 &guid, const uint8 &method)
{
    // remove member and change leader (if need) only if strong more 2 members _before_ member remove
    if(GetMembersCount() > uint32(isBGGroup() ? 1 : 2))           // in BG group case allow 1 members group
    {
        bool leaderChanged = _removeMember(guid);

        if(Player *player = sObjectMgr.GetPlayer( guid ))
        {
            // quest related GO state dependent from raid membership
            if(isRaidGroup())
                player->UpdateForQuestsGO();

            WorldPacket data;

            if(method == 1)
            {
                data.Initialize( SMSG_GROUP_UNINVITE, 0 );
                player->GetSession()->SendPacket( &data );
            }

            data.Initialize(SMSG_GROUP_LIST, 24);
            data << uint64(0) << uint64(0) << uint64(0);
            player->GetSession()->SendPacket(&data);

            _homebindIfInstance(player);
        }

        if(leaderChanged)
        {
            WorldPacket data(SMSG_GROUP_SET_LEADER, (m_memberSlots.front().name.size()+1));
            data << m_memberSlots.front().name;
            BroadcastPacket(&data);
        }

        SendUpdate();
    }
    // if group before remove <= 2 disband it
    else
        Disband(true);

    return m_memberSlots.size();
}

void Group::ChangeLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);

    if(slot==m_memberSlots.end())
        return;

    _setLeader(guid);

    WorldPacket data(SMSG_GROUP_SET_LEADER, slot->name.size()+1);
    data << slot->name;
    BroadcastPacket(&data);
    SendUpdate();
}

void Group::Disband(bool hideDestroy)
{
    Player *player;

    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if(!player)
            continue;

        player->SetGroup(NULL);

        // quest related GO state dependent from raid membership
        if(isRaidGroup())
            player->UpdateForQuestsGO();


        if(!player->GetSession())
            continue;

        WorldPacket data;
        if(!hideDestroy)
        {
            data.Initialize(SMSG_GROUP_DESTROYED, 0);
            player->GetSession()->SendPacket(&data);
        }

        data.Initialize(SMSG_GROUP_LIST, 24);
        data << uint64(0) << uint64(0) << uint64(0);
        player->GetSession()->SendPacket(&data);

        _homebindIfInstance(player);
    }
    RollId.clear();
    m_memberSlots.clear();

    RemoveAllInvites();

    if(!isBGGroup())
    {
        CharacterDatabase.BeginTransaction();
        CharacterDatabase.PExecute("DELETE FROM groups WHERE groupId='%u'", m_Id);
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE groupId='%u'", m_Id);
        CharacterDatabase.CommitTransaction();
        ResetInstances(INSTANCE_RESET_GROUP_DISBAND, NULL);
    }

    m_leaderGuid = 0;
    m_leaderName = "";
}

/*********************************************************/
/***                   LOOT SYSTEM                     ***/
/*********************************************************/

void Group::SendLootStartRoll(uint32 CountDown, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_START_ROLL, (8+4+4+4+4+4));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // randomSuffix - not used ?
    data << uint32(r.itemRandomPropId);                     // item random property ID
    data << uint32(CountDown);                              // the countdown time to choose "need" or "greed"

    for (Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr!=r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRoll(ObjectGuid const& targetGuid, uint8 rollNumber, uint8 rollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL, (8+4+8+4+4+4+1+1));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // unknown, maybe amount of players, or item slot in loot
    data << targetGuid;
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // randomSuffix - not used?
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint8(rollNumber);                              // 0: "Need for: [item name]" > 127: "you passed on: [item name]"      Roll number
    data << uint8(rollType);                                // 0: "Need for: [item name]" 0: "You have selected need for [item name] 1: need roll 2: greed roll

    for( Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr!=r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootRollWon(ObjectGuid const& targetGuid, uint8 rollNumber, uint8 rollType, const Roll &r)
{
    WorldPacket data(SMSG_LOOT_ROLL_WON, (8+4+4+4+4+8+1+1));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // the itemEntryId for the item that shall be rolled for
    data << uint32(0);                                      // randomSuffix - not used ?
    data << uint32(r.itemRandomPropId);                     // Item random property
    data << targetGuid;                                     // guid of the player who won.
    data << uint8(rollNumber);                              // rollnumber realted to SMSG_LOOT_ROLL
    data << uint8(rollType);                                // Rolltype related to SMSG_LOOT_ROLL

    for( Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr!=r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::SendLootAllPassed(Roll const& r)
{
    WorldPacket data(SMSG_LOOT_ALL_PASSED, (8+4+4+4+4));
    data << r.lootedTargetGUID;                             // creature guid what we're looting
    data << uint32(r.itemSlot);                             // item slot in loot
    data << uint32(r.itemid);                               // The itemEntryId for the item that shall be rolled for
    data << uint32(r.itemRandomPropId);                     // Item random property ID
    data << uint32(0);                                      // Item random suffix ID - not used ?

    for( Roll::PlayerVote::const_iterator itr=r.playerVote.begin(); itr!=r.playerVote.end(); ++itr)
    {
        Player *p = sObjectMgr.GetPlayer(itr->first);
        if(!p || !p->GetSession())
            continue;

        if(itr->second != NOT_VALID)
            p->GetSession()->SendPacket( &data );
    }
}

void Group::GroupLoot(ObjectGuid const& playerGUID, Loot *loot, Creature *creature)
{
    std::vector<LootItem>::iterator i;
    ItemPrototype const *item;
    uint8 itemSlot = 0;
    Player *player = sObjectMgr.GetPlayer(playerGUID);
    Group *group = player->GetGroup();

    for (i=loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        item = ObjectMgr::GetItemPrototype(i->itemid);
        if (!item)
        {
            //sLog.outDebug("Group::GroupLoot: missing item prototype for item with id: %d", i->itemid);
            continue;
        }

        //roll for over-threshold item if it's one-player loot
        if (item->Quality >= uint32(m_lootThreshold) && !i->freeforall)
        {
            Roll* r=new Roll(creature->GetGUID(),*i);

            //a vector is filled with only near party members
            for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *member = itr->getSource();
                if(!member || !member->GetSession())
                    continue;
                if ( i->AllowedForPlayer(member) )
                {
                    if (member->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                    {
                        r->playerVote[member->GetGUID()] = NOT_EMITED_YET;
                        ++r->totalPlayersRolling;
                    }
                }
            }

            if (r->totalPlayersRolling > 0)                 // has looters
            {
                r->setLoot(loot);
                r->itemSlot = itemSlot;

                if (r->totalPlayersRolling == 1)            // single looter
                    r->playerVote.begin()->second = NEED;
                else
                {
                    group->SendLootStartRoll(60000, *r);

                    loot->items[itemSlot].is_blocked = true;

                    creature->m_groupLootTimer = 60000;
                    creature->m_groupLootId = GetId();
                }

                RollId.push_back(r);
            }
            else                                            // no looters??
                delete r;
        }
        else
            i->is_underthreshold=1;

    }
}

void Group::NeedBeforeGreed(ObjectGuid const& playerGUID, Loot *loot, Creature *creature)
{
    ItemPrototype const *item;
    Player *player = sObjectMgr.GetPlayer(playerGUID);
    Group *group = player->GetGroup();

    uint8 itemSlot = 0;
    for(std::vector<LootItem>::iterator i=loot->items.begin(); i != loot->items.end(); ++i, ++itemSlot)
    {
        item = ObjectMgr::GetItemPrototype(i->itemid);

        //only roll for one-player items, not for ones everyone can get
        if (item->Quality >= uint32(m_lootThreshold) && !i->freeforall)
        {
            Roll* r=new Roll(creature->GetGUID(),*i);

            for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
            {
                Player *playerToRoll = itr->getSource();
                if(!playerToRoll || !playerToRoll->GetSession())
                    continue;

                if (playerToRoll->CanUseItem(item) && i->AllowedForPlayer(playerToRoll) )
                {
                    if (playerToRoll->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                    {
                        r->playerVote[playerToRoll->GetGUID()] = NOT_EMITED_YET;
                        ++r->totalPlayersRolling;
                    }
                }
            }

            if (r->totalPlayersRolling > 0)                 // has looters
            {
                r->setLoot(loot);
                r->itemSlot = itemSlot;

                if (r->totalPlayersRolling == 1)            // single looter
                    r->playerVote.begin()->second = NEED;
                else
                {
                    group->SendLootStartRoll(60000, *r);
                    loot->items[itemSlot].is_blocked = true;
                }

                RollId.push_back(r);
            }
            else                                            // no looters??
                delete r;
        }
        else
            i->is_underthreshold=1;
    }
}

void Group::MasterLoot(ObjectGuid const& playerGUID, Loot* /*loot*/, Creature *creature)
{
    Player *player = sObjectMgr.GetPlayer(playerGUID);
    if(!player)
        return;

    sLog.outDebug("Group::MasterLoot (SMSG_LOOT_MASTER_LIST, 330) player = [%s].", player->GetName());

    uint32 real_count = 0;

    WorldPacket data(SMSG_LOOT_MASTER_LIST, 330);
    data << (uint8)GetMembersCount();

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (!looter->IsInWorld())
            continue;

        if (looter->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
        {
            data << looter->GetGUID();
            ++real_count;
        }
    }

    data.put<uint8>(0,real_count);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *looter = itr->getSource();
        if (looter->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            looter->GetSession()->SendPacket(&data);
    }
}

void Group::CountRollVote(ObjectGuid const& playerGUID, ObjectGuid const& lootedTarget, uint32 itemSlot, uint8 choise)
{
    Rolls::iterator rollI = RollId.begin();
    for (; rollI != RollId.end(); ++rollI)
        if ((*rollI)->isValid() && (*rollI)->lootedTargetGUID == lootedTarget && (*rollI)->itemSlot == itemSlot)
            break;

    if (rollI == RollId.end())
        return;

    CountRollVote(playerGUID, rollI, choise);
}

bool Group::CountRollVote(ObjectGuid const& playerGUID, Rolls::iterator& rollI, uint8 choise)
{
    Roll* roll = *rollI;

    Roll::PlayerVote::iterator itr = roll->playerVote.find(playerGUID.GetRawValue());
    // this condition means that player joins to the party after roll begins
    if (itr == roll->playerVote.end())
        return true;                                        // result used for need iterator ++, so avoid for end of list

    if (roll->getLoot())
        if (roll->getLoot()->items.empty())
            return false;

    switch (choise)
    {
        case 0:                                             //Player choose pass
        {
            SendLootRoll(playerGUID, 128, 128, *roll);
            ++roll->totalPass;
            itr->second = PASS;
        }
        break;
        case 1:                                             //player choose Need
        {
            SendLootRoll(playerGUID, 0, 0, *roll);
            ++roll->totalNeed;
            itr->second = NEED;
        }
        break;
        case 2:                                             //player choose Greed
        {
            SendLootRoll(playerGUID, 128, 2, *roll);
            ++roll->totalGreed;
            itr->second = GREED;
        }
        break;
    }
    if (roll->totalPass + roll->totalGreed + roll->totalNeed >= roll->totalPlayersRolling)
    {
        CountTheRoll(rollI);
        return true;
    }

    return false;
}

//called when roll timer expires
void Group::EndRoll()
{
    while(!RollId.empty())
    {
        //need more testing here, if rolls disappear
        Rolls::iterator itr = RollId.begin();
        CountTheRoll(itr);                                  //i don't have to edit player votes, who didn't vote ... he will pass
    }
}

void Group::CountTheRoll(Rolls::iterator& rollI)
{
    Roll* roll = *rollI;
    if(!roll->isValid())                                    // is loot already deleted ?
    {
        rollI = RollId.erase(rollI);
        delete roll;
        return;
    }

    //end of the roll
    if (roll->totalNeed > 0)
    {
        if(!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            ObjectGuid maxguid  = (*roll->playerVote.begin()).first;
            Player *player;

            for( Roll::PlayerVote::const_iterator itr=roll->playerVote.begin(); itr!=roll->playerVote.end(); ++itr)
            {
                if (itr->second != NEED)
                    continue;

                uint8 randomN = urand(1, 99);
                SendLootRoll(itr->first, randomN, 1, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(maxguid, maxresul, 1, *roll);
            player = sObjectMgr.GetPlayer(maxguid);

            if(player && player->GetSession())
            {
                ItemPosCountVec dest;
                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
                uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count );
                if ( msg == EQUIP_ERR_OK )
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;
                    player->StoreNewItem( dest, roll->itemid, true, item->randomPropertyId);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError( msg, NULL, NULL );
                }
            }
        }
    }
    else if (roll->totalGreed > 0)
    {
        if(!roll->playerVote.empty())
        {
            uint8 maxresul = 0;
            uint64 maxguid = (*roll->playerVote.begin()).first;
            Player *player;

            Roll::PlayerVote::iterator itr;
            for (itr=roll->playerVote.begin(); itr!=roll->playerVote.end(); ++itr)
            {
                if (itr->second != GREED)
                    continue;

                uint8 randomN = urand(1, 99);
                SendLootRoll(itr->first, randomN, 2, *roll);
                if (maxresul < randomN)
                {
                    maxguid  = itr->first;
                    maxresul = randomN;
                }
            }
            SendLootRollWon(maxguid, maxresul, 2, *roll);
            player = sObjectMgr.GetPlayer(maxguid);

            if(player && player->GetSession())
            {
                ItemPosCountVec dest;
                LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
                uint8 msg = player->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, roll->itemid, item->count );
                if ( msg == EQUIP_ERR_OK )
                {
                    item->is_looted = true;
                    roll->getLoot()->NotifyItemRemoved(roll->itemSlot);
                    --roll->getLoot()->unlootedCount;
                    player->StoreNewItem( dest, roll->itemid, true, item->randomPropertyId);
                }
                else
                {
                    item->is_blocked = false;
                    player->SendEquipError( msg, NULL, NULL );
                }
            }
        }
    }
    else
    {
        SendLootAllPassed(*roll);
        LootItem *item = &(roll->getLoot()->items[roll->itemSlot]);
        if(item) item->is_blocked = false;
    }
    rollI = RollId.erase(rollI);
    delete roll;
}

void Group::SetTargetIcon(uint8 id, uint64 guid)
{
    if(id >= TARGETICONCOUNT)
        return;

    // clean other icons
    if( guid != 0 )
        for(int i=0; i<TARGETICONCOUNT; ++i)
            if( m_targetIcons[i] == guid )
                SetTargetIcon(i, 0);

    m_targetIcons[id] = guid;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (2+8));
    data << (uint8)0;
    data << id;
    data << guid;
    BroadcastPacket(&data);
}

void Group::GetDataForXPAtKill(Unit const* victim, uint32& count,uint32& sum_level, Player* & member_with_max_level, Player* & not_gray_member_with_max_level)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player* member = itr->getSource();
        if(!member || !member->isAlive())                   // only for alive
            continue;

        if(!member->IsAtGroupRewardDistance(victim))        // at req. distance
            continue;

        ++count;
        sum_level += member->getLevel();
        if(!member_with_max_level || member_with_max_level->getLevel() < member->getLevel())
            member_with_max_level = member;

        uint32 gray_level = MaNGOS::XP::GetGrayLevel(member->getLevel());
        if( victim->getLevel() > gray_level && (!not_gray_member_with_max_level
           || not_gray_member_with_max_level->getLevel() < member->getLevel()))
            not_gray_member_with_max_level = member;
    }
}

void Group::SendTargetIconList(WorldSession *session)
{
    if(!session)
        return;

    WorldPacket data(MSG_RAID_TARGET_UPDATE, (1+TARGETICONCOUNT*9));
    data << (uint8)1;

    for(int i=0; i<TARGETICONCOUNT; ++i)
    {
        if(m_targetIcons[i] == 0)
            continue;

        data << (uint8)i;
        data << m_targetIcons[i];
    }

    session->SendPacket(&data);
}

void Group::SendUpdate()
{
    Player *player;

    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        player = sObjectMgr.GetPlayer(citr->guid);
        if(!player || !player->GetSession())
            continue;
                                                            // guess size
        WorldPacket data(SMSG_GROUP_LIST, (1+1+1+4+GetMembersCount()*20)+8+1+8+1);
        data << (uint8)m_groupType;                         // group type
        data << (uint8)(citr->group | (citr->assistant?0x80:0));  // own flags (groupid | (assistant?0x80:0))

        data << uint32(GetMembersCount()-1);
        for(member_citerator citr2 = m_memberSlots.begin(); citr2 != m_memberSlots.end(); ++citr2)
        {
            if(citr->guid == citr2->guid)
                continue;

            data << citr2->name;
            data << (uint64)citr2->guid;
                                                            // online-state
            data << (uint8)(sObjectMgr.GetPlayer(citr2->guid) ? 1 : 0);
            data << (uint8)(citr2->group | (citr2->assistant?0x80:0));
        }

        data << uint64(m_leaderGuid);                       // leader guid
        if(GetMembersCount()-1)
        {
            data << (uint8)m_lootMethod;                    // loot method
            data << (uint64)m_looterGuid;                   // looter guid
            data << (uint8)m_lootThreshold;                 // loot threshold
        }
        player->GetSession()->SendPacket( &data );
    }
}

void Group::UpdatePlayerOutOfRange(Player* pPlayer)
{
    if(!pPlayer || !pPlayer->IsInWorld())
        return;

    Player *player;
    WorldPacket data;
    pPlayer->GetSession()->BuildPartyMemberStatsChangedPacket(pPlayer, &data);

    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        player = itr->getSource();
        if (player && player != pPlayer && !pPlayer->isVisibleFor(player,player->GetViewPoint()))
            player->GetSession()->SendPacket(&data);
    }
}

void Group::BroadcastPacket(WorldPacket *packet, int group, uint64 ignore)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if(!pl || (ignore != 0 && pl->GetGUID() == ignore))
            continue;

        if (pl->GetSession() && (group==-1 || itr->getSubGroup()==group))
            pl->GetSession()->SendPacket(packet);
    }
}

void Group::BroadcastReadyCheck(WorldPacket *packet)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pl = itr->getSource();
        if(pl && pl->GetSession())
            if(IsLeader(pl->GetGUID()) || IsAssistant(pl->GetGUID()))
                pl->GetSession()->SendPacket(packet);
    }
}

void Group::OfflineReadyCheck()
{
    for(member_citerator citr = m_memberSlots.begin(); citr != m_memberSlots.end(); ++citr)
    {
        Player *pl = sObjectMgr.GetPlayer(citr->guid);
        if (!pl || !pl->GetSession())
        {
            WorldPacket data(MSG_RAID_READY_CHECK_CONFIRM, 9);
            data << citr->guid;
            data << (uint8)0;
            BroadcastReadyCheck(&data);
        }
    }
}

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant)
{
    // get first not-full group
    uint8 groupid = 0;
    if (m_subGroupsCounts)
    {
        bool groupFound = false;
        for (; groupid < MAXRAIDSIZE/MAXGROUPSIZE; ++groupid)
        {
            if (m_subGroupsCounts[groupid] < MAXGROUPSIZE)
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

bool Group::_addMember(const uint64 &guid, const char* name, bool isAssistant, uint8 group)
{
    if(IsFull())
        return false;

    if(!guid)
        return false;

    Player *player = sObjectMgr.GetPlayer(guid);

    MemberSlot member;
    member.guid      = guid;
    member.name      = name;
    member.group     = group;
    member.assistant = isAssistant;
    m_memberSlots.push_back(member);

    SubGroupCounterIncrease(group);

    if(player)
    {
        player->SetGroupInvite(NULL);
        player->SetGroup(this, group);
        // if the same group invites the player back, cancel the homebind timer
        InstanceGroupBind *bind = GetBoundInstance(player->GetMapId());
        if(bind && bind->save->GetInstanceId() == player->GetInstanceId())
            player->m_InstanceValid = true;
    }

    if(!isRaidGroup())                                      // reset targetIcons for non-raid-groups
    {
        for(int i=0; i<TARGETICONCOUNT; ++i)
            m_targetIcons[i] = 0;
    }

    if(!isBGGroup())
    {
        // insert into group table
        CharacterDatabase.PExecute("INSERT INTO group_member(groupId,memberGuid,assistant,subgroup) VALUES('%u','%u','%u','%u')", m_Id, GUID_LOPART(member.guid), ((member.assistant==1)?1:0), member.group);
    }

    return true;
}

bool Group::_removeMember(const uint64 &guid)
{
    Player *player = sObjectMgr.GetPlayer(guid);
    if (player)
    {
        player->SetGroup(NULL);
    }

    _removeRolls(guid);

    member_witerator slot = _getMemberWSlot(guid);
    if (slot != m_memberSlots.end())
    {
        SubGroupCounterDecrease(slot->group);

        m_memberSlots.erase(slot);
    }

    if(!isBGGroup())
        CharacterDatabase.PExecute("DELETE FROM group_member WHERE memberGuid='%u'", GUID_LOPART(guid));

    if(m_leaderGuid == guid)                                // leader was removed
    {
        if(GetMembersCount() > 0)
            _setLeader(m_memberSlots.front().guid);
        return true;
    }

    return false;
}

void Group::_setLeader(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot==m_memberSlots.end())
        return;

    if(!isBGGroup())
    {
        // TODO: set a time limit to have this function run rarely cause it can be slow
        CharacterDatabase.BeginTransaction();

        // update the group's bound instances when changing leaders

        // remove all permanent binds from the group
        // in the DB also remove solo binds that will be replaced with permbinds
        // from the new leader
        CharacterDatabase.PExecute(
            "DELETE FROM group_instance WHERE leaderguid='%u' AND (permanent = 1 OR "
            "instance IN (SELECT instance FROM character_instance WHERE guid = '%u')"
            ")", GUID_LOPART(m_leaderGuid), GUID_LOPART(slot->guid)
        );

        Player *player = sObjectMgr.GetPlayer(slot->guid);
        if(player)
        {
            for(BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
            {
                if(itr->second.perm)
                {
                    itr->second.save->RemoveGroup(this);
                    m_boundInstances.erase(itr++);
                }
                else
                    ++itr;
            }
        }

        // update the group's solo binds to the new leader
        CharacterDatabase.PExecute("UPDATE group_instance SET leaderGuid='%u' WHERE leaderGuid = '%u'", GUID_LOPART(slot->guid), GUID_LOPART(m_leaderGuid));

        // copy the permanent binds from the new leader to the group
        // overwriting the solo binds with permanent ones if necessary
        // in the DB those have been deleted already
        Player::ConvertInstancesToGroup(player, this, slot->guid);

        // update the group leader
        CharacterDatabase.PExecute("UPDATE groups SET leaderGuid='%u' WHERE groupId='%u'", GUID_LOPART(slot->guid), m_Id);
        CharacterDatabase.CommitTransaction();
    }

    m_leaderGuid = slot->guid;
    m_leaderName = slot->name;
}

void Group::_removeRolls(const uint64 &guid)
{
    for (Rolls::iterator it = RollId.begin(); it != RollId.end(); )
    {
        Roll* roll = *it;
        Roll::PlayerVote::iterator itr2 = roll->playerVote.find(guid);
        if(itr2 == roll->playerVote.end())
        {
            ++it;
            continue;
        }

        if (itr2->second == GREED) --roll->totalGreed;
        if (itr2->second == NEED) --roll->totalNeed;
        if (itr2->second == PASS) --roll->totalPass;
        if (itr2->second != NOT_VALID) --roll->totalPlayersRolling;

        roll->playerVote.erase(itr2);

        if (!CountRollVote(guid, it, 3))
            ++it;
    }
}

bool Group::_setMembersGroup(const uint64 &guid, const uint8 &group)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_memberSlots.end())
        return false;

    slot->group = group;

    SubGroupCounterIncrease(group);

    if(!isBGGroup()) CharacterDatabase.PExecute("UPDATE group_member SET subgroup='%u' WHERE memberGuid='%u'", group, GUID_LOPART(guid));

    return true;
}

bool Group::_setAssistantFlag(const uint64 &guid, const bool &state)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_memberSlots.end())
        return false;

    slot->assistant = state;
    if(!isBGGroup()) CharacterDatabase.PExecute("UPDATE group_member SET assistant='%u' WHERE memberGuid='%u'", (state==true)?1:0, GUID_LOPART(guid));
    return true;
}

bool Group::_setMainTank(const uint64 &guid)
{
    member_citerator slot = _getMemberCSlot(guid);
    if(slot==m_memberSlots.end())
        return false;

    if(m_mainAssistant == guid)
        _setMainAssistant(0);
    m_mainTank = guid;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainTank='%u' WHERE groupId='%u'", GUID_LOPART(m_mainTank), m_Id);
    return true;
}

bool Group::_setMainAssistant(const uint64 &guid)
{
    member_witerator slot = _getMemberWSlot(guid);
    if(slot==m_memberSlots.end())
        return false;

    if(m_mainTank == guid)
        _setMainTank(0);
    m_mainAssistant = guid;
    if(!isBGGroup())
        CharacterDatabase.PExecute("UPDATE groups SET mainAssistant='%u' WHERE groupId='%u'", GUID_LOPART(m_mainAssistant), m_Id);
    return true;
}

bool Group::SameSubGroup(Player const* member1, Player const* member2) const
{
    if(!member1 || !member2) return false;
    if (member1->GetGroup() != this || member2->GetGroup() != this) return false;
    else return member1->GetSubGroup() == member2->GetSubGroup();
}

// allows setting subgroup for offline members
void Group::ChangeMembersGroup(const uint64 &guid, const uint8 &group)
{
    if(!isRaidGroup())
        return;
    Player *player = sObjectMgr.GetPlayer(guid);

    if (!player)
    {
        uint8 prevSubGroup;
        prevSubGroup = GetMemberGroup(guid);

        SubGroupCounterDecrease(prevSubGroup);

        if(_setMembersGroup(guid, group))
            SendUpdate();
    }
    else
        // This methods handles itself groupcounter decrease
        ChangeMembersGroup(player, group);
}

// only for online members
void Group::ChangeMembersGroup(Player *player, const uint8 &group)
{
    if(!player || !isRaidGroup())
        return;
    if(_setMembersGroup(player->GetGUID(), group))
    {
        uint8 prevSubGroup;
        prevSubGroup = player->GetSubGroup();

        SubGroupCounterDecrease(prevSubGroup);

        player->GetGroupRef().setSubGroup(group);
        SendUpdate();
    }
}

void Group::UpdateLooterGuid( Creature* creature, bool ifneed )
{
    switch (GetLootMethod())
    {
        case MASTER_LOOT:
        case FREE_FOR_ALL:
            return;
        default:
            // round robin style looting applies for all low
            // quality items in each loot method except free for all and master loot
            break;
    }

    member_citerator guid_itr = _getMemberCSlot(GetLooterGuid());
    if(guid_itr != m_memberSlots.end())
    {
        if(ifneed)
        {
            // not update if only update if need and ok
            Player* looter = ObjectAccessor::FindPlayer(guid_itr->guid);
            if(looter && looter->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                return;
        }
        ++guid_itr;
    }

    // search next after current
    if(guid_itr != m_memberSlots.end())
    {
        for(member_citerator itr = guid_itr; itr != m_memberSlots.end(); ++itr)
        {
            if(Player* pl = ObjectAccessor::FindPlayer(itr->guid))
            {
                if (pl->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
                {
                    bool refresh = pl->GetLootGUID()==creature->GetGUID();

                    //if(refresh)                             // update loot for new looter
                    //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                    SetLooterGuid(pl->GetGUID());
                    SendUpdate();
                    if(refresh)                             // update loot for new looter
                        pl->SendLoot(creature->GetGUID(),LOOT_CORPSE);
                    return;
                }
            }
        }
    }

    // search from start
    for(member_citerator itr = m_memberSlots.begin(); itr != guid_itr; ++itr)
    {
        if(Player* pl = ObjectAccessor::FindPlayer(itr->guid))
        {
            if (pl->IsWithinDist(creature, sWorld.getConfig(CONFIG_FLOAT_GROUP_XP_DISTANCE), false))
            {
                bool refresh = pl->GetLootGUID()==creature->GetGUID();

                //if(refresh)                               // update loot for new looter
                //    pl->GetSession()->DoLootRelease(pl->GetLootGUID());
                SetLooterGuid(pl->GetGUID());
                SendUpdate();
                if(refresh)                                 // update loot for new looter
                    pl->SendLoot(creature->GetGUID(),LOOT_CORPSE);
                return;
            }
        }
    }

    SetLooterGuid(0);
    SendUpdate();
}

uint32 Group::CanJoinBattleGroundQueue(BattleGroundTypeId bgTypeId, uint32 bgQueueType, uint32 MinPlayerCount, uint32 MaxPlayerCount, uint32 arenaSlot)
{
    // check for min / max count
    uint32 memberscount = GetMembersCount();
    if(memberscount < MinPlayerCount)
        return BG_JOIN_ERR_GROUP_NOT_ENOUGH;
    if(memberscount > MaxPlayerCount)
        return BG_JOIN_ERR_GROUP_TOO_MANY;

    // get a player as reference, to compare other players' stats to (arena team id, queue id based on level, etc.)
    Player * reference = GetFirstMember()->getSource();
    // no reference found, can't join this way
    if(!reference)
        return BG_JOIN_ERR_OFFLINE_MEMBER;

    BattleGroundBracketId bracket_id = reference->GetBattleGroundBracketIdFromLevel();
    uint32 team = reference->GetTeam();

    // check every member of the group to be able to join
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *member = itr->getSource();
        // offline member? don't let join
        if(!member)
            return BG_JOIN_ERR_OFFLINE_MEMBER;
        // don't allow cross-faction join as group
        if(member->GetTeam() != team)
            return BG_JOIN_ERR_MIXED_FACTION;
        // not in the same battleground level bracket, don't let join
        if(member->GetBattleGroundBracketIdFromLevel() != bracket_id)
            return BG_JOIN_ERR_MIXED_LEVELS;
        // don't let join if someone from the group is already in that bg queue
        if(member->InBattleGroundQueueForBattleGroundQueueType(bgQueueType))
            return BG_JOIN_ERR_GROUP_MEMBER_ALREADY_IN_QUEUE;
        // check for deserter debuff in case not arena queue
        if(!member->CanJoinToBattleground())
            return BG_JOIN_ERR_GROUP_DESERTER;
        // check if member can join any more battleground queues
        if(!member->HasFreeBattleGroundQueueId())
            return BG_JOIN_ERR_ALL_QUEUES_USED;
    }
    return BG_JOIN_ERR_OK;
}

//===================================================
//============== Roll ===============================
//===================================================

void Roll::targetObjectBuildLink()
{
    // called from link()
    getTarget()->addLootValidatorRef(this);
}

bool Group::InCombatToInstance(uint32 instanceId)
{
    for(GroupReference *itr = GetFirstMember(); itr != NULL; itr = itr->next())
    {
        Player *pPlayer = itr->getSource();
        if(pPlayer->getAttackers().size() && pPlayer->GetInstanceId() == instanceId)
            return true;
    }
    return false;
}

void Group::ResetInstances(uint8 method, Player* SendMsgTo)
{
    if(isBGGroup())
        return;

    // method can be INSTANCE_RESET_ALL, INSTANCE_RESET_GROUP_DISBAND

    for(BoundInstancesMap::iterator itr = m_boundInstances.begin(); itr != m_boundInstances.end();)
    {
        InstanceSave *p = itr->second.save;
        const MapEntry *entry = sMapStore.LookupEntry(itr->first);
        if (!entry || (!p->CanReset() && method != INSTANCE_RESET_GROUP_DISBAND))
        {
            ++itr;
            continue;
        }

        if(method == INSTANCE_RESET_ALL)
        {
            // the "reset all instances" method can only reset normal maps
            if(entry->map_type == MAP_RAID)
            {
                ++itr;
                continue;
            }
        }

        bool isEmpty = true;
        // if the map is loaded, reset it
        Map *map = sMapMgr.FindMap(p->GetMapId(), p->GetInstanceId());
        if(map && map->IsDungeon() && !(method == INSTANCE_RESET_GROUP_DISBAND && !p->CanReset()))
            isEmpty = ((InstanceMap*)map)->Reset(method);

        if(SendMsgTo)
        {
            if(isEmpty) SendMsgTo->SendResetInstanceSuccess(p->GetMapId());
            else SendMsgTo->SendResetInstanceFailed(0, p->GetMapId());
        }

        if(isEmpty || method == INSTANCE_RESET_GROUP_DISBAND)
        {
            // do not reset the instance, just unbind if others are permanently bound to it
            if(p->CanReset()) p->DeleteFromDB();
            else CharacterDatabase.PExecute("DELETE FROM group_instance WHERE instance = '%u'", p->GetInstanceId());
            // i don't know for sure if hash_map iterators
            m_boundInstances.erase(itr);
            itr = m_boundInstances.begin();
            // this unloads the instance save unless online players are bound to it
            // (eg. permanent binds or GM solo binds)
            p->RemoveGroup(this);
        }
        else
            ++itr;
    }
}

InstanceGroupBind* Group::GetBoundInstance(uint32 mapid)
{
    const MapEntry* entry = sMapStore.LookupEntry(mapid);

    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    if(itr != m_boundInstances.end())
        return &itr->second;
    else
        return NULL;
}

InstanceGroupBind* Group::BindToInstance(InstanceSave *save, bool permanent, bool load)
{
    if(save && !isBGGroup())
    {
        InstanceGroupBind& bind = m_boundInstances[save->GetMapId()];
        if(bind.save)
        {
            // when a boss is killed or when copying the players's binds to the group
            if(permanent != bind.perm || save != bind.save)
                if(!load) CharacterDatabase.PExecute("UPDATE group_instance SET instance = '%u', permanent = '%u' WHERE leaderGuid = '%u' AND instance = '%u'", save->GetInstanceId(), permanent, GUID_LOPART(GetLeaderGUID()), bind.save->GetInstanceId());
        }
        else
            if(!load) CharacterDatabase.PExecute("INSERT INTO group_instance (leaderGuid, instance, permanent) VALUES ('%u', '%u', '%u')", GUID_LOPART(GetLeaderGUID()), save->GetInstanceId(), permanent);

        if(bind.save != save)
        {
            if(bind.save) bind.save->RemoveGroup(this);
            save->AddGroup(this);
        }

        bind.save = save;
        bind.perm = permanent;
        if(!load) sLog.outDebug("Group::BindToInstance: %d is now bound to map %d, instance %d", GUID_LOPART(GetLeaderGUID()), save->GetMapId(), save->GetInstanceId());
        return &bind;
    }
    else
        return NULL;
}

void Group::UnbindInstance(uint32 mapid, bool unload)
{
    BoundInstancesMap::iterator itr = m_boundInstances.find(mapid);
    if(itr != m_boundInstances.end())
    {
        if(!unload) CharacterDatabase.PExecute("DELETE FROM group_instance WHERE leaderGuid = '%u' AND instance = '%u'", GUID_LOPART(GetLeaderGUID()), itr->second.save->GetInstanceId());
        itr->second.save->RemoveGroup(this);                // save can become invalid
        m_boundInstances.erase(itr);
    }
}

void Group::_homebindIfInstance(Player *player)
{
    if(player && !player->isGameMaster() && sMapStore.LookupEntry(player->GetMapId())->IsDungeon())
    {
        // leaving the group in an instance, the homebind timer is started
        // unless the player is permanently saved to the instance
        InstanceSave *save = sInstanceSaveMgr.GetInstanceSave(player->GetInstanceId());
        InstancePlayerBind *playerBind = save ? player->GetBoundInstance(save->GetMapId()) : NULL;
        if(!playerBind || !playerBind->perm)
            player->m_InstanceValid = false;
    }
}
