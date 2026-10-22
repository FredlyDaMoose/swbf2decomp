#include "RedTerrain.h"

#include <math.h>
#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "Battlefront2/Source/GameMemory.h"
#include "Battlefront2/Source/CollisionObject.h"
#include "Battlefront2/Source/CollisionManager.h"
#include "RedWater.h"

#include <float.h>

bool PointInSphere(const PblVector3& vPoint, const PblVector3& vCenter, float fRadius);

// FUNCTION: SWBF2 0x006c21e0
bool PointInSphere(const PblVector3& vPoint, const PblVector3& vCenter, float fRadius)
{
    const float fX = vPoint.x - vCenter.x;
    const float fY = vPoint.y - vCenter.y;
    const float fZ = vPoint.z - vCenter.z;
    return fRadius * fRadius > fX * fX + fY * fY + fZ * fZ;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

static const float EPSILON = 0.001f;

static int Log2(int i)
{
    int iLog = -1;
    for (; i; i >>= 1)
    {
        ++iLog;
    }
    return iLog;
}

// GLOBAL: SWBF2 0x0093e918
int RedTerrain::_iPointsPerDimensionLog2;
// GLOBAL: SWBF2 0x0093e91c
int RedTerrain::_iPatchesPerDimensionLog2;
// GLOBAL: SWBF2 0x0093e920
int RedTerrain::_iHeightPatchesPerDimensionLog2;
// GLOBAL: SWBF2 0x0093e924
int RedTerrain::_iPatchPointsPerDimensionLog2;
// GLOBAL: SWBF2 0x0093e928
int RedTerrain::_iHeightPatchPointsPerDimensionLog2;
// GLOBAL: SWBF2 0x0093e92c
int RedTerrain::_iNumPointsLog2;
// GLOBAL: SWBF2 0x0093e930
int RedTerrain::_iNumPatchesLog2;
// GLOBAL: SWBF2 0x0093e934
int RedTerrain::_iNumHeightPatchesLog2;
// GLOBAL: SWBF2 0x0093e938
int RedTerrain::_iNumHeightPatchPointsLog2;
// GLOBAL: SWBF2 0x0093e93c
int RedTerrain::_iPointsPerDimensionMask;
// GLOBAL: SWBF2 0x0093e940
int RedTerrain::_iPatchesPerDimensionMask;
// GLOBAL: SWBF2 0x0093e944
int RedTerrain::_iHeightPatchesPerDimensionMask;
// GLOBAL: SWBF2 0x0093e948
int RedTerrain::_iPatchPointsPerDimensionMask;
// GLOBAL: SWBF2 0x0093e94c
int RedTerrain::_iHeightPatchPointsPerDimensionMask;
// GLOBAL: SWBF2 0x0093e950
int RedTerrain::_iNumPointsMask;
// GLOBAL: SWBF2 0x0093e954
int RedTerrain::_iNumPatchesMask;
// GLOBAL: SWBF2 0x0093e958
int RedTerrain::_iNumHeightPatchesMask;
// GLOBAL: SWBF2 0x0093e95c
int RedTerrain::_iNumHeightPatchPointsMask;
// GLOBAL: SWBF2 0x0093e960
int RedTerrain::_iNumLayers;

// GLOBAL: SWBF2 0x0093e964
int RedTerrain::_iNumDecalTextures;
// GLOBAL: SWBF2 0x0093e968
int RedTerrain::_iNumDecalTiles;
// GLOBAL: SWBF2 0x0093e974
float RedTerrain::_fHeightScale;
// GLOBAL: SWBF2 0x0093e978
float RedTerrain::_fHeightMax;
// GLOBAL: SWBF2 0x0093e97c
float RedTerrain::_fHeightMin;
// GLOBAL: SWBF2 0x0093e980
float RedTerrain::_fMetersPerPoint;
// GLOBAL: SWBF2 0x0093e984
float RedTerrain::_fPointsPerMeter;

// GLOBAL: SWBF2 0x0093e988
float RedTerrain::_fTerrainCenterX;

// GLOBAL: SWBF2 0x0093e98c
float RedTerrain::_fTerrainCenterZ;
// GLOBAL: SWBF2 0x0093e990
unsigned char* RedTerrain::_puiHeight;
// GLOBAL: SWBF2 0x0093e994
unsigned int* RedTerrain::_puiScorch;
// GLOBAL: SWBF2 0x0093e998
unsigned char* RedTerrain::_puiLuminosity;
// GLOBAL: SWBF2 0x0093e99c
unsigned int RedTerrain::_uiDetailTextureHash;
// GLOBAL: SWBF2 0x0093e9a0
float RedTerrain::_fLayerMetersPerTexture[16];
// GLOBAL: SWBF2 0x0093e9e0
unsigned int RedTerrain::_uiLayerTextureHash[16];
// GLOBAL: SWBF2 0x0093ea20
unsigned int RedTerrain::_uiDecalTextureHash[16];

// GLOBAL: SWBF2 0x0093ea60
float RedTerrain::_uiLayerTextureRotation[16];
// GLOBAL: SWBF2 0x0093eaa0
unsigned char RedTerrain::_uiLayerTextureOrientation[16];
// GLOBAL: SWBF2 0x0093eab0
RedTerrain::DecalTile* RedTerrain::_pDecalTile;

// GLOBAL: SWBF2 0x0093eab5
bool RedTerrain::_bTreeHoles;

// GLOBAL: SWBF2 0x007deda0
bool RedTerrain::RenderDetail = true;
// GLOBAL: SWBF2 0x0093eabc
RedTerrain::HeightExpand* RedTerrain::_pHeightExpand;
// GLOBAL: SWBF2 0x0093eac0
RedTerrain::HeightMinMax* RedTerrain::_pHeightMinMax;

// GLOBAL: SWBF2 0x0093eac4
bool RedTerrain::mTerrainColorDarkeningFlag;
// GLOBAL: SWBF2 0x0093eac8
unsigned char* RedTerrain::_Foliage;
// GLOBAL: SWBF2 0x0093eb50
int RedTerrain::_iPointsPerDimension;
// GLOBAL: SWBF2 0x0093eb54
int RedTerrain::_iPatchesPerDimension;
// GLOBAL: SWBF2 0x0093eb58
int RedTerrain::_iHeightPatchesPerDimension;
// GLOBAL: SWBF2 0x0093eb5c
int RedTerrain::_iPatchPointsPerDimension;
// GLOBAL: SWBF2 0x0093eb60
int RedTerrain::_iHeightPatchPointsPerDimension;
// GLOBAL: SWBF2 0x0093eb64
int RedTerrain::_iNumPoints;
// GLOBAL: SWBF2 0x0093eb68
int RedTerrain::_iNumPatches;
// GLOBAL: SWBF2 0x0093eb6c
int RedTerrain::_iNumHeightPatches;

// GLOBAL: SWBF2 0x0093eb70
int RedTerrain::_iNumPatchPoints;
// GLOBAL: SWBF2 0x0093eb74
int RedTerrain::_iNumHeightPatchPoints;

// GLOBAL: SWBF2 0x0093eab7
bool sForceHiDetail;

// FUNCTION: SWBF2 0x004108b0
float RedTerrain::GetHeight(int iX, int iZ)
{
    if (!_bEnable)
    {
        return -1000.0f;
    }
    return (float)RedTerrain::GetHeightUnscaled(iX, iZ) * _fHeightScale;
}

// FUNCTION: SWBF2 0x006c15f0
RedTerrain::Hole::Hole() : TreeGridObject(-1)
{
    SetObjectData(0, COLL_HOLE);
    iNumPlanes = 0;
    aPlanes = 0;
}

// FUNCTION: SWBF2 0x006c1690
float RedTerrain::GetHoleHeight(float fX, float fZ)
{
    const PblVector3 vPos(fX, GetInterpolatedHeight(fX, fZ), fZ);
    if (PointInHole(vPos))
    {
        return -300.0f;
    }
    return vPos.y;
}

// FUNCTION: SWBF2 0x006c1720
float RedTerrain::GetInterpolatedHeight(float fX, float fZ)
{
    const float fMax = (float)(_iPointsPerDimension - 1) * _fMetersPerPoint;
    float fPosX = fX + _fTerrainCenterX;
    if (fPosX < 0.0f)
    {
        fPosX = 0.0f;
    }
    else if (fPosX > fMax)
    {
        fPosX = fMax;
    }
    float fPosZ = fZ + _fTerrainCenterZ;
    if (fPosZ < 0.0f)
    {
        fPosZ = 0.0f;
    }
    else if (fPosZ > fMax)
    {
        fPosZ = fMax;
    }

    const float fGridX = fPosX * _fPointsPerMeter;
    const float fGridZ = fPosZ * _fPointsPerMeter;
    const int iX = (int)fGridX;
    const int iZ = (int)fGridZ;
    const float fFracX = fGridX - (float)iX;
    const float fFracZ = fGridZ - (float)iZ;

    if (iZ & 1)
    {
        if (1.0f - fFracZ > fFracX)
        {
            const float fH0 = GetHeight(iX, iZ);
            const float fH = (GetHeight(iX + 1, iZ) - fH0) * fFracX + fH0;
            return fH + (GetHeight(iX, iZ + 1) - fH0) * fFracZ;
        }
        const float fH0 = GetHeight(iX + 1, iZ + 1);
        const float fH = (GetHeight(iX, iZ + 1) - fH0) * (1.0f - fFracX) + fH0;
        return fH + (GetHeight(iX + 1, iZ) - fH0) * (1.0f - fFracZ);
    }
    if (fFracZ > fFracX)
    {
        const float fH0 = GetHeight(iX, iZ + 1);
        const float fH = (GetHeight(iX + 1, iZ + 1) - fH0) * fFracX + fH0;
        return fH + (GetHeight(iX, iZ) - fH0) * (1.0f - fFracZ);
    }
    const float fH0 = GetHeight(iX + 1, iZ);
    const float fH = (GetHeight(iX + 1, iZ + 1) - fH0) * fFracZ;
    return fH + ((GetHeight(iX, iZ) - fH0) * (1.0f - fFracX) + fH0);
}

// FUNCTION: SWBF2 0x006c19b0
float RedTerrain::GetInterpolatedLight(float fX, float fZ)
{
    const float fHalf = _fMetersPerPoint * 0.5f;
    const float fX0 = fX - fHalf;
    const float fZ0 = fZ - fHalf;
    const int iX = (int)((fX0 + _fTerrainCenterX) * _fPointsPerMeter + 0.5f);
    const int iZ = (int)((fZ0 + _fTerrainCenterZ) * _fPointsPerMeter + 0.5f);
    const float fFracX = (fX - ((float)iX * _fMetersPerPoint - _fTerrainCenterX)) * _fPointsPerMeter;
    const float fFracZ = (fZ - ((float)iZ * _fMetersPerPoint - _fTerrainCenterZ)) * _fPointsPerMeter;

    const float fL00 = GetLight(fX0, fZ0);
    const float fL10 = GetLight(fX + fHalf, fZ0);
    const float fL01 = GetLight(fX0, fZ + fHalf);
    const float fL11 = GetLight(fX + fHalf, fZ + fHalf);
    const float fA = (fL01 - fL00) * fFracZ + fL00;
    const float fB = (fL11 - fL10) * fFracZ + fL10;
    return (fB - fA) * fFracX + fA;
}

static PblVector3 TriangleNormal(const PblVector3& p0, const PblVector3& p1, const PblVector3& p2)
{
    const PblVector3 a(p1.x - p0.x, p1.y - p0.y, p1.z - p0.z);
    const PblVector3 b(p2.x - p0.x, p2.y - p0.y, p2.z - p0.z);
    return PblVector3(b.y * a.z - b.z * a.y, b.z * a.x - b.x * a.z, b.x * a.y - b.y * a.x);
}

// FUNCTION: SWBF2 0x006c1b50
const PblVector3 RedTerrain::GetInterpolatedNormal(float fX, float fZ)
{
    const float fMax = (float)(_iPointsPerDimension - 1) * _fMetersPerPoint;
    float fPosX = fX + _fTerrainCenterX;
    if (fPosX < 0.0f)
    {
        fPosX = 0.0f;
    }
    else if (fPosX > fMax)
    {
        fPosX = fMax;
    }
    float fPosZ = fZ + _fTerrainCenterZ;
    if (fPosZ < 0.0f)
    {
        fPosZ = 0.0f;
    }
    else if (fPosZ > fMax)
    {
        fPosZ = fMax;
    }

    const float fGridX = fPosX * _fPointsPerMeter;
    const float fGridZ = fPosZ * _fPointsPerMeter;
    const int iX = (int)fGridX;
    const int iZ = (int)fGridZ;
    const float fFracX = fGridX - (float)iX;
    const float fFracZ = fGridZ - (float)iZ;
    const float fM = _fMetersPerPoint;

    PblVector3 vNormal;
    if (iZ & 1)
    {
        if (1.0f - fFracZ > fFracX)
        {
            vNormal = TriangleNormal(PblVector3(0.0f, GetHeight(iX, iZ), 0.0f),
                                     PblVector3(fM, GetHeight(iX + 1, iZ), 0.0f),
                                     PblVector3(0.0f, GetHeight(iX, iZ + 1), fM));
        }
        else
        {
            vNormal = TriangleNormal(PblVector3(fM, GetHeight(iX + 1, iZ + 1), fM),
                                     PblVector3(0.0f, GetHeight(iX, iZ + 1), fM),
                                     PblVector3(fM, GetHeight(iX + 1, iZ), 0.0f));
        }
    }
    else if (fFracZ > fFracX)
    {
        vNormal = TriangleNormal(PblVector3(0.0f, GetHeight(iX, iZ + 1), fM),
                                 PblVector3(0.0f, GetHeight(iX, iZ), 0.0f),
                                 PblVector3(fM, GetHeight(iX + 1, iZ + 1), fM));
    }
    else
    {
        vNormal = TriangleNormal(PblVector3(fM, GetHeight(iX + 1, iZ), 0.0f),
                                 PblVector3(fM, GetHeight(iX + 1, iZ + 1), fM),
                                 PblVector3(0.0f, GetHeight(iX, iZ), 0.0f));
    }

    PblVector3 vResult;
    D3DXVec3Normalize(&vResult, &vNormal);
    return vResult;
}

// FUNCTION: SWBF2 0x006c1ee0
float RedTerrain::GetLight(float fX, float fZ)
{
    const int iX = (int)((fX + _fTerrainCenterX) * _fPointsPerMeter + 0.5f);
    const int iZ = (int)((fZ + _fTerrainCenterZ) * _fPointsPerMeter + 0.5f);
    return (float)_puiLuminosity[iX * _iPointsPerDimension + iZ] * (1.0f / 255.0f);
}

// FUNCTION: SWBF2 0x006c1f50
bool RedTerrain::PointInHole(const PblVector3& vPos)
{
    if (!_bEnable)
    {
        return false;
    }

    static PblVector3 vLastPos(FLT_MAX, FLT_MAX, FLT_MAX);
    static bool bLastInHole;
    if (vPos.x == vLastPos.x && vPos.y == vLastPos.y && vPos.z == vLastPos.z)
    {
        return bLastInHole;
    }
    vLastPos = vPos;
    bLastInHole = false;

    const int iX = (int)((vPos.x + _fTerrainCenterX) * _fPointsPerMeter);
    const int iZ = (int)((vPos.z + _fTerrainCenterZ) * _fPointsPerMeter);
    const int iPatch = (iX >> _iHeightPatchPointsPerDimensionLog2) +
                       ((iZ >> _iHeightPatchPointsPerDimensionLog2) << _iHeightPatchesPerDimensionLog2);
    const unsigned char uiHoles = _pHeightExpand[iPatch].uiFlags & 0xf;
    if (!uiHoles)
    {
        return false;
    }
    const int iQuadrant = ((iX >> (_iHeightPatchPointsPerDimensionLog2 - 1)) & 1) |
                          ((iZ >> (_iHeightPatchPointsPerDimensionLog2 - 2)) & 2);
    if (!(uiHoles & (1 << iQuadrant)))
    {
        return false;
    }

    if (_bTreeHoles)
    {
        HoleCallbackData data;
        data.vPoint = vPos;
        data.fRadius = 0.01f;
        data.pbInHole = &bLastInHole;
        CollisionManager::GetTreeGrid()->CallbackOnSphere(vPos, 0.01f, RedTerrain::PointInHoleCallback, &data);
        return bLastInHole;
    }

    for (PblList<Hole>::Itor it = _HoleList.Begin(); it; ++it)
    {
        Hole* pHole = it;
        if (PointInSphere(vPos, pHole->GetCollisionCenter(), pHole->GetCollisionRadius()) &&
            PointInVolume(vPos, pHole->aPlanes, pHole->iNumPlanes))
        {
            bLastInHole = true;
            return true;
        }
    }
    return bLastInHole;
}

// FUNCTION: SWBF2 0x006c2230
bool PointInVolume(const PblVector3& vPoint, const PblVector4* aPlanes, int iNumPlanes)
{
    for (int i = 0; i < iNumPlanes; ++i)
    {
        if (aPlanes[i].y * vPoint.y + aPlanes[i].x * vPoint.x + aPlanes[i].z * vPoint.z + aPlanes[i].w > 0.0f)
        {
            return false;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x006c22e0
float RedTerrain::RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fA, float fB)
{
    if (!_bEnable)
    {
        return 1.0f;
    }

    if (fabsf(vStart.x - vEnd.x) < EPSILON && fabsf(vStart.z - vEnd.z) < EPSILON)
    {
        if (vEnd.y >= vStart.y)
        {
            return 1.0f;
        }
        const float fHeight = GetInterpolatedHeight(vStart.x, vStart.z);
        if (fHeight > vStart.y)
        {
            return 1.0f;
        }
        const float fT = (fHeight - vStart.y) / (vEnd.y - vStart.y);
        if (fT > 1.0f)
        {
            return 1.0f;
        }
        if (PointInHole(PblVector3(vStart.x, fHeight, vStart.z)))
        {
            return 1.0f;
        }
        return fT;
    }

    bool bAbove = true;
    if (GetInterpolatedHeight(vStart.x, vStart.z) > vStart.y)
    {
        bAbove = false;
    }
    return _RayHit(vStart, vEnd, fA, fB, bAbove);
}

// FUNCTION: SWBF2 0x006c2460
void RedTerrain::Read(PblFileChunk& chunk)
{
    const int iHeap = GameMemory::RunTimeHeap;
    DecalTile* pDecalTile = 0;
    HeightExpand* pHeightExpand = 0;
    HeightMinMax* pHeightMinMax = 0;
    unsigned char* pHeight = 0;
    char acName[256];

    for (int i = 0; i < 16; ++i)
    {
        _uiLayerTextureOrientation[i] = 0;
        _uiLayerTextureRotation[i] = 0;
    }

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        switch (child._iId)
        {
        case 0x4F464E49:
        {
            _bEnable = true;
            _fMetersPerPoint = pFile->ReadFloat();
            _fHeightScale = pFile->ReadFloat();
            _fHeightMin = pFile->ReadFloat();
            _fHeightMax = pFile->ReadFloat();
            _iPointsPerDimension = pFile->Read16();
            _iPatchPointsPerDimension = pFile->Read16();
            _iHeightPatchPointsPerDimension = pFile->Read16();
            _iNumLayers = pFile->Read16();
            _iNumDecalTextures = pFile->Read16();
            _iNumDecalTiles = pFile->Read16();

            _iPatchesPerDimension = _iPointsPerDimension / _iPatchPointsPerDimension;
            _iHeightPatchesPerDimension = _iPointsPerDimension / _iHeightPatchPointsPerDimension;
            _iNumPoints = _iPointsPerDimension * _iPointsPerDimension;
            _iNumPatches = _iPatchesPerDimension * _iPatchesPerDimension;
            _iNumHeightPatches = _iHeightPatchesPerDimension * _iHeightPatchesPerDimension;
            _iNumPatchPoints = (_iPatchPointsPerDimension + 1) * (_iPatchPointsPerDimension + 1);
            _iNumHeightPatchPoints = _iHeightPatchPointsPerDimension * _iHeightPatchPointsPerDimension;

            _iPointsPerDimensionLog2 = Log2(_iPointsPerDimension);
            _iPatchesPerDimensionLog2 = Log2(_iPatchesPerDimension);
            _iHeightPatchesPerDimensionLog2 = Log2(_iHeightPatchesPerDimension);
            _iPatchPointsPerDimensionLog2 = Log2(_iPatchPointsPerDimension);
            _iHeightPatchPointsPerDimensionLog2 = Log2(_iHeightPatchPointsPerDimension);
            _iNumPointsLog2 = Log2(_iNumPoints);
            _iNumPatchesLog2 = Log2(_iNumPatches);
            _iNumHeightPatchesLog2 = Log2(_iNumHeightPatches);
            _iNumHeightPatchPointsLog2 = Log2(_iNumHeightPatchPoints);

            _iPointsPerDimensionMask = _iPointsPerDimension - 1;
            _iPatchesPerDimensionMask = _iPatchesPerDimension - 1;
            _iHeightPatchesPerDimensionMask = _iHeightPatchesPerDimension - 1;
            _iPatchPointsPerDimensionMask = _iPatchPointsPerDimension - 1;
            _iHeightPatchPointsPerDimensionMask = _iHeightPatchPointsPerDimension - 1;
            _iNumPointsMask = _iNumPoints - 1;
            _iNumPatchesMask = _iNumPatches - 1;
            _iNumHeightPatchesMask = _iNumHeightPatches - 1;
            _iNumHeightPatchPointsMask = _iNumHeightPatchPoints - 1;

            _fPointsPerMeter = 1.0f / _fMetersPerPoint;
            _fTerrainCenterX = (float)_iPointsPerDimension * _fMetersPerPoint * 0.5f;
            _fTerrainCenterZ = _fTerrainCenterX - _fMetersPerPoint;

            const int iOldHeap = RedSetCurrentHeap(iHeap);
            pHeightExpand = new HeightExpand[_iNumHeightPatches];
            mTerrainBytesUsed += _iNumHeightPatches * sizeof(HeightExpand);
            pHeightMinMax = new HeightMinMax[_iNumHeightPatches];
            mTerrainBytesUsed += _iNumHeightPatches * sizeof(HeightMinMax);
            pHeight = new unsigned char[_iPointsPerDimension * _iPointsPerDimension];
            mTerrainBytesUsed += _iPointsPerDimension * _iPointsPerDimension;
            unsigned int* puiScorch =
                new unsigned int[_iPointsPerDimension * _iPointsPerDimension / 4u];
            mTerrainBytesUsed += _iPointsPerDimension * _iPointsPerDimension * sizeof(unsigned int) / 4;
            if (_iNumDecalTiles)
            {
                pDecalTile = new DecalTile[_iNumDecalTiles];
                mTerrainBytesUsed += _iNumDecalTiles * sizeof(DecalTile);
            }
            else
            {
                pDecalTile = 0;
            }
            RedSetCurrentHeap(iOldHeap);

            _pHeightExpand = pHeightExpand;
            _pDecalTile = pDecalTile;
            _pHeightMinMax = pHeightMinMax;
            _puiHeight = pHeight;
            _puiScorch = puiScorch;
            for (int i = 0; i < (int)(_iPointsPerDimension * _iPointsPerDimension / 4u); ++i)
            {
                puiScorch[i] = 0;
            }
            break;
        }

        case 0x50584548:
            for (int i = 0; i < _iNumHeightPatches; ++i)
            {
                pHeightExpand[i].uiShift = pFile->Read8();
                pHeightExpand[i].uiFlags = pFile->Read8();
                pHeightExpand[i].iOffset = pFile->Read16();
                pHeightMinMax[i].iMin = pFile->Read16();
                pHeightMinMax[i].iMax = pFile->Read16();
            }
            break;

        case 0x38544748:
        {
            unsigned char* p = pHeight;
            unsigned char* const pEnd = pHeight + _iNumPoints;
            while (p < pEnd)
            {
                const unsigned char uiCount = pFile->Read8();
                int iRun;
                if (uiCount & 0x80)
                {
                    const unsigned char uiValue = pFile->Read8();
                    iRun = uiCount - 127;
                    memset(p, uiValue, iRun);
                }
                else
                {
                    iRun = uiCount + 1;
                    pFile->ReadData(p, iRun);
                }
                p += iRun;
            }
            break;
        }

        case 0x494D554C:
        {
            const int iSize = child._iSize;
            const int iOldHeap = RedSetCurrentHeap(iHeap);
            unsigned char* pLum = new unsigned char[iSize];
            mTerrainBytesUsed += iSize;
            RedSetCurrentHeap(iOldHeap);
            _puiLuminosity = pLum;
            pFile->ReadData(pLum, iSize);
            break;
        }

        case 0x474C4F46:
        {
            const int iSize = pFile->Read32();
            const int iOldHeap = RedSetCurrentHeap(iHeap);
            _Foliage = new unsigned char[iSize];
            RedSetCurrentHeap(iOldHeap);
            mTerrainBytesUsed += iSize;
            pFile->ReadData(_Foliage, iSize);
            break;
        }

        case 0x4C414344:
            for (int i = 0; i < _iNumDecalTiles; ++i)
            {
                DecalTile& tile = pDecalTile[i];
                tile.uiGridX = pFile->Read8();
                tile.uiGridZ = pFile->Read8();
                tile.uiTextureIndex = pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                pFile->Read8();
                tile.uiLLv = pFile->Read16();
                tile.uiLLu = pFile->Read16();
                tile.uiLRv = pFile->Read16();
                tile.uiLRu = pFile->Read16();
                tile.uiULv = pFile->Read16();
                tile.uiULu = pFile->Read16();
                tile.uiURv = pFile->Read16();
                tile.uiURu = pFile->Read16();
            }
            break;

        case 0x4C414353:
            for (int i = 0; i < _iNumLayers; ++i)
            {
                _fLayerMetersPerTexture[i] = pFile->ReadFloat();
            }
            break;

        case 0x52544157:
            RedWater::Read(child);
            break;

        case 0x5845544C:
            for (int i = 0; i < _iNumLayers; ++i)
            {
                child.ReadString(acName, sizeof(acName));
                _uiLayerTextureHash[i] = PblHash(acName);
            }
            break;

        case 0x58455444:
            for (int i = 0; i < _iNumLayers; ++i)
            {
                child.ReadString(acName, sizeof(acName));
                _uiDecalTextureHash[i] = PblHash(acName);
            }
            break;

        case 0x584C5444:
            child.ReadString(acName, sizeof(acName));
            _uiDetailTextureHash = PblHash(acName);
            break;

        case 0x53484350:
            _ReadPatchInfo(child);
            break;

        case 0x53495841:
            for (int i = 0; i < _iNumLayers; ++i)
            {
                _uiLayerTextureOrientation[i] = pFile->Read8();
            }
            break;

        case 0x53545543:
            while (child.HasMoreChildren())
            {
                PblFileChunk cut = child.ReadNextChild();
                if (cut._iId == 0x4F464E49)
                {
                    _bTreeHoles = cut._pFile->Read32() > 10;
                }
                else if (cut._iId == 0x52545543)
                {
                    int iOldHeap = RedSetCurrentHeap(iHeap);
                    Hole* pHole = new Hole;
                    mTerrainBytesUsed += sizeof(Hole);
                    RedSetCurrentHeap(iOldHeap);

                    while (cut.HasMoreChildren())
                    {
                        PblFileChunk part = cut.ReadNextChild();
                        PblFile* pPartFile = part._pFile;
                        if (part._iId == 0x4F464E49)
                        {
                            pHole->iNumPlanes = pPartFile->Read32();
                            const float fRadius = pPartFile->ReadFloat();
                            PblVector3 vCenter;
                            vCenter.x = pPartFile->ReadFloat();
                            vCenter.y = pPartFile->ReadFloat();
                            vCenter.z = pPartFile->ReadFloat();
                            pHole->SetCollisionCenter(vCenter);
                            pHole->SetCollisionRadius(fRadius);
                            pHole->Set2DCollisionRadius(fRadius);

                            iOldHeap = RedSetCurrentHeap(iHeap);
                            pHole->aPlanes = new PblVector4[pHole->iNumPlanes];
                            mTerrainBytesUsed += pHole->iNumPlanes * sizeof(PblVector4);
                            RedSetCurrentHeap(iOldHeap);
                        }
                        else if (part._iId == 0x534E4C50)
                        {
                            for (int i = 0; i < pHole->iNumPlanes; ++i)
                            {
                                pHole->aPlanes[i].x = pPartFile->ReadFloat();
                                pHole->aPlanes[i].y = pPartFile->ReadFloat();
                                pHole->aPlanes[i].z = pPartFile->ReadFloat();
                                pHole->aPlanes[i].w = pPartFile->ReadFloat();
                            }
                        }
                    }

                    _HoleList.Add(&pHole->node, pHole);
                    if (_bTreeHoles)
                    {
                        CollisionManager::GetTreeGrid()->Add(
                            pHole, pHole->GetCollisionCenter(), pHole->GetCollisionRadius() * 2.0f);
                    }
                }
            }
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006c3130
void RedTerrain::SetForceHiDetail(bool bForce)
{
    sForceHiDetail = bForce;
}

// FUNCTION: SWBF2 0x006c3140
float RedTerrain::_RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fTolerance)
{
    float fBase = 0.0f;
    float fStep = 0.5f;
    PblVector3 vFrom = vStart;
    PblVector3 vTo = vEnd;
    const float fTolerance2 = fTolerance * fTolerance;

    while ((vFrom.y - vTo.y) * (vFrom.y - vTo.y) + (vFrom.x - vTo.x) * (vFrom.x - vTo.x) +
               (vFrom.z - vTo.z) * (vFrom.z - vTo.z) > fTolerance2)
    {
        const PblVector3 vMid((vTo.x + vFrom.x) * 0.5f, (vTo.y + vFrom.y) * 0.5f, (vTo.z + vFrom.z) * 0.5f);
        const float fAbove = GetInterpolatedHeight(vMid.x, vMid.z) - vMid.y;
        if (fAbove > fTolerance)
        {
            vTo = vMid;
        }
        else if (-fTolerance > fAbove)
        {
            vFrom = vMid;
            fBase += fStep;
        }
        else
        {
            break;
        }
        fStep *= 0.5f;
    }
    return fStep + fBase;
}

// FUNCTION: SWBF2 0x006c32f0
float RedTerrain::_RayHit(const PblVector3& vStart, const PblVector3& vEnd, float fA, float fB, bool& bAbove)
{
    if ((vStart.x - vEnd.x) * (vStart.x - vEnd.x) + (vStart.z - vEnd.z) * (vStart.z - vEnd.z) > fB * fB)
    {
        const PblVector3 vMid((vEnd.x + vStart.x) * 0.5f, (vStart.y + vEnd.y) * 0.5f, (vEnd.z + vStart.z) * 0.5f);
        const float fFirst = _RayHit(vStart, vMid, fA, fB, bAbove);
        if (1.0f > fFirst)
        {
            return fFirst * 0.5f;
        }
        return _RayHit(vMid, vEnd, fA, fB, bAbove) * 0.5f + 0.5f;
    }

    if (GetInterpolatedHeight(vEnd.x, vEnd.z) > vEnd.y)
    {
        if (bAbove)
        {
            const float fT = _RayHit(vStart, vEnd, fA);
            if (!PointInHole(Interpolate(vStart, vEnd, fT)))
            {
                return fT;
            }
        }
        bAbove = false;
        return 1.0f;
    }
    bAbove = true;
    return 1.0f;
}

// FUNCTION: SWBF2 0x006c3110
void RedTerrain::Reset()
{
    _bEnable = false;
    _HoleList.RemoveAll();
    mTerrainBytesUsed = 0;
}

// FUNCTION: SWBF2 0x006c1650
template void PblList<RedTerrain::Hole>::RemoveAll();

// FUNCTION: SWBF2 0x004108f0
int RedTerrain::GetHeightUnscaled(int iX, int iZ)
{
    const int iPatch =
        ((iX & RedTerrain::_iPointsPerDimensionMask) >> RedTerrain::_iHeightPatchPointsPerDimensionLog2) +
        (((iZ & RedTerrain::_iPointsPerDimensionMask) >> RedTerrain::_iHeightPatchPointsPerDimensionLog2)
         << RedTerrain::_iHeightPatchesPerDimensionLog2);
    const RedTerrain::HeightExpand& expand = RedTerrain::_pHeightExpand[iPatch];
    const unsigned char* puiPatch =
        RedTerrain::_puiHeight + (iPatch << RedTerrain::_iNumHeightPatchPointsLog2);
    const int iPoint = (iX & RedTerrain::_iHeightPatchPointsPerDimensionMask) +
                       ((iZ & RedTerrain::_iHeightPatchPointsPerDimensionMask)
                        << RedTerrain::_iHeightPatchPointsPerDimensionLog2);
    return (puiPatch[iPoint] << expand.uiShift) + expand.iOffset;
}

// FUNCTION: SWBF2 0x006c21b0
bool RedTerrain::PointInHoleCallback(const TreeGridObject* pObject, TreeGridCallbackData* pData, float& fDist)
{
    const RedTerrain::Hole* pHole = static_cast<const RedTerrain::Hole*>(pObject);
    HoleCallbackData* pHoleData = static_cast<HoleCallbackData*>(pData);
    if (PointInVolume(pHoleData->vPoint, pHole->aPlanes, pHole->iNumPlanes))
    {
        *pHoleData->pbInHole = true;
        return false;
    }
    return true;
}

// GLOBAL: SWBF2 0x007dedb4
PblList<RedTerrain::Hole> RedTerrain::_HoleList;
// GLOBAL: SWBF2 0x0093eab4
bool RedTerrain::_bEnable;
// GLOBAL: SWBF2 0x0093eacc
int RedTerrain::mTerrainBytesUsed;
