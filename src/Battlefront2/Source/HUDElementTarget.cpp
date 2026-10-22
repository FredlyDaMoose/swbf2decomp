#include <float.h>
#include <stdio.h>

#include <math.h>

#include "HUDElementTarget.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "EntityRemoteTerminal.h"
#include "GameLoop.h"
#include "TargetManager.h"
#include "Team.h"
#include "PebbleFL/Common/PblMath.h"

 

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

static const unsigned int kMinScaleHash = 0x4d3600d7;
static const unsigned int kMaxScaleHash = 0xb43f812d;
static const unsigned int kColorInterpHash = 0x937114c6;
static const unsigned int kColorFriendlyHash = 0x70141317;
static const unsigned int kFourSegmentLockOnHash = 0x6703febc;
static const unsigned int kEventResetTargetCommonHash = 0x92c1cee6;
static const unsigned int kEventResetTargetPlayerHash = 0x9d34215e;
static const unsigned int kBitmapHash = 0x46544626;

// GLOBAL: SWBF2 0x00789b10

char* HUD::ElementTarget::sTargetCommonStateStrings[targetCommonState_MAX] =
{
    "ObjectiveOnScreen", "ObjectiveOnScreenBehind", "ObjectiveOffScreen",
    "ObjectiveOffScreenBehind", "FlagOnScreen", "FlagOnScreenBehind", "FlagOffScreen",
    "FlagOffScreenBehind"
};

// GLOBAL: SWBF2 0x00789b30

char* HUD::ElementTarget::sTargetPlayerStateStrings[targetPlayerState_MAX] =
{
    "AttackerOnScreen", "AttackerOnScreenBehind", "AttackerOffScreen",
    "AttackerOffScreenBehind", "LockedOnScreen", "LockedOnScreenBehind", "LockedOffScreen",
    "LockedOffScreenBehind", "HintOnScreen", "HintOnScreenBehind", "HintOffScreen",
    "HintOffScreenBehind", "HintFlagOnScreen", "HintFlagOnScreenBehind", "HintFlagOffScreen",
    "HintFlagOffScreenBehind"
};

// GLOBAL: SWBF2 0x007ec220

float HUD::ElementTarget::sLockOnRotation[4] =
{
    0.0f, 1.5707964f, 3.1415927f, 4.712389f
};

