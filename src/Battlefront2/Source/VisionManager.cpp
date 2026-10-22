#include "VisionManager.h"
#include "AIUtil.h"
#include "NetGame.h"
#include "UnitController.h"
#include "Character.h"
#include "CombatUtil.h"
#include "Controllable.h"
#include "Controller.h"
#include "Entity.h"
#include "EntityBuildingArmed.h"
#include "EventInfo.h"
#include "EntityHover.h"
#include "EntityWalker.h"
#include "GameObject.h"
#include "MountedTurret.h"
#include "UnitAttackPatterns.h"
#include "UnitThreatManager.h"
#include "WeaponCannonInstance.h"
#include "WeaponTowCable.h"

#include <math.h>

class EntityFlyer
{
public:
    static PblHash rttiHashEntityFlyer;
};

// TEMPLATE: SWBF2 0x00671220
// PblHeap<VisionManager::RayRequest *,int>::FindLowest
template int PblHeap<VisionManager::RayRequest*, int>::FindLowest();

// TEMPLATE: SWBF2 0x00671460
// PblHeap<PblHandle<GameObject>,int>::FindLowest
template int PblHeap<PblHandle<GameObject>, int>::FindLowest();

namespace AI
{

    enum AIAffil
    {
        AFFIL_ENEMY = 1,
        AFFIL_NEUTRAL = 2,
        AFFIL_ALLY = 4,
        AFFIL_ALLYNEUTRAL = 6,
        AFFIL_ALL = 7
    };
}

namespace TeamManager
{

    int GetObjectsInRange(const PblVector3& vPos, float fRange, GameObject** const ppObjects, int iMax,
                          const GameObject* pObject, AI::AIAffil eAffil);
}

namespace CombatUtil
{

    bool SelectCombatResponse(Controllable* pMe, const GameObject* pTarget,
                              AttackPatterns::State* pState, float* pTime,
                              bool* pbFlag, bool bA, bool bB);

    bool CanTurretFaceDisturbance(const Controllable* pMe, const MountedTurretClass* pClass,
                                  const GameObject* pObj, const PblVector3& vPos);
}

// GLOBAL: SWBF2 0x01f9a2c8
// SYNTHETIC: SWBF2 0x00406f30
// `dynamic initializer for 'VisionManager::RayRequest::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769770
// `dynamic atexit destructor for 'VisionManager::RayRequest::sMemoryPool''
MemoryPool VisionManager::RayRequest::sMemoryPool("RayRequest", 0x14);

namespace VisionManager
{

// GLOBAL: SWBF2 0x01eafd00
int mCastHistory[8];

// GLOBAL: SWBF2 0x01eafd28
int mCurCast;

}

// GLOBAL: SWBF2 0x01faa754
int gRayRequestRefusalCount;

