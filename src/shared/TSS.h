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

#ifndef TSS_H
#define TSS_H

#include <boost/thread/tss.hpp>

#include <functional>

namespace MaNGOS
{
template <typename T>
class thread_local_ptr : public boost::thread_specific_ptr<T>
{
    private:
        typedef std::function<T* ()> Generate;
        typedef std::function<void(T*)> Cleanup;

        Generate m_generator;

    public:
        thread_local_ptr() : m_generator([] () { return new T; }) {}
        thread_local_ptr(Generate generator) : m_generator(generator) {}
        thread_local_ptr(Generate generator, Cleanup cleanup) : m_generator(generator), boost::thread_specific_ptr<T>(cleanup) {}

        // this get allows for a nullptr return value
        T* get_value() { return boost::thread_specific_ptr<T>::get(); }

        T* get()
        {
            T* ret = get_value();
            if (!ret)
            {
                ret = m_generator();
                boost::thread_specific_ptr<T>::reset(ret);
            }
            return ret;
        }

        T* operator -> ()
        {
            return get();
        }

        typename boost::detail::sp_dereference< T >::type operator*() const
        {
            return *get();
        }
};
}

#endif /*TSS_H*/