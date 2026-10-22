#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

class ParticleEmitterInfoData;

class Particle
{
public:

    Particle() : mFrameIndex(-1) {}

    PblVector3 mPosition;
    PblVector3 mVelocity;
    float mSize[1];
    float mRotation[1];
    float mRotVelocity[1];
    RedColorValue mColor[1];
    float mLifeTime;
    bool mUseHSV;
    bool mNoRegisterStep;
    unsigned char mLod;
    char mFrameIndex;
    ParticleEmitterInfoData* mInfoData;
};
 
 
 
 
