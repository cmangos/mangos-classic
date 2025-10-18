
#include "playerbot/playerbot.h"
#include "ValueActions.h"

using namespace ai;

std::string LowercaseString(const std::string& string)
{
    std::string result = string;
    if (!string.empty())
    {
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) 
        { 
            return std::tolower(c); 
        });
    }

    return result;
}

std::string LowercaseString(const char* string)
{
    std::string str = string;
    return LowercaseString(str);
}

Player* FindGroupPlayerByName(Player* player, const std::string& playerName)
{
    Player* groupPlayer = nullptr;
    if (player)
    {
        const Group* group = player->GetGroup();
        if (group)
        {
            Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
            for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
            {
                Player* member = sObjectMgr.GetPlayer(itr->guid);
                if (member)
                {
                    const std::string memberName = LowercaseString(member->GetName());
                    if (memberName == playerName)
                    {
                        groupPlayer = member;
                    }
                }
            }
        }
    }

    return groupPlayer;
}

bool SetFocusHealTargetsAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (ai->IsHeal(bot) || ai->HasStrategy("offheal", BotState::BOT_STATE_COMBAT))
    {
        const std::string param = LowercaseString(event.getParam());
        if (!param.empty())
        {
            std::list<ObjectGuid> focusHealTargets = AI_VALUE(std::list<ObjectGuid>, "focus heal targets");

            // Query current focus targets
            if (param.find('?') != std::string::npos)
            {
                std::stringstream ss;
                if (focusHealTargets.empty())
                {
                    ss << "I don't have any focus heal targets";
                }
                else
                {
                    ss << "My focus heal targets are ";
                    for (const ObjectGuid& focusTargetGuid : focusHealTargets)
                    {
                        Unit* focusHealTarget = ai->GetUnit(focusTargetGuid);
                        if (focusHealTarget)
                        {
                            if (focusTargetGuid == focusHealTargets.back())
                            {
                                ss << focusHealTarget->GetName();
                            }
                            else
                            {
                                ss << focusHealTarget->GetName() << ", ";
                            }
                        }
                    }
                }

                ai->TellPlayerNoFacing(requester, ss.str());
                return true;
            }
            else if (param == "none" || param == "unset" || param == "clear")
            {   
                focusHealTargets.clear();
                SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);
                ai->ChangeStrategy("-focus heal targets", BotState::BOT_STATE_COMBAT);
                ai->TellPlayerNoFacing(requester, "Removed focus heal targets");
                return true;
            }
            else
            {
                // Multiple focus heal targets
                std::vector<std::string> targetNames;
                if (param.find(',') != std::string::npos)
                {
                    std::string targetName;
                    std::stringstream ss(param);
                    while (std::getline(ss, targetName, ','))
                    {
                        targetNames.push_back(targetName);
                    }
                }
                else
                {
                    targetNames.push_back(param);
                }

                if (!targetNames.empty())
                {
                    if (bot->GetGroup())
                    {
                        for (const std::string& targetName : targetNames)
                        {
                            const bool add = targetName.find("+") != std::string::npos;
                            const bool remove = targetName.find("-") != std::string::npos;
                            if (add || remove)
                            {
                                const std::string playerName = targetName.substr(1);
                                const Player* target = FindGroupPlayerByName(bot, playerName);
                                if (target)
                                {
                                    const ObjectGuid& targetGuid = target->GetObjectGuid();
                                    if (add)
                                    {
                                        // Check if the target exists already on the list
                                        if (std::find(focusHealTargets.begin(), focusHealTargets.end(), targetGuid) == focusHealTargets.end())
                                        {
                                            focusHealTargets.push_back(targetGuid);
                                        }

                                        std::stringstream message; message << "Added " << playerName << " to focus heal targets";
                                        ai->TellPlayerNoFacing(requester, message.str());
                                    }
                                    else
                                    {
                                        focusHealTargets.remove(targetGuid);
                                        std::stringstream message; message << "Removed " << playerName << " from focus heal targets";
                                        ai->TellPlayerNoFacing(requester, message.str());
                                    }
                                }
                                else
                                {
                                    std::stringstream message; message << "I'm not in a group with " << playerName;
                                    ai->TellPlayerNoFacing(requester, message.str());
                                }
                            }
                            else
                            {
                                ai->TellPlayerNoFacing(requester, "Please specify a + for add or - to remove a target");
                            }
                        }

                        SET_AI_VALUE(std::list<ObjectGuid>, "focus heal targets", focusHealTargets);

                        if (focusHealTargets.empty())
                        {
                            ai->ChangeStrategy("-focus heal targets", BotState::BOT_STATE_COMBAT);
                        }
                        else
                        {
                            ai->ChangeStrategy("+focus heal targets", BotState::BOT_STATE_COMBAT);
                        }

                        return true;
                    }
                    else
                    {
                        ai->TellPlayerNoFacing(requester, "I'm not in a group");
                    }
                }
                else
                {
                    ai->TellPlayerNoFacing(requester, "Please provide one or more player names");
                }
            }
        }
        else
        {
            ai->TellPlayerNoFacing(requester, "Please provide one or more player names");
        }
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "I'm not a healer or offhealer (please change my strats to heal or offheal)");
    }

    return false;
}

bool SetWaitForAttackTimeAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::string newTimeStr = event.getParam();
    if (!newTimeStr.empty())
    {
        // Check if the param is a number
        if (newTimeStr.find_first_not_of("0123456789") == std::string::npos)
        {
            const int newTime = std::stoi(newTimeStr.c_str());
            if (newTime <= 99)
            {
                ai->GetAiObjectContext()->GetValue<uint8>("wait for attack time")->Set(newTime);
                std::ostringstream out; out << "Wait for attack time set to " << newTime << " seconds";
                ai->TellPlayerNoFacing(requester, out);
                return true;
            }
            else
            {
                ai->TellPlayerNoFacing(requester, "Please provide valid time to set (in seconds) between 1 and 99");
            }
        }
        else
        {
            ai->TellPlayerNoFacing(requester, "Please provide valid time to set (in seconds) between 1 and 99");
        }
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "Please provide a time to set (in seconds)");
    }

    return false;
}

bool SetFollowTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    const std::string param = LowercaseString(event.getParam());
    if (!param.empty())
    {
        const bool removeTarget = param == "none" || param == "unset";
        if (removeTarget)
        {
            SET_AI_VALUE(GuidPosition, "manual follow target", GuidPosition());
            ai->TellPlayerNoFacing(requester, "Removed follow target");
            return true;
        }
        else
        {
            if (bot->GetGroup())
            {
                Player* target = FindGroupPlayerByName(bot, param);
                if (target)
                {
                    if (target != bot)
                    {
                        SET_AI_VALUE(GuidPosition, "manual follow target", target);
                        std::stringstream message; message << "Set " << param << " as the follow target";
                        ai->TellPlayerNoFacing(requester, message.str());
                        return true;
                    }
                    else
                    {
                        ai->TellPlayerNoFacing(requester, "I can't follow myself!");
                    }
                }
                else
                {
                    std::stringstream message; message << "I'm not in a group with " << param;
                    ai->TellPlayerNoFacing(requester, message.str());
                }
            }
            else
            {
                ai->TellPlayerNoFacing(requester, "I'm not in a group");
            }
        }
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "Please provide one or more player names");
    }

    return false;
}

bool SetSpellTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    const std::string param = LowercaseString(event.getParam());
    if (!param.empty())
    {
        std::list<ObjectGuid> targets = AI_VALUE(std::list<ObjectGuid>, name);

        // Query current boost targets
        if (param.find('?') != std::string::npos)
        {
            std::stringstream ss;
            if (targets.empty())
            {
                ss << "I don't have any " << name << " saved";
            }
            else
            {
                ss << "My " << name << " are ";
                for (const ObjectGuid& boostTargetGuid : targets)
                {
                    Unit* boostTarget = ai->GetUnit(boostTargetGuid);
                    if (boostTarget)
                    {
                        if (boostTargetGuid == targets.back())
                        {
                            ss << boostTarget->GetName();
                        }
                        else
                        {
                            ss << boostTarget->GetName() << ", ";
                        }
                    }
                }
            }

            ai->TellPlayerNoFacing(requester, ss.str());
            return true;
        }
        else if (param == "none" || param == "unset" || param == "clear")
        {
            targets.clear();
            SET_AI_VALUE(std::list<ObjectGuid>, name, targets);
            std::stringstream ss; ss << "Removed all " << name;
            ai->TellPlayerNoFacing(requester, ss.str());
            return true;
        }
        else
        {
            // Multiple boost targets
            std::vector<std::string> targetNames;
            if (param.find(',') != std::string::npos)
            {
                std::string targetName;
                std::stringstream ss(param);
                while (std::getline(ss, targetName, ','))
                {
                    targetNames.push_back(targetName);
                }
            }
            else
            {
                targetNames.push_back(param);
            }

            if (!targetNames.empty())
            {
                if (bot->GetGroup())
                {
                    for (const std::string& targetName : targetNames)
                    {
                        const bool add = targetName.find("+") != std::string::npos;
                        const bool remove = targetName.find("-") != std::string::npos;
                        if (add || remove)
                        {
                            const std::string playerName = targetName.substr(1);
                            const Player* target = FindGroupPlayerByName(bot, playerName);
                            if (target)
                            {
                                const ObjectGuid& targetGuid = target->GetObjectGuid();
                                if (add)
                                {
                                    // Check if the target exists already on the list
                                    if (std::find(targets.begin(), targets.end(), targetGuid) == targets.end())
                                    {
                                        targets.push_back(targetGuid);
                                    }

                                    std::stringstream message; message << "Added " << playerName << " to my " << name;
                                    ai->TellPlayerNoFacing(requester, message.str());
                                }
                                else
                                {
                                    targets.remove(targetGuid);
                                    std::stringstream message; message << "Removed " << playerName << " from my " << name;
                                    ai->TellPlayerNoFacing(requester, message.str());
                                }
                            }
                            else
                            {
                                std::stringstream message; message << "I'm not in a group with " << playerName;
                                ai->TellPlayerNoFacing(requester, message.str());
                            }
                        }
                        else
                        {
                            ai->TellPlayerNoFacing(requester, "Please specify a + for add or - to remove a target");
                        }
                    }

                    SET_AI_VALUE(std::list<ObjectGuid>, name, targets);

                    return true;
                }
                else
                {
                    ai->TellPlayerNoFacing(requester, "I'm not in a group");
                }
            }
            else
            {
                ai->TellPlayerNoFacing(requester, "Please provide one or more player names");
            }
        }
    }
    else
    {
        ai->TellPlayerNoFacing(requester, "Please provide one or more player names");
    }

    return false;
}