#include <stdlib.h>

#include "EntityBuildingArmed.h"
#include "Controllable.h"
#include "Character.h"
#include "GameModel.h"
#include "MountedTurret.h"
#include "NetGame.h"
#include "PlayerStats.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "CollisionManager.h"
#include "FLEffect.h"
#include "Trackable.h"

// GLOBAL: SWBF2 0x007e6334
static int sCurrentSection = -1;

// FUNCTION: SWBF2 0x004966c0
EntityBuildingArmedClass::EntityBuildingArmedClass(unsigned int uHash)
    : EntityBuildingClass(uHash)
{
    for (int i = 0; i < 8; i++)
    {
        mTurretClass[i] = 0;
    }
    sCurrentSection = -1;
    mRespawnTime = 120.0f;
}

// FUNCTION: SWBF2 0x00496790
EntityBuildingArmedClass::~EntityBuildingArmedClass()
{
}

// GLOBAL: SWBF2 0x01ebb888
// SYNTHETIC: SWBF2 0x004028e0
// `dynamic initializer for 'EntityBuildingArmed::rttiHashEntityBuildingArmed''
PblHash EntityBuildingArmed::rttiHashEntityBuildingArmed("EntityBuildingArmed");
char EntityBuildingArmed::rttiNameEntityBuildingArmed[] = "EntityBuildingArmed";

// GLOBAL: SWBF2 0x01ebb88c
// SYNTHETIC: SWBF2 0x004028f0
// `dynamic initializer for 'EntityBuildingArmedClass::rttiHashEntityBuildingArmedClass''
PblHash EntityBuildingArmedClass::rttiHashEntityBuildingArmedClass("EntityBuildingArmedClass");
char EntityBuildingArmedClass::rttiNameEntityBuildingArmedClass[] = "EntityBuildingArmedClass";

// SYNTHETIC: SWBF2 0x00496710
// EntityBuildingArmedClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00496740
EntityBuildingArmedClass::EntityBuildingArmedClass(EntityBuildingArmedClass* pBase, unsigned int uHash)
    : EntityBuildingClass(pBase, uHash)
{
    sCurrentSection = -1;
    for (int i = 0; i < 8; i++)
    {
        mTurretClass[i] = pBase->mTurretClass[i];
    }
}

// FUNCTION: SWBF2 0x00496950
EntityClass* EntityBuildingArmedClass::Derive(unsigned int uHash)
{
    return new EntityBuildingArmedClass(this, uHash);
}

// FUNCTION: SWBF2 0x004969c0
Entity* EntityBuildingArmedClass::Build(const EntityDesc& desc)
{
    EntityBuildingArmed* pBuilding = new EntityBuildingArmed(this, desc);
    if (pBuilding)
    {
        pBuilding->AddStaticToCollisionManager();
        pBuilding->ActivatePhysics();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            pBuilding->BuildEffects();
        }
    }
    return pBuilding;
}

// FUNCTION: SWBF2 0x00476e90
bool EntityBuildingArmedClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityBuildingArmedClass || EntityBuildingClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00495a20
unsigned int EntityBuildingArmedClass::GetDerivedRtti()
{
    return rttiHashEntityBuildingArmedClass;
}

// FUNCTION: SWBF2 0x00495a30
const char* EntityBuildingArmedClass::GetDerivedRttiName()
{
    return rttiNameEntityBuildingArmedClass;
}

// FUNCTION: SWBF2 0x004959e0
EntityType EntityBuildingArmed::GetEntityType() const
{
    return ENT_ARMED;
}

