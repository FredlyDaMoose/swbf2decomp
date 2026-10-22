#pragma once

#include <stddef.h>
#include "CollisionObject.h"

class CollisionObject;
class PblVector3;
class Restrictor;

class EntityTerrain : public CollisionObject
{
public:

    EntityTerrain();
    virtual ~EntityTerrain();

    virtual const PblVector3 & GetPosition() const;
    virtual const PblMatrix & GetMatrix() const;
    virtual bool CheckForCollision(CollisionObject *, Restrictor *);
    virtual float RayHit(const PblVector3 &, const PblVector3 &, float, PblVector3 *, int *, bool);

    virtual bool RayTest(const PblVector3 &, const PblVector3 &, float);

    virtual const CollisionObject * GetCollisionObject() const;
    virtual CollisionObject * GetCollisionObject();
};
 
