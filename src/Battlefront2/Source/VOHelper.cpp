#include "VOHelper.h"
#include "AIUtil.h"
#include "Character.h"
#include "Controllable.h"
#include "EntityCarrier.h"
#include "EntitySoldier.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PlayerController.h"
#include "Team.h"
#include "Trackable.h"
#include "UnitController.h"
#include "PebbleFL/Common/PblHashTable.h"

#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

// GLOBAL: SWBF2 0x0078bce0
static const unsigned int VOTypeNames[48] =
{
    0x00000000, 0x2588f638, 0xe359b342, 0x0cb276ac,
    0xee6df4fd, 0x2af268b3, 0xaaccc29a, 0x9ce7b15e,
    0xc50f4599, 0xdeb06c63, 0xd988d8fd, 0xfdf0ecb5,
    0x74bb501c, 0x78bb5668, 0x5ad79083, 0x5ed796cf,
    0x0d430618, 0xce19d989, 0xcfd5c320, 0xdd1db480,
    0xc0a42c15, 0xeaf65ff1, 0x49dfeb81, 0xaef2d88a,
    0x421c2029, 0x1b8a0392, 0x37c344c8, 0x2ebba9e7,
    0xfe5092df, 0x048b38ec, 0xd8a23783, 0x8bb82ce4,
    0x027d1363, 0x5e739197, 0xde654ab1, 0x729931de,
    0x3f4ec8e3, 0x15b693e4, 0xd0e42acc, 0x8435f5fe,
    0x31b64121, 0x006b4d8e, 0x0ff77e53, 0x4460cdc4,
    0x1d2f03ea, 0x03a34694, 0x7208dd19, 0x335a8bad,
};

namespace AI {
namespace VOHelper {
extern VOSound* sLoadingBatch;
}
}
namespace AI {
namespace VOHelper {
extern int sLoadingBatchIndex;
}
}

