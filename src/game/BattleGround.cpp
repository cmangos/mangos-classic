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

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "MapManager.h"
#include "Language.h"
#include "Chat.h"
#include "SpellAuras.h"
#include "World.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Mail.h"
#include "WorldPacket.h"
#include "Formulas.h"

BattleGround::BattleGround()
{
    m_TypeID            = BattleGroundTypeId(0);
    m_InstanceID        = 0;
    m_Status            = STATUS_NONE;
    m_EndTime           = 0;
    m_BracketId         = MAX_BATTLEGROUND_BRACKETS;        // use as mark bg template
    m_InvitedAlliance   = 0;
    m_InvitedHorde      = 0;
    m_Winner            = 2;
    m_StartTime         = 0;
    m_Events            = 0;
    m_BuffChange        = false;
    m_Name              = "";
    m_LevelMin          = 0;
    m_LevelMax          = 0;
    m_InBGFreeSlotQueue = false;
    m_SetDeleteThis     = false;

    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers        = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers        = 0;

    m_MapId             = 0;
    m_Map               = NULL;

    m_TeamStartLocX[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocX[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocY[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocY[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocZ[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocZ[BG_TEAM_HORDE]      = 0;

    m_TeamStartLocO[BG_TEAM_ALLIANCE]   = 0;
    m_TeamStartLocO[BG_TEAM_HORDE]      = 0;

    m_BgRaids[BG_TEAM_ALLIANCE]         = NULL;
    m_BgRaids[BG_TEAM_HORDE]            = NULL;

    m_PlayersCount[BG_TEAM_ALLIANCE]    = 0;
    m_PlayersCount[BG_TEAM_HORDE]       = 0;

    m_TeamScores[BG_TEAM_ALLIANCE]      = 0;
    m_TeamScores[BG_TEAM_HORDE]         = 0;

    m_PrematureCountDown = false;
    m_PrematureCountDown = 0;
}

BattleGround::~BattleGround()
{
    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)

    int size = m_BgObjects.size();
    for(int i = 0; i < size; ++i)
        DelObject(i);

    // delete creature and go respawn times
    WorldDatabase.PExecute("DELETE FROM creature_respawn WHERE instance = '%u'",GetInstanceID());
    WorldDatabase.PExecute("DELETE FROM gameobject_respawn WHERE instance = '%u'",GetInstanceID());
    // delete instance from db
    CharacterDatabase.PExecute("DELETE FROM instance WHERE id = '%u'",GetInstanceID());
    // remove from battlegrounds
    sBattleGroundMgr.RemoveBattleGround(GetInstanceID());

    // unload map
    // map can be null at bg destruction
    if (m_Map)
        m_Map->SetUnload();
    // remove from bg free slot queue
    this->RemoveFromBGFreeSlotQueue();

    for(BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
        delete itr->second;
}

void BattleGround::Update(uint32 diff)
{
    if (!GetPlayersSize() && !GetRemovedPlayersSize())
        //BG is empty
        return;

    WorldPacket data;

    if(GetRemovedPlayersSize())
    {
        for(std::map<uint64, uint8>::iterator itr = m_RemovedPlayers.begin(); itr != m_RemovedPlayers.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);
            switch(itr->second)
            {
                //following code is handled by event:
                /*case 0:
                    sBattleGroundMgr.m_BattleGroundQueues[GetTypeID()].RemovePlayer(itr->first);
                    //RemovePlayerFromQueue(itr->first);
                    if(plr)
                    {
                        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattleGroundQueueIndex(m_TypeID), STATUS_NONE, 0, 0);
                        plr->GetSession()->SendPacket(&data);
                    }
                    break;*/
                case 1:                                     // currently in bg and was removed from bg
                    if(plr)
                        RemovePlayerAtLeave(itr->first, true, true);
                    else
                        RemovePlayerAtLeave(itr->first, false, false);
                    break;
                default:
                    sLog.outError("BattleGround: Unknown remove player case!");
            }
        }
        m_RemovedPlayers.clear();
    }

    // this code isn't efficient and its idea isn't implemented yet
    /* offline players are removed from battleground in worldsession::LogoutPlayer()
    // remove offline players from bg after ~5 minutes
    if(GetPlayersSize())
    {
        for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = objmgr.GetPlayer(itr->first);
            itr->second.LastOnlineTime += diff;

            if(plr)
                itr->second.LastOnlineTime = 0;   // update last online time
            else
                if(itr->second.LastOnlineTime >= MAX_OFFLINE_TIME)                   // 5 minutes
                    m_RemovedPlayers[itr->first] = 1;       // add to remove list (BG)
        }
    }*/

    /*********************************************************/
    /***           BATTLEGROUND BALLANCE SYSTEM            ***/
    /*********************************************************/

    // if less then minimum players are in on one side, then start premature finish timer
    if(GetStatus() == STATUS_IN_PROGRESS && sBattleGroundMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
    {
        if(!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sBattleGroundMgr.GetPrematureFinishTime();
            SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
        }
        else if(m_PrematureCountDownTimer < diff)
        {
            // time's up!
            EndBattleGround(0); // noone wins
            m_PrematureCountDown = false;
        }
        else
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if(m_PrematureCountDownTimer != sBattleGroundMgr.GetPrematureFinishTime() && newtime / 60000 != m_PrematureCountDownTimer / 60000)
                SendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING);
            m_PrematureCountDownTimer = newtime;
        }

    }
    else if (m_PrematureCountDown)
        m_PrematureCountDown = false;

    /*********************************************************/
    /***           BATTLEGROUND ENDING SYSTEM              ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // remove all players from battleground after 2 minutes
        m_EndTime += diff;
        if(m_EndTime >= TIME_TO_AUTOREMOVE)                 // 2 minutes
        {
            for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
            {
                m_RemovedPlayers[itr->first] = 1;           // add to remove list (BG)
            }
            // do not change any battleground's private variables
        }
    }

    //update start time
    m_StartTime += diff;
}

void BattleGround::SetTeamStartLoc(uint32 TeamID, float X, float Y, float Z, float O)
{
    uint8 idx = GetTeamIndexByTeamId(TeamID);
    m_TeamStartLocX[idx] = X;
    m_TeamStartLocY[idx] = Y;
    m_TeamStartLocZ[idx] = Z;
    m_TeamStartLocO[idx] = O;
}

void BattleGround::SendPacketToAll(WorldPacket *packet)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if(plr)
            plr->GetSession()->SendPacket(packet);
        else
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
    }
}

void BattleGround::SendPacketToTeam(uint32 TeamID, WorldPacket *packet, Player *sender, bool self)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        if(!self && sender == plr)
            continue;

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->GetSession()->SendPacket(packet);
    }
}

void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

void BattleGround::PlaySoundToTeam(uint32 SoundID, uint32 TeamID)
{
    WorldPacket data;

    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }
    }
}

void BattleGround::CastSpellOnTeam(uint32 SpellID, uint32 TeamID)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->CastSpell(plr, SpellID, true);
    }
}

void BattleGround::RewardHonorToTeam(uint32 Honor, uint32 TeamID)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);
    }
}

void BattleGround::RewardHonorToTeamDepOnLvl(const uint32 Honor[], uint32 TeamID)
{
    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        uint32 team = itr->second.Team;
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
        {
            uint32 range = GetLevelRange(plr->getLevel());
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor[range]);
        }
    }
}

void BattleGround::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, uint32 TeamID)
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if(!factionEntry)
        return;

    for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);

        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        uint32 team = itr->second.Team;//GetPlayerTeam(plr->GetGUID());
        if(!team) team = plr->GetTeam();

        if(team == TeamID)
            plr->GetReputationMgr().ModifyReputation(factionEntry, Reputation);
    }
}

void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

void BattleGround::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player *Source)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->GetSession()->SendPacket(&data);
}

void BattleGround::EndBattleGround(uint32 winner)
{
    this->RemoveFromBGFreeSlotQueue();

    uint32 loser_rating = 0;
    uint32 winner_rating = 0;
    WorldPacket data;
    Player *Source = NULL;
    const char *winmsg = "";

    if(winner == ALLIANCE)
    {
        winmsg = GetMangosString(LANG_BG_A_WINS);

        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        SetWinner(WINNER_ALLIANCE);
    }
    else if(winner == HORDE)
    {
        winmsg = GetMangosString(LANG_BG_H_WINS);

        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        SetWinner(WINNER_HORDE);
    }
    else
    {
        SetWinner(3);
    }

    SetStatus(STATUS_WAIT_LEAVE);
    m_EndTime = 0;

    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("BattleGround: Player (GUID: %u) not found!", GUID_LOPART(itr->first));
            continue;
        }

        // should remove spirit of redemption
        if(plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

        if(!plr->isAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
        else
        {
            //needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();
            plr->getHostileRefManager().deleteReferences();
        }

        uint32 team = itr->second.Team;
        if(!team) team = plr->GetTeam();

        if(team == winner)
        {
            if(!Source)
                Source = plr;
            RewardMark(plr,ITEM_WINNER_COUNT);
            RewardQuestComplete(plr);
        }
        else
            RewardMark(plr,ITEM_LOSER_COUNT);

        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetTeam(), plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->GetSession()->SendPacket(&data);
    }

    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID())].BGEndedRemoveInvites(this);

    if(Source)
    {
        ChatHandler(Source).FillMessageData(&data, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, Source->GetGUID(), winmsg);
        SendPacketToAll(&data);
    }
}

uint32 BattleGround::GetBonusHonorFromKill(uint32 kills) const
{
    //variable kills means how many honorable kills you scored (so we need kills * honor_for_one_kill)
    return (uint32)MaNGOS::Honor::hk_honor_at_level(GetMaxLevel(), kills);
}

uint32 BattleGround::GetLevelRange(uint32 level) const
{
    if (level < 20)
        return 0;
    else if (level < 30)
        return 1;
    else if (level < 40)
        return 2;
    else if (level < 50)
        return 3;
    else if (level < 60)
        return 4;
    else
        return 5;
}

uint32 BattleGround::GetBattlemasterEntry() const
{
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV: return 15972;
        case BATTLEGROUND_WS: return 14623;
        case BATTLEGROUND_AB: return 14879;
        default:              return 0;
    }
}

void BattleGround::RewardMark(Player *plr,uint32 count)
{
    BattleGroundMarks mark;
    bool IsSpell;
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV:
            IsSpell = true;
            if(count == ITEM_WINNER_COUNT)
                mark = SPELL_AV_MARK_WINNER;
            else
                mark = SPELL_AV_MARK_LOSER;
            break;
        case BATTLEGROUND_WS:
            IsSpell = true;
            if(count == ITEM_WINNER_COUNT)
                mark = SPELL_WS_MARK_WINNER;
            else
                mark = SPELL_WS_MARK_LOSER;
            break;
        case BATTLEGROUND_AB:
            IsSpell = true;
            if(count == ITEM_WINNER_COUNT)
                mark = SPELL_AB_MARK_WINNER;
            else
                mark = SPELL_AB_MARK_LOSER;
            break;
        default:
            return;
    }

    if (IsSpell)
        RewardSpellCast(plr,mark);
    else
        RewardItem(plr,mark,count);
}

void BattleGround::RewardSpellCast(Player *plr, uint32 spell_id)
{
    SpellEntry const *spellInfo = sSpellStore.LookupEntry(spell_id);
    if(!spellInfo)
    {
        sLog.outError("Battleground reward casting spell %u not exist.",spell_id);
        return;
    }

    plr->CastSpell(plr, spellInfo, true);
}

void BattleGround::RewardItem(Player *plr, uint32 item_id, uint32 count)
{
    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = plr->CanStoreNewItem( NULL_BAG, NULL_SLOT, dest, item_id, count, &no_space_count );

    if( msg == EQUIP_ERR_ITEM_NOT_FOUND)
    {
        sLog.outErrorDb("Battleground reward item (Entry %u) not exist in `item_template`.",item_id);
        return;
    }

    if( msg != EQUIP_ERR_OK )                               // convert to possible store amount
        count -= no_space_count;

    if( count != 0 && !dest.empty())                        // can add some
        if (Item* item = plr->StoreNewItem( dest, item_id, true, 0))
            plr->SendNewItem(item,count,true,false);

    if (no_space_count > 0)
        SendRewardMarkByMail(plr,item_id,no_space_count);
}

void BattleGround::SendRewardMarkByMail(Player *plr,uint32 mark, uint32 count)
{
    uint32 bmEntry = GetBattlemasterEntry();
    if(!bmEntry)
        return;

    ItemPrototype const* markProto = ObjectMgr::GetItemPrototype(mark);
    if (!markProto)
        return;

    if(Item* markItem = Item::CreateItem(mark,count,plr))
    {
        // save new item before send
        markItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

        // subject: item name
        std::string subject = markProto->Name1;
        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndex();
        if ( loc_idx >= 0 )
            if(ItemLocale const *il = sObjectMgr.GetItemLocale(markProto->ItemId))
                if (il->Name.size() > size_t(loc_idx) && !il->Name[loc_idx].empty())
                    subject = il->Name[loc_idx];

        // text
        std::string textFormat = plr->GetSession()->GetMangosString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf,300,textFormat.c_str(),GetName(),GetName());
        uint32 itemTextId = sObjectMgr.CreateItemText( textBuf );

        MailDraft(subject, textBuf)
            .AddItem(markItem)
            .SendMailTo(plr, MailSender(MAIL_CREATURE, bmEntry));
    }
}

void BattleGround::RewardQuestComplete(Player *plr)
{
    uint32 quest;
    switch(GetTypeID())
    {
        case BATTLEGROUND_AV:
            quest = SPELL_AV_QUEST_REWARD;
            break;
        case BATTLEGROUND_WS:
            quest = SPELL_WS_QUEST_REWARD;
            break;
        case BATTLEGROUND_AB:
            quest = SPELL_AB_QUEST_REWARD;
            break;
        default:
            return;
    }

    RewardSpellCast(plr, quest);
}

void BattleGround::BlockMovement(Player *plr)
{
    plr->SetClientControl(plr, 0);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

void BattleGround::RemovePlayerAtLeave(uint64 guid, bool Transport, bool SendPacket)
{
    uint32 team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps
    std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.find(guid);
    if(itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);   // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    BattleGroundScoreMap::iterator itr2 = m_PlayerScores.find(guid);
    if(itr2 != m_PlayerScores.end())
    {
        delete itr2->second;                                // delete player's score
        m_PlayerScores.erase(itr2);
    }

    Player *plr = sObjectMgr.GetPlayer(guid);

    // should remove spirit of redemption
    if(plr && plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);

    if(plr && !plr->isAlive())                              // resurrect on exit
    {
        plr->ResurrectPlayer(1.0f);
        plr->SpawnCorpseBones();
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code

    if(plr)
    {
        if(participant) // if the player was a match participant, remove auras, calc rating, update queue
        {
            if(!team) team = plr->GetTeam();

            BattleGroundTypeId bgTypeId = GetTypeID();
            uint32 bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID());

            WorldPacket data;
            if(SendPacket)
            {
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, team, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                plr->GetSession()->SendPacket(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);

            DecreaseInvitedCount(team);
            //we should update battleground queue, but only if bg isn't ending
            if (GetBracketId() < MAX_BATTLEGROUND_BRACKETS)
                sBattleGroundMgr.m_BattleGroundQueues[bgQueueTypeId].Update(bgTypeId, GetBracketId());

            Group * group = plr->GetGroup();
            // remove from raid group if exist
            if(group && group == GetBgRaid(team))
            {
                if(!group->RemoveMember(guid, 0))               // group was disbanded
                {
                    SetBgRaid(team, NULL);
                    delete group;
                }
            }

            // Let others know
            sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, plr);
            SendPacketToTeam(team, &data, plr, false);
        }

        // Do next only if found in battleground
        plr->SetBattleGroundId(0);                          // We're not in BG.
        // reset destination bg team
        plr->SetBGTeam(0);

        if(Transport)
            plr->TeleportTo(plr->GetBattleGroundEntryPoint());

        // Log
        sLog.outDetail("BATTLEGROUND: Removed player %s from BattleGround.", plr->GetName());
    }

    if(!GetPlayersSize() && !GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
    {
        // if no players left AND no invitees left, set this bg to delete in next update
        // direct deletion could cause crashes
        m_SetDeleteThis = true;
        // return to prevent addition to freeslotqueue
        return;
    }

    // a player exited the battleground, so there are free slots. add to queue
    this->AddToBGFreeSlotQueue();
}

// this method is called when no players remains in battleground
void BattleGround::Reset()
{
    SetBracketId(MAX_BATTLEGROUND_BRACKETS);
    SetWinner(WINNER_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);

    m_Events = 0;

    // door-event2 is always 0
    m_ActiveEvents[BG_EVENT_DOOR] = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
        sLog.outError("BattleGround system: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    m_Players.clear();

    for(BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
        delete itr->second;
    m_PlayerScores.clear();

    // reset BGSubclass
    ResetBGSubclass();
}

void BattleGround::StartBattleGround()
{
    ///this method should spawn spirit guides and so on
    SetStartTime(0);
}

void BattleGround::AddPlayer(Player *plr)
{
    // remove afk from player
    if (plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
        plr->ToggleAFK();

    // score struct must be created in inherited class

    uint64 guid = plr->GetGUID();
    uint32 team = plr->GetBGTeam();

    BattleGroundPlayer bp;
    bp.LastOnlineTime = 0;
    bp.Team = team;

    // Add to list/maps
    m_Players[guid] = bp;

    UpdatePlayersCountByTeam(team, false);                  // +1 player

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);

    // Log
    sLog.outDetail("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

/* This method should be called only once ... it adds pointer to queue */
void BattleGround::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if(!m_InBGFreeSlotQueue)
    {
        sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

/* This method removes this battleground from free queue - it must be called when deleting battleground - not used now*/
void BattleGround::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    // uncomment this code when battlegrounds will work like instances
    for (std::deque<BattleGround*>::iterator itr = sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].begin(); itr != sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == m_InstanceID)
        {
            sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].erase(itr);
            return;
        }
    }
}

// get the number of free slots for team
// works in similar way that HasFreeSlotsForTeam did, but this is needed for join as group
uint32 BattleGround::GetFreeSlotsForTeam(uint32 Team) const
{
    //if BG is starting ... invite anyone
    if (GetStatus() == STATUS_WAIT_JOIN)
        return (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;
    //if BG is already started .. do not allow to join too much players of one faction
    uint32 otherTeam;
    uint32 otherIn;
    if (Team == ALLIANCE)
    {
        otherTeam = GetInvitedCount(HORDE);
        otherIn = GetPlayersCountByTeam(HORDE);
    }
    else
    {
        otherTeam = GetInvitedCount(ALLIANCE);
        otherIn = GetPlayersCountByTeam(ALLIANCE);
    }
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // difference based on ppl invited (not necessarily entered battle)
        // default: allow 0
        uint32 diff = 0;
        // allow join one person if the sides are equal (to fill up bg to minplayersperteam)
        if (otherTeam == GetInvitedCount(Team))
            diff = 1;
        // allow join more ppl if the other side has more players
        else if(otherTeam > GetInvitedCount(Team))
            diff = otherTeam - GetInvitedCount(Team);

        // difference based on max players per team (don't allow inviting more)
        uint32 diff2 = (GetInvitedCount(Team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(Team) : 0;

        // difference based on players who already entered
        // default: allow 0
        uint32 diff3 = 0;
        // allow join one person if the sides are equal (to fill up bg minplayersperteam)
        if (otherIn == GetPlayersCountByTeam(Team))
            diff3 = 1;
        // allow join more ppl if the other side has more players
        else if (otherIn > GetPlayersCountByTeam(Team))
            diff3 = otherIn - GetPlayersCountByTeam(Team);
        // or other side has less than minPlayersPerTeam
        else if (GetInvitedCount(Team) <= GetMinPlayersPerTeam())
            diff3 = GetMinPlayersPerTeam() - GetInvitedCount(Team) + 1;

        // return the minimum of the 3 differences

        // min of diff and diff 2
        diff = diff < diff2 ? diff : diff2;

        // min of diff, diff2 and diff3
        return diff < diff3 ? diff : diff3 ;
    }

    return 0;
}

bool BattleGround::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

void BattleGround::UpdatePlayerScore(Player *Source, uint32 type, uint32 value)
{
    //this procedure is called from virtual function implemented in bg subclass
    BattleGroundScoreMap::const_iterator itr = m_PlayerScores.find(Source->GetGUID());

    if(itr == m_PlayerScores.end())                         // player not found...
        return;

    switch(type)
    {
        case SCORE_KILLING_BLOWS:                           // Killing blows
            itr->second->KillingBlows += value;
            break;
        case SCORE_DEATHS:                                  // Deaths
            itr->second->Deaths += value;
            break;
        case SCORE_HONORABLE_KILLS:                         // Honorable kills
            itr->second->HonorableKills += value;
            break;
        case SCORE_BONUS_HONOR:                             // Honor bonus
            // reward honor instantly
            if (Source->AddHonorCP(value,HONORABLE,0,0))
                itr->second->BonusHonor += value;
            break;
        default:
            sLog.outError("BattleGround: Unknown player score type %u", type);
            break;
    }
}

bool BattleGround::AddObject(uint32 type, uint32 entry, float x, float y, float z, float o, float rotation0, float rotation1, float rotation2, float rotation3, uint32 /*respawnTime*/)
{
    // must be created this way, adding to godatamap would add it to the base map of the instance
    // and when loading it (in go::LoadFromDB()), a new guid would be assigned to the object, and a new object would be created
    // so we must create it specific for this instance
    GameObject * go = new GameObject;
    if(!go->Create(sObjectMgr.GenerateLowGuid(HIGHGUID_GAMEOBJECT),entry, GetBgMap(),x,y,z,o,rotation0,rotation1,rotation2,rotation3,100,GO_STATE_READY))
    {
        sLog.outErrorDb("Gameobject template %u not found in database! BattleGround not created!", entry);
        sLog.outError("Cannot create gameobject template %u! BattleGround not created!", entry);
        delete go;
        return false;
    }
/*
    uint32 guid = go->GetGUIDLow();

    // without this, UseButtonOrDoor caused the crash, since it tried to get go info from godata
    // iirc that was changed, so adding to go data map is no longer required if that was the only function using godata from GameObject without checking if it existed
    GameObjectData& data = objmgr.NewGOData(guid);

    data.id             = entry;
    data.mapid          = GetMapId();
    data.posX           = x;
    data.posY           = y;
    data.posZ           = z;
    data.orientation    = o;
    data.rotation0      = rotation0;
    data.rotation1      = rotation1;
    data.rotation2      = rotation2;
    data.rotation3      = rotation3;
    data.spawntimesecs  = respawnTime;
    data.spawnMask      = 1;
    data.animprogress   = 100;
    data.go_state       = 1;
*/
    // add to world, so it can be later looked up from HashMapHolder
    go->AddToWorld();
    m_BgObjects[type] = go->GetGUID();
    return true;
}

//some doors aren't despawned so we cannot handle their closing in gameobject::update()
//it would be nice to correctly implement GO_ACTIVATED state and open/close doors in gameobject code
void BattleGround::DoorClose(uint64 const& guid)
{
    GameObject *obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        //if doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            //change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        sLog.outError("BattleGround: Door object not found (cannot close doors)");
    }
}

void BattleGround::DoorOpen(uint64 const& guid)
{
    GameObject *obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        //change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
    {
        sLog.outError("BattleGround: Door object not found! - doors will be closed.");
    }
}

void BattleGround::OnObjectDBLoad(Creature* creature)
{
    const BattleGroundEventIdx eventId = sBattleGroundMgr.GetCreatureEventIndex(creature->GetDBTableGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
        return;
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].creatures.push_back(creature->GetGUID());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
        SpawnBGCreature(creature->GetGUID(), RESPAWN_ONE_DAY);
}

uint64 BattleGround::GetSingleCreatureGuid(uint8 event1, uint8 event2)
{
    BGCreatures::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    if (itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end())
        return *itr;
    return 0;
}

void BattleGround::OnObjectDBLoad(GameObject* obj)
{
    const BattleGroundEventIdx eventId = sBattleGroundMgr.GetGameObjectEventIndex(obj->GetDBTableGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
        return;
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].gameobjects.push_back(obj->GetGUID());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
    {
        SpawnBGObject(obj->GetGUID(), RESPAWN_ONE_DAY);
    }
    else
    {
        // it's possible, that doors aren't spawned anymore (wsg)
        if (GetStatus() >= STATUS_IN_PROGRESS && IsDoor(eventId.event1, eventId.event2))
            DoorOpen(obj->GetGUID());
    }
}

bool BattleGround::IsDoor(uint8 event1, uint8 event2)
{
    if (event1 == BG_EVENT_DOOR)
    {
        if (event2 > 0)
        {
            sLog.outError("BattleGround too high event2 for event1:%i", event1);
            return false;
        }
        return true;
    }
    return false;
}

void BattleGround::OpenDoorEvent(uint8 event1, uint8 event2 /*=0*/)
{
    if (!IsDoor(event1, event2))
    {
        sLog.outError("BattleGround:OpenDoorEvent this is no door event1:%u event2:%u", event1, event2);
        return;
    }
    if (!IsActiveEvent(event1, event2))                 // maybe already despawned (eye)
    {
        sLog.outError("BattleGround:OpenDoorEvent this event isn't active event1:%u event2:%u", event1, event2);
        return;
    }
    BGObjects::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for(; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr)
        DoorOpen(*itr);
}

void BattleGround::SpawnEvent(uint8 event1, uint8 event2, bool spawn)
{
    // stop if we want to spawn something which was already spawned
    // or despawn something which was already despawned
    if (event2 == BG_EVENT_NONE || (spawn && m_ActiveEvents[event1] == event2)
        || (!spawn && m_ActiveEvents[event1] != event2))
        return;

    if (spawn)
    {
        // if event gets spawned, the current active event mus get despawned
        SpawnEvent(event1, m_ActiveEvents[event1], false);
        m_ActiveEvents[event1] = event2;                    // set this event to active
    }
    else
        m_ActiveEvents[event1] = BG_EVENT_NONE;             // no event active if event2 gets despawned

    BGCreatures::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    for(; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end(); ++itr)
        SpawnBGCreature(*itr, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
    BGObjects::const_iterator itr2 = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for(; itr2 != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr2)
        SpawnBGObject(*itr2, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
}

void BattleGround::SpawnBGObject(uint64 const& guid, uint32 respawntime)
{
    Map* map = GetBgMap();

    GameObject *obj = map->GetGameObject(guid);
    if(!obj)
        return;
    if (respawntime == 0)
    {
        //we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
        if (obj->getLootState() == GO_JUST_DEACTIVATED)
            obj->SetLootState(GO_READY);
        obj->SetRespawnTime(0);
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->SetRespawnTime(respawntime);
        obj->SetLootState(GO_JUST_DEACTIVATED);
    }
}

void BattleGround::SpawnBGCreature(uint64 const& guid, uint32 respawntime)
{
    Map* map = GetBgMap();

    Creature* obj = map->GetCreature(guid);
    if (!obj)
        return;
    if (respawntime == 0)
    {
        obj->Respawn();
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->setDeathState(JUST_DIED);
        obj->SetRespawnDelay(respawntime);
        obj->RemoveCorpse();
    }
}

bool BattleGround::DelObject(uint32 type)
{
    if (!m_BgObjects[type])
        return true;

    GameObject *obj = GetBgMap()->GetGameObject(m_BgObjects[type]);
    if (!obj)
    {
        sLog.outError("Can't find gobject guid: %u",GUID_LOPART(m_BgObjects[type]));
        return false;
    }

    obj->SetRespawnTime(0);                                 // not save respawn time
    obj->Delete();
    m_BgObjects[type] = 0;
    return true;
}

void BattleGround::SendMessageToAll(char const* text)
{
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToAll(&data);
}

void BattleGround::SendMessageToAll(int32 entry)
{
    char const* text = GetMangosString(entry);
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, CHAT_MSG_BG_SYSTEM_NEUTRAL, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToAll(&data);
}

// following functions are partially backported, (only the function call) from master
void BattleGround::SendMessageToAll(int32 entry, ChatMsg type, Player const* /*source*/)
{
    char const* text = GetMangosString(entry);
    WorldPacket data;
    ChatHandler::FillMessageData(&data, NULL, type, LANG_UNIVERSAL, NULL, 0, text, NULL);
    SendPacketToAll(&data);
}

void BattleGround::SendYellToAll(int32 entry, uint32 language, uint64 const& guid)
{
    Creature* source = GetBgMap()->GetCreature(guid);
    if(!source)
        return;
    char buf[256];
    sprintf(buf, "%s", GetMangosString(entry));
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }
        source->BuildMonsterChat(&data, CHAT_MSG_MONSTER_YELL, buf, language, source->GetName(), itr->first);
        plr->GetSession()->SendPacket(&data);
    }
}

void BattleGround::SendYell2ToAll(int32 entry, uint32 language, uint64 const& guid, int32 arg1, int32 arg2)
{
    Creature* source = GetBgMap()->GetCreature(guid);
    if(!source)
        return;
    char buf[256];
    sprintf(buf, GetMangosString(entry), GetMangosString(arg1), GetMangosString(arg2));
    for(std::map<uint64, BattleGroundPlayer>::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        WorldPacket data(SMSG_MESSAGECHAT, 200);
        Player *plr = sObjectMgr.GetPlayer(itr->first);
        if(!plr)
        {
            sLog.outError("BattleGround: Player " UI64FMTD " not found!", itr->first);
            continue;
        }
        source->BuildMonsterChat(&data, CHAT_MSG_MONSTER_YELL, buf, language, source->GetName(), itr->first);
        plr->GetSession()->SendPacket(&data);
    }
}

void BattleGround::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(TIME_TO_AUTOREMOVE);
    // inform invited players about the removal
    sBattleGroundMgr.m_BattleGroundQueues[BattleGroundMgr::BGQueueTypeId(GetTypeID())].BGEndedRemoveInvites(this);
}

// Battleground messages are localized using the dbc lang, they are not client language dependent
const char *BattleGround::GetMangosString(int32 entry)
{
    // FIXME: now we have different DBC locales and need localized message for each target client
    return sObjectMgr.GetMangosStringForDBCLocale(entry);
}

/*
important notice:
buffs aren't spawned/despawned when players captures anything
buffs are in their positions when battleground starts
*/
void BattleGround::HandleTriggerBuff(uint64 const& go_guid)
{
    GameObject *obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
        return;

    // static buffs are already handled just by database and don't need
    // battleground code
    if (!m_BuffChange)
    {
        obj->SetLootState(GO_JUST_DEACTIVATED);             // can be despawned or destroyed
        return;
    }

    // change buff type, when buff is used:
    // TODO this can be done when poolsystem works for instances
    int32 index = m_BgObjects.size() - 1;
    while (index >= 0 && m_BgObjects[index] != go_guid)
        index--;
    if (index < 0)
    {
        sLog.outError("BattleGround (Type: %u) has buff gameobject (Guid: %u Entry: %u Type:%u) but it hasn't that object in its internal data",GetTypeID(),GUID_LOPART(go_guid),obj->GetEntry(),obj->GetGoType());
        return;
    }

    //randomly select new buff
    uint8 buff = urand(0, 2);
    uint32 entry = obj->GetEntry();
    if (m_BuffChange && entry != Buff_Entries[buff])
    {
        //despawn current buff
        SpawnBGObject(m_BgObjects[index], RESPAWN_ONE_DAY);
        //set index for new one
        for (uint8 currBuffTypeIndex = 0; currBuffTypeIndex < 3; ++currBuffTypeIndex)
        {
            if (entry == Buff_Entries[currBuffTypeIndex])
            {
                index -= currBuffTypeIndex;
                index += buff;
            }
        }
    }

    SpawnBGObject(m_BgObjects[index], BUFF_RESPAWN_TIME);
}

void BattleGround::HandleKillPlayer( Player *player, Player *killer )
{
    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if( killer )
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for(BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player *plr = sObjectMgr.GetPlayer(itr->first);

            if(!plr || plr == killer)
                continue;

            if( plr->GetTeam() == killer->GetTeam() && plr->IsAtGroupRewardDistance(player) )
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
        }
    }

    // to be able to remove insignia
    player->SetFlag( UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE );
}

