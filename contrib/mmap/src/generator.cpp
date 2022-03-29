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

#include <sstream>

#include "MMapCommon.h"
#include "MapBuilder.h"

using namespace MMAP;

bool checkDirectories(bool debugOutput, const char* workdir)
{
    vector<string> dirFiles;
    char maps_dir[1024];
    char vmaps_dir[1024];
    char mmaps_dir[1024];
    char meshes_dir[1024];

    sprintf(maps_dir, "%s/%s", workdir, "maps");
    if (getDirContents(dirFiles, maps_dir) == LISTFILE_DIRECTORY_NOT_FOUND || !dirFiles.size())
    {
        printf("'%s' directory is empty or does not exist\n", maps_dir);
        return false;
    }

    dirFiles.clear();
    sprintf(vmaps_dir, "%s/%s", workdir, "vmaps");
    if (getDirContents(dirFiles, vmaps_dir, "*.vmtree") == LISTFILE_DIRECTORY_NOT_FOUND || !dirFiles.size())
    {
        printf("'%s' directory is empty or does not exist\n", vmaps_dir);
        return false;
    }

    dirFiles.clear();
    sprintf(mmaps_dir, "%s/%s", workdir, "mmaps");
    if (getDirContents(dirFiles, mmaps_dir) == LISTFILE_DIRECTORY_NOT_FOUND)
    {
        printf("'%s' directory does not exist\n", mmaps_dir);
        return false;
    }

    dirFiles.clear();
    if (debugOutput)
    {
        sprintf(meshes_dir, "%s/%s", workdir, "meshes");
        if (getDirContents(dirFiles, meshes_dir) == LISTFILE_DIRECTORY_NOT_FOUND)
        {
            printf("'%s' directory does not exist (no place to put debugOutput files)\n", meshes_dir);
            return false;
        }
    }

    return true;
}

void printUsage()
{
    printf("Generator command line args\n\n");
    printf("-? or /? or -h : Show this help\n");
    printf("\"[#]\" : Build maps using specified map IDs.\n");
    printf("--tile [#,#] : Build the specified tile\n");
    printf("--skipLiquid : liquid data for maps\n");
    printf("--skipContinents : skip continents\n");
    printf("--skipJunkMaps : junk maps include some unused\n");
    printf("--skipBattlegrounds : does not include PVP arenas\n");
    printf("--debug : create debugging files for use with RecastDemo\n");
    printf("--silent : Make script friendly. No wait for user input, error, completion.\n");
    printf("--offMeshInput [file.*] : Path to file containing off mesh connections data.\n\n");
    printf("--configInputPath [file.*] : Path to json configuration file.\n\n");
    printf("--buildGameObjects : builds only gameobject models for transports\n\n");
    printf("--threads [#]: specifies number of threads to use for maps processing\n\n");
    printf("--workdir [directory] : Path to basedir of maps/vmaps.\n\n");
    printf("Example:\nmovemapgen (generate all mmap with default arg\n"
           "movemapgen \"1 0 169\" (generate maps 1, 0 and 169)\n"
           "movemapgen 0 --tile 34,46 (builds only tile 34,46 of map 0)\n\n");
    printf("Please read readme file for more information and examples.\n");
}