namespace AI
{
namespace VOHelper
{

// FUNCTION: SWBF2 0x00671640
void SetModifier(VOModifier& modifier, const char* pszWord, const char*)
{
    if (!_strnicmp(pszWord, "VOUnitNum", 9))
    {
        modifier.mTargetUnitNum = atoi(pszWord + 9);
        return;
    }

    switch ((unsigned int)PblHash(pszWord))
    {
    case 0xA4213C0A: modifier.mAboveBelowCover = VO_ABC_Above; break;
    case 0x9253C0F2: modifier.mAboveBelowCover = VO_ABC_Below; break;
    case 0xBACD0480: modifier.mAboveBelowCover = VO_ABC_Cover; break;
    case 0x78E32DE5: modifier.mRightLeftBehind = VO_RLB_Right; break;
    case 0x124AEC70: modifier.mRightLeftBehind = VO_RLB_Left; break;
    case 0xDCED8A09: modifier.mRightLeftBehind = VO_RLB_Behind; break;
    case 0xE5DAB718: modifier.mDistance = VO_D_ReallyNear; break;
    case 0x54F3570F: modifier.mDistance = VO_D_Near; break;
    case 0x1CDAD1EB: modifier.mDistance = VO_D_ReallyFar; break;
    case 0xEAFD2812: modifier.mTargetHealth = VO_TH_Below25; break;
    case 0x1EED85CA: modifier.mPlayerHealth = VO_PH_Below25; break;
    case 0xEEDDAE45: modifier.mTargetType = VO_TT_IsFlyer; break;
    case 0x5D847EDD: modifier.mTargetType = VO_TT_IsHover; break;
    case 0x91668BB7: modifier.mTargetType = VO_TT_IsWalker; break;
    case 0x710F3467: modifier.mTargetType = VO_TT_IsTurret; break;
    case 0xB0C0E926: modifier.mTargetType = VO_TT_IsSniper; break;
    case 0xB79BD865: modifier.mTargetType = VO_TT_IsRocket; break;
    case 0x6B468289: modifier.mTargetType = VO_TT_IsHero; break;
    case 0x76EB7F24: modifier.mMeInfo = VO_ME_InFlyer; break;
    case 0x5968B570: modifier.mMeInfo = VO_ME_InHover; break;
    case 0x1EE5DFFC: modifier.mMeInfo = VO_ME_InWalker; break;
    case 0x1C993CF8: modifier.mMeInfo = VO_ME_InTurret; break;
    case 0xE16E9E73: modifier.mMeInfo = VO_ME_IsZoomed; break;
    case 0x3B183215: modifier.mMeInfo = VO_ME_IsProne; break;
    case 0xABC74D1C: modifier.mAdrenaline = VO_ADR_Low; break;
    case 0x33C5D9AD: modifier.mAdrenaline = VO_ADR_Medium; break;
    case 0x8929FEF6: modifier.mAdrenaline = VO_ADR_High; break;
    }
}

// FUNCTION: SWBF2 0x00671a80
void GetModifiers(const GameObject* pTarget, const PblVector3& vPos, VOModifier& modifiers)
{
    const PblMatrix& mTarget = pTarget->GetMatrix();
    const PblVector3& vTarget = mTarget.GetTranslation();

    const float fHeight = vPos.y - vTarget.y;
    const float fX = vTarget.x - vPos.x;
    const float fZ = vTarget.z - vPos.z;
    float fDist = sqrtf(fX * fX + fZ * fZ);
    if (!(fDist > 0.1f))
    {
        fDist = 0.1f;
    }
    const float fSlope = fHeight / fDist;
    if (fHeight > 3.0f && fSlope > 0.25f)
    {
        modifiers.mAboveBelowCover = VO_ABC_Above;
    }
    else if (fHeight < -3.0f && fSlope < -0.25f)
    {
        modifiers.mAboveBelowCover = VO_ABC_Below;
    }

    const PblVector3 vFacing = AIUtil::GetSafeNormalXZ(pTarget->GetMatrix().GetZAxis());
    PblVector3 vDelta;
    vDelta.x = vPos.x - vTarget.x;
    vDelta.y = vPos.y - vTarget.y;
    vDelta.z = vPos.z - vTarget.z;
    const PblVector3 vDir = AIUtil::GetSafeNormalXZ(vDelta);
    const float fFront = vDir.x * vFacing.x + vDir.y * vFacing.y + vDir.z * vFacing.z;
    if (fFront < -0.7071f)
    {
        modifiers.mRightLeftBehind = VO_RLB_Behind;
    }
    else if (fFront < 0.7071f)
    {
        const PblVector3 vSide = AIUtil::GetSafeNormalXZ(pTarget->GetMatrix().GetXAxis());
        const float fRight = vSide.x * vDir.x + vSide.y * vDir.y + vSide.z * vDir.z;
        modifiers.mRightLeftBehind = fRight < 0.0f ? VO_RLB_Right : VO_RLB_Left;
    }

    if (fDist < 10.0f)
    {
        modifiers.mDistance = VO_D_ReallyNear;
    }
    else if (fDist < 30.0f)
    {
        modifiers.mDistance = VO_D_Near;
    }
    else if (fDist > 100.0f)
    {
        modifiers.mDistance = VO_D_ReallyFar;
    }
}

// FUNCTION: SWBF2 0x00671c80
VOModifier GetModifiers(const GameObject* pTarget, const GameObject* pOther)
{
    VOModifier modifiers = VOModifier();
    if (!pOther)
    {
        return modifiers;
    }

    GetModifiers(pTarget, pOther->GetMatrix().GetTranslation(), modifiers);

    if (pOther->mCurHealth / pOther->mMaxHealth < 0.25f)
    {
        modifiers.mTargetHealth = VO_TH_Below25;
    }
    if (pTarget->mCurHealth / pTarget->mMaxHealth < 0.25f)
    {
        modifiers.mPlayerHealth = VO_PH_Below25;
    }

    switch (AIUtil::GetEntityType(pOther->GetEntity()))
    {
    case AIUtil::ENTITY_WALKER:
        modifiers.mTargetType = VO_TT_IsWalker;
        break;
    case AIUtil::ENTITY_FLYER:
        modifiers.mTargetType = VO_TT_IsFlyer;
        break;
    case AIUtil::ENTITY_HOVER:
        modifiers.mTargetType = VO_TT_IsHover;
        break;
    case AIUtil::ENTITY_TURRET:
    case AIUtil::ENTITY_BUILDING_ARMED:
    case AIUtil::ENTITY_REMOTE_TERMINAL:
        modifiers.mTargetType = VO_TT_IsTurret;
        break;
    default:
        if (AIUtil::IsSniperGuy(pOther))
        {
            modifiers.mTargetType = VO_TT_IsSniper;
        }
        else if (AIUtil::IsRocketGuy(pOther))
        {
            modifiers.mTargetType = VO_TT_IsRocket;
        }
        else if (AIUtil::IsHeroGuy(pOther))
        {
            modifiers.mTargetType = VO_TT_IsHero;
        }
        break;
    }

    switch (AIUtil::GetEntityType(pTarget->GetEntity()))
    {
    case AIUtil::ENTITY_SOLDIER:
    {
        EntitySoldier* pSoldier = static_cast<EntitySoldier*>(const_cast<Entity*>(pTarget->GetEntity()));
        if (pSoldier->GetTargetInfo()->mIsAiming)
        {
            modifiers.mMeInfo = VO_ME_IsZoomed;
        }
        else if (pSoldier->mState == SoldierState::PRONE)
        {
            modifiers.mMeInfo = VO_ME_IsProne;
        }
        break;
    }
    case AIUtil::ENTITY_WALKER:
        modifiers.mTargetType = VO_ME_InWalker;
        break;
    case AIUtil::ENTITY_FLYER:
        modifiers.mTargetType = VO_ME_InFlyer;
        break;
    case AIUtil::ENTITY_HOVER:
        modifiers.mTargetType = VO_ME_InHover;
        break;
    case AIUtil::ENTITY_TURRET:
    case AIUtil::ENTITY_BUILDING_ARMED:
    case AIUtil::ENTITY_REMOTE_TERMINAL:
        modifiers.mMeInfo = VO_ME_InTurret;
        break;
    default:
        break;
    }

    if (pTarget->GetControllable() && pTarget->GetControllable()->mCharacter)
    {
        switch (pTarget->GetControllable()->mCharacter->mAdrenaline.GetCurrentLevel())
        {
        case ADRENALINE_LOW:
            modifiers.mAdrenaline = VO_ADR_Low;
            break;
        case ADRENALINE_MEDIUM:
            modifiers.mAdrenaline = VO_ADR_Medium;
            break;
        case ADRENALINE_HIGH:
            modifiers.mAdrenaline = VO_ADR_High;
            break;
        }
    }

    modifiers.mTargetUnitNum = pOther->GetGameObjectClass()->mVOUnitType;
    return modifiers;
}

// FUNCTION: SWBF2 0x00671f00
bool HasRequiredModifiers(const VOSound* pSound, const VOModifier& modifiers)
{
    const VOModifier& required = pSound->mModifiers;
    if (required.mAboveBelowCover && required.mAboveBelowCover != modifiers.mAboveBelowCover)
    {
        return false;
    }
    if (required.mRightLeftBehind && required.mRightLeftBehind != modifiers.mRightLeftBehind)
    {
        return false;
    }
    if (required.mDistance && required.mDistance != modifiers.mDistance)
    {
        return false;
    }
    if (required.mTargetType && required.mTargetType != modifiers.mTargetType)
    {
        return false;
    }
    if (required.mMeInfo && required.mMeInfo != modifiers.mMeInfo)
    {
        return false;
    }
    if (required.mTargetUnitNum && required.mTargetUnitNum != modifiers.mTargetUnitNum)
    {
        return false;
    }
    return true;
}

// FUNCTION: SWBF2 0x00671f90
int GetModifierScore(const VOSound* pSound, const VOModifier& modifiers)
{
    const VOModifier& required = pSound->mModifiers;
    int iScore = 0;
    if (required.mAboveBelowCover && required.mAboveBelowCover == modifiers.mAboveBelowCover)
    {
        ++iScore;
    }
    if (required.mRightLeftBehind && required.mRightLeftBehind == modifiers.mRightLeftBehind)
    {
        ++iScore;
    }
    if (required.mDistance && required.mDistance == modifiers.mDistance)
    {
        ++iScore;
    }
    if (required.mTargetType && required.mTargetType == modifiers.mTargetType)
    {
        ++iScore;
    }
    if (required.mMeInfo && required.mMeInfo == modifiers.mMeInfo)
    {
        ++iScore;
    }
    if (required.mTargetUnitNum && required.mTargetUnitNum == modifiers.mTargetUnitNum)
    {
        ++iScore;
    }
    return iScore;
}

// FUNCTION: SWBF2 0x00672030
int VOSoundCompareCallback(const void* pA, const void* pB)
{
    const VOSound* pSoundA = *(const VOSound* const*)pA;
    const VOSound* pSoundB = *(const VOSound* const*)pB;
    if (pSoundB->mScore > pSoundA->mScore)
    {
        return 1;
    }
    if (pSoundB->mScore < pSoundA->mScore)
    {
        return -1;
    }
    if (pB > pA)
    {
        return 1;
    }
    if (pB < pA)
    {
        return -1;
    }
    return 0;
}

}

enum VOType
{
    VOT_NONE = 0,
    VOT_AcquiredTarget = 1,
    VOT_PathFollowWait = 6,
    VOT_RebelsShootDeadBody = 11
};

namespace VOHelper
{

struct QueuedSound
{
    VOType mType;
    PblHandle<const GameObject> mSpeaker;
    PblHandle<const GameObject> mPlayer;
    PblHandle<const GameObject> mTarget;
    PblVector3 mPos;
    bool mUseTarget;
    float mTime;
    int mHandle;

