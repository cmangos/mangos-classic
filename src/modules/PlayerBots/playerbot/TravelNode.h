#pragma once

#include <shared_mutex>
#include "WorldPosition.h"

//THEORY
// 
// Pathfinding in (c)mangos is based on detour recast an opensource nashmesh creation and pathfinding codebase.
// This system is used for mob and npc pathfinding and in this codebase also for bots.
// Because mobs and npc movement is based on following a player or a set path the pathfinder is limited to 296y.
// This means that when trying to find a path from A to B distances beyond 296y will be a best guess often moving in a straight path.
// Bots would get stuck moving from Northshire to Stormwind because there is no 296y path that doesn't go (initially) the wrong direction.
// 
// To remedy this limitation without altering the pathfinder limits too much this node system was introduced.
// 
//  <S> ---> [N1] ---> [N2] ---> [N3] ---> <E>
//
// Bot at <S> wants to move to <E>
// [N1],[N2],[N3] are predefined nodes for wich we know we can move from [N1] to [N2] and from [N2] to [N3] but not from [N1] to [N3]
// If we can move fom [S] to [N1] and from [N3] to [E] we have a complete route to travel.
// 
// Termonology:
// Node: a location on a map for which we know bots are likely to want to travel to or need to travel past to reach other nodes.
// Link: the connection between two nodes. A link signifies that the bot can travel from one node to another. A link is one-directional.
// Path: the waypointpath returned by the standard pathfinder to move from one node (or position) to another. A path can be imcomplete or empty which means there is no link.
// Route: the list of nodes that give the shortest route from a node to a distant node. Routes are calculated using a standard A* search based on links.
// 
// Nodes, links and paths are stored in db tables but will be generated if they are empty. See TravelNodeMap for more information about the generation proces.
  
enum class TravelNodePathType : uint8
{
    none = 0,
    walk = 1,
    areaTrigger = 2,
    transport = 3,
    flightPath = 4,
    teleportSpell = 5,
    staticPortal = 6
};

using namespace ai;

namespace ai
{
    class WorldPosition;

    //A connection between two nodes. 
    class TravelNodePath
    {
    public:
        //Legacy Constructor for travelnodestore
        //TravelNodePath(float distance1, float extraCost1, bool portal1 = false, uint32 portalId1 = 0, bool transport1 = false, bool calculated = false, uint8 maxLevelMob1 = 0, uint8 maxLevelAlliance1 = 0, uint8 maxLevelHorde1 = 0, float swimDistance1 = 0, bool flightPath1 = false);

        //Constructor
        TravelNodePath(float distance = 0.1f, float extraCost = 0, uint8 pathType = (uint8)TravelNodePathType::walk, uint64 pathObject = 0, bool calculated = false, std::vector<uint8> maxLevelCreature = { 0,0,0 }, float swimDistance = 0)
            : distance(distance), extraCost(extraCost), pathType(TravelNodePathType(pathType)), pathObject(pathObject), calculated(calculated), maxLevelCreature(maxLevelCreature), swimDistance(swimDistance) {
            if (pathType != (uint8)TravelNodePathType::walk) complete = true;
        };

        TravelNodePath(TravelNodePath* basePath) {
            complete = basePath->complete; path = basePath->path; extraCost = basePath->extraCost; calculated = basePath->calculated; distance = basePath->distance; maxLevelCreature = basePath->maxLevelCreature; swimDistance = basePath->swimDistance; pathType = basePath->pathType; pathObject = basePath->pathObject;
        };

        //Getters
        bool getComplete() { return complete || pathType != TravelNodePathType::walk; }
        std::vector<WorldPosition> getPath() { return path; }

        TravelNodePathType getPathType() { return pathType; }
        uint64 getPathObject() { return pathObject; }

        float getDistance() { return distance; }
        float getSwimDistance() { return swimDistance; }
        float getExtraCost() { return extraCost; }
        std::vector<uint8> getMaxLevelCreature() { return  maxLevelCreature; }

        void setCalculated(bool calculated1 = true) { calculated = calculated1; }
        bool getCalculated() { return calculated; }

       std::string print();

