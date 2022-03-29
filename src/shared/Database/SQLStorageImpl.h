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

#ifndef SQLSTORAGE_IMPL_H
#define SQLSTORAGE_IMPL_H

#include "ProgressBar.h"
#include "Log.h"
#include "DBCFileLoader.h"

template<class DerivedLoader, class StorageClass>
template<class S, class D>                                  // S source-type, D destination-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert(uint32 /*field_pos*/, S src, D& dst)
{
    dst = D(src);
}

template<class DerivedLoader, class StorageClass>
template<class S>                                           // S source-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert_to_bool(uint32 /*field_pos*/, S src, bool& dst)
{
    dst = (src != 0);
}

template<class DerivedLoader, class StorageClass>

void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert_str_to_str(uint32 /*field_pos*/, char const* src, char*& dst)
{
    if (!src)
    {
        dst = new char[1];
        *dst = 0;
    }
    else
    {
        uint32 l = strlen(src) + 1;
        dst = new char[l];
        memcpy(dst, src, l);
    }
}

template<class DerivedLoader, class StorageClass>
template<class S>                                           // S source-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert_to_str(uint32 /*field_pos*/, S /*src*/, char*& dst)
{
    dst = new char[1];
    *dst = 0;
}

template<class DerivedLoader, class StorageClass>
template<class D>                                           // D destination-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert_from_str(uint32 /*field_pos*/, char const* /*src*/, D& dst)
{
    dst = 0;
}

template<class DerivedLoader, class StorageClass>
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::convert_str_to_bool(uint32 /*field_pos*/, char const* /*src*/, bool& dst)
{
    dst = false;
}

template<class DerivedLoader, class StorageClass>
template<class S, class D>                                  // S source-type, D destination-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::default_fill(uint32 /*field_pos*/, S src, D& dst)
{
    dst = D(src);
}

template<class DerivedLoader, class StorageClass>
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::default_fill_to_str(uint32 /*field_pos*/, char const* /*src*/, char*& dst)
{
    dst = new char[1];
    *dst = 0;
}

template<class DerivedLoader, class StorageClass>
template<class V>                                           // V value-type
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::storeValue(V value, StorageClass& store, char* p, uint32 x, uint32& offset)
{
#if defined(__arm__)
    bool tmpbool;
    char tmpchar;
    uint32 tmpint;
    float tmpfloat;
    char* tmpstr = new char[1];
    uint64 tmpbigint;
#endif
    DerivedLoader* subclass = (static_cast<DerivedLoader*>(this));
    switch (store.GetDstFormat(x))
    {
#if defined(__arm__)
        case FT_LOGIC:
            subclass->convert_to_bool(x, value, tmpbool);
            memcpy(&(p[offset]), &tmpbool, sizeof(bool));
            offset += sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert(x, value, tmpchar);
            memcpy(&(p[offset]), &tmpchar, sizeof(char));
            offset += sizeof(char);
            break;
        case FT_INT:
            subclass->convert(x, value, tmpint);
            memcpy(&(p[offset]), &tmpint, sizeof(uint32));
            offset += sizeof(uint32);
            break;
        case FT_FLOAT:
            subclass->convert(x, value, tmpfloat);
            memcpy(&(p[offset]), &tmpfloat, sizeof(float));
            offset += sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_to_str(x, value, tmpstr);
            memcpy(&(p[offset]), &tmpstr, sizeof(char*));
            offset += sizeof(char*);
            break;
        case FT_NA:
            subclass->default_fill(x, value, tmpint);
            memcpy(&(p[offset]), &tmpint, sizeof(uint32));
            offset += sizeof(uint32);
            break;
        case FT_NA_BYTE:
            subclass->default_fill(x, value, tmpchar);
            memcpy(&(p[offset]), &tmpchar, sizeof(char));
            offset += sizeof(char);
            break;
        case FT_NA_FLOAT:
            subclass->default_fill(x, value, tmpfloat);
            memcpy(&(p[offset]), &tmpfloat, sizeof(float));
            offset += sizeof(float);
            break;
        case FT_64BITINT:
            subclass->default_fill(x, value, tmpbigint);
            memcpy(&(p[offset]), &tmpbigint, sizeof(uint64));
            offset += sizeof(uint64);
            break;
        case FT_IND:
        case FT_SORT:
            assert(false && "SQL storage not have sort field types");
            break;
        default:
            assert(false && "unknown format character");
            break;
#else
        case FT_LOGIC:
            subclass->convert_to_bool(x, value, *((bool*)(&p[offset])));
            offset += sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert(x, value, *((char*)(&p[offset])));
            offset += sizeof(char);
            break;
        case FT_INT:
            subclass->convert(x, value, *((uint32*)(&p[offset])));
            offset += sizeof(uint32);
            break;
        case FT_FLOAT:
            subclass->convert(x, value, *((float*)(&p[offset])));
            offset += sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_to_str(x, value, *((char**)(&p[offset])));
            offset += sizeof(char*);
            break;
        case FT_NA:
            subclass->default_fill(x, value, *((uint32*)(&p[offset])));
            offset += sizeof(uint32);
            break;
        case FT_NA_BYTE:
            subclass->default_fill(x, value, *((char*)(&p[offset])));
            offset += sizeof(char);
            break;
        case FT_NA_FLOAT:
            subclass->default_fill(x, value, *((float*)(&p[offset])));
            offset += sizeof(float);
            break;
        case FT_64BITINT:
            subclass->default_fill(x, value, *((uint64*)(&p[offset])));
            offset += sizeof(uint64);
            break;
        case FT_IND:
        case FT_SORT:
            assert(false && "SQL storage not have sort field types");
            break;
        default:
            assert(false && "unknown format character");
            break;
#endif
    }
}

