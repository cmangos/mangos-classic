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

#include "Entities/Transports.h"
#include "Maps/MapManager.h"
#include "Globals/ObjectMgr.h"
#include "Entities/ObjectGuid.h"
#include "MotionGenerators/Path.h"
#include "Maps/TransportMgr.h"

#include "WorldPacket.h"
#include "Server/DBCStores.h"
#include "ProgressBar.h"

#include "Movement/MoveSpline.h"

void MapManager::LoadTransports()
{
    QueryResult* result = WorldDatabase.Query("SELECT entry, name, period FROM transports");

    uint32 count = 0;

    if (!result)
    {
        BarGoLink bar(1);
        bar.step();
        sLog.outString(">> Loaded %u transports", count);
        sLog.outString();
        return;
    }

    BarGoLink bar(result->GetRowCount());

    do
    {
        bar.step();

        Transport* t = new Transport;

        Field* fields = result->Fetch();

        uint32 entry = fields[0].GetUInt32();
        std::string name = fields[1].GetCppString();
        t->SetPeriod(fields[2].GetUInt32());

        const GameObjectInfo* goinfo = ObjectMgr::GetGameObjectInfo(entry);

        if (!goinfo)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template missing", entry, name.c_str());
            delete t;
            continue;
        }

        if (goinfo->type != GAMEOBJECT_TYPE_MO_TRANSPORT)
        {
            sLog.outErrorDb("Transport ID:%u, Name: %s, will not be loaded, gameobject_template type wrong", entry, name.c_str());
            delete t;
            continue;
        }

        // sLog.outString("Loading transport %d between %s, %s", entry, name.c_str(), goinfo->name);

        std::set<uint32> mapsUsed;

        if (!t->GenerateWaypoints(goinfo, mapsUsed))
            // skip transports with empty waypoints list
        {
            sLog.outErrorDb("Transport (path id %u) path size = 0. Transport ignored, check DBC files or transport GO data0 field.", goinfo->moTransport.taxiPathId);
            delete t;
            continue;
        }

        TaxiPathNodeEntry const* startNode = t->GetKeyFrames().begin()->Node;
        uint32 mapId = startNode->mapid;
        float x = startNode->x;
        float y = startNode->y;
        float z = startNode->z;
        float o = t->GetKeyFrames().begin()->InitialOrientation;

        // current code does not support transports in dungeon!
        const MapEntry* pMapInfo = sMapStore.LookupEntry(mapId);
        if (!pMapInfo || pMapInfo->Instanceable())
        {
            delete t;
            continue;
        }

        // If we someday decide to use the grid to track transports, here:
        t->SetMap(sMapMgr.CreateMap(mapId, t));

        // creates the Gameobject
        if (!t->Create(entry, mapId, x, y, z, o, GO_ANIMPROGRESS_DEFAULT))
        {
            delete t;
            continue;
        }

        m_Transports.insert(t);

        for (uint32 i : mapsUsed)
            m_TransportsByMap[i].insert(t);

        // t->GetMap()->Add<GameObject>((GameObject *)t);
        ++count;
    }
    while (result->NextRow());
    delete result;

    // check transport data DB integrity
    result = WorldDatabase.Query("SELECT gameobject.guid,gameobject.id,transports.name FROM gameobject,transports WHERE gameobject.id = transports.entry");
    if (result)                                             // wrong data found
    {
        do
        {
            Field* fields = result->Fetch();

            uint32 guid  = fields[0].GetUInt32();
            uint32 entry = fields[1].GetUInt32();
            std::string name = fields[2].GetCppString();
            sLog.outErrorDb("Transport %u '%s' have record (GUID: %u) in `gameobject`. Transports DON'T must have any records in `gameobject` or its behavior will be unpredictable/bugged.", entry, name.c_str(), guid);
        }
        while (result->NextRow());

        delete result;
    }

    sLog.outString(">> Loaded %u transports", count);
    sLog.outString();
}

