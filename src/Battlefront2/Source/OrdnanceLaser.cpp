#include "OrdnanceLaser.h"

#include <stdio.h>
#include <math.h>
#include <d3dx9math.h>

#include "Character.h"
#include "CollisionManager.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "PebbleFL/Common/PblVector.h"

extern bool netIsLateTurn;

// GLOBAL: SWBF2 0x01ef2648
// SYNTHETIC: SWBF2 0x00405b10
// `dynamic initializer for 'OrdnanceLaserClass::rttiHashOrdnanceLaserClass''
PblHash OrdnanceLaserClass::rttiHashOrdnanceLaserClass("OrdnanceLaserClass");
char OrdnanceLaserClass::rttiNameOrdnanceLaserClass[] = "OrdnanceLaserClass";

// FUNCTION: SWBF2 0x00601b40
OrdnanceLaserClass::OrdnanceLaserClass(unsigned int uHash)
    : OrdnanceBulletClass(uHash)
{
    mLifeSpan = 2.0f;
    mModel = 0;
    mVelocity = 150.0f;

    mLaserWidth = 0.0f;
    mLaserLength = 0.0f;
    mTextureId = 0;
    mGlowColor = RedColor::WHITE;
    mEndGlowColor = RedColor::WHITE;
    mGlowLength = 0.0f;
    mBlurLength = 0.0f;
    mFadeOutSpeed = 0.0f;
}

// FUNCTION: SWBF2 0x00601c00
OrdnanceLaserClass::OrdnanceLaserClass(OrdnanceLaserClass* pBase, unsigned int uHash)
    : OrdnanceBulletClass(pBase, uHash)
{
    mLaserWidth = pBase->mLaserWidth;
    mLaserLength = pBase->mLaserLength;
    mTextureId = pBase->mTextureId;
    mGlowColor = pBase->mGlowColor;
    mEndGlowColor = pBase->mEndGlowColor;
    mGlowLength = pBase->mGlowLength;
    mBlurLength = pBase->mBlurLength;
    mFadeOutSpeed = pBase->mFadeOutSpeed;
}

// FUNCTION: SWBF2 0x00601180
bool OrdnanceLaserClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceLaserClass || OrdnanceBulletClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006011b0
unsigned int OrdnanceLaserClass::GetDerivedRtti()
{
    return rttiHashOrdnanceLaserClass;
}

// FUNCTION: SWBF2 0x006011c0
const char* OrdnanceLaserClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceLaserClass;
}

static const RedColor kRedBoltColor   = { 0xff, 0x0a, 0x0a, 0x64 };
static const RedColor kGreenBoltColor = { 0x64, 0xff, 0x64, 0x50 };
static const RedColor kBlueBoltColor  = { 0x0a, 0x0a, 0x7f, 0x64 };

static inline float ClampLength(float f)
{
    return f < 128.0f ? f : 128.0f;
}

