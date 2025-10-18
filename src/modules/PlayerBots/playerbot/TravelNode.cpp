#include "TravelNode.h"
#include "playerbot/TravelMgr.h"

#include <iomanip>
#include <regex>

#include "Globals/ObjectMgr.h"
#include "PlayerbotAI.h"
#include "MotionGenerators/MoveMapSharedDefines.h"
#include "MotionGenerators/PathFinder.h"
#include "Entities/Transports.h"
#include "strategy/values/BudgetValues.h"
#include "playerbot/ServerFacade.h"
#include "MotionGenerators/MoveMap.h"

using namespace ai;
using namespace MaNGOS;

//TravelNodePath(float distance = 0.1f, float extraCost = 0, TravelNodePathType pathType = TravelNodePathType::walk, uint64 pathObject = 0, bool calculated = false, std::vector<uint8> maxLevelCreature = { 0,0,0 }, float swimDistance = 0)
std::string TravelNodePath::print()
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(1);
    out << distance << "f,";
    out << extraCost << "f,";
    out << std::to_string(uint8(pathType)) << ",";
    out << pathObject << ",";
    out << (calculated ? "true" : "false") << ",";
    out << std::to_string(maxLevelCreature[0]) << "," << std::to_string(maxLevelCreature[1]) << "," << std::to_string(maxLevelCreature[2]) << ",";
    out << swimDistance << "f";

    return out.str().c_str();
}

//Gets the extra information needed to properly calculate the cost.
void TravelNodePath::calculateCost(bool distanceOnly)
{
    std::unordered_map<FactionTemplateEntry const*, bool> aReact, hReact;

    bool aFriend, hFriend;

    if (calculated)
        return;

    try
    {

        distance = 0.1f;
        maxLevelCreature = { 0,0,0 };
        swimDistance = 0;

        WorldPosition lastPoint = WorldPosition();
        for (auto& point : path)
        {
            if (!distanceOnly)
                for (auto& creaturePair : point.getCreaturesNear(50)) //Agro radius + 5
                {
                    CreatureData const cData = creaturePair->second;
                    CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(cData.id);

                    if (cInfo)
                    {
                        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->Faction);

                        if (aReact.find(factionEntry) == aReact.end())
                            aReact.insert(std::make_pair(factionEntry, PlayerbotAI::friendToAlliance(factionEntry)));
                        aFriend = aReact.find(factionEntry)->second;

                        if (hReact.find(factionEntry) == hReact.end())
                            hReact.insert(std::make_pair(factionEntry, PlayerbotAI::friendToHorde(factionEntry)));
                        hFriend = hReact.find(factionEntry)->second;

                        if (maxLevelCreature[0] < cInfo->MaxLevel && !aFriend && !hFriend)
                            maxLevelCreature[0] = cInfo->MaxLevel;
                        if (maxLevelCreature[1] < cInfo->MaxLevel && aFriend && !hFriend)
                            maxLevelCreature[1] = cInfo->MaxLevel;
                        if (maxLevelCreature[2] < cInfo->MaxLevel && !aFriend && hFriend)
                            maxLevelCreature[2] = cInfo->MaxLevel;
                    }
                }

            if (lastPoint && point.getMapId() == lastPoint.getMapId())
            {
                if (!distanceOnly && (point.isVmapLoaded() && point.isInWater()) || (lastPoint.isVmapLoaded() && lastPoint.isInWater()))
                    swimDistance += point.distance(lastPoint);

                distance += point.distance(lastPoint);
            }

            lastPoint = point;
        }

        if (!distanceOnly)
            calculated = true;
    }
    catch (...)
    {
    }
}

//The cost to travel this path. 
float TravelNodePath::getCost(Unit* unit, uint32 cGold)
{
    float modifier = 1.0f; //Global modifier
    float timeCost = 0.1f;
    float runDistance = distance - swimDistance;
    float speed = 8.0f; //default run speed
    float swimSpeed = 4.0f; //default swim speed.

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        if (path.size() && path.back().getMapId() == 530 && bot->GetLevel() < 58) //Outland
            return -1;

        if (path.size() && path.back().getMapId() == 571 && bot->GetLevel() < 68) //Northrend
            return -1;

        //Check if we can use this area trigger.
        if (getPathType() == TravelNodePathType::areaTrigger && pathObject)
        {
            uint32 triggerId = getPathObject();
            AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(pathObject);
            AreaTrigger const* at = sObjectMgr.GetAreaTrigger(pathObject);
            if (atEntry && at && atEntry->mapid == bot->GetMapId())
            {
                Map* map = WorldPosition(atEntry->mapid, atEntry->box_x, atEntry->box_y, atEntry->box_z).getMap(bot->GetInstanceId());
                if (map)
                    if (at && at->conditionId && !sObjectMgr.IsConditionSatisfied(at->conditionId, bot, map, nullptr, CONDITION_FROM_AREATRIGGER_TELEPORT))
                        return -1;
            }
        }

        if (getPathType() == TravelNodePathType::staticPortal && pathObject)
        {
            uint32 goEntry = getPathObject();

            auto data = sGOStorage.LookupEntry<GameObjectInfo>(goEntry);

            if (!data)
                return -1;

            FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(data->faction);

            if(factionEntry)
                if (PlayerbotAI::GetFactionReaction(factionEntry, bot->GetFactionTemplateEntry()) < REP_NEUTRAL)
                    return -1;
        }

        if (getPathType() == TravelNodePathType::flightPath && pathObject)
        {
            if (!bot->IsAlive())
                return -1;

            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

            if (taxiPath)
            {

                if (!bot->isTaxiCheater() && taxiPath->price > cGold)
                    return -1;

                if (!bot->isTaxiCheater() && !bot->m_taxi.IsTaximaskNodeKnown(taxiPath->to))
                    return -1;

                TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
                TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);

                if (!startTaxiNode || !endTaxiNode || !startTaxiNode->MountCreatureID[bot->GetTeam() == ALLIANCE ? 1 : 0] || !endTaxiNode->MountCreatureID[bot->GetTeam() == ALLIANCE ? 1 : 0])
                    return -1;
            }
            else
            {
                if (!PlayerbotAI::CanSpellClick(bot, pathObject)) //Click Spell flightpath.
                    return -1;
            }
        }

        speed = bot->GetSpeed(MOVE_RUN);
        swimSpeed = bot->GetSpeed(MOVE_SWIM);

        if (bot->HasSpell(1066))
            swimSpeed *= 1.5;

        uint32 level = bot->GetLevel();
        bool isAlliance = PlayerbotAI::friendToAlliance(bot->GetFactionTemplateEntry());

        int factionAnnoyance = 0;

        if (maxLevelCreature.size() > 0)
        {
            int mobAnnoyance = (maxLevelCreature[0] - level) - 10; //Mobs 10 levels below do not bother us.

            if (isAlliance)
                factionAnnoyance = (maxLevelCreature[2] - level) - 10;              //Opposite faction below 10 do not bother us.
            else if (!isAlliance)
                factionAnnoyance = (maxLevelCreature[1] - level) - 10;

            if (mobAnnoyance > 0)
                modifier += 0.1 * mobAnnoyance;     //For each level the whole path takes 10% longer.
            if (factionAnnoyance > 0)
                modifier += 0.3 * factionAnnoyance; //For each level the whole path takes 30% longer.
        }
    }
    else if (getPathType() == TravelNodePathType::flightPath)
        return -1;


    if (getPathType() != TravelNodePathType::walk)
        timeCost = extraCost * modifier;
    else
        timeCost = (runDistance / speed + swimDistance / swimSpeed) * modifier;

    return timeCost;
}

uint32 TravelNodePath::getPrice()
{
    if (getPathType() != TravelNodePathType::flightPath)
        return 0;

    if (!pathObject)
        return 0;

    TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(pathObject);

    if (!taxiPath)
        return 0;

    return taxiPath->price;
}


uint32 TravelNode::getAreaTriggerId()
{
    for (auto link : *getLinks())
    {
        if (link.second->getPathType() != TravelNodePathType::areaTrigger)
            continue;

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(link.second->getPathObject());
        if (!atEntry)
            continue;

        WorldPosition inPos = WorldPosition(atEntry->mapid, atEntry->x, atEntry->y, atEntry->z - 4.0f, 0);

        if (*getPosition() == inPos)
            return link.second->getPathObject();
    }

    return 0;
}

bool TravelNode::isAreaTriggerTarget(uint32 areaTriggerId)
{
    for (uint32 i = 0; i < sAreaTriggerStore.GetNumRows(); i++)
    {
        if (areaTriggerId && areaTriggerId != i)
            continue;

        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(i);
        if (!atEntry)
            continue;

        AreaTrigger const* at = sObjectMgr.GetAreaTrigger(i);
        if (!at)
            continue;

        WorldPosition outPos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);

        if (*getPosition() == outPos)
            return true;
    }

    return false;
}

//Creates or appends the path from one node to another. Returns if the path.
TravelNodePath* TravelNode::buildPath(TravelNode* endNode, Unit* bot, bool postProcess)
{
    if (getMapId() != endNode->getMapId())
        return nullptr;

    TravelNodePath* returnNodePath;

    returnNodePath = getPathTo(endNode);                    

    if (returnNodePath->getComplete())                      
        return returnNodePath;

    std::vector<WorldPosition> path = returnNodePath->getPath();

    WorldPosition startPos = *getPosition();     
    WorldPosition endPos = *endNode->getPosition();

    if (path.empty())
        path = { startPos };                      

    path = endPos.getPathFromPath(path, bot);     
    bool canPath = endPos.isPathTo(path);         

    //Cheat a little for walk -> portal/transport.
    if (!canPath && !isTransport() && !getAreaTriggerId() && (endNode->getAreaTriggerId() || endNode->isTransport()))
    {
        if (endPos.isPathTo(path, 20.0f))
        {
            if(path.back().distance(endPos) > 1.0f)
                path.push_back((endPos+path.back())*0.5f);
            path.push_back(endPos);
            canPath = true;
        }
    }

    if (canPath && path.size() == 2 && this->getDistance(endNode) > 5.0f) //Very small path probably bad pathfinder or flying. Stop using it.
        canPath = false;

    if (canPath && path.size() > 2) //Do not allow the path to slope too much at the end (pathfinder cheating)
    {
        WorldPosition firstPos = path.front();
        WorldPosition secondPos = path[1];

        float vDist = fabs(firstPos.getZ() - secondPos.getZ());
        float hDist = firstPos.fDist(secondPos);

        if (vDist > 10 && (hDist == 0 || vDist / hDist > 2))
            canPath = false;
        else
        {
            WorldPosition firstPos = path.back();
            WorldPosition secondPos = path[path.size() - 2];

            float vDist = fabs(firstPos.getZ() - secondPos.getZ());
            float hDist = firstPos.fDist(secondPos);

            if (vDist > 10 && (hDist == 0 || vDist / hDist > 2))
                canPath = false;
        }
    }

    returnNodePath->setPath(path);
    returnNodePath->setComplete(canPath);

    TravelNodePath* backNodePath; //Get/Build the reverse path.

    if (!endNode->hasPathTo(this))
        backNodePath = endNode->buildPath(this, bot, postProcess);
    else
        backNodePath = endNode->getPathTo(this);

    if (!canPath)
    {
        std::vector<WorldPosition> backPath = backNodePath->getPath();

        if (backPath.size())
        {
            if (backNodePath->getComplete()) //Reverse works so use that.
            {
                //MANGOS_ASSERT(startPos.isPathTo(backPath));
                std::reverse(backPath.begin(), backPath.end());
                path = backPath;
                canPath = backNodePath->getComplete();
            }
            else  if (path.back().distance(backPath.back()) < 5.0f) //Both paths are nearly touching. Make a jump.
            {
                std::reverse(backPath.begin(), backPath.end());
                path.insert(path.end(), backPath.begin(), backPath.end());
                canPath = true;
            }
        }
    }
   
    returnNodePath->setPath(path);
    returnNodePath->setComplete(canPath);

    if (canPath && !hasLinkTo(endNode))
        setLinkTo(endNode);

    if (!returnNodePath->getCalculated())
    {
        returnNodePath->calculateCost(!postProcess);
    }

    return returnNodePath;
}


//Generic routine to remove references to nodes. 
void TravelNode::removeLinkTo(TravelNode* node, bool removePaths) {

    if (node) //Unlink this specific node
    {
        if (removePaths)
            paths.erase(node);

        links.erase(node);
        routes.erase(node);
    }
    else { //Remove all references to this node.        
        for (auto& node : sTravelNodeMap.getNodes())
        {
            if (node->hasPathTo(this))
                node->removeLinkTo(this, removePaths);
        }
        links.clear();
        paths.clear();
        routes.clear();
    }
}