Transport::Transport() : GenericTransport(), m_isMoving(true), m_pendingStop(false)
{
    m_updateFlag = (UPDATEFLAG_TRANSPORT | UPDATEFLAG_ALL | UPDATEFLAG_HAS_POSITION);
}

bool Transport::Create(uint32 guidlow, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress)
{
    Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        sLog.outError("Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
                      guidlow, x, y);
        return false;
    }

    Object::_Create(guidlow, 0, HIGHGUID_MO_TRANSPORT);

    GameObjectInfo const* goinfo = ObjectMgr::GetGameObjectInfo(guidlow);

    if (!goinfo)
    {
        sLog.outErrorDb("Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;

    // initialize waypoints
    m_nextFrame = GetKeyFrames().begin();
    m_currentFrame = m_nextFrame++;

    m_pathProgress = time(nullptr) % (m_transportTemplate.pathTime / 1000);
    m_pathProgress *= 1000;

    SetObjectScale(goinfo->size);

    SetUInt32Value(GAMEOBJECT_FACTION, goinfo->faction);
    SetUInt32Value(GAMEOBJECT_FLAGS, goinfo->flags);

    SetEntry(goinfo->id);

    SetDisplayId(goinfo->displayId);
    SetUInt32Value(GAMEOBJECT_DISPLAYID, goinfo->displayId);

    SetGoState(GO_STATE_READY);
    SetGoType(GameobjectTypes(goinfo->type));

    SetGoAnimProgress(animprogress);

    SetName(goinfo->name);

    return true;
}

class SplineRawInitializer
{
    public:
        SplineRawInitializer(Movement::PointsArray& points) : _points(points) { }

        void operator()(uint8& mode, bool& cyclic, Movement::PointsArray& points, int& lo, int& hi) const
        {
            mode = Movement::SplineBase::ModeCatmullrom;
            cyclic = false;
            points.assign(_points.begin(), _points.end());
            lo = 1;
            hi = points.size() - 2;
        }

        Movement::PointsArray& _points;
};

bool Transport::GenerateWaypoints(GameObjectInfo const* goinfo, std::set<uint32>& mapsUsed)
{
    uint32 pathid = goinfo->moTransport.taxiPathId;
    if (pathid >= sTaxiPathNodesByPath.size())
        return false;

    uint32 pathId = goinfo->moTransport.taxiPathId;
    TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];
    std::vector<KeyFrame>& keyFrames = GetKeyFrames();
    Movement::PointsArray splinePath, allPoints;
    bool mapChange = false;
    for (size_t i = 0; i < path.size(); ++i)
        allPoints.push_back(G3D::Vector3(path[i]->x, path[i]->y, path[i]->z));

    // Add extra points to allow derivative calculations for all path nodes
    allPoints.insert(allPoints.begin(), allPoints.front().lerp(allPoints[1], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -0.2f));
    allPoints.push_back(allPoints.back().lerp(allPoints[allPoints.size() - 2], -1.0f));

    SplineRawInitializer initer(allPoints);
    TransportSpline orientationSpline;
    orientationSpline.init_spline_custom(initer);
    orientationSpline.initLengths();

    for (size_t i = 1; i < path.size() - 1; ++i)
    {
        if (!mapChange)
        {
            TaxiPathNodeEntry const& node_i = *path[i];
            if (i != path.size() - 1 && (node_i.actionFlag & 1 || node_i.mapid != path[i + 1]->mapid))
            {
                keyFrames.back().Teleport = true;
                mapChange = true;
            }
            else
            {
                KeyFrame k(node_i);
                G3D::Vector3 h;
                orientationSpline.evaluate_derivative(i + 1, 0.0f, h);
                k.InitialOrientation = MapManager::NormalizeOrientation(atan2(h.y, h.x) + M_PI);

                keyFrames.push_back(k);
                splinePath.push_back(G3D::Vector3(node_i.x, node_i.y, node_i.z));
                mapsUsed.insert(k.Node->mapid);
            }
        }
        else
            mapChange = false;
    }

    MANGOS_ASSERT(!keyFrames.empty());

    if (mapsUsed.size() > 1)
    {
        for (const auto itr : mapsUsed)
            MANGOS_ASSERT(!sMapStore.LookupEntry(itr)->Instanceable());

        m_transportTemplate.inInstance = false;
    }
    else
        m_transportTemplate.inInstance = sMapStore.LookupEntry(*mapsUsed.begin())->Instanceable();

    // last to first is always "teleport", even for closed paths
    keyFrames.back().Teleport = true;

    float const speed = float(goinfo->moTransport.moveSpeed);
    float const accel = float(goinfo->moTransport.accelRate);
    float const accel_dist = 0.5f * speed * speed / accel;

    m_transportTemplate.accelTime = speed / accel;
    m_transportTemplate.accelDist = accel_dist;

    int32 firstStop = -1;
    int32 lastStop = -1;

    // first cell is arrived at by teleportation :S
    keyFrames[0].DistFromPrev = 0;
    keyFrames[0].Index = 1;
    if (keyFrames[0].IsStopFrame())
    {
        firstStop = 0;
        lastStop = 0;
    }

    // find the rest of the distances between key points
    // Every path segment has its own spline
    size_t start = 0;
    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        if (keyFrames[i - 1].Teleport || i + 1 == keyFrames.size())
        {
            size_t extra = !keyFrames[i - 1].Teleport ? 1 : 0;
            TransportSpline* spline = new TransportSpline();
            spline->init_spline(&splinePath[start], i - start + extra, Movement::SplineBase::ModeCatmullrom);
            spline->initLengths();
            for (size_t j = start; j < i + extra; ++j)
            {
                keyFrames[j].Index = j - start + 1;
                keyFrames[j].DistFromPrev = spline->length(j - start, j + 1 - start);
                if (j > 0)
                    keyFrames[j - 1].NextDistFromPrev = keyFrames[j].DistFromPrev;
                keyFrames[j].Spline = spline;
            }

            if (keyFrames[i - 1].Teleport)
            {
                keyFrames[i].Index = i - start + 1;
                keyFrames[i].DistFromPrev = 0.0f;
                keyFrames[i - 1].NextDistFromPrev = 0.0f;
                keyFrames[i].Spline = spline;
            }

            start = i;
        }

        if (keyFrames[i].IsStopFrame())
        {
            // remember first stop frame
            if (firstStop == -1)
                firstStop = i;
            lastStop = i;
        }
    }

    keyFrames.back().NextDistFromPrev = keyFrames.front().DistFromPrev;

    if (firstStop == -1 || lastStop == -1)
        firstStop = lastStop = 0;

    // at stopping keyframes, we define distSinceStop == 0,
    // and distUntilStop is to the next stopping keyframe.
    // this is required to properly handle cases of two stopping frames in a row (yes they do exist)
    float tmpDist = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame() || j == lastStop)
            tmpDist = 0.0f;
        else
            tmpDist += keyFrames[j].DistFromPrev;
        keyFrames[j].DistSinceStop = tmpDist;
    }

    tmpDist = 0.0f;
    for (int32 i = int32(keyFrames.size()) - 1; i >= 0; i--)
    {
        int32 j = (i + firstStop) % keyFrames.size();
        tmpDist += keyFrames[(j + 1) % keyFrames.size()].DistFromPrev;
        keyFrames[j].DistUntilStop = tmpDist;
        if (keyFrames[j].IsStopFrame() || j == firstStop)
            tmpDist = 0.0f;
    }

    for (auto& keyFrame : keyFrames)
    {
        float total_dist = keyFrame.DistSinceStop + keyFrame.DistUntilStop;
        if (total_dist < 2 * accel_dist) // won't reach full speed
        {
            if (keyFrame.DistSinceStop < keyFrame.DistUntilStop) // is still accelerating
            {
                // calculate accel+brake time for this short segment
                float segment_time = 2.0f * sqrt((keyFrame.DistUntilStop + keyFrame.DistSinceStop) / accel);
                // substract acceleration time
                keyFrame.TimeTo = segment_time - sqrt(2 * keyFrame.DistSinceStop / accel);
            }
            else // slowing down
                keyFrame.TimeTo = sqrt(2 * keyFrame.DistUntilStop / accel);
        }
        else if (keyFrame.DistSinceStop < accel_dist) // still accelerating (but will reach full speed)
        {
            // calculate accel + cruise + brake time for this long segment
            float segment_time = (keyFrame.DistUntilStop + keyFrame.DistSinceStop) / speed + (speed / accel);
            // substract acceleration time
            keyFrame.TimeTo = segment_time - sqrt(2 * keyFrame.DistSinceStop / accel);
        }
        else if (keyFrame.DistUntilStop < accel_dist) // already slowing down (but reached full speed)
            keyFrame.TimeTo = sqrt(2 * keyFrame.DistUntilStop / accel);
        else // at full speed
            keyFrame.TimeTo = (keyFrame.DistUntilStop / speed) + (0.5f * speed / accel);
    }

    // calculate tFrom times from tTo times
    float segmentTime = 0.0f;
    for (size_t i = 0; i < keyFrames.size(); ++i)
    {
        int32 j = (i + lastStop) % keyFrames.size();
        if (keyFrames[j].IsStopFrame() || j == lastStop)
            segmentTime = keyFrames[j].TimeTo;
        keyFrames[j].TimeFrom = segmentTime - keyFrames[j].TimeTo;
    }

    // calculate path times
    keyFrames[0].ArriveTime = 0;
    float curPathTime = 0.0f;
    if (keyFrames[0].IsStopFrame())
    {
        curPathTime = float(keyFrames[0].Node->delay);
        keyFrames[0].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
    }

    for (size_t i = 1; i < keyFrames.size(); ++i)
    {
        curPathTime += keyFrames[i - 1].TimeTo;
        if (keyFrames[i].IsStopFrame())
        {
            keyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            curPathTime += float(keyFrames[i].Node->delay);
            keyFrames[i].DepartureTime = uint32(curPathTime * IN_MILLISECONDS);
        }
        else
        {
            curPathTime -= keyFrames[i].TimeTo;
            keyFrames[i].ArriveTime = uint32(curPathTime * IN_MILLISECONDS);
            keyFrames[i - 1].NextArriveTime = keyFrames[i].ArriveTime;
            keyFrames[i].DepartureTime = keyFrames[i].ArriveTime;
        }
    }

    keyFrames.back().NextArriveTime = keyFrames.back().DepartureTime;
    // the client destroys a transport by itself after a while, refresh is needed mid course
    // Feathermoon 303 & Teldrassil 293 ferries
    if (pathId == 303 || pathId == 293)
        keyFrames[12].Update = true;
    m_transportTemplate.pathTime = keyFrames.back().DepartureTime;

    return true;
}

