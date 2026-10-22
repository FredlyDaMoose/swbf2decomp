#include <float.h>
#include <string.h>

#include "UnitThreatManager.h"
#include "AIDifficulty.h"
#include "AIUtil.h"
#include "Controllable.h"
#include "ControllerManager.h"
#include "Entity.h"
#include "FlagItem.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "PlayerController.h"
#include "UnitController.h"
#include "VisionManager.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

namespace AI
{

// FUNCTION: SWBF2 0x006698b0
Threat::Threat()
{
    Reset();
}

// FUNCTION: SWBF2 0x00669950
void Threat::Reset()
{
    mThreatObject = 0;
    mThreatStatus = TS_UNKNOWN;
    mLastStatusTime = 0.0f;
    mLastSeenTime = 0.0f;
    mLastSeenPos = PblVector3(0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x006698d0
Threat::Threat(GameObject* pObj, ThreatStatus eStatus)
{
    Reset();
    mThreatObject = pObj;
    mThreatStatus = eStatus;
    mLastStatusTime = GameLoop::GetMissionTime();
    if (eStatus == TS_SEEN || eStatus == TS_FORCEDSEEN || eStatus == TS_LOCKED)
    {
        mLastSeenTime = GameLoop::GetMissionTime();
        mLastSeenPos = pObj->GetMatrix().GetTranslation();
    }
}

// FUNCTION: SWBF2 0x006699a0
unsigned char Threat::GetPriority(float fTime, Controllable* pMe) const
{
    if (!mThreatObject)
    {
        return 0;
    }

    const PblMatrix& mMe = pMe->GetMatrix();
    const PblVector3& vHim = mThreatObject->GetPosition();
    PblVector3 vToMe(mMe.GetTranslation().x - vHim.x, mMe.GetTranslation().y - vHim.y,
                     mMe.GetTranslation().z - vHim.z);
    PblVector3 vDir;
    D3DXVec3Normalize(&vDir, &vToMe);

    if (pMe->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
    {
        const PblVector3& vFwd = mMe.GetZAxis();
        if (-vDir.x * vFwd.x + -vDir.y * vFwd.y + -vDir.z * vFwd.z <= 0.0f)
        {
            return 0;
        }
    }

    if (mThreatStatus == TS_LOCKED)
    {
        return (fTime - mLastSeenTime < 4.0f) ? 255 : 0;
    }

    int iPriority = 18 * mThreatStatus + 100;
    int iVisual = 100 - (int)((float)VisionManager::GetVisualPriority(pMe, mThreatObject) * 100.0f /
                              (float)60000);

    if (!mThreatObject->mControllableList.GetFirst()->_pObject ||
        !PlayerControllerPtr(mThreatObject->mControllableList.GetFirst()->_pObject->mCtrl))
    {
        iVisual /= 2;
    }
    else
    {
        const PblVector3& vHisFwd = mThreatObject->GetMatrix().GetZAxis();
        float fFacing = vDir.x * vHisFwd.x + vDir.y * vHisFwd.y + vDir.z * vHisFwd.z;
        float fMin = AIDifficulty::GetPlayerThreatAngle() * 2.0f - 1.0f;
        if (fMin < 0.0f)
        {
            fMin = 0.0f;
        }
        float fScale = (fFacing - 0.6f) / 0.4f;
        if (fScale < fMin)
        {
            fScale = fMin;
        }
        else if (fScale > 1.0f)
        {
            fScale = 1.0f;
        }
        iVisual = (int)((float)iVisual * fScale);
    }

    if (mThreatObject->mControllableList.GetFirst()->_pObject)
    {
        for (PblList<FlagItem>::Itor it(*FlagItem::sList); (FlagItem*)it; ++it)
        {
            if (reinterpret_cast<EntityItem*>((FlagItem*)it)->mOwner ==
                mThreatObject->mControllableList.GetFirst()->_pObject->mCharacter)
            {
                iVisual = 100;
                break;
            }
        }
    }

    iPriority += iVisual;
    float fSinceSeen = fTime - mLastSeenTime;
    if (mThreatStatus != TS_UNKNOWN)
    {
        iPriority += (int)(fSinceSeen * -10.0f);
    }
    if (mThreatStatus == TS_FORCEDSEEN)
    {
        iPriority += (int)(fSinceSeen * -10.0f);
    }
    return (unsigned char)(iPriority > 0 ? iPriority : 0);
}

// FUNCTION: SWBF2 0x00669d70
Threat::~Threat()
{
}

// FUNCTION: SWBF2 0x00669d80
UnitThreatManager::~UnitThreatManager()
{
}

// FUNCTION: SWBF2 0x00669d30
UnitThreatManager::UnitThreatManager()
{
    memset(mThreats, 0, sizeof(mThreats));
    mMyCtrl = 0;
    mVisibleObjectIndex = -1;
}

// FUNCTION: SWBF2 0x00669da0
void UnitThreatManager::Update(float dt, UnitController* pCtrl)
{
    mMyCtrl = pCtrl;
    float fTime = GameLoop::GetMissionTime();

    for (int i = 0; i < MAX_THREATS; ++i)
    {
        Threat& threat = mThreats[i];
        if (threat.mThreatObject && !threat.GetPriority(fTime, mMyCtrl->mOwner))
        {
            threat.mThreatObject = 0;
        }
    }

    if (fTime > AIUtil::GetThreatSwitchTime(mMyCtrl) + mLastVisObjUpdateTime)
    {
        int iBest = GetHighestPriorityIndex(false);
        if (iBest != mVisibleObjectIndex)
        {
            mLastVisObjUpdateTime = fTime;
            if ((unsigned int)iBest >= MAX_THREATS || mVisibleObjectIndex < 0 ||
                mVisibleObjectIndex >= MAX_THREATS)
            {
                mVisibleObjectIndex = iBest;
            }
            else
            {
                Controllable* pMe = mMyCtrl->mOwner;
                unsigned char uCur = mThreats[mVisibleObjectIndex].GetPriority(fTime, pMe);
                if (mThreats[iBest].GetPriority(fTime, pMe) > uCur + 10)
                {
                    mVisibleObjectIndex = iBest;
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0066a1a0
const GameObject* UnitThreatManager::GetVisibleObject() const
{
    if (mVisibleObjectIndex < 0)
    {
        return 0;
    }
    return mThreats[mVisibleObjectIndex].mThreatObject;
}

// FUNCTION: SWBF2 0x0066a1e0
const PblVector3& UnitThreatManager::GetVisibleObjectPos() const
{
    static PblVector3 sZero(0.0f, 0.0f, 0.0f);
    if (mVisibleObjectIndex < 0)
    {
        return sZero;
    }
    return mThreats[mVisibleObjectIndex].mLastSeenPos;
}

// FUNCTION: SWBF2 0x0066a230
int UnitThreatManager::GetHighestPriorityIndex(bool) const
{
    int iBest = -1;
    int iBestPriority = -1;
    float fTime = GameLoop::GetMissionTime();

    for (int i = 0; i < MAX_THREATS; ++i)
    {
        const Threat& threat = mThreats[i];
        if (!threat.mThreatObject)
        {
            continue;
        }
        ThreatStatus eStatus = threat.mThreatStatus;
        if (eStatus != TS_SEEN && eStatus != TS_FORCEDSEEN && eStatus != TS_LOCKED)
        {
            continue;
        }
        int iPriority = threat.GetPriority(fTime, mMyCtrl->mOwner);
        if (iPriority > iBestPriority)
        {
            iBestPriority = iPriority;
            iBest = i;
        }
    }
    return iBest;
}

// FUNCTION: SWBF2 0x00669f40
void UnitThreatManager::RayhitResult(GameObject* pObj, bool bHit)
{
    UpdateThreat(pObj, bHit ? TS_SEEN : TS_UNSEEN);
}

// FUNCTION: SWBF2 0x00669f60
void UnitThreatManager::ForceThreat(GameObject* pObj)
{
    UpdateThreat(pObj, TS_FORCEDSEEN);
}

// FUNCTION: SWBF2 0x00669f80
void UnitThreatManager::LockThreat(GameObject* pObj)
{
    UpdateThreat(pObj, TS_LOCKED);
}

// FUNCTION: SWBF2 0x00669fa0
void UnitThreatManager::AudibleResult(GameObject* pObj)
{
    UpdateThreat(pObj, TS_UNKNOWN);
}

// FUNCTION: SWBF2 0x00669fc0
void UnitThreatManager::UpdateThreat(GameObject* pObj, ThreatStatus eStatus)
{
    if (!mMyCtrl || !pObj)
    {
        return;
    }

    int iIndex = -1;
    float fTime = GameLoop::GetMissionTime();
    Threat newThreat(pObj, eStatus);
    unsigned char uNewPriority = newThreat.GetPriority(fTime, mMyCtrl->mOwner);

    for (int i = 0; i < MAX_THREATS; ++i)
    {
        if (mThreats[i].mThreatObject == pObj)
        {
            if (eStatus == TS_UNKNOWN && mThreats[i].mThreatStatus > TS_UNKNOWN)
            {
                return;
            }
            iIndex = i;
            break;
        }
    }

    if (iIndex < 0)
    {
        if (eStatus == TS_UNSEEN)
        {
            return;
        }

        int iWorst = -1;
        unsigned char uWorst = uNewPriority;
        for (int i = 0; i < MAX_THREATS; ++i)
        {
            if (!mThreats[i].mThreatObject && uWorst > 0)
            {
                uWorst = 0;
                iWorst = i;
            }
            else if (mThreats[i].GetPriority(fTime, mMyCtrl->mOwner) < uWorst)
            {
                iWorst = i;
                uWorst = mThreats[i].GetPriority(fTime, mMyCtrl->mOwner);
            }
        }
        if (iWorst >= 0)
        {
            mThreats[iWorst].Reset();
            iIndex = iWorst;
        }
        if (iIndex < 0)
        {
            return;
        }
    }

    Threat& threat = mThreats[iIndex];
    threat.mThreatObject = pObj;
    threat.mLastStatusTime = GameLoop::GetMissionTime();
    threat.mThreatStatus = eStatus;
    if (eStatus == TS_SEEN || eStatus == TS_FORCEDSEEN || eStatus == TS_LOCKED)
    {
        threat.mLastSeenTime = GameLoop::GetMissionTime();
        threat.mLastSeenPos = pObj->GetMatrix().GetTranslation();
    }

    if (PlayerControllerPtr(pObj->GetControllable()))
    {
        ControllerManager::RequestControllerUpdateASAP(mMyCtrl);
    }
}

// FUNCTION: SWBF2 0x0066a2d0
float UnitThreatManager::TimeSinceLastRaytest(const GameObject* pObj) const
{
    for (int i = 0; i < MAX_THREATS; ++i)
    {
        if ((const GameObject*)mThreats[i].mThreatObject == pObj)
        {
            return GameLoop::GetMissionTime() - mThreats[i].mLastStatusTime;
        }
    }
    return FLT_MAX;
}

// FUNCTION: SWBF2 0x0066a340
bool UnitThreatManager::ShouldRaytestUnit(const GameObject* pObj) const
{
    bool bPlayer = false;
    Controllable* pHim = pObj->mControllableList.GetFirst()->_pObject;
    if (pHim && pHim->mCtrl)
    {
        bPlayer = PlayerControllerPtr(pHim->mCtrl) != 0;
    }

    float fRaytestInterval[5] = { 4.0f, 4.0f, 2.0f, 1.0f, 1.0f };
    if (!bPlayer)
    {
        const GameObject* pVisible = GetVisibleObject();
        if (!pVisible)
        {
            return true;
        }
        Controllable* pVisCtl = pVisible->mControllableList.GetFirst()->_pObject;
        if (pVisCtl && pVisCtl->mCtrl && PlayerControllerPtr(pVisCtl->mCtrl))
        {
            return false;
        }
    }
    return TimeSinceLastRaytest(pObj) >= fRaytestInterval[mMyCtrl->mLodState];
}

}

// FUNCTION: SWBF2 0x00669eb0
void AI::UnitThreatManager::AddThreat(GameObject* pObj)
{
    Controllable* pHim = pObj->mControllableList.GetFirst()->_pObject;
    if (pHim && pHim->mPlayerId >= 0)
    {
        if (GetVisibleObject() &&
            GetVisibleObject()->mControllableList.GetFirst()->_pObject &&
            GetVisibleObject()->mControllableList.GetFirst()->_pObject->mPlayerId < 0)
        {
            ControllerManager::RequestControllerUpdateASAP(mMyCtrl);
        }
        UpdateThreat(pObj, AI::TS_SEEN);
    }
    else
    {
        UpdateThreat(pObj, AI::TS_UNKNOWN);
    }
}