// FUNCTION: SWBF2 0x00558b80
HUD::Item* HUD::ElementTarget::Target::Factory::CreateItem(const char* const pName,
                                                          ItemCallback pCallback,
                                                          void* const pUserData)
{
    HUD::Item* pItem = new Target(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00558c00
HUD::Item* HUD::ElementTarget::Factory::CreateItem(const char* const pName,
                                                   ItemCallback pCallback, void* const pUserData)
{
    HUD::Item* pItem = new ElementTarget(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00558c80
void* HUD::ElementTarget::GetParent()
{
    return this;
}

// FUNCTION: SWBF2 0x00558c90
HUD::ElementTarget::Target::Factory::Factory()
    : HUD::ElementBitmapBase::Factory(0xCCD2D3D9u)
{
    mName = "TargetIcon";

    mMinScale.Init("MinScale", mProperties, 0.0f, FLT_MAX, 0.1f);
    mMaxScale.Init("MaxScale", mProperties, 0.0f, FLT_MAX, 0.1f);
    mColorInterpolationRate.Init("ColorInterp", mProperties, 0.0f, 1.0f, 0.01f);
}

// FUNCTION: SWBF2 0x00558de0
HUD::ElementTarget::Target::Target(const char* const pName, Factory::ItemCallback pCallback,
                                   void* const pUserData)
    : HUD::ElementBitmap(pName, pCallback, pUserData)
{
    mMinScale = 2.0f;
    mMaxScale = 0.1f;
    mColorInterpolationRate = 0.5f;
    mColorFriendly.b = 0xff;
    mColorFriendly.g = 0;
    mColorFriendly.r = 0;
    mColorFriendly.a = 0;
    mDynamicAlloc = false;
    mWriteEnabled = false;
}

// SYNTHETIC: SWBF2 0x00558ea0
// HUD::ElementTarget::Target::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055acc8
// [thunk]:HUD::ElementTarget::Target::`scalar deleting destructor' adjustor{288}
// FUNCTION: SWBF2 0x00558ed0
HUD::ElementTarget::Target::~Target()
{
}

// FUNCTION: SWBF2 0x00558f30
void HUD::ElementTarget::Target::Update(float fDeltaTime, HUD::ElementBitmap& bitmap,
                                        const LockOnManager* const pManager,
                                        const LockOnManager::Target& target,
                                        const LockOnManager::TargetInfo& info, float fWidth,
                                        float fHeight, bool bStateChanged,
                                        HUD::ElementBitmap* const pLockOn)
{
    const PblVector3 vPos = info.mPosition;
    const float fDist = vPos.z;

    const GameObject* pObject = target.obj;
    const bool bShow = pObject && target.maxDistTrack > fDist && info.mPositionValid;
    const bool bEnabled = bitmap.RedInterfaceElement::IsEnabled();
    if (bShow != bEnabled || bStateChanged)
    {
        if (bShow || bStateChanged)
        {
            RedColor color;
            if (mColorFriendly.a > 0)
            {
                color = info.mFriendly ? mColorFriendly : m_color;
            }
            else
            {
                color = bEnabled ? bitmap.m_color : m_color;
            }
            bitmap = *this;
            bitmap.SetColorOnly(color);
        }
        if (pLockOn && bShow && info.mOnScreen)
        {
            bitmap.HUD::Element::Enable(false);
            for (int i = 0; i < 4; ++i)
            {
                pLockOn[i] = *this;
                pLockOn[i].HUD::Element::Enable(true);
            }
        }
        else
        {
            bitmap.HUD::Element::Enable(bShow);
        }
    }
    if (!bShow)
    {
        return;
    }

    const Character* pCharacter = pManager->mPlayerCharacter;
    bool bVehicleView;
    float fVehicleScale;
    if (info.mOnScreen && pCharacter && pCharacter->mVehicle &&
        pCharacter->mVehicle->GetGameObject() &&
        (pCharacter->mVehicle->GetGameObject()->IsRtti(EntityFlyer::rttiHashEntityFlyer) ||
         pCharacter->mVehicle->GetGameObject()->IsRtti(
             EntityRemoteTerminal::rttiHashEntityRemoteTerminal)))
    {
        bVehicleView = true;
        float fT = (fDist - 200.0f) / 1500.0f;
        if (fT < 0.0f)
        {
            fT = 0.0f;
        }
        else if (fT > 1.0f)
        {
            fT = 1.0f;
        }
        float fFarScale;
        float fNearScale;
        if (pLockOn)
        {
            fFarScale = 0.1f;
            fNearScale = 1.5f;
        }
        else
        {
            fFarScale = info.mFlagCarrier ? 0.5f : 0.25f;
            fNearScale = info.mFlagCarrier ? 0.8f : 0.7f;
        }
        const Controllable* pVehicle = pCharacter->mVehicle;
        fVehicleScale = (1.0f - fT) * fNearScale + fFarScale * fT;
        const GameObject* pTarget = target.obj;
        if (pVehicle->GetGameObject() == pTarget && info.mFlagCarrier)
        {
            fVehicleScale *= 1.6f;
        }
    }
    else
    {
        bVehicleView = false;
        fVehicleScale = 1.0f;
    }

    float fT = (fDist - target.minDist) / (target.maxDist - target.minDist);
    if (fT < 0.0f)
    {
        fT = 0.0f;
    }
    else if (fT > 1.0f)
    {
        fT = 1.0f;
    }

    bool bInterpolate = true;
    const bool bLocked = info.mLocked;
    const RedColor current = bitmap.m_color;
    RedColor color = (mColorFriendly.a > 0 && info.mFriendly) ? mColorFriendly : m_color;
    color.a = (unsigned char)(int)(mAlpha * 255.0f);
    if (color.a == 0)
    {

        const GameObject* pTeamObject = target.obj;
        color = pCharacter->mTeamPtr->mColor[pTeamObject->mTeam];
        color.a = bLocked ? 0xd0 : 0xa0;
    }

    if (bLocked && pLockOn && info.mOnScreen)
    {
        const RedColor& pulse = info.mVisible ? RedColor::WHITE : RedColor::DKGREY;
        const float fTime = fmod(GameLoop::GetMissionTime(), 1.0f);
        color.a = 0xff;
        const unsigned short uBlend = (unsigned short)(int)(fabs(fTime - 0.5f) * 2.0f * 255.0f);
        const unsigned short uInverse = (unsigned short)(255 - uBlend);
        color.r = (unsigned char)((color.r * uBlend) >> 8) + (unsigned char)((pulse.r * uInverse) >> 8);
        color.g = (unsigned char)((color.g * uBlend) >> 8) + (unsigned char)((pulse.g * uInverse) >> 8);
        bInterpolate = false;
        color.b = (unsigned char)((color.b * uBlend) >> 8) + (unsigned char)((pulse.b * uInverse) >> 8);
    }

    float fScale = bVehicleView ? fVehicleScale : (1.0f - fT) * mMinScale + mMaxScale * fT;

    if (&target >= LockOnManager::sTargetCommon &&
        &target < LockOnManager::sTargetCommon + 32)
    {
        for (unsigned int i = 0; i < TargetManager::MAX_MARKERS; ++i)
        {
            if (TargetManager::gMarkers[i].actor == target.obj)
            {
                const TargetManager::Marker* pMarker = &TargetManager::gMarkers[i];
                if (pMarker)
                {
                    const RedColor markerColor = pMarker->color;
                    const bool bPulseSize = pMarker->pulseSize && info.mOnScreen;
                    const bool bFlashAlpha = pMarker->flashAlpha;
                    const float fTime = (bPulseSize || bFlashAlpha)
                                            ? fmod(GameLoop::GetMissionTime(), 1.0f)
                                            : 1.0f;
                    if (bPulseSize)
                    {
                        fScale = (1.0f - fTime) * fScale + pMarker->pulseStartScale;
                    }
                    if (bFlashAlpha)
                    {
                        bInterpolate = false;
                        color.a = (unsigned char)(((int)((1.0f - fTime) * 255.0f) *
                                                   markerColor.a) >> 8);
                    }
                    else
                    {
                        color.a = markerColor.a;
                    }
                    color.r = markerColor.r;
                    color.g = markerColor.g;
                    color.b = markerColor.b;
                }
                break;
            }
        }
    }

    const float fX = HUD::ConvertXForWideScreen((vPos.x * 0.5f + 0.5f) * fWidth, fWidth, fHeight);
    const float fY = (vPos.y * 0.5f + 0.5f) * fHeight;
    if (bInterpolate)
    {
        const float fRate = mColorInterpolationRate * fDeltaTime;
        color.b = (unsigned char)(int)(current.b * fRate + color.b * (1.0f - fRate) + 0.5f);
        color.g = (unsigned char)(int)(current.g * fRate + color.g * (1.0f - fRate) + 0.5f);
        color.r = (unsigned char)(int)(current.r * fRate + color.r * (1.0f - fRate) + 0.5f);
        color.a = (unsigned char)(int)(current.a * fRate + color.a * (1.0f - fRate) + 0.5f);
    }
    RedBitmapElement& element = bitmap;
    element.SetColorOnly(color);

    if (!info.mOnScreen)
    {
        const float fRotation = PblMath::Atan2(vPos.x, -vPos.y);
        element.SetMatrixPosRotScale(fX, fY, fRotation, fScale, fScale);
        return;
    }

    if (pLockOn)
    {
        float fLockOnScale;
        if (bVehicleView)
        {
            if (fVehicleScale < 0.9f)
            {
                fLockOnScale = 0.9f;
            }
            else if (fVehicleScale > 1.4f)
            {
                fLockOnScale = 1.4f;
            }
            else
            {
                fLockOnScale = fVehicleScale;
            }
        }
        else
        {
            fVehicleScale = fScale;
            fLockOnScale = 1.2f;
        }
        float fLeft, fTop, fRight, fBottom;
        RedBitmapElement::GetRect(fLeft, fTop, fRight, fBottom);
        const float fHalfWidth = (fRight - fLeft) * fVehicleScale;
        const float fHalfHeight = (fBottom - fTop) * fVehicleScale;
        const float fCorners[4][2] =
        {
            { fX - fHalfWidth, fY - fHalfHeight },
            { fX + fHalfWidth, fY - fHalfHeight },
            { fX + fHalfWidth, fY + fHalfHeight },
            { fX - fHalfWidth, fY + fHalfHeight }
        };
        for (int i = 0; i < 4; ++i)
        {
            pLockOn[i].RedInterfaceElement::SetColor(bitmap.m_color);
            pLockOn[i].SetMatrixPosRotScale(fCorners[i][0], fCorners[i][1],
                                            HUD::ElementTarget::sLockOnRotation[i],
                                            fLockOnScale, fLockOnScale);
        }
        return;
    }

    float fOffset;
    if (info.mLocked && info.mFlagCarrier)
    {
        float fLeft, fTop, fRight, fBottom;
        RedBitmapElement::GetRect(fLeft, fTop, fRight, fBottom);
        fOffset = (fBottom - fTop) * fScale;
    }
    else
    {
        fOffset = 0.0f;
    }
    element.SetMatrixPosScale(fX, fY - fOffset, fScale, fScale);
}

// FUNCTION: SWBF2 0x00559b50
bool HUD::ElementTarget::Target::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    switch (uHashID)
    {
    case kMinScaleHash:
        mMinScale = *(const float*)pValue;
        return true;
    case kColorInterpHash:
        mColorInterpolationRate = *(const float*)pValue;
        return true;
    case kMaxScaleHash:
        mMaxScale = *(const float*)pValue;
        return true;
    }
    return ElementBitmap::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x00559bb0
bool HUD::ElementTarget::Target::GetProperty(const unsigned int uHashID, void* const pValue)
{
    switch (uHashID)
    {
    case kMinScaleHash:
        *(float*)pValue = mMinScale;
        return true;
    case kColorInterpHash:
        *(float*)pValue = mColorInterpolationRate;
        return true;
    case kMaxScaleHash:
        *(float*)pValue = mMaxScale;
        return true;
    }
    return ElementBitmap::GetProperty(uHashID, pValue);
}

static inline float ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        return 0.0f;
    }
    if (f > 255.0f)
    {
        return 255.0f;
    }
    return f;
}

// FUNCTION: SWBF2 0x00559c10
bool HUD::ElementTarget::Target::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kColorFriendlyHash:
        mColorFriendly.b = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(2));
        mColorFriendly.g = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(1));
        mColorFriendly.r = (unsigned char)(int)ClampColorComponent(data.GetFloatArg(0));
        mColorFriendly.a = 0xff;
        return true;
    case kMinScaleHash:
        mMinScale = data.GetFloatArg(0);
        return true;
    case kColorInterpHash:
        mColorInterpolationRate = data.GetFloatArg(0);
        return true;
    case kMaxScaleHash:
        mMaxScale = data.GetFloatArg(0);
        return true;
    }
    return ElementBitmap::ReadData(config, data);
}

