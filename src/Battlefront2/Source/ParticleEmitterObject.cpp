#include "ParticleEmitterObject.h"
#include "PebbleFL/Common/PblVector.h"
#include "ParticleEmitter.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Particle.h"
#include "ParticleSystem.h"
#include "Battlefront2Lod.h"

#include <math.h>

static const float EPSILON = 0.0001f;

extern PblVector3 gVectorZero;

PblVector3 Min(const PblVector3& a, const PblVector3& b);
PblVector3 Max(const PblVector3& a, const PblVector3& b);

// GLOBAL: SWBF2 0x01eaed56
bool ParticleEmitterObject::sForceRegisterStep;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

extern Particle sParticleCache[128];
extern ParticleEmitterObject sPEO;

namespace ParticleSystem { bool IsLodActive(int iLod); }

// GLOBAL: SWBF2 0x01ef5090
// SYNTHETIC: SWBF2 0x00405cc0
// `dynamic initializer for 'ParticleEmitterObject::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769240
// `dynamic atexit destructor for 'ParticleEmitterObject::sMemoryPool''
MemoryPool ParticleEmitterObject::sMemoryPool("ParticleEmitterObject", 0x110);

// FUNCTION: SWBF2 0x0060b910
ParticleEmitterInfoData* ParticleEmitterInfoData::Create()
{
    if (sMemoryPool.mUsed >= sMemoryPool.mCount)
    {
        return 0;
    }
    return new ParticleEmitterInfoData;
}

// SYNTHETIC: SWBF2 0x0060b960
// ParticleEmitterInfoData::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0060ba30
ParticleEmitterObject* ParticleEmitterObject::Create(const ParticleEmitterClass* pClass)
{
    if (sMemoryPool.mUsed >= sMemoryPool.mCount)
    {
        return 0;
    }
    if (NetGame::IsEnabled() && !NetGame::IsClient() &&
        !(NetGame::IsOneWorld() && !(NetGame::IsHost() && NetGame::IsDedicated())))
    {
        return 0;
    }
    return new ParticleEmitterObject(pClass);
}

// FUNCTION: SWBF2 0x0060bb00
ParticleEmitterObject::ParticleEmitterObject(const ParticleEmitterClass* pClass)
    : RedSceneObject(pClass->mLodData)
{
    mLifeTime = 0.0f;
    mEmitter = ParticleEmitter::sMemoryPool.mUsed < ParticleEmitter::sMemoryPool.mCount
                   ? new ParticleEmitter(pClass)
                   : 0;
    mEmitterInfoData = 0;
    ActivateThread(0, PER_FRAME);
    mRegisterTime = 0.0f;
    mStopFlag = false;
    mIsChildPEO = false;
    if (mEmitter)
    {
        mEmitterBoundingRadius = pClass->mBoundingRadius;
        _pLodData = mEmitter->mClass->mLodData;
    }
    m_uiAttachFlags |= UPDATE_REQUIRED;
    bIsEmitterEmitter = false;
    mbAddedToScene = true;
}

// SYNTHETIC: SWBF2 0x0060b8f5
// [thunk]:ParticleEmitterObject::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x0060b8fd
// [thunk]:ParticleEmitterObject::`scalar deleting destructor' adjustor{128}
// FUNCTION: SWBF2 0x0060bc60
ParticleEmitterObject::~ParticleEmitterObject()
{
    delete mEmitter;
    mEmitter = 0;
}

// FUNCTION: SWBF2 0x0060be10
void ParticleEmitterObject::StopAndFinish()
{
    mStopFlag = true;
    if (mEmitter)
    {
        mEmitter->StopAndFinish(mLifeTime);
    }
}

// FUNCTION: SWBF2 0x0060be40
void ParticleEmitterObject::StopNow()
{
    while (mEmitter)
    {
        mEmitter = mEmitter->DeleteAndReturnNext();
    }
}

// FUNCTION: SWBF2 0x006371c0
void ParticleEmitterObject::PermaRemoveParticleEffect()
{
    RedSceneObject::RemoveFromScene();
    mbAddedToScene = false;
}

// FUNCTION: SWBF2 0x0060c400
bool ParticleEmitterObject::IsEffectActive() const
{
    return mEmitter != 0;
}

