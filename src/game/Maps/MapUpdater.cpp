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

#include "MapUpdater.h"
#include "MapWorkers.h"

MapUpdater::MapUpdater(size_t num_threads) : _cancelationToken(false), pending_requests(0)
{
    for (size_t i = 0; i < num_threads; ++i)
        _workerThreads.push_back(std::thread(&MapUpdater::WorkerThread, this));
}

void MapUpdater::activate(size_t num_threads)
{
    if (activated())
        return;

    for (size_t i = 0; i < num_threads; ++i)
        _workerThreads.push_back(std::thread(&MapUpdater::WorkerThread, this));
}

void MapUpdater::deactivate()
{
    _cancelationToken = true;

    _queue.Cancel();

    for (auto& thread : _workerThreads)
        thread.join();
}

void MapUpdater::wait()
{
    std::unique_lock<std::mutex> lock(_lock);

    while (pending_requests > 0)
        _condition.wait(lock);
}

void MapUpdater::join()
{
    wait();
    deactivate();
}

bool MapUpdater::activated()
{
    return _workerThreads.size() > 0;
}

void MapUpdater::update_finished()
{
    std::lock_guard<std::mutex> lock(_lock);

    --pending_requests;
    _condition.notify_all();
}

void MapUpdater::schedule_update(Worker* worker)
{
    std::lock_guard<std::mutex> lock(_lock);

    ++pending_requests;
    _queue.Push(std::move(worker));
}

void MapUpdater::WorkerThread()
{
    while (true)
    {
        Worker* request = nullptr;

        _queue.WaitAndPop(request);

        if (_cancelationToken)
        {
            delete request;
            return;
        }

        request->execute();

        delete request;
    }
}