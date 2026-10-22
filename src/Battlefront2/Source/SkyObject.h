#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/Renderable.h"

class PblConfig;
class RedModel;

class SkyObjectManager
{
public:

    // VTABLE: SWBF2 0x007ae638
    // VTABLE: SWBF2 0x007ae658
    class SkyObjectClass : public Renderable, public Thread
    {
    public:

        enum SkyObjectType
        {
            BIGSHIP = 0,
            SMALLSHIP = 1,
            ROCKET = 2,
            NUM_OBJECT_TYPES = 3
        };

        class SkyObject
        {
        public:

            SkyObject() {}

            bool Update(float fDeltaTime);

            PblMatrix mMatrix;
            PblVector3 mVelocity;
            PblVector3 mAcceleration;
            float mAlpha;
            float mLifeTime;
        };

        SkyObjectClass(PblConfig& config);

        virtual void Render(int value);

        virtual bool Update(float fDeltaTime);

        void Spawn(int i);

        SkyObjectType mType;
        RedModel* mModel;
        float mVelocityX;
        float mVelocityVarianceX;
        float mVelocityY;
        float mVelocityVarianceY;
        float mVelocityZ;
        float mVelocityVarianceZ;
        float mHeight;
        float mHeightVariance;
        float mDistance;
        float mInDirectionFactor;
        float mMaxDist;
        float mMaxLifeTime;
        PblVector3 mMidPos;
        PblVector3 mAcceleration;
        SkyObject* mObjects;
        int mNumObjects;
    };

    static void Clear();

    static void Load(PblConfig& config);

    static SkyObjectClass* mObjectClasses[10];
    static int mNumObjectClasses;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
  