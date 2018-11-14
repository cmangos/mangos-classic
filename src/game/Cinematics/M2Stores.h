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

#ifndef MANGOS_M2STORES_H
#define MANGOS_M2STORES_H

#include "Common.h"

struct CamLocation
{
    CamLocation() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    CamLocation(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
    float x, y, z, w;
};

struct FlyByCamera
{
    uint32 timeStamp;
    CamLocation locations;
};

typedef std::vector<FlyByCamera> FlyByCameraCollection;

extern std::unordered_map<uint32, FlyByCameraCollection> sFlyByCameraStore;

void LoadM2Cameras(std::string const& dataPath);

#endif