std::vector<TravelNode*> TravelNode::getNodeMap(bool importantOnly, std::vector<TravelNode*> ignoreNodes, bool mapOnly)
{
    std::vector<TravelNode*> openList;
    std::vector<TravelNode*> closeList;

    openList.push_back(this);

    uint32 i = 0;

    while (i < openList.size())
    {
        TravelNode* currentNode = openList[i];

        i++;

        if (!importantOnly || currentNode->isImportant())
            closeList.push_back(currentNode);

        for (auto& nextPath : *currentNode->getLinks())
        {
            TravelNode* nextNode = nextPath.first;

            if (mapOnly && nextNode->getMapId() != getMapId())
                continue;

            if (std::find(openList.begin(), openList.end(), nextNode) != openList.end())
                continue;

            if (!ignoreNodes.empty() && std::find(ignoreNodes.begin(), ignoreNodes.end(), nextNode) != ignoreNodes.end())
                continue;

            openList.push_back(nextNode);
        }
    }

    return closeList;
}

bool TravelNode::isUselessLink(TravelNode* farNode)
{
    if (getPathTo(farNode)->getPathType() != TravelNodePathType::walk)
        return false;

    float farLength;
    TravelNodePath* farPath = nullptr;
    if (hasLinkTo(farNode))
    {
        farPath = getPathTo(farNode);
        farLength = farPath->getDistance();

        if (uint32 areaTriggerId = getAreaTriggerId()) //If area triggers are linked to a nearby exit remove all other links.
        {
            bool farIsTarget = farNode->isAreaTriggerTarget();

            for (auto& link : *getLinks())
            {
                TravelNode* nearNode = link.first;

                if (farNode == nearNode)
                    continue;

                if (farNode->getMapId() != nearNode->getMapId())
                    continue;

                WorldPosition nearPos = *nearNode->getPosition();
                float nearLength = link.second->getDistance();

                if (!farIsTarget && nearNode->isAreaTriggerTarget() && nearLength < 20.0f)
                    return true;
            }
        }
    }
    else
        farLength = getDistance(farNode);

    for (auto& link : *getLinks())
    {
        TravelNode* nearNode = link.first;
        WorldPosition nearPos = *nearNode->getPosition();
        float nearLength = link.second->getDistance();

        if (farNode == nearNode)
            continue;

        if (farNode->hasLinkTo(this) && !nearNode->hasLinkTo(this))
            continue;

        if (nearNode->getAreaTriggerId()) //This node might get removed later, can not use it as a faster route.
            continue;

        if (nearNode->hasLinkTo(farNode))
        {
            //Is it quicker to go past second node to reach first node instead of going directly?
            if (nearLength + nearNode->linkDistanceTo(farNode) < farLength * 1.1)
                return true;

            //Does the path come across the nearby node.
            if (farPath)
                if (nearPos.closestSq(farPath->getPath()).distance(nearPos) < INTERACTION_DISTANCE)
                    return true;
        }
        else
        {
            if (!nearNode->hasRouteTo(farNode, true))
                continue;

            if (nearNode->getAreaTriggerId()) //This node might get removed later, can not use it as a faster route.
                continue;

            TravelNodeRoute route = sTravelNodeMap.getRoute(nearNode, farNode, nullptr);

            if (route.isEmpty())
                continue;

            if (route.hasNode(this))
                continue;

            //Is it quicker to go past second (and multiple) nodes to reach the first node instead of going directly?
            if (nearLength + route.getTotalDistance() < farLength * 1.1)
                return true;
        }
    }

    return false;
}

bool TravelNode::cropUselessLinks()
{
    bool hasRemoved = false;

    for (auto& firstLink : *getPaths())
    {
        TravelNode* farNode = firstLink.first;
        if (this->hasLinkTo(farNode) && this->isUselessLink(farNode))
        {
            this->removeLinkTo(farNode, false);
            farNode->removeLinkTo(this, false);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                std::ostringstream out;
                out << getName() << ",";
                out << farNode->getName() << ",";
                WorldPosition().printWKT({ *getPosition(),*farNode->getPosition() },out,1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }
        if (farNode->hasLinkTo(this) && farNode->isUselessLink(this))
        {
            farNode->removeLinkTo(this, false);
            this->removeLinkTo(farNode, false);
            hasRemoved = true;

            if (sPlayerbotAIConfig.hasLog("crop.csv"))
            {
                std::ostringstream out;
                out << getName() << ",";
                out << farNode->getName() << ",";
                WorldPosition().printWKT({ *getPosition(),*farNode->getPosition() }, out,1);
                out << std::fixed;

                sPlayerbotAIConfig.log("crop.csv", out.str().c_str());
            }
        }
    }

    return hasRemoved;

    /*

    //vector<std::pair<TravelNode*, TravelNode*>> toRemove;
    for (auto& firstLink : getLinks())
    {


        TravelNode* firstNode = firstLink.first;
        float firstLength = firstLink.second.getDistance();
        for (auto& secondLink : getLinks())
        {
            TravelNode* secondNode = secondLink.first;
            float secondLength = secondLink.second.getDistance();

            if (firstNode == secondNode)
                continue;

            if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](std::pair<TravelNode*, TravelNode*> pair) {return pair.first == firstNode || pair.first == secondNode;}) != toRemove.end())
                continue;

            if (firstNode->hasLinkTo(secondNode))
            {
                //Is it quicker to go past first node to reach second node instead of going directly?
                if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
            else
            {
                TravelNodeRoute route = sTravelNodeMap.getRoute(firstNode, secondNode, false);

                if (route.isEmpty())
                    continue;

                if (route.hasNode(this))
                    continue;

                //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going directly?
                if (firstLength + route.getLength() < secondLength * 1.1)
                {
                    if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                        continue;

                    toRemove.push_back(make_pair(this, secondNode));
                }
            }
        }

        //Reverse cleanup. This is needed when we add a node in an existing map.
        if (firstNode->hasLinkTo(this))
        {
            firstLength = firstNode->getPathTo(this)->getDistance();

            for (auto& secondLink : firstNode->getLinks())
            {
                TravelNode* secondNode = secondLink.first;
                float secondLength = secondLink.second.getDistance();

                if (this == secondNode)
                    continue;

                if (std::find(toRemove.begin(), toRemove.end(), [firstNode, secondNode](std::pair<TravelNode*, TravelNode*> pair) {return pair.first == firstNode || pair.first == secondNode; }) != toRemove.end())
                    continue;

                if (firstNode->hasLinkTo(secondNode))
                {
                    //Is it quicker to go past first node to reach second node instead of going directly?
                    if (firstLength + firstNode->linkLengthTo(secondNode) < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
                else
                {
                    TravelNodeRoute route = sTravelNodeMap.getRoute(firstNode, secondNode, false);

                    if (route.isEmpty())
                        continue;

                    if (route.hasNode(this))
                        continue;

                    //Is it quicker to go past first (and multiple) nodes to reach the second node instead of going directly?
                    if (firstLength + route.getLength() < secondLength * 1.1)
                    {
                        if (secondNode->hasLinkTo(this) && !firstNode->hasLinkTo(this))
                            continue;

                        toRemove.push_back(make_pair(this, secondNode));
                    }
                }
            }
        }

    }

    for (auto& nodePair : toRemove)
        nodePair.first->unlinkNode(nodePair.second, false);
        */
}

bool TravelNode::isEqual(TravelNode* compareNode)
{
    if (!hasLinkTo(compareNode))
        return false;

    if (!compareNode->hasLinkTo(this))
        return false;

    for (auto& node : sTravelNodeMap.getNodes())
    {
        if (node == this || node == compareNode)
            continue;

        if (node->hasLinkTo(this) != node->hasLinkTo(compareNode))
            return false;

        if (hasLinkTo(node) != compareNode->hasLinkTo(node))
            return false;
    }

    return true;
}

void TravelNode::print(bool printFailed)
{
    WorldPosition* startPosition = getPosition();

    uint32 mapSize = getNodeMap(true).size();

    std::ostringstream out;
    std::string name = getName();
    name.erase(remove(name.begin(), name.end(), '\"'), name.end());
    out << name.c_str() << ",";
    out << std::fixed << std::setprecision(2);
    point.printWKT(out);
    out << getZ() << ",";
    out << getO() << ",";
    out << (isImportant() ? 1 : 0) << ",";
    out << mapSize;

    sPlayerbotAIConfig.log("travelNodes.csv", out.str().c_str());

    std::vector<WorldPosition> ppath;

    for (auto& endNode : sTravelNodeMap.getNodes())
    {
        if (endNode == this)
            continue;

        if (!hasPathTo(endNode))
            continue;

        TravelNodePath* path = getPathTo(endNode);

        if (!hasLinkTo(endNode) && urand(0, 20) && !printFailed)
            continue;

        ppath = path->getPath();

        if (ppath.size() < 2 && hasLinkTo(endNode))
        {
            ppath.push_back(point);
            ppath.push_back(*endNode->getPosition());
        }

        if (ppath.size() > 1)
        {
            std::ostringstream out;

            uint32 pathType = 1;
            if (!hasLinkTo(endNode) && !path->getComplete())
                pathType = 0;
            else if (path->getPathType() == TravelNodePathType::transport)
                pathType = 2;
            else if (path->getPathType() == TravelNodePathType::areaTrigger && getMapId() == endNode->getMapId())
                pathType = 3;
            else if (path->getPathType() == TravelNodePathType::areaTrigger)
                pathType = 4;
            else if (path->getPathType() == TravelNodePathType::flightPath)
                pathType = 5;
            else if (!hasLinkTo(endNode))
                pathType = 6;
            else if (path->getPathType() == TravelNodePathType::staticPortal)
                pathType = 7;

            out << pathType << ",";
            out << std::fixed << std::setprecision(2);
            point.printWKT(ppath, out, 1);
            out << path->getPathObject() << ",";
            out << path->getDistance() << ",";
            out << path->getCost() << ",";
            out << (path->getComplete() ? 0 : 1) << ",";
            out << std::to_string(path->getMaxLevelCreature()[0])<< ",";
            out << std::to_string(path->getMaxLevelCreature()[1]) << ",";
            out << std::to_string(path->getMaxLevelCreature()[2]) << ",";

            name = getName();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << name.c_str() << ",";

            name = endNode->getName();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << name.c_str();

            sPlayerbotAIConfig.log("travelPaths.csv", out.str().c_str());
        }
    }
}

//Attempts to move ahead of the path.
bool TravelPath::makeShortCut(WorldPosition startPos, float maxDist, Unit* bot)
{
    if (getPath().empty())
        return false;
    float maxDistSq = maxDist * maxDist;
    float minDist = -1;
    float totalDist = fullPath.begin()->point.sqDistance(startPos);
    std::vector<PathNodePoint> newPath;
    WorldPosition firstNode;

    for (auto& p : fullPath) //cycle over the full path
    {
        //if (p.point.getMapId() != startPos.getMapId())
        //    continue;

        if (p.point.getMapId() == startPos.getMapId())
        {
            float curDist = p.point.sqDistance(startPos);

            if (&p != &fullPath.front())
                totalDist += p.point.sqDistance(std::prev(&p)->point);

            if (curDist < sPlayerbotAIConfig.tooCloseDistance * sPlayerbotAIConfig.tooCloseDistance) //We are on the path. This is a good starting point
            {
                minDist = curDist;
                totalDist = curDist;
                newPath.clear();
            }

            if (p.type != PathNodeType::NODE_PREPATH) //Only look at the part after the first node and in the same map.
            {
                if (!firstNode)
                    firstNode = p.point;

                if (minDist == -1 || curDist < minDist || (curDist < maxDistSq && curDist < totalDist / 2)) //Start building from the last closest point or a point that is close but far on the path.
                {
                    minDist = curDist;
                    totalDist = curDist;
                    newPath.clear();
                }
            }
        }

        newPath.push_back(p);
    }

    if (newPath.empty() || minDist > maxDistSq || newPath.front().point.getMapId() != startPos.getMapId())
    {
        clear();
        return false;
    }

    WorldPosition beginPos = newPath.begin()->point;

    //The old path seems to be the best.
    if (beginPos.distance(firstNode) < sPlayerbotAIConfig.tooCloseDistance)
        return false;

    //We are (nearly) on the new path. Just follow the rest.
    if (beginPos.distance(startPos) < sPlayerbotAIConfig.tooCloseDistance)
    {
        fullPath = newPath;
        return true;
    }

    std::vector<WorldPosition> toPath = startPos.getPathTo(beginPos, bot);

    //We can not reach the new begin position. Follow the complete path.
    if (!beginPos.isPathTo(toPath))
        return false;

    //Move to the new path and continue.
    fullPath.clear();
    addPath(toPath);
    addPath(newPath);

    return true;
}

bool TravelPath::shouldMoveToNextPoint(WorldPosition startPos, std::vector<PathNodePoint>::iterator beg, std::vector<PathNodePoint>::iterator ed, std::vector<PathNodePoint>::iterator p, float& moveDist, float maxDist)
{
    if (p == ed) //We are the end. Stop now.
        return false;

    auto nextP = std::next(p);

    // Fix assertion fail due to nextP to be invalidate
    if (nextP == ed) //We are the end. Stop now.
        return false;

    //We are moving to a area trigger node and want to move to the next teleport node.
    if (p->type == PathNodeType::NODE_AREA_TRIGGER && nextP->type == PathNodeType::NODE_AREA_TRIGGER && p->entry == nextP->entry)
    {
        return false; //Move to teleport and activate area trigger.
    }

    //We are moving to a area trigger node and want to move to the next teleport node.
    if (p->type == PathNodeType::NODE_STATIC_PORTAL && nextP->type == PathNodeType::NODE_STATIC_PORTAL && p->entry == nextP->entry)
    {
        return false; //Move to teleport and activate area trigger.
    }

    //We are using a hearthstone.
    if (p->type == PathNodeType::NODE_TELEPORT && nextP->type == PathNodeType::NODE_TELEPORT && p->entry == nextP->entry)
    {
        return false; //Use the teleport
    }

    //We are almost at a transport node. Move to the node before this.   
    if (nextP->type == PathNodeType::NODE_TRANSPORT && nextP->entry)
    {
        return false;
    }
    
    //We are moving to a transport node.
    if (p->type == PathNodeType::NODE_TRANSPORT && p->entry)
    {
        if (nextP->type != PathNodeType::NODE_TRANSPORT && p != beg && std::prev(p)->type != PathNodeType::NODE_TRANSPORT) //We are not using the transport. Skip it.
            return true;

        return false; //Teleport to exit of transport.
    }

    //We are moving to a flightpath and want to fly.
    if (p->type == PathNodeType::NODE_FLIGHTPATH && nextP->type == PathNodeType::NODE_FLIGHTPATH)
    {
        return false;
    }

    float nextMove = p->point.distance(nextP->point);

    if (p->point.getMapId() != startPos.getMapId() || ((moveDist + nextMove > maxDist || startPos.distance(nextP->point) > maxDist) && moveDist > 0))
    {
        return false;
    }

    moveDist += nextMove;

    return true;
}

//Next position to move to
WorldPosition TravelPath::getNextPoint(WorldPosition startPos, float maxDist, TravelNodePathType& pathType, uint32& entry, bool onTransport, WorldPosition& telePosition)
{
    if (getPath().empty())
        return WorldPosition();

    auto beg = fullPath.begin();
    auto ed = fullPath.end();

    float minDist = 0.0f;
    auto startP = beg;

    //Get the closest point on the path to start from.
    for (auto p = startP; p != ed; p++)
    {
        if (p->point.getMapId() != startPos.getMapId())
            continue;

        float curDist = p->point.distance(startPos);

        if (curDist <= minDist || p == beg)
        {
            minDist = curDist;
            startP = p;
        }
    }

    float moveDist = startP->point.distance(startPos);

    //Move as far as we are allowed
    for (auto p = startP; p != ed; p++)
    {
        if (shouldMoveToNextPoint(startPos, beg, ed, p, moveDist, maxDist))
            continue;

        startP = p;

        break;
    }

    auto prevP = startP, nextP = startP;
    if (startP != beg)
        prevP = std::prev(prevP);
    if (std::next(nextP) != ed)
        nextP = std::next(nextP);

    //We are moving towards a teleport. Move to portal an activate area trigger
    if (startP->type == PathNodeType::NODE_AREA_TRIGGER)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = startP->entry;
        return startP->point;
    }

    //We are moving towards a static portal. Move to portal and use it.
    if (startP->type == PathNodeType::NODE_STATIC_PORTAL && startPos.distance(startP->point) < INTERACTION_DISTANCE)
    {
        pathType = TravelNodePathType::staticPortal;
        entry = startP->entry;
        return startP->point;
    }

    //We are using a hearthstone
    if (nextP->type == PathNodeType::NODE_TELEPORT)
    {
        pathType = TravelNodePathType::teleportSpell;
        entry = startP->entry;
        return startP->point;
    }

    //We are moving towards a flight path. Move to flight master and activate flight path.
    if (startP->type == PathNodeType::NODE_FLIGHTPATH && startPos.distance(startP->point) < INTERACTION_DISTANCE)
    {
        pathType = TravelNodePathType::flightPath;
        entry = startP->entry;
        return startP->point;
    }

    if (nextP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = 0;

        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                return p->point;              //We want to move here.
            }
        }
    }

    if (startP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::areaTrigger;
        entry = 0;

        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                return p->point;              //We want to move here.
            }
        }
    }

    //We are on or very near a transport.
    if (startP->type == PathNodeType::NODE_TRANSPORT)
    {
        
        
        if (onTransport) //We are moving with the transport
        {
            pathType = TravelNodePathType::transport;
            entry = startP->entry;
        }
        else//We are probably in the water. Just teleport now.
        {
            pathType = TravelNodePathType::areaTrigger;
            entry = 0;
        }
        
        for (auto p = startP + 1; p != ed; p++) //Move along the transport path to the end of the boat ride. 
        {
            if (p->type != PathNodeType::NODE_TRANSPORT)
            {
                telePosition = prevP->point;  //Boat needs to be here
                return p->point;              //We want to move here.
            }
            prevP = p;
        }
    }

    //We are about the enter a transport.
    if (nextP->type == PathNodeType::NODE_TRANSPORT)
    {
        pathType = TravelNodePathType::transport;
        entry = nextP->entry;
        telePosition = nextP->point; //Boat needs to be here.
        return startP->point;        //We want to stand somewhere here.
    } 

    //We have to move far for next point. Try to make a cropped path.
    if (moveDist < sPlayerbotAIConfig.targetPosRecalcDistance && std::next(startP) != ed)
    {
        //vector<WorldPosition> path = startPos.getPathTo(std::next(startP)->point, nullptr);
        //startP->point = startPos.lastInRange(path, -1, maxDist);
        return WorldPosition();
    }

    return startP->point;
}