void Transport::MoveToNextWayPoint()
{
    m_currentFrame = m_nextFrame++;
    if (m_nextFrame == GetKeyFrames().end())
        m_nextFrame = GetKeyFrames().begin();
}

void Transport::TeleportTransport(uint32 newMapid, float x, float y, float z, float o)
{
    Map const* oldMap = GetMap();
    Relocate(x, y, z);

    auto& passengers = GetPassengers();
    for (auto itr = passengers.begin(); itr != passengers.end();)
    {
        auto it2 = itr;
        ++itr;

        WorldObject* obj = *it2;
        if (!obj)
        {
            passengers.erase(it2);
            continue;
        }

        if (!obj->IsUnit())
            return;

        Unit* passengerUnit = static_cast<Unit*>(obj);

        float destX, destY, destZ, destO;
        destX = passengerUnit->GetTransOffsetX();
        destY = passengerUnit->GetTransOffsetY();
        destZ = passengerUnit->GetTransOffsetZ();
        destO = passengerUnit->GetTransOffsetO();
        CalculatePassengerPosition(destX, destY, destZ, &destO, x, y, z, o);

        if (obj->IsPlayer())
        {
            Player* player = static_cast<Player*>(obj);
            if (player->IsDead() && !player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST))
                player->ResurrectPlayer(1.0);
            player->TeleportTo(newMapid, destX, destY, destZ, destO, TELE_TO_NOT_LEAVE_TRANSPORT);
        }
    }

    // we need to create and save new Map object with 'newMapid' because if not done -> lead to invalid Map object reference...
    // player far teleport would try to create same instance, but we need it NOW for transport...
    // correct me if I'm wrong O.o
    Map* newMap = sMapMgr.CreateMap(newMapid, this);
    SetMap(newMap);

    if (oldMap != newMap)
    {
        UpdateForMap(oldMap);
        UpdateForMap(newMap);
    }
}

