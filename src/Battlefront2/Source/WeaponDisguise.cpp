#include "WeaponDisguise.h"
#include "AIUtil.h"
#include "Controllable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "SoldierAnimationBank.h"
#include "Team.h"
#include "Trigger.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include <new>

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

// GLOBAL: SWBF2 0x01f9a8b8
// SYNTHETIC: SWBF2 0x00407130
// `dynamic initializer for 'WeaponDisguiseClass::rttiHashWeaponDisguiseClass''
PblHash WeaponDisguiseClass::rttiHashWeaponDisguiseClass("WeaponDisguiseClass");
char WeaponDisguiseClass::rttiNameWeaponDisguiseClass[] = "WeaponDisguiseClass";

// FUNCTION: SWBF2 0x00683390
WeaponDisguiseClass::WeaponDisguiseClass(unsigned int uHash)
    : WeaponClass(uHash)
{
    mTargetSides = 0;
    mTargetTypes = 0;
    mAITargetTypes = 0;
}

// FUNCTION: SWBF2 0x00683400
WeaponDisguiseClass::WeaponDisguiseClass(WeaponDisguiseClass* pBase, unsigned int uHash)
    : WeaponClass(pBase, uHash)
{
}

// FUNCTION: SWBF2 0x00683420
WeaponDisguiseClass::~WeaponDisguiseClass()
{
}

// SYNTHETIC: SWBF2 0x006833d0
// WeaponDisguiseClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00682cb0
bool WeaponDisguiseClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashWeaponDisguiseClass || WeaponClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00682ce0
unsigned int WeaponDisguiseClass::GetDerivedRtti()
{
    return rttiHashWeaponDisguiseClass;
}

// FUNCTION: SWBF2 0x00682cf0
const char* WeaponDisguiseClass::GetDerivedRttiName()
{
    return rttiNameWeaponDisguiseClass;
}

// FUNCTION: SWBF2 0x00683430
void WeaponDisguiseClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x8DA6FEC5)
    {
        return;
    }
    WeaponClass::SetProperty(uProperty, pszValue);
}

// FUNCTION: SWBF2 0x00683450
WeaponClass* WeaponDisguiseClass::Derive(unsigned int uHash)
{
    return new WeaponDisguiseClass(this, uHash);
}

// GLOBAL: SWBF2 0x01f9a8b4
// SYNTHETIC: SWBF2 0x00407120
// `dynamic initializer for 'WeaponDisguise::rttiHashWeaponDisguise''
PblHash WeaponDisguise::rttiHashWeaponDisguise("WeaponDisguise");
char WeaponDisguise::rttiNameWeaponDisguise[] = "WeaponDisguise";

// FUNCTION: SWBF2 0x00682c60
bool WeaponDisguise::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashWeaponDisguise || Weapon::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00682c90
unsigned int WeaponDisguise::GetDerivedRtti() const
{
    return rttiHashWeaponDisguise;
}

// FUNCTION: SWBF2 0x00682ca0
const char* WeaponDisguise::GetDerivedRttiName() const
{
    return rttiNameWeaponDisguise;
}

// FUNCTION: SWBF2 0x00682d00
WeaponDisguise::WeaponDisguise(const WeaponDisguiseClass* pClass, const WeaponDesc& desc)
    : Weapon(pClass, desc)
{
    mSavedOwnerModel = 0;
    mOtherTeam = 0;
    m_eState = STATE_OFF;
}

// FUNCTION: SWBF2 0x00682d70
WeaponDisguise::~WeaponDisguise()
{
}

// SYNTHETIC: SWBF2 0x00682d40
// WeaponDisguise::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00682d80
void WeaponDisguise::Select(unsigned int uFlags, bool bOffhand)
{
    Weapon::Select(uFlags, bOffhand);
    mOtherTeam = Team::GetPtr(3 - mOwner->GetGameObject()->GetTeam());
}

// FUNCTION: SWBF2 0x00682dd0
void WeaponDisguise::Deselect(unsigned int uFlags, bool bOffhand)
{
    if (m_eState != STATE_OFF)
    {
        InputToggleDisguise();
    }
    Weapon::Deselect(uFlags, bOffhand);
}

// FUNCTION: SWBF2 0x00682e00
void WeaponDisguise::EnterIdle()
{
    Weapon::EnterIdle();
}

// FUNCTION: SWBF2 0x00682e10
bool WeaponDisguise::UpdateIdle(float fDeltaTime)
{
    return Weapon::UpdateIdle(fDeltaTime);
}

// FUNCTION: SWBF2 0x00682e30
void WeaponDisguise::ExitIdle()
{
    Weapon::ExitIdle();
}

// FUNCTION: SWBF2 0x00682e40
void WeaponDisguise::EnterFire()
{
    InputToggleDisguise();
    Weapon::EnterFire();
}

// FUNCTION: SWBF2 0x00682e50
bool WeaponDisguise::UpdateFire(float fDeltaTime)
{
    return Weapon::UpdateFire(fDeltaTime);
}

// FUNCTION: SWBF2 0x00682e70
void WeaponDisguise::ExitFire()
{
    Weapon::ExitFire();
}

// FUNCTION: SWBF2 0x00682e80
bool WeaponDisguise::CheckFire() const
{
    return mTrigger && mTrigger->IsPressed();
}

// FUNCTION: SWBF2 0x00682ea0
bool WeaponDisguise::CheckReload() const
{
    return false;
}

// FUNCTION: SWBF2 0x00682eb0
bool WeaponDisguise::CheckEmpty() const
{
    return false;
}

// FUNCTION: SWBF2 0x00682ec0
bool WeaponDisguise::FireDone() const
{
    return mTrigger->m_uiButtonState == Trigger::BUTTONSTATE_RELEASED;
}

