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

#ifndef DBCSTORE_H
#define DBCSTORE_H

#include "DBCFileLoader.h"

template<class T>
class DBCStorage
{
        typedef std::list<char*> StringPoolList;
    public:
        explicit DBCStorage(const char* f) : nCount(0), fieldCount(0), fmt(f), indexTable(nullptr), m_dataTable(nullptr) { }
        ~DBCStorage() { Clear(); }

        uint32  GetNumRows() const { return loaded ? data.size() : nCount; }
        char const* GetFormat() const { return fmt; }
        uint32 GetFieldCount() const { return fieldCount; }

        T const* LookupEntry(uint32 id) const
        {
            if (loaded)
            {
                typename std::map<uint32, T const*>::const_iterator it = data.find(id);
                if (it != data.end())
                    return it->second;
            }
            return (id >= nCount) ? nullptr : indexTable[id];
        }

        bool Load(char const* fn)
        {
            DBCFileLoader dbc;
            // Check if load was sucessful, only then continue
            if (!dbc.Load(fn, fmt))
                return false;

            fieldCount = dbc.GetCols();

            // load raw non-string data
            m_dataTable = (T*)dbc.AutoProduceData(fmt, nCount, (char**&)indexTable);

            // load strings from dbc data
            m_stringPoolList.push_back(dbc.AutoProduceStrings(fmt, (char*)m_dataTable));

            // error in dbc file at loading if nullptr
            return indexTable != nullptr;
        }

        void SetEntry(uint32 id, T* t) // Cryptic they say..
        {
            if (!loaded)
            {
                for (uint32 i = 0; i < nCount; ++i)
                {
                    T const* node = LookupEntry(i);
                    if (!node)
                        continue;
                    data[i] = node;
                }
                loaded = true;
            }
            data[id] = t;
        }

        bool LoadStringsFrom(char const* fn)
        {
            // DBC must be already loaded using Load
            if (!indexTable)
                return false;

            DBCFileLoader dbc;
            // Check if load was successful, only then continue
            if (!dbc.Load(fn, fmt))
                return false;

            // load strings from another locale dbc data
            m_stringPoolList.push_back(dbc.AutoProduceStrings(fmt, (char*)m_dataTable));

            return true;
        }

        void Clear()
        {
            if (loaded)
            {
                data.clear();
                loaded = false;
            }

            if (!indexTable)
                return;

            delete[]((char*)indexTable);
            indexTable = nullptr;
            delete[]((char*)m_dataTable);
            m_dataTable = nullptr;

            while (!m_stringPoolList.empty())
            {
                delete[] m_stringPoolList.front();
                m_stringPoolList.pop_front();
            }
            nCount = 0;
        }

        void EraseEntry(uint32 id) { assert(id < nCount && "To be erased entry must be in bounds!") ; indexTable[id] = nullptr; }
        void InsertEntry(T* entry, uint32 id) { assert(id < nCount && "To be inserted entry must be in bounds!"); indexTable[id] = entry; }

    private:
        uint32 nCount;
        uint32 fieldCount;
        char const* fmt;
        T** indexTable;
        T* m_dataTable;
        std::map<uint32, T const*> data;
        bool loaded;
        StringPoolList m_stringPoolList;
};

#endif
