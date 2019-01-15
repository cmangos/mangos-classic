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

#ifndef DBC_FILE_LOADER_H
#define DBC_FILE_LOADER_H
#include "Platform/Define.h"
#include "Utilities/ByteConverter.h"
#include <cassert>

enum FieldFormat
{
    FT_NA = 'x',                                            // ignore/ default, 4 byte size, in Source String means field is ignored, in Dest String means field is filled with default value
    FT_NA_BYTE = 'X',                                       // ignore/ default, 1 byte size, see above
    FT_NA_FLOAT = 'F',                                      // ignore/ default,  float size, see above
    FT_NA_POINTER = 'p',                                    // fill default value into dest, pointer size, Use this only with static data (otherwise mem-leak)
    FT_STRING = 's',                                        // char*
    FT_FLOAT = 'f',                                         // float
    FT_INT = 'i',                                           // uint32
    FT_BYTE = 'b',                                          // uint8
    FT_SORT = 'd',                                          // sorted by this field, field is not included
    FT_IND = 'n',                                           // the same,but parsed to data
    FT_LOGIC = 'l',                                         // Logical (boolean)
    FT_64BITINT = 'L'                                       // uint64
};

class DBCFileLoader
{
    public:
        DBCFileLoader();
        ~DBCFileLoader();

        bool Load(const char* filename, const char* fmt);

        class Record
        {
            public:
                float getFloat(size_t field) const
                {
                    assert(field < file.fieldCount);
                    float val = *reinterpret_cast<float*>(offset + file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                uint32 getUInt(size_t field) const
                {
                    assert(field < file.fieldCount);
                    uint32 val = *reinterpret_cast<uint32*>(offset + file.GetOffset(field));
                    EndianConvert(val);
                    return val;
                }
                uint8 getUInt8(size_t field) const
                {
                    assert(field < file.fieldCount);
                    return *reinterpret_cast<uint8*>(offset + file.GetOffset(field));
                }

                const char* getString(size_t field) const
                {
                    assert(field < file.fieldCount);
                    size_t stringOffset = getUInt(field);
                    assert(stringOffset < file.stringSize);
                    return reinterpret_cast<char*>(file.stringTable + stringOffset);
                }

            private:
                Record(DBCFileLoader& file_, unsigned char* offset_): offset(offset_), file(file_) {}
                unsigned char* offset;
                DBCFileLoader& file;

                friend class DBCFileLoader;
        };

        // Get record by id
        Record getRecord(size_t id);
        /// Get begin iterator over records

        uint32 GetNumRows() const { return recordCount;}
        uint32 GetCols() const { return fieldCount; }
        uint32 GetOffset(size_t id) const { return (fieldsOffset != nullptr && id < fieldCount) ? fieldsOffset[id] : 0; }
        bool IsLoaded() const { return data != nullptr; }
        char* AutoProduceData(const char* format, uint32& records, char**& indexTable);
        char* AutoProduceStrings(const char* format, char* dataTable);
        static uint32 GetFormatRecordSize(const char* format, int32* index_pos = nullptr);
    private:

        uint32 recordSize;
        uint32 recordCount;
        uint32 fieldCount;
        uint32 stringSize;
        uint32* fieldsOffset;
        unsigned char* data;
        unsigned char* stringTable;
};
#endif
