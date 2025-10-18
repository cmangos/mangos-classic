
#include "playerbot/playerbot.h"
#include "ChooseRpgTargetAction.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/strategy/values/PossibleRpgTargetsValue.h"
#include "playerbot/TravelMgr.h"
#include "playerbot/strategy/values/BudgetValues.h"
#include "GuildCreateActions.h"
#include "RpgSubActions.h"
#include "playerbot/strategy/values/ItemUsageValue.h"
#include "playerbot/strategy/values/PositionValue.h"
#include "playerbot/strategy/values/TravelValues.h"
#include <iomanip>

using namespace ai;

bool ChooseRpgTargetAction::HasSameTarget(ObjectGuid guid, uint32 max, std::list<ObjectGuid>& nearGuids)
{
    if (ai->HasRealPlayerMaster())
        return false;

    uint32 num = 0;

    for (auto& i : nearGuids)
    {
        Player* player = sObjectMgr.GetPlayer(i);

        if (!player)
            continue;

        PlayerbotAI* ai = player->GetPlayerbotAI();

        if (!ai)
            continue;

        if (!ai->AllowActivity(GRIND_ACTIVITY))
            continue;

        if (PAI_VALUE(GuidPosition,"rpg target") != guid)
            continue;

        num++;

        if (num >= max)
            return true;
    }

    return false;
}

#define SkipRpgTarget(reason){ if(debug) debugTargets[guidP] = reason; continue;}

