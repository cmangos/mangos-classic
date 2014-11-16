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

#include "ObjectPosSelector.h"
#include "Object.h"

// The bigger this value, the more space npcs require around their target
#define OCCUPY_POS_ANGLE_ATAN_FACTOR                      1.8f

ObjectPosSelector::ObjectPosSelector(float x, float y, float dist, float searchedForSize, WorldObject const* searchPosFor) :
    m_centerX(x), m_centerY(y), m_searcherDist(dist), m_searchPosFor(searchPosFor)
{
    // if size == 0, m_anglestep will become 0 -> freeze
    if (searchedForSize == 0.0f)
        searchedForSize = DEFAULT_WORLD_OBJECT_SIZE;

    // undefined behaviour
    if (m_searcherDist == 0.0f)
        m_searcherDist = DEFAULT_WORLD_OBJECT_SIZE;

    m_searchedForReqHAngle = atan(OCCUPY_POS_ANGLE_ATAN_FACTOR * searchedForSize / m_searcherDist);

    // Really init in InitilizeAngle
    m_nextUsedAreaItr[USED_POS_PLUS]  = m_UsedAreaLists[USED_POS_PLUS].begin();
    m_nextUsedAreaItr[USED_POS_MINUS] = m_UsedAreaLists[USED_POS_MINUS].begin();
    m_stepAngle[USED_POS_PLUS]  = 0.0f;
    m_stepAngle[USED_POS_MINUS] = 0.0f;
}

/**
 * Add used area (circle) near target object excluded from possible searcher position
 *
 *
 * @param obj  Object that occupies area
 * @param angle Angle of used circle center point from target-searcher line
 * @param dist  Distance from target object center point to used circle center point
 *
 * Used circles data stored as projections to searcher dist size circle as angle coordinate and half angle size
 */
void ObjectPosSelector::AddUsedArea(WorldObject const* obj, float angle, float dist)
{
    MANGOS_ASSERT(obj);

    // skip some unexpected results.
    if (dist == 0.0f)
        return;

    // (half) angle that obj occupies
    float sr_angle = atan(OCCUPY_POS_ANGLE_ATAN_FACTOR * obj->GetObjectBoundingRadius() / dist);

    if (angle >= 0)
        m_UsedAreaLists[USED_POS_PLUS].insert(UsedArea(angle, OccupiedArea(sr_angle, obj)));
    else
        m_UsedAreaLists[USED_POS_MINUS].insert(UsedArea(-angle, OccupiedArea(sr_angle, obj)));
}

/**
 * Check searcher circle not intercepting with used circle
 *
 * @param usedArea Used circle as projection to searcher distance circle in angles form
 * @param side     Side of used circle
 * @param angle    Checked angle
 *
 * @return true, if used circle not intercepted with searcher circle in terms projection angles
 */
bool ObjectPosSelector::CheckAngle(UsedArea const& usedArea, UsedAreaSide side, float angle) const
{
    float used_angle = usedArea.first * SignOf(side);
    float used_offset = usedArea.second.angleOffset;

    return fabs(used_angle - angle) > used_offset || (m_searchPosFor && usedArea.second.occupyingObj == m_searchPosFor);
}

/**
 * Check original (0.0f) angle fit to existed used area excludes
 *
 * @return true, if 0.0f angle with m_searcher_halfangle*2 angle size not intercept with used circles
 */
bool ObjectPosSelector::CheckOriginalAngle() const
{
    // check first left/right used angles if exists
    return (m_UsedAreaLists[USED_POS_PLUS].empty()  || CheckAngle(*m_UsedAreaLists[USED_POS_PLUS].begin(), USED_POS_PLUS, 0.0f)) &&
           (m_UsedAreaLists[USED_POS_MINUS].empty() || CheckAngle(*m_UsedAreaLists[USED_POS_MINUS].begin(), USED_POS_MINUS, 0.0f));
}

/**
 * Initialize data for search angles starting from first possible angle at both sides
 */
void ObjectPosSelector::InitializeAngle()
{
    InitializeAngle(USED_POS_PLUS);
    InitializeAngle(USED_POS_MINUS);
}

/**
 * Initialize data for search angles starting from first possible angle at side
 */
