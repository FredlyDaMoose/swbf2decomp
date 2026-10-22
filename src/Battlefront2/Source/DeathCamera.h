#pragma once

#include <stddef.h>

#include "GameCamera.h"
#include "PebbleFL/Common/PblMatrix.h"

class Character;
class Trackable;

// VTABLE: SWBF2 0x00798e8c
class DeathCamera : public GameCamera
{
public:
    DeathCamera(CameraManager* pManager, int iView);
    virtual ~DeathCamera();
    virtual void Update(float fDeltaTime);
    virtual void SetupCamera(RedCamera* pCamera, float fDeltaTime);
    virtual void SetupIconVisibility(int iView);

    void SetInfo(Trackable* pOwner, Character* pKiller, const PblMatrix& rMatrix);

    Trackable* mOwner;
    PblMatrix mMatrix;
    PblVector3 mDeathMidPos;
    PblVector3 mDeathPos;
    float mTimer;
    Character* mKiller;
};
 
 
 
