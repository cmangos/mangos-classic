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

#include "wmo.h"
#include "vmapexport.h"
#include "vec3d.h"
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <map>
#include <fstream>
#include "adtfile.h"
#undef min
#undef max
#include "mpq_libmpq04.h"

using namespace std;
extern uint16* LiqType;

WMORoot::WMORoot(std::string& filename)
    : filename(filename), color(0), nTextures(0), nGroups(0), nPortals(0), nLights(0),
    nDoodadNames(0), nDoodadDefs(0), nDoodadSets(0), RootWMOID(0), flags(0)
{
}

bool WMORoot::open()
{
    MPQFile f(filename.c_str());
    if (f.isEof())
    {
        printf("No such file %s.\n", filename.c_str());
        return false;
    }

    uint32 size;
    char fourcc[5];

    StringSet failedPaths;

    while (!f.isEof())
    {
        f.read(fourcc, 4);
        f.read(&size, 4);

        flipcc(fourcc);
        fourcc[4] = 0;

        size_t nextpos = f.getPos() + size;

        if (!strcmp(fourcc, "MOHD")) //header
        {
            f.read(&nTextures, 4);
            f.read(&nGroups, 4);
            f.read(&nPortals, 4);
            f.read(&nLights, 4);
            f.read(&nDoodadNames, 4);
            f.read(&nDoodadDefs, 4);
            f.read(&nDoodadSets, 4);
            f.read(&color, 4);
            f.read(&RootWMOID, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&flags, 4);
        }
        else if (!strcmp(fourcc, "MODS"))
        {
            DoodadData.Sets.resize(size / sizeof(WMO::MODS));
            f.read(DoodadData.Sets.data(), size);
        }
        else if (!strcmp(fourcc, "MODN"))
        {
            char* ptr = f.getPointer();
            char* end = ptr + size;
            DoodadData.Paths = std::make_unique<char[]>(size);
            memcpy(DoodadData.Paths.get(), ptr, size);
            while (ptr < end)
            {
                std::string path = ptr;

                char* s = GetPlainName(ptr);
                fixnamen(s, strlen(s));
                fixname2(s, strlen(s));

                uint32 doodadNameIndex = ptr - f.getPointer();
                ptr += path.length() + 1;

                std::string fixedName;
                if (ExtractSingleModel(path, fixedName, failedPaths))
                    ValidDoodadNames.insert(doodadNameIndex);
            }
        }
        else if (!strcmp(fourcc, "MODD"))
        {
            DoodadData.Spawns.resize(size / sizeof(WMO::MODD));
            f.read(DoodadData.Spawns.data(), size);
        }
        else if (!strcmp(fourcc, "MOGN"))
        {
            GroupNames.resize(size);
            f.read(GroupNames.data(), size);
        }
        /*
        else if (!strcmp(fourcc,"MOTX"))
        {
        }
        else if (!strcmp(fourcc,"MOMT"))
        {
        }
        else if (!strcmp(fourcc,"MOGI"))
        {
        }
        else if (!strcmp(fourcc,"MOLT"))
        {
        }
        else if (!strcmp(fourcc,"MOSB"))
        {
        }
        else if (!strcmp(fourcc,"MOPV"))
        {
        }
        else if (!strcmp(fourcc,"MOPT"))
        {
        }
        else if (!strcmp(fourcc,"MOPR"))
        {
        }
        else if (!strcmp(fourcc,"MFOG"))
        {
        }
        */
        f.seek((int)nextpos);
    }
    f.close();
    return true;
}

bool WMORoot::ConvertToVMAPRootWmo(FILE* pOutfile)
{
    //printf("Convert RootWmo...\n");

    fwrite(szRawVMAPMagic, 1, 8, pOutfile);
    unsigned int nVectors = 0;
    fwrite(&nVectors, sizeof(nVectors), 1, pOutfile); // will be filled later
    fwrite(&nGroups, 4, 1, pOutfile);
    fwrite(&RootWMOID, 4, 1, pOutfile);
    return true;
}

WMORoot::~WMORoot()
{
}

