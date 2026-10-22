#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

class Character;

// VTABLE: SWBF2 0x007970f8
class CaptureDisplay : public Thread
{
public:
    CaptureDisplay();
    virtual ~CaptureDisplay();
    virtual bool Update(float fDeltaTime);

    static void Create();

    static void InitAll();
    static void Destroy();
    static void EnableViewport(int iViewport, bool bEnable);

    void Init();
    void Bind(unsigned int uiViewport);
    void Unbind();
    void Show(Character* pOwner);
    void Hide(Character* pOwner);
    void AllowRendering(bool bAllow);
    void UpdateCapture(int iTeam, int iOtherTeam, float fRatio);
    void UpdateClear();
    void UpdateNeutralize(int iTeam, float fRatio);

private:
    void UpdateIcon();

public:
    int mJoystickIndex;
    int mViewport;
    bool mCanRender;
    RedScreenGroupElement mFlagGroup;
    RedBitmapElement mFlagTop;
    RedBitmapElement mFlagBottom;
    RedBitmapElement mFlagLocked;
    Character* mOwner;
    float mSplitRatio;
    RedColor mColorTop;
    RedColor mColorBottom;
    bool mVisible : 1;

    static CaptureDisplay* sInstance[1];
};
 
 
 
 
 
 
 