    ~QueuedSound();
};
extern QueuedSound mSndQueue[20];
extern int mNextQueueHandle;

int Queue(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
          const GameObject* pOther, const PblVector3* pPos, float fDelay);

void GetModifiers(const GameObject* pTarget, const PblVector3& vPos, VOModifier& modifiers);

VOModifier GetModifiers(const GameObject* pTarget, const GameObject* pOther);

void _Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
           const VOModifier& modifiers);

// FUNCTION: SWBF2 0x00672070
void _Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
           const VOModifier& modifiers)
{
    if (!pSpeaker)
    {
        float fClosestSq = FLT_MAX;
        const Team* pTeam = Team::sTeams[pTarget->GetTeam()];
        for (int i = 0; i < pTeam->mMemberCount; ++i)
        {
            const Character* pCharacter = pTeam->mMemberArray[i];
            if (!pCharacter || !pCharacter->mUnit)
            {
                continue;
            }
            const GameObject* pObj = pCharacter->mUnit->GetGameObject();
            if (pObj == pTarget)
            {
                continue;
            }
            const float fRadius = pObj->GetGameObjectClass()->mVORadius;
            const PblVector3& vObj = pObj->GetPosition();
            const PblVector3& vTarget = pTarget->GetPosition();
            const float fX = vTarget.x - vObj.x;
            const float fY = vTarget.y - vObj.y;
            const float fZ = vTarget.z - vObj.z;
            const float fDistSq = fY * fY + fX * fX + fZ * fZ;
            if (fRadius * fRadius > fDistSq && fClosestSq > fDistSq)
            {
                pSpeaker = pObj;
                fClosestSq = fDistSq;
            }
        }
        if (!pSpeaker)
        {
            return;
        }
    }

    const float fRadius = pSpeaker->GetGameObjectClass()->mVORadius;
    const PblVector3& vTarget = pTarget->GetPosition();
    const PblVector3& vSpeaker = pSpeaker->GetPosition();
    const float fX = vSpeaker.x - vTarget.x;
    const float fY = vSpeaker.y - vTarget.y;
    const float fZ = vSpeaker.z - vTarget.z;
    if (fY * fY + fX * fX + fZ * fZ > fRadius * fRadius)
    {
        return;
    }

    if (pSpeaker->GetAffiliationToMe(pTarget) < 0 && AIUtil::GetRandomInt(0, 100) > 30 &&
        eType != VOT_RebelsShootDeadBody)
    {
        return;
    }

    if (!pSpeaker->GetControllable())
    {
        return;
    }
    const Controller* pCtrl = pSpeaker->GetControllable()->mCtrl;
    if (!PlayerControllerPtr(pCtrl))
    {
        const UnitController* pUnit = UnitControllerPtr(pCtrl);
        if (!pUnit)
        {
            return;
        }
        if (!pSpeaker->IsRtti(EntityFlyer::rttiHashEntityFlyer) && pUnit->mLodState < 3)
        {
            return;
        }
    }

    const GameObject* pVoice = pSpeaker;
    Controllable* pSpeakerCtrl = const_cast<Controllable*>(pSpeaker->GetControllable());
    if (pSpeakerCtrl && pSpeakerCtrl->GetPilot())
    {
        pVoice = pSpeakerCtrl->GetPilot()->GetGameObject();
        if (!pVoice)
        {
            pVoice = pSpeaker;
        }
    }

    VOSound* pSounds[64];
    int iCount = 0;
    for (VOSound* pSound = pVoice->GetGameObjectClass()->mVOSoundList; pSound && iCount < 64;
         pSound = pSound->mNext)
    {
        if (pSound->mType == eType && HasRequiredModifiers(pSound, modifiers))
        {
            pSound->mScore = (unsigned char)GetModifierScore(pSound, modifiers);
            pSounds[iCount++] = pSound;
        }
    }
    if (iCount > 1)
    {
        qsort(pSounds, iCount, sizeof(pSounds[0]), VOSoundCompareCallback);
    }
    if (iCount <= 0)
    {
        return;
    }

    const bool bLocal = pSpeaker == pTarget || pVoice == pTarget;
    const PblVector3& vPos = pSpeaker->GetMatrix().GetTranslation();
    if (bLocal && pSounds[0]->mSound.GetPropertiesType() == GameSound::type_soundStream)
    {

        unsigned char* pProps = (unsigned char*)pSounds[0]->mSound.GetProperties();
        if (pProps)
        {
            pProps[0x50] &= ~0x40;
        }
    }
    pSounds[0]->mSound.Play(&vPos, 0, true, !bLocal);
}

