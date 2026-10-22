#pragma once

#include <stddef.h>
#include <string.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"

class PblFileChunk;
class PblMatrix;

class CompressedVector3
{
public:
    unsigned char x;
    unsigned char y;
    unsigned char z;
};
 

class CompressedAABB
{
public:
    CompressedVector3 boxmin;
    CompressedVector3 boxmax;
};
 
 

class CollisionTreeNode
{
public:

    static void* operator new(unsigned int uSize)
    {
        void* p = memblock + membytes;
        membytes += uSize;
        return p;
    }

    static void AddAllocPool(int iBytes, int iIndices);

    static char* memblock;
    static unsigned int membytes;
    static unsigned int memtotal;
    static unsigned short* indexpool;
    static int nodes;
    static int leafs;

    unsigned int data;
    CompressedAABB aabb;
    unsigned char numVerts;
    unsigned char isleaf;
};
 

class CollisionTreeBranch : public CollisionTreeNode
{
public:
    CollisionTreeBranch()
    {
        ++nodes;
        numVerts = 0;
        isleaf = 0;
        data = 0;
    }
    void SetLChild(CollisionTreeNode*) {}
    void SetGChild(CollisionTreeNode* pChild) { data = (unsigned int)pChild; }
};

class CollisionTreeLeaf : public CollisionTreeNode
{
public:

    CollisionTreeLeaf(unsigned char uNumVerts)
    {
        ++leafs;
        numVerts = uNumVerts;
        data = (unsigned int)indexpool;
        indexpool += uNumVerts;
        isleaf = 1;
    }
    unsigned short* GetIndexPointer() { return (unsigned short*)data; }
};

class CollisionAABB
{
public:
    CollisionAABB() {}
    CollisionAABB(const CollisionAABB& parent, const CompressedAABB& compressed);

    PblVector3 boxmin;
    PblVector3 boxmax;
};
 
 
 

// VTABLE: SWBF2 0x00797448
template <int N> class PolyListContainer;
class CollisionMeshCache;
class EntitySoldier;

class CollisionMesh
{
public:

    static bool _RayTestAABB(const PblVector3& vStart, const PblVector3& vDir, float fLength,
                             const CollisionAABB& aabb, PblVector3& vCoord);
    int _RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, CollisionTreeNode* pNode,
                const CollisionAABB& aabb, float& fT, CollisionTreeNode** ppHitLeaf) const;
    void __GetTriHits(const PblVector3& vPos, float fRadius2, const PblVector3& vMin, const PblVector3& vMax,
                      CollisionTreeNode* pNode, const CollisionAABB& aabb, PolyListContainer<100>& list,
                      CollisionMeshCache* pCache) const;
    CollisionMeshCache* _GetTriHits(const PblVector3& vPos, float fRadius, const PblVector3& vMin,
                                    const PblVector3& vMax, PolyListContainer<100>& list,
                                    EntitySoldier* pSoldier) const;
    CollisionMeshCache* GetTriHits(const PblVector3& vPos, float fRadius, const PblVector3& vMin,
                                   const PblVector3& vMax, PolyListContainer<100>& list,
                                   EntitySoldier* pSoldier) const;
    CollisionMesh();
    virtual ~CollisionMesh();

    // SYNTHETIC: SWBF2 0x00460f80
    // CollisionMesh::`scalar deleting destructor'

    static PblHashTable<CollisionMesh, 1024> _HashTable;

    static int _HashTableNumEntries;
    static void ClearAll();

    static const CollisionMesh* Find(unsigned int uHash);

    void _Read(PblFileChunk& chunk);

    static void Read(PblFileChunk& chunk);

    unsigned int GetName() const { return mNameHash; }
    unsigned char Get8BitMask() const { return mMask; }
    bool RayTest(const PblVector3&, const PblVector3&, float) const;
    float RayHit(const PblVector3&, const PblVector3&, float, PblVector3*) const;
    void Render(const PblMatrix&) const;

    PblBox GetWorldspaceAABB(const PblMatrix& matrix) const;

    unsigned int mNameHash;
    unsigned int mPoslCount;

    CollisionAABB mAABB;
    PblVector3* mPoslList;
    CollisionTreeNode* mRoot;
    CollisionTreeNode* mHitLeaf;
    unsigned char mMask;
    CollisionTreeLeaf* mDrawThisLeaf;
    char mDrawThisAABB[24];
};
 
 
 
 
 
 
 
 

class CollisionMeshCache
{
public:
    PblVector3 mInitialPos;
    PblVector3 lastPos;
    CollisionMesh* mMesh;
    float aClosest;
    unsigned char mNumPolys;
    CollisionTreeLeaf* mPolys[70];
};
 
 
 

template <int N>
class PolyListContainer
{
public:
    PolyListContainer() {}

    void Clear()
    {
        mPolyCount = 0;
        mVertCount = 0;
    }

    void GetPolygon(int iPoly, int& iNumVerts, const PblVector3*& pVerts)
    {
        int iFirst = 0;
        for (int i = 0; i < iPoly; ++i)
        {
            iFirst += mNumVerticesPerPoly[i];
        }
        iNumVerts = mNumVerticesPerPoly[iPoly];
        pVerts = &mVertices[iFirst];
    }

    void AddPolygon(unsigned char uNumVerts, const PblVector3* pVerts)
    {
        if (mPolyCount == N)
        {
            return;
        }
        if (mVertCount + uNumVerts >= 3 * N)
        {
            return;
        }
        for (int i = 0; i < uNumVerts; ++i)
        {
            mVertices[mVertCount + i] = pVerts[i];
        }
        mNumVerticesPerPoly[mPolyCount] = uNumVerts;
        mPolyCount++;
        mVertCount += uNumVerts;
    }

    void RemovePolygon(int iPoly)
    {
        mPolyCount--;
        mVertCount -= mNumVerticesPerPoly[iPoly];
        if (mPolyCount > iPoly)
        {
            int iFirst = 0;
            for (int i = 0; i < iPoly; ++i)
            {
                iFirst += mNumVerticesPerPoly[i];
            }
            int iNumVerts = mNumVerticesPerPoly[iPoly];
            memcpy(&mVertices[iFirst], &mVertices[iFirst + iNumVerts],
                   (mVertCount - iFirst) * sizeof(PblVector3));
            memcpy(&mNumVerticesPerPoly[iPoly], &mNumVerticesPerPoly[iPoly + 1], N - iPoly);
        }
    }

    int mPolyCount;
    int mVertCount;
    unsigned char mNumVerticesPerPoly[N];
    PblVector3 mVertices[3 * N];
};
 