// FUNCTION: SWBF2 0x004967a0
void EntityBuildingArmedClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xd89b1a5cu)
    {
        switch (PblHash(pszValue))
        {
        case 0xfb2aa4b7:
        case 0xea20fef2:
            sCurrentSection = 0;
            break;
        case 0xe920fd5f:
            sCurrentSection = 1;
            break;
        case 0xe820fbcc:
            sCurrentSection = 2;
            break;
        case 0xe720fa39:
            sCurrentSection = 3;
            break;
        case 0xe620f8a6:
            sCurrentSection = 4;
            break;
        case 0xe520f713:
            sCurrentSection = 5;
            break;
        case 0xe420f580:
            sCurrentSection = 6;
            break;
        case 0xf3210d1d:
            sCurrentSection = 7;
            break;
        case 0xdbaa7975:
        default:
            sCurrentSection = -1;
            return;
        }
        if (!mTurretClass[sCurrentSection])
        {
            mTurretClass[sCurrentSection] = new MountedTurretClass(mId);
        }
        return;
    }

    if (sCurrentSection >= 0 &&
        mTurretClass[sCurrentSection]->SetProperty(uProperty, pszValue, mModel ? mModel->GetRedModel(0) : 0))
    {
        return;
    }
    EntityBuildingClass::SetProperty(uProperty, pszValue);
}

static inline Controllable* TurretControllable(MountedTurret* pTurret)
{
    return static_cast<Controllable*>(pTurret);
}

// FUNCTION: SWBF2 0x00495a40
EntityBuildingArmed::EntityBuildingArmed(EntityBuildingArmedClass* pClass, const EntityDesc& desc)
    : EntityBuilding(pClass, desc)
{
    mNumTurrets = 0;
    for (int i = 0; i < 8; ++i)
    {
        if (pClass->mTurretClass[i])
        {
            mTurret[mNumTurrets] = new MountedTurret(this, pClass->mTurretClass[i], &mPose);
            MountedTurret* pTurret = mTurret[mNumTurrets];
            mControllableList.AddLast(&pTurret->mNode, pTurret);
            ++mNumTurrets;
        }
    }
    mModel->GetRedModel(0)->SetupPose(mPose);
    mSmokePlume = PblHandle<FLEffectObject>();
    mNoAIBoard = false;
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x00495bf0
EntityBuildingArmed::~EntityBuildingArmed()
{
    NetGame::Destroyed(this);
    if (mSmokePlume)
    {
        mSmokePlume->DeactivateEffect();
    }
    while (mNumTurrets)
    {
        --mNumTurrets;
        mControllableList.Remove(&mTurret[mNumTurrets]->mNode);
        delete mTurret[mNumTurrets];
    }
}

// SYNTHETIC: SWBF2 0x004959f0
// EntityBuildingArmed::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00496a81
// [thunk]:EntityBuildingArmed::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x00496a8c
// [thunk]:EntityBuildingArmed::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00496a94
// [thunk]:EntityBuildingArmed::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00496a9f
// [thunk]:EntityBuildingArmed::`scalar deleting destructor' adjustor{608}
// SYNTHETIC: SWBF2 0x00496aaa
// [thunk]:EntityBuildingArmed::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x00495d90
void EntityBuildingArmed::Init()
{
    GameObject::Init();
    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->Init();
        }
    }
    if (mModel->GetRedModel(0)->_pStencilShadow)
    {
        RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
        if (!pLight->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                         ? RedLight::GetGlobalDirectionalLight(1)
                         : NULL;
        }
        if (pLight)
        {
            const PblVector3& vDir = pLight->m_vDirection;
            float fGrow = _Sphere.w;
            if (vDir.y < -0.5f)
            {
                fGrow /= vDir.y * -2.0f;
            }
            SetRenderPos(PblVector3(vDir.x * fGrow + _Sphere.x, vDir.y * fGrow + _Sphere.y,
                                    vDir.z * fGrow + _Sphere.z));
            SetRenderRadius(_Sphere.w + fGrow);
            SetTestBBox(false);
        }
    }
    if (mpShadow)
    {
        mpShadow->RemoveFromScene();
    }
    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            it->SetupController();
        }
    }
}

