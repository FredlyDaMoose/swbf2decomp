#include "EntityWater.h"
#include "CollisionManager.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern PblVector3 gVectorZero;

// FUNCTION: SWBF2 0x0050b070
EntityWater::EntityWater()
    : CollisionObject(COLL_WATER, 0, 0, 0)
{
    totalTime = 0.0f;
    SetCollisionCenter(gVectorZero);
    SetCollisionRadius(10000.0f);
    Set2DCollisionRadius(10000.0f);
    CollisionManager::SetWater(this);
    ActivateThread(10000, PER_LOCAL_TURN);
}

// FUNCTION: SWBF2 0x0050b220
bool EntityWater::Update(float fDeltaTime)
{
    totalTime += fDeltaTime;
    return RedWater::Update(totalTime);
}

// FUNCTION: SWBF2 0x0050b250
float EntityWater::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fLength, PblVector3* pNormal,
                          int* pBodyId, bool bOrdnance)
{
    float fHit = RedWater::RayHit(vStart, vDir, fLength, pNormal);
    if (pBodyId)
    {
        *pBodyId = 0;
    }
    return fHit;
}

// FUNCTION: SWBF2 0x0050b1b0
EntityWater::~EntityWater()
{
}

// SYNTHETIC: SWBF2 0x0050b180
// EntityWater::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x0050b2d0
// [thunk]:EntityWater::`scalar deleting destructor' adjustor{136}

// FUNCTION: SWBF2 0x0050b030
const PblVector3& EntityWater::GetPosition() const
{
    return gVectorZero;
}

// FUNCTION: SWBF2 0x0050b040
const PblMatrix& EntityWater::GetMatrix() const
{
    return *reinterpret_cast<const PblMatrix*>(&gMatrixIdentity);
}

// FUNCTION: SWBF2 0x0050b290
bool EntityWater::RayTest(const PblVector3& vStart, const PblVector3& vEnd, float fMaxT)
{
    PblVector3 vNormal;
    return RayHit(vStart, vEnd, fMaxT, &vNormal, 0, false) < 1.0f;
}

// FUNCTION: SWBF2 0x0050b050
const CollisionObject* EntityWater::GetCollisionObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x0050b060
CollisionObject* EntityWater::GetCollisionObject()
{
    return this;
}