// FUNCTION: SWBF2 0x006723a0
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const PblVector3& vPos, float fDelay)
{
    if ((eType == VOT_AcquiredTarget || eType == VOT_PathFollowWait) &&
        AIUtil::GetRandomInt(0, 100) > 70)
    {
        return -1;
    }

    if (fDelay > GameLoop::GetMissionTime())
    {
        return Queue(eType, pSpeaker, pTarget, 0, &vPos, fDelay);
    }

    if (!pTarget)
    {
        Character* pPlayer = NetGame::GetLocalPlayer(0);
        if (pPlayer && pPlayer->GetControllable())
        {
            pTarget = pPlayer->GetControllable()->GetGameObject();
        }
    }
    if (pTarget)
    {
        VOModifier modifiers = VOModifier();
        GetModifiers(pTarget, vPos, modifiers);
        _Play(eType, pSpeaker, pTarget, modifiers);
    }
    return -1;
}

// FUNCTION: SWBF2 0x00672480
int Play(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
         const GameObject* pOther, float fDelay)
{
    if ((eType == VOT_AcquiredTarget || eType == VOT_PathFollowWait) &&
        AIUtil::GetRandomInt(0, 100) > 70)
    {
        return -1;
    }

    if (fDelay > GameLoop::GetMissionTime())
    {
        return Queue(eType, pSpeaker, pTarget, pOther, 0, fDelay);
    }

    if (!pTarget)
    {
        Character* pPlayer = NetGame::GetLocalPlayer(0);
        if (pPlayer && pPlayer->GetControllable())
        {
            pTarget = pPlayer->GetControllable()->GetGameObject();
        }
    }
    if (pTarget)
    {
        _Play(eType, pSpeaker, pTarget, GetModifiers(pTarget, pOther));
    }
    return -1;
}

