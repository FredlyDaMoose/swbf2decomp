#include "ParticleSpawner.h"

#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblMath.h"

#include "Particle.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x0060d8d0
void SetupRotationResult(PblVector3& vDir, const ParticleSpawner::RotationData& data,
                float fTime, int iIndex, PblRandom& rand)
{
    const float fStartX = data.mStartX.GetValue(rand);
    const float fMoveX = data.mMoveX.GetValue(rand);
    const float fStartMoveX = data.mStartMoveX.GetValue(rand);
    float fAngle = (fMoveX * fTime + fStartX) + (float)iIndex * fStartMoveX;
    if (fAngle != 0.0f)
    {
        vDir.x = PblMath::Sin(fAngle);
    }

    const float fStartY = data.mStartY.GetValue(rand);
    const float fMoveY = data.mMoveY.GetValue(rand);
    const float fStartMoveY = data.mStartMoveY.GetValue(rand);
    fAngle = (fMoveY * fTime + fStartY) + (float)iIndex * fStartMoveY;
    if (fAngle != 0.0f)
    {
        vDir.y = PblMath::Sin(fAngle);
    }

    const float fStartZ = data.mStartZ.GetValue(rand);
    const float fMoveZ = data.mMoveZ.GetValue(rand);
    const float fStartMoveZ = data.mStartMoveZ.GetValue(rand);
    fAngle = (fMoveZ * fTime + fStartZ) + (float)iIndex * fStartMoveZ;
    if (fAngle != 0.0f)
    {
        vDir.z = PblMath::Sin(fAngle);
    }
}

// FUNCTION: SWBF2 0x0060d5b0
ParticleSpawner::ParticleSpawner()
{
    mDirectionType = SPAWN_NONE;
    mDirectionData.mRotationData.mStartX.Setup(0.0f, 1.0f);
    mDirectionData.mRotationData.mStartY.Setup(0.0f, 1.0f);
    mDirectionData.mRotationData.mStartZ.Setup(0.0f, 1.0f);
    mDirectionData.mRotationData.mMoveX.Setup(1.5f);
    mDirectionData.mRotationData.mMoveY.Setup(0.0f);
    mDirectionData.mRotationData.mMoveZ.Setup(1.2f);
    mDirectionData.mRotationData.mStartMoveX.Setup(0.033f);
    mDirectionData.mRotationData.mStartMoveY.Setup(0.073f);
    mDirectionData.mRotationData.mStartMoveZ.Setup(0.093f);
    mOffset.mPositionX.Setup(0.0f);
    mOffset.mPositionY.Setup(0.0f);
    mOffset.mPositionZ.Setup(0.0f);
    mPositionDirectionScale.Setup(5.0f);
    mVelocityDirectionScale.Setup(0.01f);
    mInheritVelocityFactor.Setup(0.0f);
    mNumSizes = 1;
    mSize[0].Setup(0.3f);
    mNumColors = 1;
    mRed[0].Setup(0.9f, 1.0f);
    mGreen[0].Setup(0.9f, 1.0f);
    mBlue[0].Setup(0.9f, 1.0f);
    mAlpha[0].Setup(0.5f);
    mFadeInTime = 0.0f;
    mUseHSV = false;
    mNumRotations = 1;
    mStartRotation[0].Setup(0.0f, 360.0f);
    mRotVelocity[0].Setup(-50.0f, 50.0f);
    mDirectionalColorFlag = false;
    mCache = 0;
    mSeed = PblRandom::GetGlobalInt();
}

// FUNCTION: SWBF2 0x0060d790
void SetupSpread(PblVector3& v, const ParticleSpawner::SpreadData& data, PblRandom& rand)
{
    v.x = data.mPositionX.GetValue(rand);
    v.y = data.mPositionY.GetValue(rand);
    v.z = data.mPositionZ.GetValue(rand);
}

// FUNCTION: SWBF2 0x0060dcd0
void NormalizeSafe(PblVector3& v)
{
    if (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f)
    {
        v.y = 1.0f;
        return;
    }
    PblVector3 vNorm;
    D3DXVec3Normalize(&vNorm, &v);
    v = vNorm;
}

