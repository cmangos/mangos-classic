#ifndef RECASTDEMOTOOLS_H
#define RECASTDEMOTOOLS_H

//#include "CMaNGOS_Map.h"
class CMaNGOS_Map;

/// Tool types.
enum SampleToolType
{
    TOOL_NONE = 0,
    TOOL_TILE_EDIT,
    TOOL_TILE_HIGHLIGHT,
    TOOL_TEMP_OBSTACLE,
    TOOL_NAVMESH_TESTER,
    TOOL_CONVEX_VOLUME,
    TOOL_CROWD,
    TOOL_NAVMESH_PRUNE,
    TOOL_OFFMESH_CONNECTION,
    MAX_TOOLS
};

struct SampleTool
{
    virtual ~SampleTool() {}
    virtual int type() = 0;
    virtual void init(class CMaNGOS_Map* sample) = 0;
    virtual void reset() = 0;
    virtual void handleMenu() = 0;
    virtual void handleClick(const float* s, const float* p, bool shift) = 0;
    virtual void handleRender() = 0;
    virtual void handleRenderOverlay(double* proj, double* model, int* view) = 0;
    virtual void handleToggle() = 0;
    virtual void handleStep() = 0;
    virtual void handleUpdate(const float dt) = 0;
};

struct SampleToolState
{
    virtual ~SampleToolState() {}
    virtual void init(class CMaNGOS_Map* sample) = 0;
    virtual void reset() = 0;
    virtual void handleRender() = 0;
    virtual void handleRenderOverlay(double* proj, double* model, int* view) = 0;
    virtual void handleUpdate(const float dt) = 0;
};

#endif
