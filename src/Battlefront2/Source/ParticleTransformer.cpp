#include "ParticleTransformer.h"
#include "Particle.h"

// GLOBAL: SWBF2 0x01ef7b50
// SYNTHETIC: SWBF2 0x00405d20
// `dynamic initializer for 'ParticleTransformer::PositionTransformer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769250
// `dynamic atexit destructor for 'ParticleTransformer::PositionTransformer::sMemoryPool''
MemoryPool ParticleTransformer::PositionTransformer::sMemoryPool("ParticleTransformer::PositionTransformer",
                                                                 sizeof(ParticleTransformer::PositionTransformer));
// GLOBAL: SWBF2 0x01ef7bf0
// SYNTHETIC: SWBF2 0x00405d40
// `dynamic initializer for 'ParticleTransformer::SizeTransformer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769260
// `dynamic atexit destructor for 'ParticleTransformer::SizeTransformer::sMemoryPool''
MemoryPool ParticleTransformer::SizeTransformer::sMemoryPool("ParticleTransformer::SizeTransformer",
                                                             sizeof(ParticleTransformer::SizeTransformer));
// GLOBAL: SWBF2 0x01ef7c40
// SYNTHETIC: SWBF2 0x00405d60
// `dynamic initializer for 'ParticleTransformer::RotationTransformer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769270
// `dynamic atexit destructor for 'ParticleTransformer::RotationTransformer::sMemoryPool''
MemoryPool ParticleTransformer::RotationTransformer::sMemoryPool("ParticleTransformer::RotationTransformer",
                                                                 sizeof(ParticleTransformer::RotationTransformer));
// GLOBAL: SWBF2 0x01ef7ba0
// SYNTHETIC: SWBF2 0x00405d80
// `dynamic initializer for 'ParticleTransformer::ColorTransformer::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769280
// `dynamic atexit destructor for 'ParticleTransformer::ColorTransformer::sMemoryPool''
MemoryPool ParticleTransformer::ColorTransformer::sMemoryPool("ParticleTransformer::ColorTransformer",
                                                              sizeof(ParticleTransformer::ColorTransformer));

ParticleTransformer::TransformState::~TransformState()
{
    delete mNextState;
}

// FUNCTION: SWBF2 0x0060ebc0
ParticleTransformer::ParticleTransformer()
    : mPositionTrans(0), mNumSizeTrans(0), mNumRotationTrans(0), mNumColorTrans(0)
{
    mSizeTrans[0] = 0;
    mRotationTrans[0] = 0;
    mColorTrans[0] = 0;
}

// FUNCTION: SWBF2 0x0060ec00
ParticleTransformer::~ParticleTransformer()
{
    delete mPositionTrans;

    for (int i = 0; i < 1; ++i)
    {
        if (mSizeTrans[i])
        {
            delete mSizeTrans[i];
            mSizeTrans[i] = 0;
        }
    }
    for (int i = 0; i < 1; ++i)
    {
        if (mRotationTrans[i])
        {
            delete mRotationTrans[i];
            mRotationTrans[i] = 0;
        }
    }
    for (int i = 0; i < 1; ++i)
    {
        if (mColorTrans[i])
        {
            delete mColorTrans[i];
            mColorTrans[i] = 0;
        }
    }
}

// FUNCTION: SWBF2 0x0060ec50
bool ParticleTransformer::Update(Particle& particle, float fDeltaTime) const
{
    if (particle.mLifeTime >= mLifeTime)
    {
        return false;
    }

    if (mPositionTrans)
    {
        mPositionTrans->Update(particle.mPosition, particle.mVelocity, particle.mLifeTime, fDeltaTime);
    }
    else
    {
        particle.mPosition.x += particle.mVelocity.x * particle.mLifeTime;
        particle.mPosition.y += particle.mVelocity.y * particle.mLifeTime;
        particle.mPosition.z += particle.mVelocity.z * particle.mLifeTime;
    }

    for (int i = 0; i < mNumSizeTrans; ++i)
    {
        mSizeTrans[i]->Update(particle.mSize[i], particle.mLifeTime);
    }

    for (int i = 0; i < mNumRotationTrans; ++i)
    {
        mRotationTrans[i]->Update(particle.mRotation[i], particle.mRotVelocity[i], particle.mLifeTime);
    }

    if (mNumRotationTrans == 0)
    {
        particle.mRotation[0] += particle.mRotVelocity[0] * particle.mLifeTime;
    }

    for (int i = 0; i < mNumColorTrans; ++i)
    {
        mColorTrans[i]->Update(particle.mColor[i], particle.mLifeTime);
    }

    return true;
}