std::unordered_map<ObjectGuid, float> ChooseRpgTargetAction::GetTargets(Player* requester, bool debug)
{
    TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target");
    focusQuestTravelList focusList = AI_VALUE(focusQuestTravelList, "focus travel target");

    GuidPosition masterRpgTarget;
    if (requester && ai->IsSafe(requester) && requester->GetPlayerbotAI())
    {
        Player* player = requester;
        masterRpgTarget = PAI_VALUE(GuidPosition, "rpg target");
    }

    std::unordered_map<ObjectGuid, float> targets;
    std::vector<ObjectGuid> targetList;

    //Gather all nearby npc's (+some creatures), game objects and players.
    std::list<ObjectGuid> possibleTargets = AI_VALUE(std::list<ObjectGuid>, "possible rpg targets");
    std::list<ObjectGuid> possibleObjects = bot->GetMap()->IsDungeon() ? AI_VALUE(std::list<ObjectGuid>, "nearest game objects") : AI_VALUE(std::list<ObjectGuid>, "nearest game objects no los");
    std::list<ObjectGuid> possiblePlayers = AI_VALUE(std::list<ObjectGuid>, "nearest friendly players");

    //List of targets that we rpg'ed with before and should be ignored.
    std::set<ObjectGuid>& ignoreList = AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");

    //Add all targets with an initial priority of 0.
    for (auto target : possibleTargets)
        targets[target] = -1.0f;

    for (auto target : possibleObjects)
        targets[target] = -1.0f;

    //Add up to 5 random players.
    for (uint32 i = 0; i < 5 && possiblePlayers.size(); i++)
    {
        auto p = possiblePlayers.begin();
        std::advance(p, urand(0, possiblePlayers.size() - 1));
        if (ignoreList.find(*p) != ignoreList.end())
            continue;

        targets[*p] = -1.0f;
        possiblePlayers.erase(p);
    }

    if (targets.empty())
        return targets;

    uint32 ignored = 0;

    //1 in 10 chance to skip the ignore list. Otherwise remove targets that should be ignored.
    if (urand(0, 9))
    {
        for (auto target : ignoreList)
        {
            targets.erase(target);
            ignored++;
        }
    }

    if (debug || ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
    {
        std::ostringstream out;
        out << "Rpg: found ";
        out << std::to_string(possibleTargets.size()) + " targets,";
        out << std::to_string(possibleObjects.size()) + " objects,";
        out << std::to_string(possiblePlayers.size()) + " players.";
        out << " Of which " + std::to_string(ignored) + " are ignored.";
        ai->TellPlayerNoFacing(requester, out);
    }

    //Force RpgTrigger to be active even if we aren't currently close to the rpg target.
    SET_AI_VALUE(std::string, "next rpg action", this->getName());

    bool hasGoodRelevance = false;
    rgpActionReason.clear();

    //Shuffle the target list to randomly process it each time.
    for (auto& target : targets)
        targetList.push_back(target.first);

    std::shuffle(targetList.begin(), targetList.end(), *GetRandomGenerator());

    //We are going to create a number of 'can free move::objectGuid' values. We remove some old ones here to clean up memory.
    context->ClearExpiredValues("can free move", 10); //Clean up old free move to.

    //Only check up to 50 targets. Selfbots and bots with a real master can check more.
    uint16 checked = 0, sametarget = 0, maxCheck = 50;
    if (ai->HasRealPlayerMaster())
        maxCheck = 500;

    std::unordered_map<ObjectGuid, std::string> debugTargets;

    for (auto& guid : targetList)
    {
        GuidPosition guidP(guid, bot->GetMapId(), bot->GetInstanceId());        

        if (!guidP)
            SkipRpgTarget("not found on map/instance.");       

        bool isTravelTarget = (guidP.GetEntry() == travelTarget->GetEntry());

        //Stop to save peformance.
        if (checked >= maxCheck && !isTravelTarget)
            continue;

        //Check if we are allowed to move to this position. This is based on movement strategies follow, free, guard, stay. Bots are limited to finding targets near the center of those movement strategies.
        //For bots with real players they are also slightly limited in range unless the player stands still for a while. See free move values.
        if (guidP.GetWorldObject(bot->GetInstanceId()) && !AI_VALUE2(bool, "can free move to", guidP.to_string()))
            SkipRpgTarget("Can not free move to.");

        if (guidP.IsGameObject())
        {
            //Ignore game objects that are not spawned or being used by others.
            GameObject* go = guidP.GetGameObject(bot->GetInstanceId());
            if (!go || !sServerFacade.isSpawned(go)
                || go->IsInUse()
                || go->GetGoState() != GO_STATE_READY)
                SkipRpgTarget("Go is not spawned, ready or is in use.");

            //Ignore objects that are too high or low compared to the bot.
            if (fabs(go->GetPositionZ() - bot->GetPositionZ()) > 20.f)
                SkipRpgTarget("Go is too high/low compared to bot.");
        }
        else if (guidP.IsPlayer())
        {
            Player* player = guidP.GetPlayer();

            //Ignore players that can not be found.
            if (!player)
                SkipRpgTarget("Can not find player.");

            //Ignore yourself.
            if (player == bot)
                SkipRpgTarget("Player is bot.");

            if (player->GetPlayerbotAI())
            {
                GuidPosition guidPP = PAI_VALUE(GuidPosition, "rpg target");

                //Ignore bots that are rpg'ing with another player. This is to prevent two bots rpg-ing with eachother.
                if (guidPP.IsPlayer())
                    SkipRpgTarget("Player has bot as rpg target.");

                //Leaders are not allowed to rpg with groupmembers to prevent follow from making members run away.
                if (bot->GetGroup() && player->GetGroup() == bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetObjectGuid()))
                    SkipRpgTarget("Player is group member and I am leader of same group.");
            }
        }

        //Limit the amount of bots that can rpg with 1 target. Only if the calculation doesn't involve checking 200+ players.
        if (possiblePlayers.size() < 200 && HasSameTarget(guidP, urand(5, 15), possiblePlayers))
        {
            sametarget++;
            SkipRpgTarget("Too many bots are rpging with this npc.");
        }

        //For all rpg actions that are triggered/possible for this target get the highest relevance.
        float relevance = getMaxRelevance(guidP);

        //If this rpg target is our travel target increase the relevance by 50% to make it more likely to be picked.
        if (isTravelTarget)
        {
            if (focusList.empty())
                relevance *= 1.5f;
            else
                relevance *= 10.0f;
        }

        //If we already had a different target with a relevance above 1 and this only has 1 (trivial) skip this target.
        if (!hasGoodRelevance || relevance > 1.0f)
        {
            float newRelevance = relevance;

            //Scale relevance based on distance
            hasGoodRelevance = true;

            if (!isTravelTarget || focusList.empty())
            {
                newRelevance = newRelevance / (1 + sqrt(guidP.sqDistance(bot)) / 30);

                if (relevance > 1.0f && newRelevance <= 1.0f)
                    newRelevance = 1.01f;
            }

            targets[guidP] = newRelevance;
        }

        checked++;
    }

    if (debug)
    {
        std::sort(targetList.begin(), targetList.end(), [targets](ObjectGuid i, ObjectGuid j) {return targets.at(i) < targets.at(j); });

        for (auto& target : targetList)
        {
            GuidPosition guidP(target, bot->GetMapId(), bot->GetInstanceId());
            std::ostringstream out;
            out << chat->formatGuidPosition(guidP, bot) << " " << targets.at(target) << " ";

            if (debugTargets[target].empty())
                out << rgpActionReason[target];
            else
                out << debugTargets[target];

            ai->TellPlayerNoFacing(requester, out);
        }
    }

    return targets;
}

