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

#include "packet_builder.h"
#include "MoveSpline.h"
#include "WorldPacket.h"

namespace Movement
{
    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    void PacketBuilder::WriteCommonMonsterMovePart(const MoveSpline& move_spline, WorldPacket& data)
    {
        MoveSplineFlag splineflags = move_spline.splineflags;

        /*if (mov.IsBoarded())
        {
            data.SetOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
            data << mov.GetTransport()->Owner.GetPackGUID();
        }*/

        data << move_spline.spline.getPoint(move_spline.spline.first());
        data << move_spline.GetId();

        switch (splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
            default:
                data << uint8(MonsterMoveNormal);
                break;
            case MoveSplineFlag::Final_Target:
                data << uint8(MonsterMoveFacingTarget);
                data << move_spline.facing.target;
                break;
            case MoveSplineFlag::Final_Angle:
                data << uint8(MonsterMoveFacingAngle);
                data << move_spline.facing.angle;
                break;
            case MoveSplineFlag::Final_Point:
                data << uint8(MonsterMoveFacingSpot);
                data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
                break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        splineflags.enter_cycle = move_spline.isCyclic();
        // add fake Runmode flag - client has strange issues without that flag
        data << uint32(splineflags & ~MoveSplineFlag::Mask_No_Monster_Move | MoveSplineFlag::Runmode);
        data << move_spline.Duration();
    }

    void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        Movement::SplineBase::ControlArray const& pathPoint = spline.getPoints(); // get ref of whole path points array

        uint32 pathSize = spline.last() - spline.first() - 1; // -1 as we send destination first and last index is destination
        MANGOS_ASSERT(pathSize >= 0);                       // should never be less than 0

        Vector3 destination = pathPoint[spline.last()];     // destination of this path should be send right after path size
        data << pathSize;
        data << destination;

        for (uint32 i = spline.first(); i < spline.first() + pathSize; i++) // from first real index (this array contain also special data)
        {
            Vector3 offset = destination - pathPoint[i];    // we have to send offset relative to destination instead of directly path point.
            data.appendPackXYZ(offset.x, offset.y, offset.z); // we have to pack x,y,z before send
        }
    }

    void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << count;
        data.append<Vector3>(&spline.getPoint(2), count);
    }

    void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << uint32(count + 1);
        data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.getPoint(1), count);
    }

    void PacketBuilder::WriteMonsterMove(const MoveSpline& move_spline, WorldPacket& data)
    {
        WriteCommonMonsterMovePart(move_spline, data);

        const Spline<int32>& spline = move_spline.spline;
        MoveSplineFlag splineflags = move_spline.splineflags;
        if (splineflags & MoveSplineFlag::Mask_CatmullRom)
        {
            if (splineflags.cyclic)
                WriteCatmullRomCyclicPath(spline, data);
            else
                WriteCatmullRomPath(spline, data);
        }
        else
            WriteLinearPath(spline, data);
    }

    void PacketBuilder::WriteCreate(const MoveSpline& move_spline, ByteBuffer& data)
    {
        // WriteClientStatus(mov,data);
        // data.append<float>(&mov.m_float_values[SpeedWalk], SpeedMaxCount);
        // if (mov.SplineEnabled())
        {
            MoveSplineFlag splineFlags = move_spline.splineflags;

            data << splineFlags.raw();

            if (splineFlags.final_point)
            {
                data << move_spline.facing.f.x << move_spline.facing.f.y << move_spline.facing.f.z;
            }
            else if (splineFlags.final_target)
            {
                data << move_spline.facing.target;
            }
            else if (splineFlags.final_angle)
            {
                data << move_spline.facing.angle;
            }

            data << move_spline.timePassed();
            data << move_spline.Duration();
            data << move_spline.GetId();

            uint32 nodes = move_spline.getPath().size();
            data << nodes;
            data.append<Vector3>(&move_spline.getPath()[0], nodes);
            data << (move_spline.isCyclic() ? Vector3::zero() : move_spline.FinalDestination());
        }
    }
}
