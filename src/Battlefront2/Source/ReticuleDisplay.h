#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblVector.h"

class Character;
class RedScreenGroupElement;

// VTABLE: SWBF2 0x007adff4
class ReticuleInterpolator : public Thread
{
public:

    // FUNCTION: SWBF2 0x00630480
    ReticuleInterpolator() {}
    virtual ~ReticuleInterpolator();

    virtual bool Update(float fDeltaTime);

    void Create();
    void Destroy();
    void SetNext(PblVector3* pPos, RedScreenGroupElement* pGroupAimArea, int iJoystickIndex);

    int mCount;
    PblVector3 mPos[2];
    RedScreenGroupElement* mGroupAimArea;
    int mJoystickIndex;
};
 
 
 

// VTABLE: SWBF2 0x007ae00c
class ReticuleDisplay : public Thread
{
public:
    ReticuleDisplay();
    virtual ~ReticuleDisplay();

    virtual bool Update(float fDeltaTime);

    static void Create();

    static void BindAll();
    static void Destroy();
    static void EnableViewport(unsigned int uiViewport, bool bEnable);

    void Init();
    void Bind(unsigned int uiJoystickIndex);
    void Unbind();
    void Show(Character* pCharacter);
    void Hide(Character* pCharacter);
    void AllowRendering(bool bAllow);
    void HitIndicatorSetTexture(unsigned int uiTexture);

    static ReticuleDisplay* sInstance[1];

private:
    void Hide();

public:
    int mJoystickIndex;
    unsigned int mViewport;
    bool mCanRender;
    bool mVisible;
};
 
 
 

extern ReticuleInterpolator reticuleInterpolator[1];