// return the player's team based on battlegroundplayer info
// used in same faction arena matches mainly
uint32 BattleGround::GetPlayerTeam(uint64 guid)
{
    std::map<uint64, BattleGroundPlayer>::const_iterator itr = m_Players.find(guid);
    if(itr!=m_Players.end())
        return itr->second.Team;
    return 0;
}

uint32 BattleGround::GetAlivePlayersCountByTeam(uint32 Team) const
{
    int count = 0;
    for(std::map<uint64, BattleGroundPlayer>::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if(itr->second.Team == Team)
        {
            Player * pl = sObjectMgr.GetPlayer(itr->first);
            if(pl && pl->isAlive())
                ++count;
        }
    }
    return count;
}

void BattleGround::SetBgRaid( uint32 TeamID, Group *bg_raid )
{
    Group* &old_raid = TeamID == ALLIANCE ? m_BgRaids[BG_TEAM_ALLIANCE] : m_BgRaids[BG_TEAM_HORDE];
    if(old_raid) old_raid->SetBattlegroundGroup(NULL);
    if(bg_raid) bg_raid->SetBattlegroundGroup(this);
    old_raid = bg_raid;
}

WorldSafeLocsEntry const* BattleGround::GetClosestGraveYard( Player* player )
{
    return sObjectMgr.GetClosestGraveYard( player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam() );
}
