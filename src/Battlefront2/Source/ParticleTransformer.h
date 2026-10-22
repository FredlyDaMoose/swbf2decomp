#pragma once

#include <stddef.h>

#include "RedEngineFL/Memory/RedMemoryPool.h"

class Particle;
class PblVector3;
class RedColorValue;

class ParticleTransformer
{
public:

    // VTABLE: SWBF2 0x007ad430
    class TransformState
    {
    public:
        TransformState() : mNextState(0), mLifeTime(1.0f), mTimeScale(1.0f) {}

        // SYNTHETIC: SWBF2 0x0060ea90
        // ParticleTransformer::TransformState::`scalar deleting destructor'
        virtual ~TransformState();

        TransformState* mNextState;
        float mLifeTime;
        float mTimeScale;

    protected:

        float GetScale(float fTime) const { return mTimeScale * fTime; }
    };

    class DataTransformer
    {
    public:
        enum DataType { NONE = 0, MOVE = 1, REACH = 2, SCALE = 3 };

        DataTransformer() : mType(NONE) {}

        void Update(float& fData, float fTime) const;

        float mData;
        DataType mType;
    };

    // VTABLE: SWBF2 0x007ad428
    class PositionTransformer : public TransformState
    {
    public:
        enum DataType { NONE = 0, ACCELERATE = 1, REACH = 2, SCALE = 3 };

        struct Accelerate { float mAccX, mAccY, mAccZ; };
        struct Reach { float mVelX, mVelY, mVelZ; };
        struct Scale { float mScale; };

        PositionTransformer() : mType(NONE) {}

        // SYNTHETIC: SWBF2 0x0060eac0
        // ParticleTransformer::PositionTransformer::`scalar deleting destructor'

        static void* operator new(size_t size) { return sMemoryPool.Allocate((unsigned int)size); }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        void Update(PblVector3& position, PblVector3& velocity, float fTime, float fDeltaTime) const;

        union
        {
            Accelerate mAccelerate;
            Reach mReach;
            Scale mScale;
        } mData;
        DataType mType;

        static MemoryPool sMemoryPool;
    };

    // VTABLE: SWBF2 0x007ad440
    class SizeTransformer : public TransformState
    {
    public:
        // SYNTHETIC: SWBF2 0x0060eb40
        // ParticleTransformer::SizeTransformer::`scalar deleting destructor'

        static void* operator new(size_t size) { return sMemoryPool.Allocate((unsigned int)size); }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        void Update(float& fSize, float fTime) const;

        DataTransformer mData;

        static MemoryPool sMemoryPool;
    };

    // VTABLE: SWBF2 0x007ad420
    class RotationTransformer : public TransformState
    {
    public:
        // SYNTHETIC: SWBF2 0x0060eb80
        // ParticleTransformer::RotationTransformer::`scalar deleting destructor'

        static void* operator new(size_t size) { return sMemoryPool.Allocate((unsigned int)size); }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        void Update(float& fRotation, float& fRotVelocity, float fTime) const;

        DataTransformer mData;

        static MemoryPool sMemoryPool;
    };

    // VTABLE: SWBF2 0x007ad438
    class ColorTransformer : public TransformState
    {
    public:
        enum DataType { NONE = 0, MOVE = 1, REACH = 2 };

        ColorTransformer() : mType(NONE) {}

        // SYNTHETIC: SWBF2 0x0060eb00
        // ParticleTransformer::ColorTransformer::`scalar deleting destructor'

        static void* operator new(size_t size) { return sMemoryPool.Allocate((unsigned int)size); }
        static void operator delete(void* p) { sMemoryPool.Free(p); }

        void Update(RedColorValue& color, float fTime) const;

        float mR;
        float mG;
        float mB;
        float mA;
        DataType mType;

        static MemoryPool sMemoryPool;
    };

    ParticleTransformer();
    ~ParticleTransformer();

    bool Update(Particle& particle, float fDeltaTime) const;

    PositionTransformer* mPositionTrans;
    SizeTransformer* mSizeTrans[1];
    int mNumSizeTrans;
    RotationTransformer* mRotationTrans[1];
    int mNumRotationTrans;
    ColorTransformer* mColorTrans[1];
    int mNumColorTrans;
    float mLifeTime;
};
 
 
 
 
 
 
 