std::ostringstream TravelPath::print()
{
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00," << "1,";
    out << std::fixed;

    WorldPosition().printWKT(getPointPath(), out, 1);

    return out;
}

float TravelNodeRoute::getTotalDistance()
{
    float totalLength = 0;
    for (uint32 i = 0; i < nodes.size() - 2; i++)
    {
        totalLength += nodes[i]->linkDistanceTo(nodes[i + 1]);
    }

    return totalLength;
}

TravelPath TravelNodeRoute::buildPath(std::vector<WorldPosition> pathToStart, std::vector<WorldPosition> pathToEnd, Unit* bot)
{
    TravelPath travelPath;

    Unit* botForPath = bot;

    if (!pathToStart.empty()) //From start position to start of path.
        travelPath.addPath(pathToStart, PathNodeType::NODE_PREPATH);

    TravelNode* prevNode = nullptr;
    for (auto& node : nodes)
    {
        if (prevNode)
        {
            TravelNodePath* nodePath = nullptr;
            if (prevNode->hasPathTo(node))  //Get the path to the next node if it exists.
                nodePath = prevNode->getPathTo(node);

            if (!nodePath || !nodePath->getComplete()) //Build the path to the next node if it doesn't exist.
            {
                if (!prevNode->isTransport())
                    nodePath = prevNode->buildPath(node, botForPath);
                else //For transports we have no proper path since the node is in air/water. Instead we build a reverse path and follow that.
                {
                    node->buildPath(prevNode, botForPath); //Reverse build to get proper path.
                    nodePath = prevNode->getPathTo(node);
                }
            }

            TravelNodePath returnNodePath;

            if (!nodePath || !nodePath->getComplete()) //It looks like we can't properly path to our node. Make a temporary reverse path and see if that works instead.
            {
                returnNodePath = *node->buildPath(prevNode, botForPath); //Build reverse path and save it to a temporary variable.
                std::vector<WorldPosition> path = returnNodePath.getPath();
                reverse(path.begin(), path.end()); //Reverse the path 
                returnNodePath.setPath(path);
                nodePath = &returnNodePath;
            }

            if (!nodePath || !nodePath->getComplete()) //If we can not build a path just try to move to the node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_NODE);
            }
            else if (nodePath->getPathType() == TravelNodePathType::areaTrigger) //Teleport to next node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject()); //Entry point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_AREA_TRIGGER, nodePath->getPathObject());     //Exit point
            }
            else if (nodePath->getPathType() == TravelNodePathType::staticPortal) //Teleport to next node.
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject()); //Entry point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_STATIC_PORTAL, nodePath->getPathObject());     //Exit point
            }
            else if (nodePath->getPathType() == TravelNodePathType::transport) //Move onto transport
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject()); //Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TRANSPORT, nodePath->getPathObject());     //Arrival point        
            }
            else if (nodePath->getPathType() == TravelNodePathType::flightPath) //Use the flightpath
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject()); //Departure point
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_FLIGHTPATH, nodePath->getPathObject());     //Arrival point        
            }
            else if (nodePath->getPathType() == TravelNodePathType::teleportSpell)
            {
                travelPath.addPoint(*prevNode->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
                travelPath.addPoint(*node->getPosition(), PathNodeType::NODE_TELEPORT, nodePath->getPathObject());
            }
            else
            {
                std::vector<WorldPosition> path = nodePath->getPath();

                if (path.size() > 1 && node != nodes.back()) //Remove the last point since that will also be the start of the next path.
                    path.pop_back();

                if (path.size() > 1 && prevNode->isPortal() && nodePath->getPathType() != TravelNodePathType::areaTrigger) //Do not move to the area trigger if we don't plan to take the portal.
                    path.erase(path.begin());

                if (path.size() > 1 && prevNode->isTransport() && nodePath->getPathType() != TravelNodePathType::transport) //Do not move to the transport if we aren't going to take it.
                    path.erase(path.begin());

                travelPath.addPath(path, PathNodeType::NODE_PATH);
            }
        }
        prevNode = node;
    }

    if (!pathToEnd.empty())
        travelPath.addPath(pathToEnd, PathNodeType::NODE_PATH);

    return travelPath;
}

std::ostringstream TravelNodeRoute::print()
{
    std::ostringstream out;

    out << sPlayerbotAIConfig.GetTimestampStr();
    out << "+00" << ",0," << "\"LINESTRING(";

    for (auto& node : nodes)
    {
        out << std::fixed << node->getPosition()->getDisplayX() << " " << node->getPosition()->getDisplayY() << ",";
    }

    out << ")\"";

    return out;
}

TravelNodeMap::TravelNodeMap(TravelNodeMap* baseMap)
{
    TravelNode* newNode;

    baseMap->m_nMapMtx.lock_shared();

    for (auto& node : baseMap->getNodes())
    {
        addNode(*node->getPosition(), node->getName(), node->isImportant(), true, node->isTransport(), node->getTransportId());
    }

    for (auto& node : baseMap->getNodes())
    {
        newNode = getNode(node);

        for (auto& path : *node->getPaths())
        {
            TravelNode* endNode = getNode(path.first);

            newNode->setPathTo(endNode, path.second);
        }
    }

    baseMap->m_nMapMtx.unlock_shared();
}

TravelNode* TravelNodeMap::addNode(WorldPosition pos, std::string preferedName, bool isImportant, bool checkDuplicate, bool transport, uint32 transportId)
{
    TravelNode* newNode;

    if (checkDuplicate)
    {
        newNode = getNode(pos, nullptr, 5.0f);
        if (newNode)
            return newNode;
    }

    std::string finalName = preferedName;

    if (!isImportant)
    {
        std::regex last_num("[[:digit:]]+$");
        finalName = std::regex_replace(finalName, last_num, "");
        uint32 nameCount = 1;

        for (auto& node : getNodes(pos))
        {
            if (node->getName().find(preferedName + std::to_string(nameCount)) != std::string::npos)
                nameCount++;
        }

        if(nameCount)
            finalName += std::to_string(nameCount);
    }

    newNode = new TravelNode(pos, finalName, isImportant);

    m_nodes.push_back(newNode);
    m_map_nodes[pos.getMapId()].push_back(newNode);

    return newNode;
}

void TravelNodeMap::removeNode(TravelNode* node)
{
    node->removeLinkTo(NULL, true);

    uint32 mapId = node->getMapId();

    for (auto& tnode : m_map_nodes[mapId])
    {
        if (tnode == node)
        {
            delete tnode;
            tnode = nullptr;
        }
    }

    for (auto& tnode : m_nodes)
    {
        if (tnode == node)
        {
            tnode = nullptr;
        }
    }

    m_nodes.erase(std::remove(m_nodes.begin(), m_nodes.end(), nullptr), m_nodes.end());
    m_map_nodes[mapId].erase(std::remove(m_map_nodes[mapId].begin(), m_map_nodes[mapId].end(), nullptr), m_map_nodes[mapId].end());
}

void TravelNodeMap::fullLinkNode(TravelNode* startNode, Unit* bot)
{
    WorldPosition* startPosition = startNode->getPosition();
    std::vector<TravelNode*> linkNodes = getNodes(*startPosition);

    for (auto& endNode : linkNodes)
    {
        if (endNode == startNode)
            continue;

        if (startNode->hasLinkTo(endNode))
            continue;

        startNode->buildPath(endNode, bot);
        endNode->buildPath(startNode, bot);
    }

    startNode->setLinked(true);
}

