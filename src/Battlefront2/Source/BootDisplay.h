#pragma once

#include <stddef.h>

#include "Thread.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"

class Character;

// VTABLE: SWBF2 0x007970b0
class BootDisplay : public Thread
{
public:
    BootDisplay();
    virtual ~BootDisplay();

    // SYNTHETIC: SWBF2 0x0044d470
    // BootDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Create();

    static void InitAll();
    static void Destroy();
    static void EnableViewport(int iViewport, bool bEnable);
    static const unsigned short* GetBootReasonString(int iReason);

    void Init();
    void Bind(unsigned int uiViewport);
    void Unbind();
    void Show(Character* pOwner);
    void Hide(Character* pOwner);
    void AllowRendering(bool bAllow);
    void SetSpawnMode(bool bSpawnMode);

    // FUNCTION: SWBF2 0x004294c0
    static BootDisplay* Instance(unsigned int uViewport)
    {
        return uViewport < 1 ? sInstance[uViewport] : 0;
    }
    void SetLobbyMode(bool bLobbyMode);

private:
    void Hide();
    void UpdateMode();

public:
    int mJoystickIndex;
    int mViewport;
    bool mCanRender;
    Character* mOwner;
    bool mVisible;
    bool mSpawnMode;
    bool mLobbyMode;
    RedScreenGroupElement mStatusGroup;
    RedTextElement mStatusText0;
    RedTextElement mStatusText1;
    RedTextElement mStatusText2;

    static BootDisplay* sInstance[1];
};
 
 
 
 
 
 
