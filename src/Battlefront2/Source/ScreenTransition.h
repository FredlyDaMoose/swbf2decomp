#pragma once

#include <stddef.h>

// VTABLE: SWBF2 0x007ae44c
class ScreenTransition
{
public:

    enum TransitionType
    {
        FADE_ZOOM = 0,
        BLUR = 1,
        FADE = 2,
        SWIPE_LR = 3,
        SWIPE_RL = 4,
        SWIPE_TB = 5,
        SWIPE_BT = 6,
        SWIPE_RAND = 7
    };

    enum TransitionState
    {
        FADE_OFF = 0,
        FADE_IN = 1,
        FADE_HOLD = 2,
        FADE_OUT = 3
    };

    struct TransitionData
    {
        TransitionState mState;
        float mCurTime;
        float mInTime;
        float mHoldTime;
        float mOutTime;
        TransitionType mInType;
        TransitionType mOutType;
    };

    ScreenTransition();

    static void Create();
    static void Destroy();

    virtual void Update(float fDelta);

    void BeginTransition(unsigned int uiScreen, float fInTime, float fHoldTime, float fOutTime,
                         TransitionType eInType, TransitionType eOutType,
                         void (__cdecl* pCallback)(void*), void* pCallbackData);

    void SetTransition(unsigned int uiScreen, TransitionType eType);

private:
    void UpdateFadeIn(int iScreen, TransitionType eType, float fTime);
    void UpdateFadeHold(int iScreen, TransitionType eType, float fTime);
    void UpdateFadeOut(int iScreen, TransitionType eType, float fTime);
    void UpdateScreen(int iScreen, float fDelta);

public:
    TransitionData mData[2];
    void (__cdecl* mCallback)(void*);
    void* mCallbackData;

    static ScreenTransition* sInstance;
};
 
 
 
 
 
