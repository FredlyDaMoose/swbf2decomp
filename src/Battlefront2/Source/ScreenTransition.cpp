#include <string.h>

#include "ScreenTransition.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedColor.h"

namespace FLRenderer
{
void SetTransitionBlurColor(int iView, RedColor const& rColor);
void SetTransitionBlurZoom(int iView, float fZoom);
void SetTransitionOverlayColor(int iView, RedColor const& rColor);
void SetTransitionOverlayRect(int iView, float fX0, float fY0, float fX1, float fY1);
}

// GLOBAL: SWBF2 0x01eaf028
ScreenTransition* ScreenTransition::sInstance;

// FUNCTION: SWBF2 0x006342c0
ScreenTransition::ScreenTransition()
{
    memset(mData, 0, sizeof(mData));
    mCallback = 0;
    mCallbackData = 0;
}

// FUNCTION: SWBF2 0x00634270
void ScreenTransition::Create()
{
    if (!sInstance)
    {
        sInstance = new ScreenTransition();
    }
}

static inline RedColor MakeColor(unsigned char r, unsigned char g, unsigned char b,
                                 unsigned char a)
{
    RedColor c;
    c.b = b;
    c.g = g;
    c.r = r;
    c.a = a;
    return c;
}

// FUNCTION: SWBF2 0x006343f0
void ScreenTransition::UpdateFadeIn(int iScreen, TransitionType eType, float fTime)
{
    switch (eType)
    {
    case BLUR:
        FLRenderer::SetTransitionBlurZoom(iScreen, 0.0f);
        FLRenderer::SetTransitionBlurColor(iScreen,
            MakeColor(0xff, 0xff, 0xff, (unsigned char)(0xff - (int)(fTime * 255.0f))));
        FLRenderer::SetTransitionOverlayColor(iScreen, MakeColor(0, 0, 0, 0));
        FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, 0.0f, 1.0f, 1.0f);
        break;

    case FADE:
        FLRenderer::SetTransitionBlurZoom(iScreen, 0.0f);
        FLRenderer::SetTransitionBlurColor(iScreen, MakeColor(0, 0, 0, 0));
        FLRenderer::SetTransitionOverlayColor(iScreen,
            MakeColor(0, 0, 0, (unsigned char)(0xff - (int)(fTime * 255.0f))));
        FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, 0.0f, 1.0f, 1.0f);
        break;

    case SWIPE_LR:
    case SWIPE_RL:
        FLRenderer::SetTransitionBlurZoom(iScreen, 0.0f);
        FLRenderer::SetTransitionBlurColor(iScreen, MakeColor(0, 0, 0, 0));
        FLRenderer::SetTransitionOverlayColor(iScreen, MakeColor(0, 0, 0, 0xff));
        if (eType == SWIPE_LR)
        {
            FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, 0.0f, 1.0f - fTime, 1.0f);
        }
        else
        {
            FLRenderer::SetTransitionOverlayRect(iScreen, fTime, 0.0f, 1.0f, 1.0f);
        }
        break;

    case SWIPE_TB:
    case SWIPE_BT:
        FLRenderer::SetTransitionBlurZoom(iScreen, 0.0f);
        FLRenderer::SetTransitionBlurColor(iScreen, MakeColor(0, 0, 0, 0));
        FLRenderer::SetTransitionOverlayColor(iScreen, MakeColor(0, 0, 0, 0xff));
        if (eType == SWIPE_TB)
        {
            FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, 0.0f, 1.0f, 1.0f - fTime);
        }
        else
        {
            FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, fTime, 1.0f, 1.0f);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00634600
void ScreenTransition::UpdateFadeHold(int iScreen, TransitionType eType, float fTime)
{
    UpdateFadeIn(iScreen, eType, fTime);
}

// FUNCTION: SWBF2 0x00634610
void ScreenTransition::UpdateFadeOut(int iScreen, TransitionType eType, float fTime)
{
    UpdateFadeIn(iScreen, eType, 1.0f - fTime);
}

// FUNCTION: SWBF2 0x00634630
void ScreenTransition::UpdateScreen(int iScreen, float fDelta)
{
    TransitionData& rData = mData[iScreen];
    rData.mCurTime -= fDelta;
    if (rData.mState == FADE_OFF)
    {
        return;
    }

    if (rData.mState == FADE_IN)
    {
        if (rData.mCurTime <= 0.0f)
        {
            rData.mState = FADE_HOLD;
            rData.mCurTime = rData.mHoldTime;
        }
        else
        {
            UpdateFadeIn(iScreen, rData.mInType, rData.mCurTime / rData.mInTime);
        }
    }

    if (rData.mState == FADE_HOLD)
    {
        if (rData.mCurTime <= 0.0f)
        {
            rData.mState = FADE_OUT;
            rData.mCurTime = rData.mOutTime;
        }
        else
        {
            UpdateFadeHold(iScreen, rData.mInType, 0.0f);
        }
    }

    if (rData.mState == FADE_OUT)
    {
        if (rData.mCurTime <= 0.0f)
        {
            rData.mState = FADE_OFF;
            FLRenderer::SetTransitionBlurZoom(iScreen, 0.0f);
            FLRenderer::SetTransitionBlurColor(iScreen, MakeColor(0, 0, 0, 0));
            FLRenderer::SetTransitionOverlayColor(iScreen, MakeColor(0, 0, 0, 0));
            FLRenderer::SetTransitionOverlayRect(iScreen, 0.0f, 0.0f, 1.0f, 1.0f);
            if (mCallback)
            {
                mCallback(mCallbackData);
                mCallback = 0;
                mCallbackData = 0;
            }
        }
        else
        {
            UpdateFadeOut(iScreen, rData.mOutType, rData.mCurTime / rData.mOutTime);
        }
    }
}

// FUNCTION: SWBF2 0x00634780
void ScreenTransition::Update(float fDelta)
{
    for (unsigned int i = 0; i < 2; ++i)
    {
        UpdateScreen(i, fDelta);
    }
}

// FUNCTION: SWBF2 0x00634320
void ScreenTransition::BeginTransition(unsigned int uiScreen, float fInTime, float fHoldTime, float fOutTime,
                                       TransitionType eInType, TransitionType eOutType,
                                       void (__cdecl* pCallback)(void*), void* pCallbackData)
{
    if (uiScreen > 1)
    {
        return;
    }

    TransitionData& rData = mData[uiScreen];
    rData.mState = FADE_IN;
    rData.mInTime = fInTime;
    rData.mHoldTime = fHoldTime;
    rData.mOutTime = fOutTime;
    rData.mCurTime = fInTime;
    mCallback = pCallback;
    mCallbackData = pCallbackData;

    if (eInType == SWIPE_RAND)
    {
        eInType = (TransitionType)(SWIPE_LR + PblRandom::GetGlobalInt() % 4);
    }
    if (eOutType == SWIPE_RAND)
    {
        eOutType = (TransitionType)(SWIPE_LR + PblRandom::GetGlobalInt() % 4);
    }
    rData.mInType = eInType;
    rData.mOutType = eOutType;

    UpdateScreen(uiScreen, 0.0f);
}

// FUNCTION: SWBF2 0x006342f0
void ScreenTransition::SetTransition(unsigned int uiScreen, TransitionType eType)
{
    BeginTransition(uiScreen, 0.0f, 0.0f, 0.3f, eType, eType, 0, 0);
}

// FUNCTION: SWBF2 0x006342a0
void ScreenTransition::Destroy()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = 0;
    }
}