void ObjectPosSelector::InitializeAngle(UsedAreaSide side)
{
    m_nextUsedAreaItr[side] = m_UsedAreaLists[side].begin();

    // if another side not alow use 0.0f angle calculate possible value in 0..m_searchedForReqHAngle range
    if (!m_UsedAreaLists[~side].empty())
    {
        UsedArea const& otherArea = *m_UsedAreaLists[~side].begin();
        m_stepAngle[side] = std::max(m_searchedForReqHAngle + otherArea.second.angleOffset - otherArea.first, 0.0f);
    }
    else                                                    // Other side empty. start from 0
        m_stepAngle[side] = 0.0f;

    // As m_stepAngle will be incremented first in ::NextSideAngle
    m_stepAngle[side] -= m_searchedForReqHAngle;
}

/**
 * Find next angle in free area
 *
 * @param angle    Return at success found angle
 *
 * @return true, if angle found
 */
bool ObjectPosSelector::NextAngle(float& angle)
{
    // loop until both side fail and leave 0..PI
    for (;;)
    {
        // ++ direction less updated
        if (m_stepAngle[USED_POS_PLUS] < M_PI_F && m_stepAngle[USED_POS_PLUS] <= m_stepAngle[USED_POS_MINUS])
        {
            if (NextSideAngle(USED_POS_PLUS, angle))
                return true;
        }
        // -- direction less updated
        else if (m_stepAngle[USED_POS_MINUS] < M_PI_F)
        {
            if (NextSideAngle(USED_POS_MINUS, angle))
                return true;
        }
        // both sides finishes
        else
            break;
    }

    // no angles
    return false;
}

/**
 * Find next angle at side
 *
 * @param side     Side of angle
 * @param angle    Return at success found angle
 *
 * @return true, if angle found
 *
 */
bool ObjectPosSelector::NextSideAngle(UsedAreaSide side, float& angle)
{
    // next possible angle
    m_stepAngle[side] += (m_searchedForReqHAngle + 0.01);

    // prevent jump to another side
    if (m_stepAngle[side] > M_PI_F)
        return false;

    // no used area anymore on this side
    if (m_nextUsedAreaItr[side] == m_UsedAreaLists[side].end())
    {
        angle = m_stepAngle[side] * SignOf(side);
        return true;
    }

    // Already occupied and no better found
    if ((m_searchPosFor && m_nextUsedAreaItr[side]->second.occupyingObj == m_searchPosFor) ||
            // Next occupied is too far away
            (m_stepAngle[side] + m_searchedForReqHAngle < m_nextUsedAreaItr[side]->first - m_nextUsedAreaItr[side]->second.angleOffset))
    {
        angle = m_stepAngle[side] * SignOf(side);
        return true;
    }

    // angle set at first possible pos after passed m_nextUsedAreaItr
    m_stepAngle[side] = m_nextUsedAreaItr[side]->first + m_nextUsedAreaItr[side]->second.angleOffset;

    ++m_nextUsedAreaItr[side];

    return false;
}

/**
 * Find next angle in used area, that used if no angle found in free area with LoS
 *
 * @param angle    Return at success found angle
 *
 * @return true, if angle found
 */
bool ObjectPosSelector::NextUsedAngle(float& angle)
{
    if (m_nextUsedAreaItr[USED_POS_PLUS] == m_UsedAreaLists[USED_POS_PLUS].end() &&
            m_nextUsedAreaItr[USED_POS_MINUS] == m_UsedAreaLists[USED_POS_MINUS].end())
        return false;

    // ++ direction less updated
    if (m_nextUsedAreaItr[USED_POS_PLUS] != m_UsedAreaLists[USED_POS_PLUS].end() &&
            (m_nextUsedAreaItr[USED_POS_MINUS] == m_UsedAreaLists[USED_POS_MINUS].end() ||
             m_nextUsedAreaItr[USED_POS_PLUS]->first <= m_nextUsedAreaItr[USED_POS_MINUS]->first))
    {
        angle = m_nextUsedAreaItr[USED_POS_PLUS]->first * SignOf(USED_POS_PLUS);
        ++m_nextUsedAreaItr[USED_POS_PLUS];
    }
    else
    {
        angle = m_nextUsedAreaItr[USED_POS_MINUS]->first * SignOf(USED_POS_MINUS);
        ++m_nextUsedAreaItr[USED_POS_MINUS];
    }

    return true;
}
