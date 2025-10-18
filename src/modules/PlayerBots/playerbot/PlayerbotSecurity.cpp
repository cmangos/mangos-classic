
#include "PlayerbotMgr.h"
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "PlayerbotAI.h"
#include "ChatHelper.h"
#include "playerbot/ServerFacade.h"

PlayerbotSecurity::PlayerbotSecurity(Player* const bot) : bot(bot), account(0)
{
    if (bot)
    {
        account = sObjectMgr.GetPlayerAccountIdByGUID(bot->GetObjectGuid());
    }
}

PlayerbotSecurityLevel PlayerbotSecurity::LevelFor(Player* from, DenyReason* reason, bool ignoreGroup)
{

    // Allow everything if request is from gm account
    if (from->GetSession()->GetSecurity() >= SEC_GAMEMASTER)
    {
        return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL;
    }
    // Check if the bot is an alt bot of the requester
    else if ( from->GetSession()->GetAccountId() == account)
    {
        return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL;
    }
    // If a bot is a random bot
    else
    {
        Group* group = from->GetGroup();
        if (group && !ignoreGroup)
        {
            for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->getSource();
                if (player == bot)
                    return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL;
            }
        }

        if (bot->GetPlayerbotAI()->IsOpposing(from))
        {
            if (sWorld.getConfig(CONFIG_BOOL_ALLOW_TWO_SIDE_INTERACTION_GROUP))
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL;
            if (reason) *reason = DenyReason::PLAYERBOT_DENY_OPPOSING;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_DENY_ALL;
        }

        if ((int)bot->GetLevel() - (int)from->GetLevel() > sPlayerbotAIConfig.levelCheck)
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason) *reason = DenyReason::PLAYERBOT_DENY_LOW_LEVEL;
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
            }
        }

        if (sPlayerbotAIConfig.gearscorecheck)
        {
            uint32 botGS = bot->GetPlayerbotAI()->GetEquipGearScore(bot, false, false);
            uint32 fromGS = bot->GetPlayerbotAI()->GetEquipGearScore(from, false, false);
            if (botGS && bot->GetLevel() > 15 && botGS > fromGS && (100 * (botGS - fromGS) / botGS) >= 12 * sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL) / from->GetLevel())
            {
                if (reason) *reason = DenyReason::PLAYERBOT_DENY_GEARSCORE;
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
            }
        }

        if (bot->InBattleGroundQueue())
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason) *reason = DenyReason::PLAYERBOT_DENY_BG;
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
            }
        }

#ifdef MANGOSBOT_ONE
        if (bot->GetPlayerbotAI()->HasRealPlayerMaster() && bot->GetSession()->m_lfgInfo.queued)
#endif
#ifdef MANGOSBOT_ZERO
        if (sWorld.GetLFGQueue().IsPlayerInQueue(bot->GetObjectGuid()))
#endif
#ifdef MANGOSBOT_TWO
        if (false/*sLFGMgr.GetQueueInfo(bot->GetObjectGuid())*/)
#endif
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason) *reason = DenyReason::PLAYERBOT_DENY_LFG;
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
            }
        }

        /*if (sServerFacade.UnitIsDead(bot))
        {
            if (reason) *reason = DenyReason::PLAYERBOT_DENY_DEAD;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
        }*/

        group = bot->GetGroup();
        if (!group)
        {
            /*if (bot->GetMapId() != from->GetMapId() || bot->GetDistance(from) > sPlayerbotAIConfig.whisperDistance)
            {
                if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
                {
                    if (reason) *reason = DenyReason::PLAYERBOT_DENY_FAR;
                    return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK;
                }
            }*/

            if (reason) *reason = DenyReason::PLAYERBOT_DENY_INVITE;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE;
        }

        for (GroupReference *gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->getSource();
            if (player == from)
                return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_ALLOW_ALL;
        }

        if (group->IsFull())
        {
            if (reason) *reason = DenyReason::PLAYERBOT_DENY_FULL_GROUP;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_GUILD;
        }

        if (group->GetLeaderGuid() != bot->GetObjectGuid())
        {
            if (reason) *reason = DenyReason::PLAYERBOT_DENY_NOT_LEADER;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_GUILD;
        }
        else
        {
            if (reason) *reason = DenyReason::PLAYERBOT_DENY_IS_LEADER;
            return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE;
        }

        if (reason) *reason = DenyReason::PLAYERBOT_DENY_INVITE;
        return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE;
    }

    return PlayerbotSecurityLevel::PLAYERBOT_SECURITY_DENY_ALL;
}