std::vector<TravelNode*> TravelNodeMap::getNodes(WorldPosition pos, float range)
{
    std::vector<TravelNode*> retVec;
    for (auto& node : m_map_nodes[pos.getMapId()])
    {
        if (range == -1 || node->getDistance(pos) <= range)
            retVec.push_back(node);
    }

    std::sort(retVec.begin(), retVec.end(), [pos](TravelNode* i, TravelNode* j) { return i->getPosition()->distance(pos) < j->getPosition()->distance(pos); });
    return retVec;
}

TravelNode* TravelNodeMap::getNode(WorldPosition pos, std::vector<WorldPosition>& ppath, Unit* bot, float range)
{
    float x = pos.getX();
    float y = pos.getY();
    float z = pos.getZ();

    if (bot && !bot->IsInWorld())
        return NULL;

    uint32 c = 0;

    std::vector<TravelNode*> nodes = sTravelNodeMap.getNodes(pos, range);
    for (auto& node : nodes)
    {
        if (!bot || pos.canPathTo(*node->getPosition(), bot))
            return node;

        c++;

        if (c > 5) //Max 5 attempts
            break;
    }

    return NULL;
}

TravelNodeRoute TravelNodeMap::getRoute(TravelNode* start, TravelNode* goal, Unit* unit)
{
    float unitSpeed = unit ? unit->GetSpeed(MOVE_RUN) : 7.0f;

    if (start == goal)
        return TravelNodeRoute();

    if(!start->hasRouteTo(goal))
        return TravelNodeRoute();

    //Basic A* algoritm
    std::unordered_map<TravelNode*, TravelNodeStub> m_stubs;

    TravelNodeStub* startStub = &m_stubs.insert(std::make_pair(start, TravelNodeStub(start))).first->second;

    TravelNodeStub* currentNode, * childNode;

    float f, g, h;

    std::vector<TravelNodeStub*> open, closed;

    std::vector<TravelNode*> portNodes;

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        PlayerbotAI* ai = bot->GetPlayerbotAI();
        if (ai)
        {
            AiObjectContext* context = ai->GetAiObjectContext();

            if (ai->HasCheat(BotCheatMask::gold))
                startStub->currentGold = 10000000;
            else {
                startStub->currentGold = 9999999;
                for (ObjectGuid guid : AI_VALUE(std::list<ObjectGuid>, "group members"))
                {
                    Player* player = sObjectMgr.GetPlayer(guid);

                    if (!player)
                        continue;

                    if (!ai->IsGroupLeader() && player != bot)
                        continue;

                    if (player->IsBeingTeleported())
                    {
                        startStub->currentGold = 0;
                        continue;
                    }

                    if (!player->GetPlayerbotAI())
                        continue;

                    startStub->currentGold = std::min(startStub->currentGold, PAI_VALUE2(uint32, "free money for", (uint32)NeedMoneyFor::travel));
                }
            }

            if (AI_VALUE2(bool, "action useful", "hearthstone") && bot->IsAlive())
            {
                TravelNode* homeNode = sTravelNodeMap.getNode(AI_VALUE(WorldPosition, "home bind"), nullptr, 50.0f);
                if (homeNode)
                {
                    PortalNode* portNode = new PortalNode(start);
                    portNode->SetPortal(start, homeNode, 8690);

                    childNode = &m_stubs.insert(std::make_pair(portNode, TravelNodeStub(portNode))).first->second;

                    childNode->m_g = std::max((uint32)2, (10 - AI_VALUE(uint32, "death count")) * MINUTE); //If we can walk there in 10 minutes, walk instead.
                    childNode->m_h = childNode->dataNode->fDist(goal) / unitSpeed;
                    childNode->m_f = childNode->m_g + childNode->m_h;

                    open.push_back(childNode);
                    std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
                    childNode->open = true;
                    portNodes.push_back(portNode);
                }
            }
        }
        else
            startStub->currentGold = bot->GetMoney();       

        std::vector<uint32> teleSpells = { 3561,3562,3563,3565,3566,3567,18960 };

        for (auto spellId : teleSpells)
        {
            if (!bot->IsAlive())
                continue;

            if (bot->IsInCombat())
                continue;

            if (!bot->HasSpell(spellId))
                continue;

            if (!sServerFacade.IsSpellReady(bot, spellId))
                continue;

            if (!sSpellMgr.GetSpellTargetPosition(spellId))
                continue;

            if (ai)
            {
                AiObjectContext* context = ai->GetAiObjectContext();
                if (AI_VALUE2(uint32, "has reagents for", spellId) == 0)
                    continue;
            }

            WorldPosition telePos(sSpellMgr.GetSpellTargetPosition(spellId));

            TravelNode* homeNode = sTravelNodeMap.getNode(telePos, nullptr, 10.0f);

            if (!homeNode)
                continue;

            PortalNode* portNode = new PortalNode(start);
            portNode->SetPortal(start, homeNode, spellId);

            childNode = &m_stubs.insert(std::make_pair(portNode, TravelNodeStub(portNode))).first->second;

            childNode->m_g = MINUTE; //If we can walk there in a minute. Walk instead.
            childNode->m_h = childNode->dataNode->fDist(goal) / unitSpeed;
            childNode->m_f = childNode->m_g + childNode->m_h;

            open.push_back(childNode);
            std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
            childNode->open = true;
            portNodes.push_back(portNode);
        }
    }

    if (open.size() == 0 && !start->hasRouteTo(goal))
    {
        for (auto node : portNodes) delete node;
        return TravelNodeRoute();
    }

    std::make_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });

    open.push_back(startStub);
    std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
    startStub->open = true;

    while (!open.empty())
    {
        std::sort(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });

        currentNode = open.front(); // pop n node from open for which f is minimal

        std::pop_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
        open.pop_back();
        currentNode->open = false;

        currentNode->close = true;
        closed.push_back(currentNode);

        if (currentNode->dataNode == goal)
        {
            TravelNodeStub* parent = currentNode->parent;

            std::vector<TravelNode*> path;

            path.push_back(currentNode->dataNode);

            while (parent != nullptr)
            {
                path.push_back(parent->dataNode);
                parent = parent->parent;
            }

            std::reverse(path.begin(), path.end());

            return TravelNodeRoute(path, portNodes);
        }

        for (const auto& link : *currentNode->dataNode->getLinks())// for each successor n' of n
        {
            TravelNode* linkNode = link.first;

            float linkCost = link.second->getCost(unit, currentNode->currentGold);

            if (linkCost <= 0)
                continue;

            childNode = &m_stubs.insert(std::make_pair(linkNode, TravelNodeStub(linkNode))).first->second;

            g = currentNode->m_g + linkCost; // stance from start + distance between the two nodes
            if ((childNode->open || childNode->close) && childNode->m_g <= g) // n' is already in opend or closed with a lower cost g(n')
                continue; // consider next successor
            float distance = childNode->dataNode->fDist(goal);

            if (distance == FLT_MAX)
                continue;

            h = distance / unitSpeed;
            f = g + h; // compute f(n')
            childNode->m_f = f;
            childNode->m_g = g;
            childNode->m_h = h;
            childNode->parent = currentNode;

            if (bot && !bot->isTaxiCheater())
                childNode->currentGold = currentNode->currentGold - link.second->getPrice();

            if (childNode->close)
                childNode->close = false;
            if (!childNode->open)
            {
                open.push_back(childNode);
                std::push_heap(open.begin(), open.end(), [](TravelNodeStub* i, TravelNodeStub* j) {return i->m_f < j->m_f; });
                childNode->open = true;
            }
        }
    }

    for (auto node : portNodes) delete node;

    return TravelNodeRoute();
}

TravelNodeRoute TravelNodeMap::getRoute(WorldPosition startPos, WorldPosition endPos, std::vector<WorldPosition>& startPath, Unit* unit)
{
    if (m_nodes.empty())
        return TravelNodeRoute();

    std::vector<WorldPosition> newStartPath;
    std::vector<TravelNode*> startNodes = getNodes(startPos), endNodes = getNodes(endPos);

    if(!startNodes.size() || !endNodes.size())
        return TravelNodeRoute();

    uint32 startNr = std::min(5, (int)startNodes.size()-1);
    uint32 endNr = std::min(5, (int)endNodes.size()-1);

    //Partial sort to get the closest 5 nodes at the begin of the array.        
    std::partial_sort(startNodes.begin(), startNodes.begin() + startNr, startNodes.end(), [startPos](TravelNode* i, TravelNode* j) {return i->getPosition()->sqDistance(startPos) < j->getPosition()->sqDistance(startPos); });
    std::partial_sort(endNodes.begin(), endNodes.begin() + endNr, endNodes.end(), [endPos](TravelNode* i, TravelNode* j) {return i->getPosition()->sqDistance(endPos) < j->getPosition()->sqDistance(endPos); });

    //Cycle over the combinations of these 5 nodes.
    uint32 startI = 0, endI = 0;
    while (startI <= startNr && endI <= endNr)
    {
        TravelNode* startNode = startNodes[startI];
        TravelNode* endNode = endNodes[endI];

        WorldPosition startNodePosition = *startNode->getPosition();
        WorldPosition endNodePosition = *endNode->getPosition();

        float maxStartDistance = startNode->isTransport() ? 20.0f : sPlayerbotAIConfig.targetPosRecalcDistance;

        TravelNodeRoute route = getRoute(startNode, endNode, unit);

        if (!route.isEmpty())
        {
            //Check if the bot can actually walk to this start position.
            newStartPath = startPath;
            if (startNodePosition.cropPathTo(newStartPath, maxStartDistance) || startNode->getPosition()->isPathTo(newStartPath = startPos.getPathTo(startNodePosition, unit), maxStartDistance))
            {
                startPath = newStartPath;
                return route;
            }
            startI++;
        }

        //Prefer a differnt end-node. 
        endI++;

        //Cycle to a different start-node if needed.
        if (endI > startI + 1)
        {
            startI++;
            endI = 0;
        }
    }

    Player* bot = dynamic_cast<Player*>(unit);
    if (bot)
    {
        Player* player = bot;
        if (PAI_VALUE2(bool, "action useful", "hearthstone"))
        {
            startPath.clear();
            TravelNode* botNode = new TravelNode(startPos, "Bot Pos", false);
            botNode->setPoint(startPos);

            endI = 0;
            while (endI <= endNr)
            {
                TravelNode* endNode = endNodes[endI];
                TravelNodeRoute route = getRoute(botNode, endNode, bot);
                route.addTempNodes({ botNode });

                if (!route.isEmpty())
                    return route;
                endI++;
            }
        }
    }

    return TravelNodeRoute();
}

TravelPath TravelNodeMap::getFullPath(WorldPosition startPos, WorldPosition endPos, Unit* unit)
{
    TravelPath movePath;
    std::vector<WorldPosition> beginPath, endPath;

    beginPath = endPos.getPathFromPath({ startPos }, unit, 40);

    if (endPos.isPathTo(beginPath,sPlayerbotAIConfig.spellDistance)) //If we can get within spell distance a longer route won't help.
        return TravelPath(beginPath);

    //[[Node pathfinding system]]
                //We try to find nodes near the bot and near the end position that have a route between them.
                //Then bot has to move towards/along the route.
    sTravelNodeMap.m_nMapMtx.lock_shared();

    //Find the route of nodes starting at a node closest to the start position and ending at a node closest to the endposition.
    //Also returns longPath: The path from the start position to the first node in the route.
    TravelNodeRoute route = sTravelNodeMap.getRoute(startPos, endPos, beginPath, unit);

    if (route.isEmpty())
    {
        route.cleanTempNodes();
        return movePath;
    }

    if (sPlayerbotAIConfig.hasLog("bot_pathfinding.csv"))
    {
        Player* bot = dynamic_cast<Player*>(unit);
        if (bot)
        {
            PlayerbotAI* ai = bot->GetPlayerbotAI();
            if (ai)
            {
                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
                    sPlayerbotAIConfig.log("bot_pathfinding.csv", route.print().str().c_str());
                }
            }
        }
    }

    //endPath = route.getNodes().back()->getPosition()->getPathTo(endPos, bot);
    endPath = { *route.getNodes().back()->getPosition(),endPos };
    movePath = route.buildPath(beginPath, endPath);

    route.cleanTempNodes();

    if (sPlayerbotAIConfig.hasLog("bot_pathfinding.csv"))
    {
        Player* bot = dynamic_cast<Player*>(unit);
        if (bot)
        {
            PlayerbotAI* ai = bot->GetPlayerbotAI();
            if (ai)
            {
                if (ai->HasStrategy("debug move", BotState::BOT_STATE_NON_COMBAT))
                {
                    sPlayerbotAIConfig.openLog("bot_pathfinding.csv", "w");
                    sPlayerbotAIConfig.log("bot_pathfinding.csv", movePath.print().str().c_str());
                }
            }
        }
    }

    sTravelNodeMap.m_nMapMtx.unlock_shared();

    return movePath;
}