// FUNCTION: SWBF2 0x00601cb0
void OrdnanceLaserClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x30580110:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        RedColor color;
        color.b = (unsigned char)b;
        color.g = (unsigned char)g;
        color.r = (unsigned char)r;
        color.a = (unsigned char)a;
        mGlowColor = color;
        break;
    }
    case 0x266F54F7:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        RedColor color;
        color.b = (unsigned char)b;
        color.g = (unsigned char)g;
        color.r = (unsigned char)r;
        color.a = (unsigned char)a;
        mEndGlowColor = color;
        break;
    }
    case 0x3F30370C:
        switch (PblHash(pszValue))
        {
        case 0xDD9D59EC:
            mLaserWidth = 0.35f;
            mLaserLength = 12.0f;
            mTextureId = 0x5AE97AEA;
            mGlowColor = kRedBoltColor;
            break;
        case 0x5BFD7D80:
            mLaserWidth = 0.1f;
            mLaserLength = 5.0f;
            mTextureId = 0x5AE97AEA;
            mGlowColor = kRedBoltColor;
            break;
        case 0x09D3224C:
            mLaserWidth = 0.35f;
            mLaserLength = 12.0f;
            mTextureId = 0xD89A2A5A;
            mGlowColor = kGreenBoltColor;
            break;
        case 0x8AB31EE0:
            mLaserWidth = 0.2f;
            mLaserLength = 10.0f;
            mTextureId = 0xD89A2A5A;
            mGlowColor = kGreenBoltColor;
            break;
        case 0xD6578027:
            mLaserWidth = 0.35f;
            mLaserLength = 12.0f;
            mTextureId = 0xD89A2A5A;
            mGlowColor = kBlueBoltColor;
            break;
        case 0x4A4A69FF:
            mLaserWidth = 0.2f;
            mLaserLength = 10.0f;
            mTextureId = 0xD89A2A5A;
            mGlowColor = kBlueBoltColor;
            break;
        }
        mGlowLength = ClampLength(mLaserLength * 3.0f);
        mBlurLength = ClampLength(mLaserLength * 15.0f);
        break;
    case 0x6F179AB8:
        sscanf(pszValue, "%f", &mLaserWidth);
        break;
    case 0x798DC484:
        sscanf(pszValue, "%f", &mFadeOutSpeed);
        if (mFadeOutSpeed > 0.0f)
        {
            mFadeOutSpeed = 1.0f / mFadeOutSpeed;
        }
        else
        {
            mFadeOutSpeed = 0.0f;
        }
        break;
    case 0xB709B5E7:
        mTextureId = PblHash(pszValue);
        break;
    case 0x9D0FB7A4:
        sscanf(pszValue, "%f", &mBlurLength);
        mBlurLength = ClampLength(mBlurLength);
        break;
    case 0xBA858240:
        sscanf(pszValue, "%f", &mLaserLength);
        mLaserLength = ClampLength(mLaserLength);
        mGlowLength = ClampLength(mLaserLength * 3.0f);
        mBlurLength = ClampLength(mLaserLength * 15.0f);
        break;
    case 0xC3A8B510:
        sscanf(pszValue, "%f", &mGlowLength);
        mGlowLength = ClampLength(mGlowLength);
        break;
    default:
        OrdnanceBulletClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00602040
OrdnanceClass* OrdnanceLaserClass::Derive(unsigned int uHash)
{
    return new OrdnanceLaserClass(this, uHash);
}

// FUNCTION: SWBF2 0x006020b0
Ordnance* OrdnanceLaserClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }
    Character* pCharacter = desc.mDamageOwner.mCharacter;
    if ((!pCharacter || pCharacter->mPlayerId < 0) &&
        Ordnance::sMemoryPool.mUsed + 20 > Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceLaser* pLaser = new OrdnanceLaser(this, desc);
    pLaser->ActivateThread(-40, PER_LOCAL_TURN);
    pLaser->AddOrdnanceToCollisionManager();
    return pLaser;
}

// GLOBAL: SWBF2 0x01ef264c
// SYNTHETIC: SWBF2 0x00405b00
// `dynamic initializer for 'OrdnanceLaser::rttiHashOrdnanceLaser''
PblHash OrdnanceLaser::rttiHashOrdnanceLaser("OrdnanceLaser");
char OrdnanceLaser::rttiNameOrdnanceLaser[] = "OrdnanceLaser";

static inline float ChargeLerp(float fValue, float fNext, float fRatio)
{
    return (fNext - fValue) * fRatio + fValue;
}

// FUNCTION: SWBF2 0x006011d0
OrdnanceLaser::OrdnanceLaser(OrdnanceLaserClass* pClass, const OrdnanceDesc& desc)
    : OrdnanceBullet(pClass, desc)
{
    mStartPos = desc.mPosition;
    mGrowLength = 0.0f;
    mFadeOutTime = 1.0f;
    mDeadFlag = false;

    const OrdnanceLaserClass* pLaser = (const OrdnanceLaserClass*)mClass;
    const OrdnanceLaserClass* pNext = (const OrdnanceLaserClass*)pLaser->mNextCharge;
    float fMax = ChargeLerp(pLaser->mLaserLength, pNext->mLaserLength, mRatio);
    const float fGlow = ChargeLerp(pLaser->mGlowLength, pNext->mGlowLength, mRatio);
    if (fMax <= fGlow)
    {
        fMax = fGlow;
    }
    const float fBlur = ChargeLerp(pLaser->mBlurLength, pNext->mBlurLength, mRatio);
    if (fMax <= fBlur)
    {
        fMax = fBlur;
    }
    mMaxLength = fMax;
    SetRenderRadius(mMaxLength * 2.0f);
}