// FUNCTION: SWBF2 0x00495f20
void EntityBuildingArmed::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    if (mState == STATE_ALIVE)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->ActivatePhysics();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00495f70
void EntityBuildingArmed::DeactivatePhysics()
{
    if (mState == STATE_ALIVE)
    {
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->DeactivatePhysics();
            }
        }
    }
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00496450
void EntityBuildingArmed::Kill()
{
    if (mState == STATE_DEAD)
    {
        return;
    }

    if (mNumTurrets > 0 && mDamageOwner.mCharacter && GetControllable() &&
        GetControllable()->mPilotType == PILOT_SELF &&
        GetTeam() != mDamageOwner.mTeam)
    {
        Character::GetPlayerStats(mDamageOwner.mCharacter).PointsProcess(13, 0);
    }

    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->Kill();
        mTurret[i]->DeactivatePhysics();
    }
    EntityBuilding::Kill();
}

// FUNCTION: SWBF2 0x00496a70
const MountedTurretClass* EntityBuildingArmedClass::GetTurretClass(int iIndex) const
{
    return mTurretClass[iIndex];
}

// FUNCTION: SWBF2 0x00496500
void EntityBuildingArmed::Respawn()
{
    if (mSmokePlume)
    {
        mSmokePlume->DeactivateEffect();
        mSmokePlume = 0;
    }

    for (int i = 0; i < mNumTurrets; ++i)
    {
        mTurret[i]->ActivatePhysics();
    }

    if (!NetGame::IsEnabled() || !NetGame::IsClient())
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            Controllable* pControllable = it;
            if (pControllable->mPilotType != PILOT_SELF && pControllable->mPilotType != PILOT_VEHICLESELF)
            {
                continue;
            }
            if (pControllable->mPilotType == PILOT_SELF ||
                (pControllable->mPilotType == PILOT_VEHICLESELF && pControllable->mPilot == pControllable) ||
                !pControllable->mPilot)
            {
                pControllable->SetPilot(pControllable);
            }
        }
    }

    EntityBuilding::Respawn();
}