bool TravelNodeMap::cropUselessNode(TravelNode* startNode)
{
    if (!startNode->isLinked() || startNode->isImportant())
        return false;

    std::vector<TravelNode*> ignore = { startNode };

    for (auto& node : getNodes(*startNode->getPosition(), 5000))
    {
        if (startNode == node)
            continue;

        if (node->getNodeMap(true).size() > node->getNodeMap(true, ignore).size())
            return false;
    }

    removeNode(startNode);

    return true;
}

TravelNode* TravelNodeMap::addZoneLinkNode(TravelNode* startNode)
{
    for (auto& path : *startNode->getPaths())
    {


        TravelNode* endNode = path.first;

        std::string zoneName = startNode->getPosition()->getAreaName(true, true);
        for (auto& pos : path.second.getPath())
        {
            std::string newZoneName = pos.getAreaName(true, true);
            if (zoneName != newZoneName)
            {
                if (!getNode(pos, NULL, 100.0f))
                {
                    std::string nodeName = zoneName + " to " + newZoneName;
                    return sTravelNodeMap.addNode(pos, nodeName, false, true);
                }
                zoneName = newZoneName;
            }

        }
    }

    return NULL;
}

TravelNode* TravelNodeMap::addRandomExtNode(TravelNode* startNode)
{
    std::unordered_map<TravelNode*, TravelNodePath> paths = *startNode->getPaths();

    if (paths.empty())
        return NULL;

    for (uint32 i = 0; i < 20; i++)
    {
        auto random_it = std::next(std::begin(paths), urand(0, paths.size() - 1));

        TravelNode* endNode = random_it->first;
        std::vector<WorldPosition> path = random_it->second.getPath();

        if (path.empty())
            continue;

        //Prefer to skip complete links
        if (endNode->hasLinkTo(startNode) && startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        //Prefer to skip no links
        if (!startNode->hasLinkTo(endNode) && !urand(0, 20))
            continue;

        WorldPosition point = path[urand(0, path.size() - 1)];

        if (!getNode(point, NULL, 100.0f))
            return sTravelNodeMap.addNode(point, startNode->getName(), false, true);
    }

    return NULL;
}

void TravelNodeMap::manageNodes(Unit* bot, bool mapFull)
{
    bool rePrint = false;

    if (!bot->IsInWorld())
        return;

    if (m_nMapMtx.try_lock())
    {

        TravelNode* startNode;
        TravelNode* newNode;

        for (auto startNode : m_nodes)
        {
            cropUselessNode(startNode);
        }

        //Pick random Node
        for (uint32 i = 0; i < (mapFull ? (uint32)20 : (uint32)1); i++)
        {
            std::vector<TravelNode*> rnodes = getNodes(WorldPosition(bot));

            if (!rnodes.empty())
            {
                uint32 j = urand(0, rnodes.size() - 1);

                startNode = rnodes[j];
                newNode = NULL;

                bool nodeDone = false;

                if (!nodeDone)
                    nodeDone = cropUselessNode(startNode);

                if (!nodeDone && !urand(0, 20))
                    newNode = addZoneLinkNode(startNode);

                if (!nodeDone && !newNode && !urand(0, 20))
                    newNode = addRandomExtNode(startNode);

                rePrint = nodeDone || rePrint || newNode;
            }

        }

        if (rePrint && (mapFull || !urand(0, 20)))
            printMap();

        m_nMapMtx.unlock();
    }

    sTravelNodeMap.m_nMapMtx.lock_shared();

    if (!rePrint && mapFull)
        printMap();

    m_nMapMtx.unlock_shared();
}

void TravelNodeMap::LoadMaps()
{
    sLog.outError("Trying to load all maps and tiles for node generation. Please ignore any maps that could not be loaded.");
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;

        uint32 mapId = sMapStore.LookupEntry(i)->MapID;
        if (mapId == 0 || mapId == 1 || mapId == 530 || mapId == 571)
        {
#ifndef MANGOSBOT_TWO
            MMAP::MMapFactory::createOrGetMMapManager()->loadAllMapTiles(sWorld.GetDataPath(), mapId);
#else
            MMAP::MMapFactory::createOrGetMMapManager()->loadAllMapTiles(sWorld.GetDataPath(), mapId, 0);
#endif
        }
        else
        {
            MMAP::MMapFactory::createOrGetMMapManager()->loadMapInstance(sWorld.GetDataPath(), mapId, 0);
        }
    }

#ifndef MANGOSBOT_TWO
    for (uint32 i = 0; i < sMapStore.GetNumRows(); ++i)
    {
        if (!sMapStore.LookupEntry(i))
            continue;

        uint32 mapId = sMapStore.LookupEntry(i)->MapID;

        for (const auto& entry : boost::filesystem::directory_iterator(sWorld.GetDataPath() + "mmaps"))
        {
            if (entry.path().extension() == ".mmtile")
            {
                auto filename = entry.path().filename();
                auto fileNameString = filename.c_str();
                // trying to avoid string copy
                uint32 fileMapId = (fileNameString[0] - '0') * 100 + (fileNameString[1] - '0') * 10 + (fileNameString[2] - '0');
                if (fileMapId != mapId)
                    continue;

                uint32 x = (fileNameString[3] - '0') * 10 + (fileNameString[4] - '0');
                uint32 y = (fileNameString[5] - '0') * 10 + (fileNameString[6] - '0');

                if (!MMAP::MMapFactory::createOrGetMMapManager()->IsMMapIsLoaded(mapId, x, y))
                    MMAP::MMapFactory::createOrGetMMapManager()->loadMap(sWorld.GetDataPath(), mapId, x, y);
            }
        }
    }
#endif
}

void TravelNodeMap::generateNpcNodes()
{
    std::unordered_map<uint32, GuidPosition> bossMap;

    for (auto& creaturePair : WorldPosition().getCreaturesNear())
    {
        GuidPosition guidP(creaturePair);
        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(guidP.GetEntry());

        if (!cInfo)
            continue;

        uint32 flagMask = UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_FLIGHTMASTER | UNIT_NPC_FLAG_SPIRITHEALER | UNIT_NPC_FLAG_SPIRITGUIDE;

        if (cInfo->NpcFlags & flagMask)
        {
            std::string nodeName = guidP.getAreaName(false);

            if (cInfo->NpcFlags & UNIT_NPC_FLAG_INNKEEPER)
                nodeName += " innkeeper";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_FLIGHTMASTER)
                nodeName += " flightMaster";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_SPIRITHEALER)
                nodeName += " spirithealer";
            else if (cInfo->NpcFlags & UNIT_NPC_FLAG_SPIRITGUIDE)
                nodeName += " spiritguide";

            TravelNode* node = sTravelNodeMap.addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->Rank == 3)
        {
            std::string nodeName = cInfo->Name;

            sTravelNodeMap.addNode(guidP, nodeName, true, true);
        }
        else if (cInfo->Rank == 1 && !guidP.isOverworld())
        {
            if (bossMap.find(cInfo->Entry) == bossMap.end())
                bossMap[cInfo->Entry] = guidP;
            else if (bossMap[cInfo->Entry])
                bossMap[cInfo->Entry] = GuidPosition();
        }
    }

    for (auto boss : bossMap)
    {
        GuidPosition guidP = boss.second;

        if (!guidP)
            continue;

        CreatureInfo const* cInfo = ObjectMgr::GetCreatureTemplate(guidP.GetEntry());

        if (!cInfo)
            continue;

        std::string nodeName = cInfo->Name;

        sTravelNodeMap.addNode(guidP, nodeName, true, true);
    }
}

void TravelNodeMap::generateStartNodes()
{
    std::map<uint8, std::string> startNames;
    startNames[RACE_HUMAN] = "Human";
    startNames[RACE_ORC] = "Orc and Troll";
    startNames[RACE_DWARF] = "Dwarf and Gnome";
    startNames[RACE_NIGHTELF] = "Night Elf";
    startNames[RACE_UNDEAD] = "Undead";
    startNames[RACE_TAUREN] = "Tauren";
    startNames[RACE_GNOME] = "Dwarf and Gnome";
    startNames[RACE_TROLL] = "Orc and Troll";
    startNames[RACE_GOBLIN] = "Goblin";
#ifndef MANGOSBOT_ZERO
    startNames[RACE_BLOODELF] = "Blood elf";
    startNames[RACE_DRAENEI] = "Draenei";
#endif

    for (uint32 i = 0; i < MAX_RACES; i++)
    {
        for (uint32 j = 0; j < MAX_CLASSES; j++)
        {
            PlayerInfo const* info = sObjectMgr.GetPlayerInfo(i, j);

            if (!info)
                continue;

             WorldPosition pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            std::string nodeName = startNames[i] + " start";

            sTravelNodeMap.addNode(pos, nodeName, true, true);

            break;
        }
    }
}

void TravelNodeMap::generateAreaTriggerNodes()
{
    //Entrance nodes

    for (uint32 i = 0; i < sAreaTriggerStore.GetNumRows(); i++)
    {
        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(i);
        if (!atEntry)
            continue;

        AreaTrigger const* at = sObjectMgr.GetAreaTrigger(i);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(atEntry->mapid, atEntry->x, atEntry->y, atEntry->z - 4.0f, 0);

        WorldPosition outPos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);

        std::string nodeName;

        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        sTravelNodeMap.addNode(inPos, nodeName, true, true);
    }

    //Exit nodes

    for (uint32 i = 0; i < sAreaTriggerStore.GetNumRows(); i++)
    {
        AreaTriggerEntry const* atEntry = sAreaTriggerStore.LookupEntry(i);
        if (!atEntry)
            continue;

        AreaTrigger const* at = sObjectMgr.GetAreaTrigger(i);
        if (!at)
            continue;

        WorldPosition inPos = WorldPosition(atEntry->mapid, atEntry->x, atEntry->y, atEntry->z - 4.0f, 0);

        WorldPosition outPos = WorldPosition(at->target_mapId, at->target_X, at->target_Y, at->target_Z, at->target_Orientation);

        std::string nodeName;

        if (!outPos.isOverworld())
            nodeName = outPos.getAreaName(false) + " entrance";
        else if (!inPos.isOverworld())
            nodeName = inPos.getAreaName(false) + " exit";
        else
            nodeName = inPos.getAreaName(false) + " portal";

        TravelNode* outNode = sTravelNodeMap.addNode(outPos, nodeName, true, true); //Exit side, portal exit.

        TravelNode* inNode = sTravelNodeMap.getNode(inPos, nullptr, 5.0f); //Entry side, portal center.

        //Portal link from area trigger to area trigger destination.
        if (outNode && inNode)
        {
            TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::areaTrigger, i, true);
            travelPath.setPath({ *inNode->getPosition(), *outNode->getPosition() });
            inNode->setPathTo(outNode, travelPath);
        }
    }
}

void TravelNodeMap::generatePortalNodes()
{
    //Static portals.
    for (auto goData : WorldPosition().getGameObjectsNear(0, 0))
    {
        GuidPosition go(goData);

        auto data = sGOStorage.LookupEntry<GameObjectInfo>(go.GetEntry());

        if (!data)
            continue;

        if (data->type != GAMEOBJECT_TYPE_SPELLCASTER)
            continue;

        const SpellEntry* pSpellInfo = sServerFacade.LookupSpellInfo(data->spellcaster.spellId);

        if(pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr.GetSpellTargetPosition(pSpellInfo->Id);

        if (!pos)
            continue;

        WorldPosition inPos(go);
        WorldPosition outPos(pos);

        TravelNode* inNode = sTravelNodeMap.addNode(inPos, data->name, true, true);
        TravelNode* outNode = sTravelNodeMap.addNode(outPos, data->name, true, true);

        TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::staticPortal, go.GetEntry(), true);
        travelPath.setPath({ *inNode->getPosition(), *outNode->getPosition() });
        inNode->setPathTo(outNode, travelPath); 
    }

    //Portal spell destinations.
    for (uint32 i = 0; i < GetSpellStore()->GetMaxEntry(); ++i)
    {
        const SpellEntry* pSpellInfo = GetSpellStore()->LookupEntry<SpellEntry>(i);

        if (!pSpellInfo)
            continue;

        if (pSpellInfo->EffectTriggerSpell[0])
            pSpellInfo = sServerFacade.LookupSpellInfo(pSpellInfo->EffectTriggerSpell[0]);

        if (!pSpellInfo)
            continue;

        if (pSpellInfo->Effect[0] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[1] != SPELL_EFFECT_TELEPORT_UNITS && pSpellInfo->Effect[2] != SPELL_EFFECT_TELEPORT_UNITS)
            continue;

        SpellTargetPosition const* pos = sSpellMgr.GetSpellTargetPosition(pSpellInfo->Id);

        if (!pos)
            continue;

        WorldPosition outPos(pos);

        if (outPos.isOverworld() && outPos.currentHeight() > 0.5f && outPos.currentHeight() < 50.0f)
        {
            sLog.outError("%s adjusting height down from %f", pSpellInfo->SpellName[0], outPos.currentHeight());
            outPos.setZ(outPos.getZ() - outPos.currentHeight() + 0.5f);
        }

        TravelNode* destNode = sTravelNodeMap.addNode(outPos, pSpellInfo->SpellName[0], true, true);
    }
}

