#pragma once

#include <stddef.h>

#include "Thread.h"
#include "GameMusic.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

class EndDisplay : public Thread
{
public:
    EndDisplay();

    // SYNTHETIC: SWBF2 0x00490d00
    // EndDisplay::`scalar deleting destructor'
    virtual ~EndDisplay();

    void Init();
    void Show(unsigned int uiWinners, float fMinDisplayTime);
    void Hide();
    static void HideCallback(void* pData);
    virtual bool Update(float fDeltaTime);

    bool HasBeenDisplayed();
    bool IsMusicFinished() const;

    bool IsReallyFinished();

    bool IsFinished();

    void SetVictoryMusic(unsigned int uTeam, const char* pName);
    void SetDefeatMusic(unsigned int uTeam, const char* pName);
    void SetEndMovie(const char* pFileName, const char* pIdName);

    RedScreenGroupElement mEndGroup[1];
    RedBitmapElement mEndText[1];
    GameMusic mVictoryDefeat[2][2];
    char mMovieFileName[64];
    char mMovieIdName[64];
    float mMinDisplayTime;
    bool mDisplayed : 1;
    bool mMoviePlayed : 1;
    bool mTransition : 1;
    bool mHideIt : 1;

    static void Create();
    static EndDisplay* sInstance;
    static bool sCanRender;
};
 
 
 
 
 
 