// SYNTHETIC: SWBF2 0x00602165
// [thunk]:OrdnanceLaser::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x00602170
// [thunk]:OrdnanceLaser::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00601150
// OrdnanceLaser::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006012f0
OrdnanceLaser::~OrdnanceLaser()
{
}

// FUNCTION: SWBF2 0x00601100
bool OrdnanceLaser::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceLaser || OrdnanceBullet::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00601130
unsigned int OrdnanceLaser::GetDerivedRtti() const
{
    return rttiHashOrdnanceLaser;
}

// FUNCTION: SWBF2 0x00601140
const char* OrdnanceLaser::GetDerivedRttiName() const
{
    return rttiNameOrdnanceLaser;
}

// FUNCTION: SWBF2 0x00601310
void OrdnanceLaser::KillOrdnance()
{
    mDeadFlag = true;
    const float dx = mPosition.x - mStartPos.x;
    const float dy = mPosition.y - mStartPos.y;
    const float dz = mPosition.z - mStartPos.z;
    float fLength = sqrtf(dx * dx + dy * dy + dz * dz);
    if (mMaxLength <= fLength)
    {
        fLength = mMaxLength;
    }
    mGrowLength = fLength;
    CollisionManager::RemoveOrdnance(this);
}

// FUNCTION: SWBF2 0x006013a0
bool OrdnanceLaser::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    const OrdnanceLaserClass* pClass = (const OrdnanceLaserClass*)mClass;
    if (mDeadFlag)
    {
        if (pClass->mFadeOutSpeed <= 0.0f)
        {
            return false;
        }
        mFadeOutTime -= pClass->mFadeOutSpeed * fDeltaTime;
        return mFadeOutTime > 0.0f;
    }

    if (!OrdnanceBullet::Update(fDeltaTime))
    {
        KillOrdnance();
        return true;
    }

    float fLength;
    if (mFinalCollisionOnNextUpdate)
    {
        const float dx = mPosition.x - mStartPos.x;
        const float dy = mPosition.y - mStartPos.y;
        const float dz = mPosition.z - mStartPos.z;
        fLength = sqrtf(dx * dx + dy * dy + dz * dz);
    }
    else
    {
        fLength = pClass->mVelocity * fDeltaTime + mGrowLength;
    }
    if (mMaxLength <= fLength)
    {
        fLength = mMaxLength;
    }
    mGrowLength = fLength;
    return true;
}

