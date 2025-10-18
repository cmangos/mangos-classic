
#include "playerbot/playerbot.h"
#include "InviteToGroupAction.h"
#include "playerbot/ServerFacade.h"
#include "playerbot/strategy/values/Formations.h"
#include "Guilds/GuildMgr.h"

namespace ai
{
    bool InviteToGroupAction::Invite(Player* inviter, Player* player)
    {
        if (!player)
            return false;

        if (inviter == player)
            return false;

        if (!player->GetPlayerbotAI() && !ai->GetSecurity()->CheckLevelFor(PlayerbotSecurityLevel::PLAYERBOT_SECURITY_INVITE, true, player))
            return false;

        if (Group* group = inviter->GetGroup())
        {
            if(player->GetPlayerbotAI() && !player->GetPlayerbotAI()->IsRealPlayer())
                if (!group->IsRaidGroup() && group->GetMembersCount() > 4)
                    group->ConvertToRaid();
        }

        WorldPacket p;
        uint32 roles_mask = 0;
        p << player->GetName();
        p << roles_mask;
        inviter->GetSession()->HandleGroupInviteOpcode(p);

        return true;
    }

    bool JoinGroupAction::Execute(Event& event)
    {
        if (bot->InBattleGround())
            return false;

        if (bot->InBattleGroundQueue())
            return false;

        Player* master = event.getOwner();

        Group* group = master->GetGroup();

        if (group)
        {
            if (group->IsFull())
                return false;

            if (bot->GetGroup() == group)
                return false;
        }

        if (bot->GetGroup())
        {
            if (ai->HasRealPlayerMaster() && master != ai->GetMaster())
                return false;

            if (!ai->DoSpecificAction("leave", event, true))
                return false;
        }

        if (bot->GetGroupInvite())
            bot->GetGroupInvite()->RemoveInvite(bot);

        return Invite(master, bot);
    }

    bool LfgAction::Execute(Event& event)
    {
        Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
        if (bot->InBattleGround())
            return false;

        if (bot->InBattleGroundQueue())
            return false;

        if (!ai->IsSafe(requester))
            return false;

        if (requester->GetLevel() == DEFAULT_MAX_LEVEL && bot->GetLevel() != DEFAULT_MAX_LEVEL)
            return false;

        if (requester->GetLevel() > bot->GetLevel() + 4 || bot->GetLevel() > requester->GetLevel() + 4)
            return false;

        std::string param = event.getParam();

        if (!param.empty() && param != "40" && param != "25" && param != "20" && param != "10" && param != "5")
        {
            ai->TellError(requester, BOT_TEXT2("Unknown group size. Valid sizes for lfg are 40, 25, 20, 10 and 5.", {}));
            return false;
        }

        Group* group = requester->GetGroup();

        std::unordered_map<Classes, std::unordered_map<BotRoles,uint32>> allowedClassNr;
        std::unordered_map<BotRoles, uint32> allowedRoles;

        allowedRoles[BOT_ROLE_TANK] = 1;
        allowedRoles[BOT_ROLE_HEALER] = 1;
        allowedRoles[BOT_ROLE_DPS] = 3;

        BotRoles role = ai->IsTank(requester, false) ? BOT_ROLE_TANK : (ai->IsHeal(requester, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
        Classes cls = (Classes)requester->getClass();

        if (group)
        {
            //If no input use max raid for raid groups.
            if (param.empty() && group->IsRaidGroup())
#ifdef MANGOSBOT_ZERO
                param = "40";
#else
            /// Default to TBC Raiding. Max size 25
                param = "25";
#endif

            //Select optimal group layout.
            if (param == "40")
            {
                allowedRoles[BOT_ROLE_TANK] = 4;
                allowedRoles[BOT_ROLE_HEALER] = 16;
                allowedRoles[BOT_ROLE_DPS] = 20;

                allowedClassNr[CLASS_PALADIN][BOT_ROLE_TANK] = 0;
                allowedClassNr[CLASS_DRUID][BOT_ROLE_TANK] = 1;

                allowedClassNr[CLASS_DRUID][BOT_ROLE_HEALER] = 3;
                allowedClassNr[CLASS_PALADIN][BOT_ROLE_HEALER] = 4;
                allowedClassNr[CLASS_SHAMAN][BOT_ROLE_HEALER] = 4;
                allowedClassNr[CLASS_PRIEST][BOT_ROLE_HEALER] = 11;

                allowedClassNr[CLASS_WARRIOR][BOT_ROLE_DPS] = 8;
                allowedClassNr[CLASS_PALADIN][BOT_ROLE_DPS] = 4;
                allowedClassNr[CLASS_HUNTER][BOT_ROLE_DPS] = 4;
                allowedClassNr[CLASS_ROGUE][BOT_ROLE_DPS] = 6;
                allowedClassNr[CLASS_PRIEST][BOT_ROLE_DPS] = 1;
                allowedClassNr[CLASS_SHAMAN][BOT_ROLE_DPS] = 4;
                allowedClassNr[CLASS_MAGE][BOT_ROLE_DPS] = 15;
                allowedClassNr[CLASS_WARLOCK][BOT_ROLE_DPS] = 4;
                allowedClassNr[CLASS_DRUID][BOT_ROLE_DPS] = 1;
            }
            else if (param == "25")
            {
                allowedRoles[BOT_ROLE_TANK] = 3;
                allowedRoles[BOT_ROLE_HEALER] = 7;
                allowedRoles[BOT_ROLE_DPS] = 15;
            }
            else if (param == "20")
            {
                allowedRoles[BOT_ROLE_TANK] = 2;
                allowedRoles[BOT_ROLE_HEALER] = 5;
                allowedRoles[BOT_ROLE_DPS] = 13;
            }
            else if (param == "10")
            {
                allowedRoles[BOT_ROLE_TANK] = 2;
                allowedRoles[BOT_ROLE_HEALER] = 3;
                allowedRoles[BOT_ROLE_DPS] = 5;
            }

            if (group->IsFull())
            {
                if (param.empty() || param == "5"  || group->IsRaidGroup())
                    return false; //Group or raid is full so stop trying.
                else
                    group->ConvertToRaid(); //We want a raid but are in a group so convert and continue.
            }

            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                // Only add group member targets that are alive and near the player
                Player* player = sObjectMgr.GetPlayer(itr->guid);

                if (!ai->IsSafe(player))
                    return false;

                role = ai->IsTank(player, false) ? BOT_ROLE_TANK : (ai->IsHeal(player, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
                cls = (Classes)player->getClass();

                if (allowedRoles[role] > 0)
                    allowedRoles[role]--;

                if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] > 0)
                    allowedClassNr[cls][role]--;
            }
        }
        else
        {
            if (allowedRoles[role] > 0)
                allowedRoles[role]--;

            if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] > 0)
                allowedClassNr[cls][role]--;
        }

