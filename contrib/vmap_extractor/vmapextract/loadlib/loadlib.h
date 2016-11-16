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

#ifndef LOAD_LIB_H
#define LOAD_LIB_H

#include <cstdint>

typedef std::int64_t int64;
typedef std::int32_t int32;
typedef std::int16_t int16;
typedef std::int8_t int8;
typedef std::uint64_t uint64;
typedef std::uint32_t uint32;
typedef std::uint16_t uint16;
typedef std::uint8_t uint8;

#define FILE_FORMAT_VERSION    18

//
// File version chunk
//
struct file_MVER
{
    union
    {
        uint32 fcc;
        char   fcc_txt[4];
    };
    uint32 size;
    uint32 ver;
};

class FileLoader
{
        uint8*  data;
        uint32  data_size;
    public:
        virtual bool prepareLoadedData();
        uint8* GetData()     {return data;}
        uint32 GetDataSize() {return data_size;}

        file_MVER* version;
        FileLoader();
        ~FileLoader();
        bool loadFile(char* filename, bool log = true);
        virtual void free();
};
#endif