template<class DerivedLoader, class StorageClass>
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::storeValue(char const* value, StorageClass& store, char* p, uint32 x, uint32& offset)
{
#if defined(__arm__)
    bool tmpbool;
    char tmpchar;
    uint32 tmpint;
    float tmpfloat;
    char* tmpstr = new char[1];
    uint64 tmpbigint;
#endif
    DerivedLoader* subclass = (static_cast<DerivedLoader*>(this));
    switch (store.GetDstFormat(x))
    {
#if defined(__arm__)
        case FT_LOGIC:
            subclass->convert_str_to_bool(x, value, tmpbool);
            memcpy(&(p[offset]), &tmpbool, sizeof(bool));
            offset += sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert_from_str(x, value, tmpchar);
            memcpy(&(p[offset]), &tmpchar, sizeof(char));
            offset += sizeof(char);
            break;
        case FT_INT:
            subclass->convert_from_str(x, value, tmpint);
            memcpy(&(p[offset]), &tmpint, sizeof(uint32));
            offset += sizeof(uint32);
            break;
        case FT_FLOAT:
            subclass->convert_from_str(x, value, tmpfloat);
            memcpy(&(p[offset]), &tmpfloat, sizeof(float));
            offset += sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_str_to_str(x, value, tmpstr);
            memcpy(&(p[offset]), &tmpstr, sizeof(char*));
            offset += sizeof(char*);
            break;
        case FT_NA_POINTER:
            subclass->default_fill_to_str(x, value, tmpstr);
            memcpy(&(p[offset]), &tmpstr, sizeof(char*));
            offset += sizeof(char*);
            break;
        case FT_64BITINT:
            subclass->convert_from_str(x, value, tmpbigint);
            memcpy(&(p[offset]), &tmpbigint, sizeof(uint64));
            offset += sizeof(uint64);
            break;
        case FT_IND:
        case FT_SORT:
            assert(false && "SQL storage not have sort field types");
            break;
        default:
            assert(false && "unknown format character");
            break;
#else
        case FT_LOGIC:
            subclass->convert_str_to_bool(x, value, *((bool*)(&p[offset])));
            offset += sizeof(bool);
            break;
        case FT_BYTE:
            subclass->convert_from_str(x, value, *((char*)(&p[offset])));
            offset += sizeof(char);
            break;
        case FT_INT:
            subclass->convert_from_str(x, value, *((uint32*)(&p[offset])));
            offset += sizeof(uint32);
            break;
        case FT_FLOAT:
            subclass->convert_from_str(x, value, *((float*)(&p[offset])));
            offset += sizeof(float);
            break;
        case FT_STRING:
            subclass->convert_str_to_str(x, value, *((char**)(&p[offset])));
            offset += sizeof(char*);
            break;
        case FT_NA_POINTER:
            subclass->default_fill_to_str(x, value, *((char**)(&p[offset])));
            offset += sizeof(char*);
            break;
        case FT_64BITINT:
            subclass->convert_from_str(x, value, *((uint64*)(&p[offset])));
            offset += sizeof(uint64);
            break;
        case FT_IND:
        case FT_SORT:
            assert(false && "SQL storage not have sort field types");
            break;
        default:
            assert(false && "unknown format character");
            break;
#endif
    }
}