        //Setters
        void setComplete(bool complete1) { complete = complete1; }
        void setPath(std::vector<WorldPosition> path1) { path = path1; }
        void setPathAndCost(std::vector<WorldPosition> path1, float speed) { setPath(path1); calculateCost(true); extraCost = distance / speed; }
        //void setPortal(bool portal1, uint32 portalId1 = 0) { portal = portal1; portalId = portalId1; }
        //void setTransport(bool transport1) { transport = transport1; }
        void setPathType(TravelNodePathType pathType1) { pathType = pathType1; }
        void setPathObject(uint64 pathObject1) { pathObject = pathObject1; }

        void calculateCost(bool distanceOnly = false);

        float getCost(Unit* unit = nullptr, uint32 cGold = 0);
        uint32 getPrice();
    private:
        //Does the path have all the points to get to the destination?
        bool complete = false;

        //List of WorldPositions to get to the destination.
        std::vector<WorldPosition> path = {};

        //The extra (loading/transport) time it takes to take this path.
        float extraCost = 0;

        bool calculated = false;

        //Derived distance in yards
        float distance = 0.1f;

        //Calculated mobs level along the way.
        std::vector<uint8> maxLevelCreature = { 0,0,0 }; //mobs, horde, alliance

        //Calculated swiming distances along the way.
        float swimDistance = 0;

        TravelNodePathType pathType = TravelNodePathType::walk;
        uint64 pathObject = 0;

        /*
        //Is the path a portal/teleport to the destination?
        bool portal = false;
        //Area trigger Id
        uint32 portalId = 0;

        //Is the path transport based?
        bool transport = false;

        //Is the path a flightpath?
        bool flightPath = false;
        */
    };

    //A waypoint to travel from or to.
    //Each node knows which other nodes can be reached without help.
    class TravelNode
    {
    public:
        //Constructors
        TravelNode() {};
        TravelNode(WorldPosition point1, std::string nodeName1 = "Travel Node", bool important1 = false) { nodeName = nodeName1; point = point1; important = important1; }
        TravelNode(TravelNode* baseNode) { nodeName = baseNode->nodeName; point = baseNode->point; important = baseNode->important; }

        //Setters
        void setLinked(bool linked1) { linked = linked1; }
        void setPoint(WorldPosition point1) { point = point1; }

        //Getters
        std::string getName() { return nodeName; };
        WorldPosition* getPosition() { return &point; };
        std::unordered_map<TravelNode*, TravelNodePath>* getPaths() { return &paths; }
        std::unordered_map<TravelNode*, TravelNodePath*>* getLinks() { return &links; }
        bool isImportant() { return important; };
        bool isLinked() { return linked; }
        bool isTransport() { for (auto link : *getLinks()) if (link.second->getPathType() == TravelNodePathType::transport) return true; return false; }
        uint32 getTransportId() { for (auto link : *getLinks()) if (link.second->getPathType() == TravelNodePathType::transport) return link.second->getPathObject(); return false; }
        bool isPortal() { for (auto link : *getLinks()) if (link.second->getPathType() == TravelNodePathType::areaTrigger || link.second->getPathType() == TravelNodePathType::staticPortal) return true; return false; }
        uint32 getAreaTriggerId();
        bool isAreaTriggerTarget(uint32 areaTriggerId = 0);

        //WorldLocation shortcuts
        uint32 getMapId() { return point.getMapId(); }
        float getX() { return point.getX(); }
        float getY() { return point.getY(); }
        float getZ() { return point.getZ(); }
        float getO() { return point.getO(); }
        float getDistance(WorldPosition pos) { return point.distance(pos); }
        float getDistance(TravelNode* node) { return point.distance(*node->getPosition()); }
        float fDist(WorldPosition pos) { return point.fDist(pos); }
        float fDist(TravelNode* node) { return point.fDist(*node->getPosition()); }

        TravelNodePath* setPathTo(TravelNode* node, TravelNodePath path = TravelNodePath(), bool isLink = true) { if (this != node) { paths[node] = path; if (isLink) links[node] = &paths[node]; return  &paths[node]; } else return nullptr; }
        bool hasPathTo(TravelNode* node) { return paths.find(node) != paths.end(); }
        TravelNodePath* getPathTo(TravelNode* node) { return &paths[node]; }
        bool hasCompletePathTo(TravelNode* node) { return hasPathTo(node) && getPathTo(node)->getComplete(); }
        TravelNodePath* buildPath(TravelNode* endNode, Unit* bot, bool postProcess = false);

