#ifndef MODELHEADERS_H
#define MODELHEADERS_H

typedef unsigned char uint8;
typedef char int8;
typedef unsigned short uint16;
typedef short int16;
typedef unsigned int uint32;
typedef int int32;

#pragma pack(push,1)

struct ModelHeader
{
    char id[4];
    uint8 version[4];
    uint32 nameLength;
    uint32 nameOfs;
    uint32 type;
    uint32 nGlobalSequences;
    uint32 ofsGlobalSequences;
    uint32 nAnimations;
    uint32 ofsAnimations;
    uint32 nC;
    uint32 ofsC;
    uint32 nD;
    uint32 ofsD;
    uint32 nBones;
    uint32 ofsBones;
    uint32 nF;
    uint32 ofsF;

    uint32 nVertices;
    uint32 ofsVertices;
    uint32 nViews;
    uint32 ofsViews;

    uint32 nColors;
    uint32 ofsColors;

    uint32 nTextures;
    uint32 ofsTextures;

    uint32 nTransparency; // H
    uint32 ofsTransparency;
    uint32 nI;   // always unused ?
    uint32 ofsI;
    uint32 nTexAnims;    // J
    uint32 ofsTexAnims;
    uint32 nK;
    uint32 ofsK;

    uint32 nTexFlags;
    uint32 ofsTexFlags;
    uint32 nY;
    uint32 ofsY;

    uint32 nTexLookup;
    uint32 ofsTexLookup;

    uint32 nTexUnitLookup;        // L
    uint32 ofsTexUnitLookup;
    uint32 nTransparencyLookup; // M
    uint32 ofsTransparencyLookup;
    uint32 nTexAnimLookup;
    uint32 ofsTexAnimLookup;

    float floats[14];

    uint32 nBoundingTriangles;
    uint32 ofsBoundingTriangles;
    uint32 nBoundingVertices;
    uint32 ofsBoundingVertices;
    uint32 nBoundingNormals;
    uint32 ofsBoundingNormals;

    uint32 nO;
    uint32 ofsO;
    uint32 nP;
    uint32 ofsP;
    uint32 nQ;
    uint32 ofsQ;
    uint32 nLights; // R
    uint32 ofsLights;
    uint32 nCameras; // S
    uint32 ofsCameras;
    uint32 nT;
    uint32 ofsT;
    uint32 nRibbonEmitters; // U
    uint32 ofsRibbonEmitters;
    uint32 nParticleEmitters; // V
    uint32 ofsParticleEmitters;

};

struct ModelVertex {
    Vec3D pos;
    uint8 weights[4];
    uint8 bones[4];
    Vec3D normal;
    Vec2D texcoords;
    int unk1, unk2; // always 0,0 so this is probably unused
};

struct ModelView {
    uint32 nIndex, ofsIndex; // Vertices in this model (index into vertices[])
    uint32 nTris, ofsTris;     // indices
    uint32 nProps, ofsProps; // additional vtx properties
    uint32 nSub, ofsSub;     // materials/renderops/submeshes
    uint32 nTex, ofsTex;     // material properties/textures
    int32 lod;                 // LOD bias?
};

/// A texture unit (sub of material)
struct ModelTexUnit{
    // probably the texture units
    // size always >=number of materials it seems
    uint16 flags;        // Flags
    uint16 order;        // ?
    uint16 op;            // Material this texture is part of (index into mat)
    uint16 op2;            // Always same as above?
    int16 colorIndex;    // color or -1
    uint16 flagsIndex;    // more flags...
    uint16 texunit;        // Texture unit (0 or 1)
    uint16 d4;            // ? (seems to be always 1)
    uint16 textureid;    // Texture id (index into global texture list)
    uint16 texunit2;    // copy of texture unit value?
    uint16 transid;        // transparency id (index into transparency list)
    uint16 texanimid;    // texture animation id
};

#pragma pack(pop)
#endif
