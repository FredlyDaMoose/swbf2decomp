#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"

class CameraManager;
class PblMatrix;
class RedCamera;

extern PblVector3 gVectorZero;

// VTABLE: SWBF2 0x00797158
class GameCamera
{
public:
    GameCamera(CameraManager* pManager, int iView) : mManager(pManager), mView(iView) {}
    // SYNTHETIC: SWBF2 0x00453700
    // GameCamera::`scalar deleting destructor'
    virtual ~GameCamera() {}
    // FUNCTION: SWBF2 0x00453690
    virtual void Update(float) {}
    // FUNCTION: SWBF2 0x004536a0
    virtual void SetupCamera(RedCamera*, float) {}
    // FUNCTION: SWBF2 0x004536b0
    virtual void SetupInterpolatedCamera(RedCamera*) {}
    // FUNCTION: SWBF2 0x004536c0
    virtual void SetupIconVisibility(int) {}
    // FUNCTION: SWBF2 0x004536d0
    virtual void SetMatrix(const PblMatrix&) {}
    // FUNCTION: SWBF2 0x004536e0
    virtual PblVector3 GetVelocity() { return gVectorZero; }

    CameraManager* mManager;
    int mView;
};
 
 
 
