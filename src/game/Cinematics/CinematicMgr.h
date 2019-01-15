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

#ifndef MANGOS_CINEMATICMGR_H
#define MANGOS_CINEMATICMGR_H

#include "Entities/Object.h"
#include "M2Stores.h"

#define CINEMATIC_LOOKAHEAD (2 * IN_MILLISECONDS)
#define CINEMATIC_UPDATEDIFF 200

class Player;
class CinematicMgr;

typedef std::unique_ptr<CinematicMgr> CinematicMgrUPtr;

class CinematicMgr
{
    friend class Player;
public:
    explicit CinematicMgr(Player* playerref);
    ~CinematicMgr();

    // Cinematic camera data and remote sight functions
    uint32 GetActiveCinematicCamera() const { return m_activeCinematicCameraId; }
    void SetActiveCinematicCamera(uint32 cinematicCameraId = 0) { m_activeCinematicCameraId = cinematicCameraId; }
    bool IsOnCinematic() const { return (m_cinematicCamera != nullptr); }
    void BeginCinematic();
    void EndCinematic();
    bool UpdateCinematicLocation(uint32 diff);

private:
    // Remote location information
    Player*     player;

protected:
    uint32      m_cinematicDiff;
    uint32      m_lastCinematicCheck;
    uint32      m_activeCinematicCameraId;
    uint32      m_cinematicLength;
    FlyByCameraCollection* m_cinematicCamera;
    Position    m_remoteSightPosition;
    Creature*   m_CinematicObject;
};

#endif
