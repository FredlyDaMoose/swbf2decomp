#include <math.h>
#include <string.h>

#include "TreeGrid.h"
#include "CollisionObject.h"
#include "CollisionUtil.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/RedStack.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

const int MAX_TREEGRID_STACKS = 15;

static const float EPSILON = 0.0001f;

// GLOBAL: SWBF2 0x01f9a068
// SYNTHETIC: SWBF2 0x00406b40
// `dynamic initializer for 'TreeGridStack::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769640
// `dynamic atexit destructor for 'TreeGridStack::sMemoryPool''
MemoryPool TreeGridStack::sMemoryPool("TreeGridStack", sizeof(TreeGridStack));

void* TreeGridStack::operator new(unsigned int uiSize)
{
    return sMemoryPool.Allocate(uiSize);
}

void TreeGridStack::operator delete(void* pBlock)
{
    sMemoryPool.Free(pBlock);
}

// FUNCTION: SWBF2 0x0065ae10
TreeGrid::TreeGrid(const PblVector3& rMin, const PblVector3& rMax, float fMinSize)
{
    float fSizeX = rMax.x - rMin.x;
    float fSizeZ = rMax.z - rMin.z;
    mMaxSize = fSizeX > fSizeZ ? fSizeX : fSizeZ;
    mMinSize = fMinSize;
    mOffset = rMin.z > rMin.x ? rMin.x : rMin.z;
    mDepth = 1;
    mNumCells = 1;
    mMaxGridSize = 1;
    mSizePerMeter = 1.0f / mMaxSize;

    for (float fSize = mMaxSize * 0.5f; fSize > mMinSize; fSize *= 0.5f)
    {
        mMaxGridSize *= 2;
        ++mDepth;
        mNumCells += mMaxGridSize * mMaxGridSize;
    }

    mGrid = new TreeGridStack*[mNumCells];
    memset(mGrid, 0, mNumCells * sizeof(TreeGridStack*));

    mLayerData = new TreeGridLayerData[mDepth];
    mLayerData[0].mSize = mMaxSize;
    mLayerData[0].mGridSize = 1;
    mLayerData[0].mSizePerMeter = 1.0f / mMaxSize;
    mLayerData[0].mStartIndex = 0;
    for (int i = 1; i < mDepth; ++i)
    {
        mLayerData[i].mSize = mLayerData[i - 1].mSize * 0.5f;
        mLayerData[i].mSizePerMeter = 1.0f / mLayerData[i].mSize;
        mLayerData[i].mGridSize = mLayerData[i - 1].mGridSize * 2;
        mLayerData[i].mStartIndex = mLayerData[i - 1].mStartIndex
                                  + mLayerData[i - 1].mGridSize * mLayerData[i - 1].mGridSize;
    }
}

// FUNCTION: SWBF2 0x0065afa0
TreeGrid::~TreeGrid()
{
    delete[] mGrid;
    delete[] mLayerData;
}

// FUNCTION: SWBF2 0x0065afc0
void TreeGrid::Add(TreeGridObject* pObj, const PblVector3& rPos, float fSize)
{
    Add(pObj, rPos, fSize, fSize);
}

// FUNCTION: SWBF2 0x0065afe0
void TreeGrid::Add(TreeGridObject* pObj, const PblVector3& rPos, float fSize, float f2dSize)
{
    int iCell = 0;
    int iLevel = 0;
    float fX = (rPos.x - f2dSize * 0.5f - mOffset) * mSizePerMeter;
    float fZ = (rPos.z - f2dSize * 0.5f - mOffset) * mSizePerMeter;
    if (fX > 0.0f && fX < 1.0f && fZ > 0.0f && fZ < 1.0f)
    {
        for (float fCellSize = mMaxSize * 0.5f; fCellSize > f2dSize && fCellSize > mMinSize;
             fCellSize *= 0.5f)
        {
            ++iLevel;
        }

        const TreeGridLayerData& rLayer = mLayerData[iLevel];
        iCell = rLayer.mStartIndex + (int)(rLayer.mGridSize * fX)
              + rLayer.mGridSize * (int)(rLayer.mGridSize * fZ);
    }

    if (!mGrid[iCell])
    {
        mGrid[iCell] = new TreeGridStack;
    }
    mGrid[iCell] = mGrid[iCell]->AddObject(pObj, rPos, fSize);
    pObj->mTreeGridCellIndex = iCell;
}

