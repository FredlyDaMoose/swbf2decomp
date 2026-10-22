#include "CollisionMesh.h"

#include <float.h>
#include <math.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include <string.h>
#include <stdlib.h>

#include "CameraManager.h"
#include "Cloth.h"
#include "GameModel.h"
#include "HUDElement.h"
#include "HUDManager.h"
#include "Music.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Util/ext/basearray.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "GameMemory.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrAnimBankReader.h"
#include "Zephyr/Source/ZephyrAnimJoint.h"
#include "Zephyr/Source/ZephyrCommon.h"
#include "Zephyr/Source/ZephyrFile.h"
#include "Zephyr/Source/ZephyrSkeletonShared.h"
#include "Zephyr/Source/ZephyrSkinReader.h"
#include "Zephyr/Source/ZephyrSkinShared.h"

// GLOBAL: SWBF2 0x01e30374
char sCurrentCollision[32];

unsigned int __cdecl strlcpy(char* pcDest, char const* pcSource, unsigned int uiSize);

struct CollisionSizeData
{
    char mName[32];
    int mSize;
};
// GLOBAL: SWBF2 0x01e303a0
CollisionSizeData mTopCollision[25];
int __cdecl _topCollisionCallback(const void* pA, const void* pB);

// GLOBAL: SWBF2 0x01e30345

unsigned char sCurrentRenderHandleWasNull;
// GLOBAL: SWBF2 0x01e30346

unsigned char sRenderStateReset;
// GLOBAL: SWBF2 0x01e30724

unsigned int sCurrentRenderHandle;

