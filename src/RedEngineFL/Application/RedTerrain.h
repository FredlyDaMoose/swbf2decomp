#pragma once

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "Battlefront2/Source/CollisionObject.h"
#include "Battlefront2/Source/TreeGrid.h"

class PblFileChunk;
class PblVector3;

bool PointInSphere(const PblVector3& vPoint, const PblVector3& vCenter, float fRadius);

class RedTerrain
{
public:
    class Hole;

    struct DecalTile
    {
        unsigned char uiGridX;
        unsigned char uiGridZ;
        unsigned char uiTextureIndex;
        unsigned char _reserved_;
        unsigned short uiLLu;
        unsigned short uiLLv;
        unsigned short uiLRu;
        unsigned short uiLRv;
        unsigned short uiULu;
        unsigned short uiULv;
        unsigned short uiURu;
        unsigned short uiURv;
    };

    struct HeightExpand
    {
        unsigned char uiShift;
        unsigned char uiFlags;
        short iOffset;
    };

    struct HeightMinMax
    {
        short iMin;
        short iMax;
    };

    static void Read(PblFileChunk& chunk);
    static void Activate();
    static void Deactivate();
    static void Render(unsigned int uiFlags);

    static float GetHeight(int iX, int iZ);

    static int GetHeightUnscaled(int iX, int iZ);
    static bool PointInHoleCallback(const TreeGridObject* pObject, TreeGridCallbackData* pData, float& fDist);
    static float RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fA, float fB);

    static float GetInterpolatedHeight(float fX, float fZ);

    static float GetInterpolatedLight(float fX, float fZ);
    static float GetLight(float fX, float fZ);

    static float GetHoleHeight(float fX, float fZ);

    static float GetHoleHeight(const PblVector3& vPos) { return GetHoleHeight(vPos.x, vPos.z); }
    static float GetInterpolatedHeight(const PblVector3& vPos) { return GetInterpolatedHeight(vPos.x, vPos.z); }

    static void SetBumpDetailTexture(unsigned int uiHash, float fA, float fB);
    static void SetBumpSpecularTexture(unsigned int uiHash, float fA);
    static void SetFarTerrainMaxDistance(float fDist);
    static void SetFarTerrainTexture(unsigned int uiHash, float fA, unsigned int uiB, float fC);
    static void SetForceHiDetail(bool bForce);

    static const PblVector3 GetInterpolatedNormal(float fX, float fZ);

    static bool PointInHole(const PblVector3& vPos);
    static PblBox GetBoundingBox();
    static void Reset();

    static PblList<Hole> _HoleList;

    static int _iPointsPerDimension;
    static int _iPatchesPerDimension;
    static int _iHeightPatchesPerDimension;
    static int _iPatchPointsPerDimension;
    static int _iHeightPatchPointsPerDimension;
    static int _iNumPoints;
    static int _iNumHeightPatches;
    static int _iNumPatchPoints;
    static int _iNumHeightPatchPoints;
    static int _iPointsPerDimensionLog2;
    static int _iPatchesPerDimensionLog2;
    static int _iHeightPatchesPerDimensionLog2;
    static int _iPatchPointsPerDimensionLog2;
    static int _iHeightPatchPointsPerDimensionLog2;
    static int _iNumPointsLog2;
    static int _iNumPatchesLog2;
    static int _iNumHeightPatchesLog2;
    static int _iNumHeightPatchPointsLog2;
    static int _iPointsPerDimensionMask;
    static int _iPatchesPerDimensionMask;
    static int _iHeightPatchesPerDimensionMask;
    static int _iPatchPointsPerDimensionMask;
    static int _iHeightPatchPointsPerDimensionMask;
    static int _iNumPointsMask;
    static int _iNumPatchesMask;
    static int _iNumHeightPatchesMask;
    static int _iNumHeightPatchPointsMask;
    static int _iNumLayers;
    static int _iNumDecalTextures;
    static int _iNumDecalTiles;
    static float _fHeightScale;
    static float _fHeightMax;
    static float _fHeightMin;
    static float _fPointsPerMeter;
    static float _fTerrainCenterX;
    static float _fTerrainCenterZ;
    static unsigned char* _puiHeight;
    static unsigned int* _puiScorch;
    static unsigned char* _puiLuminosity;
    static unsigned int _uiDetailTextureHash;
    static float _fLayerMetersPerTexture[16];
    static unsigned int _uiLayerTextureHash[16];
    static unsigned int _uiDecalTextureHash[16];
    static float _uiLayerTextureRotation[16];
    static unsigned char _uiLayerTextureOrientation[16];
    static DecalTile* _pDecalTile;
    static bool _bTreeHoles;
    static bool RenderDetail;
    static HeightExpand* _pHeightExpand;
    static HeightMinMax* _pHeightMinMax;
    static unsigned char* _Foliage;
    static bool mTerrainColorDarkeningFlag;
    static bool _bEnable;
    static int mTerrainBytesUsed;
    static float _fMetersPerPoint;
    static int _iNumPatches;

private:

    static float _RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fTolerance);

    static float _RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fA, float fB, bool& bAbove);

    static void _ReadPatchInfo(PblFileChunk& chunk);

public:

    static void ProcessHoles(void (__cdecl *callback)(void*));
};

class RedTerrain::Hole : public TreeGridObject
{
public:
    Hole();

    Hole(Hole* pSource);

    int iNumPlanes;
    PblVector4* aPlanes;
    PblList<Hole>::Node node;
};
 
 
 
 

class HoleCallbackData : public TreeGridCallbackData
{
public:
    HoleCallbackData() { mData[0] = COLL_HOLE; }

    PblVector3 vPoint;
    float fRadius;
    bool* pbInHole;
};
 
 

bool PointInVolume(const PblVector3& vPoint, const PblVector4* aPlanes, int iNumPlanes);

// FUNCTION: SWBF2 0x005b4260
inline PblBox RedTerrain::GetBoundingBox()
{
    float fExtent = (float)_iPointsPerDimension * _fMetersPerPoint;
    return PblBox(-_fTerrainCenterX, _fHeightMin, -_fTerrainCenterZ,
                  fExtent - _fTerrainCenterX, _fHeightMax, fExtent - _fTerrainCenterZ);
}
