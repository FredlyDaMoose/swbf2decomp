#include "CollisionBody.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <stdio.h>
#include <d3dx9math.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"

// LIBRARY: SWBF2 0x0041fa10
// sqrtf

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static inline const PblVector3 operator-(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline const float Length(const PblVector3& v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline const PblMatrix operator*(const PblMatrix& a, const PblMatrix& b)
{
    PblMatrix result;
    D3DXMatrixMultiply((D3DXMATRIX*)&result, (const D3DXMATRIX*)&a, (const D3DXMATRIX*)&b);
    return result;
}

static inline const PblVector3& LastPosition(const CollisionObject* pObject)
{
    return pObject->mLastPosition;
}

// GLOBAL: SWBF2 0x01eb1368
int CollisionBodyPrimitive::_HashTableNumEntries;
// GLOBAL: SWBF2 0x01eb136c
// SYNTHETIC: SWBF2 0x00402440
// `dynamic initializer for 'CollisionBodyPrimitive::_HashTable''

PblHashTable<CollisionBodyPrimitive, 1024> CollisionBodyPrimitive::_HashTable;

// FUNCTION: SWBF2 0x0045ce40
bool CollisionBody::CheckForCollision(CollisionObject*, CollisionObject*, const CollisionBody*, Restrictor*)
{
    return false;
}

// FUNCTION: SWBF2 0x0045ce50
bool CollisionBodyPrimitive::CheckForCollision(CollisionObject* pObject, CollisionObject* pOtherObject,
                                               const CollisionBody* pOtherBody, Restrictor* pRestrictor)
{
    PblMatrix matrix = GetWorldMatrix(pObject->GetMatrix(), pObject->GetPose());
    PblMatrix otherMatrix = pOtherBody->GetWorldMatrix(pOtherObject->GetMatrix(), pOtherObject->GetPose());
    return pOtherBody->ReceiveCollision(matrix, mPrimitive, pObject, otherMatrix, pOtherObject, pRestrictor);
}

// FUNCTION: SWBF2 0x0045cee0
bool CollisionBodyMesh::CollideTerrain(const PblMatrix&, PblVector3&, float&, PblVector3&) const
{
    return false;
}

// FUNCTION: SWBF2 0x0045cef0
CollisionBody::CollisionBody(const char* pcName, const PblMatrix& matrix, unsigned int uiParentHashName)
{
    Init(pcName, matrix, uiParentHashName);
}

// SYNTHETIC: SWBF2 0x0045cf50
// CollisionBody::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0045cf80
CollisionBody::~CollisionBody()
{
}

// FUNCTION: SWBF2 0x0045cf90
void CollisionBody::Init(const char* pcName, const PblMatrix& matrix, unsigned int uiParentHashName)
{
    mName = PblHash(pcName);
    mMatrix = matrix;
    mParentHashName = uiParentHashName;
    mMask = 0;
}

// FUNCTION: SWBF2 0x0045cfe0
PblMatrix CollisionBody::GetLocalMatrix(const RedPose* pPose) const
{
    if (pPose && mParentHashName)
    {
        const PblMatrix* pParent = pPose->_Table.Find(mParentHashName);
        if (pParent)
        {
            return mMatrix * *pParent;
        }
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x0045d060
PblMatrix CollisionBody::GetWorldMatrix(const PblMatrix& matrix, const RedPose* pPose) const
{
    return GetLocalMatrix(pPose) * matrix;
}

// FUNCTION: SWBF2 0x0045d090
float CollisionBody::GetHeight() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x0045d0a0
CollisionBodyPrimitive::CollisionBodyPrimitive()
    : CollisionBody(NULL, _IdentityMatrix, 0)
{
    mMask = 0;
    bIsMesh = false;
}

// SYNTHETIC: SWBF2 0x0045d0d0
// CollisionBodyPrimitive::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004550b0
CollisionBodyPrimitive::~CollisionBodyPrimitive()
{
}

// FUNCTION: SWBF2 0x0045d100
CollisionBodyPrimitive::CollisionBodyPrimitive(CollisionPrimitive primitive, const char* pcName,
                                               const PblMatrix& matrix, unsigned int uiParentHashName)
    : CollisionBody(pcName, matrix, uiParentHashName)
{
    mPrimitive = primitive;
    mMask = 0;
    bIsMesh = false;
}

// FUNCTION: SWBF2 0x0045d150
void CollisionBodyPrimitive::Init(CollisionPrimitive primitive, const char* pcName, const PblMatrix& matrix,
                                  unsigned int uiParentHashName)
{
    mPrimitive = primitive;
    CollisionBody::Init(pcName, matrix, uiParentHashName);
}

// FUNCTION: SWBF2 0x0045d190
bool CollisionBodyPrimitive::RayTest(const RedPose* pPose, const PblVector3& vStart, const PblVector3& vEnd,
                                     float fLength)
{
    return mPrimitive.RayTest(GetLocalMatrix(pPose), vStart, vEnd, fLength);
}

// FUNCTION: SWBF2 0x0045d1d0
float CollisionBodyPrimitive::RayHit(const RedPose* pPose, const PblVector3& vStart, const PblVector3& vEnd,
                                     float fLength, PblVector3* pNormal, float fRadius)
{
    return mPrimitive.RayHit(GetLocalMatrix(pPose), vStart, vEnd, fLength, pNormal, fRadius);
}

// FUNCTION: SWBF2 0x0045d230
bool CollisionBodyPrimitive::CollideTerrain(const PblMatrix& matrix, PblVector3& vNormal, float& fDepth,
                                            PblVector3& vPos) const
{
    return mPrimitive.CollideTerrain(matrix, vNormal, fDepth, vPos);
}

// FUNCTION: SWBF2 0x0045d240
void CollisionBodyPrimitive::Render(const PblMatrix& matrix) const
{
    mPrimitive.Render(matrix);
}

// FUNCTION: SWBF2 0x0045d250
bool CollisionBodyPrimitive::ReceiveCollision(const PblMatrix& matrix, const CollisionPrimitive& primitive,
                                              CollisionObject* pObject, const PblMatrix& otherMatrix,
                                              CollisionObject* pOtherObject, Restrictor*) const
{
    CollisionObject::sCollidingWithMesh = false;
    CollisionResult& result = CollisionModel::sCollisionResult;
    return primitive.CheckForCollision(matrix, pObject->GetPosition() - LastPosition(pObject), mPrimitive,
                                       otherMatrix, pOtherObject->GetPosition() - LastPosition(pOtherObject),
                                       result.mSeparationNormal, result.mOverlap);
}

// FUNCTION: SWBF2 0x0045d300
void CollisionBodyPrimitive::Read(PblFileChunk& chunk)
{
    char acBaseName[128];
    char acName[128];
    char acParentName[128];
    char acPrimName[128];
    PblMatrix matrix;
    int iNumPrims = 0;
    int iPrim = 0;
    unsigned char ucMask = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            child.ReadString(acBaseName, 128);
            iNumPrims = child._pFile->Read32();
            break;
        case 0x544E5250:
            child.ReadString(acParentName, 128);
            break;
        case 0x4D524658:
            matrix.m[0][0] = child._pFile->ReadFloat();
            matrix.m[0][1] = child._pFile->ReadFloat();
            matrix.m[0][2] = child._pFile->ReadFloat();
            matrix.m[0][3] = 0.0f;
            matrix.m[1][0] = child._pFile->ReadFloat();
            matrix.m[1][1] = child._pFile->ReadFloat();
            matrix.m[1][2] = child._pFile->ReadFloat();
            matrix.m[1][3] = 0.0f;
            matrix.m[2][0] = child._pFile->ReadFloat();
            matrix.m[2][1] = child._pFile->ReadFloat();
            matrix.m[2][2] = child._pFile->ReadFloat();
            matrix.m[2][3] = 0.0f;
            matrix.m[3][0] = child._pFile->ReadFloat();
            matrix.m[3][1] = child._pFile->ReadFloat();
            matrix.m[3][2] = child._pFile->ReadFloat();
            matrix.m[3][3] = 1.0f;
            break;
        case 0x41544144:
        {
            int iType = child._pFile->Read32();
            float f0 = child._pFile->ReadFloat();
            float f1 = child._pFile->ReadFloat();
            float f2 = child._pFile->ReadFloat();
            if (iPrim < iNumPrims)
            {
                _snprintf(acPrimName, 127, "%s%d", acBaseName, iPrim);
                unsigned int uiHash = PblHash(acPrimName);
                if (!_HashTable.Find(uiHash))
                {
                    CollisionPrimitive primitive;
                    switch (iType)
                    {
                    case 0:
                    case 1:
                    case 3:
                        primitive.SetSphere(f0);
                        break;
                    case 2:
                        primitive.SetCylinder(f0, f1);
                        break;
                    case 4:
                        primitive.SetBox(f0, f1, f2, -1.0f);
                        break;
                    }
                    CollisionBodyPrimitive* pBody =
                        new CollisionBodyPrimitive(primitive, acName, matrix, PblTEMPHash(acParentName));
                    if (ucMask)
                    {
                        pBody->SetMask(ucMask);
                    }
                    if (_HashTable.Store(uiHash, pBody))
                    {
                        ++_HashTableNumEntries;
                    }
                }
            }
            ++iPrim;
            break;
        }
        case 0x454D414E:
            child.ReadString(acName, 128);
            break;
        case 0x4B53414D:
            ucMask = child._pFile->Read8();
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0045d690
float CollisionBodyPrimitive::GetRadius() const
{
    if (mPrimitive.mType == CollisionPrimitive::AXIS_ALIGNED_CYLINDER ||
        mPrimitive.mType == CollisionPrimitive::CYLINDER)
    {
        return sqrtf(mPrimitive.mHeight * 0.25f * mPrimitive.mHeight + mPrimitive.mRadius * mPrimitive.mRadius);
    }
    return mPrimitive.mRadius;
}

// FUNCTION: SWBF2 0x0045d6e0
PblMatrix CollisionBodyPrimitive::GetMomentOfInertia() const
{
    PblMatrix inertia = _IdentityMatrix;
    float fXX, fYY, fZZ;
    if (mPrimitive.mType == CollisionPrimitive::BOX)
    {
        float fHeight2 = mPrimitive.mHeight * mPrimitive.mHeight;
        float fDepth2 = mPrimitive.mDepth * mPrimitive.mDepth;
        float fWidth2 = mPrimitive.mWidth * mPrimitive.mWidth;
        fXX = (fHeight2 + fDepth2) * 0.33333f;
        fYY = (fWidth2 + fDepth2) * 0.33333f;
        fZZ = (fWidth2 + fHeight2) * 0.33333f;
    }
    else if (mPrimitive.mType == CollisionPrimitive::AXIS_ALIGNED_CYLINDER ||
             mPrimitive.mType == CollisionPrimitive::CYLINDER)
    {
        float fRadius = mPrimitive.mRadius;
        fXX = mPrimitive.mHeight * 0.08333f * mPrimitive.mHeight + fRadius * 0.25f * fRadius;
        fYY = fRadius * 0.5f * fRadius;
        fZZ = fXX;
    }
    else
    {
        fXX = mPrimitive.mRadius * 0.4f * mPrimitive.mRadius;
        fYY = fXX;
        fZZ = fXX;
    }
    inertia.m[0][0] = fXX;
    inertia.m[0][1] = 0.0f;
    inertia.m[0][2] = 0.0f;
    inertia.m[0][3] = 0.0f;
    inertia.m[1][0] = 0.0f;
    inertia.m[1][1] = fYY;
    inertia.m[1][2] = 0.0f;
    inertia.m[1][3] = 0.0f;
    inertia.m[2][0] = 0.0f;
    inertia.m[2][1] = 0.0f;
    inertia.m[2][2] = fZZ;
    inertia.m[2][3] = 0.0f;
    return inertia;
}

// FUNCTION: SWBF2 0x0045d820
float CollisionBodyPrimitive::GetHeight() const
{
    return mPrimitive.mHeight;
}

void CollisionBodyPrimitive::ClearAll()
{
    _HashTable.Clear();
    _HashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x0045d830
CollisionBodyMesh::CollisionBodyMesh(const CollisionMesh* pMesh, const char* pcName, const PblMatrix& matrix,
                                     unsigned int uiParentHashName)
    : CollisionBody(pcName, matrix, uiParentHashName)
{
    mMesh = const_cast<CollisionMesh*>(pMesh);
    mMask = pMesh->Get8BitMask();
    bIsMesh = true;
}

// SYNTHETIC: SWBF2 0x0045d870
// CollisionBodyMesh::`scalar deleting destructor'

CollisionBodyMesh::~CollisionBodyMesh()
{
}

// FUNCTION: SWBF2 0x0045d8a0
bool CollisionBodyMesh::RayTest(const RedPose*, const PblVector3& vStart, const PblVector3& vEnd, float fLength)
{
    return mMesh->RayTest(vStart, vEnd, fLength);
}

// FUNCTION: SWBF2 0x0045d8c0
float CollisionBodyMesh::RayHit(const RedPose*, const PblVector3& vStart, const PblVector3& vEnd, float fLength,
                                PblVector3* pNormal, float)
{
    return mMesh->RayHit(vStart, vEnd, fLength, pNormal);
}

// FUNCTION: SWBF2 0x0045d8f0
void CollisionBodyMesh::Render(const PblMatrix& matrix) const
{
    mMesh->Render(matrix);
}

// FUNCTION: SWBF2 0x0045d900
bool CollisionBodyMesh::ReceiveCollision(const PblMatrix& matrix, const CollisionPrimitive& primitive,
                                         CollisionObject* pObject, const PblMatrix& meshMatrix,
                                         CollisionObject* pMeshObject, Restrictor* pRestrictor) const
{
    CollisionMeshCache* pCache = NULL;
    if (!primitive.InitMeshCollision(matrix, mMesh, meshMatrix, pObject, pCache))
    {
        return false;
    }

    CollisionObject::sCollidingWithMesh = true;
    bool bCollided = false;
    PblVector3 vLastPos = pObject->GetPosition();
    if (pCache)
    {
        pCache->aClosest += Length(pCache->lastPos - vLastPos);
        pCache->lastPos = vLastPos;
    }

    float fClosest = 100000.0f;
    if (pCache)
    {
        fClosest = pCache->aClosest;
    }

    CollisionResult& result = CollisionModel::sCollisionResult;
    while (primitive.CollideMesh(matrix, mMesh, meshMatrix, result.mSeparationNormal, result.mOverlap, fClosest))
    {
        if (pObject->CollisionCallback(&result, pMeshObject, pRestrictor))
        {
            PblVector3 vPos = pObject->GetPosition();
            PblVector3 vMove = vPos - vLastPos;
            vLastPos = vPos;
            fClosest += Length(vMove);
            CollisionPrimitive::MoveMeshTestPos(vMove);
            bCollided = true;
        }
    }

    if (pCache)
    {
        pCache->aClosest = fClosest;
    }
    return bCollided;
}
