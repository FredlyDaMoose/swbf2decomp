#include "ParticleGeometry.h"

#include "RedEngineFL/Graphics/RedModel.h"
#include "Particle.h"
#include "ParticleEmitter.h"
#include "ParticleSystem.h"
#include "PebbleFL/Common/PblMath.h"
#include <math.h>

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
namespace ParticleSystem { PblBox& GetBoundingBox(); }
namespace ParticleSystem { float GetSizeMultiplier(); }
namespace ParticleSystem { float GetMinParticleSize(); }
namespace ParticleSystem { float GetLodAlpha(int iLod); }
namespace ParticleSystem { void SetupParticleCache(unsigned int uTexture, RedBlendMode eBlend, float fBlurValue, int iBlurParam); }
namespace ParticleSystem { void CacheParticle(const PblVector3& vPos, const RedColorValue& color, float fSize, float fRotation); }

// GLOBAL: SWBF2 0x01eaed64
int ParticleGeometry::mParticleCount;

// FUNCTION: SWBF2 0x0060c930
ParticleGeometry::ParticleGeometry()
{
    mTextureHash = 0;
    mTextureWidthLog = 0;
    mTextureHeightLog = 0;
    mBlendMode = BLEND_NORMAL;
    mType = PT_PARTICLE;
    mEmitterClass = 0;
    mBlurValue = 0.0f;
    mBlurParam = 0;
    mBlurRes = 0.0f;
    mSparkLength = 0.0f;
    mFrameSizeLog = 5.0f;
    mTimePerFrame = 2.0f;
    mTotalFrames = 4.0f;
    mLooping = true;
}

// FUNCTION: SWBF2 0x0060c900
void ParticleGeometry::Setup()
{
    if (mType == PT_GEOMETRY)
    {
        m_pModel = RedModel::_HashTable.Find(mTextureHash);
    }
}

// FUNCTION: SWBF2 0x0060c9a0
void HSVtoRGB(float& r, float& g, float& b, float h, float s, float v)
{
    const float fSector = h * 6.0f;
    if (s == 0.0f)
    {
        b = v;
        g = v;
        r = v;
        return;
    }
    const int iSector = (int)fSector;
    const float f = fSector - (float)iSector;
    const float p = (1.0f - s) * v;
    const float q = (1.0f - f * s) * v;
    const float t = (1.0f - (1.0f - f) * s) * v;
    switch (iSector)
    {
    case 0:
        r = v;
        g = t;
        b = p;
        break;
    case 1:
        r = q;
        g = v;
        b = p;
        break;
    case 2:
        r = p;
        g = v;
        b = t;
        break;
    case 3:
        r = p;
        g = q;
        b = v;
        break;
    case 4:
        r = t;
        g = p;
        b = v;
        break;
    default:
        r = v;
        g = p;
        b = q;
        break;
    }
}

