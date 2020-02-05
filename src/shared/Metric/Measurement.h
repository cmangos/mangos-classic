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

#ifndef MANGOSSERVER_MEASUREMENT_H
#define MANGOSSERVER_MEASUREMENT_H

#include <boost/any.hpp>
#include <iostream>
#include <map>

#include "Common.h"

struct Measurement
{
    Measurement(std::string measurement, std::map<std::string, std::string> tags, std::map<std::string, boost::any> fields, uint64 timestamp = 0);

    friend std::stringstream& operator<<(std::stringstream &out, const Measurement &measurement);

    std::string _measurement;
    std::map<std::string, std::string> _tags; // tags are used for selecting queries
    std::map<std::string, boost::any> _fields; // fields are used for displaying data
    uint64 _timestamp;
};

#endif // MANGOSSERVER_MEASUREMENT_H