WMOGroup::WMOGroup(std::string& filename) : filename(filename),
    MOPY(0), MOVI(0), MoviEx(0), MOVT(0), MOBA(0), MobaEx(0), hlq(0), LiquEx(0), LiquBytes(0)
{
}

bool WMOGroup::open()
{
    MPQFile f(filename.c_str());
    if (f.isEof())
    {
        printf("No such file.\n");
        return false;
    }
    uint32 size;
    char fourcc[5];
    while (!f.isEof())
    {
        f.read(fourcc, 4);
        f.read(&size, 4);
        flipcc(fourcc);
        if (!strcmp(fourcc, "MOGP")) //Fix sizeoff = Data size.
        {
            size = 68;
        }
        fourcc[4] = 0;
        size_t nextpos = f.getPos() + size;
        LiquEx_size = 0;
        liquflags = 0;

        if (!strcmp(fourcc, "MOGP")) //header
        {
            f.read(&groupName, 4);
            f.read(&descGroupName, 4);
            f.read(&mogpFlags, 4);
            f.read(bbcorn1, 12);
            f.read(bbcorn2, 12);
            f.read(&moprIdx, 2);
            f.read(&moprNItems, 2);
            f.read(&nBatchA, 2);
            f.read(&nBatchB, 2);
            f.read(&nBatchC, 4);
            f.read(&fogIdx, 4);
            f.read(&liquidType, 4);
            f.read(&groupWMOID, 4);

        }
        else if (!strcmp(fourcc, "MOPY"))
        {
            MOPY = new char[size];
            mopy_size = size;
            nTriangles = (int)size / 2;
            f.read(MOPY, size);
        }
        else if (!strcmp(fourcc, "MOVI"))
        {
            MOVI = new uint16[size / 2];
            f.read(MOVI, size);
        }
        else if (!strcmp(fourcc, "MOVT"))
        {
            MOVT = new float[size / 4];
            f.read(MOVT, size);
            nVertices = (int)size / 12;
        }
        else if (!strcmp(fourcc, "MONR"))
        {
        }
        else if (!strcmp(fourcc, "MOTV"))
        {
        }
        else if (!strcmp(fourcc, "MOBA"))
        {
            MOBA = new uint16[size / 2];
            moba_size = size / 2;
            f.read(MOBA, size);
        }
        else if (!strcmp(fourcc, "MODR"))
        {
            DoodadReferences.resize(size / sizeof(uint16));
            f.read(DoodadReferences.data(), size);
        }
        else if (!strcmp(fourcc, "MLIQ"))
        {
            liquflags |= 1;
            hlq = new WMOLiquidHeader;
            f.read(hlq, 0x1E);
            LiquEx_size = sizeof(WMOLiquidVert) * hlq->xverts * hlq->yverts;
            LiquEx = new WMOLiquidVert[hlq->xverts * hlq->yverts];
            f.read(LiquEx, LiquEx_size);
            int nLiquBytes = hlq->xtiles * hlq->ytiles;
            LiquBytes = new char[nLiquBytes];
            f.read(LiquBytes, nLiquBytes);

            /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
            llog << filename;
            llog << "\nbbox: " << bbcorn1[0] << ", " << bbcorn1[1] << ", " << bbcorn1[2] << " | " << bbcorn2[0] << ", " << bbcorn2[1] << ", " << bbcorn2[2];
            llog << "\nlpos: " << hlq->pos_x << ", " << hlq->pos_y << ", " << hlq->pos_z;
            llog << "\nx-/yvert: " << hlq->xverts << "/" << hlq->yverts << " size: " << size << " expected size: " << 30 + hlq->xverts*hlq->yverts*8 + hlq->xtiles*hlq->ytiles << std::endl;
            llog.close(); */
        }
        f.seek((int)nextpos);
    }
    f.close();
    return true;
}