        void setLinkTo(TravelNode* node, float distance = 0.1f) {
            if (this != node)
            {
                if (!hasPathTo(node))
                    setPathTo(node, TravelNodePath(distance));
                else
                    links[node] = &paths[node];
            }
        }

        bool hasLinkTo(TravelNode* node) { return links.find(node) != links.end(); }
        float linkCostTo(TravelNode* node) { return paths.find(node)->second.getDistance(); }
        float linkDistanceTo(TravelNode* node) { return paths.find(node)->second.getDistance(); }
        void removeLinkTo(TravelNode* node, bool removePaths = false);

        bool isEqual(TravelNode* compareNode);

        //Removes links to other nodes that can also be reached by passing another node.
        bool isUselessLink(TravelNode* farNode);
        bool cropUselessLinks();

        //Returns all nodes that can be reached from this node.
        std::vector<TravelNode*> getNodeMap(bool importantOnly = false, std::vector<TravelNode*> ignoreNodes = {}, bool mapOnly = false);

        //Checks if it is even possible to route to this node.
        bool hasRouteTo(TravelNode* node, bool mapOnly = false) { if (routes.empty()) for (auto mNode : getNodeMap(false, {}, mapOnly)) routes[mNode] = true; return routes.find(node) != routes.end(); };
        uint32 getRouteSize(bool mapOnly = false) { if (routes.empty()) for (auto mNode : getNodeMap(false, {}, mapOnly)) routes[mNode] = true; return routes.size(); }
        void clearRoutes() { routes.clear(); }

        void print(bool printFailed = true);
    protected:
        //Logical name of the node
        std::string nodeName;
        //WorldPosition of the node.
        WorldPosition point;

        //List of paths to other nodes.
        std::unordered_map<TravelNode*, TravelNodePath> paths;
        //List of links to other nodes.
        std::unordered_map<TravelNode*, TravelNodePath*> links;

        //List of nodes and if there is 'any' route possible
        std::unordered_map<TravelNode*, bool> routes;

        //This node should not be removed
        bool important = false;

        //This node has been checked for nearby links
        bool linked = false;

        //This node is a (moving) transport.
        //bool transport = false;
        //Entry of transport.
        //uint32 transportId = 0;
    };

    class PortalNode : public TravelNode
    {
    public:
        PortalNode(TravelNode* baseNode) : TravelNode(baseNode) {};

        void SetPortal(TravelNode* baseNode, TravelNode* endNode, uint32 portalSpell)
        {
            nodeName = baseNode->getName();
            point = *baseNode->getPosition();
            paths.clear();
            links.clear();
            TravelNodePath path(0.1f, 0.1f, (uint8)TravelNodePathType::teleportSpell, portalSpell, true);
            setPathTo(endNode, path);
        };
    };

    //Route step type
    enum class PathNodeType : uint8
    {
        NODE_PREPATH = 0,
        NODE_PATH = 1,
        NODE_NODE = 2,
        NODE_AREA_TRIGGER = 3,
        NODE_TRANSPORT = 4,
        NODE_FLIGHTPATH = 5,
        NODE_TELEPORT = 6,
        NODE_STATIC_PORTAL = 7
    };

    struct PathNodePoint
    {
        WorldPosition point;
        PathNodeType type = PathNodeType::NODE_PATH;
        uint32 entry = 0;
    };

    //A complete list of points the bots has to walk to or teleport to.
    class TravelPath
    {
    public:
        TravelPath() {};
        TravelPath(std::vector<PathNodePoint> fullPath1) { fullPath = fullPath1; }
        TravelPath(std::vector<WorldPosition> path, PathNodeType type = PathNodeType::NODE_PATH, uint32 entry = 0) { addPath(path, type, entry); }

        void addPoint(PathNodePoint point) { fullPath.push_back(point); }
        void addPoint(WorldPosition point, PathNodeType type = PathNodeType::NODE_PATH, uint32 entry = 0) { fullPath.push_back(PathNodePoint{ point, type, entry }); }
        void addPath(std::vector<WorldPosition> path, PathNodeType type = PathNodeType::NODE_PATH, uint32 entry = 0) { for (auto& p : path) { fullPath.push_back(PathNodePoint{ p, type, entry }); }; }
        void addPath(std::vector<PathNodePoint> newPath) { fullPath.insert(fullPath.end(), newPath.begin(), newPath.end()); }
        void clear() { fullPath.clear(); }