bool GenericTransport::AddPassenger(Unit* passenger)
{
    if (m_passengers.find(passenger) == m_passengers.end())
    {
        DETAIL_LOG("Unit %s boarded transport %s.", passenger->GetName(), GetName());
        m_passengers.insert(passenger);
    }
    return true;
}

bool GenericTransport::RemovePassenger(Unit* passenger)
{
    if (m_passengers.erase(passenger))
        DETAIL_LOG("Unit %s removed from transport %s.", passenger->GetName(), GetName());
    return true;
}

void Transport::Update(const uint32 diff)
{
    uint32 const positionUpdateDelay = 50;

    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving() || !m_pendingStop)
        m_pathProgress = m_pathProgress + diff;

    uint32 pathProgress = m_pathProgress % GetPeriod();
    while (true)
    {
        if (pathProgress >= m_currentFrame->ArriveTime && pathProgress < m_currentFrame->DepartureTime)
        {
            SetMoving(false);
            break;  // its a stop frame and we are waiting
        }

        // not waiting anymore
        SetMoving(true);

        if (pathProgress >= m_currentFrame->DepartureTime && pathProgress < m_currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWayPoint();

        // first check help in case client-server transport coordinates de-synchronization
        if (m_currentFrame->Node->mapid != GetMapId() || m_currentFrame->IsTeleportFrame())
        {
            TeleportTransport(m_nextFrame->Node->mapid, m_currentFrame->Node->x, m_currentFrame->Node->y, m_currentFrame->Node->z, m_currentFrame->InitialOrientation);
            return;
        }

        /*
        for(PlayerSet::const_iterator itr = m_passengers.begin(); itr != m_passengers.end();)
        {
            PlayerSet::const_iterator it2 = itr;
            ++itr;
            //(*it2)->SetPosition( m_curr->second.x + (*it2)->GetTransOffsetX(), m_curr->second.y + (*it2)->GetTransOffsetY(), m_curr->second.z + (*it2)->GetTransOffsetZ(), (*it2)->GetTransOffsetO() );
        }
        */

        if (m_currentFrame == GetKeyFrames().begin())
            DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, " ************ BEGIN ************** %s", GetName());

        DETAIL_FILTER_LOG(LOG_FILTER_TRANSPORT_MOVES, "%s moved to %f %f %f %d", GetName(), m_currentFrame->Node->x, m_currentFrame->Node->y, m_currentFrame->Node->z, m_currentFrame->Node->mapid);
    }

    // Set position
    m_positionChangeTimer.Update(diff);
    if (m_positionChangeTimer.Passed())
    {
        m_positionChangeTimer.Reset(positionUpdateDelay);
        if (IsMoving() && pathProgress)
        {
            float t = CalculateSegmentPos(float(pathProgress) * 0.001f);
            G3D::Vector3 pos, dir;
            m_currentFrame->Spline->evaluate_percent(m_currentFrame->Index, t, pos);
            m_currentFrame->Spline->evaluate_derivative(m_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, atan2(dir.y, dir.x) + M_PI);
        }
    }
}