// FUNCTION: SWBF2 0x0060cab0
void ParticleGeometry::Render(Particle& particle, bool bLast) const
{
    const ParticleGeometry::ParticleEmitterType eType = mType;
    if (eType != ParticleGeometry::PT_STREAK && !(particle.mColor[0].a > 0.0f))
    {
        return;
    }
    if (eType == ParticleGeometry::PT_PARTICLE && ParticleSystem::GetMinParticleSize() > particle.mSize[0])
    {
        return;
    }

    ParticleInfoType eInfo;
    switch (eType)
    {
    case ParticleGeometry::PT_STREAK:
        eInfo = bLast ? PIT_STREAK_TAIL : PIT_STREAK;
        break;
    case ParticleGeometry::PT_ANIMATED:
        eInfo = PIT_ANIMATED;
        break;
    case ParticleGeometry::PT_GEOMETRY:
        eInfo = PIT_GEOMETRY;
        break;
    case ParticleGeometry::PT_BILLBOARD:
        eInfo = PIT_FLAT;
        break;
    default:
        eInfo = PIT_PARTICLE;
        break;
    }

    if (mFirstTime)
    {
        if (eInfo != PIT_GEOMETRY)
        {
            if (eType == ParticleGeometry::PT_PARTICLE)
            {
                ParticleSystem::SetupParticleCache(mTextureHash, mBlendMode, mBlurValue,
                           mBlurParam);
            }
            else
            {
                ParticleSystem::FlushParticleCache();
                RedParticleRenderer::SubmitTexture(mTextureHash, mBlendMode, 0);
                RedParticleRenderer::SetBlurParams(mBlurValue, mBlurParam);
            }
        }
        mFirstTime = false;
        if (eInfo == PIT_STREAK)
        {
            eInfo = PIT_STREAK_HEAD;
        }
        else if (eInfo == PIT_STREAK_TAIL)
        {
            return;
        }
    }

    RedColorValue color;
    color.r = particle.mColor[0].r;
    color.g = particle.mColor[0].g;
    color.b = particle.mColor[0].b;
    const float fFade = 1.0f - particle.mColor[0].a;
    color.a = (1.0f - fFade * fFade + particle.mColor[0].a) * 0.5f;
    if (mBlendMode == BLEND_BLUR)
    {
        color.a *= 2.0f;
        if (!(1.0f > color.a))
        {
            color.a = 1.0f;
        }
    }
    if (eType != ParticleGeometry::PT_STREAK)
    {
        color.a *= ParticleSystem::GetLodAlpha(particle.mLod);
    }
    if (particle.mUseHSV)
    {
        HSVtoRGB(color.r, color.g, color.b, color.r, color.g, color.b);
    }

    const PblBox& box = ParticleSystem::GetBoundingBox();
    const PblVector3& vPos = particle.mPosition;
    if (vPos.x < box._vMin.x || box._vMax.x < vPos.x)
    {
        return;
    }
    if (vPos.y < box._vMin.y || box._vMax.y < vPos.y)
    {
        return;
    }
    if (vPos.z < box._vMin.z || box._vMax.z < vPos.z)
    {
        return;
    }

    RedColor redColor;
    if (mType == ParticleGeometry::PT_SPARK)
    {
        const float fSizeMultiplier = ParticleSystem::GetSizeMultiplier();
        const float fSize = particle.mSize[0] * fSizeMultiplier;
        const int iFrame = particle.mFrameIndex;
        const float fLength = mSparkLength * fSizeMultiplier;
        const PblVector3 vStreak(particle.mVelocity.x * fLength, particle.mVelocity.y * fLength,
                                 particle.mVelocity.z * fLength);
        RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        if (iFrame >= 0)
        {
            const int iFrameSizeLog = (int)mFrameSizeLog;
            const int iUBits = mTextureWidthLog - iFrameSizeLog;
            const int iVBits = mTextureHeightLog - iFrameSizeLog;
            texCoords.u0 = (iFrame & ((1 << iUBits) - 1)) << (8 - iUBits);
            texCoords.v0 = (iFrame & (((1 << iVBits) - 1) << iUBits)) << (8 - iVBits - iUBits);
            texCoords.u1 = texCoords.u0 + (1 << (8 - iUBits)) - 1;
            texCoords.v1 = texCoords.v0 + (1 << (8 - iVBits)) - 1;
        }
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        const PblVector3 vHead(vPos.x - vStreak.x, vPos.y - vStreak.y, vPos.z - vStreak.z);
        RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vHead, redColor, fSize, 0.0f, texCoords, gVectorUnitX,
                                            gVectorUnitY);
        const PblVector3 vMid(vPos.x - vStreak.x * 0.5f, vPos.y - vStreak.y * 0.5f, vPos.z - vStreak.z * 0.5f);
        RedParticleRenderer::SubmitParticle(PIT_STREAK, vMid, redColor, fSize, 0.5f, texCoords, gVectorUnitX,
                                            gVectorUnitY);
        RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vPos, redColor, fSize, 1.0f, texCoords, gVectorUnitX,
                                            gVectorUnitY);
        ++ParticleGeometry::mParticleCount;
        return;
    }

    if (eInfo == PIT_GEOMETRY)
    {
        if (!m_pModel)
        {
            return;
        }

        const float fCos = (float)cos((double)particle.mRotation[0]);
        const float fSin = (float)sin((double)particle.mRotation[0]);
        const float fSize = particle.mSize[0];

        PblMatrix mLocal;
        mLocal.m[0][0] = fSize * fCos;
        mLocal.m[0][1] = 0.0f;
        mLocal.m[0][2] = -(fSize * fSin);
        mLocal.m[0][3] = 0.0f;
        mLocal.m[1][0] = 0.0f;
        mLocal.m[1][1] = fSize;
        mLocal.m[1][2] = 0.0f;
        mLocal.m[1][3] = 0.0f;
        mLocal.m[2][0] = fSize * fSin;
        mLocal.m[2][1] = 0.0f;
        mLocal.m[2][2] = fSize * fCos;
        mLocal.m[2][3] = 0.0f;
        mLocal.m[3][0] = 0.0f;
        mLocal.m[3][1] = 0.0f;
        mLocal.m[3][2] = 0.0f;
        mLocal.m[3][3] = 1.0f;
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, &mLocal, &particle.mInfoData->mMatrix);
        mWorld.m[3][0] = vPos.x;
        mWorld.m[3][1] = vPos.y;
        mWorld.m[3][2] = vPos.z;
        mWorld.m[3][3] = 1.0f;
        unsigned int uiFlags = 1;
        if (mBlendMode == BLEND_ADDITIVE)
        {
            uiFlags = 0x800001;
        }
        else if (mBlendMode == BLEND_BLUR)
        {
            uiFlags = 0x41;
        }
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        m_pModel->Render(&mWorld, 0, redColor, uiFlags, 0);
        ++ParticleGeometry::mParticleCount;
        return;
    }

    switch (mType)
    {
    case ParticleGeometry::PT_PARTICLE:
        ParticleSystem::CacheParticle(vPos, color, ParticleSystem::GetSizeMultiplier() * particle.mSize[0], particle.mRotation[0]);
        break;

    case ParticleGeometry::PT_ANIMATED:
    {
        const int iFrame = particle.mFrameIndex;
        if (iFrame >= 0)
        {
            const int iFrameSizeLog = (int)mFrameSizeLog;
            const int iUBits = mTextureWidthLog - iFrameSizeLog;
            const int iVBits = mTextureHeightLog - iFrameSizeLog;
            RedParticleRenderer::TexCoords texCoords;
            texCoords.u0 = (iFrame & ((1 << iUBits) - 1)) << (8 - iUBits);
            texCoords.v0 = (iFrame & (((1 << iVBits) - 1) << iUBits)) << (8 - iVBits - iUBits);
            texCoords.u1 = texCoords.u0 + (1 << (8 - iUBits)) - 1;
            texCoords.v1 = texCoords.v0 + (1 << (8 - iVBits)) - 1;
            redColor.b = (unsigned char)(int)(color.b * 255.0f);
            redColor.g = (unsigned char)(int)(color.g * 255.0f);
            redColor.r = (unsigned char)(int)(color.r * 255.0f);
            redColor.a = (unsigned char)(int)(color.a * 255.0f);
            RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0],
                                                particle.mRotation[0], texCoords, gVectorUnitX, gVectorUnitY);
        }
        break;
    }

    case ParticleGeometry::PT_BILLBOARD:
    {
        const float fRotation = particle.mRotation[0];
        const int iFace = (int)fRotation;
        PblVector3 vRight(0.0f, 0.0f, 0.0f);
        PblVector3 vUp(0.0f, 0.0f, 0.0f);
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, (fRotation - (float)iFace) * 6.2831855f);
        switch (iFace)
        {
        case 0:
            vUp.y = fSin;
            vRight.x = 1.0f;
            vUp.z = fCos;
            break;
        case 1:
            vUp.x = fSin;
            vRight.y = 1.0f;
            vUp.z = fCos;
            break;
        case 2:
            vUp.x = fCos;
            vRight.z = 1.0f;
            vUp.y = fSin;
            break;
        case 3:
            vRight.x = fSin;
            vRight.y = fCos;
            vUp.x = fCos;
            vUp.y = -fSin;
            break;
        case 4:
            vRight.x = fSin;
            vRight.z = fCos;
            vUp.x = fCos;
            vUp.z = -fSin;
            break;
        default:
            vRight.y = fSin;
            vRight.z = fCos;
            vUp.z = -fSin;
            vUp.y = fCos;
            break;
        }
        if (particle.mNoRegisterStep)
        {
            PblVector3 v;
            D3DXVec3TransformNormal(&v, &vRight, &particle.mInfoData->mMatrix);
            vRight = v;
            D3DXVec3TransformNormal(&v, &vUp, &particle.mInfoData->mMatrix);
            vUp = v;
        }
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        const RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0], 0.0f, texCoords,
                                            vRight, vUp);
        break;
    }

    default:
    {
        redColor.b = (unsigned char)(int)(color.b * 255.0f);
        redColor.g = (unsigned char)(int)(color.g * 255.0f);
        redColor.r = (unsigned char)(int)(color.r * 255.0f);
        redColor.a = (unsigned char)(int)(color.a * 255.0f);
        const RedParticleRenderer::TexCoords texCoords = { 0, 0, 255, 255 };
        RedParticleRenderer::SubmitParticle(eInfo, vPos, redColor, ParticleSystem::GetSizeMultiplier() * particle.mSize[0],
                                            particle.mRotation[0], texCoords, gVectorUnitX, gVectorUnitY);
        break;
    }
    }
    ++ParticleGeometry::mParticleCount;
}

// FUNCTION: SWBF2 0x0060d590
void ParticleGeometry::Begin() const
{
    mFirstTime = true;
}
