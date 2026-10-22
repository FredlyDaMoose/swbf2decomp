#include "ParticleSystem.h"

#include <float.h>

#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

struct CacheParticle
{
    PblVector3 mPos;
    RedColorValue mColor;
    float mSize;
    float mRotation;
};
 

extern CacheParticle sCachedParticles[300];

// GLOBAL: SWBF2 0x0078aa94
static bool sLodMask[4][4] = {
    { true, true, true, true },
    { true, true, true, false },
    { true, false, true, false },
    { true, false, false, false },
};
// GLOBAL: SWBF2 0x0078aaa4
static bool sLodFadeMask[4][4] = {
    { false, false, false, true },
    { false, true, false, false },
    { false, false, true, false },
    { false, false, false, false },
};

namespace ParticleSystem
{

// GLOBAL: SWBF2 0x01eb05ac

float sSmallEffectMaxDist;
// GLOBAL: SWBF2 0x01eb05cc

float sLargeEffectMaxDist;
// GLOBAL: SWBF2 0x01eb05ec

float sMediumEffectMaxDist;
// GLOBAL: SWBF2 0x01eaed90
float mFadeBias;

// FUNCTION: SWBF2 0x0060e3a0
void SetMaxDistanceBias(float fBias)
{
    sSmallEffectMaxDist = fBias + 50.0f;
    sMediumEffectMaxDist = fBias + 150.0f;
    sLargeEffectMaxDist = fBias + 500.0f;
}

// FUNCTION: SWBF2 0x0060e3e0
void SetFadeBias(float fBias)
{
    ParticleSystem::mFadeBias = fBias;
}

// FUNCTION: SWBF2 0x0060e390
void Init()
{
    SetMaxDistanceBias(0.0f);
    SetFadeBias(0.0f);
}

// GLOBAL: SWBF2 0x01eaed68
float mMinParticleSize;
// GLOBAL: SWBF2 0x01eaed6c
float mCacheBlurValue;
// GLOBAL: SWBF2 0x01eaed70
unsigned int uiRenderFlags;
// GLOBAL: SWBF2 0x01eaed74
int mCacheBlurParam;
// GLOBAL: SWBF2 0x01eaed78
float mFadeVal;
// GLOBAL: SWBF2 0x01eaed7c
unsigned int mNumCachedParticles;
// GLOBAL: SWBF2 0x01eaed80
float mSizeMultiplier;
// GLOBAL: SWBF2 0x01eaed84
int mLod;
// GLOBAL: SWBF2 0x01eaed88
float mLodFadeVal;
// GLOBAL: SWBF2 0x01eaed8c
int mCurrentLod;
// GLOBAL: SWBF2 0x01eaed94
RedBlendMode mCacheBlendMode;
// GLOBAL: SWBF2 0x01eaed98
unsigned int mCacheTexture;
// GLOBAL: SWBF2 0x01ef2718
PblBox mBoundingBox;

// FUNCTION: SWBF2 0x0060e3f0
bool PrepareForRender(ParticleEmitter* pEmitter, const PblVector3& vPos, int iUnused, float fAlpha,
                      unsigned int uiFlags, float fRadius, const PblVector3& vMin, const PblVector3& vMax,
                      float fMaxDist)
{
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    const float fDist = mCamera.m[2][0] * (mCamera.m[3][0] - vPos.x) +
                        mCamera.m[2][1] * (mCamera.m[3][1] - vPos.y) +
                        mCamera.m[2][2] * (mCamera.m[3][2] - vPos.z);
    const float fZoom = RedRenderer::GetCamera()->GetZoom();
    fMaxDist = fZoom * fMaxDist;

    if (fDist > fMaxDist * 0.8f)
    {
        fAlpha = (1.0f - (fDist - fMaxDist * 0.8f) / (fMaxDist * 0.2f)) * fAlpha;
    }

    if (!(uiFlags & 0x20000))
    {
        const float fFadeStart = FLRenderer::GetNearSceneFadeStart() * fZoom - fRadius;
        const float fFadeEnd = FLRenderer::GetNearSceneFadeEnd() * fZoom + fRadius;
        if (fDist > fFadeStart)
        {
            float fT = (fDist - fFadeStart) / (fFadeEnd - fFadeStart);
            if (fT >= 1.0f)
            {
                fT = 1.0f;
            }
            fAlpha = (1.0f - fT) * fAlpha;
        }
    }

    if (fAlpha <= 0.0f)
    {
        return false;
    }

    const float fDepth = fDist - fRadius;
    if (fMaxDist >= 90.0f)
    {
        fMaxDist = 90.0f;
    }
    mFadeVal = fAlpha;

    float fLod = (4.0f / (fMaxDist + ParticleSystem::mFadeBias - 20.0f)) * (fDepth - 20.0f);
    if (fLod <= 0.0f)
    {
        fLod = 0.0f;
    }
    else if (fLod >= 3.0f)
    {
        fLod = 3.0f;
    }
    const int iLod = (int)fLod;
    mLodFadeVal = 1.0f - (fLod - (float)iLod);
    if (fLod < 1.0f)
    {
        fLod = 1.0f;
    }
    ParticleSystem::mCurrentLod = iLod;
    mSizeMultiplier = fLod * 0.1f + 1.2f;
    mMinParticleSize = fDepth * 0.002f + 0.01f;

    RedParticleRenderer::SubmitBounds(vMin, vMax);
    mBoundingBox._vMax = vMax;
    mBoundingBox._vMin = vMin;
    mLod = iLod;
    uiRenderFlags = uiFlags;
    return true;
}

// FUNCTION: SWBF2 0x0060e760
void FlushParticleCache()
{
    if (!mNumCachedParticles)
    {
        return;
    }

    RedParticleRenderer::SubmitTexture(mCacheTexture, mCacheBlendMode, 0);
    RedParticleRenderer::SetBlurParams(mCacheBlurValue, mCacheBlurParam);

    PblHeap<unsigned int, float>::RECORD records[301];
    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
    PblHeap<unsigned int, float> heap(300, FLT_MAX, records);

    while (mNumCachedParticles)
    {
        --mNumCachedParticles;
        CacheParticle& particle = sCachedParticles[mNumCachedParticles];
        const float fDist = mCamera.m[2][0] * (mCamera.m[3][0] - particle.mPos.x) +
                            mCamera.m[2][1] * (mCamera.m[3][1] - particle.mPos.y) +
                            mCamera.m[2][2] * (mCamera.m[3][2] - particle.mPos.z) - 1.0f;
        if (fDist > 0.0f)
        {
            if (particle.mSize > fDist)
            {
                particle.mColor.a = (fDist / particle.mSize) * particle.mColor.a;
            }
            heap.Push(mNumCachedParticles, fDist);
        }
    }

    while (!heap.IsEmpty())
    {
        const unsigned int i = heap.Get(0);
        heap.Pop();
        const CacheParticle& particle = sCachedParticles[i];

        RedColor color;
        color.a = (unsigned char)(int)(particle.mColor.a * 255.0f);
        color.b = (unsigned char)(int)(particle.mColor.b * 255.0f);
        color.g = (unsigned char)(int)(particle.mColor.g * 255.0f);
        color.r = (unsigned char)(int)(particle.mColor.r * 255.0f);

        RedParticleRenderer::TexCoords texCoords;
        texCoords.u0 = 0;
        texCoords.v0 = 0;
        texCoords.u1 = 255;
        texCoords.v1 = 255;
        RedParticleRenderer::SubmitParticle(PIT_PARTICLE, particle.mPos, color, particle.mSize,
                                            particle.mRotation, texCoords, gVectorUnitX, gVectorUnitY);
    }
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e380
PblBox& GetBoundingBox()
{
    return ParticleSystem::mBoundingBox;
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e660
float GetSizeMultiplier()
{
    return ParticleSystem::mSizeMultiplier;
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e670
float GetMinParticleSize()
{
    return ParticleSystem::mMinParticleSize;
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e680
bool IsLodActive(int iLod)
{
    return sLodMask[ParticleSystem::mCurrentLod][iLod];
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e690
float GetLodAlpha(int iLod)
{
    float fAlpha = ParticleSystem::mFadeVal;
    if (sLodFadeMask[ParticleSystem::mCurrentLod][iLod])
    {
        fAlpha *= ParticleSystem::mLodFadeVal;
    }
    return fAlpha;
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e6b0
void SetupParticleCache(unsigned int uTexture, RedBlendMode eBlend, float fBlurValue, int iBlurParam)
{
    if (uTexture != ParticleSystem::mCacheTexture || eBlend != ParticleSystem::mCacheBlendMode)
    {
        ParticleSystem::FlushParticleCache();
    }
    ParticleSystem::mCacheTexture = uTexture;
    ParticleSystem::mCacheBlendMode = eBlend;
    ParticleSystem::mCacheBlurValue = fBlurValue;
    ParticleSystem::mCacheBlurParam = iBlurParam;
}

}

namespace ParticleSystem {

// FUNCTION: SWBF2 0x0060e700
void CacheParticle(const PblVector3& vPos, const RedColorValue& color, float fSize, float fRotation)
{
    if (ParticleSystem::mNumCachedParticles < 300)
    {
        ::CacheParticle& particle = sCachedParticles[ParticleSystem::mNumCachedParticles++];
        particle.mPos = vPos;
        particle.mColor = color;
        particle.mSize = fSize;
        particle.mRotation = fRotation;
    }
}

}

// TEMPLATE: SWBF2 0x0060ea00
// PblHeap<unsigned int,float>::Pop
template void PblHeap<unsigned int, float>::Pop();

// SYNTHETIC: SWBF2 0x0060e370
// CacheParticle::CacheParticle

// GLOBAL: SWBF2 0x01ef5120
// SYNTHETIC: SWBF2 0x00405d00
// `dynamic initializer for 'sCachedParticles''
CacheParticle sCachedParticles[300];
