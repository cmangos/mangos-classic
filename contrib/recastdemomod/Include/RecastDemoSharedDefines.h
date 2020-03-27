#ifndef RECASTDEMOSHARED_H
#define RECASTDEMOSHARED_H

//#ifdef WIN32
//#	define snprintf _snprintf
//#endif

#include <cstddef>

/// These are just sample areas to use consistent values across the samples.
/// The use should specify these base on his needs.
enum SamplePolyAreas
{
    SAMPLE_POLYAREA_GROUND,
    SAMPLE_POLYAREA_WATER,
    SAMPLE_POLYAREA_ROAD,
    SAMPLE_POLYAREA_DOOR,
    SAMPLE_POLYAREA_GRASS,
    SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
    SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
    SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
    SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
    SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
    SAMPLE_POLYFLAGS_DISABLED = 0x10,		// Disabled polygon
    SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
};

enum SamplePartitionType
{
    SAMPLE_PARTITION_WATERSHED,
    SAMPLE_PARTITION_MONOTONE,
    SAMPLE_PARTITION_LAYERS,
};

namespace RecastDemo
{
    const static float BLOCK_SIZE = 533 + 1 / 3.0f;
    const static float BASE_UNIT_DIM = BLOCK_SIZE / 2000.0f;

    // All are in UNIT metrics!
    const static int VERTEX_PER_MAP = int(BLOCK_SIZE / BASE_UNIT_DIM + 0.5f);
    const static int VERTEX_PER_TILE = 80; // must divide VERTEX_PER_MAP
    const static int TILES_PER_MAP = VERTEX_PER_MAP / VERTEX_PER_TILE;
}

#endif