float Transport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *m_currentFrame;
    float const speed = float(m_goInfo->moTransport.moveSpeed);
    float const accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f / IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = m_transportTemplate.accelTime;
    float accelDist = m_transportTemplate.accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < m_transportTemplate.accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return segmentPos / frame.NextDistFromPrev;
}

bool ElevatorTransport::Create(uint32 guidlow, uint32 name_id, Map* map, float x, float y, float z, float ang, float rotation0, float rotation1, float rotation2, float rotation3, uint32 animprogress, GOState go_state)
{
    if (GenericTransport::Create(guidlow, name_id, map, x, y, z, ang, rotation0, rotation1, rotation2, rotation3, animprogress, go_state))
    {
        m_pathProgress = 0;
        m_animationInfo = sTransportMgr.GetTransportAnimInfo(GetGOInfo()->id);
        m_currentSeg = 0;
        return true;
    }
    return false;
}

void ElevatorTransport::Update(const uint32 diff)
{
    if (!m_animationInfo)
        return;

    if (GetGoState() == GO_STATE_READY)
    {
        m_pathProgress += diff;
        // TODO: Fix movement in unloaded grid - currently GO will just disappear
        uint32 timer = GetMap()->GetCurrentMSTime() % m_animationInfo->TotalTime;
        TransportAnimationEntry const* nodeNext = m_animationInfo->GetNextAnimNode(timer);
        TransportAnimationEntry const* nodePrev = m_animationInfo->GetPrevAnimNode(timer);
        if (nodeNext && nodePrev)
        {
            m_currentSeg = nodePrev->TimeSeg;

            G3D::Vector3 posPrev = G3D::Vector3(nodePrev->X, -nodePrev->Y, nodePrev->Z);
            G3D::Vector3 posNext = G3D::Vector3(nodeNext->X, -nodeNext->Y, nodeNext->Z);
            G3D::Vector3 currentPos;
            if (posPrev == posNext)
                currentPos = posPrev;
            else
            {
                uint32 timeElapsed = timer - nodePrev->TimeSeg;
                uint32 timeDiff = nodeNext->TimeSeg - nodePrev->TimeSeg;
                G3D::Vector3 segmentDiff = posNext - posPrev;
                float velocityX = float(segmentDiff.x) / timeDiff, velocityY = float(segmentDiff.y) / timeDiff, velocityZ = float(segmentDiff.z) / timeDiff;

                currentPos = G3D::Vector3(timeElapsed * velocityX, timeElapsed * velocityY, timeElapsed * velocityZ);
                currentPos += posPrev;
            }

            currentPos += G3D::Vector3(m_stationaryPosition.x, m_stationaryPosition.y, m_stationaryPosition.z);

            UpdatePosition(currentPos.x, currentPos.y, currentPos.z, GetOrientation());
            SummonCreature(1, currentPos.x, currentPos.y, currentPos.z, GetOrientation(), TEMPSPAWN_TIMED_DESPAWN, 5000);
        }

    }
}