//This method will temporary set the rpg target and finds the highest rpg action relevance that 'could' be triggered when near the target.
float ChooseRpgTargetAction::getMaxRelevance(GuidPosition guidP)
{
    GuidPosition currentRpgTarget = AI_VALUE(GuidPosition, "rpg target");
    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    Strategy* rpgStrategy;

    std::list<TriggerNode*> triggerNodes;

    float maxRelevance = 0.0f;

    //Loop over all strategies containing rpg that are enabled.
    for (auto& strategy : ai->GetAiObjectContext()->GetSupportedStrategies())
    {
        if (strategy.find("rpg") == std::string::npos)
            continue;

        if (!ai->HasStrategy(strategy, BotState::BOT_STATE_NON_COMBAT))
            continue;

        rpgStrategy = ai->GetAiObjectContext()->GetStrategy(strategy);

        rpgStrategy->InitTriggers(triggerNodes, BotState::BOT_STATE_NON_COMBAT);

        //Loop over all triggers of this strategy.
        for (auto& triggerNode : triggerNodes)
        {
            Trigger* trigger = context->GetTrigger(triggerNode->getName());

            if (trigger)
            {
                triggerNode->setTrigger(trigger);

                if (triggerNode->getFirstRelevance() < maxRelevance || triggerNode->getFirstRelevance() > 2.0f)
                    continue;

                Trigger* trigger = triggerNode->getTrigger();

                if (!trigger->IsActive())
                    continue;

                NextAction** nextActions = triggerNode->getHandlers();

                bool isRpg = false;

                //Loop over all actions triggered by this trigger and check if any is an 'rpg action'.
                for (int32 i = 0; i < NextAction::size(nextActions); i++)
                {
                    NextAction* nextAction = nextActions[i];

                    Action* action = ai->GetAiObjectContext()->GetAction(nextAction->getName());

                    if (dynamic_cast<RpgEnabled*>(action))
                        isRpg = true;
                }
                NextAction::destroy(nextActions);

                //Note the highest relevance of this node and the reason it triggers.
                if (isRpg)
                {
                    maxRelevance = triggerNode->getFirstRelevance();
                    rgpActionReason[guidP] = triggerNode->getName();
                }
            }
        }

        for (std::list<TriggerNode*>::iterator i = triggerNodes.begin(); i != triggerNodes.end(); i++)
        {
            TriggerNode* trigger = *i;
            delete trigger;
        }

        triggerNodes.clear();
    }

    SET_AI_VALUE(GuidPosition,"rpg target", currentRpgTarget);

    if (!maxRelevance)
        return 0.0;

    //Normalize the relevances from normal engine level to rpg level for better comparison with distance.
    //Ie. trivial actions have 1.001f => 1, normal actions have 1.050f => 50.
    return floor((maxRelevance - 1.0) * 1000.0f);
}

