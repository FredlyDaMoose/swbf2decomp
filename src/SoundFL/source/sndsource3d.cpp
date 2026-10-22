#include <math.h>

#include "sndsource3d.h"
#include "sndsource.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblMath.h"
#include "sndenginebase.h"

extern "C" ::PblVector3* __stdcall D3DXVec3Normalize(::PblVector3* pOut,
                                                     const ::PblVector3* pV);

namespace Snd
{

float Source3D::sPitchMultiplier = 1.0f;

static const float fLength2EstAlpha = 0.94805944f;
static const float fLength2EstBeta = 0.39269909f;

static float Length2Est(float fX, float fY, float fZ)
{

    float fA = (float)fabs(fX);
    float fB = (float)fabs(fY);
    float fXY = (fA > fB) ? (fA * fLength2EstAlpha + fB * fLength2EstBeta)
                          : (fA * fLength2EstBeta + fB * fLength2EstAlpha);

    float fC = (float)fabs(fXY);
    float fD = (float)fabs(fZ);
    return (fC > fD) ? (fC * fLength2EstAlpha + fD * fLength2EstBeta)
                     : (fC * fLength2EstBeta + fD * fLength2EstAlpha);
}

// FUNCTION: SWBF2 0x0073afd0
Source3D::Properties::Properties()
{
    mRollInFactor = 0.0f;
    mRollInDistance = 0.0f;
    mMinDistance = 0.0f;
    mMuteDistance = 1.0f;
    mMaxDistance = 1.0f;
    mRollOffFactor = 0.0f;

    mOffCentreGain = 0.6f;
    mSpaceDistance = 0.0f;
    mFlags = (unsigned char)flags_dopplerMode;
}

// FUNCTION: SWBF2 0x0073b010
Source3D::Source3D(Properties* pProps)
{
    mProps = pProps;
    mClosestListener = 0;
    mDistanceLength = 0.0f;
    mPos3D = &Globals::smNullVector;
    mVel3D = &Globals::smNullVector;
    mPrevAngle = 0.0f;
    mFlags = (unsigned char)flags_updateListener;
}

// FUNCTION: SWBF2 0x0073b050
void Source3D::CalculateGain(float& rfGain) const
{
    const Properties* pProps = mProps;
    float fDistance = mDistanceLength;

    if (pProps->mRollInDistance > fDistance)
    {
        float fOffset = pProps->mRollInDistance - fDistance;
        rfGain = (pProps->mRollInDistance - fOffset)
               / (pProps->mRollOffFactor * mClosestListener->mRollOffFactor
                    * fOffset
                  + pProps->mRollInDistance);
    }

    else if (pProps->mMinDistance >= fDistance)
    {
        rfGain = 1.0f;
    }

    else if (fDistance >= pProps->mMuteDistance)
    {
        rfGain = 0.0f;
    }
    else
    {
        float fOffset = fDistance - pProps->mMinDistance;
        float fRange = pProps->mMaxDistance - pProps->mMinDistance;
        rfGain = (fRange - fOffset)
               / (pProps->mRollOffFactor * mClosestListener->mRollOffFactor
                    * fOffset
                  + fRange);
    }

    if (!(mProps->mFlags & Properties::flags_centrePeakMode))
    {
        return;
    }

    ::PblVector3 vDirection;
    CalculateNormalizedDirection(vDirection);

    const Listener* pListener = mClosestListener;

    float fRight = pListener->mRight.x * vDirection.x
                 + pListener->mRight.y * vDirection.y
                 + pListener->mRight.z * vDirection.z;
    float fCone = 1.0f - (float)fabs(fRight) * 2.0f;

    float fFront = pListener->mFront->x * vDirection.x
                 + pListener->mFront->y * vDirection.y
                 + pListener->mFront->z * vDirection.z;
    fCone *= -fFront;

    if (mProps->mOffCentreGain > fCone)
    {
        fCone = mProps->mOffCentreGain;
    }

    rfGain *= fCone;
}

// FUNCTION: SWBF2 0x0073b1d0
void Source3D::CalculateGain(float fGain, float fReverbGain, GainMatrix& rMatrix) const
{
    ::PblVector3 vDirection = mDistance;
    float fDistanceGain;
    CalculateGain(fDistanceGain);

    if (fDistanceGain == 0.0f)
    {
        rMatrix.frontLeft = 0.0f;
        rMatrix.frontRight = 0.0f;
        rMatrix.rearLeft = 0.0f;
        rMatrix.rearRight = 0.0f;
        rMatrix.reverb = fReverbGain;
        return;
    }

    float fRight;
    float fBack;
    if (EngineBase::smSpeakerConfig == EngineBase::speakerConfig_Mono)
    {
        fRight = 0.5f;
        fBack = 0.5f;
    }
    else
    {
        CalculateNormalizedDirection(vDirection);

        const ::PblVector3 vRight = mClosestListener->mRight;
        float fDot = vDirection.x * vRight.x + vDirection.y * vRight.y + vDirection.z * vRight.z;
        if (fDot < -1.0f)
        {
            fDot = -1.0f;
        }
        else if (fDot > 1.0f)
        {
            fDot = 1.0f;
        }

        float fLeft = 0.5f - fDot * 0.5f;
        if (fLeft < 0.0f)
        {
            fLeft = 0.0f;
        }
        else if (fLeft > 1.0f)
        {
            fLeft = 1.0f;
        }
        fRight = 1.0f - fLeft;

        const ::PblVector3 vFront = *mClosestListener->mFront;
        fBack = 0.5f - (vFront.x * vDirection.x * 0.5f + vFront.y * vDirection.y * 0.5f
                        + vFront.z * vDirection.z * 0.5f);
        if (fBack < 0.0f)
        {
            fBack = 0.0f;
        }
        else if (fBack > 1.0f)
        {
            fBack = 1.0f;
        }
    }

    const float fTotal = fDistanceGain * fGain;
    rMatrix.reverb = fReverbGain;

    switch (EngineBase::smSpeakerConfig)
    {
    case EngineBase::speakerConfig_Mono:
        rMatrix.frontLeft = fTotal;
        rMatrix.frontRight = fTotal;
        return;

    case EngineBase::speakerConfig_Headphones:
        fRight = fRight * 0.5f + 0.25f;

    case EngineBase::speakerConfig_Stereo:
    case EngineBase::speakerConfig_Quadraphonic:
    case EngineBase::speakerConfig_5point1:
    case EngineBase::speakerConfig_7point1:
    {
        const float fLeft = 1.0f - fRight;
        rMatrix.frontRight = fRight * fTotal;
        rMatrix.rearRight = fRight * fTotal;
        rMatrix.frontLeft = fLeft * fTotal;
        rMatrix.rearLeft = fLeft * fTotal;
        return;
    }

    case EngineBase::speakerConfig_AnalogSurround:
    {
        const float fX = (float)fabs((double)(fBack - 0.5f));
        const float fY = (float)fabs((double)(fRight - 0.5f));
        float fAngle = (float)asin((double)(fY / (float)sqrt((double)(fY * fY + fX * fX))));

        if (fRight < 0.5f)
        {
            if (fBack < 0.5f)
            {
                fAngle += PblMath::fPi;
            }
            else
            {
                fAngle = (PblMath::fHalfPi - fAngle) + (PblMath::fPi + PblMath::fHalfPi);
            }
        }
        else if (fBack < 0.5f)
        {
            fAngle = (PblMath::fHalfPi - fAngle) + PblMath::fHalfPi;
        }

        if ((float)fabs((double)(fAngle - mPrevAngle)) > PblMath::fPi)
        {
            if (mFlags & 8)
            {
                mFlags &= ~8;
            }
            else
            {
                mFlags |= 8;
            }
        }
        mPrevAngle = fAngle;

        const float fPan = fAngle * 0.5f + PblMath::fHalfPi * 0.5f;
        rMatrix.frontLeft = PblMath::Sin(fPan + PblMath::fHalfPi) * fTotal;
        rMatrix.frontRight = PblMath::Sin(fPan) * fTotal;
        if (mFlags & 8)
        {
            rMatrix.frontLeft = -rMatrix.frontLeft;
            rMatrix.frontRight = -rMatrix.frontRight;
        }
        rMatrix.rearLeft = 0.0f;
        rMatrix.rearRight = 0.0f;
        break;
    }

    default:
        return;
    }
}

// FUNCTION: SWBF2 0x0073b590
void Source3D::CalculateNormalizedDirection(::PblVector3& rvDirection) const
{
    if (0.0f == mDistance.x && 0.0f == mDistance.y && 0.0f == mDistance.z)
    {
        rvDirection = *mClosestListener->mFront;
        return;
    }

    ::PblVector3 vNormal;

    if (EngineBase::smSpeakerConfig == EngineBase::speakerConfig_AnalogSurround)
    {
        D3DXVec3Normalize(&vNormal, &mDistance);
    }
    else
    {
        float fLength = Length2Est(mDistance.x, mDistance.y, mDistance.z);

        if (fLength == 0.0f)
        {
            vNormal = mDistance;
        }
        else
        {

            float fScale = 1.0f / fLength;
            vNormal.x = mDistance.x * fScale;
            vNormal.y = mDistance.y * fScale;
            vNormal.z = mDistance.z * fScale;
        }
    }

    rvDirection = vNormal;
}

// FUNCTION: SWBF2 0x0073b6f0
float Source3D::CalculatePitch() const
{

    if (!(mProps->mFlags & Properties::flags_dopplerMode))
    {
        return 1.0f;
    }

    const Listener* pListener = mClosestListener;

    float fNextX = (mPos3D->x - mVel3D->x)
                 - (pListener->mPos->x - pListener->mVel->x);
    float fNextY = (mPos3D->y - mVel3D->y)
                 - (pListener->mPos->y - pListener->mVel->y);
    float fNextZ = (mPos3D->z - mVel3D->z)
                 - (pListener->mPos->z - pListener->mVel->z);
    float fNext = Length2Est(fNextX, fNextY, fNextZ);

    float fNow = Length2Est(mPos3D->x - pListener->mPos->x,
                            mPos3D->y - pListener->mPos->y,
                            mPos3D->z - pListener->mPos->z);

    if (172.0f <= fNext - fNow)
    {
        return 1.0f;
    }

    float fSpeed = pListener->mDistanceFactor * 344.0f;
    float fClosing = (fNow - fNext) * pListener->mDopplerFactor;

    return (fSpeed - fClosing) / fSpeed * sPitchMultiplier;
}

// FUNCTION: SWBF2 0x0073b8a0
void Source3D::Properties::CopyParameters(const Properties& rOther)
{
    const bool bCentrePeak = (mFlags & flags_centrePeakMode) != 0;
    mRollInFactor = rOther.mRollInFactor;
    mRollInDistance = rOther.mRollInDistance;
    mMinDistance = rOther.mMinDistance;
    mMuteDistance = rOther.mMuteDistance;
    mMaxDistance = rOther.mMaxDistance;
    mRollOffFactor = rOther.mRollOffFactor;
    mSpaceDistance = rOther.mSpaceDistance;
    mFlags = rOther.mFlags;
    if (bCentrePeak)
    {
        mFlags |= flags_centrePeakMode;
    }
    else
    {
        mFlags &= (unsigned char)~(unsigned int)flags_centrePeakMode;
    }
}

// FUNCTION: SWBF2 0x0073b900
void Source3D::CopyParameters(const Source3D& rOther)
{
    mClosestListener = rOther.mClosestListener;
    mDistance = rOther.mDistance;
    mDistanceLength = rOther.mDistanceLength;
    mPos3D = rOther.mPos3D;
    mVel3D = rOther.mVel3D;
    mFlags |= (unsigned char)((rOther.mFlags & ~8u) | flags_updateListener);
    if (mProps && rOther.mProps)
    {
        mProps->CopyParameters(*rOther.mProps);
    }
}

// FUNCTION: SWBF2 0x0073b960
void Source3D::DetermineClosestListener()
{
    if ((mFlags & flags_updateListener) || mClosestListener == 0)
    {
        mClosestListener = static_cast<Listener*>(
            ListenerBase::GetClosestListener(mPos3D, &mDistance,
                                             &mDistanceLength));
    }
}

// FUNCTION: SWBF2 0x0073b990
bool Source3D::Properties::Read(PblFileChunk& chunk, unsigned int uiPropertyID)
{
    PblFile* const pFile = chunk._pFile;
    switch (uiPropertyID)
    {
    case 0x1CE8C2E1u:
        pFile->Read8();
        SetRollIn(pFile->ReadFloat());
        return true;
    case 0x0FBA43F2u:
        pFile->Read8();
        SetDopplerMode(pFile->ReadFloat() > 0.0f);
        return true;
    case 0x47C87922u:
        pFile->Read8();
        mSpaceDistance = pFile->ReadFloat();
        return true;
    case 0x80A163FEu:
        pFile->Read8();
        SetRollInDistance(pFile->ReadFloat());
        return true;
    case 0xAB59D4BBu:
        pFile->Read8();
        SetRollOff(pFile->ReadFloat());
        return true;
    case 0xB835CF50u:
        pFile->Read8();
        SetMinDistance(pFile->ReadFloat());
        return true;
    case 0xE4B12EDFu:
        pFile->Read8();
        SetMuteDistance(pFile->ReadFloat());
        return true;
    case 0xE6915CF6u:
        pFile->Read8();
        SetMaxDistance(pFile->ReadFloat());
        return true;
    }
    return false;
}

}