// FUNCTION: SWBF2 0x00559d00
void HUD::ElementTarget::Target::WriteData(HUD::File& file, unsigned int uTabs)
{
    file.WriteProperty("ColorFriendly", uTabs, (int)mColorFriendly.r, (int)mColorFriendly.g,
                       (int)mColorFriendly.b, 255, 255, 255);
    file.WriteProperty("ColorInterp", uTabs, mColorInterpolationRate, 0.5f);
    file.WriteProperty("MinScale", uTabs, mMinScale, 2.0f);
    file.WriteProperty("MaxScale", uTabs, mMaxScale, 0.1f);
    ElementBitmap::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x00559db0
HUD::ElementTarget::Factory::Factory()
    : HUD::ElementGroupPlayer::Factory(0x32608848u)
{
    mName = "Target";

    mEventResetTargetCommon.Init("EventResetTargetCommon", mProperties, Property::type_Event);
    mEventResetTargetPlayer.Init("EventResetTargetPlayer", mProperties, Property::type_Event);
    mFourSegmentLockOn.Init("FourSegmentLockOn", mProperties, Property::type_Bool);
}

// FUNCTION: SWBF2 0x00559ec0
HUD::ElementTarget::ElementTarget(const char* const pName, Factory::ItemCallback pCallback,
                                  void* const pUserData)
    : HUD::ElementGroupPlayer(*static_cast<RedGroupElement*>(this), pName, pCallback, pUserData)
{
    mFourSegmentLockOn = false;

    const HUD::Item::Factory* pBitmapFactory = HUD::Item::Factory::FindByHashID(kBitmapHash);

    unsigned int i;
    for (i = 0; i < targetCommonState_MAX; ++i)
    {
        mTargetCommonAll[i].HUD::Item::SetName(sTargetCommonStateStrings[i]);
        mTargetCommonAll[i].HUD::Element::Enable(false);
        mTargetCommonAll[i].SetFactory(pBitmapFactory);
    }
    for (i = 0; i < targetPlayerState_MAX; ++i)
    {
        mTargetPlayerAll[i].HUD::Item::SetName(sTargetPlayerStateStrings[i]);
        mTargetPlayerAll[i].HUD::Element::Enable(false);
        mTargetPlayerAll[i].SetFactory(pBitmapFactory);
    }

    for (i = 0; i < 32; ++i)
    {
        ElementGroupBase::AddElement(&mTargetCommon[i]);
        mTargetCommon[i].mDynamicAlloc = false;
        mTargetCommon[i].SetFactory(pBitmapFactory);
        mTargetCommon[i].mWriteEnabled = false;
        mTargetCommon[i].HUD::Element::Enable(false);
        mTargetCommonState[i] = targetCommonState_OnScreen;
    }
    for (i = 0; i < 64; ++i)
    {
        ElementGroupBase::AddElement(&mTargetPlayer[i]);
        mTargetPlayer[i].mDynamicAlloc = false;
        mTargetPlayer[i].SetFactory(pBitmapFactory);
        mTargetPlayer[i].mWriteEnabled = false;
        mTargetPlayer[i].HUD::Element::Enable(false);
        mTargetPlayerState[i] = targetPlayerState_AttackerOnScreen;
    }
    for (i = 0; i < 4; ++i)
    {
        ElementGroupBase::AddElement(&mTargetLockOn[i]);
        mTargetLockOn[i].mDynamicAlloc = false;
        mTargetLockOn[i].SetFactory(pBitmapFactory);
        mTargetLockOn[i].mWriteEnabled = false;
        mTargetLockOn[i].HUD::Element::Enable(false);
    }

    mEventResetTargetCommon.Init(EventRefreshTargetCommon, this);
    mEventResetTargetPlayer.Init(EventRefreshTargetPlayer, this);
}

// SYNTHETIC: SWBF2 0x0055a1d0
// HUD::ElementTarget::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055a200
// HUD::ElementTarget::Target::`default constructor closure'

// SYNTHETIC: SWBF2 0x0055acbd
// [thunk]:HUD::ElementTarget::`scalar deleting destructor' adjustor{384}

// FUNCTION: SWBF2 0x0055a210
HUD::ElementTarget::~ElementTarget()
{
}

// FUNCTION: SWBF2 0x0055a340
void HUD::ElementTarget::Update(const float fDeltaTime)
{
    if (mPlayerIndex >= 0)
    {
        const LockOnManager* pManager = LockOnManager::GetByPlayerIndex(mPlayerIndex);
        const float fWidth = mWidth * sViewportWidth;
        const float fHeight = mHeight * sViewportHeight;

        const LockOnManager::Target* pTargets = pManager->GetPlayerTargets();
        const LockOnManager::TargetInfo* pInfos = pManager->GetPlayerTargetInfos();
        bool bAnyLocked = false;
        unsigned int i;
        for (i = 0; i < 64; ++i)
        {
            const TargetPlayerState eState = TranslatePlayerTargetInfo(pInfos[i]);
            const bool bLocked = eState == targetPlayerState_LockedOnScreen ||
                                 eState == targetPlayerState_LockedOnScreenBehind;
            bAnyLocked |= bLocked;
            mTargetPlayerAll[eState].Update(fDeltaTime, mTargetPlayer[i], pManager, pTargets[i],
                                            pInfos[i], fWidth, fHeight,
                                            eState != mTargetPlayerState[i],
                                            mFourSegmentLockOn && bLocked ? mTargetLockOn : 0);
            if (mFourSegmentLockOn && (eState == targetPlayerState_LockedOnScreen ||
                                       eState == targetPlayerState_LockedOnScreenBehind))
            {
                if (pInfos[i].mFlagCarrier)
                {
                    mTargetPlayerAll[eState == targetPlayerState_LockedOnScreen
                                         ? targetPlayerState_HintFlagOnScreen
                                         : targetPlayerState_HintFlagOnScreenBehind]
                        .Update(fDeltaTime, mTargetPlayer[i], pManager, pTargets[i], pInfos[i],
                                fWidth, fHeight, true, 0);
                }
                else
                {
                    mTargetPlayer[i].HUD::Element::Enable(false);
                }
            }
            mTargetPlayerState[i] = eState;
        }

        if (!bAnyLocked && mFourSegmentLockOn)
        {
            for (i = 0; i < 4; ++i)
            {
                mTargetLockOn[i].HUD::Element::Enable(false);
            }
        }

        const LockOnManager::Target* pCommonTargets = LockOnManager::GetCommonTargets();
        const LockOnManager::TargetInfo* pCommonInfos = pManager->GetCommonTargetInfo();
        for (i = 0; i < 32; ++i)
        {
            const TargetCommonState eState = TranslateCommonTargetInfo(pCommonInfos[i]);
            mTargetCommonAll[eState].Update(fDeltaTime, mTargetCommon[i], pManager,
                                            pCommonTargets[i], pCommonInfos[i], fWidth, fHeight,
                                            eState != mTargetCommonState[i], 0);
            mTargetCommonState[i] = eState;
        }
    }
    HUD::Element::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0055a5f0
void HUD::ElementTarget::Read(PblConfig& config, Factory::ItemCallback pCallback,
                              void* const pUserData)
{
    unsigned int i;
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            for (i = 0; i < targetCommonState_MAX; ++i)
            {
                if (data.GetId() == mTargetCommonAll[i].GetNameHash())
                {
                    break;
                }
            }
            if (i < targetCommonState_MAX)
            {
                PblConfig scope(config.ReadNextScope(), true);
                mTargetCommonAll[i].Read(scope, 0, 0);
                continue;
            }

            for (i = 0; i < targetPlayerState_MAX; ++i)
            {
                if (data.GetId() == mTargetPlayerAll[i].GetNameHash())
                {
                    break;
                }
            }
            if (i < targetPlayerState_MAX)
            {
                PblConfig scope(config.ReadNextScope(), true);
                mTargetPlayerAll[i].Read(scope, 0, 0);
                continue;
            }

            ReadChild(config, data, pCallback, pUserData);
        }
        else
        {
            ReadData(config, data);
        }
    }

    char buffer[256];
    for (i = 0; i < targetCommonState_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mTargetCommonAll[i].GetName());
        mTargetCommonAll[i].SetDisplayName(buffer);
    }
    for (i = 0; i < targetPlayerState_MAX; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mTargetPlayerAll[i].GetName());
        mTargetPlayerAll[i].SetDisplayName(buffer);
    }

    for (i = targetPlayerState_LockedOnScreen; i <= targetPlayerState_LockedOnScreenBehind; ++i)
    {
        float fLeft, fTop, fRight, fBottom;
        mTargetPlayerAll[i].GetRect(fLeft, fTop, fRight, fBottom);
        mTargetPlayerAll[i].HUD::ElementBitmapBase::SetRect(fRight - fLeft, fBottom - fTop,
                                                            RedBitmapElement::BITMAP_HALIGN_RIGHT,
                                                            RedBitmapElement::BITMAP_VALIGN_BOTTOM);
    }

    const PblVector3& vPos = RedGroupElement::GetPosition();
    float fX = vPos.x;
    if (CameraManager::Instance()->GetNumCameras() <= 1)
    {
        fX = ConvertWideScreenX(fX, sViewportWidth, sViewportHeight);
    }
    RedGroupElement::SetPosition(fX, vPos.y, vPos.z);

    PostReadSetup();
}