void TravelNodeMap::makeDockNode(TravelNode* node, WorldPosition pos, std::string dockName)
{
    pos.loadMapAndVMap(0);
    WorldPosition exitPos = pos;

    if (exitPos.ClosestCorrectPoint(20.0f, 1.0f, 0))
    {
        TravelNode* exitNode = getNode(exitPos, nullptr, 1.0f);

        if (!exitNode) //Only add paths if we are adding a new node or 
        {
            exitNode = sTravelNodeMap.addNode(exitPos, node->getName() + dockName, true, false);

            TravelNodePath travelPath(exitPos.distance(pos), 0.1f, (uint8)TravelNodePathType::transport, 0, true); //The path is part of the transport.
            travelPath.setComplete(true);
            travelPath.setPath({ exitPos, pos });
            exitNode->setPathTo(node, travelPath, true);
            travelPath.setPath({ pos, exitPos });
            node->setPathTo(exitNode, travelPath, true);
            node->setLinked(true);
        }
    }
}

void TravelNodeMap::generateTransportNodes()
{
    for (uint32 entry = 1; entry <= sGOStorage.GetMaxEntry(); ++entry)
    {        
        auto data = sGOStorage.LookupEntry<GameObjectInfo>(entry);

        if (data && (data->type == GAMEOBJECT_TYPE_TRANSPORT || data->type == GAMEOBJECT_TYPE_MO_TRANSPORT))
        {
            if (data->displayId == 808) //Remove plunger
                continue;

            TransportAnimation const* animation = sTransportMgr.GetTransportAnimInfo(entry);

            uint32 pathId = data->moTransport.taxiPathId;
            float moveSpeed = data->moTransport.moveSpeed;
            if (pathId >= sTaxiPathNodesByPath.size())
                continue;

            TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

            std::vector<WorldPosition> ppath;
            TravelNode* prevNode = nullptr;

            //Elevators/Trams
            if (path.empty())
            {
                if (animation)
                {
                    TransportPathContainer aPath = animation->Path;
                    float timeStart;

                    for (auto& transport : WorldPosition().getGameObjectsNear(0, entry))
                    {
                        GuidPosition guidP(transport);
                        prevNode = nullptr;
                        WorldPosition basePos(guidP);
                        WorldPosition lPos = WorldPosition();

                        for (auto& p : aPath)
                        {
#ifndef MANGOSBOT_TWO
                            float dx = cos(basePos.getO()) * p.second->X - sin(basePos.getO()) * p.second->Y;
                            float dy = sin(basePos.getO()) * p.second->X + cos(basePos.getO()) * p.second->Y;
#else
                            float dx = -1 * p.second->X;
                            float dy = -1 * p.second->Y;
#endif

                            WorldPosition pos = WorldPosition(basePos.getMapId(), basePos.getX() + dx, basePos.getY() + dy, basePos.getZ() + p.second->Z, basePos.getO());

                            if (prevNode)
                            {
                                ppath.push_back(pos);
                            }

                            if (pos.distance(lPos) == 0)
                            {
                                TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);

                                WorldPosition exitPos = pos;

                                if (data->displayId == 3831) //Subway
                                    exitPos.setZ(exitPos.getZ() - 10.0f);
                                if (data->displayId == 807) //Vator
                                    exitPos.setZ(exitPos.getZ() - 1.25f);
                                if (data->displayId == 455) //Undervator
                                    exitPos.setZ(exitPos.getZ() - 0.46f);

                                makeDockNode(node, exitPos, "entry");

                                if (!prevNode)
                                {
                                    ppath.push_back(pos);
                                    timeStart = p.second->TimeSeg;
                                }
                                else
                                {
                                    float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;

                                    TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::transport, entry, true);
                                    node->setPathTo(prevNode, travelPath);
                                    ppath.clear();
                                    ppath.push_back(pos);
                                    timeStart = p.second->TimeSeg;
                                }

                                prevNode = node;
                            }

                            lPos = pos;
                        }

                        if (prevNode)
                        {
                            for (auto& p : aPath)
                            {
#ifndef MANGOSBOT_TWO
                                float dx = cos(basePos.getO()) * p.second->X - sin(basePos.getO()) * p.second->Y;
                                float dy = sin(basePos.getO()) * p.second->X + cos(basePos.getO()) * p.second->Y;
#else
                                float dx = -1 * p.second->X;
                                float dy = -1 * p.second->Y;
#endif
                                WorldPosition pos = WorldPosition(basePos.getMapId(), basePos.getX() + dx, basePos.getY() + dy, basePos.getZ() + p.second->Z, basePos.getO());

                                ppath.push_back(pos);

                                if (pos.distance(lPos) == 0)
                                {
                                    TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);

                                    WorldPosition exitPos = pos;

                                    if (data->displayId == 3831) //Subway
                                        exitPos.setZ(exitPos.getZ() - 10.0f);
                                    if (data->displayId == 807) //Vator
                                        exitPos.setZ(exitPos.getZ() - 1.24f);
                                    if (data->displayId == 455) //Undervator
                                        exitPos.setZ(exitPos.getZ() - 0.46f);

                                    makeDockNode(node, exitPos, "entry");

                                    if (node != prevNode) {
                                        if (p.second->TimeSeg < timeStart)
                                            timeStart = 0;

                                        float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;

                                        TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::transport, entry, true);
                                        travelPath.setPath(ppath);
                                        node->setPathTo(prevNode, travelPath);
                                        ppath.clear();
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }
                                }

                                lPos = pos;
                            }
                        }

                        ppath.clear();
                    }
                }
            }
            else //Boats/Zepelins
            {
                //Loop over the path and connect stop locations.
                for (auto& p : path)
                {
                    WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                    if (prevNode)
                    {
                        ppath.push_back(pos);
                    }

                    if (p->delay > 0)
                    {
                        TravelNode* node = sTravelNodeMap.addNode(pos, data->name, true, true, true, entry);

                        WorldPosition exitPos = pos;

                        if (data->displayId == 3015)  //Boat
                            exitPos.setZ(exitPos.getZ() + 6.0f);
                        else if (data->displayId == 3031) //Zepelin
                            exitPos.setZ(exitPos.getZ() - 17.0f);
                        else if (data->displayId == 7087) //Moonspray
                            exitPos.setZ(exitPos.getZ() + 4.88f);

                        makeDockNode(node, exitPos, "dock");

                        if (!prevNode)
                        {
                            ppath.push_back(pos);
                        }
                        else
                        {
                            TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                            travelPath.setPathAndCost(ppath, moveSpeed);
                            node->setPathTo(prevNode, travelPath);
                            ppath.clear();
                            ppath.push_back(pos);
                        }

                        prevNode = node;
                    }
                }

                if (prevNode)
                {
                    //Continue from start until first stop and connect to end.
                    for (auto& p : path)
                    {
                        WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                        //if (data->displayId == 3015)
                        //    pos.setZ(pos.getZ() + 6.0f);
                        //else if (data->displayId == 3031)
                        //    pos.setZ(pos.getZ() - 17.0f);

                        ppath.push_back(pos);

                        if (p->delay > 0)
                        {
                            TravelNode* node = sTravelNodeMap.getNode(pos, NULL, 5.0f);

                            if (node != prevNode) {
                                TravelNodePath travelPath(0.1f, 0.0, (uint8)TravelNodePathType::transport, entry, true);
                                travelPath.setPathAndCost(ppath, moveSpeed);

                                node->setPathTo(prevNode, travelPath);
                            }
                        }
                    }
                }
                ppath.clear();
            }
        }
    }
}

void TravelNodeMap::generateZoneMeanNodes()
{
    //Zone means   
    for (auto& [entry, dests] :sTravelMgr.GetExploreLocs())
    {
        std::vector<WorldPosition*> points;
        for (auto& dest : dests)
        {
            for (auto p : dest->GetPoints())
                if (!p->isUnderWater())
                    points.push_back(p);

            if (points.empty())
                points = dest->GetPoints();
        }

        WorldPosition  pos = WorldPosition(points, WP_MEAN_CENTROID);

        TravelNode* node = sTravelNodeMap.addNode(pos, pos.getAreaName(), true, true, false);
    }
}

void TravelNodeMap::addManualNodes()
{
    TravelNode* node;
    node = sTravelNodeMap.addNode(WorldPosition(0, -10416.45f, -3832.53f, -36.92f), "c1-Sunken Temple", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -10408.70f, -3834.29f, -44.69f), "c2-Sunken Temple", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -10325.23f, -3865.86f, -44.45f), "c3-Sunken Temple", true, false);
    //otherNode = sTravelNodeMap.addNode(WorldPosition(0, -10319.19f, -3868.16f, -40.90f), "c4-Sunken temple", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(0, -11367.45f, 1617.10f, 71.22f), "c1-Deadmine exit", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -11367.12f, 1610.48f, 76.63f), "c2-Deadmine exit", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -11381.49f, 1584.11f, 82.10f), "c3-Deadmine exit", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -11379.64f, 1578.79f, 87.84f), "c4-Deadmine exit", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -11340.23f, 1571.61f, 94.44f), "c5-Deadmine exit", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(1, -3034.70f, 144.04f, 70.87f), "c-Tauren start", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(1, -1424.04f, 2945.01f, 134.54f), "c-Maraudon", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(1, 4158.01f, 877.60f, -20.68f), "c1-Blackfathom Deeps", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(1, 4156.60f, 909.89f, -20.97f), "c2-Blackfathom Deeps", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(1, 4157.46f, 916.44f, -17.40f), "c3-Blackfathom Deeps", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(1, -3626.39f, 917.37f, 150.13f), "c1-Dire Maul", true, false);
    TravelNode* otherNode;
    otherNode = sTravelNodeMap.addNode(WorldPosition(1, -3628.08f, 919.55f, 137.84f), "c2-Dire Maul", true, false);
    node->setPathTo(otherNode);

    node = sTravelNodeMap.addNode(WorldPosition(1, -588.53f, -2037.69f, 57.60f), "c1-Wailing Caverns", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(1, 7016.75f, -2153.84f, 595.09f), "c1-Timbermaw Hold", true, false);

#ifndef MANGOSBOT_ZERO
    node = sTravelNodeMap.addNode(WorldPosition(530, 571.10f, 6938.97f, -16.81f), "c1-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 571.10f, 6938.97f, -15.20f), "c2-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 651.07f, 6865.37f, -82.34f), "c3-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 607.17f, 6908.68f, -49.20f), "c4-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 574.68f, 6942.93f, -37.72f), "c5-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 723.74f, 6865.78f, -74.10f), "c6-Coilfang", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(530, 731.57f, 6866.01f, -70.47f), "c7-Coilfang", true, false);
#endif

#ifdef MANGOSBOT_TWO
    //Teleport doodah
    node = sTravelNodeMap.addNode(WorldPosition(0, -7502.20f, -1152.98f, 269.55f), "c1-Blackrock Mountain", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(0, -7591.31f, -1114.44f, 249.91f), "c2-Blackrock Mountain", true, false);

    node = sTravelNodeMap.addNode(WorldPosition(609, 2390.02f, -5640.91f, 377.09f), "c1-Ebon Hold", true, false);
    otherNode = sTravelNodeMap.addNode(WorldPosition(609, 2383.65f, -5645.20f, 420.77f), "c2-Ebon Hold", true, false);

    TravelNodePath travelPath(0.1f, 3.0f, (uint8)TravelNodePathType::areaTrigger, 0, true);
    travelPath.setPath({*node->getPosition(), *otherNode->getPosition()});
    node->setPathTo(otherNode, travelPath);
    travelPath.setPath({*otherNode->getPosition(), *node->getPosition()});
    otherNode->setPathTo(node, travelPath);

    //Scourge gryphons.
    //These use HandleSpellClick
    node = sTravelNodeMap.addNode(WorldPosition(609, 2325.03f, -5659.60f, 382.24f), "c3-Ebon Hold", true, false, false, 29488);
    otherNode = sTravelNodeMap.addNode(WorldPosition(609, 2409.09f, -5722.37f, 154.00f), "c3-Scarlet Enclave", true, false, false, 29501);

    travelPath = TravelNodePath(0.1f, 3.0f, (uint8)TravelNodePathType::flightPath, 29488, true);
    travelPath.setPath({*node->getPosition(), *otherNode->getPosition()});
    node->setPathTo(otherNode, travelPath);
    travelPath = TravelNodePath(0.1f, 3.0f, (uint8)TravelNodePathType::flightPath, 29501, true);
    travelPath.setPath({*otherNode->getPosition(), *node->getPosition()});
    otherNode->setPathTo(node, travelPath);

    node = sTravelNodeMap.addNode(WorldPosition(609, 2348.58f, -5695.35f, 382.24f), "c4-Ebon Hold", true, false, false, 29488);
    otherNode = sTravelNodeMap.addNode(WorldPosition(609, 2402.86f, -5727.03f, 154.00f), "c4-Scarlet Enclave", true, false, false, 29501);

    travelPath = TravelNodePath(0.1f, 3.0f, (uint8)TravelNodePathType::flightPath, 29488, true);
    travelPath.setPath({*node->getPosition(), *otherNode->getPosition()});
    node->setPathTo(otherNode, travelPath);
    travelPath = TravelNodePath(0.1f, 3.0f, (uint8)TravelNodePathType::flightPath, 29501, true);
    travelPath.setPath({*otherNode->getPosition(), *node->getPosition()});
    otherNode->setPathTo(node, travelPath);

    node = sTravelNodeMap.addNode(WorldPosition(609, 2528.22f, -5580.44f, 162.02f), "c1-The Noxious Pass", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(609, 2538.35f, -5573.05f, 162.46f), "c2-The Noxious Pass", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(609, 2546.61f, -5563.89f, 162.88f), "c3-The Noxious Pass", true, false);
    node = sTravelNodeMap.addNode(WorldPosition(609, 2563.52f, -5547.79f, 163.27f), "c4-The Noxious Pass", true, false);
