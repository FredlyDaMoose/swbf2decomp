#pragma once

#include "Thread.h"

class RedColor;
class Character;
class Team;

// VTABLE: SWBF2 0x007aa4f0
class MessageDisplay : public Thread
{
public:
    // SYNTHETIC: SWBF2 0x005a3970
    // MessageDisplay::`scalar deleting destructor'
    virtual ~MessageDisplay();
    virtual bool Update(float fDeltaTime);

    enum DISPLAY_TYPE
    {
        DISPLAY_SYSTEM_MSG = 0,
        DISPLAY_PLAYER_CHAT = 1,
        DISPLAY_END = 2
    };

    MessageDisplay(DISPLAY_TYPE eType);

    static MessageDisplay* Instance(unsigned int uViewport, DISPLAY_TYPE eType);

    static void EnableViewport(unsigned int uiViewport, bool bEnable);

    void GetInfo(unsigned int& uViewport, DISPLAY_TYPE& eType) const;

    void AddMessage(const unsigned short* pText, RedColor color);

    void AddKillMessage(const Character* pVictim, const Character* pKiller, unsigned int uIcon,
                        const Team* pTeam);

    bool mCanRender;
    bool mVisible;

    static void Create();
    static void Destroy();
    static MessageDisplay* sInstance[1][DISPLAY_END];
};
 