// FUNCTION: SWBF2 0x0065b100
void TreeGrid::Remove(TreeGridObject* pObj)
{
    int iCell = pObj->mTreeGridCellIndex;
    mGrid[iCell] = mGrid[iCell]->RemoveObject(pObj);
    pObj->mTreeGridCellIndex = -1;
}

// FUNCTION: SWBF2 0x0065b140
void TreeGrid::Update(TreeGridObject* pObj, const PblVector3& rPos, float fSize)
{
    Update(pObj, rPos, fSize, fSize);
}

// FUNCTION: SWBF2 0x0065b160
void TreeGrid::Update(TreeGridObject* pObj, const PblVector3& rPos, float fSize, float f2dSize)
{
    PblVector3 vPos = rPos;
    Remove(pObj);
    Add(pObj, vPos, fSize, f2dSize);
}

// FUNCTION: SWBF2 0x0065adb0
bool TreeGrid::ProcessCell(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                           float& fDist)
{
    for (TreeGridStack* pStack = mGrid[iCell]; pStack; pStack = pStack->mNext)
    {
        for (int i = 0; i < pStack->mStackSize; ++i)
        {
            if (!pCallback(pStack->mObject[i], pData, fDist))
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065b1b0
bool TreeGrid::ProcessCellRect(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                               float& fDist, const PblVector3& rMin, const PblVector3& rMax)
{
    RedStack stack;
    TreeGridObject** ppHits = (TreeGridObject**)stack.Alloc(16 * sizeof(TreeGridObject*));

    for (TreeGridStack* pStack = mGrid[iCell]; pStack; pStack = pStack->mNext)
    {
        int iNumHits = 0;
        for (int i = 0; i < pStack->mStackSize; ++i)
        {
            if ((pStack->mData[i][0] & pData->mData[0])
                && CollisionUtil::CollideSphereAABB(pStack->mSphere[i].GetCenter(),
                                                    pStack->mSphere[i].GetRadius(), rMin, rMax))
            {
                ppHits[iNumHits++] = pStack->mObject[i];
            }
        }
        for (int i = 0; i < iNumHits; ++i)
        {
            if (!pCallback(ppHits[i], pData, fDist))
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065b2d0
bool TreeGrid::ProcessCellRay(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                              float& fDist, const PblVector3& rFrom, const PblVector3& rDir,
                              float fLength)
{
    for (TreeGridStack* pStack = mGrid[iCell]; pStack; pStack = pStack->mNext)
    {
        for (int i = 0; i < pStack->mStackSize; ++i)
        {
            if ((pStack->mData[i][0] & pData->mData[0])
                && CollisionUtil::CollideSphereRay(pStack->mSphere[i].GetCenter(),
                                                   pStack->mSphere[i].GetRadius(),
                                                   rFrom, rDir, fLength)
                && !pCallback(pStack->mObject[i], pData, fDist))
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065b380
bool TreeGrid::ProcessCellSphere(int iCell, TreeGridCallback pCallback, TreeGridCallbackData* pData,
                                 float& fDist, const PblVector3& rCenter, float fRadius)
{
    RedStack stack;
    TreeGridObject** ppHits = (TreeGridObject**)stack.Alloc(16 * sizeof(TreeGridObject*));

    for (TreeGridStack* pStack = mGrid[iCell]; pStack; pStack = pStack->mNext)
    {
        int iNumHits = 0;
        for (int i = 0; i < pStack->mStackSize; ++i)
        {
            if ((pStack->mData[i][0] & pData->mData[0])
                && CollisionUtil::CollideSpheres2(pStack->mSphere[i].GetCenter(),
                                                  pStack->mSphere[i].GetRadius(), rCenter, fRadius))
            {
                ppHits[iNumHits++] = pStack->mObject[i];
            }
        }
        for (int i = 0; i < iNumHits; ++i)
        {
            if (!pCallback(ppHits[i], pData, fDist))
            {
                return false;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0065b4a0
void TreeGrid::CallbackOnRect(const PblVector3& rMin, const PblVector3& rMax,
                              TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    float fDist = 1e11f;
    for (int i = 0; i < 5; ++i)
    {
        if (!ProcessCellRect(i, pCallback, pData, fDist, rMin, rMax))
        {
            return;
        }
    }

    float fMaxX = (rMax.x - mOffset) * mSizePerMeter;
    if (fMaxX < 0.0f)
    {
        return;
    }
    float fMaxZ = (rMax.z - mOffset) * mSizePerMeter;
    if (fMaxZ < 0.0f)
    {
        return;
    }
    float fMinX = (rMin.x - mOffset) * mSizePerMeter;
    if (fMinX >= 1.0f)
    {
        return;
    }
    float fMinZ = (rMin.z - mOffset) * mSizePerMeter;
    if (fMinZ >= 1.0f)
    {
        return;
    }

    int iMinX = fMinX > 0.0f ? (int)(mMaxGridSize * fMinX) : 0;
    int iMinZ = fMinZ > 0.0f ? (int)(mMaxGridSize * fMinZ) : 0;
    int iGridSize = mMaxGridSize;
    int iMaxX = fMaxX < 1.0f ? (int)(iGridSize * fMaxX) : iGridSize - 1;
    int iMaxZ = fMaxZ < 1.0f ? (int)(iGridSize * fMaxZ) : iGridSize - 1;
    int iStartIndex = mNumCells - iGridSize * iGridSize;

    while (iGridSize > 2)
    {
        int iX0 = iMinX - 1 > 0 ? iMinX - 1 : 0;
        int iZ0 = iMinZ - 1 > 0 ? iMinZ - 1 : 0;
        int iCell = iStartIndex + iX0 + iGridSize * iZ0;
        for (int z = iZ0; z <= iMaxZ; ++z)
        {
            for (int x = iX0; x <= iMaxX; ++x, ++iCell)
            {
                if (!ProcessCellRect(iCell, pCallback, pData, fDist, rMin, rMax))
                {
                    return;
                }
            }
            iCell += iGridSize - (iMaxX - iX0 + 1);
        }

        iMinX >>= 1;
        iMinZ >>= 1;
        iMaxX >>= 1;
        iMaxZ >>= 1;
        iGridSize >>= 1;
        iStartIndex -= iGridSize * iGridSize;
    }
}

// FUNCTION: SWBF2 0x0065b6a0
void TreeGrid::CallbackOnLazyRay(const PblVector3& rFrom, const PblVector3& rTo,
                                 TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    float fMinX, fMaxX, fMinZ, fMaxZ;
    if (rTo.x > rFrom.x)
    {
        fMinX = rFrom.x;
        fMaxX = rTo.x;
    }
    else
    {
        fMinX = rTo.x;
        fMaxX = rFrom.x;
    }
    if (rTo.z > rFrom.z)
    {
        fMinZ = rFrom.z;
        fMaxZ = rTo.z;
    }
    else
    {
        fMinZ = rTo.z;
        fMaxZ = rFrom.z;
    }

    PblVector3 vDir(rTo.x - rFrom.x, rTo.y - rFrom.y, rTo.z - rFrom.z);
    float fLength = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    PblVector3 vNormal;
    D3DXVec3Normalize(&vNormal, &vDir);
    vDir = vNormal;

    float fDist = 1e11f;
    for (int i = 0; i < 5; ++i)
    {
        if (!ProcessCellRay(i, pCallback, pData, fDist, rFrom, vDir, fLength))
        {
            return;
        }
    }

    fMaxX = (fMaxX - mOffset) * mSizePerMeter;
    if (fMaxX < 0.0f)
    {
        return;
    }
    fMaxZ = (fMaxZ - mOffset) * mSizePerMeter;
    if (fMaxZ < 0.0f)
    {
        return;
    }
    fMinX = (fMinX - mOffset) * mSizePerMeter;
    if (fMinX >= 1.0f)
    {
        return;
    }
    fMinZ = (fMinZ - mOffset) * mSizePerMeter;
    if (fMinZ >= 1.0f)
    {
        return;
    }

    int iMinX = fMinX > 0.0f ? (int)(mMaxGridSize * fMinX) : 0;
    int iMinZ = fMinZ > 0.0f ? (int)(mMaxGridSize * fMinZ) : 0;
    int iGridSize = mMaxGridSize;
    int iMaxX = fMaxX < 1.0f ? (int)(iGridSize * fMaxX) : iGridSize - 1;
    int iMaxZ = fMaxZ < 1.0f ? (int)(iGridSize * fMaxZ) : iGridSize - 1;
    int iStartIndex = mNumCells - iGridSize * iGridSize;

    while (iGridSize > 2)
    {
        int iX0 = iMinX - 1 > 0 ? iMinX - 1 : 0;
        int iZ0 = iMinZ - 1 > 0 ? iMinZ - 1 : 0;
        int iCell = iStartIndex + iX0 + iGridSize * iZ0;
        for (int z = iZ0; z <= iMaxZ; ++z)
        {
            for (int x = iX0; x <= iMaxX; ++x, ++iCell)
            {
                if (!ProcessCellRay(iCell, pCallback, pData, fDist, rFrom, vDir, fLength))
                {
                    return;
                }
            }
            iCell += iGridSize - (iMaxX - iX0 + 1);
        }

        iMinX >>= 1;
        iMinZ >>= 1;
        iMaxX >>= 1;
        iMaxZ >>= 1;
        iGridSize >>= 1;
        iStartIndex -= iGridSize * iGridSize;
    }
}

// FUNCTION: SWBF2 0x0065b960
void TreeGrid::CallbackOnSphere(const PblVector3& rCenter, float fRadius,
                                TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    float fDist = 1e11f;
    for (int i = 0; i < 5; ++i)
    {
        if (!ProcessCellSphere(i, pCallback, pData, fDist, rCenter, fRadius))
        {
            return;
        }
    }

    float fMaxX = (rCenter.x + fRadius - mOffset) * mSizePerMeter;
    if (fMaxX < 0.0f)
    {
        return;
    }
    float fMaxZ = (rCenter.z + fRadius - mOffset) * mSizePerMeter;
    if (fMaxZ < 0.0f)
    {
        return;
    }
    float fMinX = (rCenter.x - fRadius - mOffset) * mSizePerMeter;
    if (fMinX >= 1.0f)
    {
        return;
    }
    float fMinZ = (rCenter.z - fRadius - mOffset) * mSizePerMeter;
    if (fMinZ >= 1.0f)
    {
        return;
    }

    int iMinX = fMinX > 0.0f ? (int)(mMaxGridSize * fMinX) : 0;
    int iMinZ = fMinZ > 0.0f ? (int)(mMaxGridSize * fMinZ) : 0;
    int iGridSize = mMaxGridSize;
    int iMaxX = fMaxX < 1.0f ? (int)(iGridSize * fMaxX) : iGridSize - 1;
    int iMaxZ = fMaxZ < 1.0f ? (int)(iGridSize * fMaxZ) : iGridSize - 1;
    int iStartIndex = mNumCells - iGridSize * iGridSize;

    while (iGridSize > 2)
    {
        int iX0 = iMinX - 1 > 0 ? iMinX - 1 : 0;
        int iZ0 = iMinZ - 1 > 0 ? iMinZ - 1 : 0;
        int iCell = iStartIndex + iX0 + iGridSize * iZ0;
        for (int z = iZ0; z <= iMaxZ; ++z)
        {
            for (int x = iX0; x <= iMaxX; ++x, ++iCell)
            {
                if (!ProcessCellSphere(iCell, pCallback, pData, fDist, rCenter, fRadius))
                {
                    return;
                }
            }
            iCell += iGridSize - (iMaxX - iX0 + 1);
        }

        iMinX >>= 1;
        iMinZ >>= 1;
        iMaxX >>= 1;
        iMaxZ >>= 1;
        iGridSize >>= 1;
        iStartIndex -= iGridSize * iGridSize;
    }
}

// FUNCTION: SWBF2 0x0065bb70
bool TreeGrid::ProcessNeighborhood(int iX, int iZ, int iGridSize, int iStartIndex, int* piExtents,
                                   TreeGridCallback pCallback, TreeGridCallbackData* pData,
                                   float& fDist, bool bFull)
{
    static int sLastX;
    static int sLastZ;

    if (bFull)
    {
        int iMinX = iX + piExtents[0] - 1 > 0 ? iX + piExtents[0] - 1 : 0;
        int iMinZ = iZ - 1 + piExtents[2] > 0 ? iZ - 1 + piExtents[2] : 0;
        int iMaxX = iX + piExtents[1] < iGridSize - 1 ? iX + piExtents[1] : iGridSize - 1;
        int iMaxZ = iZ + piExtents[3] < iGridSize - 1 ? iZ + piExtents[3] : iGridSize - 1;
        int iCell = iStartIndex + iMinX + iGridSize * iMinZ;
        for (int z = iMinZ; z <= iMaxZ; ++z)
        {
            for (int x = iMinX; x <= iMaxX; ++x, ++iCell)
            {
                if (!ProcessCell(iCell, pCallback, pData, fDist))
                {
                    return false;
                }
            }
            iCell += iGridSize - (iMaxX - iMinX + 1);
        }
    }
    else if (sLastX == iX)
    {

        int z = iZ < sLastZ ? iZ + piExtents[2] - 1 : iZ + piExtents[3];
        if (z >= 0 && z <= iGridSize - 1)
        {
            int iMinX = iX - 1 + piExtents[0] > 0 ? iX - 1 + piExtents[0] : 0;
            int iMaxX = iX + piExtents[1] < iGridSize - 1 ? iX + piExtents[1] : iGridSize - 1;
            int iCell = iStartIndex + iMinX + iGridSize * z;
            for (int x = iMinX; x <= iMaxX; ++x, ++iCell)
            {
                if (!ProcessCell(iCell, pCallback, pData, fDist))
                {
                    return false;
                }
            }
        }
    }
    else
    {

        int x = iX < sLastX ? iX + piExtents[0] - 1 : iX + piExtents[1];
        if (x >= 0 && x <= iGridSize - 1)
        {
            int iMinZ = iZ - 1 + piExtents[2] > 0 ? iZ - 1 + piExtents[2] : 0;
            int iMaxZ = iZ + piExtents[3] < iGridSize - 1 ? iZ + piExtents[3] : iGridSize - 1;
            int iCell = iStartIndex + x + iGridSize * iMinZ;
            for (int z = iMinZ; z <= iMaxZ; ++z, iCell += iGridSize)
            {
                if (!ProcessCell(iCell, pCallback, pData, fDist))
                {
                    return false;
                }
            }
        }
    }

    sLastX = iX;
    sLastZ = iZ;
    return true;
}

// FUNCTION: SWBF2 0x0065bdb0
void TreeGrid::_CallbackOnRay(const PblVector3& rFrom, const PblVector3& rTo, int* piExtents,
                              TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    float fDist = 1e11f;
    if (!ProcessCell(0, pCallback, pData, fDist))
    {
        return;
    }

    float fStartX = rFrom.x;
    float fStartZ = rFrom.z;
    float fEndX = rTo.x;
    float fEndZ = rTo.z;
    if (floorf(fStartX) == fStartX)
    {
        fStartX += EPSILON;
    }
    if (floorf(fStartZ) == fStartZ)
    {
        fStartZ += EPSILON;
    }
    if (floorf(fEndX) == fEndX)
    {
        fEndX += EPSILON;
    }
    if (floorf(fEndZ) == fEndZ)
    {
        fEndZ += EPSILON;
    }

    int iGridSize = mMaxGridSize;
    fStartZ = (fStartZ - mOffset) * mSizePerMeter * iGridSize;
    fStartX = (fStartX - mOffset) * mSizePerMeter * iGridSize;
    fEndX = (fEndX - mOffset) * mSizePerMeter * iGridSize;
    fEndZ = (fEndZ - mOffset) * mSizePerMeter * iGridSize;

    int iX = (int)floorf(fStartX);
    int iZ = (int)floorf(fStartZ);
    int iEndX = (int)floorf(fEndX);
    int iEndZ = (int)floorf(fEndZ);
    int iStepX = 1;
    int iStepZ = 1;
    float fDeltaX = 0.0f;
    float fDeltaZ = 0.0f;
    float fCellSize = 1.0f;
    if (iX != iEndX || iZ != iEndZ)
    {
        fCellSize = mMaxSize / iGridSize;
        iStepX = 2 * (fEndX > fStartX) - 1;
        float fDX = fEndX - fStartX;
        iStepZ = 2 * (fEndZ > fStartZ) - 1;
        float fDZ = fEndZ - fStartZ;
        float fLength = sqrtf(fDX * fDX + fDZ * fDZ);
        if (fDist > fCellSize * fLength)
        {
            fDist = fCellSize * fLength;
        }
        fDeltaX = fLength / fabsf(fDX);
        fDeltaZ = fLength / fabsf(fDZ);
    }

    int iStartIndex = mNumCells - iGridSize * iGridSize;
    if (iGridSize <= 1)
    {
        return;
    }

    do
    {
        int x = iX;
        int z = iZ;
        if (iX == iEndX)
        {
            if (!ProcessNeighborhood(iX, iZ, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                     fDist, true))
            {
                return;
            }
            while (z != iEndZ)
            {
                z += iStepZ;
                if (!ProcessNeighborhood(iX, z, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                         fDist, false))
                {
                    return;
                }
            }
        }
        else if (iZ == iEndZ)
        {
            if (!ProcessNeighborhood(iX, iZ, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                     fDist, true))
            {
                return;
            }
            while (x != iEndX)
            {
                x += iStepX;
                if (!ProcessNeighborhood(x, iZ, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                         fDist, false))
                {
                    return;
                }
            }
        }
        else
        {
            float fNextX = fabsf((fEndX > fStartX ? ceilf(fStartX) : floorf(fStartX)) - fStartX)
                         * fDeltaX;
            float fNextZ = fabsf((fEndZ > fStartZ ? ceilf(fStartZ) : floorf(fStartZ)) - fStartZ)
                         * fDeltaZ;
            if (!ProcessNeighborhood(iX, iZ, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                     fDist, true))
            {
                return;
            }
            while (x != iEndX || z != iEndZ)
            {
                if (fDist <= (fNextX - fDeltaX) * fCellSize && fDist <= (fNextZ - fDeltaZ) * fCellSize)
                {
                    break;
                }
                if (fNextZ > fNextX)
                {
                    x += iStepX;
                    fNextX += fDeltaX;
                }
                else
                {
                    z += iStepZ;
                    fNextZ += fDeltaZ;
                }
                if (!ProcessNeighborhood(x, z, iGridSize, iStartIndex, piExtents, pCallback, pData,
                                         fDist, false))
                {
                    return;
                }
            }
        }

        fCellSize *= 2.0f;
        iX >>= 1;
        iZ >>= 1;
        iEndX >>= 1;
        iEndZ >>= 1;
        iGridSize >>= 1;
        fStartZ *= 0.5f;
        fStartX *= 0.5f;
        fEndX *= 0.5f;
        fEndZ *= 0.5f;
        iStartIndex -= iGridSize * iGridSize;
        if (piExtents[1])
        {
            piExtents[1] = (piExtents[1] + 1) / 2;
        }
        if (piExtents[3])
        {
            piExtents[3] = (piExtents[3] + 1) / 2;
        }
        if (piExtents[0])
        {
            piExtents[0] = (piExtents[0] - 1) / 2;
        }
        if (piExtents[2])
        {
            piExtents[2] = (piExtents[2] - 1) / 2;
        }
    } while (iGridSize > 1);
}

void TreeGrid::CallbackOnRay(const PblVector3& rFrom, const PblVector3& rTo,
                             TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    int aiExtents[4] = { 0, 0, 0, 0 };
    _CallbackOnRay(rFrom, rTo, aiExtents, pCallback, pData);
}

// FUNCTION: SWBF2 0x0065c530
void TreeGrid::CallbackOnFatRay(const PblVector3& rFrom, const PblVector3& rTo, float fRadius,
                                TreeGridCallback pCallback, TreeGridCallbackData* pData)
{
    int aiExtents[4] = { 0, 0, 0, 0 };
    PblVector3 vDir(rTo.x - rFrom.x, rTo.y - rFrom.y, rTo.z - rFrom.z);
    PblVector3 vNormal;
    D3DXVec3Normalize(&vNormal, &vDir);

    PblVector3 vHalf(vNormal.x * fRadius * 0.5f, vNormal.y * fRadius * 0.5f,
                     vNormal.z * fRadius * 0.5f);
    PblVector3 vSide(vHalf.z, 0.0f, -vHalf.x);
    PblVector3 vFrom(rFrom.x + (-vHalf.x - vSide.x), rFrom.y + (-vHalf.y - vSide.y),
                     rFrom.z + (-vHalf.z - vSide.z));
    PblVector3 vTo(rTo.x + (vHalf.x - vSide.x), rTo.y + (vHalf.y - vSide.y),
                   rTo.z + (vHalf.z - vSide.z));

    float fCellSize = mMaxSize / mMaxGridSize;
    int iSideX = (int)(vSide.x * 2.0f / fCellSize);
    if (iSideX < 0)
    {
        aiExtents[0] = iSideX;
    }
    else
    {
        aiExtents[1] = iSideX;
    }
    int iSideZ = (int)(vSide.z * 2.0f / fCellSize);
    if (iSideZ < 0)
    {
        aiExtents[2] = iSideZ;
    }
    else
    {
        aiExtents[3] = iSideZ;
    }

    _CallbackOnRay(vFrom, vTo, aiExtents, pCallback, pData);
}

// FUNCTION: SWBF2 0x0065c6c0
TreeGridStack* TreeGridStack::AddObject(TreeGridObject* pObj, const PblVector3& rPos, float fSize)
{
    TreeGridStack* pStack = this;
    if (mStackSize == MAX_TREEGRID_STACKS)
    {
        pStack = new TreeGridStack;
        pStack->mNext = this;
    }

    pObj->mStackPtr = pStack;
    pObj->mStackIdx = pStack->mStackSize;
    pStack->mObject[pStack->mStackSize] = pObj;
    pStack->mSphere[pStack->mStackSize].SetCenter(rPos);
    pStack->mSphere[pStack->mStackSize].SetRadius(fSize * 0.5f);
    pStack->mData[pStack->mStackSize][0] = pObj->mData[0];
    ++pStack->mStackSize;
    return pStack;
}

// FUNCTION: SWBF2 0x0065c770
TreeGridStack* TreeGridStack::RemoveObject(TreeGridObject* pObj)
{
    TreeGridStack* pStack = pObj->mStackPtr;
    int iIdx = pObj->mStackIdx;
    pObj->mCollisionSphere = pStack->mSphere[iIdx];
    pObj->mData[0] = pStack->mData[iIdx][0];
    pObj->mStackPtr = 0;
    pObj->mStackIdx = -1;

    TreeGridObject* pLast = mObject[mStackSize - 1];
    if (pStack->mObject[iIdx] != pLast)
    {
        pStack->mObject[iIdx] = pLast;
        pStack->mSphere[iIdx] = mSphere[mStackSize - 1];
        pStack->mData[iIdx][0] = mData[mStackSize - 1][0];
        pStack->mObject[iIdx]->mStackPtr = pStack;
        pStack->mObject[iIdx]->mStackIdx = iIdx;
    }

    if (--mStackSize == 0)
    {
        TreeGridStack* pNext = mNext;
        delete this;
        return pNext;
    }
    return this;
}