// FUNCTION: SWBF2 0x006014c0
void OrdnanceLaser::Render(unsigned char, float, unsigned int)
{
    const OrdnanceLaserClass* pClass = (const OrdnanceLaserClass*)mClass;
    const OrdnanceLaserClass* pNext = (const OrdnanceLaserClass*)pClass->mNextCharge;

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);

    const float fRatio = mRatio;
    float fLaserLength = ChargeLerp(pClass->mLaserLength, pNext->mLaserLength, fRatio);
    float fGlowLength = ChargeLerp(pClass->mGlowLength, pNext->mGlowLength, fRatio);
    const float fLaserWidth = ChargeLerp(pClass->mLaserWidth, pNext->mLaserWidth, fRatio);

    RedColor color;
    color.b = (unsigned char)(int)(pClass->mGlowColor.b * (1.0f - fRatio) + pNext->mGlowColor.b * fRatio + 0.5f);
    color.g = (unsigned char)(int)(pClass->mGlowColor.g * (1.0f - fRatio) + pNext->mGlowColor.g * fRatio + 0.5f);
    color.r = (unsigned char)(int)(pClass->mGlowColor.r * (1.0f - fRatio) + pNext->mGlowColor.r * fRatio + 0.5f);
    color.a = (unsigned char)(int)(pClass->mGlowColor.a * (1.0f - fRatio) + pNext->mGlowColor.a * fRatio + 0.5f);

    if (pClass->mFadeColorLight)
    {
        const float fElapsed = mLifeElapsed - pClass->mDamageTransitionDelay;
        if (fElapsed > 0.0f)
        {
            float t = 1.0f;
            if (pClass->mDamageTransitionPeriod > 0.0f)
            {
                t = fElapsed / pClass->mDamageTransitionPeriod;
                if (t < 0.0f)
                {
                    t = 0.0f;
                }
                else if (t > 1.0f)
                {
                    t = 1.0f;
                }
            }
            color.b = (unsigned char)(int)(pClass->mEndGlowColor.b * t + color.b * (1.0f - t) + 0.5f);
            color.g = (unsigned char)(int)(pClass->mEndGlowColor.g * t + color.g * (1.0f - t) + 0.5f);
            color.r = (unsigned char)(int)(pClass->mEndGlowColor.r * t + color.r * (1.0f - t) + 0.5f);
            color.a = (unsigned char)(int)(pClass->mEndGlowColor.a * t + color.a * (1.0f - t) + 0.5f);
        }
    }

    if (mFadeOutTime < 1.0f)
    {
        color.r = (unsigned char)(int)(color.r * mFadeOutTime);
        color.g = (unsigned char)(int)(color.g * mFadeOutTime);
        color.b = (unsigned char)(int)(color.b * mFadeOutTime);
        color.a = (unsigned char)(int)(color.a * mFadeOutTime);
    }

    float fWidth = (mFadeOutTime * 0.9f + 0.1f) * fLaserWidth;
    if (mGrowLength <= fLaserLength)
    {
        fLaserLength = mGrowLength;
    }
    if (mGrowLength <= fGlowLength)
    {
        fGlowLength = mGrowLength;
    }

    const float fRadius = (fWidth * 3.0f + mGrowLength) * 2.0f;
    const PblVector3 vMax(mPosition.x + fRadius, mPosition.y + fRadius, mPosition.z + fRadius);
    const PblVector3 vMin(mPosition.x - fRadius, mPosition.y - fRadius, mPosition.z - fRadius);
    RedParticleRenderer::SubmitBounds(vMin, vMax);

    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;

    if (fLaserLength >= 0.0f)
    {
        RedColor laserColor = mRenderColor;
        laserColor.a = (unsigned char)(int)(mRenderColor.a * mFadeOutTime);
        RedParticleRenderer::SubmitTexture(pClass->mTextureId, BLEND_NORMAL, 0);
        RedParticleRenderer::SubmitParticle(PIT_LASER_START, mPosition, laserColor, fWidth, 1.0f,
                                            texCoords, gVectorUnitX, gVectorUnitY);
        const PblVector3 vTail(mPosition.x - vDir.x * fLaserLength, mPosition.y - vDir.y * fLaserLength,
                               mPosition.z - vDir.z * fLaserLength);
        RedParticleRenderer::SubmitParticle(PIT_LASER_END, vTail, laserColor, fWidth * 0.5f, 1.0f,
                                            texCoords, gVectorUnitX, gVectorUnitY);
    }

    if (fGlowLength >= 0.0f)
    {

        const RedColor glowColor = color * mRenderColor;
        RedParticleRenderer::SubmitTexture(0x716BD81A, BLEND_ADDITIVE, 0);
        fWidth *= 2.0f;
        RedParticleRenderer::SubmitParticle(PIT_LASER_START, mPosition, glowColor, fWidth, 1.0f,
                                            texCoords, gVectorUnitX, gVectorUnitY);
        const PblVector3 vTail(mPosition.x - vDir.x * fGlowLength, mPosition.y - vDir.y * fGlowLength,
                               mPosition.z - vDir.z * fGlowLength);
        RedParticleRenderer::SubmitParticle(PIT_LASER_END, vTail, glowColor, fWidth * 0.8f, 1.0f,
                                            texCoords, gVectorUnitX, gVectorUnitY);
    }
}

// SYNTHETIC: SWBF2 0x00601bd0
// OrdnanceLaserClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00601c90
OrdnanceLaserClass::~OrdnanceLaserClass()
{
}
