#pragma once

#include <stddef.h>

#include "CollisionObject.h"
#include "Thread.h"

class EntityWater : public CollisionObject, public Thread
{
public:
    EntityWater();
    virtual ~EntityWater();

    virtual const PblVector3& GetPosition() const;
    virtual bool RayTest(const PblVector3& vStart, const PblVector3& vEnd, float fMaxT);
    virtual const PblMatrix& GetMatrix() const;
    virtual float RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, PblVector3* pNormal,
                         int* pBodyId, bool bOrdnance);
    virtual const CollisionObject* GetCollisionObject() const;
    virtual CollisionObject* GetCollisionObject();

    virtual bool Update(float fDeltaTime);

    float totalTime;
};
 
 