// FUNCTION: SWBF2 0x00682ee0
void WeaponDisguise::InputToggleDisguise()
{
    if (m_eState == STATE_OFF)
    {
        StartRaisingDisguise();
        FinishRaisingDisguise();
    }
    else
    {
        if (m_eState == STATE_ON)
        {
            StartDroppingDisguise();
        }
        FinishDroppingDisguise();
    }
}

// FUNCTION: SWBF2 0x00682f10
void WeaponDisguise::StartRaisingDisguise()
{
    mSavedOwnerModel = const_cast<GameModel*>(mOwner->GetGameObject()->GetModel());
}

// FUNCTION: SWBF2 0x00682f30
void WeaponDisguise::FinishRaisingDisguise()
{
    GameObjectClass* pDisguise = mOtherTeam->GetClassPtr(0);
    mOwner->GetGameObject()->SetModel(pDisguise->GetModel());
    mRenderClass = const_cast<WeaponClass*>(pDisguise->GetWeaponClass(0));
    mSoldierAnimationMap = SoldierAnimationBank::INVALID_MAP;
    if (mRenderClass->mSoldierAnimationWeapon != -1 && mOwner &&
        mOwner->GetSoldierAnimationBank() != SoldierAnimationBank::INVALID_BANK)
    {
        mSoldierAnimationMap = SoldierAnimationBank::FindMap(
            mOwner->GetSoldierAnimationBank(),
            (SoldierAnimationBank::WEAPON)mRenderClass->mSoldierAnimationWeapon);
    }
    if (mSoldierAnimationMap == SoldierAnimationBank::INVALID_MAP)
    {
        mSoldierAnimationMap = 0;
    }

    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        WhoTwiggedMe();
    }

    mOwner->GetGameObject()->SetPerceivedTeam(mOtherTeam->GetNumber());
    m_eState = STATE_ON;
}

// FUNCTION: SWBF2 0x00683040
void WeaponDisguise::StartDroppingDisguise()
{
    mOwner->GetGameObject()->SetPerceivedTeam(mOwner->GetGameObject()->GetTeam());
    AIUtil::ResetSpied(mOwner->mCharacter);
}

// FUNCTION: SWBF2 0x00683090
void WeaponDisguise::FinishDroppingDisguise()
{
    mOwner->GetGameObject()->SetModel(mSavedOwnerModel);
    mRenderClass = mStart;
    mSoldierAnimationMap = SoldierAnimationBank::INVALID_MAP;
    if (mRenderClass->mSoldierAnimationWeapon != -1 && mOwner &&
        mOwner->GetSoldierAnimationBank() != SoldierAnimationBank::INVALID_BANK)
    {
        mSoldierAnimationMap = SoldierAnimationBank::FindMap(
            mOwner->GetSoldierAnimationBank(),
            (SoldierAnimationBank::WEAPON)mRenderClass->mSoldierAnimationWeapon);
    }
    if (mSoldierAnimationMap == SoldierAnimationBank::INVALID_MAP)
    {
        mSoldierAnimationMap = 0;
    }
    mSavedOwnerModel = 0;
    m_eState = STATE_OFF;
}

typedef PblHeap<const Controllable*, float> TwiggerHeap;

// TEMPLATE: SWBF2 0x00683540
// PblHeap<Controllable const *,float>::Pop
template void PblHeap<const Controllable*, float>::Pop();

// TEMPLATE: SWBF2 0x006835d0
// PblHeap<Controllable const *,float>::FindLowest
template int PblHeap<const Controllable*, float>::FindLowest();

// FUNCTION: SWBF2 0x00683120
void WeaponDisguise::WhoTwiggedMe()
{
    GameObject* pMe = mOwner->GetGameObject();
    PblVector3 vPos = pMe->GetMatrix().GetTranslation();
    TwiggerHeap heap(5, 6400.0f, 0);

    GameObject* apObjects[30];
    const int iCount = TeamManager::GetObjectsInRange(vPos, 40.0f, apObjects, 30, pMe, AI::AFFIL_ENEMY);
    GameObject* pObject = 0;
    for (int i = 0; i < iCount; ++i)
    {
        pObject = apObjects[i];
        if (!pObject->IsAlive())
        {
            continue;
        }
        for (PblList<Controllable>::Itor it(pObject->GetControllableList()); (Controllable*)it; ++it)
        {
            const Controllable* pWatcher = it;
            if (!pWatcher->mCharacter)
            {
                continue;
            }
            float fDistSq = -1.0f;
            if (!AIUtil::CanPotentiallySee(const_cast<Controllable*>(pWatcher), pMe, &fDistSq))
            {
                continue;
            }
            float fKey = 6400.0f - fDistSq;
            if (heap.IsFull())
            {
                const int iLowest = heap.FindLowest();
                if (fKey > heap.Value(iLowest))
                {
                    heap.ReplaceLowest(pWatcher, fKey, iLowest);
                }
            }
            else
            {
                heap.Push(pWatcher, fKey);
            }
        }
    }

    Character* pMyCharacter = mOwner->mCharacter;
    while (!heap.IsEmpty())
    {
        const Controllable* pWatcher = heap.Get(0);
        heap.Pop();
        if (AIUtil::CanSee(mOwner, pObject, &vPos))
        {
            AIUtil::AddSpied(pMyCharacter, pWatcher->mCharacter);
        }
    }
}

// FUNCTION: SWBF2 0x006834c0
Weapon* WeaponDisguiseClass::Build(const WeaponDesc& desc)
{
    PostLoadInit();
    void* pMemory = Weapon::sMemoryPool.Allocate(0x120);
    return pMemory ? reinterpret_cast<Weapon*>(::new (pMemory) WeaponDisguise(this, desc)) : 0;
}
