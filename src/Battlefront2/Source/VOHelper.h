#pragma once

#include <stddef.h>

#include "GameSound.h"

class GameObject;
class PblVector3;

namespace AI
{

enum VO_AboveBelowCover { VO_ABC_None = 0, VO_ABC_Above = 1, VO_ABC_Below = 2, VO_ABC_Cover = 3 };
enum VO_RightLeftBehind { VO_RLB_None = 0, VO_RLB_Right = 1, VO_RLB_Left = 2, VO_RLB_Behind = 3 };
enum VO_Distance { VO_D_None = 0, VO_D_ReallyNear = 1, VO_D_Near = 2, VO_D_ReallyFar = 3 };
enum VO_TargetHealth { VO_TH_None = 0, VO_TH_Below25 = 1 };
enum VO_PlayerHealth { VO_PH_None = 0, VO_PH_Below25 = 1 };
enum VO_TargetType
{
    VO_TT_None = 0,
    VO_TT_IsFlyer = 1,
    VO_TT_IsHover = 2,
    VO_TT_IsWalker = 3,
    VO_TT_IsTurret = 4,
    VO_TT_IsSniper = 5,
    VO_TT_IsRocket = 6,
    VO_TT_IsHero = 7
};
enum VO_MeInfo
{
    VO_ME_None = 0,
    VO_ME_InFlyer = 1,
    VO_ME_InHover = 2,
    VO_ME_InWalker = 3,
    VO_ME_InTurret = 4,
    VO_ME_IsZoomed = 5,
    VO_ME_IsProne = 6
};
enum VO_Adrenaline { VO_ADR_None = 0, VO_ADR_Low = 1, VO_ADR_Medium = 2, VO_ADR_High = 3 };

struct VOModifier
{
    unsigned int mAboveBelowCover : 2;
    unsigned int mRightLeftBehind : 2;
    unsigned int mDistance : 2;
    unsigned int mTargetHealth : 1;
    unsigned int mPlayerHealth : 1;
    unsigned int mTargetType : 3;
    unsigned int mMeInfo : 3;
    unsigned int mAdrenaline : 2;
    unsigned int mTargetUnitNum : 8;
};
 

class VOSound
{
public:

    VOSound();

    // FUNCTION: SWBF2 0x00671a70
    ~VOSound() {}

    GameSound mSound;
    int mType;
    VOModifier mModifiers;
    VOSound* mNext;
    unsigned char mScore;
};
 
 

namespace VOHelper
{

void SetModifier(VOModifier& modifier, const char* pszWord, const char* pszDebug);

int TypeStrToEnum(const char* pszType);

VOSound* GetNewVOSound(const char* pszSound, const char* pszType, const VOModifier& modifiers);

void CancelQueuedSound(int iHandle);

void GetModifiers(const GameObject* pTarget, const PblVector3& vPos, VOModifier& modifiers);
VOModifier GetModifiers(const GameObject* pTarget, const GameObject* pOther);
bool HasRequiredModifiers(const VOSound* pSound, const VOModifier& modifiers);
int GetModifierScore(const VOSound* pSound, const VOModifier& modifiers);
int VOSoundCompareCallback(const void* pA, const void* pB);
}

}