// FUNCTION: SWBF2 0x00462070
void CollisionMesh::Render(const PblMatrix&) const
{
    sRenderStateReset = 0;
    sCurrentRenderHandleWasNull = sCurrentRenderHandle == 0;
    sCurrentRenderHandle = 0;
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x00460f50
CollisionMesh::CollisionMesh()
{
    mNameHash = 0;
    mPoslList = 0;
    mRoot = 0;
    mPoslCount = 0;
    mDrawThisLeaf = 0;
    mMask = 0;
}

// FUNCTION: SWBF2 0x00460fb0
CollisionMesh::~CollisionMesh()
{
    delete[] mRoot;
    delete[] mPoslList;
}

// FUNCTION: SWBF2 0x00461480
PblBox CollisionMesh::GetWorldspaceAABB(const PblMatrix& matrix) const
{
    const CollisionAABB aabb = mAABB;
    PblBox box;
    box._vMin.x = box._vMin.y = box._vMin.z = FLT_MAX;
    box._vMax.x = box._vMax.y = box._vMax.z = -FLT_MAX;
    for (int i = 0; i < 8; ++i)
    {
        PblVector3 vCorner;
        vCorner.x = (i & 4) ? aabb.boxmax.x : aabb.boxmin.x;
        vCorner.y = (i & 2) ? aabb.boxmax.y : aabb.boxmin.y;
        vCorner.z = (i & 1) ? aabb.boxmax.z : aabb.boxmin.z;
        PblVector3 v;
        D3DXVec3TransformCoord(&v, &vCorner, &matrix);
        if (box._vMin.x > v.x)
        {
            box._vMin.x = v.x;
        }
        if (box._vMin.y > v.y)
        {
            box._vMin.y = v.y;
        }
        if (box._vMin.z > v.z)
        {
            box._vMin.z = v.z;
        }
        if (v.x > box._vMax.x)
        {
            box._vMax.x = v.x;
        }
        if (v.y > box._vMax.y)
        {
            box._vMax.y = v.y;
        }
        if (v.z > box._vMax.z)
        {
            box._vMax.z = v.z;
        }
    }
    return box;
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

static const float EPSILON = 0.00001f;

// GLOBAL: SWBF2 0x007e5f08
unsigned int u8tof32_hex[256] = {
    0x00000000, 0x3b808080, 0x3c008080, 0x3c40c0c1, 0x3c808080, 0x3ca0a0a1, 0x3cc0c0c1, 0x3ce0e0e1,
    0x3d008080, 0x3d109091, 0x3d20a0a1, 0x3d30b0b1, 0x3d40c0c1, 0x3d50d0d1, 0x3d60e0e1, 0x3d70f0f1,
    0x3d808080, 0x3d888889, 0x3d909091, 0x3d989899, 0x3da0a0a1, 0x3da8a8a9, 0x3db0b0b1, 0x3db8b8b9,
    0x3dc0c0c1, 0x3dc8c8c9, 0x3dd0d0d1, 0x3dd8d8d9, 0x3de0e0e1, 0x3de8e8e9, 0x3df0f0f1, 0x3df8f8f9,
    0x3e008080, 0x3e048485, 0x3e088889, 0x3e0c8c8d, 0x3e109091, 0x3e149495, 0x3e189899, 0x3e1c9c9d,
    0x3e20a0a1, 0x3e24a4a5, 0x3e28a8a9, 0x3e2cacad, 0x3e30b0b1, 0x3e34b4b5, 0x3e38b8b9, 0x3e3cbcbd,
    0x3e40c0c1, 0x3e44c4c5, 0x3e48c8c9, 0x3e4ccccd, 0x3e50d0d1, 0x3e54d4d5, 0x3e58d8d9, 0x3e5cdcdd,
    0x3e60e0e1, 0x3e64e4e5, 0x3e68e8e9, 0x3e6ceced, 0x3e70f0f1, 0x3e74f4f5, 0x3e78f8f9, 0x3e7cfcfd,
    0x3e808080, 0x3e828283, 0x3e848485, 0x3e868687, 0x3e888889, 0x3e8a8a8b, 0x3e8c8c8d, 0x3e8e8e8f,
    0x3e909091, 0x3e929293, 0x3e949495, 0x3e969697, 0x3e989899, 0x3e9a9a9b, 0x3e9c9c9d, 0x3e9e9e9f,
    0x3ea0a0a1, 0x3ea2a2a3, 0x3ea4a4a5, 0x3ea6a6a7, 0x3ea8a8a9, 0x3eaaaaab, 0x3eacacad, 0x3eaeaeaf,
    0x3eb0b0b1, 0x3eb2b2b3, 0x3eb4b4b5, 0x3eb6b6b7, 0x3eb8b8b9, 0x3ebababb, 0x3ebcbcbd, 0x3ebebebf,
    0x3ec0c0c1, 0x3ec2c2c3, 0x3ec4c4c5, 0x3ec6c6c7, 0x3ec8c8c9, 0x3ecacacb, 0x3ecccccd, 0x3ecececf,
    0x3ed0d0d1, 0x3ed2d2d3, 0x3ed4d4d5, 0x3ed6d6d7, 0x3ed8d8d9, 0x3edadadb, 0x3edcdcdd, 0x3edededf,
    0x3ee0e0e1, 0x3ee2e2e3, 0x3ee4e4e5, 0x3ee6e6e7, 0x3ee8e8e9, 0x3eeaeaeb, 0x3eececed, 0x3eeeeeef,
    0x3ef0f0f1, 0x3ef2f2f3, 0x3ef4f4f5, 0x3ef6f6f7, 0x3ef8f8f9, 0x3efafafb, 0x3efcfcfd, 0x3efefeff,
    0x3f008080, 0x3f018182, 0x3f028283, 0x3f038384, 0x3f048485, 0x3f058586, 0x3f068687, 0x3f078788,
    0x3f088889, 0x3f09898a, 0x3f0a8a8b, 0x3f0b8b8c, 0x3f0c8c8d, 0x3f0d8d8e, 0x3f0e8e8f, 0x3f0f8f90,
    0x3f109091, 0x3f119192, 0x3f129293, 0x3f139394, 0x3f149495, 0x3f159596, 0x3f169697, 0x3f179798,
    0x3f189899, 0x3f19999a, 0x3f1a9a9b, 0x3f1b9b9c, 0x3f1c9c9d, 0x3f1d9d9e, 0x3f1e9e9f, 0x3f1f9fa0,
    0x3f20a0a1, 0x3f21a1a2, 0x3f22a2a3, 0x3f23a3a4, 0x3f24a4a5, 0x3f25a5a6, 0x3f26a6a7, 0x3f27a7a8,
    0x3f28a8a9, 0x3f29a9aa, 0x3f2aaaab, 0x3f2babac, 0x3f2cacad, 0x3f2dadae, 0x3f2eaeaf, 0x3f2fafb0,
    0x3f30b0b1, 0x3f31b1b2, 0x3f32b2b3, 0x3f33b3b4, 0x3f34b4b5, 0x3f35b5b6, 0x3f36b6b7, 0x3f37b7b8,
    0x3f38b8b9, 0x3f39b9ba, 0x3f3ababb, 0x3f3bbbbc, 0x3f3cbcbd, 0x3f3dbdbe, 0x3f3ebebf, 0x3f3fbfc0,
    0x3f40c0c1, 0x3f41c1c2, 0x3f42c2c3, 0x3f43c3c4, 0x3f44c4c5, 0x3f45c5c6, 0x3f46c6c7, 0x3f47c7c8,
    0x3f48c8c9, 0x3f49c9ca, 0x3f4acacb, 0x3f4bcbcc, 0x3f4ccccd, 0x3f4dcdce, 0x3f4ececf, 0x3f4fcfd0,
    0x3f50d0d1, 0x3f51d1d2, 0x3f52d2d3, 0x3f53d3d4, 0x3f54d4d5, 0x3f55d5d6, 0x3f56d6d7, 0x3f57d7d8,
    0x3f58d8d9, 0x3f59d9da, 0x3f5adadb, 0x3f5bdbdc, 0x3f5cdcdd, 0x3f5dddde, 0x3f5ededf, 0x3f5fdfe0,
    0x3f60e0e1, 0x3f61e1e2, 0x3f62e2e3, 0x3f63e3e4, 0x3f64e4e5, 0x3f65e5e6, 0x3f66e6e7, 0x3f67e7e8,
    0x3f68e8e9, 0x3f69e9ea, 0x3f6aeaeb, 0x3f6bebec, 0x3f6ceced, 0x3f6dedee, 0x3f6eeeef, 0x3f6feff0,
    0x3f70f0f1, 0x3f71f1f2, 0x3f72f2f3, 0x3f73f3f4, 0x3f74f4f5, 0x3f75f5f6, 0x3f76f6f7, 0x3f77f7f8,
    0x3f78f8f9, 0x3f79f9fa, 0x3f7afafb, 0x3f7bfbfc, 0x3f7cfcfd, 0x3f7dfdfe, 0x3f7efeff, 0x3f800000,
};
static float* const u8tof32_lut = (float*)u8tof32_hex;

// GLOBAL: SWBF2 0x01e30394
CollisionTreeLeaf* sDrawThisLeaf;
// GLOBAL: SWBF2 0x01eb336c
CollisionAABB sDrawThisAABB;

// FUNCTION: SWBF2 0x004609e0
CollisionAABB::CollisionAABB(const CollisionAABB& parent, const CompressedAABB& compressed)
{
    const PblVector3 dim(parent.boxmax.x - parent.boxmin.x, parent.boxmax.y - parent.boxmin.y,
                         parent.boxmax.z - parent.boxmin.z);
    boxmin.x = parent.boxmin.x + dim.x * u8tof32_lut[compressed.boxmin.x];
    boxmin.y = parent.boxmin.y + dim.y * u8tof32_lut[compressed.boxmin.y];
    boxmin.z = parent.boxmin.z + dim.z * u8tof32_lut[compressed.boxmin.z];
    boxmax.x = parent.boxmin.x + dim.x * u8tof32_lut[compressed.boxmax.x];
    boxmax.y = parent.boxmin.y + dim.y * u8tof32_lut[compressed.boxmax.y];
    boxmax.z = parent.boxmin.z + dim.z * u8tof32_lut[compressed.boxmax.z];
}

// FUNCTION: SWBF2 0x00460b10
int RayConvexPoly_intersect(const PblVector3& vStart, const PblVector3& vDir, const PblVector3* const pVerts, int iNumVerts,
               const unsigned short* const pIndices, float& fT)
{
    const PblVector3 v0 = pVerts[pIndices[0]];
    PblVector3 vPrev = pVerts[pIndices[1]];
    const PblVector3 v2 = pVerts[pIndices[2]];
    const PblVector3 e1(vPrev.x - v0.x, vPrev.y - v0.y, vPrev.z - v0.z);
    const PblVector3 e2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    PblVector3 vCross(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
    PblVector3 vNormal;
    D3DXVec3Normalize(&vNormal, &vCross);

    const float fDenom = vDir.x * vNormal.x + vDir.y * vNormal.y + vDir.z * vNormal.z;
    if (fDenom > -EPSILON)
    {
        return 0;
    }
    const float t = ((v0.x - vStart.x) * vNormal.x + (v0.y - vStart.y) * vNormal.y + (v0.z - vStart.z) * vNormal.z)
                  / fDenom;
    if (0.0f > t || t > fT)
    {
        return 0;
    }
    const PblVector3 p(vStart.x + vDir.x * t, vStart.y + vDir.y * t, vStart.z + vDir.z * t);

    PblVector3 vFrom = v0;
    PblVector3 vTo = vPrev;

    bool bClosing = false;
    for (int i = 2;; ++i)
    {
        const PblVector3 e(vTo.x - vFrom.x, vTo.y - vFrom.y, vTo.z - vFrom.z);
        const PblVector3 d(p.x - vFrom.x, p.y - vFrom.y, p.z - vFrom.z);
        const PblVector3 c(e.y * d.z - e.z * d.y, e.z * d.x - e.x * d.z, e.x * d.y - e.y * d.x);
        if (0.0f > c.x * vNormal.x + c.y * vNormal.y + c.z * vNormal.z)
        {
            return 0;
        }
        if (bClosing)
        {
            break;
        }
        vFrom = vTo;
        bClosing = i >= iNumVerts;
        vTo = bClosing ? v0 : pVerts[pIndices[i]];
    }
    fT = t;
    return 1;
}

// FUNCTION: SWBF2 0x00461800
float BoxDistSq(const PblVector3& vPoint, const CollisionAABB& aabb)
{
    const float dx = vPoint.x - (aabb.boxmin.x + aabb.boxmax.x) * 0.5f;
    const float dy = vPoint.y - (aabb.boxmin.y + aabb.boxmax.y) * 0.5f;
    const float dz = vPoint.z - (aabb.boxmin.z + aabb.boxmax.z) * 0.5f;
    return dx * dx + dy * dy + dz * dz;
}

// FUNCTION: SWBF2 0x004615e0
CollisionAABB* CopyCollisionAABB(CollisionAABB* pThis, const CollisionAABB& src)
{
    pThis->boxmin = src.boxmin;
    pThis->boxmax = src.boxmax;
    return pThis;
}

// FUNCTION: SWBF2 0x00461610
bool CollisionMesh::_RayTestAABB(const PblVector3& vStart, const PblVector3& vDir, float fLength, const CollisionAABB& aabb,
                PblVector3& vCoord)
{
    bool bInside = true;
    float maxT[3] = { -1.0f, -1.0f, -1.0f };
    const float minB[3] = { aabb.boxmin.x, aabb.boxmin.y, aabb.boxmin.z };
    const float maxB[3] = { aabb.boxmax.x, aabb.boxmax.y, aabb.boxmax.z };
    const float origin[3] = { vStart.x, vStart.y, vStart.z };
    const float dir[3] = { vDir.x, vDir.y, vDir.z };
    float coord[3];

    for (int i = 0; i < 3; ++i)
    {
        float fPlane;
        if (origin[i] < minB[i])
        {
            fPlane = minB[i];
        }
        else if (origin[i] > maxB[i])
        {
            fPlane = maxB[i];
        }
        else
        {
            continue;
        }
        bInside = false;
        coord[i] = fPlane;
        if (dir[i] != 0.0f)
        {
            maxT[i] = (fPlane - origin[i]) / dir[i];
        }
    }
    if (bInside)
    {
        vCoord = vStart;
        return true;
    }

    int whichPlane = (maxT[1] > maxT[0]) ? 1 : 0;
    if (maxT[2] > maxT[whichPlane])
    {
        whichPlane = 2;
    }
    const float t = maxT[whichPlane];
    if (t < 0.0f || t > fabsf(fLength / dir[whichPlane]))
    {
        return false;
    }
    for (int i = 0; i < 3; ++i)
    {
        if (i == whichPlane)
        {
            continue;
        }
        coord[i] = dir[i] * t + origin[i];
        if (coord[i] < minB[i] - EPSILON || coord[i] > maxB[i] + EPSILON)
        {
            return false;
        }
    }
    const float dx = coord[0] - origin[0];
    const float dy = coord[1] - origin[1];
    const float dz = coord[2] - origin[2];
    if (dx * dx + dy * dy + dz * dz > fLength * fLength)
    {
        return false;
    }
    vCoord = PblVector3(coord[0], coord[1], coord[2]);
    return true;
}

// FUNCTION: SWBF2 0x00461860
int CollisionMesh::_RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, CollisionTreeNode* pNode, const CollisionAABB& aabb, float& fT, CollisionTreeNode** ppHitLeaf) const
{
    const CollisionMesh* pMesh = (const CollisionMesh*)this;
    if (!pNode)
    {
        return 0;
    }
    PblVector3 vCoord;
    if (!CollisionMesh::_RayTestAABB(vStart, vDir, fT, aabb, vCoord))
    {
        return 0;
    }

    if (!pNode->isleaf)
    {
        CollisionTreeNode* pLChild = pNode + 1;
        CollisionTreeNode* pGChild = (CollisionTreeNode*)pNode->data;
        CollisionAABB lAABB(aabb, pLChild->aabb);
        CollisionAABB gAABB(aabb, pGChild->aabb);
        const float fLDist = BoxDistSq(vStart, lAABB);
        if (BoxDistSq(vStart, gAABB) > fLDist)
        {
            const int iFirst = ((CollisionMesh*)pMesh)->_RayHit(vStart, vDir, fLength, pLChild, lAABB, fT, ppHitLeaf);
            const int iSecond = ((CollisionMesh*)pMesh)->_RayHit(vStart, vDir, fLength, pGChild, gAABB, fT, ppHitLeaf);
            return (iSecond | iFirst) != 0;
        }
        const int iFirst = ((CollisionMesh*)pMesh)->_RayHit(vStart, vDir, fLength, pGChild, gAABB, fT, ppHitLeaf);
        const int iSecond = ((CollisionMesh*)pMesh)->_RayHit(vStart, vDir, fLength, pLChild, lAABB, fT, ppHitLeaf);
        return (iFirst | iSecond) != 0;
    }

    float t = fT;
    if (!RayConvexPoly_intersect(vStart, vDir, pMesh->mPoslList, pNode->numVerts, ((CollisionTreeLeaf*)pNode)->GetIndexPointer(),
                    t))
    {
        return 0;
    }
    if (fT > t)
    {
        fT = t;
        sDrawThisLeaf = (CollisionTreeLeaf*)pNode;
        *ppHitLeaf = pNode;
        sDrawThisAABB = aabb;
    }
    return 1;
}

// FUNCTION: SWBF2 0x00461a30
float CollisionMesh::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, PblVector3* pNormal) const
{
    CollisionTreeNode* pHitLeaf = 0;
    float fT = fLength;
    if (!_RayHit(vStart, vDir, fLength, mRoot, mAABB, fT, &pHitLeaf) || !pHitLeaf)
    {
        return 1.0f;
    }
    if (pNormal)
    {
        const unsigned short* pIndices = ((CollisionTreeLeaf*)pHitLeaf)->GetIndexPointer();
        const PblVector3 v0 = mPoslList[pIndices[0]];
        const PblVector3& v1 = mPoslList[pIndices[1]];
        const PblVector3& v2 = mPoslList[pIndices[2]];
        const PblVector3 e1(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
        const PblVector3 e2(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
        PblVector3 vCross(e1.y * e2.z - e1.z * e2.y, e1.z * e2.x - e1.x * e2.z, e1.x * e2.y - e1.y * e2.x);
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &vCross);
        *pNormal = vNormal;
    }
    return fT / fLength;
}

// FUNCTION: SWBF2 0x00417260
const PblVector3 __cdecl Normalize(const PblVector3& rv)
{
    PblVector3 vResult;
    D3DXVec3Normalize(&vResult, &rv);
    return vResult;
}

// FUNCTION: SWBF2 0x00461b70
bool CollisionMesh::RayTest(const PblVector3& vStart, const PblVector3& vDir, float fLength) const
{
    PblVector3 vHit;
    return CollisionMesh::_RayTestAABB(vStart, vDir, fLength, mAABB, vHit);
}

// FUNCTION: SWBF2 0x00460f00
void CollisionTreeNode::AddAllocPool(int iBytes, int iIndices)
{
    membytes = 0;
    memtotal = iBytes;
    memblock = new char[iBytes];
    nodes = 0;
    leafs = 0;
    indexpool = (unsigned short*)(memblock + iBytes - 2 * iIndices);
}

// FUNCTION: SWBF2 0x00460fd0
CollisionTreeNode* ReadTree(PblFileChunk& chunk)
{
    PblFileChunk child = chunk.ReadNextChild();
    switch (child._iId)
    {
    case 0x45444F4E:
    {
        CollisionTreeBranch* pBranch = new CollisionTreeBranch;
        child._pFile->ReadData(&pBranch->aabb, 6);
        pBranch->SetLChild(ReadTree(chunk));
        pBranch->SetGChild(ReadTree(chunk));
        return pBranch;
    }
    case 0x4641454C:
    {
        unsigned char uNumVerts = child._pFile->Read8();
        CollisionTreeLeaf* pLeaf = new CollisionTreeLeaf(uNumVerts);
        child._pFile->ReadData(&pLeaf->aabb, 6);
        unsigned short* pIndices = pLeaf->GetIndexPointer();
        pIndices[0] = child._pFile->Read16();
        for (int i = pLeaf->numVerts - 1; i > 0; --i)
        {
            pIndices[i] = child._pFile->Read16();
        }
        return pLeaf;
    }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00461120
void CollisionMesh::ClearAll()
{
    _HashTable.Clear();
    _HashTableNumEntries = 0;
    memset(mTopCollision, 0, sizeof(mTopCollision));
}

// FUNCTION: SWBF2 0x00461150
void CollisionMesh::_Read(PblFileChunk& chunk)
{
    int iNumNodes = -1;
    int iNumLeafs = -1;
    int iNumIndices = -1;
    mPoslCount = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x454D414E:
        {
            char acName[128];
            child.ReadString(acName, 128);
            strlcpy(sCurrentCollision, acName, 32);
            mNameHash = PblHash(acName);
            if (_HashTable.Find(PblHash(sCurrentCollision)))
            {
                return;
            }
            break;
        }
        case 0x45455254:
        {
            int iBytes = 2 * (iNumIndices + 6 * (iNumNodes + iNumLeafs));
            if (iBytes > mTopCollision[0].mSize)
            {
                strlcpy(mTopCollision[0].mName, sCurrentCollision, 32);
                mTopCollision[0].mSize = iBytes;
                qsort(mTopCollision, 25, sizeof(CollisionSizeData), _topCollisionCallback);
            }
            CollisionTreeNode::AddAllocPool(iBytes, iNumIndices);
            mRoot = ReadTree(child);
            break;
        }
        case 0x45444F4E:
        {
            char acName[128];
            child.ReadString(acName, 128);
            break;
        }
        case 0x49534F50:
            mPoslList = new PblVector3[mPoslCount];
            for (unsigned int i = 0; i < mPoslCount; ++i)
            {
                mPoslList[i].x = child._pFile->ReadFloat();
                mPoslList[i].y = child._pFile->ReadFloat();
                mPoslList[i].z = child._pFile->ReadFloat();
            }
            break;
        case 0x4B53414D:
            mMask = child._pFile->Read8();
            break;
        case 0x4F464E49:
        {
            mPoslCount = child._pFile->Read32();
            iNumNodes = child._pFile->Read32();
            iNumLeafs = child._pFile->Read32();
            iNumIndices = child._pFile->Read32();
            PblVector3 vMin, vMax;
            vMin.x = child._pFile->ReadFloat();
            vMin.y = child._pFile->ReadFloat();
            vMin.z = child._pFile->ReadFloat();
            vMax.x = child._pFile->ReadFloat();
            vMax.y = child._pFile->ReadFloat();
            vMax.z = child._pFile->ReadFloat();
            mAABB.boxmin = vMin;
            mAABB.boxmax = vMax;
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x00461410
void CollisionMesh::Read(PblFileChunk& chunk)
{
    CollisionMesh* pMesh = new CollisionMesh;
    pMesh->_Read(chunk);
    if (!CollisionMesh::_HashTable.Find(pMesh->GetName()))
    {
        if (CollisionMesh::_HashTable.Store(pMesh->GetName(), pMesh))
        {
            ++CollisionMesh::_HashTableNumEntries;
        }
    }
    else
    {
        delete pMesh;
    }
}

// FUNCTION: SWBF2 0x004610f0
int __cdecl _topCollisionCallback(const void* pA, const void* pB)
{
    const int iA = ((const int*)pA)[8];
    const int iB = ((const int*)pB)[8];
    if (iA > iB)
    {
        return 1;
    }
    return iA < iB ? -1 : 0;
}

// GLOBAL: SWBF2 0x01e30728
char* CollisionTreeNode::memblock;

// GLOBAL: SWBF2 0x01e30730
unsigned int CollisionTreeNode::membytes;

// GLOBAL: SWBF2 0x01e3072c
unsigned int CollisionTreeNode::memtotal;

// GLOBAL: SWBF2 0x01e3039c
unsigned short* CollisionTreeNode::indexpool;

// GLOBAL: SWBF2 0x01e30398
int CollisionTreeNode::nodes;

// GLOBAL: SWBF2 0x01e30370
int CollisionTreeNode::leafs;

// GLOBAL: SWBF2 0x01eb3390
int CollisionMesh::_HashTableNumEntries;

// GLOBAL: SWBF2 0x01eb3394
// SYNTHETIC: SWBF2 0x00402460
// `dynamic initializer for 'CollisionMesh::_HashTable''

PblHashTable<CollisionMesh, 1024> CollisionMesh::_HashTable;