        role = ai->IsTank(bot, false) ? BOT_ROLE_TANK : (ai->IsHeal(bot, false) ? BOT_ROLE_HEALER : BOT_ROLE_DPS);
        cls = (Classes)bot->getClass();

        if (allowedRoles[role] == 0)
            return false;

        if (allowedClassNr[cls].find(role) != allowedClassNr[cls].end() && allowedClassNr[cls][role] == 0)
            return false;        

        if (bot->GetGroup())
        {
            if (ai->HasRealPlayerMaster())
                return false;

            if (!ai->DoSpecificAction("leave", event, true))
                return false;
        }

        bool invite = Invite(requester, bot);

        if (invite)
        {
            if (!ai->DoSpecificAction("accept invitation", event, true))
                return false;

            std::map<std::string, std::string> placeholders;
            placeholders["%role"] = (role == BOT_ROLE_TANK ? "tank" : (role == BOT_ROLE_HEALER ? "healer" : "dps"));
            placeholders["%spotsleft"] = std::to_string(allowedRoles[role] - 1);

            if(allowedRoles[role] > 1)
                ai->TellPlayer(requester, BOT_TEXT2("Joining as %role, %spotsleft %role spots left.", placeholders));
            else
                ai->TellPlayer(requester, BOT_TEXT2("Joining as %role.", placeholders));

            return true;
        }

