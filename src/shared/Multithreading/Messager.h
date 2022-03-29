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

#ifndef MANGOS_MESSAGER_H
#define MANGOS_MESSAGER_H

#include <vector>
#include <mutex>
#include <functional>

template <class T>
class Messager
{
    public:
        void AddMessage(const std::function<void(T*)>& message)
        {
            std::lock_guard<std::mutex> guard(m_messageMutex);
            m_messageVector.push_back(message);
        }
        void Execute(T* object)
        {
            std::vector<std::function<void(T*)>> messageVectorCopy;
            {
                std::lock_guard<std::mutex> guard(m_messageMutex);
                std::swap(m_messageVector, messageVectorCopy);
            }
            for (auto& message : messageVectorCopy)
                message(object);

            messageVectorCopy.clear();
        }
    private:
        std::vector<std::function<void(T*)>> m_messageVector;
        std::mutex m_messageMutex;  
};

#endif