// FUNCTION: SWBF2 0x00672550
int Queue(VOType eType, const GameObject* pSpeaker, const GameObject* pTarget,
          const GameObject* pOther, const PblVector3* pPos, float fDelay)
{
    for (int i = 0; i < 20; ++i)
    {
        QueuedSound& rSound = mSndQueue[i];
        if (rSound.mType != VOT_NONE)
        {
            continue;
        }
        rSound.mType = eType;
        rSound.mSpeaker = pSpeaker;
        rSound.mPlayer = pTarget;
        rSound.mTarget = pOther;
        if (pPos)
        {
            rSound.mPos = *pPos;
        }
        rSound.mUseTarget = pPos == 0;
        rSound.mTime = fDelay;
        rSound.mHandle = mNextQueueHandle++;
        return rSound.mHandle;
    }
    return -1;
}

// FUNCTION: SWBF2 0x00672620
void CancelQueuedSound(int iHandle)
{
    for (int i = 0; i < 20; ++i)
    {
        if (mSndQueue[i].mHandle == iHandle)
        {
            mSndQueue[i].mType = VOT_NONE;
        }
    }
}

}
}

 
 
 

// FUNCTION: SWBF2 0x00671940
int AI::VOHelper::TypeStrToEnum(const char* pszType)
{
    PblHash hash(pszType);
    for (int i = 0; i < 48; ++i)
    {
        if (hash == VOTypeNames[i])
        {
            return i;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00671970
AI::VOSound* AI::VOHelper::GetNewVOSound(const char* pszSound, const char* pszType, const AI::VOModifier& modifiers)
{
    using namespace AI::VOHelper;
    if (!sLoadingBatch)
    {
        sLoadingBatch = new AI::VOSound[16];
        sLoadingBatchIndex = 0;
    }
    AI::VOSound* pSound = &sLoadingBatch[sLoadingBatchIndex];

    int iType = TypeStrToEnum(pszType);
    pSound->mSound.SetID(pszSound, false, false);
    pSound->mType = iType;
    pSound->mModifiers = modifiers;
    pSound->mNext = 0;

    if (++sLoadingBatchIndex >= 16)
    {
        sLoadingBatch = 0;
    }
    return pSound;
}

// FUNCTION: SWBF2 0x00671a60
AI::VOSound::VOSound()
    : mModifiers()
{
}

// FUNCTION: SWBF2 0x006715f0
AI::VOHelper::QueuedSound::~QueuedSound()
{
}

namespace AI
{

namespace VOHelper
{

// SYNTHETIC: SWBF2 0x006715c0
// AI::VOHelper::QueuedSound::QueuedSound

// GLOBAL: SWBF2 0x01f9a320
// SYNTHETIC: SWBF2 0x00406f50
// `dynamic initializer for 'AI::VOHelper::mSndQueue''
// SYNTHETIC: SWBF2 0x00769780
// `dynamic atexit destructor for 'AI::VOHelper::mSndQueue''
QueuedSound mSndQueue[20];

// GLOBAL: SWBF2 0x01eafd30
int mNextQueueHandle;
// GLOBAL: SWBF2 0x01eafd34
VOSound* sLoadingBatch;
// GLOBAL: SWBF2 0x01eafd38
int sLoadingBatchIndex;

// FUNCTION: SWBF2 0x00671600
void Reset()
{
    sLoadingBatchIndex = 0;
    sLoadingBatch = 0;
    memset(mSndQueue, 0, sizeof(mSndQueue));
    mNextQueueHandle = 1;
}

}
}
