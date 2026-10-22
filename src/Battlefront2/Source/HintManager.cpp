#include "HintManager.h"

#include <float.h>
#include <stdio.h>

#include "PebbleFL/Common/PblMatrix.h"

#include "AIUtil.h"
#include "CombatUtil.h"
#include "CommandPost.h"
#include "CommandPostManager.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "TreeGrid.h"
#include "UnitController.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

#include "AIGoalManager.h"
#include "ListPool.h"
#include "PebbleFL/Common/PblHeap.h"
#include "NetGame.h"
#include "TeamManager.h"

// GLOBAL: SWBF2 0x01ec6090
// SYNTHETIC: SWBF2 0x00403f20
// `dynamic initializer for 'BaseHint::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768b10
// `dynamic atexit destructor for 'BaseHint::sMemoryPool''
MemoryPool BaseHint::sMemoryPool("BaseHint", 0x40);

extern int WORLD_EXTENTS;

// FUNCTION: SWBF2 0x005438f0
void HintManager::AddHint(BaseHint* pHint)
{
    if (pHint->GetType() == BaseHint::HINT_LAND && !pHint->GetCommandPost())
    {
        const PblVector3 vPos = pHint->GetCollisionCenter();
        const CommandPost* pBest = 0;
        float fBestDistSq = FLT_MAX;
        for (int i = 0; i < CommandPostManager::Instance()->GetCount(); ++i)
        {
            const CommandPost* pPost = CommandPostManager::Instance()->FindPostFromIndex(i);
            const PblVector3& vPost = pPost->GetMatrix().GetTranslation();
            const float fDX = vPost.x - vPos.x;
            const float fDY = vPost.y - vPos.y;
            const float fDZ = vPost.z - vPos.z;
            const float fDistSq = fDY * fDY + fDX * fDX + fDZ * fDZ;
            if (fDistSq < fBestDistSq)
            {
                fBestDistSq = fDistSq;
                pBest = pPost;
            }
        }
        pHint->SetCommandPost(pBest);
    }

    pHint->SetCollisionCenter(pHint->GetCollisionCenter());
    pHint->SetCollisionRadius(1.0f);
    pHint->Set2DCollisionRadius(1.0f);
    sHints->Add(pHint, pHint->GetCollisionCenter(), pHint->GetCollisionRadius());
}

// FUNCTION: SWBF2 0x00543830
void HintManager::Initialize()
{
    sHints = new TreeGrid(PblVector3((float)-WORLD_EXTENTS, 0.0f, (float)-WORLD_EXTENTS),
                          PblVector3((float)WORLD_EXTENTS, 0.0f, (float)WORLD_EXTENTS), 16.0f);
}

// FUNCTION: SWBF2 0x00543360
BaseHint::BaseHint(const HintDesc& desc, Type eType)
    : TreeGridObject(-1),
      mForward(desc.mMatrix.GetZAxis())
{
    SetCollisionCenter(desc.mMatrix.GetTranslation());
    mType = eType;
    mPrimaryStance = 1;
    mSecondaryStance = 1;
    mSidestep = 0;
}

// FUNCTION: SWBF2 0x00543460
BaseHint::Type BaseHint::GetType() const
{
    return (Type)mType;
}

// FUNCTION: SWBF2 0x00543400
float BaseHint::GetReserveTime() const
{
    return mType == HINT_MINE ? 10.0f : -1.0f;
}

// FUNCTION: SWBF2 0x00543420
bool BaseHint::IsReserved(int iTeam) const
{
    return mReserve.IsReserved(iTeam, GetReserveTime());
}

// FUNCTION: SWBF2 0x00543430
void BaseHint::Reserve(const GameObject* pObj) const
{
    const_cast<ReserveObject&>(mReserve).Reserve(pObj);
}

// FUNCTION: SWBF2 0x00543440
void BaseHint::Unreserve(const GameObject* pObj) const
{
    const_cast<ReserveObject&>(mReserve).Unreserve(pObj);
}

// FUNCTION: SWBF2 0x00543470
const PblVector3& BaseHint::GetZAxis() const
{
    return mForward;
}

