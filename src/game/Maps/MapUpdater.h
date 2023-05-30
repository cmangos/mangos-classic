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

#ifndef _MAP_UPDATER_H_INCLUDED
#define _MAP_UPDATER_H_INCLUDED

#include "Platform/Define.h"
#include "Util/ProducerConsumerQueue.h"

#include <mutex>
#include <thread>
#include <atomic>
#include <vector>
#include <condition_variable>

class Worker;

class MapUpdater
{
    public:
        MapUpdater() : _cancelationToken(false), pending_requests(0) {}
        MapUpdater(size_t num_threads);
        MapUpdater(const MapUpdater&) = delete;
        
        void activate(size_t num_threads);
        void deactivate();
        void wait();
        void join();
        bool activated();
        void update_finished();
        void schedule_update(Worker* worker);

    private:
        ProducerConsumerQueue<Worker *> _queue;

        std::vector<std::thread> _workerThreads;
        std::atomic<bool> _cancelationToken;

        std::mutex _lock;
        std::condition_variable _condition;
        size_t pending_requests;

        void WorkerThread();
};

#endif //_MAP_UPDATER_H_INCLUDED