// FUNCTION: SWBF2 0x0060edb0
void ParticleTransformer::PositionTransformer::Update(PblVector3& position, PblVector3& velocity, float fTime,
                                                      float fDeltaTime) const
{
    float fScale = GetScale(fTime);
    float fStep = (fScale < 1.0f ? fScale : 1.0f) * 0.5f * mLifeTime;

    position.x += velocity.x * fStep;
    position.y += velocity.y * fStep;
    position.z += velocity.z * fStep;

    switch (mType)
    {
    case ACCELERATE:
        if (fScale >= 1.0f)
        {
            velocity.x += mData.mAccelerate.mAccX * fDeltaTime;
            velocity.y += mData.mAccelerate.mAccY * fDeltaTime;
            velocity.z += mData.mAccelerate.mAccZ * fDeltaTime;
        }
        else
        {
            velocity.x += mData.mAccelerate.mAccX * fScale * fDeltaTime;
            velocity.y += mData.mAccelerate.mAccY * fScale * fDeltaTime;
            velocity.z += mData.mAccelerate.mAccZ * fScale * fDeltaTime;
        }
        break;
    case REACH:
        if (fScale >= 1.0f)
        {
            velocity.x = mData.mReach.mVelX * fDeltaTime;
            velocity.y = mData.mReach.mVelY * fDeltaTime;
            velocity.z = mData.mReach.mVelZ * fDeltaTime;
        }
        else
        {
            velocity.x *= 1.0f - fScale;
            velocity.y *= 1.0f - fScale;
            velocity.z *= 1.0f - fScale;
            velocity.x += mData.mReach.mVelX * fScale * fDeltaTime;
            velocity.y += mData.mReach.mVelY * fScale * fDeltaTime;
            velocity.z += mData.mReach.mVelZ * fScale * fDeltaTime;
        }
        break;
    case SCALE:
        if (fScale >= 1.0f)
        {
            velocity.x *= mData.mScale.mScale;
            velocity.y *= mData.mScale.mScale;
            velocity.z *= mData.mScale.mScale;
        }
        else
        {
            velocity.x = velocity.x * (1.0f - fScale) + velocity.x * mData.mScale.mScale * fScale;
            velocity.y = velocity.y * (1.0f - fScale) + velocity.y * mData.mScale.mScale * fScale;
            velocity.z = velocity.z * (1.0f - fScale) + velocity.z * mData.mScale.mScale * fScale;
        }
        break;
    }

    position.x += velocity.x * fStep;
    position.y += velocity.y * fStep;
    position.z += velocity.z * fStep;

    if (fScale > 1.0f)
    {
        if (mNextState)
        {
            ((const PositionTransformer*)mNextState)->Update(position, velocity, fTime - mLifeTime, fDeltaTime);
        }
        else
        {
            float fRest = (fScale - 1.0f) * mLifeTime;
            position.x += velocity.x * fRest;
            position.y += velocity.y * fRest;
            position.z += velocity.z * fRest;
        }
    }
}

// FUNCTION: SWBF2 0x0060f0c0
void ParticleTransformer::DataTransformer::Update(float& fData, float fTime) const
{
    switch (mType)
    {
    case MOVE:
        fData += mData * (fTime > 1.0f) ? 1.0f : fTime;
        break;
    case REACH:
        if (fTime >= 1.0f)
        {
            fData = mData;
        }
        else
        {
            fData = (1.0f - fTime) * fData + mData * fTime;
        }
        break;
    case SCALE:
        if (fTime >= 1.0f)
        {
            fData *= mData;
        }
        else
        {
            fData = fData * mData * fTime + (1.0f - fTime) * fData;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x0060f190
void ParticleTransformer::SizeTransformer::Update(float& fSize, float fTime) const
{
    float fScale = GetScale(fTime);
    mData.Update(fSize, fScale);
    if (fScale > 1.0f && mNextState)
    {
        ((const SizeTransformer*)mNextState)->Update(fSize, fTime - mLifeTime);
    }
}

// FUNCTION: SWBF2 0x0060f1f0
void ParticleTransformer::RotationTransformer::Update(float& fRotation, float& fRotVelocity, float fTime) const
{
    const RotationTransformer* pState = this;
    for (;;)
    {
        float fScale = pState->GetScale(fTime);
        pState->mData.Update(fRotVelocity, fScale);
        if (fScale <= 1.0f || !pState->mNextState)
        {
            break;
        }
        fRotation += pState->mLifeTime * fRotVelocity;
        fTime -= pState->mLifeTime;
        pState = (const RotationTransformer*)pState->mNextState;
    }
    fRotation += fRotVelocity * fTime;
}

// FUNCTION: SWBF2 0x0060f270
void ParticleTransformer::ColorTransformer::Update(RedColorValue& color, float fTime) const
{
    float fScale = GetScale(fTime);

    switch (mType)
    {
    case MOVE:
    {
        float r, g, b, a;
        if (fScale >= 1.0f)
        {
            r = mR + color.r;
            g = mG + color.g;
            b = mB + color.b;
            a = mA + color.a;
        }
        else
        {
            r = mR * fScale + color.r;
            g = mG * fScale + color.g;
            b = mB * fScale + color.b;
            a = mA * fScale + color.a;
        }
        color.r = r < 0.0f ? 0.0f : (r > 1.0f ? 1.0f : r);
        color.g = g < 0.0f ? 0.0f : (g > 1.0f ? 1.0f : g);
        color.b = b < 0.0f ? 0.0f : (b > 1.0f ? 1.0f : b);
        color.a = a < 0.0f ? 0.0f : (a > 1.0f ? 1.0f : a);
        break;
    }
    case REACH:
        if (fScale >= 1.0f)
        {
            color.r = mR;
            color.g = mG;
            color.b = mB;
            color.a = mA;
        }
        else
        {
            color.r = mR * fScale + color.r * (1.0f - fScale);
            color.g = mG * fScale + color.g * (1.0f - fScale);
            color.b = mB * fScale + color.b * (1.0f - fScale);
            color.a = mA * fScale + color.a * (1.0f - fScale);
        }
        break;
    }

    if (fScale > 1.0f && mNextState)
    {
        ((const ColorTransformer*)mNextState)->Update(color, fTime - mLifeTime);
    }
}
