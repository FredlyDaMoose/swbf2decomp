#pragma once

class PblFileChunk;

#include <stddef.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "CollisionPrimitive.h"

class CollisionBody;
class CollisionMesh;
class CollisionObject;
class CollisionPrimitive;
class PblMatrix;
class PblVector3;
class RedPose;
class Restrictor;

// VTABLE: SWBF2 0x007973f4
class CollisionBody
{
protected:
    CollisionBody(const char* pcName, const PblMatrix& matrix, unsigned int uiParentHashName);

public:
    virtual ~CollisionBody();
    virtual bool CheckForCollision(CollisionObject *, CollisionObject *, const CollisionBody *, Restrictor *);
    virtual bool ReceiveCollision(const PblMatrix &, const CollisionPrimitive &, CollisionObject *, const PblMatrix &, CollisionObject *, Restrictor *) const = 0;
    virtual bool RayTest(const RedPose *, const PblVector3 &, const PblVector3 &, float) = 0;
    virtual float RayHit(const RedPose *, const PblVector3 &, const PblVector3 &, float, PblVector3 *, float) = 0;
    virtual bool CollideTerrain(const PblMatrix &, PblVector3 &, float &, PblVector3 &) const = 0;
    virtual void Render(const PblMatrix &) const = 0;
    virtual float GetHeight() const;

    void Init(const char* pcName, const PblMatrix& matrix, unsigned int uiParentHashName);

    void SetMask(unsigned char ucMask) { mMask = ucMask; }
    PblMatrix GetLocalMatrix(const RedPose* pPose) const;
    PblMatrix GetWorldMatrix(const PblMatrix& matrix, const RedPose* pPose) const;

    // FUNCTION: SWBF2 0x004ddfb0
    void SetMatrix(const PblMatrix& matrix) { mMatrix = matrix; }

    PblMatrix mMatrix;
    unsigned int mName;
    unsigned char mMask;
    bool bIsMesh;
    unsigned int mParentHashName;
    unsigned int profileHashName;
};
 
 
 
 
 

// VTABLE: SWBF2 0x007973ac
class CollisionBodyMesh : public CollisionBody
{
public:
    CollisionBodyMesh(const CollisionMesh* pMesh, const char* pcName, const PblMatrix& matrix,
                      unsigned int uiParentHashName);
    virtual ~CollisionBodyMesh();

    virtual bool ReceiveCollision(const PblMatrix &, const CollisionPrimitive &, CollisionObject *, const PblMatrix &, CollisionObject *, Restrictor *) const;
    virtual bool RayTest(const RedPose *, const PblVector3 &, const PblVector3 &, float);
    virtual float RayHit(const RedPose *, const PblVector3 &, const PblVector3 &, float, PblVector3 *, float);
    virtual bool CollideTerrain(const PblMatrix &, PblVector3 &, float &, PblVector3 &) const;
    virtual void Render(const PblMatrix &) const;

    CollisionMesh* mMesh;
};
 
 

// VTABLE: SWBF2 0x007973d0
class CollisionBodyPrimitive : public CollisionBody
{
public:

    static void Read(PblFileChunk& chunk);

    CollisionBodyPrimitive();
    CollisionBodyPrimitive(CollisionPrimitive primitive, const char* pcName, const PblMatrix& matrix,
                           unsigned int uiParentHashName);
    virtual ~CollisionBodyPrimitive();

    void Init(CollisionPrimitive primitive, const char* pcName, const PblMatrix& matrix,
              unsigned int uiParentHashName);

    virtual bool CheckForCollision(CollisionObject *, CollisionObject *, const CollisionBody *, Restrictor *);
    virtual bool ReceiveCollision(const PblMatrix &, const CollisionPrimitive &, CollisionObject *, const PblMatrix &, CollisionObject *, Restrictor *) const;
    virtual bool RayTest(const RedPose *, const PblVector3 &, const PblVector3 &, float);
    virtual float RayHit(const RedPose *, const PblVector3 &, const PblVector3 &, float, PblVector3 *, float);
    virtual bool CollideTerrain(const PblMatrix &, PblVector3 &, float &, PblVector3 &) const;
    virtual void Render(const PblMatrix &) const;
    virtual float GetHeight() const;

    float GetRadius() const;
    PblMatrix GetMomentOfInertia() const;

    static void ClearAll();

    static CollisionBodyPrimitive* Find(unsigned int uHash);

    CollisionPrimitive mPrimitive;

    static int _HashTableNumEntries;
    static PblHashTable<CollisionBodyPrimitive, 1024> _HashTable;
};
 
 
