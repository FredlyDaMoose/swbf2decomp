#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblVector.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class CollisionObject;
class GameObject;

class StickInfo
{
public:

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    // FUNCTION: SWBF2 0x004d0880
    StickInfo()
    {
        mLocalRotation._fS = 1.0f;
        mLocalRotation._fX = 0.0f;
        mLocalRotation._fY = 0.0f;
        mLocalRotation._fZ = 0.0f;
    }

    PblHandle<GameObject> mCollHandle;
    CollisionObject* mCollObject;
    int mCollBodyID;
    PblVector3 mLocalPosition;
    PblQuaternion mLocalRotation;

    static MemoryPool sMemoryPool;
};

 
 
