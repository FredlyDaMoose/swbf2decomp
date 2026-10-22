#pragma once

#include <stddef.h>
#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

// VTABLE: SWBF2 0x007af2c4
class SpectatorDisplay : public Thread
{
public:
    SpectatorDisplay();
    virtual ~SpectatorDisplay();

    // SYNTHETIC: SWBF2 0x0064f450
    // SpectatorDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    void Init();
    void Show();
    void Show(bool bShow);
    void Hide();
    void ChangeSpectatedName();
    void UpdatePregame();
    void Bind(unsigned int uiViewport);
    void SetPosition();
    static void EnableViewport(int iViewport, bool bEnable);

    void Unbind();
    void AddMessage(const unsigned short* pMessage, RedColor color);
    void AddMessage(const unsigned short* pName, const unsigned short* pMessage,
                    RedColor color);
    static void AllowRendering(bool bAllow);
    static void Create();

    static void InitAll();
    static void Destroy();

    RedScreenGroupElement mMessageGroup;
    RedBitmapElement mBitmapBackground;
    RedTextElement mSpectatorPrompt;
    RedBitmapElement mBitmapChangeView;
    RedTextElement mTxtChangeView;
    RedBitmapElement mBitmapJoinGame;
    RedTextElement mTxtJoinGame;
    RedScreenGroupElement mPregameGroup;
    RedBitmapElement mBitmapBGPregame;
    RedTextElement mPregameTime;
    RedTextElement mPregamePrompt;
    RedTextElement mPregamePrompt1;
    int mJoystickIndex;
    int mViewport;

    static SpectatorDisplay* sInstance[1];

    static bool sCanRender;

    static bool sDisable;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