namespace VisionManager
{

// GLOBAL: SWBF2 0x007e9ee4
float mGlobalVisibilityModifier = 1.0f;

// FUNCTION: SWBF2 0x00670270
void SetGlobalVisModifier(float fModifier)
{
    mGlobalVisibilityModifier = fModifier;
}

// FUNCTION: SWBF2 0x00670280
void Initialize()
{
    sPVRect = new GameObject*[1200];
    sHeap1 = new PblHeap<RayRequest*, int>(200, 300, 0);
    sHeap2 = new PblHeap<RayRequest*, int>(200, 300, 0);
    sHeap = sHeap1;
    sTmpHeap = sHeap2;
    SetGlobalVisModifier(1.0f);
    for (int i = 0; i < 8; ++i)
    {
        mCastHistory[i] = 100;
    }
}

// FUNCTION: SWBF2 0x00670400
float MaxVisibleDist(const GameObject* pSeer, const GameObject* pTarget)
{
    float fDist = pTarget->GetCollisionObject()->GetCollisionRadius() * 20.0f;
    float fModifier = mGlobalVisibilityModifier;
    if (pTarget->GetControllable() && pTarget->GetControllable()->mCtrl)
    {
        fModifier *= pTarget->GetControllable()->mCtrl->mTotalVisFactor;
    }
    if (pTarget->GetControllable() && pTarget->GetControllable()->GetPlayerId() >= 0)
    {
        fModifier *= 2.0f;
    }
    if (pSeer)
    {
        if (pSeer->GetEntity()->IsRtti(EntityFlyer::rttiHashEntityFlyer))
        {
            fModifier *= 2.0f;
        }
        else if (pSeer->GetEntity()->IsRtti(EntityBuildingArmed::rttiHashEntityBuildingArmed))
        {
            fModifier *= 10.0f;
        }
    }
    return (fDist + 30.0f) * fModifier;
}

// FUNCTION: SWBF2 0x00670520
void UpdatePotentiallyVisible(UnitController* pController)
{
    typedef PblHeap<PblHandle<GameObject>, int> VisHeap;
    char heapMem[(150 + 1) * sizeof(VisHeap::RECORD)];
    VisHeap heap(150, 120000, heapMem);

    Controllable* pMe = pController->mOwner;
    GameObject* pMeObj = pMe->GetGameObject();
    PblVector3 vEyePos = pMe->mEyePoint;

    GameObject* pPlayerTarget = 0;
    if (pController->mLodHumanPlayer && pController->mLodHumanPlayer->GetControllable())
    {
        pPlayerTarget = const_cast<GameObject*>(
            (const GameObject*)pController->mLodHumanPlayer->GetControllable()->mReticuleTarget[0]);
    }

    int iPriority = heap.GetMaxSize();
    int iNumObjects;
    if ((pMeObj->IsRtti(EntityWalker::rttiHashEntityWalker) ||
         pMeObj->IsRtti(EntityHover::rttiHashEntityHover)) &&
        pMeObj->GetControllable()->GetPilot() &&
        pMeObj->GetControllable()->GetPilot()->GetPlayerId() >= 0 &&
        pPlayerTarget &&
        AIUtil::IsValidEnemy(pMeObj, pPlayerTarget) &&
        pMe->GetEntity()->GetEntityClass()->IsRtti(MountedTurretClass::rttiHashMountedTurretClass) &&
        CombatUtil::CanTurretFaceDisturbance(
            pMe, static_cast<const MountedTurretClass*>(pMe->GetEntity()->GetEntityClass()), pMeObj,
            pPlayerTarget->GetMatrix().GetTranslation()))
    {
        iNumObjects = 1;
        sPVRect[0] = pPlayerTarget;
    }
    else
    {
        iNumObjects = TeamManager::GetObjectsInRange(vEyePos, CombatUtil::GetMaxCombatRange(pMe), sPVRect,
                                                     1200, pMeObj, AI::AFFIL_ENEMY);
    }

    for (int i = 0; i < iNumObjects; ++i)
    {
        GameObject* pObj = sPVRect[i];
        if (!AIUtil::IsValidEnemy(pMeObj, pObj))
        {
            continue;
        }
        if (pObj->GetCollisionObject() && pObj->GetCollisionObject()->GetCollisionType() == COLL_ATTACHED)
        {
            continue;
        }

        float fDistSq = -1.0f;
        if (!(pController->mLodState == 4 && pObj == pPlayerTarget) && !AIUtil::CanPotentiallySee(pMe, pObj, &fDistSq))
        {
            continue;
        }
        float fMaxDist = MaxVisibleDist(pMeObj, pObj);
        if (!(fMaxDist * fMaxDist > fDistSq))
        {
            continue;
        }

        AttackPatterns::State eState;
        float fTime;
        bool bFallback = false;
        bool bResponse = CombatUtil::SelectCombatResponse(pMe, pObj, &eState, &fTime, &bFallback, false, false);
        if (!bResponse && !bFallback)
        {
            continue;
        }

        int iVisPriority = GetVisualPriority(pMe, pObj);
        if (iVisPriority < 1)
        {
            iVisPriority = 1;
        }
        if (!bResponse && bFallback)
        {
            iVisPriority += 60000;
        }
        int iKey = 120000 - iVisPriority;
        PblHandle<GameObject> hObj(pObj);
        if (heap.GetCount() == iPriority)
        {

            int iLowest = heap.FindLowest();
            if (iKey > heap.Value(iLowest))
            {
                heap.ReplaceLowest(hObj, iKey, iLowest);
            }
        }
        else
        {
            heap.Push(hObj, iKey);
        }
    }

    AI::UnitThreatManager& threats = *reinterpret_cast<AI::UnitThreatManager*>(pController->mThreatManager);
    for (int i = 0; i < AI::UnitThreatManager::MAX_THREATS; ++i)
    {
        const AI::Threat& threat = threats.GetThreat(i);
        if (threat.GetThreatStatus() != AI::TS_SEEN)
        {
            continue;
        }
        GameObject* pThreat = threat.GetThreatObject();
        int j;
        for (j = 0; j < heap.GetCount(); ++j)
        {
            if (heap.Get(j) == pThreat)
            {
                break;
            }
        }
        if (j >= heap.GetCount())
        {
            threats.RayhitResult(pThreat, false);
        }
    }

    int iNumRequests = 0;
    while (!heap.IsEmpty())
    {
        PblHandle<GameObject> hObj = heap.Get(0);
        heap.Pop();
        if (threats.ShouldRaytestUnit(hObj))
        {

            int iMaxRequests[5] = { 1, 1, 2, 4, 5 };
            if (AIUtil::IsUberMode())
            {
                iMaxRequests[0] = 0;
                iMaxRequests[1] = 0;
                iMaxRequests[2] = 1;
                iMaxRequests[3] = 2;
                iMaxRequests[4] = 3;
            }
            if (iNumRequests < iMaxRequests[pController->mLodState])
            {
                AddRayRequest(pController, hObj, iPriority--);
                ++iNumRequests;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00670e80
int GetRelativePriority(const GameObject* pMe, const GameObject* pObj)
{
    AIUtil::EntityType eMe = AIUtil::GetEntityType(pMe);
    AIUtil::EntityType eObj = AIUtil::GetEntityType(pObj);
    switch (eMe)
    {
    case AIUtil::ENTITY_SOLDIER:
    case AIUtil::ENTITY_DROID:
    case AIUtil::ENTITY_DROIDEKA:
        switch (eObj)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROIDEKA:
        case AIUtil::ENTITY_BUILDING:
            return 10;
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_TAUNTAUN:
            return 5;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_BUILDING_ARMED:
            return 20;
        case AIUtil::ENTITY_FLYER:
            return 3;
        case AIUtil::ENTITY_HOVER:
            return 15;
        }
        break;
    case AIUtil::ENTITY_WALKER:
    case AIUtil::ENTITY_TAUNTAUN:
    case AIUtil::ENTITY_HOVER:
    case AIUtil::ENTITY_BUILDING_ARMED:
        switch (eObj)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROIDEKA:
            return 5;
        case AIUtil::ENTITY_DROID:
        case AIUtil::ENTITY_TAUNTAUN:
            return 1;
        case AIUtil::ENTITY_WALKER:
            return 20;
        case AIUtil::ENTITY_FLYER:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING_ARMED:
            return 10;
        case AIUtil::ENTITY_BUILDING:
            return 8;
        }
        break;
    case AIUtil::ENTITY_FLYER:
        switch (eObj)
        {
        case AIUtil::ENTITY_SOLDIER:
        case AIUtil::ENTITY_DROIDEKA:
            return 5;
        case AIUtil::ENTITY_DROID:
            return 1;
        case AIUtil::ENTITY_WALKER:
        case AIUtil::ENTITY_FLYER:
            return 20;
        case AIUtil::ENTITY_TAUNTAUN:
        case AIUtil::ENTITY_HOVER:
        case AIUtil::ENTITY_BUILDING:
            return 10;
        case AIUtil::ENTITY_BUILDING_ARMED:
            return 15;
        }
        break;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00670fb0
int GetVisualPriority(const Controllable* pMe, const GameObject* pObj)
{
    int iPriority = GetRelativePriority(pMe->GetGameObject(), pObj);
    for (int i = 0; i < pMe->GetWeaponCount(); ++i)
    {
        if (pMe->GetWeaponPtr(i) &&
            pMe->GetWeaponPtr(i)->mClass->IsRtti(WeaponTowCableClass::rttiHashWeaponTowCableClass))
        {
            if (!pObj->GetControllable() || !AIUtil::IsATAT(pObj->GetControllable()))
            {
                iPriority = 0;
            }
        }
    }

    PblVector3 vEyePos = pMe->mEyePoint;
    PblVector3 vTarget = pObj->GetTargetPoint(vEyePos, pMe->mEyeDir, -1);
    float fDx = vTarget.x - vEyePos.x;
    float fDy = vTarget.y - vEyePos.y;
    float fDz = vTarget.z - vEyePos.z;
    int iDist = (int)sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);
    if (iDist > 1500)
    {
        iDist = 1500;
    }
    int iKey = (21 - iPriority) * iDist;

    if (pObj->GetControllable() && pObj->GetControllable()->GetPlayerId() >= 0 &&
        !AIUtil::IsVehicle(pMe->GetGameObject()))
    {
        return iKey;
    }
    return (int)((float)iKey * 2.0f);
}

// FUNCTION: SWBF2 0x00670200
RayRequest::RayRequest(UnitController* pCtrl, GameObject* pHim, int iPriority)
{
    mCtrl = pCtrl;
    mHim = pHim;
    mPriority = iPriority;
}

// FUNCTION: SWBF2 0x00670a60
bool VisionTest(UnitController* pController, GameObject* pTarget)
{
    if (!pController || !pTarget)
    {
        return false;
    }

    PblVector3 vSeenPos;
    bool bCanSee = AIUtil::CanSee(pController->mOwner, pTarget, &vSeenPos);
    if (bCanSee && !pController->IsNewCommand())
    {
        AI::EventInfo info(AI::EVT_Vision_CanSeeEnemy, pTarget, vSeenPos);
        pController->SendEvent(&info);
    }
    reinterpret_cast<AI::UnitThreatManager*>(pController->mThreatManager)->RayhitResult(pTarget, bCanSee);

    UnitController* pHim = UnitControllerPtr(pTarget->mControllableList.GetFirst()->_pObject);
    if (pHim && bCanSee)
    {
        AI::EventInfo info(AI::EVT_Vision_CanSeeEnemy, pController->mOwner->GetGameObject());
        if (!pHim->IsNewCommand() && pHim->SendEvent(&info) == AI::EVT_Handled)
        {
            reinterpret_cast<AI::UnitThreatManager*>(pHim->mThreatManager)
                ->ForceThreat(pController->mOwner->GetGameObject());
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00670c60
void AddRayRequest(UnitController* pController, GameObject* pObj, int iPriority)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    if (!sHeap->IsFull())
    {
        RayRequest* pRequest = new RayRequest(pController, pObj, iPriority);
        sHeap->Push(pRequest, pRequest->mPriority);
        return;
    }

    int iLowest = sHeap->FindLowest();
    if (iPriority > sHeap->Value(iLowest))
    {
        RayRequest* pRequest = new RayRequest(pController, pObj, iPriority);
        RayRequest* pOld = sHeap->Get(iLowest);
        sHeap->ReplaceLowest(pRequest, pRequest->mPriority, iLowest);
        delete pOld;
    }
    else
    {
        ++gRayRequestRefusalCount;
    }
}

// FUNCTION: SWBF2 0x00670d70
void RemoveRequests(UnitController* pController)
{
    sTmpHeap->Flush();
    while (!sHeap->IsEmpty())
    {
        RayRequest* pRequest = sHeap->Get(0);
        sHeap->Pop();
        UnitController* pCtrl = pRequest->mCtrl;
        if (pCtrl != pController && AIUtil::GetAlivePtr(pRequest->mHim, false) && pCtrl)
        {
            sTmpHeap->Push(pRequest, pRequest->mPriority);
        }
        else
        {
            delete pRequest;
        }
    }
    PblHeap<RayRequest*, int>* pHeap = sHeap;
    sHeap = sTmpHeap;
    sTmpHeap = pHeap;
}

}

// FUNCTION: SWBF2 0x00671280
template PblHeap<PblHandle<GameObject>, int>::PblHeap(int, int, void*);

// FUNCTION: SWBF2 0x006714e0
template void PblHeap<VisionManager::RayRequest*, int>::SortDown(int, VisionManager::RayRequest*&, int);

// FUNCTION: SWBF2 0x00671540
template void PblHeap<PblHandle<GameObject>, int>::SortDown(int, PblHandle<GameObject>&, int);

// FUNCTION: SWBF2 0x006713a0
template void PblHeap<PblHandle<GameObject>, int>::Pop();

// TEMPLATE: SWBF2 0x00671190
// PblHeap<VisionManager::RayRequest *,int>::Pop
template void PblHeap<VisionManager::RayRequest*, int>::Pop();

// FUNCTION: SWBF2 0x00671130
void __fastcall VisionManager::AddFakeRayhitResult(UnitController* pUnit, const GameObject* pObject)
{
    if (!pUnit || !pObject || !pUnit->mOwner)
    {
        return;
    }
    GameObject* pMe = pUnit->mOwner->GetGameObject();
    if (!pMe || pMe->GetAffiliationToMe(pObject) >= 0)
    {
        return;
    }

    GameObject* pHim = const_cast<GameObject*>(pObject);
    reinterpret_cast<AI::UnitThreatManager*>(pUnit->mThreatManager)->ForceThreat(pHim);
    AI::EventInfo info(AI::EVT_Vision_CanSeeEnemy, pHim);
    pUnit->SendEvent(&info);
}

namespace VisionManager { bool RayTests(int iMaxHits); }

// FUNCTION: SWBF2 0x00670b40
bool VisionManager::RayTests(int iMaxHits)
{
    using namespace VisionManager;

    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return false;
    }

    int iTotal = 0;
    for (int i = 0; i < 8; ++i)
    {
        iTotal += mCastHistory[i];
    }
    int iLimit = iTotal / 8 + 1;
    if (iMaxHits < iLimit)
    {
        iLimit = iMaxHits;
    }

    bool bHit = false;
    int iHits = 0;
    while (iHits < iLimit && !sHeap->IsEmpty())
    {
        RayRequest* pRequest = sHeap->Get(0);
        sHeap->Pop();
        UnitController* pController = pRequest->mCtrl;
        if (VisionTest(pController, AIUtil::GetAlivePtr(pRequest->mHim, false)))
        {
            ++iHits;
            bHit = true;
        }
        delete pRequest;
    }

    mCastHistory[mCurCast] = iHits;
    mCurCast = (mCurCast + 1) % 8;
    return bHit;
}

// TEMPLATE: SWBF2 0x00671370
// PblHeap<PblHandle<GameObject>,int>::Get
template PblHandle<GameObject> PblHeap<PblHandle<GameObject>, int>::Get(int);

namespace VisionManager {

// FUNCTION: SWBF2 0x006703c0
void Reset()
{
    sPVRect = 0;
    sHeap1 = 0;
    sHeap2 = 0;
    sHeap = 0;
    sTmpHeap = 0;
}
}

// TEMPLATE: SWBF2 0x006714b0
// PblHeap<PblHandle<GameObject>,int>::RECORD::RECORD
// TEMPLATE: SWBF2 0x006714d0
// PblHeap<PblHandle<GameObject>,int>::RECORD::~RECORD
 

namespace VisionManager {

// GLOBAL: SWBF2 0x01eafd24
GameObject** sPVRect;
}

namespace VisionManager {
// GLOBAL: SWBF2 0x01eafd20
PblHeap<RayRequest*, int>* sHeap1;
}

namespace VisionManager {
// GLOBAL: SWBF2 0x01eafd2c
PblHeap<RayRequest*, int>* sHeap2;
}

namespace VisionManager {
// GLOBAL: SWBF2 0x01eafcfc
PblHeap<RayRequest*, int>* sHeap;
}

namespace VisionManager {
// GLOBAL: SWBF2 0x01eafcf8
PblHeap<RayRequest*, int>* sTmpHeap;
}
