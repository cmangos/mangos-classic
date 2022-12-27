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

#include <chrono>

#include "Measurement.h"
#include "Util/Errors.h"

Measurement::Measurement(std::string measurement, std::map<std::string, std::string> tags, std::map<std::string, boost::any> fields,uint64 timestamp)
    : _measurement(measurement), _tags(tags), _fields(fields)
{
    if (!timestamp)
    {
        auto now = std::chrono::system_clock::now();
        _timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    }
    else
        _timestamp = timestamp;
}

std::stringstream& operator<<(std::stringstream& out, const Measurement& measurement)
{
    out << measurement._measurement;

    for (const auto& tag : measurement._tags)
        out << "," << tag.first << "=" << tag.second;

    out << " ";

    MANGOS_ASSERT(measurement._fields.size() > 0);

    const auto& last = measurement._fields.rbegin()->first;
    for (const auto& field : measurement._fields)
    {

        out << field.first << "=";

        if (field.second.type() == typeid(int32))
            out << boost::any_cast<int32>(field.second) << "i";
        else if (field.second.type() == typeid(int64))
            out << boost::any_cast<int64>(field.second) << "i";
        else if (field.second.type() == typeid(float))
            out << boost::any_cast<float>(field.second);
        else if (field.second.type() == typeid(bool))
            out << (boost::any_cast<bool>(field.second) ? "t" : "f");
        else if (field.second.type() == typeid(std::string))
            out << boost::any_cast<std::string>(field.second);
        else
            out << "undefined";

        if (measurement._fields.size() > 1 && field.first != last)
            out << ",";
    }

    out << " " << measurement._timestamp;

    return out;
}