        bool empty() { return fullPath.empty(); }
        std::vector<PathNodePoint> getPath() { return fullPath; }
        WorldPosition getFront() { return fullPath.front().point; }
        WorldPosition getBack() { return fullPath.back().point; }

        std::vector<WorldPosition> getPointPath() { std::vector<WorldPosition> retVec; for (const auto& p : fullPath) retVec.push_back(p.point); return retVec; };

        bool makeShortCut(WorldPosition startPos, float maxDist, Unit* bot);
        bool shouldMoveToNextPoint(WorldPosition startPos, std::vector<PathNodePoint>::iterator beg, std::vector<PathNodePoint>::iterator ed, std::vector<PathNodePoint>::iterator p, float& moveDist, float maxDist);
        WorldPosition getNextPoint(WorldPosition startPos, float maxDist, TravelNodePathType& pathType, uint32& entry, bool onTransport, WorldPosition& telePosition);

        std::ostringstream print();
    private:
        std::vector<PathNodePoint> fullPath;
    };

    //An stored A* search that gives a complete route from one node to another.
    class TravelNodeRoute
    {
    public:
        TravelNodeRoute() {}
        TravelNodeRoute(std::vector<TravelNode*> nodes1, std::vector<TravelNode*> tempNodes) { nodes = nodes1; if (tempNodes.size()) addTempNodes(tempNodes); }

        bool isEmpty() { return nodes.empty(); }

        bool hasNode(TravelNode* node) { return findNode(node) != nodes.end(); }
        float getTotalDistance();

        void setNodes(std::vector<TravelNode*> nodes1) { nodes = nodes1; }
        std::vector<TravelNode*> getNodes() { return nodes; }

        void addTempNodes(std::vector<TravelNode*> nodes) { tempNodes.insert(tempNodes.end(), nodes.begin(), nodes.end()); }
        void cleanTempNodes() { for (auto node : tempNodes) delete node; }

        TravelPath buildPath(std::vector<WorldPosition> pathToStart = {}, std::vector<WorldPosition> pathToEnd = {}, Unit* bot = nullptr);

        std::ostringstream print();
    private:
        std::vector<TravelNode*>::iterator findNode(TravelNode* node) { return std::find(nodes.begin(), nodes.end(), node); }
        std::vector<TravelNode*> nodes;
        std::vector<TravelNode*> tempNodes;
    };

    //A node container to aid A* calculations with nodes.
    class TravelNodeStub
    {
    public:
        TravelNodeStub(TravelNode* dataNode1) { dataNode = dataNode1; }

        TravelNode* dataNode;
        float m_f = 0.0, m_g = 0.0, m_h = 0.0;
        bool open = false, close = false;
        TravelNodeStub* parent = nullptr;
        uint32 currentGold = 0.0;
    };

    //The container of all nodes.
    class TravelNodeMap
    {
    public:
        TravelNodeMap() {};
        TravelNodeMap(TravelNodeMap* baseMap);

        TravelNode* addNode(WorldPosition pos, std::string preferedName = "Travel Node", bool isImportant = false, bool checkDuplicate = true, bool transport = false, uint32 transportId = 0);
        void removeNode(TravelNode* node);
        bool removeNodes() { if (m_nMapMtx.try_lock_for(std::chrono::seconds(10))) { for (auto& node : m_nodes) removeNode(node); m_nMapMtx.unlock(); return true; } return false; };
        void fullLinkNode(TravelNode* startNode, Unit* bot);

        //Get all nodes
        std::vector<TravelNode*> getNodes() { return m_nodes; }
        std::vector<TravelNode*> getNodes(WorldPosition pos, float range = -1);

        //Find nearest node.
        TravelNode* getNode(TravelNode* sameNode) { for (auto& node : m_map_nodes[sameNode->getMapId()]) { if (node->getName() == sameNode->getName() && node->getPosition() == sameNode->getPosition()) return node; } return nullptr; }
        TravelNode* getNode(WorldPosition pos, std::vector<WorldPosition>& ppath, Unit* bot = nullptr, float range = -1);
        TravelNode* getNode(WorldPosition pos, Unit* bot = nullptr, float range = -1) { std::vector<WorldPosition> ppath; return getNode(pos, ppath, bot, range); }

        //Get Random Node
        TravelNode* getRandomNode(WorldPosition pos) { std::vector<TravelNode*> rNodes = getNodes(pos); if (rNodes.empty()) return nullptr; return  rNodes[urand(0, rNodes.size() - 1)]; }

