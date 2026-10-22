#pragma once

#include <stddef.h>

#include "Thread.h"

class Character;
class GameSound;

// VTABLE: SWBF2 0x007a0344
class HeroMessageDisplay : public Thread
{
public:
    HeroMessageDisplay();
    virtual ~HeroMessageDisplay();

    // SYNTHETIC: SWBF2 0x00542de0
    // HeroMessageDisplay::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    static void Create();

    static void InitAll();
    static void Destroy();
    static void EnableViewport(int iViewport, bool bEnable);
    static void PlayHeroSoundSpawn(int iTeam);
    static void PlayHeroSoundDefeat(int iTeam);

    static void PlayHeroSoundUnlock(int iTeam);

    static void PlayHeroSoundSelectable(int iTeam);

    void Init();
    void Bind(unsigned int uJoystick);
    void Show(Character* pOwner);
    void Hide(Character* pOwner);
    void SetSpawnMode(bool bSpawnMode);

    static HeroMessageDisplay* Instance(unsigned int uiViewport)
    {
        return uiViewport < 1 ? sInstance[uiViewport] : 0;
    }
    bool IsInHeroSelecting() { return mIsInHeroSelecting; }

    void UpdateHeroSelect(float fDeltaTime, int iTeam, float& fTotalTime,
                          float& fTimeLeft, Character* pCharacter);

private:
    void Hide();

public:
    int mJoystickIndex;
    int mViewport;
    bool mCanRender;
    Character* mOwner;
    bool mVisible;
    bool mSpawnMode;
    bool mLobbyMode;
    bool mIsInHeroSelecting;

    static HeroMessageDisplay* sInstance[1];
    static GameSound* mHeroesUnlocked;
    static GameSound* mHeroUnlocked[3];
    static GameSound* mHeroSelectable[3];
    static GameSound* mHeroSpawned[3];
    static GameSound* mHeroDefeated[3];
    static GameSound* mYouKilledHero[3];
};
 
 
 
 