        return false;
    }

    bool InviteNearbyToGroupAction::Execute(Event& event)
    {
        if (!bot->GetGroup())  //Select a random formation to copy.
        {
            std::vector<std::string> formations = { "melee","queue","chaos","circle","line","shield","arrow","near","far"};
            FormationValue* value = (FormationValue*)context->GetValue<Formation*>("formation");
            std::string newFormation = formations[urand(0, formations.size() - 1)];
            value->Load(newFormation);
        }

        std::list<ObjectGuid> nearGuids = ai->GetAiObjectContext()->GetValue<std::list<ObjectGuid> >("nearest friendly players")->Get();
        for (auto& i : nearGuids)
        {
            Player* player = sObjectMgr.GetPlayer(i);

            if (!player)
                continue;

            if (player == bot)
                continue;

            if (player->GetMapId() != bot->GetMapId())
                continue;

#ifdef MANGOSBOT_TWO
            if (player->InSamePhase(bot->GetPhaseMask()))
                continue;
#endif

            if (player->GetGroup())
                continue;

            if (!sPlayerbotAIConfig.randomBotInvitePlayer && player->isRealPlayer())
                continue;

            Group* group = bot->GetGroup();

            if (player->isDND())
                continue;

            if (player->IsBeingTeleported())
                continue;

            PlayerbotAI* botAi = player->GetPlayerbotAI();

            if (botAi)
            {
                if (botAi->GetGrouperType() == GrouperType::SOLO && !botAi->HasRealPlayerMaster()) //Do not invite solo players. 
                    continue;

                if (botAi->HasActivePlayerMaster()) //Do not invite alts of active players. 
                    continue;
            }

            if (abs(int32(player->GetLevel() - bot->GetLevel())) > 2)
                continue;

            if (sServerFacade.GetDistance2d(bot, player) > sPlayerbotAIConfig.spellDistance)
                continue;

            //When inviting the 5th member of the group convert to raid for future invites.
            if (group && ai->GetGrouperType() > GrouperType::LEADER_5 && !group->IsRaidGroup() && bot->GetGroup()->GetMembersCount() > 3)
                group->ConvertToRaid();

            Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());
            if (sPlayerbotAIConfig.inviteChat && (sRandomPlayerbotMgr.IsFreeBot(bot) || !ai->HasActivePlayerMaster()))
            {
                if (guild && bot->IsInGuild(player))
                {
                    BroadcastHelper::BroadcastGuildGroupOrRaidInvite(
                        ai,
                        bot,
                        player,
                        group,
                        guild
                    );
                }
                else
                {

                    std::map<std::string, std::string> placeholders;
                    placeholders["%player"] = player->GetName();

                    if (group && group->IsRaidGroup())
                        bot->Say(BOT_TEXT2("join_raid", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                    else
                        bot->Say(BOT_TEXT2("join_group", placeholders), (bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH));
                }
            }

            return Invite(bot, player);
        }

        return false;
    }

    bool InviteNearbyToGroupAction::isUseful()
    {
        if (!sPlayerbotAIConfig.randomBotGroupNearby)
            return false;

        if (bot->InBattleGround())
            return false;

        if (bot->InBattleGroundQueue())
            return false;

        GrouperType grouperType = ai->GetGrouperType();

        if (grouperType == GrouperType::SOLO || grouperType == GrouperType::MEMBER)
            return false;

        Group* group = bot->GetGroup();

        if (group)
        {
            if (group->IsRaidGroup() && group->IsFull())
                return false;

            if (ai->GetGroupMaster() != bot)
                return false;

            uint32 memberCount = group->GetMembersCount();

            if (memberCount >= uint8(grouperType))
                return false;
        }

        if (ai->HasActivePlayerMaster()) //Alts do not invite randomly
           return false;

        return true;
    }

    std::vector<Player*> InviteGuildToGroupAction::getGuildMembers()
    {
        Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());

        FindGuildMembers worker;
        guild->BroadcastWorker(worker);

        return worker.GetResult();
    }

    bool InviteGuildToGroupAction::Execute(Event& event)
    {
        Guild* guild = sGuildMgr.GetGuildById(bot->GetGuildId());

        for (auto& member : getGuildMembers())
        {
            Player* player = member;

            if (!player)
                continue;

            if (player == bot)
                continue;

            if (player->GetGroup())
                continue;

            if (player->isDND())
                continue;

            if (!sPlayerbotAIConfig.randomBotInvitePlayer && player->isRealPlayer())
                continue;

            if (player->IsBeingTeleported())
                continue;

            if (player->GetMapId() != bot->GetMapId() && player->GetLevel() < 30)
                continue;

#ifdef MANGOSBOT_TWO
            if (player->GetMapId() == 609 && player->GetMapId() != bot->GetMapId())
                continue;
#endif

            if (WorldPosition(player).distance(bot) > 1000 && player->GetLevel() < 15)
                continue;

            PlayerbotAI* playerAi = player->GetPlayerbotAI();

            if (playerAi)
            {
                if (playerAi->GetGrouperType() == GrouperType::SOLO && !playerAi->HasRealPlayerMaster()) //Do not invite solo players.
                    continue;

                if (playerAi->HasActivePlayerMaster()) //Do not invite alts of active players.
                    continue;

                if (player->GetLevel() > bot->GetLevel() + 5) //Invite higher levels that need money so they can grind money and help out.
                {
                    if (!PAI_VALUE(bool, "should get money"))
                        continue;
                }
            }

            if (bot->GetLevel() > player->GetLevel() + 5) //Do not invite members that too low level or risk dragging them to deadly places.
                continue;

            if (!playerAi && sServerFacade.GetDistance2d(bot, player) > sPlayerbotAIConfig.sightDistance)
                continue;

            Group* group = bot->GetGroup();
            //When inviting the 5th member of the group convert to raid for future invites.
            if (group && ai->GetGrouperType() > GrouperType::LEADER_5 && !group->IsRaidGroup() && bot->GetGroup()->GetMembersCount() > 3)
            {
                group->ConvertToRaid();
            }

            if (sPlayerbotAIConfig.inviteChat && (sRandomPlayerbotMgr.IsFreeBot(bot) || !ai->HasActivePlayerMaster()))
            {
                BroadcastHelper::BroadcastGuildGroupOrRaidInvite(
                    ai,
                    bot,
                    player,
                    group,
                    guild
                );
            }

            return Invite(bot, player);
        }

        return false;
    }
}