// FUNCTION: SWBF2 0x00543450
void BaseHint::SetReserveDelay(const GameObject* pObj, float fDelay) const
{

    (void)pObj;
    const_cast<ReserveObject&>(mReserve).SetReserveDelay(0, fDelay);
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x00543480
PblMatrix BaseHint::BuildMatrix() const
{
    PblMatrix mat;
    const PblVector3& vPos = GetCollisionCenter();
    mat.m[3][0] = vPos.x;
    mat.m[3][1] = vPos.y;
    mat.m[3][2] = vPos.z;
    mat.m[3][3] = 1.0f;
    mat.m[2][0] = mForward.x;
    mat.m[2][1] = mForward.y;
    mat.m[2][2] = mForward.z;
    mat.m[2][3] = 0.0f;

    const PblVector3 vUp(0.0f, 1.0f, 0.0f);
    PblVector3 vRight(mForward.y * vUp.z - mForward.z * vUp.y,
                      mForward.z * vUp.x - mForward.x * vUp.z,
                      mForward.x * vUp.y - mForward.y * vUp.x);
    PblVector3 vX;
    D3DXVec3Normalize(&vX, &vRight);
    mat.m[0][0] = vX.x;
    mat.m[0][1] = vX.y;
    mat.m[0][2] = vX.z;
    mat.m[0][3] = 0.0f;

    mat.m[1][0] = mForward.y * vX.z - mForward.z * vX.y;
    mat.m[1][1] = mForward.z * vX.x - mForward.x * vX.z;
    mat.m[1][2] = mForward.x * vX.y - mForward.y * vX.x;
    mat.m[1][3] = 0.0f;
    return mat;
}

// FUNCTION: SWBF2 0x005435a0
const CommandPost* BaseHint::GetCommandPost() const
{
    return 0;
}

// FUNCTION: SWBF2 0x005435b0
ControllableHeight BaseHint::GetRandomStance(char cStanceMask) const
{
    int iStance;
    do
        iStance = AIUtil::GetRandomInt(0, 2);
    while (!((1 << iStance) & cStanceMask));
    return (ControllableHeight)iStance;
}

// FUNCTION: SWBF2 0x005435e0
ControllableHeight BaseHint::GetRandomPrimaryStance() const
{
    return GetRandomStance(mPrimaryStance);
}

// FUNCTION: SWBF2 0x005435f0
ControllableHeight BaseHint::GetRandomSecondaryStance() const
{
    return GetRandomStance(mSecondaryStance);
}

// FUNCTION: SWBF2 0x00543610
bool BaseHint::HasSidestep() const
{
    return mSidestep != 0;
}

// FUNCTION: SWBF2 0x00543630
BaseHint::Sidestep BaseHint::GetRandomSidestep() const
{
    if (!HasSidestep())
    {
        return STEP_LEFT;
    }
    int iStep;
    do
        iStep = AIUtil::GetRandomInt(0, 1);
    while (!((1 << iStep) & mSidestep));
    return (Sidestep)iStep;
}

// FUNCTION: SWBF2 0x0062f460
bool ReserveObject::IsReserved(int iTeam, float fDelay) const
{
    const float fNow = GameLoop::GetMissionTime();
    float fWait = 2.0f;
    if (fDelay >= 0.0f)
    {
        fWait = fDelay;
    }
    return GetReserveTime(iTeam) + fWait > fNow;
}

// FUNCTION: SWBF2 0x0062f4b0
float ReserveObject::GetReserveTime(int  ) const
{
    return mReserveTime;
}

// FUNCTION: SWBF2 0x0062f4f0
void ReserveObject::Reserve(const GameObject* pObj)
{
    mReserver = pObj;
    mReserveTime = GameLoop::GetMissionTime();
}

// FUNCTION: SWBF2 0x00543730
SnipeHint::SnipeHint(const HintDesc& desc)
    : BaseHint(desc, HINT_SNIPE)
{
}

// FUNCTION: SWBF2 0x00543770
CoverHint::CoverHint(const HintDesc& desc)
    : BaseHint(desc, HINT_COVER)
{
}

// FUNCTION: SWBF2 0x005437f0
FortificationHint::FortificationHint(const HintDesc& desc)
    : BaseHint(desc, HINT_FORTIFICATION)
{
}

// FUNCTION: SWBF2 0x00543810
VehicleCoverHint::VehicleCoverHint(const HintDesc& desc)
    : BaseHint(desc, HINT_VEHICLECOVER)
{
}

// FUNCTION: SWBF2 0x00543750
PatrolHint::PatrolHint(const HintDesc& desc)
    : BaseHint(desc, HINT_PATROL)
{
}

// FUNCTION: SWBF2 0x00543790
JetJumpHint::JetJumpHint(const HintDesc& desc)
    : BaseHint(desc, HINT_JETJUMP)
{
}

// FUNCTION: SWBF2 0x005437b0
MinefieldHint::MinefieldHint(const HintDesc& desc)
    : BaseHint(desc, HINT_MINE)
{
}

// FUNCTION: SWBF2 0x005437d0
LandHint::LandHint(const HintDesc& desc)
    : BaseHint(desc, HINT_LAND)
{
}

void BaseHint::SetCommandPost(const CommandPost*  )
{
}

// FUNCTION: SWBF2 0x00543670
void BaseHint::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x844CCDB4:
        SetCommandPost(CommandPostManager::Instance()->FindPost(pszValue));
        break;
    case 0xBF0F1CD1:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        int iStance = iValue & 3;
        if (iStance)
        {
            mPrimaryStance = iStance;
        }
        break;
    }
    case 0x92C05A59:
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        int iStance = iValue & 7;
        if (iStance & 3)
        {
            mSecondaryStance = iStance;
        }
        mSidestep = iValue >> 3;
        break;
    }
    }
}

