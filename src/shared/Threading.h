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

#ifndef THREADING_H
#define THREADING_H

#include <thread>
#include <atomic>

namespace MaNGOS
{
    class Runnable
    {
        public:
            virtual ~Runnable() {}
            virtual void run() = 0;

            void incReference() { ++m_refs; }
            void decReference()
            {
                if (!--m_refs)
                    delete this;
            }
        private:
            std::atomic_long m_refs;
    };

    enum Priority
    {
        Priority_Idle,
        Priority_Lowest,
        Priority_Low,
        Priority_Normal,
        Priority_High,
        Priority_Highest,
        Priority_Realtime,
    };

    class Thread
    {
        public:
            Thread();
            explicit Thread(Runnable* instance);
            ~Thread();

            bool wait();
            void destroy();

            void setPriority(Priority type);

            static void Sleep(unsigned long msecs);
            static std::thread::id currentId();

        private:
            Thread(const Thread&);
            Thread& operator=(const Thread&);

            static void ThreadTask(void* param);

            Runnable* const m_task;
            std::thread::id m_iThreadId;
            std::thread m_ThreadImp;
    };
}
#endif