        //Finds the best nodePath between two nodes
        TravelNodeRoute getRoute(TravelNode* start, TravelNode* goal, Unit* unit = nullptr);

        //Find the best node between two positions
        TravelNodeRoute getRoute(WorldPosition startPos, WorldPosition endPos, std::vector<WorldPosition>& startPath, Unit* unit = nullptr);

        //Find the full path between those locations
        static TravelPath getFullPath(WorldPosition startPos, WorldPosition endPos, Unit* unit = nullptr);

        //Manage/update nodes
        void manageNodes(Unit* bot, bool mapFull = false);

        void setHasToGen() { hasToGen = true; }
        bool gethasToGen() { return hasToGen || hasToFullGen; }

        void LoadMaps();

        //Below are the steps to creating the content stored in the node, link and path tables. 
        //Nodes are placed based on key locations based on objects/creatures in the world and paths are generated using the standardpathfinder.

        void generateNpcNodes();                  //Creates node at innkeepers, flightmasters, spirithealers and bosses.
        void generateStartNodes();                //Create node at lvl1 players spawn.
        void generateAreaTriggerNodes();          //Create node at area trigger (dungeon portals/teleports) and also link the entry and exit.
        void makeDockNode(TravelNode* node, WorldPosition pos, std::string dockName); //Create helper node to enter/exit transport.
        void generateTransportNodes();            //Create node at transport (boats/zepelins/elevators) and also create the path they move.
        void generateZoneMeanNodes();             //Create node at zone mean (the avg location of all objects and creatures of a certain area/zone)
        void generatePortalNodes();               //Create node at static portal (ie. dalaran->ironforge) and the desination of teleport spell (ie. teleport to ironforge)
        void addManualNodes();                    //Nodes manually placed to reach places that can not be pathed automatically.
        void generateNodes();                     //Call all above methods.

        void generateWalkPathMap(uint32 mapId, BarGoLink* bar);   //Pathfind from all nodes to all nodes in a specific map. Create a path for all attemps and a link for all paths that actually reach the end node.
        void generateWalkPaths();                 //Call above method for all maps async.

        //Helper nodes take a long time to generate and have limited impact. Disable is generation time becomes an issue. It does make some places connected to the network though like some caves and tauren start.
        void generateHelperNodes(uint32 mapId, BarGoLink* bar);   //For all nodes, objects and creatures, that can't be directly reached from one of the 5 nearby nodes, place a node on one of the paths of the nearby nodes so they can be reached or else place a node at the object. Also generate walkPaths for each node and the entire map again afterwards.
        void generateHelperNodes();               //Call above method for all maps async.

        void removeLowNodes();                    //Remove any node in the overworld that can reach less than 4 nodes directly or indirectly.
        void removeUselessPathMap(uint32 mapId);  //Remove any path which can be replaced by multiple paths of combined equivalent or shorter length. (Single paths get a 10% length increase to make it prefer multiple paths with a slight detour) 
        void removeUselessPaths();                //Call above method for all maps async.
        void calculatePathCosts();                //Calculate distance, swim distance and expected max alliance, horde and mob level along the path.
        void generateTaxiPaths();                 //Create paths and links from all flightmasters along their taxi routes.
        void generatePaths(bool helpers = true);  //Call above methods.

        void generateAll();                       //Generate nodes, paths when needed. Map-offsets,transfers and node coverage is also calculated.

        void printMap();

        void printNodeStore();
        void saveNodeStore(bool force = false);
        void loadNodeStore();

        bool cropUselessNode(TravelNode* startNode);
        TravelNode* addZoneLinkNode(TravelNode* startNode);
        TravelNode* addRandomExtNode(TravelNode* startNode);

        void calcMapOffset();                      
        WorldPosition getMapOffset(uint32 mapId);

        std::shared_timed_mutex m_nMapMtx;
    private:
        std::vector<TravelNode*> m_nodes;
        std::unordered_map<uint32, std::vector<TravelNode*>> m_map_nodes;

        std::vector<std::pair<uint32, WorldPosition>> mapOffsets;

        bool hasToSave = false;
        bool hasToGen = false;
        bool hasToFullGen = false;
    };
}

#define sTravelNodeMap MaNGOS::Singleton<TravelNodeMap>::Instance()