template<class DerivedLoader, class StorageClass>
void SQLStorageLoaderBase<DerivedLoader, StorageClass>::Load(StorageClass& store, bool error_at_empty /*= true*/)
{
    Field* fields = nullptr;
    QueryResult* result  = WorldDatabase.PQuery("SELECT MAX(%s) FROM %s", store.EntryFieldName(), store.GetTableName());
    if (!result)
    {
        sLog.outError("Error loading %s table (not exist?)\n", store.GetTableName());
        Log::WaitBeforeContinueIfNeed();
        exit(1);                                            // Stop server at loading non exited table or not accessable table
    }

    uint32 maxRecordId = (*result)[0].GetUInt32() + 1;
    uint32 recordCount = 0;
    uint32 recordsize = 0;
    delete result;

    result = WorldDatabase.PQuery("SELECT COUNT(*) FROM %s", store.GetTableName());
    if (result)
    {
        fields = result->Fetch();
        recordCount = fields[0].GetUInt32();
        delete result;
    }

    result = WorldDatabase.PQuery("SELECT * FROM %s", store.GetTableName());

    if (!result)
    {
        if (error_at_empty)
            sLog.outError("%s table is empty!\n", store.GetTableName());
        else
            sLog.outString("%s table is empty!\n", store.GetTableName());

        recordCount = 0;
        return;
    }

    if (store.GetSrcFieldCount() != result->GetFieldCount())
    {
        recordCount = 0;
        sLog.outError("Error in %s table, probably sql file format was updated (there should be %d fields in sql).\n", store.GetTableName(), store.GetSrcFieldCount());
        delete result;
        Log::WaitBeforeContinueIfNeed();
        exit(1);                                            // Stop server at loading broken or non-compatible table.
    }

    // get struct size
    uint32 offset = 0;
    for (uint32 x = 0; x < store.GetDstFieldCount(); ++x)
    {
        switch (store.GetDstFormat(x))
        {
            case FT_LOGIC:
                recordsize += sizeof(bool);   break;
            case FT_BYTE:
                recordsize += sizeof(char);   break;
            case FT_INT:
                recordsize += sizeof(uint32); break;
            case FT_FLOAT:
                recordsize += sizeof(float);  break;
            case FT_STRING:
                recordsize += sizeof(char*);  break;
            case FT_NA:
                recordsize += sizeof(uint32); break;
            case FT_NA_BYTE:
                recordsize += sizeof(char);   break;
            case FT_NA_FLOAT:
                recordsize += sizeof(float);  break;
            case FT_NA_POINTER:
                recordsize += sizeof(char*);  break;
            case FT_64BITINT:
                recordsize += sizeof(uint64);  break;
            case FT_IND:
            case FT_SORT:
                assert(false && "SQL storage not have sort field types");
                break;
            default:
                assert(false && "unknown format character");
                break;
        }
    }

    // Prepare data storage and lookup storage
    store.prepareToLoad(maxRecordId, recordCount, recordsize);

    BarGoLink bar(recordCount);
    do
    {
        fields = result->Fetch();
        bar.step();

        char* record = store.createRecord(fields[0].GetUInt32());
        offset = 0;

        // dependend on dest-size
        // iterate two indexes: x over dest, y over source
        //                      y++ If and only If x != FT_NA*
        //                      x++ If and only If a value is stored
        for (uint32 x = 0, y = 0; x < store.GetDstFieldCount();)
        {
            switch (store.GetDstFormat(x))
            {
                // For default fill continue and do not increase y
                case FT_NA:         storeValue((uint32)0, store, record, x, offset);         ++x; continue;
                case FT_NA_BYTE:    storeValue((char)0, store, record, x, offset);           ++x; continue;
                case FT_NA_FLOAT:   storeValue((float)0.0f, store, record, x, offset);       ++x; continue;
                case FT_NA_POINTER: storeValue((char const*)nullptr, store, record, x, offset); ++x; continue;
                default:
                    break;
            }

            // It is required that the input has at least as many columns set as the output requires
            if (y >= store.GetSrcFieldCount())
                assert(false && "SQL storage has too few columns!");

            switch (store.GetSrcFormat(y))
            {
                case FT_LOGIC:  storeValue((bool)(fields[y].GetUInt32() > 0), store, record, x, offset);  ++x; break;
                case FT_BYTE:   storeValue((char)fields[y].GetUInt8(), store, record, x, offset);         ++x; break;
                case FT_INT:    storeValue((uint32)fields[y].GetUInt32(), store, record, x, offset);      ++x; break;
                case FT_FLOAT:  storeValue((float)fields[y].GetFloat(), store, record, x, offset);        ++x; break;
                case FT_STRING: storeValue((char const*)fields[y].GetString(), store, record, x, offset); ++x; break;
                case FT_64BITINT: storeValue((uint64)fields[y].GetUInt64(), store, record, x, offset);            ++x; break;
                case FT_NA:
                case FT_NA_BYTE:
                case FT_NA_FLOAT:
                    // Do Not increase x
                    break;
                case FT_IND:
                case FT_SORT:
                case FT_NA_POINTER:
                    assert(false && "SQL storage not have sort or pointer field types");
                    break;
                default:
                    assert(false && "unknown format character");
            }
            ++y;
        }
    }
    while (result->NextRow());

    delete result;
}

#endif
