#include "ToolTips.h"
#include "CameraManager.h"
#include "NetGame.h"
#include "PlayerProfile.h"
#include "HUDEvent.h"
#include "HUDGameEvents.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"

// FUNCTION: SWBF2 0x00658560
void ToolTipsDisplay::SetAlwaysOff()
{
    mAlwaysOn = false;
    mAlwaysOff = true;
    for (unsigned int i = 0; i < CameraManager::sInstance->GetNumCameras(); ++i)
    {
        HUD::Event event(HUD::GameEvents::gPlayerEvents[i].disableToolTips, false);
        event.Send();
    }
}

// GLOBAL: SWBF2 0x01e56032
bool gAttractMode;

namespace PlayerPrefs { int __fastcall FindByViewport(int iCamera); }

// GLOBAL: SWBF2 0x007e9ac0
TipClass sTipData[TTT_COUNT + 1] =
{
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0x00000000u },
    { 0.5f, 10.0f, 8.0f, 60.0f, 15.0f, 2.0f, 4, 128, 0xF25A2919u },
    { 0.5f, 28.0f, 10.0f, 60.0f, 5.0f, 2.0f, 2, 128, 0x85ACEFCBu },
    { 0.5f, 2.0f, 10.0f, 60.0f, 5.0f, 6.0f, 1, 128, 0xB1E55713u },
    { 0.5f, 40.0f, 10.0f, 120.0f, 5.0f, 6.0f, 2, 128, 0x595C091Au },
    { 0.5f, 0.0f, 2.0f, 1.0f, 1.0f, 0.0f, 255, 129, 0xB946B816u },
    { 0.5f, 10.0f, 10.0f, 60.0f, 5.0f, 2.0f, 2, 128, 0x7FBF4CA3u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0xFA611CDAu },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 10.0f, 4, 128, 0x6BDE2E1Eu },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x64BCC6D2u },
    { 0.5f, 1.0f, 8.0f, 10.0f, 5.0f, 2.0f, 4, 128, 0x7F5DC765u },
    { 0.5f, 1.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x053D95E9u },
    { 0.5f, 0.0f, 8.0f, 2.0f, 1.0f, 2.0f, 4, 128, 0xAF053ED5u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x47EA53D7u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x2D6BC8B7u },
    { 2.0f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x26F400E8u },
    { 2.0f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x65218E7Cu },
    { 2.0f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0xA7192EDCu },
    { 0.5f, 5.0f, 8.0f, 40.0f, 6.0f, 2.0f, 1, 128, 0x05190EE9u },
    { 0.5f, 6.0f, 8.0f, 40.0f, 6.0f, 2.0f, 2, 128, 0x0CD0DCF2u },
    { 0.5f, 7.0f, 8.0f, 40.0f, 6.0f, 2.0f, 4, 128, 0xDEA48E20u },
    { 0.5f, 8.0f, 8.0f, 40.0f, 20.0f, 2.0f, 4, 128, 0x6613607Du },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x59183222u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x54182A43u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x531828B0u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x5818308Fu },
    { 5.0f, 0.0f, 5.0f, 5.0f, 1.0f, 2.0f, 10, 128, 0x3FC32EDCu },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x57182EFCu },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x56182D69u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0x55182BD6u },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 129, 0xE1CEEBCEu },
    { 0.5f, 0.0f, 8.0f, 40.0f, 5.0f, 2.0f, 4, 128, 0xDD8201FFu },
    { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0, 0xFFFFFFFFu },
};

// GLOBAL: SWBF2 0x01eafb42
bool g_noDisableHUD;

// FUNCTION: SWBF2 0x00657d20
bool TipType::SetState(FsmState eState, float fTime)
{
    if (mState == STATE_Active && !g_noDisableHUD)
    {
        HUD::Event event;
        event.mClass = HUD::GameEvents::gPlayerEvents[mCameraIdx].disableToolTips;
        event.mData.boolValue = false;
        event.Send();
    }
    mState = eState;
    mTimer = fTime;
    switch (eState)
    {
    case STATE_Active:
    {
        HUD::Event event;
        event.mClass = HUD::GameEvents::gPlayerEvents[mCameraIdx].displayToolTips;
        event.mData.uintValue = mClass->hash;
        event.Send();
        break;
    }
    case STATE_CoolOff:
        mTimer = mClass->mCoolOff;
        break;
    case STATE_Waiting:
        mTimer = mClass->mTimeout;
        break;
    }
    return mState == STATE_Active;
}