// FUNCTION: SWBF2 0x0055a890
bool HUD::ElementTarget::SetProperty(const unsigned int uHashID, const void* const pValue)
{
    switch (uHashID)
    {
    case kFourSegmentLockOnHash:
        mFourSegmentLockOn = *(const bool*)pValue;
        return true;
    case kEventResetTargetCommonHash:
        Item::SetEvent(*(const char* const*)pValue, mEventResetTargetCommon);
        return true;
    case kEventResetTargetPlayerHash:
        Item::SetEvent(*(const char* const*)pValue, mEventResetTargetPlayer);
        return true;
    }
    return ElementGroupPlayer::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0055a900
bool HUD::ElementTarget::GetProperty(const unsigned int uHashID, void* const pValue)
{
    switch (uHashID)
    {
    case kFourSegmentLockOnHash:
        *(bool*)pValue = mFourSegmentLockOn;
        return true;
    case kEventResetTargetCommonHash:
        *(const char**)pValue = mEventResetTargetCommon.GetRegisteredClassName(false);
        return true;
    case kEventResetTargetPlayerHash:
        *(const char**)pValue = mEventResetTargetPlayer.GetRegisteredClassName(false);
        return true;
    }
    return ElementGroupPlayer::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0055a970
bool HUD::ElementTarget::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kFourSegmentLockOnHash:
        mFourSegmentLockOn = data.GetBoolArg(0);
        return true;
    case kEventResetTargetCommonHash:
        Item::ReadEvent(data, mEventResetTargetCommon);
        return true;
    case kEventResetTargetPlayerHash:
        Item::ReadEvent(data, mEventResetTargetPlayer);
        return true;
    }
    return ElementGroupPlayer::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0055a9f0
void HUD::ElementTarget::WriteData(HUD::File& file, unsigned int uTabs)
{
    const PblVector3& vPos = RedGroupElement::GetPosition();
    float fX;
    if (CameraManager::Instance()->GetNumCameras() <= 1)
    {
        fX = ConvertXForWideScreen(vPos.x, GetContainerViewWidth(), GetContainerViewHeight());
    }
    else
    {
        fX = vPos.x;
    }
    RedGroupElement::SetPosition(fX, vPos.y, vPos.z);

    file.WriteProperty("EventResetTargetCommon", uTabs,
                       mEventResetTargetCommon.GetRegisteredClassName(true), "");
    file.WriteProperty("EventResetTargetPlayer", uTabs,
                       mEventResetTargetPlayer.GetRegisteredClassName(true), "");
    file.WriteProperty("FourSegmentLockOn", uTabs, (int)mFourSegmentLockOn, 0);
    ElementGroupPlayer::WriteData(file, uTabs);
    file.WriteString("\n");

    unsigned int i;
    for (i = 0; i < targetCommonState_MAX; ++i)
    {
        mTargetCommonAll[i].Write(file, uTabs, mTargetCommonAll[i].GetName());
    }
    for (i = 0; i < targetPlayerState_MAX; ++i)
    {
        mTargetPlayerAll[i].Write(file, uTabs, mTargetPlayerAll[i].GetName());
    }
}

// FUNCTION: SWBF2 0x0055ab30
void HUD::ElementTarget::EventRefreshTargetCommon(const HUD::Event& event, void* const pData)
{
    if (event.mClass->mType != HUD::EventClass::type_Uint)
    {
        return;
    }

    ElementTarget* pTarget = static_cast<ElementTarget*>(pData);
    const unsigned int slot = event.mData.uintValue;
    pTarget->mTargetCommonState[slot] = targetCommonState_MAX;
    pTarget->mTargetCommon[slot].RedInterfaceElement::SetColor(TargetManager::gMarkers[slot].color);
}

// FUNCTION: SWBF2 0x0055abb0
void HUD::ElementTarget::EventRefreshTargetPlayer(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() != HUD::EventClass::type_Uint)
    {
        return;
    }

    ElementTarget* pTarget = (ElementTarget*)pData;
    if (pTarget->mPlayerIndex < 0)
    {
        return;
    }

    const unsigned int uSlot = event.GetData().uintValue;
    const LockOnManager::TargetInfo& info =
        LockOnManager::GetByPlayerIndex(pTarget->mPlayerIndex)->GetPlayerTargetInfos()[uSlot];
    pTarget->mTargetPlayerState[uSlot] = targetPlayerState_MAX;
    const TargetPlayerState eState = TranslatePlayerTargetInfo(info);
    pTarget->mTargetPlayer[uSlot].RedInterfaceElement::SetColor(
        pTarget->mTargetPlayerAll[eState].m_color);
}

// FUNCTION: SWBF2 0x0055ac50
HUD::ElementTarget::TargetCommonState HUD::ElementTarget::TranslateCommonTargetInfo(
    const LockOnManager::TargetInfo& info)
{
    return (TargetCommonState)((info.mFlagCarrier ? targetCommonState_FlagOnScreen : targetCommonState_OnScreen) +
                               (info.mOnScreen ? 0 : 2) + (info.mVisible ? 0 : 1));
}

// FUNCTION: SWBF2 0x0055ac80
HUD::ElementTarget::TargetPlayerState HUD::ElementTarget::TranslatePlayerTargetInfo(
    const LockOnManager::TargetInfo& info)
{
    int iState;
    if (info.mLocked)
    {
        iState = targetPlayerState_LockedOnScreen;
    }
    else if (info.mAttacker)
    {
        iState = targetPlayerState_AttackerOnScreen;
    }
    else
    {
        iState = info.mFlagCarrier ? targetPlayerState_HintFlagOnScreen : targetPlayerState_HintOnScreen;
    }
    return (TargetPlayerState)(iState + (info.mOnScreen ? 0 : 2) + (info.mVisible ? 0 : 1));
}
