#pragma once

#include <stddef.h>
#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

// VTABLE: SWBF2 0x0079719c
class ChatMessageDisplay : public Thread
{
public:
    ChatMessageDisplay();
    virtual ~ChatMessageDisplay();

    // SYNTHETIC: SWBF2 0x004547a0
    // ChatMessageDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    void Init();
    void Show();
    void Show(bool bShow);
    void Show(bool bShow, unsigned short usType);
    void Hide();

    void AddMessage(const unsigned short* pMessage, RedColor color);
    void AddMessage(const unsigned short* pName, const unsigned short* pMessage,
                    RedColor color);
    static void Create();
    static void Destroy();

    static ChatMessageDisplay* Instance() { return sInstance; }

    RedScreenGroupElement mMessageGroup;
    RedBitmapElement mBitmapBackground;
    RedTextElement mTextElement;
    RedTextElement mTextElementPrompt;

    static ChatMessageDisplay* sInstance;

    static bool sCanRender;

    static bool sDisable;
};
 
 
 
 
 