// FUNCTION: SWBF2 0x00657cf0
void TipType::Setup(unsigned char cameraIdx, const TipClass* pClass)
{
    mCameraIdx = cameraIdx;
    mClass = (TipClass*)pClass;
    mInput.mFilterRate = pClass->mInputHold;
    SetState(STATE_Waiting, 0.0f);
}

// FUNCTION: SWBF2 0x00657df0
void TipType::SetShowCnt(unsigned char cnt)
{
    mShowCnt = cnt;
    if (cnt < mClass->mMaxCnt)
    {
        SetState(STATE_Waiting, 0.0f);
    }
    else
    {
        SetState(STATE_Off, 0.0f);
    }
}

// FUNCTION: SWBF2 0x00657e20
void TipType::CancelState()
{
    g_noDisableHUD = mState != STATE_Active;
    mState = STATE_Active;
    mTimer = 0.0f;
    Update(1.0f, false);
    g_noDisableHUD = false;
}

// GLOBAL: SWBF2 0x01eafccc
ToolTipsDisplay* ToolTipsDisplay::sInstance;

// FUNCTION: SWBF2 0x00658020
void ToolTipsDisplay::Create()
{
    const unsigned int uNumCameras = CameraManager::sInstance->GetNumCameras();
    sInstance = new ToolTipsDisplay[uNumCameras];
    for (unsigned char i = 0; i < uNumCameras; ++i)
    {
        sInstance[i].mCameraIndex = i;
    }
}

// FUNCTION: SWBF2 0x00658140
ToolTipsDisplay::ToolTipsDisplay()
{
}

// SYNTHETIC: SWBF2 0x006580e0
// ToolTipsDisplay::`vector deleting destructor'

// FUNCTION: SWBF2 0x00658190
ToolTipsDisplay::~ToolTipsDisplay()
{
}

// FUNCTION: SWBF2 0x006581a0
void ToolTipsDisplay::Init()
{
    for (int i = 0; i < TTT_COUNT; ++i)
    {
        mTips[i].Setup(mCameraIndex, &sTipData[i]);
    }

    mAlwaysOn = false;
    mAlwaysOff = false;

    int iSlot = PlayerPrefs::FindByViewport(mCameraIndex);
    PlayerProfile::tProfile* pProfile =
        *(PlayerProfile::tProfile**)((char*)&PlayerPrefs::mAllUserPrefs + 0x14 + iSlot * 0x2c);
    if (pProfile)
    {
        if (pProfile->mToolTipOptions.mOn)
        {
            for (int i = 0; i < TTT_COUNT; ++i)
            {
                mTips[i].SetShowCnt(0);
            }
            mAlwaysOn = true;
        }
        else if (pProfile->mToolTipOptions.mOff)
        {
            for (int i = 0; i < TTT_COUNT; ++i)
            {
                mTips[i].SetShowCnt(0);
            }
            mAlwaysOff = true;
        }
        else
        {
            for (int i = 0; i < TTT_COUNT; ++i)
            {
                mTips[i].SetShowCnt(pProfile->mToolTipOptions.mToolTipCount[i]);
            }
        }
    }

    ActivateThread(0, PER_FRAME);
}