// FUNCTION: SWBF2 0x0060dd30
void ParticleSpawner::SetupParticle(Particle& particle, const PblMatrix& mat, unsigned int uSeed, int iIndex, float fAge, float fSize, const PblVector3& vVelocity, float fInheritVelocity, bool bNoRegisterStep) const
{
    PblRandom rand(((unsigned int)iIndex * 0x40018FC9u + uSeed * 0x4000029Du) % 0x40000003u ^ 0xC71C2830u);
    SetupLocalParticle(particle, rand, iIndex);
    particle.mLod = (unsigned char)(iIndex & 3);

    PblVector3 v;
    D3DXVec3TransformCoord(&v, &particle.mPosition, &mat);
    particle.mPosition = v;
    D3DXVec3TransformNormal(&v, &particle.mVelocity, &mat);
    particle.mVelocity = v;

    const float fInherit = mInheritVelocityFactor.GetValue(rand) + fInheritVelocity;
    particle.mVelocity.x += vVelocity.x * fInherit;
    particle.mVelocity.y += vVelocity.y * fInherit;
    particle.mVelocity.z += vVelocity.z * fInherit;
    particle.mSize[0] *= fSize;
    particle.mUseHSV = mUseHSV;
    particle.mLifeTime = fAge;
    particle.mNoRegisterStep = bNoRegisterStep;
}

// FUNCTION: SWBF2 0x0060de90
void ParticleSpawner::SetupLocalParticle(Particle& particle, PblRandom& rand, int iIndex) const
{
    PblVector3 vDir;
    switch (mDirectionType)
    {
    case ParticleSpawner::SPAWN_SPREAD:
        SetupSpread(vDir, mDirectionData.mSpreadData, rand);
        break;
    case ParticleSpawner::SPAWN_VORTEX:
        SetupRotationResult(vDir, mDirectionData.mRotationData, 0.0f, iIndex, rand);
        break;
    case ParticleSpawner::SPAWN_CIRCLE:
        SetupSpread(vDir, mDirectionData.mSpreadData, rand);
        NormalizeSafe(vDir);
        break;
    case ParticleSpawner::SPAWN_ROTATOR:
        SetupRotationResult(vDir, mDirectionData.mRotationData, 0.0f, iIndex, rand);
        NormalizeSafe(vDir);
        break;
    default:
        vDir.z = 0.0f;
        vDir.y = 0.0f;
        vDir.x = 0.0f;
        break;
    }

    PblVector3 vOffset;
    SetupSpread(vOffset, mOffset, rand);
    const float fPositionScale = mPositionDirectionScale.GetValue(rand);
    particle.mPosition.x = vOffset.x + fPositionScale * vDir.x;
    particle.mPosition.y = vOffset.y + vDir.y * fPositionScale;
    particle.mPosition.z = vOffset.z + vDir.z * fPositionScale;
    const float fVelocityScale = mVelocityDirectionScale.GetValue(rand);
    particle.mVelocity.x = fVelocityScale * vDir.x;
    particle.mVelocity.y = vDir.y * fVelocityScale;
    particle.mVelocity.z = vDir.z * fVelocityScale;
    particle.mSize[0] = mSize[0].GetValue(rand);
    particle.mColor[0].r = mRed[0].GetValue(rand);
    particle.mColor[0].g = mGreen[0].GetValue(rand);
    particle.mColor[0].b = mBlue[0].GetValue(rand);
    particle.mColor[0].a = mAlpha[0].GetValue(rand);
    particle.mRotation[0] = mStartRotation[0].GetValue(rand);
    particle.mRotVelocity[0] = mRotVelocity[0].GetValue(rand);
}

struct CachedSpawnedParticle
{
    PblVector3 mPos;
    float mSize;
    PblVector3 mVel;
    float mRot;
    float mRotVel;
    RedColorValue mColor;
    float mInheritVelocityFactor;
    ParticleSpawner* mSpawner;
};

// SYNTHETIC: SWBF2 0x0060d5a0
// CachedSpawnedParticle::CachedSpawnedParticle

class ParticleSpawnCache
{
public:

    ParticleSpawnCache() { mSpawner = 0; }

    ParticleSpawner* mSpawner;
    CachedSpawnedParticle mCache[1];

    static int mCurrentCache;
    static ParticleSpawnCache mCaches[1];
};

// GLOBAL: SWBF2 0x01ef50e0
// SYNTHETIC: SWBF2 0x00405ce0
// `dynamic initializer for 'ParticleSpawnCache::mCaches''
ParticleSpawnCache ParticleSpawnCache::mCaches[1];