namespace HintManager
{

struct HintList : public TreeGridCallbackData
{
    HintList(const GameObject* pMe, int iTypes, int iMaxHints, const BaseHint** ppHints)
        : mCount(0), mMaxHints(iMaxHints), mHints(ppHints), mTypes(iTypes), mMe(pMe)
    {
    }

    int mCount;
    int mMaxHints;
    const BaseHint** mHints;
    int mTypes;
    const GameObject* mMe;
};

// FUNCTION: SWBF2 0x00543ab0
bool GenerateHintListCallback(const TreeGridObject* pObject, TreeGridCallbackData* pData, float&  )
{
    const BaseHint* pHint = static_cast<const BaseHint*>(pObject);
    HintList* pList = static_cast<HintList*>(pData);
    if ((pHint->GetType() & pList->mTypes) > 0 &&
        (!pList->mMe || AIUtil::IsHintAvailable(pList->mMe, pHint)))
    {
        if (pList->mCount >= pList->mMaxHints)
        {
            return false;
        }
        pList->mHints[pList->mCount++] = pHint;
    }
    return true;
}

// FUNCTION: SWBF2 0x00543b00
int RequestHintsCommon(int iType, const UnitController* pController,
                       const PblVector3& vPos, int iRadius, int iMaxHints,
                       const BaseHint** ppHints)
{
    const GameObject* pMe = 0;
    if (pController)
    {

        const Controllable* pOwner = pController->mOwner;
        if (pOwner)
        {
            for (int i = 0; i < pOwner->GetWeaponCount(); ++i)
            {
                if (CombatUtil::GetWeaponLabel(pOwner->GetWeaponPtr(i)) == WeaponClass::MELEE)
                {
                    return 0;
                }
            }
        }
        pMe = pController->GetMe()->GetGameObject();
        if (pOwner && pOwner->GetControllableClass()->mIgnoreHintNodes)
        {
            return 0;
        }
    }

    HintList list(pMe, iType, iMaxHints, ppHints);
    const float fRadius = (float)iRadius;
    const PblVector3 vMin(vPos.x - fRadius, vPos.y - fRadius, vPos.z - fRadius);
    const PblVector3 vMax(vPos.x + fRadius, vPos.y + fRadius, vPos.z + fRadius);
    sHints->CallbackOnRect(vMin, vMax, HintManager::GenerateHintListCallback, &list);
    return list.mCount;
}

// FUNCTION: SWBF2 0x00543c20
static int RequestCPHintsCommon(const UnitController* pController, int iType, const CommandPost* pPost,
                                int iMaxHints, const BaseHint** ppHints)
{
    static const BaseHint* pCPHints[50];
    int iCount = RequestHintsCommon(iType, pController, AIUtil::GetCommandPostPos(pPost), 300, 50, pCPHints);
    int iFound = 0;
    for (int i = 0; i < iCount && iFound < iMaxHints; ++i)
    {
        const CommandPost* pHintPost = pCPHints[i]->GetCommandPost();
        if (pHintPost && pHintPost == pPost)
        {
            ppHints[iFound++] = pCPHints[i];
        }
    }
    return iFound;
}

// FUNCTION: SWBF2 0x00543c90
int RequestSnipeHints(const UnitController* pController, const PblVector3& vPos,
                      int iRadius, int iMaxHints, const BaseHint** ppHints)
{
    return RequestHintsCommon(BaseHint::HINT_SNIPE, pController, vPos, iRadius, iMaxHints, ppHints);
}

// FUNCTION: SWBF2 0x00543cb0
int RequestCoverHints(const UnitController* pController, const PblVector3& vPos,
                      int iRadius, int iMaxHints, const BaseHint** ppHints)
{
    return RequestHintsCommon(BaseHint::HINT_COVER, pController, vPos, iRadius, iMaxHints, ppHints);
}

// FUNCTION: SWBF2 0x00543cd0
int RequestPatrolHints(const UnitController* pController, const CommandPost* pPost,
                       int iMaxHints, const BaseHint** ppHints)
{
    return RequestCPHintsCommon(pController, BaseHint::HINT_PATROL, pPost, iMaxHints, ppHints);
}

// FUNCTION: SWBF2 0x00543d10
int RequestMinefieldHints(const UnitController* pController, const PblVector3& vPos,
                          int iArg3, int iArg4, const BaseHint** ppHints)
{
    int iCount = RequestHintsCommon(16  , pController, vPos,
                                    iArg3, iArg4, ppHints);

    const int iTeam = pController->GetMe()->GetGameObject()->GetTeam();

    int i = 0;
    if (iCount > 0)
    {
        const BaseHint** ppLast = &ppHints[iCount - 1];
        do
        {
            const CommandPost* const pTest = ppHints[i]->GetCommandPost();
            if (pTest == 0 || pTest->CanTeamSpawnHere(iTeam))
            {
                ++i;
            }
            else
            {
                --iCount;
                ppHints[i] = *ppLast--;
            }
        } while (i < iCount);
    }
    return iCount;
}

// FUNCTION: SWBF2 0x00543e70
int RequestFortificationHints(const UnitController* pController, const PblVector3& vPos,
                              int iArg3, int iArg4, const BaseHint** ppHints)
{
    return RequestHintsCommon(BaseHint::HINT_FORTIFICATION, pController, vPos,
                              iArg3, iArg4, ppHints);
}

// FUNCTION: SWBF2 0x00543e90
int RequestVehicleCoverHints(const UnitController* pController, const PblVector3& vPos,
                             int iArg3, int iArg4, const BaseHint** ppHints)
{
    return RequestHintsCommon(BaseHint::HINT_VEHICLECOVER, pController, vPos,
                              iArg3, 30, ppHints);
}

}