// FUNCTION: SWBF2 0x006582a0
bool ToolTipsDisplay::Update(float fDeltaTime)
{
    if (NetGame::IsEnabled() || mAlwaysOff || gAttractMode
        || SpawnDisplay::Instance(mCameraIndex)->mMode == SpawnDisplay::eCHANGECLASS)
    {
        return true;
    }

    TipType* pActive = 0;
    TipType* pNext = 0;
    for (int i = 1; i < TTT_COUNT; ++i)
    {
        TipType* pTip = &mTips[i];
        if (pTip->mState == TipType::STATE_Active || pTip->mState == TipType::STATE_CoolOff)
        {
            pActive = pTip;
            if (pActive->mState == TipType::STATE_CoolOff)
            {
                for (int j = 1; j < TTT_COUNT; ++j)
                {
                    TipType* pCand = &mTips[j];
                    if (pCand->mState == TipType::STATE_Waiting && pCand->mInput.mTrueTime > 0.0f
                        && pCand->mTimer - fDeltaTime < 0.0f)
                    {
                        if (!pNext || pCand->mClass->mPriority > pNext->mClass->mPriority)
                        {
                            pNext = pCand;
                        }
                    }
                }
                if (pNext && pNext->mClass->mPriority > pActive->mClass->mPriority)
                {
                    pActive->CancelTip(false);
                    pNext->Update(fDeltaTime, true);
                }
            }
            pActive->Update(fDeltaTime, false);
            break;
        }
    }

    for (int i = 1; i < TTT_COUNT; ++i)
    {
        TipType* pTip = &mTips[i];
        if (pTip != pNext && pTip != pActive)
        {
            pTip->Update(fDeltaTime, pActive == 0);
        }
        if (!pActive && (pTip->mState == TipType::STATE_Active || pTip->mState == TipType::STATE_CoolOff))
        {
            pActive = pTip;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00657ed0
bool TipType::Update(float fDeltaTime, bool bArg)
{
    mInput.Update(fDeltaTime);
    switch (mState)
    {
    case STATE_Waiting:
        if (mInput.GetState())
        {
            mTimer -= fDeltaTime;
        }
        if (mTimer < 0.0f)
        {
            if (bArg)
            {
                return SetState(STATE_Active, mClass->mActiveTime);
            }
            return SetState(STATE_Resetting, mClass->mFailTime);
        }
        break;
    case STATE_Active:
        mTimer -= fDeltaTime;
        if (mTimer <= 0.0f)
        {
            return SetState(STATE_CoolOff, 0.0f);
        }
        break;
    case STATE_CoolOff:
        mTimer -= fDeltaTime;
        if (mTimer <= 0.0f)
        {
            if (mClass->mMaxCnt < 0xff && ++mShowCnt >= mClass->mMaxCnt)
            {
                return SetState(STATE_Off, 0.0f);
            }
            return SetState(STATE_Latched, 0.0f);
        }
        break;
    case STATE_Latched:
        if (!mInput.GetState())
        {
            return SetState(STATE_Resetting, mClass->mResetTime);
        }
        break;
    case STATE_Resetting:
        mTimer -= fDeltaTime;
        if (mTimer <= 0.0f)
        {
            return SetState(STATE_Waiting, 0.0f);
        }
        break;
    }
    return false;
}

// FUNCTION: SWBF2 0x00658500
bool ToolTipsDisplay::ShouldTestForToolTipState(ToolTipType eType)
{
    if (mAlwaysOn)
    {
        return true;
    }
    if (mAlwaysOff)
    {
        return false;
    }
    const TipType& tip = mTips[eType];
    return tip.mClass->mMaxCnt == 0xff || tip.mShowCnt < tip.mClass->mMaxCnt;
}

// FUNCTION: SWBF2 0x00658420
void ToolTipsDisplay::ShowTip(ToolTipType eType, int iParam)
{
    if (NetGame::IsEnabled())
    {
        return;
    }
    if (mIsControllerDefault)
    {
        mTips[eType].mInput.SetTrue();
    }
}

// FUNCTION: SWBF2 0x00658480
void ToolTipsDisplay::CancelTimeoutTip(ToolTipType eType, int iParam)
{
    if (!NetGame::IsEnabled())
    {
        mTips[eType].CancelState();
    }
}

// FUNCTION: SWBF2 0x00657e50
void TipType::CancelTip(bool bArg)
{
    if (mState == STATE_Active || mState == STATE_CoolOff)
    {
        unsigned char showCnt = mShowCnt;
        SetState(STATE_CoolOff, 0.0f);
        mTimer = -1.0f;
        Update(1.0f, false);
        if (!bArg)
        {
            mShowCnt = showCnt;
        }
    }
}

// FUNCTION: SWBF2 0x00657ea0
void TipType::Reset()
{
    if (mState == STATE_Active)
    {
        mTimer = mClass->mActiveTime;
    }
    else if (mState == STATE_Resetting)
    {
        mState = STATE_Waiting;
        mTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00658470
void ToolTipsDisplay::ResetTipLatch(int)
{
    mTips[TTT_SoldierTargetCP].Reset();
}

// FUNCTION: SWBF2 0x006584c0
void ToolTipsDisplay::CancelTip(int)
{
    if (!NetGame::IsEnabled())
    {
        for (int i = 1; i < TTT_COUNT; ++i)
        {
            mTips[i].CancelTip(true);
        }
    }
}