bool PlayerbotSecurity::CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup)
{
    DenyReason reason = DenyReason::PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel realLevel = LevelFor(from, &reason, ignoreGroup);
    if (realLevel >= level || from == bot)
        return true;

    //Do not report security errors to bots.
    if (silent || (from->GetPlayerbotAI() && !from->GetPlayerbotAI()->IsRealPlayer()))
        return false;

    Player* master = bot->GetPlayerbotAI()->GetMaster();
    if (master && bot->GetPlayerbotAI() && bot->GetPlayerbotAI()->IsOpposing(master) && master->GetSession()->GetSecurity() < SEC_GAMEMASTER)
        return false;

    std::ostringstream out;
    switch (realLevel)
    {
    case PlayerbotSecurityLevel::PLAYERBOT_SECURITY_DENY_ALL:
        out << "I'm kind of busy now";
        break;
    case PlayerbotSecurityLevel::PLAYERBOT_SECURITY_TALK:
        switch (reason)
        {
        case DenyReason::PLAYERBOT_DENY_NONE:
            out << "I'll do it later";
            break;
        case DenyReason::PLAYERBOT_DENY_LOW_LEVEL:
            out << "You are too low level: |cffff0000" << (uint32)from->GetLevel() << "|cffffffff/|cff00ff00" << (uint32)bot->GetLevel();
            break;
        case DenyReason::PLAYERBOT_DENY_GEARSCORE:
            {
                int botGS = (int)bot->GetPlayerbotAI()->GetEquipGearScore(bot, false, false);
                int fromGS = (int)bot->GetPlayerbotAI()->GetEquipGearScore(from, false, false);
                int diff = (100 * (botGS - fromGS) / botGS);
                int req = 12 * sWorld.getConfig(CONFIG_UINT32_MAX_PLAYER_LEVEL) / from->GetLevel();
                out << "Your gearscore is too low: |cffff0000" << fromGS << "|cffffffff/|cff00ff00" << botGS << " |cffff0000" << diff << "%|cffffffff/|cff00ff00" << req << "%";
            }
            break;
        case DenyReason::PLAYERBOT_DENY_NOT_YOURS:
            out << "I have a master already";
            break;
        case DenyReason::PLAYERBOT_DENY_IS_BOT:
            out << "You are a bot";
            break;
        case DenyReason::PLAYERBOT_DENY_OPPOSING:
            out << "You are the enemy";
            break;
        case DenyReason::PLAYERBOT_DENY_DEAD:
            out << "I'm dead. Will do it later";
            break;
        case DenyReason::PLAYERBOT_DENY_INVITE:
            out << "Invite me to your group first";
            break;
        case DenyReason::PLAYERBOT_DENY_FAR:
            {
                out << "You must be closer to invite me to your group. I am in ";

                uint32 area = sServerFacade.GetAreaId(bot);
                if (area)
                {
					const AreaTableEntry* entry = GetAreaEntryByAreaID(area);
                    if (entry)
                    {
                        out << " |cffffffff(|cffff0000" << entry->area_name[0] << "|cffffffff)";
                    }
                }
            }
            break;
        case DenyReason::PLAYERBOT_DENY_FULL_GROUP:
            out << "I am in a full group. Will do it later";
            break;
        case DenyReason::PLAYERBOT_DENY_IS_LEADER:
            out << "I am currently leading a group. I can invite you if you want.";
            break;
        case DenyReason::PLAYERBOT_DENY_NOT_LEADER:
            if (bot->GetPlayerbotAI()->GetGroupMaster() && bot->GetPlayerbotAI()->IsSafe(bot->GetPlayerbotAI()->GetGroupMaster()))
                out << "I am in a group with " << bot->GetPlayerbotAI()->GetGroupMaster()->GetName() << ". You can ask him for invite.";
            else
                out << "I am in a group with someone else";
            break;
        case DenyReason::PLAYERBOT_DENY_BG:
            out << "I am in a queue for BG. Will do it later";
            break;
        case DenyReason::PLAYERBOT_DENY_LFG:
            out << "I am in a queue for dungeon. Will do it later";
            break;
        default:
            out << "I can't do that";
            break;
        }
        break;
    case PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE:
        out << "Invite me to your group first";
        break;
        default:
            out << "I can't do that";
            break;
    }

    std::string text = out.str();
    uint64 guid = from->GetObjectGuid().GetRawValue();
    time_t lastSaid = whispers[guid][text];
    if (!lastSaid || (time(0) - lastSaid) >= sPlayerbotAIConfig.repeatDelay / 1000)
    {
        whispers[guid][text] = time(0);
        bot->Whisper(text, LANG_UNIVERSAL, ObjectGuid(guid));
    }
    return false;
}