// FUNCTION: SWBF2 0x00495d50
void EntityBuildingArmed::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x966F4238u)
    {
        mNoAIBoard = atoi(pszValue) != 0;
    }
    else
    {
        reinterpret_cast<GameObject*>(this)->GameObject::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x00476e50
Controllable* EntityBuildingArmed::GetControllable()
{
    if (mControllableList.GetCount() == 1)
    {
        return mControllableList.GetFirst()->_pObject;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00476e30
const Controllable* EntityBuildingArmed::GetControllable() const
{
    if (mControllableList.GetCount() == 1)
    {
        return mControllableList.GetFirst()->_pObject;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00476e70
GameObjectType EntityBuildingArmed::GetGameObjectType()
{
    return (GameObjectType)(GO_BUILDING | GO_ARMEDBUILDING);
}

// FUNCTION: SWBF2 0x00496610
Trackable* EntityBuildingArmed::GetTrackable()
{
    for (int i = 0; i < mNumTurrets; i++)
    {
        Controllable* pPilot = TurretControllable(mTurret[i])->GetPilot();
        if (pPilot && pPilot->mPlayerId >= 0 &&
            (NetGame::GetSpectator(NetGame::GetLocalIndex(0)) == pPilot->mPlayerId ||
             NetGame::GetJoystickIndex(pPilot->mPlayerId) >= 0))
        {
            return TurretControllable(mTurret[i]);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00476db0
bool EntityBuildingArmed::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityBuildingArmed || EntityBuilding::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004959c0
unsigned int EntityBuildingArmed::GetDerivedRtti() const
{
    return rttiHashEntityBuildingArmed;
}

// FUNCTION: SWBF2 0x004959d0
const char* EntityBuildingArmed::GetDerivedRttiName() const
{
    return rttiNameEntityBuildingArmed;
}

// FUNCTION: SWBF2 0x00476e00
int EntityBuildingArmed::GetTurretCount() const
{
    return mNumTurrets;
}

// FUNCTION: SWBF2 0x00476e10
const Controllable* EntityBuildingArmed::GetTurretPtr(int iIndex) const
{
    return TurretControllable(mTurret[iIndex]);
}

// FUNCTION: SWBF2 0x00476e80
const RedPose* EntityBuildingArmed::GetPose() const
{
    return &mPose;
}

// FUNCTION: SWBF2 0x00496040
void EntityBuildingArmed::Render(unsigned char uFlags, float fDist, unsigned int uMask)
{
    if (GetTrackable())
    {
        for (int i = 0; i < GetTurretCount(); ++i)
        {
            if (GetTurretPtr(i) && !GetTrackable()->RenderTrackable(GetTurretPtr(i)->mPlayerId))
            {
                return;
            }
        }
    }

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(int)(fDist * 255.0f);

    if (!mModel)
    {
        return;
    }

    if (mClass->m_bDynamicLighting)
    {
        uMask &= ~0x80;
    }
    else
    {
        uMask |= 0x80;
    }

    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const RedCamera* pCamera = RedRenderer::GetCamera();
    if (GetModel()->GetRedModel(0)->_pStencilShadow && RedStencilShadow::GetShadowEnable())
    {
        RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
        if (!pLight->IsShadowCaster())
        {
            pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                         ? RedLight::GetGlobalDirectionalLight(1)
                         : NULL;
        }
        float fLength;
        if (pLight)
        {
            uMask |= 0x10000;
            const PblVector3& vDir = pLight->m_vDirection;
            float fMinLength = _Sphere.w * -2.0f / vDir.y;
            PblVector3 vStart(_Sphere.x, _Sphere.w + _Sphere.y, _Sphere.z);
            const GameObject* pIgnore = this;
            CollisionObject* pHit;
            fLength = CollisionManager::RayHit(vStart, vDir, 50.0f, pHit, NULL, &pIgnore, 1, 0x90, false) * 50.0f;
            if (fMinLength > fLength)
            {
                fLength = fMinLength;
            }
        }
        else
        {
            fLength = _Sphere.w * 2.0f;
        }

        static float fMaxShadowLength =
            static_cast<const EntityGeometryClass*>(mEntityClass)->mMaxShadowLength;
        float fShadowLength = fMaxShadowLength;
        if (fShadowLength <= 0.0f || fShadowLength > fLength)
        {
            fShadowLength = fLength;
        }
        RedRenderer::SetStencilShadowLength(fShadowLength);

        const PblMatrix& mView = pCamera->GetMatrix();
        float fDX = mView.m[3][0] - mView.m[2][0] * fNear - _Sphere.x;
        float fDY = mView.m[3][1] - mView.m[2][1] * fNear - _Sphere.y;
        float fDZ = mView.m[3][2] - mView.m[2][2] * fNear - _Sphere.z;
        float fReach = _Sphere.w + fLength + fWidth * 1.33f;
        RedRenderer::SetStencilShadowZFail(fReach * fReach > fDY * fDY + fDX * fDX + fDZ * fDZ);
    }
    else
    {
        uMask &= ~0x10000;
    }

    PblMatrix mSaved[8];
    if (mNumTurrets > 0)
    {
        PblMatrix mInverse = mMatrix.GetInverseFast();
        for (int i = 0; i < mNumTurrets; ++i)
        {
            if (mTurret[i])
            {
                mTurret[i]->Render(mMatrix, mInverse, mPose, uMask, mSaved[i]);
            }
        }
    }

    mModel->Render(uFlags, fDist, &mMatrix, &mPose, color, uMask, NULL);

    for (int i = 0; i < mNumTurrets; ++i)
    {
        if (mTurret[i])
        {
            mTurret[i]->PostRender(mSaved[i]);
        }
    }
}

// FUNCTION: SWBF2 0x00495fc0
bool EntityBuildingArmed::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    UpdateStatus(fDeltaTime);
    EntityBuilding::Update(fDeltaTime);

    if (IsAlive() || mClass->mDestroyedModel || mClass->mUnbuiltModel)
    {
        return true;
    }
    return NetGame::IsEnabled();
}