uint32 ElevatorTransport::GetPathProgress() const
{
    return GetMap()->GetCurrentMSTime() % m_animationInfo->TotalTime;
}

void GenericTransport::UpdatePosition(float x, float y, float z, float o)
{
    Relocate(x, y, z, o);
    UpdateModelPosition();

    UpdatePassengerPositions(m_passengers);
}

void GenericTransport::UpdatePassengerPositions(PassengerSet& passengers)
{
    for (const auto passenger : passengers)
        UpdatePassengerPosition(passenger);
}

void GenericTransport::UpdatePassengerPosition(WorldObject* passenger)
{
    // transport teleported but passenger not yet (can happen for players)
    if (passenger->IsInWorld() && passenger->GetMap() != GetMap())
        return;

    if (!passenger->IsUnit())
        return;

    Unit* passengerUnit = static_cast<Unit*>(passenger);

    // Do not use Unit::UpdatePosition here, we don't want to remove auras
    // as if regular movement occurred
    float x, y, z, o;
    x = passengerUnit->GetTransOffsetX();
    y = passengerUnit->GetTransOffsetY();
    z = passengerUnit->GetTransOffsetZ();
    o = passengerUnit->GetTransOffsetO();
    CalculatePassengerPosition(x, y, z, &o);
    if (!MaNGOS::IsValidMapCoord(x, y, z))
    {
        sLog.outError("[TRANSPORTS] Object %s [guid %u] has invalid position on transport.", passenger->GetName(), passenger->GetGUIDLow());
        return;
    }
    switch (passenger->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            Creature* creature = dynamic_cast<Creature*>(passenger);
            GetMap()->CreatureRelocation(creature, x, y, z, o);
            creature->m_movementInfo.t_time = GetPathProgress();
            break;
        }
        case TYPEID_PLAYER:
            //relocate only passengers in world and skip any player that might be still logging in/teleporting
            if (passenger->IsInWorld())
                GetMap()->PlayerRelocation(dynamic_cast<Player*>(passenger), x, y, z, o);
            else
            {
                passenger->Relocate(x, y, z, o);
                dynamic_cast<Player*>(passenger)->m_movementInfo.t_guid = GetObjectGuid();
            }
            dynamic_cast<Player*>(passenger)->m_movementInfo.t_time = GetPathProgress();
            break;
        case TYPEID_GAMEOBJECT:
            //GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o, false);
            break;
        case TYPEID_DYNAMICOBJECT:
            //GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
            break;
        default:
            break;
    }
}

