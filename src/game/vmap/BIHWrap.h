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

#pragma once

#include <G3D/Table.h>
#include <G3D/Array.h>
#include <G3D/Set.h>
#include "BIH.h"

template<class T, class BoundsFunc = BoundsTrait<T> >
class BIHWrap
{
        template<class RayCallback>
        struct MDLCallback
        {
            const T* const* objects;
            RayCallback& cb;

            MDLCallback(RayCallback& callback, const T* const* objects_array) : objects(objects_array), cb(callback) {}

            bool operator()(const Ray& r, uint32 Idx, float& MaxDist, bool /*stopAtFirst*/)
            {
                if (const T* obj = objects[Idx])
                    return cb(r, *obj, MaxDist/*, stopAtFirst*/);
                return false;
            }

            void operator()(const Vector3& p, uint32 Idx)
            {
                if (const T* obj = objects[Idx])
                    cb(p, *obj);
            }
        };

        typedef G3D::Array<const T*> ObjArray;

        BIH m_tree;
        ObjArray m_objects;
        G3D::Table<const T*, uint32> m_obj2Idx;
        G3D::Set<const T*> m_objects_to_push;
        int unbalanced_times;

    public:

        BIHWrap() : unbalanced_times(0) {}

        void insert(const T& obj)
        {
            ++unbalanced_times;
            m_objects_to_push.insert(&obj);
        }

        void remove(const T& obj)
        {
            ++unbalanced_times;
            uint32 Idx = 0;
            const T* temp;
            if (m_obj2Idx.getRemove(&obj, temp, Idx))
                m_objects[Idx] = nullptr;
            else
                m_objects_to_push.remove(&obj);
        }

        void balance()
        {
            if (unbalanced_times == 0)
                return;

            unbalanced_times = 0;
            m_objects.fastClear();
            m_obj2Idx.getKeys(m_objects);
            m_objects_to_push.getMembers(m_objects);

            m_tree.build(m_objects, BoundsFunc::getBounds2);
        }

        template<typename RayCallback>
        void intersectRay(const Ray& r, RayCallback& intersectCallback, float& maxDist) const
        {
            MDLCallback<RayCallback> temp_cb(intersectCallback, m_objects.getCArray());
            m_tree.intersectRay(r, temp_cb, maxDist, true);
        }

        template<typename IsectCallback>
        void intersectPoint(const Vector3& p, IsectCallback& intersectCallback) const
        {
            MDLCallback<IsectCallback> temp_cb(intersectCallback, m_objects.getCArray());
            m_tree.intersectPoint(p, temp_cb);
        }
};