bool ChooseRpgTargetAction::Execute(Event& event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    std::unordered_map<ObjectGuid, float> targets = GetTargets(requester);

    if (targets.empty())
        return false;

    GuidPosition masterRpgTarget;
    if (requester && ai->IsSafe(requester) && requester->GetPlayerbotAI())
    {
        Player* player = requester;
        masterRpgTarget = PAI_VALUE(GuidPosition, "rpg target");
    }

    //Enable range check for rpg triggers.
    SET_AI_VALUE(std::string, "next rpg action", "");

    bool hasGoodRelevance = false;
    for (auto& [target, relevance] : targets)
    {
        if (relevance > 1.0f)
        {
            hasGoodRelevance = true;
            break;
        }
    }

    uint32 checked = 0;
    for (auto it = begin(targets); it != end(targets);)
    {        
        if (it->second >= 0)
            checked++;

        bool shouldErase = false;
        if (it->second <= 0) //Remove empty targets.
            shouldErase = true;
        else if (hasGoodRelevance && it->second <= 1.0)  //Remove useless targets if there's any good ones.
            shouldErase = true;
        else if (!hasGoodRelevance && requester && requester != bot && (!masterRpgTarget || it->first != masterRpgTarget)) //Remove useless targets if it's not masters target. This prevents group members running all over the place to emote with random npc's which aren't near the master.
            shouldErase = true;

        if (shouldErase)
            it = targets.erase(it);
        else
            ++it;
    }

    //We have no targets. Quit.
    if (targets.empty())
    {
        if (ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
        {
            std::ostringstream out;
            out << "found: no targets, " << checked << " checked.";
            ai->TellPlayerNoFacing(requester, out);
        }
        RESET_AI_VALUE(std::set<ObjectGuid>&,"ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    //Report the list of potential targets and their relevance and reason for interaction.
    if (ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT))
    {
        std::vector<std::pair<ObjectGuid, float>> sortedTargets(targets.begin(), targets.end());

        std::sort(sortedTargets.begin(), sortedTargets.end(), [](std::pair<ObjectGuid, float>i, std::pair<ObjectGuid, float> j) {return i.second > j.second; });

        ai->TellPlayerNoFacing(requester, "------" + std::to_string(targets.size()) + "------");

        uint32 checked = 0;

        for (auto target : sortedTargets)
        {
            GuidPosition guidP(target.first, bot->GetMapId(), bot->GetInstanceId());

            if (!guidP.GetWorldObject(bot->GetInstanceId()))
                continue;

            std::ostringstream out;
            out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));

            out << std::fixed << std::setprecision(2);
            out << " " << rgpActionReason[guidP] << " " << target.second;

            ai->TellPlayerNoFacing(requester, out);

            checked++;

            if (checked >= 10)
            {
                std::ostringstream out;
                out << "and " << (sortedTargets.size()-checked) << " more...";
                ai->TellPlayerNoFacing(requester, out);
                break;
            }
        }
    }

    std::set<ObjectGuid>& ignoreList = AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");

    std::vector<ObjectGuid> guidps;
    std::vector<float> relevances;

    //If we have only trivial targets ignore them all the next time we look for a new target.
    for (auto& target : targets)
    {
        if (!target.second || target.second < 0)
            continue;

        guidps.push_back(target.first);
        relevances.push_back(target.second);

        if (target.second == 1)
            ignoreList.insert(target.first);
    }

    //If we can't find a target clear ignore list and try again later.
    if (guidps.empty())
    {
        RESET_AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    //We pick a random target from the list with targets having a higher relevance of being picked.
    std::mt19937 gen(time(0));
    WeightedShuffle(guidps.begin(), guidps.end(), relevances.begin(), relevances.end(), gen);
    GuidPosition guidP(guidps.front(),bot->GetMapId(), bot->GetInstanceId());

    //If we can't find a target clear ignore list and try again later.
    if (!guidP)
    {
        RESET_AI_VALUE(std::set<ObjectGuid>&, "ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    //Report the target that was found.
    if ((ai->HasStrategy("debug", BotState::BOT_STATE_NON_COMBAT) || ai->HasStrategy("debug rpg", BotState::BOT_STATE_NON_COMBAT)) && guidP.GetWorldObject(bot->GetInstanceId()))
    {
        std::ostringstream out;
        out << "found: ";
        out << chat->formatWorldobject(guidP.GetWorldObject(bot->GetInstanceId()));

        out << std::fixed << std::setprecision(2);
        out << " " << rgpActionReason[guidP] << " " << targets[guidP];

        ai->TellPlayerNoFacing(requester, out);
    }

    //Save the current rpg target and the ignorelist.
    SET_AI_VALUE(GuidPosition, "rpg target", guidP);
    ignoreList.clear();

    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    if (!ai->AllowActivity(RPG_ACTIVITY))
        return false;

    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    if (guidP && guidP.distance(bot) < sPlayerbotAIConfig.reactDistance * 2)
        return false;

    if (AI_VALUE(bool, "travel target traveling"))
        return false;

    if (AI_VALUE(std::list<ObjectGuid>, "possible rpg targets").empty())
        return false;

    //Not stay, not guard, not combat, not trading and group ready.
    if (!AI_VALUE(bool, "can move around"))
        return false;

    //Check if nearby rpg targets (max rpg distance) can be inside the free move range at al.
    float range = AI_VALUE(float, "free move range");
    if(range)
    {
        GuidPosition center(AI_VALUE(GuidPosition, "free move center"));

        if (center.getMapId() != bot->GetMapId())
            return false;

        if (center.sqDistance2d(bot) > range * range + sPlayerbotAIConfig.rpgDistance * sPlayerbotAIConfig.rpgDistance)
            return false;
    }

    return true;
}