void GenericTransport::CalculatePassengerPosition(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
{
    float inx = x, iny = y, inz = z;
    if (o)
        *o = MapManager::NormalizeOrientation(transO + *o);

    x = transX + inx * std::cos(transO) - iny * std::sin(transO);
    y = transY + iny * std::cos(transO) + inx * std::sin(transO);
    z = transZ + inz;
}

void GenericTransport::CalculatePassengerOffset(float& x, float& y, float& z, float* o, float transX, float transY, float transZ, float transO)
{
    if (o)
        *o = MapManager::NormalizeOrientation(*o - transO);

    z -= transZ;
    y -= transY;    // y = searchedY * std::cos(o) + searchedX * std::sin(o)
    x -= transX;    // x = searchedX * std::cos(o) + searchedY * std::sin(o + pi)
    float inx = x, iny = y;
    y = (iny - inx * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
    x = (inx + iny * std::tan(transO)) / (std::cos(transO) + std::sin(transO) * std::tan(transO));
}

void Transport::UpdateForMap(Map const* targetMap)
{
    Map::PlayerList const& pl = targetMap->GetPlayers();
    if (pl.isEmpty())
        return;

    if (GetMapId() == targetMap->GetId())
    {
        for (const auto& itr : pl)
        {
            if (this != itr.getSource()->GetTransport())
            {
                UpdateData updateData;
                BuildCreateUpdateBlockForPlayer(&updateData, itr.getSource());
                WorldPacket packet = updateData.BuildPacket(0, true); // always only one packet
                itr.getSource()->SendDirectMessage(packet);
            }
        }
    }
    else
    {
        UpdateData updateData;
        BuildOutOfRangeUpdateBlock(&updateData);
        WorldPacket packet = updateData.BuildPacket(0, true); // always only one packet
        for (const auto& itr : pl)
            if (this != itr.getSource()->GetTransport())
                itr.getSource()->SendDirectMessage(packet);
    }
}

TransportTemplate::~TransportTemplate()
{
    // Collect shared pointers into a set to avoid deleting the same memory more than once
    std::set<TransportSpline*> splines;
    for (const auto& keyFrame : keyFrames)
        splines.insert(keyFrame.Spline);

    for (const auto& spline : splines)
        delete spline;
}