int WMOGroup::ConvertToVMAPGroupWmo(FILE* output, WMORoot* rootWMO, bool pPreciseVectorData)
{
    fwrite(&mogpFlags, sizeof(uint32), 1, output);
    fwrite(&groupWMOID, sizeof(uint32), 1, output);
    // group bound
    fwrite(bbcorn1, sizeof(float), 3, output);
    fwrite(bbcorn2, sizeof(float), 3, output);
    fwrite(&liquflags, sizeof(uint32), 1, output);
    int nColTriangles = 0;
    if (pPreciseVectorData)
    {
        char GRP[] = "GRP ";
        fwrite(GRP, 1, 4, output);

        int k = 0;
        int moba_batch = moba_size / 12;
        MobaEx = new int[moba_batch * 4];
        for (int i = 8; i < moba_size; i += 12)
        {
            MobaEx[k++] = MOBA[i];
        }
        int moba_size_grp = moba_batch * 4 + 4;
        fwrite(&moba_size_grp, 4, 1, output);
        fwrite(&moba_batch, 4, 1, output);
        fwrite(MobaEx, 4, k, output);
        delete [] MobaEx;

        uint32 nIdexes = nTriangles * 3;

        if (fwrite("INDX", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        int wsize = sizeof(uint32) + sizeof(unsigned short) * nIdexes;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            exit(0);
        }
        if (fwrite(&nIdexes, sizeof(uint32), 1, output) != 1)
        {
            printf("Error while writing file nIndexes");
            exit(0);
        }
        if (nIdexes > 0)
        {
            if (fwrite(MOVI, sizeof(unsigned short), nIdexes, output) != nIdexes)
            {
                printf("Error while writing file indexarray");
                exit(0);
            }
        }

        if (fwrite("VERT", 4, 1, output) != 1)
        {
            printf("Error while writing file nbraches ID");
            exit(0);
        }
        wsize = sizeof(int) + sizeof(float) * 3 * nVertices;
        if (fwrite(&wsize, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file wsize");
            exit(0);
        }
        if (fwrite(&nVertices, sizeof(int), 1, output) != 1)
        {
            printf("Error while writing file nVertices");
            exit(0);
        }
        if (nVertices > 0)
        {
            if (fwrite(MOVT, sizeof(float) * 3, nVertices, output) != nVertices)
            {
                printf("Error while writing file vectors");
                exit(0);
            }
        }

        nColTriangles = nTriangles;
    }
    else
    {
        char GRP[] = "GRP ";
        fwrite(GRP, 1, 4, output);
        int k = 0;
        int moba_batch = moba_size / 12;
        MobaEx = new int[moba_batch * 4];
        for (int i = 8; i < moba_size; i += 12)
        {
            MobaEx[k++] = MOBA[i];
        }

        int moba_size_grp = moba_batch * 4 + 4;
        fwrite(&moba_size_grp, 4, 1, output);
        fwrite(&moba_batch, 4, 1, output);
        fwrite(MobaEx, 4, k, output);
        delete [] MobaEx;

        //-------INDX------------------------------------
        //-------MOPY--------
        MoviEx = new uint16[nTriangles * 3]; // "worst case" size...
        int* IndexRenum = new int[nVertices];
        memset(IndexRenum, 0xFF, nVertices * sizeof(int));
        for (int i = 0; i < nTriangles; ++i)
        {
            // Skip no collision triangles
            bool isRenderFace = (MOPY[2 * i] & WMO_MATERIAL_RENDER) && !(MOPY[2 * i] & WMO_MATERIAL_DETAIL);
            bool isCollision = MOPY[2 * i] & WMO_MATERIAL_COLLISION || isRenderFace;
            if (!isCollision)
                continue;

            // Use this triangle
            for (int j = 0; j < 3; ++j)
            {
                IndexRenum[MOVI[3 * i + j]] = 1;
                MoviEx[3 * nColTriangles + j] = MOVI[3 * i + j];
            }
            ++nColTriangles;
        }

        // assign new vertex index numbers
        int nColVertices = 0;
        for (uint32 i = 0; i < nVertices; ++i)
        {
            if (IndexRenum[i] == 1)
            {
                IndexRenum[i] = nColVertices;
                ++nColVertices;
            }
        }

        // translate triangle indices to new numbers
        for (int i = 0; i < 3 * nColTriangles; ++i)
        {
            assert(MoviEx[i] < nVertices);
            MoviEx[i] = IndexRenum[MoviEx[i]];
        }

        // write triangle indices
        int INDX[] = {0x58444E49, nColTriangles * 6 + 4, nColTriangles * 3};
        fwrite(INDX, 4, 3, output);
        fwrite(MoviEx, 2, nColTriangles * 3, output);

        // write vertices
        int VERT[] = {0x54524556, int(nColVertices * 3 * sizeof(float) + 4), nColVertices}; // "VERT"
        int check = 3 * nColVertices;
        fwrite(VERT, 4, 3, output);
        for (uint32 i = 0; i < nVertices; ++i)
            if (IndexRenum[i] >= 0)
                check -= fwrite(MOVT + 3 * i, sizeof(float), 3, output);

        assert(check == 0);

        delete [] MoviEx;
        delete [] IndexRenum;
    }

    //------LIQU------------------------
    if (LiquEx_size != 0)
    {
        int LIQU_h[] = {0x5551494C, int(sizeof(WMOLiquidHeader) + LiquEx_size + hlq->xtiles * hlq->ytiles)}; // "LIQU"
        fwrite(LIQU_h, 4, 2, output);

        // according to https://wowdev.wiki/
        uint32 liquidEntry;
        if (rootWMO->flags & 4)
            liquidEntry = liquidType;
        else if (liquidType == 15)
            liquidEntry = 0;
        else
            liquidEntry = liquidType + 1;

        if (!liquidEntry)
        {
            int v1; // edx@1
            int v2; // eax@1

            v1 = hlq->xtiles * hlq->ytiles;
            v2 = 0;
            if (v1 > 0)
            {
                while ((LiquBytes[v2] & 0xF) == 15)
                {
                    ++v2;
                    if (v2 >= v1)
                        break;
                }

                if (v2 < v1 && (LiquBytes[v2] & 0xF) != 15)
                    liquidEntry = (LiquBytes[v2] & 0xF) + 1;
            }
        }

        if (liquidEntry && liquidEntry < 21)
        {
            switch (((uint8)liquidEntry - 1) & 3)
            {
                case 0:
                    liquidEntry = ((mogpFlags & 0x80000) != 0) + 1;
                    break;
                case 1:
                    liquidEntry = 2;        // ocean
                    break;
                case 2:
                    liquidEntry = 3;        // magma
                    break;
                case 3:
                    if (filename.find("Stratholme_raid") != string::npos)
                    {
                        liquidEntry = 21;   // Naxxramas slime
                    }
                    else
                        liquidEntry = 4;    // Normal slime
                    break;
                default:
                    break;
            }
        }

        hlq->type = liquidEntry;

        /* std::ofstream llog("Buildings/liquid.log", ios_base::out | ios_base::app);
        llog << filename;
        llog << ":\ntype: " << hlq->type << " (root:" << rootWMO->flags << " group:" << flags << ")\n";
        llog.close(); */

        fwrite(hlq, sizeof(WMOLiquidHeader), 1, output);
        // only need height values, the other values are unknown anyway
        for (uint32 i = 0; i < LiquEx_size / sizeof(WMOLiquidVert); ++i)
            fwrite(&LiquEx[i].height, sizeof(float), 1, output);
        // todo: compress to bit field
        fwrite(LiquBytes, 1, hlq->xtiles * hlq->ytiles, output);
    }

    return nColTriangles;
}

bool WMOGroup::ShouldSkip(WMORoot const& root) const
{
    // skip unreachable
    if (mogpFlags & 0x80)
        return true;

    // skip antiportals
    if (mogpFlags & 0x4000000)
        return true;

    if (groupName < int32(root.GroupNames.size()) && !strcmp(&root.GroupNames[groupName], "antiportal"))
        return true;

    return false;
}

WMOGroup::~WMOGroup()
{
    delete [] MOPY;
    delete [] MOVI;
    delete [] MOVT;
    delete [] MOBA;
    delete hlq;
    delete [] LiquEx;
    delete [] LiquBytes;
}

WMOInstance::WMOInstance(MPQFile& f, const char* WmoInstName, uint32 mapID, uint32 tileX, uint32 tileY, FILE* pDirfile)
{
    m_wmo.Position = Vec3D(0, 0, 0);

    float ff[3];
    f.read(&m_wmo.UniqueId, 4);
    f.read(ff, 12);
    m_wmo.Position = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    m_wmo.Rotation = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    m_wmo.Bounds.min = Vec3D(ff[0], ff[1], ff[2]);
    f.read(ff, 12);
    m_wmo.Bounds.max = Vec3D(ff[0], ff[1], ff[2]);
    f.read(&m_wmo.Flags, 2);
    f.read(&m_wmo.DoodadSet, 2);

    uint16 trash, adtId;
    f.read(&adtId, 2);
    f.read(&trash, 2);

    // destructible wmo, do not dump. we can handle the vmap for these
    // in dynamic tree (gameobject vmaps)
    if ((m_wmo.Flags & 0x1) != 0)
        return;

    //-----------add_in _dir_file----------------

    char tempname[512 + (sizeof(szWorkDirWmo) / sizeof(szWorkDirWmo[0]))];
    sprintf(tempname, "%s/%s", szWorkDirWmo, WmoInstName);
    FILE* input;
    input = fopen(tempname, "r+b");

    if (!input)
    {
        printf("WMOInstance::WMOInstance: couldn't open %s\n", tempname);
        return;
    }

    fseek(input, 8, SEEK_SET); // get the correct no of vertices
    int nVertices;
    fread(&nVertices, sizeof(int), 1, input);
    fclose(input);

    if (nVertices == 0)
        return;

    if (m_wmo.Position.x == 0 && m_wmo.Position.z == 0)
    {
        m_wmo.Position.x = 533.33333f * 32;
        m_wmo.Position.z = 533.33333f * 32;
    }
    m_wmo.Position = fixCoords(m_wmo.Position);
    m_wmo.Bounds.min = fixCoords(m_wmo.Bounds.min);
    m_wmo.Bounds.max = fixCoords(m_wmo.Bounds.max);

    uint32 uniqueId = GenerateUniqueObjectId(m_wmo.UniqueId, 0);

    float scale = 1.0f;
    uint32 flags = MOD_HAS_BOUND;
    if (tileX == 65 && tileY == 65) flags |= MOD_WORLDSPAWN;
    //write mapID, tileX, tileY, Flags, ID, Pos, Rot, Scale, Bound_lo, Bound_hi, name
    fwrite(&mapID, sizeof(uint32), 1, pDirfile);
    fwrite(&tileX, sizeof(uint32), 1, pDirfile);
    fwrite(&tileY, sizeof(uint32), 1, pDirfile);
    fwrite(&flags, sizeof(uint32), 1, pDirfile);
    fwrite(&adtId, sizeof(uint16), 1, pDirfile);
    fwrite(&uniqueId, sizeof(uint32), 1, pDirfile);
    fwrite(&m_wmo.Position, sizeof(Vec3D), 1, pDirfile);
    fwrite(&m_wmo.Rotation, sizeof(Vec3D), 1, pDirfile);
    fwrite(&scale, sizeof(float), 1, pDirfile);
    fwrite(&m_wmo.Bounds, sizeof(AaBox3D), 1, pDirfile);
    uint32 nlen = strlen(WmoInstName);
    fwrite(&nlen, sizeof(uint32), 1, pDirfile);
    fwrite(WmoInstName, sizeof(char), nlen, pDirfile);

    /* fprintf(pDirfile,"%s/%s %f,%f,%f_%f,%f,%f 1.0 %d %d %d,%d %d\n",
        MapName,
        WmoInstName,
        (float) x, (float) pos.y, (float) z,
        (float) rot.x, (float) rot.y, (float) rot.z,
        nVertices,
        realx1, realy1,
        realx2, realy2
        ); */

    // fclose(dirfile);
}
