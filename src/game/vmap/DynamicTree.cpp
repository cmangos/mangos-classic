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

#include "DynamicTree.h"
#include "Log.h"
#include "Timer.h"
#include "BIHWrap.h"
#include "RegularGrid.h"
#include "GameObjectModel.h"

template<> struct HashTrait< GameObjectModel>
{
    static size_t hashCode(const GameObjectModel& g) { return (size_t)(void*)&g; }
};

template<> struct PositionTrait< GameObjectModel>
{
    static void getPosition(const GameObjectModel& g, Vector3& p) { p = g.getPosition(); }
};

template<> struct BoundsTrait< GameObjectModel>
{
    static void getBounds(const GameObjectModel& g, G3D::AABox& out) { out = g.getBounds();}
    static void getBounds2(const GameObjectModel* g, G3D::AABox& out) { out = g->getBounds();}
};

/*
static bool operator == (const GameObjectModel& mdl, const GameObjectModel& mdl2){
    return &mdl == &mdl2;
}
*/

// int valuesPerNode = 5, numMeanSplits = 3;

//int UNBALANCED_TIMES_LIMIT = 5;
int CHECK_TREE_PERIOD = 200;

typedef RegularGrid2D<GameObjectModel, BIHWrap<GameObjectModel> > ParentTree;

struct DynTreeImpl : public ParentTree/*, public Intersectable*/
{
    typedef GameObjectModel Model;
    typedef ParentTree base;

    DynTreeImpl() :
        rebalance_timer(CHECK_TREE_PERIOD),
        unbalanced_times(0)
    {
    }

    void insert(const Model& mdl)
    {
        base::insert(mdl);
        ++unbalanced_times;
    }

    void remove(const Model& mdl)
    {
        base::remove(mdl);
        ++unbalanced_times;
    }

    void balance()
    {
        base::balance();
        unbalanced_times = 0;
    }

    void update(uint32 difftime)
    {
        if (!size())
            return;

        rebalance_timer.Update(difftime);
        if (rebalance_timer.Passed())
        {
            rebalance_timer.Reset(CHECK_TREE_PERIOD);
            if (unbalanced_times > 0)
                balance();
        }
    }

    ShortTimeTracker rebalance_timer;
    int unbalanced_times;
};

DynamicMapTree::DynamicMapTree() : impl(*new DynTreeImpl())
{
}

DynamicMapTree::~DynamicMapTree()
{
    delete &impl;
}

void DynamicMapTree::insert(const GameObjectModel& mdl)
{
    impl.insert(mdl);
}

void DynamicMapTree::remove(const GameObjectModel& mdl)
{
    impl.remove(mdl);
}

bool DynamicMapTree::contains(const GameObjectModel& mdl) const
{
    return impl.contains(mdl);
}

void DynamicMapTree::balance()
{
    impl.balance();
}

int DynamicMapTree::size() const
{
    return impl.size();
}

void DynamicMapTree::update(uint32 t_diff)
{
    impl.update(t_diff);
}

struct DynamicTreeIntersectionCallback
{
    bool did_hit;
    DynamicTreeIntersectionCallback() : did_hit(false) {}
    bool operator()(const G3D::Ray& r, const GameObjectModel& obj, float& distance)
    {
        did_hit = obj.intersectRay(r, distance, true);
        return did_hit;
    }
    bool didHit() const { return did_hit;}
};

struct DynamicTreeIntersectionCallback_WithLogger
{
    bool did_hit;
    DynamicTreeIntersectionCallback_WithLogger() : did_hit(false)
    {
        DEBUG_LOG("Dynamic Intersection log");
    }
    bool operator()(const G3D::Ray& r, const GameObjectModel& obj, float& distance)
    {
        DEBUG_LOG("testing intersection with %s", obj.name.c_str());
        bool hit = obj.intersectRay(r, distance, true);
        if (hit)
        {
            did_hit = true;
            DEBUG_LOG("result: intersects");
        }
        return hit;
    }
    bool didHit() const { return did_hit;}
};

//=========================================================
/**
If intersection is found within pMaxDist, sets pMaxDist to intersection distance and returns true.
Else, pMaxDist is not modified and returns false;
*/

bool DynamicMapTree::getIntersectionTime(const G3D::Ray& ray, const Vector3& endPos, float& pMaxDist) const
{
    float distance = pMaxDist;
    DynamicTreeIntersectionCallback callback;
    impl.intersectRay(ray, callback, distance, endPos);
    if (callback.didHit())
        pMaxDist = distance;
    return callback.didHit();
}
//=========================================================

bool DynamicMapTree::getObjectHitPos(float x1, float y1, float z1, float x2, float y2, float z2, float& rx, float& ry, float& rz, float pModifyDist) const
{
    Vector3 pos1 = Vector3(x1, y1, z1);
    Vector3 pos2 = Vector3(x2, y2, z2);
    Vector3 resultPos;
    bool result = getObjectHitPos(pos1, pos2, resultPos, pModifyDist);
    rx = resultPos.x;
    ry = resultPos.y;
    rz = resultPos.z;
    return result;
}

//=========================================================
/**
When moving from pos1 to pos2 check if we hit an object. Return true and the position if we hit one
Return the hit pos or the original dest pos
*/
bool DynamicMapTree::getObjectHitPos(const Vector3& pPos1, const Vector3& pPos2, Vector3& pResultHitPos, float pModifyDist) const
{
    bool result = false;
    float maxDist = (pPos2 - pPos1).magnitude();
    // valid map coords should *never ever* produce float overflow, but this would produce NaNs too:
    MANGOS_ASSERT(maxDist < std::numeric_limits<float>::max());
    // prevent NaN values which can cause BIH intersection to enter infinite loop
    if (maxDist < 1e-10f)
    {
        pResultHitPos = pPos2;
        return false;
    }
    Vector3 dir = (pPos2 - pPos1) / maxDist;            // direction with length of 1
    G3D::Ray ray(pPos1, dir);
    float dist = maxDist;
    if (getIntersectionTime(ray, pPos2, dist))
    {
        pResultHitPos = pPos1 + dir * dist;
        if (pModifyDist < 0)
        {
            if ((pResultHitPos - pPos1).magnitude() > -pModifyDist)
            {
                pResultHitPos = pResultHitPos + dir * pModifyDist;
            }
            else
            {
                pResultHitPos = pPos1;
            }
        }
        else
        {
            pResultHitPos = pResultHitPos + dir * pModifyDist;
        }
        result = true;
    }
    else
    {
        pResultHitPos = pPos2;
        result = false;
    }
    return result;
}

bool DynamicMapTree::isInLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2) const
{
    Vector3 v1(x1, y1, z1), v2(x2, y2, z2);

    float maxDist = (v2 - v1).magnitude();

    if (!G3D::fuzzyGt(maxDist, 0))
        return true;

    G3D::Ray r(v1, (v2 - v1) / maxDist);
    DynamicTreeIntersectionCallback callback;
    impl.intersectRay(r, callback, maxDist, v2);

    return !callback.did_hit;
}

float DynamicMapTree::getHeight(float x, float y, float z, float maxSearchDist) const
{
    Vector3 v(x, y, z);
    G3D::Ray r(v, Vector3(0, 0, -1));
    DynamicTreeIntersectionCallback callback;
    impl.intersectZAllignedRay(r, callback, maxSearchDist);

    if (callback.didHit())
        return v.z - maxSearchDist;
    else
        return -G3D::inf();
}