bool handleArgs(int argc, char** argv,
                std::vector<uint32>& mapIds,
                int& tileX,
                int& tileY,
                bool& skipLiquid,
                bool& skipContinents,
                bool& skipJunkMaps,
                bool& skipBattlegrounds,
                bool& debugOutput,
                bool& silent,
                bool& buildGameObjects,
                char*& offMeshInputPath,
                char*& configInputPath,
                int& threads,
                char*& workdir)
{
    char* param = NULL;
    workdir = "./";

    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--tile") == 0 && i + 1 < argc)
        {
            param = argv[++i];
            if (!param)
                return false;

            char* stileX = strtok(param, ",");
            char* stileY = strtok(NULL, ",");

            int tilex = atoi(stileX);
            int tiley = atoi(stileY);

            if ((tilex > 0 && tilex < 64) || (tilex == 0 && strcmp(stileX, "0") == 0))
                tileX = tilex;
            if ((tiley > 0 && tiley < 64) || (tiley == 0 && strcmp(stileY, "0") == 0))
                tileY = tiley;

            if (tileX < 0 || tileY < 0)
            {
                printf("invalid tile coords.\n");
                return false;
            }
        }
        else if (strcmp(argv[i], "--skipLiquid") == 0)
        {
            skipLiquid = true;
        }
        else if (strcmp(argv[i], "--skipContinents") == 0)
        {
            skipContinents = true;
        }
        else if (strcmp(argv[i], "--skipJunkMaps") == 0)
        {
            skipJunkMaps = true;
        }
        else if (strcmp(argv[i], "--skipBattlegrounds") == 0)
        {
            skipBattlegrounds = true;
        }
        else if (strcmp(argv[i], "--debug") == 0)
        {
            debugOutput = true;
        }
        else if (strcmp(argv[i], "--silent") == 0)
        {
            silent = true;
        }
        else if (strcmp(argv[i], "--buildGameObjects") == 0)
        {
            buildGameObjects = true;
        }
        else if (strcmp(argv[i], "--offMeshInput") == 0 && i + 1 < argc)
        {
            param = argv[++i];
            if (!param)
                return false;

            offMeshInputPath = param;
        }
        else if (strcmp(argv[i], "--configInputPath") == 0 && i + 1 < argc)
        {
            param = argv[++i];
            if (!param)
                return false;

            configInputPath = param;
        }
        else if (strcmp(argv[i], "--threads") == 0 && i + 1 < argc)
        {
            param = argv[++i];
            if (!param)
                return false;

            threads = 0;
            try
            {
                threads = std::stoi(param);
            }
            catch (std::invalid_argument& e) {}
            catch (std::out_of_range& e) {}
            if (threads <= 0)
            {
                printf("Invalid number of threads.\n");
                return false;
            }
        }
        else if (strcmp(argv[i], "--workdir") == 0 && i + 1 < argc)
        {
            param = argv[++i];
            if (!param)
                return false;

            workdir = param;
        }
        else if ((strcmp(argv[i], "-?") == 0) || (strcmp(argv[i], "/?") == 0) || (strcmp(argv[i], "-h") == 0))
        {
            printUsage();
            exit(1);
        }
        else
        {
            std::istringstream iss(argv[i]);
            std::string token;
            while (std::getline(iss, token, ' ')) {
                try
                {
                    mapIds.push_back(std::stoi(token));
                }
                catch (std::invalid_argument& e) {}
                catch (std::out_of_range& e) {}
            }
            if (!mapIds.size()) {
                printf("Invalid map IDs provided.\n");
                return false;
            }
        }
    }

    return true;
}

int main(int argc, char** argv)
{
    std::vector<uint32> mapIds;
    int threads = -1;
    int tileX = -1, tileY = -1;

    bool skipLiquid = false;
    bool skipContinents = false;
    bool skipJunkMaps = false;
    bool skipBattlegrounds = false;
    bool debug = false;
    bool silent = false;
    bool buildGameObjects = false;

    char* offMeshInputPath = "offmesh.txt";
    char* configInputPath = "config.json";
    char* workdir = NULL;

    bool validParam = handleArgs(argc, argv, mapIds, tileX, tileY, skipLiquid,
                                 skipContinents, skipJunkMaps, skipBattlegrounds,
                                 debug, silent, buildGameObjects, offMeshInputPath, configInputPath, threads, workdir);

    if (!validParam)
    {
        if (!silent)
        {
            printf("You have specified invalid parameters (use -? for more help)");
            printUsage();
        }
        return -1;
    }

    if (threads == -1) {
        threads = std::thread::hardware_concurrency();
    }

    if ((mapIds.size() == 0) && debug)
    {
        if (silent)
            return -2;

        printf("You have specified debug output, but didn't specify maps to generate.\n");
        printf("This will generate debug output for ALL maps.\n");
        printf("Are you sure you want to continue? (y/n) ");
        if (getchar() != 'y')
            return 0;
    }

    if (!checkDirectories(debug, workdir))
        return -3;

    MapBuilder builder(configInputPath, threads, skipLiquid, skipContinents, skipJunkMaps, skipBattlegrounds, debug, offMeshInputPath, workdir);

    if (mapIds.size() == 1 && tileX > -1 && tileY > -1)
        builder.buildSingleTile(mapIds.front(), tileX, tileY);
    else
        builder.BuildMaps(mapIds);

    if (buildGameObjects)
    {
        builder.buildTransports();
    }

    if (!silent)
        printf("Movemap build is complete!\n");

    return 0;
}
