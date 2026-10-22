#pragma once

#include <stddef.h>
#include "RedEngineFL/Interface/RedTextElement.h"
#include "PebbleFL/Common/PblList.h"
#include "Thread.h"

class pcInterfaceShader;

enum
{
    FLASHY_STATE_DELAY = 0,
    FLASHY_STATE_BG = 1,
    FLASHY_STATE_DELAY2 = 2,
    FLASHY_STATE_CURSOR = 3,
    FLASHY_STATE_EXPAND = 4,
    FLASHY_STATE_ALPHAIN = 5,
    FLASHY_STATE_DONE = 6
};

// VTABLE: SWBF2 0x0079eeb0
class FlashyTextElement : public RedTextElement
{
public:

    struct FlashyVertexT
    {
        float x;
        float y;
        float z;
        float u;
        float v;
    };

    FlashyTextElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x00525f50
    // FlashyTextElement::`scalar deleting destructor'

    virtual ~FlashyTextElement();
    virtual void Init();

    // FUNCTION: SWBF2 0x00525f40
    virtual bool IsFlashyText() const { return true; }

    virtual void Cleanup();

    void CommonConstructor();

    void PlatformConstructor();

    void SetBackgroundSize(char cOffsetX, char cOffsetY, char cExpandX, char cExpandY);
    void SetBackground(unsigned int uHashLeft, unsigned int uHashMid, unsigned int uHashRight);
    void StartFlashiness();

    bool UpdateFlashiness(float fDeltaTime);

protected:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

    void RenderBitmap(const PblMatrix& mContext, const RedColor& color, unsigned int uHash,
                      float fX0, float fY0, float fX1, float fY1);

public:
    PblList<FlashyTextElement>::Node mNode;
    float mTextYScale;
    RedColor mTextColor;
    float mTimeDelay;
    float mTime;
    unsigned int mHashLeft;
    unsigned int mHashMid;
    unsigned int mHashRight;
    char mBGOffsetX;
    char mBGOffsetY;
    char mBGExpandX;
    char mBGExpandY;
    unsigned short mWidth;
    unsigned char mTail;
    unsigned int mState : 3;
    unsigned int mFlipped : 1;
    unsigned int mXFlipped : 1;
    unsigned int mRightJustifyBG : 1;
    unsigned int mDrawBG : 1;
    pcInterfaceShader* m_pInterfaceBitmapShader;
};
 
 
 
 
 

// VTABLE: SWBF2 0x0079eefc
class FlashyTextManager : public Thread
{
public:
    FlashyTextManager();
    virtual ~FlashyTextManager();

    // SYNTHETIC: SWBF2 0x00526980
    // FlashyTextManager::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Init(bool bMission);

    static bool IsElementActive(FlashyTextElement* pElement);
    static void AddToUpdateList(FlashyTextElement* pElement);
    static void RemoveFromUpdateList(FlashyTextElement* pElement);
    static void Clean();

    PblList<FlashyTextElement> mFTEList;
};
 
 