// FUNCTION: SWBF2 0x0060c410
void ParticleEmitterObject::ActivateEffect()
{
    if (mEmitter)
    {
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x0060c420
void ParticleEmitterObject::DeactivateEffect()
{
    if (mEmitter)
    {
        StopAndFinish();
        return;
    }
    m_uiAttachFlags &= ~UPDATE_REQUIRED;
    FLEffectObject::DeactivateEffect();
}

// FUNCTION: SWBF2 0x0060c690
void ParticleEmitterObject::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    Render_(ucViewport, fDist, uiFlags);
}

// FUNCTION: SWBF2 0x0060a590
ParticleEmitter* ParticleEmitter::DeleteAndReturnNext()
{
    ParticleEmitter* pNext = mNext;
    mNext = 0;
    delete this;
    return pNext;
}

// FUNCTION: SWBF2 0x0060a5c0
bool ParticleEmitter::Update(float fTime)
{
    if (mNext && !mNext->Update(fTime))
    {
        mNext = mNext->DeleteAndReturnNext();
    }

    if (!mFirstInfo)
    {
        return true;
    }

    const float fLifeTime = mClass->mTransformer.mLifeTime;
    if (mFirstInfo->mStartTime + mSpawnDelay > mStopTime)
    {
        return false;
    }

    if (mAmbientSound.GetPlaying())
    {
        mAmbientSound.Update(&mClass->mAmbientSound, &mFirstInfo->mMatrix.GetTranslation(),
                             &mFirstInfo->mVelocity);
    }
    else if (!mSoundStarted)
    {
        mAmbientSound.Play(&mClass->mAmbientSound, &mFirstInfo->mMatrix.GetTranslation(),
                           &mFirstInfo->mVelocity, true);
        mSoundStarted = 1;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    while (fTime > pInfo->mStartTime + mSpawnDelay + fLifeTime)
    {
        PblRandom rand(mSeed);
        rand.GetInt();
        mSeed = rand.GetInt();
        mSpawnDelay += mClass->mBurstDelay.GetValue(rand);
        int iCount = (int)(mClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iCount > 128)
        {
            iCount = 128;
        }
        mParticleCount += (short)iCount;
        if (mMaxParticles != -1 && mParticleCount >= mMaxParticles)
        {
            return false;
        }

        if (mSpawnDelay > pInfo->mLifeTime)
        {
            ParticleEmitterInfoData* pNext;
            do
            {
                pNext = mFirstInfo->mNext;
                if (!pNext)
                {
                    break;
                }
                pNext->mPrev = mFirstInfo->mPrev;
                if (mFirstInfo->mPrev)
                {
                    mFirstInfo->mPrev->mNext = pNext;
                }
                mSpawnDelay -= mFirstInfo->mLifeTime;
                mFirstInfo->Release();
                mFirstInfo = pNext;
            } while (mSpawnDelay > pNext->mLifeTime);
        }
        pInfo = mFirstInfo;
    }
    return true;
}

// FUNCTION: SWBF2 0x0060bf00
void ParticleEmitterObject::RegisterStep(float fDeltaTime)
{
    if (!mEmitterInfoData)
    {
        mEmitterInfoData = ParticleEmitterInfoData::Create();
        if (!mEmitterInfoData)
        {
            return;
        }
        mEmitterInfoData->mPrev = 0;
        mEmitterInfoData->mNext = 0;
        mEmitterInfoData->mStartTime = 0.0f;
        mEmitterInfoData->mLifeTime = 0.0f;
        mEmitterInfoData->mVelocity = gVectorZero;
        mEmitterInfoData->mInitialVelocity = gVectorZero;
        mEmitterInfoData->mMatrix = m_mWorldMatrix;
        mEmitter->Register(mEmitterInfoData);
        SetRenderPos(m_mWorldMatrix.GetTranslation());
        SetRenderRadius(mEmitterBoundingRadius);
        if (!bIsEmitterEmitter && mbAddedToScene)
        {
            AddToScene();
        }
        mEmitterInfoData->Release();
        return;
    }

    ParticleEmitterInfoData* pPrev = mEmitterInfoData->mPrev;
    const PblVector3& vPos = m_mWorldMatrix.GetTranslation();
    const PblVector3& vLast = mEmitterInfoData->mMatrix.GetTranslation();
    const float fTime = mLifeTime - mEmitterInfoData->mStartTime;
    const float fDX = vPos.x - vLast.x;
    const float fDY = vPos.y - vLast.y;
    const float fDZ = vPos.z - vLast.z;
    PblVector3 vVelocity;
    if (fTime > 0.0f)
    {
        const float fInv = 1.0f / fTime;
        vVelocity = PblVector3(fDX * fInv, fDY * fInv, fDZ * fInv);
    }
    else
    {
        vVelocity = PblVector3(0.0f, 0.0f, 0.0f);
    }

    ParticleEmitterInfoData* pNew = 0;
    mRegisterTime -= fDeltaTime;
    if (!pPrev || mRegisterTime < 0.0f)
    {
        mRegisterTime = 0.1f;
        pNew = ParticleEmitterInfoData::Create();
        if (pNew)
        {
            pNew->mStartTime = mLifeTime;
            pNew->mVelocity = gVectorZero;
            bool bRegister = sForceRegisterStep || !pPrev;
            if (!bRegister)
            {
                const float fVX = vVelocity.x - pPrev->mInitialVelocity.x;
                const float fVY = vVelocity.y - pPrev->mInitialVelocity.y;
                const float fVZ = vVelocity.z - pPrev->mInitialVelocity.z;
                bRegister = fVY * fVY + fVX * fVX + fVZ * fVZ > 0.5f ||
                            (fTime == 0.0f && (fDX != 0.0f || fDY != 0.0f || fDZ != 0.0f));
            }
            if (bRegister)
            {
                pNew->mMatrix = m_mWorldMatrix;
                pNew->mLifeTime = 0.0f;
                ParticleEmitterInfoData* pCur = mEmitterInfoData;
                if (pCur->mNext)
                {
                    pCur->mNext->mPrev = pNew;
                }
                pNew->mNext = pCur->mNext;
                pNew->mPrev = pCur;
                pCur->mNext = pNew;
                mEmitterInfoData->mInitialVelocity = vVelocity;
                mEmitterInfoData->mVelocity = mEmitterInfoData->mInitialVelocity;
                mEmitterInfoData->mLifeTime = fTime;
                mEmitterInfoData = pNew;
                mEmitter->Register(pNew);
                pNew->Release();
                return;
            }
        }
    }

    if (pPrev)
    {
        pPrev->mLifeTime += fTime;
        mEmitterInfoData->mMatrix = m_mWorldMatrix;
        mEmitterInfoData->mStartTime = mLifeTime;
        const PblVector3& vNow = mEmitterInfoData->mMatrix.GetTranslation();
        const PblVector3& vThen = pPrev->mMatrix.GetTranslation();
        const float fInv = 1.0f / (pPrev->mLifeTime + EPSILON);
        pPrev->mVelocity = PblVector3((vNow.x - vThen.x) * fInv, (vNow.y - vThen.y) * fInv,
                                      (vNow.z - vThen.z) * fInv);
    }
    if (pNew)
    {
        pNew->Release();
    }
}

// FUNCTION: SWBF2 0x0060c440
bool ParticleEmitterObject::Update(float fDeltaTime)
{
    if (!mEmitter)
    {
        m_uiAttachFlags &= ~UPDATE_REQUIRED;
        FLEffectObject::DeactivateEffect();
        return false;
    }

    mLifeTime += fDeltaTime;
    if (FLEffectObject::Update(fDeltaTime))
    {
        if (!mStopFlag)
        {
            RegisterStep(fDeltaTime);
        }
        if (!mEmitter->Update(mLifeTime))
        {
            mEmitter = mEmitter->DeleteAndReturnNext();
            if (!mEmitter)
            {
                return false;
            }
        }
        if (mEmitterInfoData)
        {
            PblVector3 vMin = mEmitterInfoData->mMatrix.GetTranslation();
            PblVector3 vMax = vMin;
            for (ParticleEmitterInfoData* p = mEmitterInfoData->mPrev; p; p = p->mPrev)
            {
                vMin = Min(vMin, p->mMatrix.GetTranslation());
                vMax = Max(vMax, p->mMatrix.GetTranslation());
            }
            mMidPos = PblVector3((vMax.x + vMin.x) * 0.5f, (vMax.y + vMin.y) * 0.5f,
                                 (vMax.z + vMin.z) * 0.5f);
            mMidSize.x = (float)fabs(mMidPos.x - vMin.x);
            mMidSize.y = (float)fabs(mMidPos.y - vMin.y);
            mMidSize.z = (float)fabs(mMidPos.z - vMin.z);
            SetRenderPos(mMidPos);
            float fRadius = mMidSize.x > mMidSize.y ? mMidSize.x : mMidSize.y;
            fRadius = fRadius > mMidSize.z ? fRadius : mMidSize.z;
            SetRenderRadius(mEmitterBoundingRadius + fRadius);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0060be70
void ParticleEmitterObject::SetEmitterColor(const RedColorValue& color)
{
    if (mEmitter)
    {
        mEmitter->SetColor(color);
    }
}

// FUNCTION: SWBF2 0x0060bec0
void ParticleEmitterObject::ScaleSet(float fScale)
{
    if (mEmitter)
    {
        for (ParticleEmitter* p = mEmitter; p; p = p->mNext)
        {
            p->mSize = fScale;
        }
    }
}

// FUNCTION: SWBF2 0x0060ba00
void ParticleEmitter::SetSpawnerSeed(unsigned int uSeed)
{
    mSpawnerSeed = uSeed;
    for (ParticleEmitter* p = mNext; p; p = p->mNext)
    {
        p->mSpawnerSeed = (uSeed += 5);
    }
}

// FUNCTION: SWBF2 0x0060bd00
void ParticleEmitterObject::Reset(const ParticleEmitterClass* pClass, unsigned int uSeed)
{
    mLifeTime = 0.0f;
    if (mEmitter)
    {
        mEmitter->Reset(pClass);
    }
    else
    {
        mEmitter = ParticleEmitter::Create(pClass);
    }
    mEmitterInfoData = 0;
    mRegisterTime = 0.0f;
    mStopFlag = false;
    mIsChildPEO = false;
    if (mEmitter)
    {
        mEmitter->SetSpawnerSeed(uSeed);
        mEmitterBoundingRadius = pClass->mBoundingRadius;
        _pLodData = mEmitter->mClass->mLodData;
    }
    m_uiAttachFlags |= UPDATE_REQUIRED;
    bIsEmitterEmitter = true;
}

// FUNCTION: SWBF2 0x0060be00
void ParticleEmitterObject::SetChildFlag()
{
    mIsChildPEO = true;
}

// FUNCTION: SWBF2 0x0060a860
void ParticleEmitter::Render(float fTime)
{
    if (mClass->mGeometry.mType == ParticleGeometry::PT_EMITTER)
    {
        RenderAsEmitter(fTime);
        return;
    }
    if (!mFirstInfo)
    {
        return;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    unsigned int uSeed = mSeed;
    float fDelay = mSpawnDelay;
    short sCount = mParticleCount;

    PblMatrix mat = pInfo->mMatrix;
    mat.m[3][0] += pInfo->mVelocity.x * fDelay;
    mat.m[3][1] += pInfo->mVelocity.y * fDelay;
    mat.m[3][2] += pInfo->mVelocity.z * fDelay;
    mat.m[3][3] = 1.0f;
    while (fDelay > pInfo->mLifeTime && pInfo->mNext)
    {
        fDelay -= pInfo->mLifeTime;
        pInfo = pInfo->mNext;
        mat = pInfo->mMatrix;
        mat.m[3][0] += pInfo->mVelocity.x * fDelay;
        mat.m[3][1] += pInfo->mVelocity.y * fDelay;
        mat.m[3][2] += pInfo->mVelocity.z * fDelay;
        mat.m[3][3] = 1.0f;
    }

    PblMatrix matFirst;
    if (mClass->mNoRegisterStep)
    {
        ParticleEmitterInfoData* pLast = mFirstInfo;
        while (pLast->mNext)
        {
            pLast = pLast->mNext;
        }
        matFirst = pLast->mMatrix;
    }
    else
    {
        matFirst = mat;
    }

    mClass->mGeometry.Begin();
    int iNum = 0;
    bool bLastBurst = false;
    for (;;)
    {
        const float fStart = pInfo->mStartTime + fDelay;
        if (fTime < fStart || !(mStopTime > fStart))
        {
            break;
        }
        if (mMaxParticles != -1 && sCount >= mMaxParticles)
        {
            break;
        }
        if (iNum >= 127)
        {
            break;
        }

        PblRandom rand(uSeed);
        rand.GetInt();
        uSeed = rand.GetInt();
        const float fBurstDelay = mClass->mBurstDelay.GetValue(rand);
        int iBurst = (int)(mClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iBurst > 128)
        {
            iBurst = 128;
        }
        if (mMaxParticles != -1 && mMaxParticles - sCount < iBurst)
        {
            iBurst = mMaxParticles - sCount;
        }

        unsigned int uParticleSeed = uSeed;
        PblRandom spawnRand(mSpawnerSeed ^ 0xC71C2830u);
        for (int i = 0; i < iBurst; ++i, ++sCount)
        {
            if (iNum >= 127)
            {
                break;
            }
            PblRandom particleRand(uParticleSeed);
            particleRand.GetInt();
            uParticleSeed = particleRand.GetInt();
            if (mClass->mGeometry.mType != ParticleGeometry::PT_STREAK && !ParticleSystem::IsLodActive(sCount & 3))
            {
                continue;
            }

            Particle& particle = sParticleCache[iNum];
            const float fAge = fTime - (pInfo->mStartTime + fDelay);
            if (mClass->mNoRegisterStep)
            {
                mat = matFirst;
            }
            if (mClass->mWorldAlignedFlag)
            {
                for (int r = 0; r < 3; ++r)
                {
                    mat.m[r][0] = _IdentityMatrix.m[r][0];
                    mat.m[r][1] = _IdentityMatrix.m[r][1];
                    mat.m[r][2] = _IdentityMatrix.m[r][2];
                    mat.m[r][3] = 0.0f;
                }
            }
            mClass->mSpawner.SetupParticle(particle, mat, spawnRand.GetInt(), sCount, fAge, mSize, pInfo->mVelocity, mInheritVelocityFactor, mClass->mNoRegisterStep);
            particle.mInfoData = pInfo;
            sParticleCache[i].mFrameIndex = mClass->mGeometry.mType == ParticleGeometry::PT_ANIMATED ? 0 : -1;
            if (mClass->mGeometry.mType == ParticleGeometry::PT_STREAK)
            {
                particle.mRotation[0] *= fTime;
            }
            const float fBorn = pInfo->mStartTime + fDelay;
            if (mClass->mSpawner.mFadeInTime > fBorn)
            {
                const float fFade = fBorn / mClass->mSpawner.mFadeInTime;
                for (int k = 0; k < mClass->mSpawner.mNumColors; ++k)
                {
                    particle.mColor[k].a *= fFade;
                }
            }
            ++iNum;
        }
        if (bLastBurst)
        {
            break;
        }

        fDelay += fBurstDelay;
        mat.m[3][0] += pInfo->mVelocity.x * fBurstDelay;
        mat.m[3][1] += pInfo->mVelocity.y * fBurstDelay;
        mat.m[3][2] += pInfo->mVelocity.z * fBurstDelay;
        mat.m[3][3] = 1.0f;
        while (fDelay > pInfo->mLifeTime && pInfo->mNext)
        {
            fDelay -= pInfo->mLifeTime;
            pInfo = pInfo->mNext;
            mat = pInfo->mMatrix;
            mat.m[3][0] += pInfo->mVelocity.x * fDelay;
            mat.m[3][1] += pInfo->mVelocity.y * fDelay;
            mat.m[3][2] += pInfo->mVelocity.z * fDelay;
            mat.m[3][3] = 1.0f;
        }
        if (fDelay > pInfo->mLifeTime && !pInfo->mNext)
        {
            bLastBurst = true;
            fDelay = 0.0f;
            mat = pInfo->mMatrix;
            mat.m[3][3] = 1.0f;
        }
    }

    for (int i = 0; i < iNum; ++i)
    {
        Particle& particle = sParticleCache[i];
        mClass->mTransformer.Update(particle, mSize);
        if (mClass->mGeometry.mType == ParticleGeometry::PT_ANIMATED)
        {
            const int iTotal = (int)mClass->mGeometry.mTotalFrames;
            const int iFrame = (int)(particle.mLifeTime / mClass->mGeometry.mTimePerFrame);
            if (mClass->mGeometry.mLooping)
            {
                particle.mFrameIndex = (char)(iFrame % iTotal);
            }
            else
            {
                particle.mFrameIndex = iFrame < iTotal ? (char)iFrame : -1;
            }
        }
        else
        {
            particle.mFrameIndex = -1;
        }
    }

    for (int i = 0; i < iNum; ++i)
    {
        for (int k = 0; k < mClass->mSpawner.mNumColors; ++k)
        {
            RedColorValue& color = sParticleCache[i].mColor[k];
            color.r *= mColor.r;
            color.g *= mColor.g;
            color.b *= mColor.b;
            color.a *= mColor.a;
        }
    }

    if (mClass->mGeometry.mType != ParticleGeometry::PT_STREAK || iNum >= 3)
    {
        for (int i = 0; i < iNum; ++i)
        {
            mClass->mGeometry.Render(sParticleCache[i], i == iNum - 1);
        }
    }
}

// FUNCTION: SWBF2 0x0060b0f0
void ParticleEmitter::RenderAsEmitter(float fTime)
{
    if (!mFirstInfo)
    {
        return;
    }

    ParticleEmitterInfoData* pInfo = mFirstInfo;
    unsigned int uSeed = mSeed;
    float fDelay = mSpawnDelay;
    short sCount = mParticleCount;

    PblMatrix mat = pInfo->mMatrix;
    mat.m[3][0] += pInfo->mVelocity.x * fDelay;
    mat.m[3][1] += pInfo->mVelocity.y * fDelay;
    mat.m[3][2] += pInfo->mVelocity.z * fDelay;
    mat.m[3][3] = 1.0f;
    while (fDelay > pInfo->mLifeTime && pInfo->mNext)
    {
        fDelay -= pInfo->mLifeTime;
        pInfo = pInfo->mNext;
        mat = pInfo->mMatrix;
        mat.m[3][0] += pInfo->mVelocity.x * fDelay;
        mat.m[3][1] += pInfo->mVelocity.y * fDelay;
        mat.m[3][2] += pInfo->mVelocity.z * fDelay;
        mat.m[3][3] = 1.0f;
    }

    PblMatrix matFirst;
    if (mClass->mNoRegisterStep)
    {
        ParticleEmitterInfoData* pLast = mFirstInfo;
        while (pLast->mNext)
        {
            pLast = pLast->mNext;
        }
        matFirst = pLast->mMatrix;
    }

    mClass->mGeometry.Begin();
    int iNum = 0;
    for (;;)
    {
        const float fStart = pInfo->mStartTime + fDelay;
        if (fTime < fStart)
        {
            break;
        }
        if (mMaxParticles != -1 && sCount >= mMaxParticles)
        {
            break;
        }
        if (!(mStopTime > fStart) || sCount >= 31)
        {
            break;
        }

        PblRandom rand(uSeed);
        rand.GetInt();
        uSeed = rand.GetInt();
        const float fBurstDelay = mClass->mBurstDelay.GetValue(rand);
        int iBurst = (int)(mClass->mBurstCount.GetValue(rand) + 0.5f);
        if (iBurst > 128)
        {
            iBurst = 128;
        }
        if (mMaxParticles != -1 && mMaxParticles - sCount < iBurst)
        {
            iBurst = mMaxParticles - sCount;
        }

        unsigned int uParticleSeed = uSeed;
        PblRandom spawnRand(mSpawnerSeed ^ 0xC71C2830u);
        for (int i = 0; i < iBurst && sCount < 31; ++i)
        {
            if (iNum >= 128)
            {
                continue;
            }
            PblRandom particleRand(uParticleSeed);
            particleRand.GetInt();
            uParticleSeed = particleRand.GetInt();
            const float fAge = fTime - (pInfo->mStartTime + fDelay);
            if (mClass->mNoRegisterStep)
            {
                mat = matFirst;
            }
            if (mClass->mWorldAlignedFlag)
            {
                for (int r = 0; r < 3; ++r)
                {
                    mat.m[r][0] = _IdentityMatrix.m[r][0];
                    mat.m[r][1] = _IdentityMatrix.m[r][1];
                    mat.m[r][2] = _IdentityMatrix.m[r][2];
                    mat.m[r][3] = 0.0f;
                }
            }
            if (!mClass->mGeometry.mEmitterClass)
            {
                return;
            }
            mClass->mGeometry.mEmitterClass->SetNextSeed(uParticleSeed);

            ParticleEmitterObject* pChild = &sPEO;
            pChild->Reset(mClass->mGeometry.mEmitterClass, mSpawnerSeed + sCount * 10);
            pChild->SetChildFlag();

            Particle particle;
            Particle step;
            float fStepTime = 0.0f;
            float fStepDelta = 0.3f;
            mClass->mSpawner.SetupParticle(particle, mat, spawnRand.GetInt(), sCount, fAge, mSize, pInfo->mVelocity, mInheritVelocityFactor, mClass->mNoRegisterStep);
            particle.mInfoData = pInfo;
            if (fAge >= 0.0f)
            {
                do
                {
                    step = particle;
                    step.mLifeTime = fStepTime;
                    mClass->mTransformer.Update(step, mSize);
                    pChild->AttachEffectToVector(&step.mPosition, false);
                    ParticleEmitterObject::sForceRegisterStep = true;
                    pChild->Update(fStepDelta > fStepTime ? fStepTime : fStepDelta);
                    ParticleEmitterObject::sForceRegisterStep = false;
                    if (fStepTime == fAge)
                    {
                        break;
                    }
                    fStepTime += 0.3f;
                    if (fStepTime > fAge)
                    {
                        fStepDelta -= fStepTime - fAge;
                        fStepTime = fAge;
                    }
                } while (fAge >= fStepTime);
            }

            for (int k = 0; k < mClass->mSpawner.mNumColors; ++k)
            {
                RedColorValue& color = step.mColor[k];
                color.r *= mColor.r;
                color.g *= mColor.g;
                color.b *= mColor.b;
                color.a *= mColor.a;
            }
            pChild->SetEmitterColor(step.mColor[0]);
            pChild->Render_(0, 1.0f, 0);
            ++iNum;
            ++sCount;
        }

        fDelay += fBurstDelay;
        mat.m[3][0] += pInfo->mVelocity.x * fBurstDelay;
        mat.m[3][1] += pInfo->mVelocity.y * fBurstDelay;
        mat.m[3][2] += pInfo->mVelocity.z * fBurstDelay;
        mat.m[3][3] = 1.0f;
        while (fDelay > pInfo->mLifeTime && pInfo->mNext)
        {
            fDelay -= pInfo->mLifeTime;
            pInfo = pInfo->mNext;
            mat = pInfo->mMatrix;
            mat.m[3][0] += pInfo->mVelocity.x * fDelay;
            mat.m[3][1] += pInfo->mVelocity.y * fDelay;
            mat.m[3][2] += pInfo->mVelocity.z * fDelay;
            mat.m[3][3] = 1.0f;
        }
    }
}

// FUNCTION: SWBF2 0x0060c6b0
void ParticleEmitterObject::Render_(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!mEmitter || RedParticleRenderer::IsFull())
    {
        return;
    }

    const PblVector3 vOld = m_mWorldMatrix.GetTranslation();
    InterpolateWorldMatrix();
    const PblVector3 vNew = m_mWorldMatrix.GetTranslation();
    for (ParticleEmitterInfoData* p = mEmitter->mFirstInfo; p; p = p->mNext)
    {
        p->mMatrix.m[3][0] += vNew.x - vOld.x;
        p->mMatrix.m[3][1] += vNew.y - vOld.y;
        p->mMatrix.m[3][2] += vNew.z - vOld.z;
        p->mMatrix.m[3][3] = 1.0f;
    }

    if (!mIsChildPEO)
    {
        const PblVector3 vMin(mMidPos.x - mMidSize.x - 450.0f, mMidPos.y - mMidSize.y - 450.0f,
                              mMidPos.z - mMidSize.z - 450.0f);
        const PblVector3 vMax(mMidSize.x + mMidPos.x + 450.0f, mMidSize.y + mMidPos.y + 450.0f,
                              mMidSize.z + mMidPos.z + 450.0f);
        if (!ParticleSystem::PrepareForRender(mEmitter, GetRenderPos(), ucViewport, fDist, uiFlags, _Sphere.w, vMin,
                                              vMax, GetLodData()->mMaxDist))
        {
            return;
        }
    }

    for (ParticleEmitter* pEmitter = mEmitter; pEmitter; pEmitter = pEmitter->mNext)
    {
        pEmitter->Render(mLifeTime);
    }

    if (!mIsChildPEO)
    {
        ParticleSystem::FlushParticleCache();
        RedParticleRenderer::RenderAll();
    }
}

// FUNCTION: SWBF2 0x0060be90
void ParticleEmitterObject::ScaleMultiply(float fScale)
{
    if (mEmitter)
    {
        mEmitter->ScaleMultiply(fScale);
    }
}

// FUNCTION: SWBF2 0x0060bee0
void ParticleEmitterObject::SetInheritVelocity(float fVelocity)
{
    if (mEmitter)
    {
        mEmitter->SetInheritVelocityFactor(fVelocity);
    }
}

// FUNCTION: SWBF2 0x0060c8d0
void ParticleEmitterObject::DeleteEmitter()
{
    delete mEmitter;
    mEmitter = 0;
}