#endif
}

void TravelNodeMap::generateNodes()
{
    sLog.outString("-Generating Start nodes");
    generateStartNodes();
    sLog.outString("-Generating npc nodes");
    generateNpcNodes();
    sLog.outString("-Generating area trigger nodes");
    generateAreaTriggerNodes();
    sLog.outString("-Generating transport nodes");
    generateTransportNodes();
    sLog.outString("-Generating zone mean nodes");
    generateZoneMeanNodes();
    sLog.outString("-Generating static portal nodes");
    generatePortalNodes();
    sLog.outString("-Adding manually defined nodes");
    addManualNodes();
}

void TravelNodeMap::generateWalkPathMap(uint32 mapId, BarGoLink* bar)
{
    for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
    {
        if (bar)
        {
            m_nMapMtx.lock();
            bar->step();
            m_nMapMtx.unlock();
        }

        if (startNode->isLinked())
            continue;
        
        for (auto& endNode : sTravelNodeMap.getNodes(*startNode->getPosition(), 2000.0f))
        {
            if (endNode->isTransport() && endNode->isLinked())
                continue;

            if (startNode == endNode)
                continue;

            if (startNode->hasCompletePathTo(endNode))
                continue;

            if (startNode->getMapId() != endNode->getMapId())
                continue;

            startNode->buildPath(endNode, nullptr, false);
        }

        startNode->setLinked(true);
    }
}

void TravelNodeMap::generateWalkPaths()
{
    //Pathfinder
    std::vector<WorldPosition> ppath;

    std::map<uint32, bool> nodeMaps;

    uint32 nodes = 0;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        nodes++;
        nodeMaps[startNode->getMapId()] = true;
    }

    BarGoLink bar(nodes);

    std::vector<std::future<void>> calculations;

    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this,mapId, &bar] { generateWalkPathMap(mapId, &bar); }));
    }

    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
    }

    sLog.outString(">> Generated paths for " SIZEFMTD " nodes.", sTravelNodeMap.getNodes().size());
}

void TravelNodeMap::generateHelperNodes(uint32 mapId, BarGoLink* bar)
{
    std::vector<TravelNode*> startNodes = getNodes(WorldPosition(mapId, 1, 1));

    std::vector<std::pair<WorldPosition, std::string>> places_to_reach;

    //Find all places we might want to reach.
    for (auto& node : startNodes)
    {
        if (node->isTransport())
            continue;

        if (node->getAreaTriggerId())
            continue;

        if (node->getRouteSize() > 1000)
            continue;

        places_to_reach.push_back(make_pair(GuidPosition(0, *node->getPosition()), node->getName()));
    }

    if (places_to_reach.empty() || startNodes.empty())
        return;

    for (auto& pos : places_to_reach)
    {
        std::vector<TravelNode*> startNodes = getNodes(WorldPosition(mapId, 1, 1));
        //Find closest 5 nodes.
        std::partial_sort(startNodes.begin(), startNodes.begin() + std::min(int(startNodes.size()), 5), startNodes.end(), [pos](TravelNode* i, TravelNode* j) {return i->fDist(pos.first) < j->fDist(pos.first); });

        bool found = false;

        for (uint8 i = 0; i < std::min(int(startNodes.size()), 5); i++)
        {
            TravelNode* node = startNodes[i];

            if (node->isTransport())
                continue;

            if (node->getAreaTriggerId())
                continue;

            if (node->getPosition()->canPathTo(pos.first, nullptr))
                continue;

            TravelNode* otherNode = getNode(pos.first, nullptr, 1.0f);

            if (otherNode && node->hasLinkTo(otherNode))
                continue;

            for (auto& path : *node->getPaths())
            {
                WorldPosition prevPoint;
                for (auto& ppoint : path.second.getPath())
                {
                    if (prevPoint && ppoint.sqDistance2d(prevPoint) < 100.0f)
                        continue;

                    prevPoint = ppoint;

                    if (!ppoint.canPathTo(pos.first, nullptr))
                        continue;

                    std::string name = node->getName() + " to " + pos.second;
                    sTravelNodeMap.addNode(ppoint, name, false, true);
                    found = true;

                    break;
                }

                if (found)
                    break;
            }

            if (found)
            {
                sTravelNodeMap.generateWalkPathMap(mapId, nullptr);
                break;
            }
        }

        if (!found) {
            std::string name = pos.second;
            sTravelNodeMap.addNode(pos.first, name, false, true);
        }

        m_nMapMtx.lock();
        bar->step();
        m_nMapMtx.unlock();
    }

    for (auto& node : startNodes)
    {
        if (!node->isTransport())
            node->setLinked(false);
    }

    sTravelNodeMap.generateWalkPathMap(mapId, nullptr);
}

void TravelNodeMap::generateHelperNodes()
{
    //Pathfinder
    std::vector<WorldPosition> ppath;

    std::map<uint32, bool> nodeMaps;

    uint32 old = sTravelNodeMap.getNodes().size();

    {
        BarGoLink bar(old);

        for (auto& startNode : sTravelNodeMap.getNodes())
        {
            bar.step();
            nodeMaps[startNode->getMapId()] = true;
        }
    }


    if (sTravelNodeMap.getNodes().size() > old)
    {
        sLog.outString("-Calculating walkable paths for %d new nodes.", uint32(sTravelNodeMap.getNodes().size() - old));
        generateWalkPaths();
    }

    uint32 places_to_reach = 0;

    for (auto& map : nodeMaps)
    {
        std::vector<TravelNode*> startNodes = getNodes(WorldPosition(map.first, 1, 1));
        //Find all places we might want to reach.
        for (auto& node : startNodes)
        {
            if (node->isTransport())
                continue;

            if (node->getAreaTriggerId())
                continue;

            if (node->getRouteSize() > 1000)
                continue;

            places_to_reach++;
        }
    }

    sLog.outString("-Finding new nodes to reach %d nodes that can currently not be properly reached.", places_to_reach);

    BarGoLink bar(places_to_reach);

    std::vector<std::future<void>> calculations;

    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this, mapId, &bar] { generateHelperNodes(mapId, &bar); }));
    }

    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
    }

    sLog.outString(">> Generated " SIZEFMTD " helpdernodes.", sTravelNodeMap.getNodes().size()-old);
}

void TravelNodeMap::generateTaxiPaths()
{
    for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
    {
        TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

        if (!taxiPath)
            continue;

        TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);

        if (!startTaxiNode)
            continue;

        TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);

        if (!endTaxiNode)
            continue;

        TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];

        if (nodes.empty())
            continue;

        WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
        WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

        TravelNode* startNode = sTravelNodeMap.getNode(startPos, nullptr, 15.0f);
        TravelNode* endNode = sTravelNodeMap.getNode(endPos, nullptr, 15.0f);

        if (!startNode || !endNode)
            continue;

        std::vector<WorldPosition> ppath;

        for (auto& n : nodes)
            ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

        float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

        TravelNodePath travelPath(0.1f, totalTime, (uint8)TravelNodePathType::flightPath, i, true);
        travelPath.setPath(ppath);

        startNode->setPathTo(endNode, travelPath);
    }
}

void TravelNodeMap::removeLowNodes()
{
    std::vector<TravelNode*> goodNodes;
    std::vector<TravelNode*> remNodes;
    for (auto& node : sTravelNodeMap.getNodes())
    {
        if (!node->getPosition()->isOverworld())
            continue;

        if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
            continue;

        if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
            continue;

        std::vector<TravelNode*> nodes = node->getNodeMap(true);

        if (nodes.size() < 5)
            remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
        else
            goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
    }

    for (auto& node : remNodes)
        sTravelNodeMap.removeNode(node);

    sLog.outString("-Removed %d nodes had below 5 connections to other nodes.", (uint32)remNodes.size());
}

void TravelNodeMap::removeUselessPathMap(uint32 mapId)
{
    //Clean up node links
    for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
    {
        for (auto& path : *startNode->getPaths())
            if (path.second.getComplete() && startNode->hasLinkTo(path.first))
                MANGOS_ASSERT(true);
    }
    uint32 it = 0, rem = 0;
    while (true)
    {
        uint32 rem = 0;
        //Clean up node links
        for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
        {
            if (startNode->cropUselessLinks())
                rem++;
        }

        for (auto& startNode : sTravelNodeMap.getNodes(WorldPosition(mapId, 1, 1)))
        {
            startNode->clearRoutes();
        }

        if (!rem)
            break;

        hasToSave = true;

        it++;

        sLog.outDetail("MapId %d Iteration %d, removed %d", mapId, it, rem);
    }
}

void TravelNodeMap::removeUselessPaths()
{
    //Pathfinder
    std::vector<WorldPosition> ppath;

    std::map<uint32, bool> nodeMaps;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        nodeMaps[startNode->getMapId()] = true;
    }

    std::vector<std::future<void>> calculations;

    BarGoLink bar(nodeMaps.size());
    for (auto& map : nodeMaps)
    {
        uint32 mapId = map.first;
        calculations.push_back(std::async([this, mapId] { removeUselessPathMap(mapId); }));
        bar.step();
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        calculations[i].wait();
        bar2.step();
    }
}

void TravelNodeMap::calculatePathCosts()
{   
    BarGoLink bar(sTravelNodeMap.getNodes().size());

    std::vector<std::future<void>> calculations;

    for (auto& startNode : sTravelNodeMap.getNodes())
    {
        bar.step();

        for (auto& path : *startNode->getLinks())
        {
            TravelNodePath* nodePath = path.second;

            if (path.second->getPathType() != TravelNodePathType::walk)
                continue;

            if (nodePath->getCalculated())
                continue;

            calculations.push_back(std::async([nodePath] {nodePath->calculateCost(); }));
        }
    }

    BarGoLink bar2(calculations.size());
    for (uint32 i = 0; i < calculations.size(); i++)
    {
        bar2.step();
        calculations[i].wait();
    }

    sLog.outString(">> Calculated cost for " SIZEFMTD " paths.", calculations.size());

    if (calculations.size()) //Repeat until we have all paths calculated.
        calculatePathCosts();
}

void TravelNodeMap::generatePaths(bool helpers)
{
    sTravelMgr.SetMobAvoidArea();

    sLog.outString("-Calculating walkable paths");
    generateWalkPaths();

    if (helpers)
    {
        sLog.outString("-Generating helper nodes");
        generateHelperNodes();
    }

    sLog.outString("-Removing useless nodes");
    removeLowNodes();
    sLog.outString("-Removing useless paths");
    removeUselessPaths();

    printMap();

    sLog.outString("-Calculating path costs");
    calculatePathCosts();
    sLog.outString("-Generating taxi paths");
    generateTaxiPaths();
}

void TravelNodeMap::generateAll()
{
    if (hasToGen || hasToFullGen)
        LoadMaps();

    if (hasToFullGen)
        generateNodes();

    sLog.outString("-Calculating mapoffset");
    calcMapOffset();

    sLog.outString("-Generating maptransfers");
    sTravelMgr.LoadMapTransfers();

    if (hasToGen || hasToFullGen)
    {
        generatePaths(false);
        hasToGen = false;
        hasToFullGen = false;
        hasToSave = true;
    }

    sLog.outString("-Calculating coverage"); //This prevents crashes when bots from multiple maps try to calculate this on the fly.
    for (auto& node : getNodes())
        node->hasRouteTo(node);
}