namespace HintManager
{

// FUNCTION: SWBF2 0x00543cf0
int RequestJetJumpHints(const UnitController* pController, const PblVector3& vPos,
                        int iArg3, int iArg4, const BaseHint** ppHints)
{
    return RequestHintsCommon(BaseHint::HINT_JETJUMP, pController, vPos, iArg3, iArg4, ppHints);
}

// FUNCTION: SWBF2 0x00543da0
int RequestLandHints(const UnitController* pController, const PblVector3& vPos,
                     int iArg3, int iArg4, const BaseHint** ppHints, bool bGotoNonOwnedCP)
{
    int iCount = RequestHintsCommon(BaseHint::HINT_LAND, pController, vPos, iArg3, iArg4, ppHints);

    const int iTeam = pController->GetMe()->GetGameObject()->GetTeam();

    int i = 0;
    if (iCount > 0)
    {
        const BaseHint** ppLast = &ppHints[iCount - 1];
        do
        {
            const BaseHint* pHint = ppHints[i];
            const CommandPost* const pPost = pHint->GetCommandPost();
            bool bKeep = false;
            if (pPost)
            {
                const bool bOwned = pPost->CanTeamSpawnHere(iTeam);
                if ((bGotoNonOwnedCP ? !bOwned : bOwned) && pHint->GetCollisionCenter().y <= vPos.y)
                {
                    bKeep = true;
                }
            }
            if (bKeep)
            {
                ++i;
            }
            else
            {
                --iCount;
                ppHints[i] = *ppLast--;
            }
        } while (i < iCount);
    }
    return iCount;
}

}

namespace HintManager {

// FUNCTION: SWBF2 0x005438e0
void Reset()
{
    sHints = 0;
}
}

// GLOBAL: SWBF2 0x01e56c2c
TreeGrid* sHints;