void TravelNodeMap::printMap()
{
    if (!sPlayerbotAIConfig.hasLog("travelNodes.csv") && !sPlayerbotAIConfig.hasLog("travelPaths.csv"))
        return;

    printf("\r [Qgis] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog("travelNodes.csv", "w");
    sPlayerbotAIConfig.openLog("travelPaths.csv", "w");

    std::vector<TravelNode*> anodes = getNodes();

    uint32 nr = 0;

    for (auto& node : anodes)
    {
        node->print(hasToSave);
    }
}

void TravelNodeMap::printNodeStore()
{
    std::string nodeStore = "TravelNodeStore.h";

    if (!sPlayerbotAIConfig.hasLog(nodeStore))
        return;

    printf("\r [Map] \r\x3D");
    fflush(stdout);

    sPlayerbotAIConfig.openLog(nodeStore, "w");

    std::unordered_map<TravelNode*, uint32> saveNodes;

    std::vector<TravelNode*> anodes = getNodes();

    sPlayerbotAIConfig.log(nodeStore, "#pragma once");
    sPlayerbotAIConfig.log(nodeStore, "#include \"TravelMgr.h\"");
    sPlayerbotAIConfig.log(nodeStore, "namespace ai");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "    class TravelNodeStore");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "    public:");
    sPlayerbotAIConfig.log(nodeStore, "    static void loadNodes()");
    sPlayerbotAIConfig.log(nodeStore, "    {");
    sPlayerbotAIConfig.log(nodeStore, "        TravelNode** nodes = new TravelNode*[%d];", anodes.size());

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        std::ostringstream out;

        std::string name = node->getName();
        name.erase(remove(name.begin(), name.end(), '\"'), name.end());

        //        struct addNode {uint32 node; WorldPosition point; std::string name; bool isPortal; bool isTransport; uint32 transportId; };
        out << std::fixed << std::setprecision(2) << "        addNodes.push_back(addNode{" << i << ",";
        out << "WorldPosition(" << node->getMapId() << ", " << node->getX() << "f, " << node->getY() << "f, " << node->getZ() << "f, " << node->getO() << "f),";
        out << "\"" << name << "\"";
        if (node->isTransport())
            out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();
        out << "});";

        /*
                out << std::fixed << std::setprecision(2) << "        nodes[" << i << "] = sTravelNodeMap.addNode(&WorldPosition(" << node->getMapId() << "," << node->getX() << "f," << node->getY() << "f," << node->getZ() << "f,"<< node->getO() <<"f), \""
                    << name << "\", " << (node->isImportant() ? "true" : "false") << ", true";
                if (node->isTransport())
                    out << "," << (node->isTransport() ? "true" : "false") << "," << node->getTransportId();

                out << ");";
                */
        sPlayerbotAIConfig.log(nodeStore, out.str().c_str());

        saveNodes.insert(std::make_pair(node, i));
    }

    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        for (auto& Link : *node->getLinks())
        {
            std::ostringstream out;

            //        struct linkNode { uint32 node1; uint32 node2; float distance; float extraCost; bool isPortal; bool isTransport; uint32 maxLevelMob; uint32 maxLevelAlliance; uint32 maxLevelHorde; float swimDistance; };

            out << std::fixed << std::setprecision(2) << "        linkNodes3.push_back(linkNode3{" << i << "," << saveNodes.find(Link.first)->second << ",";
            out << Link.second->print() << "});";

            //out << std::fixed << std::setprecision(1) << "        nodes[" << i << "]->setPathTo(nodes[" << saveNodes.find(Link.first)->second << "],TravelNodePath(";
            //out << Link.second->print() << "), true);";
            sPlayerbotAIConfig.log(nodeStore, out.str().c_str());
        }
    }

    sPlayerbotAIConfig.log(nodeStore, "	}");
    sPlayerbotAIConfig.log(nodeStore, "};");
    sPlayerbotAIConfig.log(nodeStore, "}");

    printf("\r [Done] \r\x3D");
    fflush(stdout);
}

void TravelNodeMap::saveNodeStore(bool force)
{
    if (!hasToSave && !force)
        return;

    hasToSave = false;

    WorldDatabase.BeginTransaction();

    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode");
    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode_link");
    WorldDatabase.PExecute("DELETE FROM ai_playerbot_travelnode_path");

    WorldDatabase.CommitTransaction();

    std::unordered_map<TravelNode*, uint32> saveNodes;
    std::vector<TravelNode*> anodes = sTravelNodeMap.getNodes();

    std::sort(anodes.begin(), anodes.end(), [](TravelNode* i, TravelNode* j) {return i->getName() + std::to_string(i->getMapId()) + std::to_string(i->getX()) < j->getName() + std::to_string(j->getMapId()) + std::to_string(j->getX()); });

    WorldDatabase.BeginTransaction();

    BarGoLink bar(anodes.size());
    for (uint32 i = 0; i < anodes.size(); i++)
    {
        TravelNode* node = anodes[i];

        std::string name = node->getName();
        name.erase(remove(name.begin(), name.end(), '\''), name.end());

        WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode` (`id`, `name`, `map_id`, `x`, `y`, `z`, `linked`) VALUES ('%u', '%s', '%d', '%f', '%f', '%f', '%d')"
            , i, name.c_str(), node->getMapId(), node->getX(), node->getY(), node->getZ(), (node->isLinked() ? 1 : 0));

        saveNodes.insert(std::make_pair(node, i));

        bar.step();
    }

    WorldDatabase.CommitTransaction();

    sLog.outString(">> Saved " SIZEFMTD " travelNodes.", anodes.size());

    {
        uint32 paths = 0, points = 0;
        BarGoLink bar(anodes.size());

        WorldDatabase.BeginTransaction();

        for (uint32 i = 0; i < anodes.size(); i++)
        {
            TravelNode* node = anodes[i];

            std::vector<std::pair<TravelNode*, TravelNodePath*>> links;

            for (auto& link : *node->getLinks())
                links.push_back(std::make_pair(link.first, link.second));

            std::sort(links.begin(), links.end(), [](std::pair<TravelNode*, TravelNodePath*> i, std::pair<TravelNode*, TravelNodePath*> j) {return i.first->getName() + std::to_string(i.first->getMapId()) + std::to_string(i.first->getX()) < j.first->getName() + std::to_string(j.first->getMapId()) + std::to_string(j.first->getX()); });

            for (auto& link : links)
            {
                TravelNodePath* path = link.second;

                WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode_link` (`node_id`, `to_node_id`,`type`,`object`,`distance`,`swim_distance`, `extra_cost`,`calculated`, `max_creature_0`,`max_creature_1`,`max_creature_2`) VALUES ('%d','%d', '%d', '%lu', '%f', '%f', '%f', '%d', '%d', '%d', '%d')"
                    , i
                    , saveNodes.find(link.first)->second
                    , uint8(path->getPathType())
                    , path->getPathObject()
                    , path->getDistance()
                    , path->getSwimDistance()
                    , path->getExtraCost()
                    , (path->getCalculated() ? 1 : 0)
                    , path->getMaxLevelCreature()[0]
                    , path->getMaxLevelCreature()[1]
                    , path->getMaxLevelCreature()[2]);

                paths++;

                std::vector<WorldPosition> ppath = path->getPath();

                for (uint32 j = 0; j < ppath.size(); j++)
                {
                    WorldPosition point = ppath[j];
                    WorldDatabase.PExecute("INSERT INTO `ai_playerbot_travelnode_path` (`node_id`, `to_node_id`, `nr`, `map_id`, `x`, `y`, `z`) VALUES ('%d', '%d', '%d','%d', '%f', '%f', '%f')"
                        , i
                        , saveNodes.find(link.first)->second
                        , j
                        , point.getMapId()
                        , point.getX()
                        , point.getY()
                        , point.getZ());

                    points++;
                }
            }

            bar.step();
        }

        WorldDatabase.CommitTransaction();

        sLog.outString(">> Saved %d travelNode Paths, %d points.", paths,points);
    }
}

void TravelNodeMap::loadNodeStore()
{    
    std::string query = "SELECT id, name, map_id, x, y, z, linked FROM ai_playerbot_travelnode";

    std::unordered_map<uint32, TravelNode*> saveNodes;   

    {
        auto result = WorldDatabase.PQuery("%s", query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* node = addNode(WorldPosition(fields[2].GetUInt32(), fields[3].GetFloat(), fields[4].GetFloat(), fields[5].GetFloat()), fields[1].GetCppString(), true);

                if (fields[6].GetBool())
                    node->setLinked(true);
                else
                    hasToGen = true;

                saveNodes.insert(std::make_pair(fields[0].GetUInt32(), node));

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNodes.", saveNodes.size());
        }
        else
        {
            hasToFullGen = true;
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNodes.");
            return;
        }
    }

    {
        //                     0        1          2    3      4         5              6          7          8               9             10 
        std::string query = "SELECT node_id, to_node_id,type,object,distance,swim_distance, extra_cost,calculated, max_creature_0,max_creature_1,max_creature_2 FROM ai_playerbot_travelnode_link";

        auto result = WorldDatabase.PQuery("%s", query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* startNode = saveNodes.find(fields[0].GetUInt32())->second;
                TravelNode* endNode = saveNodes.find(fields[1].GetUInt32())->second;

                if (!startNode || !endNode)
                    continue;

                startNode->setPathTo(endNode, TravelNodePath(fields[4].GetFloat(), fields[6].GetFloat(), fields[2].GetUInt8(), fields[3].GetUInt64(), fields[7].GetBool(), { fields[8].GetUInt8(),fields[9].GetUInt8(),fields[10].GetUInt8() }, fields[5].GetFloat()), true);

                if (!fields[7].GetBool())
                    hasToGen = true;

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNode paths.", result->GetRowCount());
        }
        else
        {
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNode links.");
        }
    }

    {
        //                     0        1           2   3      4   5  6
        std::string query = "SELECT node_id, to_node_id, nr, map_id, x, y, z FROM ai_playerbot_travelnode_path order by node_id, to_node_id, nr";

        auto result = WorldDatabase.PQuery("%s", query.c_str());

        if (result)
        {
            BarGoLink bar(result->GetRowCount());
            do
            {
                Field* fields = result->Fetch();
                bar.step();

                TravelNode* startNode = saveNodes.find(fields[0].GetUInt32())->second;
                TravelNode* endNode = saveNodes.find(fields[1].GetUInt32())->second;

                if (!startNode || !endNode || !startNode->hasPathTo(endNode))
                    continue;

                TravelNodePath* path = startNode->getPathTo(endNode);

                std::vector<WorldPosition> ppath = path->getPath();
                ppath.push_back(WorldPosition(fields[3].GetUInt32(), fields[4].GetFloat(), fields[5].GetFloat(), fields[6].GetFloat()));

                path->setPath(ppath);

                if (path->getCalculated())
                    path->setComplete(true);

            } while (result->NextRow());

            sLog.outString(">> Loaded " SIZEFMTD " travelNode paths points.", result->GetRowCount());
        }
        else
        {
            sLog.outString();
            sLog.outErrorDb(">> Error loading travelNode paths.");
        }
    }
}

void TravelNodeMap::calcMapOffset()
{
    mapOffsets.push_back(std::make_pair(0, WorldPosition(0, 0, 0, 0, 0)));
    mapOffsets.push_back(std::make_pair(1, WorldPosition(1, -3680.0, 13670.0, 0, 0)));
    mapOffsets.push_back(std::make_pair(530, WorldPosition(530, 15000.0, -20000.0, 0, 0)));
    mapOffsets.push_back(std::make_pair(571, WorldPosition(571, 10000.0, 5000.0, 0, 0)));

    std::vector<uint32> mapIds;

    for (auto& node : m_nodes)
    {
        if (!node->getPosition()->isOverworld())
            if (std::find(mapIds.begin(), mapIds.end(), node->getMapId()) == mapIds.end())
                mapIds.push_back(node->getMapId());
    }

    std::sort(mapIds.begin(), mapIds.end());

    std::vector<WorldPosition> min, max;

    for (auto& mapId : mapIds)
    {
        bool doPush = true;
        for (auto& node : m_nodes)
        {
            if (node->getMapId() != mapId)
                continue;

            if (doPush)
            {
                min.push_back(*node->getPosition());
                max.push_back(*node->getPosition());
                doPush = false;
            }
            else
            {
                min.back().setX(std::min(min.back().getX(), node->getX()));
                min.back().setY(std::min(min.back().getY(), node->getY()));
                max.back().setX(std::max(max.back().getX(), node->getX()));
                max.back().setY(std::max(max.back().getY(), node->getY()));
            }
        }
    }

    WorldPosition curPos = WorldPosition(0, -13000, -13000, 0, 0);
    WorldPosition endPos = WorldPosition(0, 3000, -13000, 0, 0);

    uint32 i = 0;
    float maxY = 0;
    //+X -> -Y
    for (auto& mapId : mapIds)
    {
        mapOffsets.push_back(std::make_pair(mapId, WorldPosition(mapId, curPos.getX() - min[i].getX(), curPos.getY() - max[i].getY(), 0, 0)));

        maxY = std::max(maxY, (max[i].getY() - min[i].getY() + 500));
        curPos.setX(curPos.getX() + (max[i].getX() - min[i].getX() + 500));

        if (curPos.getX() > endPos.getX())
        {
            curPos.setY(curPos.getY() - maxY);
            curPos.setX(-13000);
        }
        i++;
    }
}

WorldPosition TravelNodeMap::getMapOffset(uint32 mapId)
{
    for (auto& offset : mapOffsets)
    {
        if (offset.first == mapId)
            return offset.second;
    }

    return WorldPosition(mapId, 0, 0, 0, 0);
}
