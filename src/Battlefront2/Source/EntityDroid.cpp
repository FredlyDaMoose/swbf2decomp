#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <math.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "EntityDroid.h"
#include "Factory.h"
#include "PebbleFL/Common/PblList.h"
#include "EnergyBar.h"
#include "AILowLevel.h"
#include "AIUtil.h"
#include "Character.h"
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionModel.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "FirstPerson.h"
#include "GameLoop.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "FLEffect.h"
#include "MedalsMgr.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "PlayerController.h"
#include "Restrictor.h"
#include "TargetManager.h"
#include "Tracker.h"
#include "TriggerRegionManager.h"
#include "UnitController.h"
#include "UnitMobileController.h"
#include "WeaponCannonInstance.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

// SYNTHETIC: SWBF2 0x0049d340
// EntityDroidClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0049d6b0
EntityDroidClass::~EntityDroidClass()
{
}

// FUNCTION: SWBF2 0x0049e1b0
void* __fastcall GetFirstWeaponFactoryNode(const void* pList)
{
    return static_cast<const PblList<Factory<Weapon, WeaponClass, WeaponDesc>>*>(pList)->_head._pNext;
}

// FUNCTION: SWBF2 0x0049e1c0
void* __fastcall InitializeWeaponFactoryIterator(void* pItor, void*, void* pNode)
{
    *reinterpret_cast<void**>(pItor) = pNode;
    return pItor;
}

// FUNCTION: SWBF2 0x0049e1d0
void* __fastcall GetWeaponFactoryIteratorObject(const void* pItor)
{
    return *static_cast<const PblList<Factory<Weapon, WeaponClass, WeaponDesc>>::Itor*>(pItor);
}

// FUNCTION: SWBF2 0x0049e1e0
void* __fastcall DereferenceWeaponFactoryIterator(const void* pItor)
{
    return *static_cast<const PblList<Factory<Weapon, WeaponClass, WeaponDesc>>::Itor*>(pItor);
}

// FUNCTION: SWBF2 0x0049e1f0
void* __fastcall ConvertWeaponFactoryIteratorToPointer(const void* pItor)
{
    return *static_cast<const PblList<Factory<Weapon, WeaponClass, WeaponDesc>>::Itor*>(pItor);
}

// TEMPLATE: SWBF2 0x0049e200
// PblList<Factory<Weapon,WeaponClass,WeaponDesc> >::Itor::operator++
template PblList<Factory<Weapon, WeaponClass, WeaponDesc> >::Itor&
PblList<Factory<Weapon, WeaponClass, WeaponDesc> >::Itor::operator++();
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblQuaternion* __stdcall D3DXQuaternionNormalize(PblQuaternion* pOut, const PblQuaternion* pQ);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationQuaternion(PblMatrix* pOut, const PblQuaternion* pQ);

extern bool netFrameLock;
extern bool netIsLocalTurn;
extern bool netIsLateTurn;
extern PblVector3 gVectorZero;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

Controllable* GetControllablePilot(Controllable* pControllable);

// GLOBAL: SWBF2 0x01e51d94
static int sWeaponSection;

// FUNCTION: SWBF2 0x0049d010
EntityDroidClass::EntityDroidClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    mHealthType = HEALTH_TYPE_DROID;
    m_bDynamicLighting = true;
    mPilotType = PILOT_SELF;

    mSetAltitude = 0.0f;
    mGravityScale = 1.0f;
    mLiftSpring = 5.0f;
    mLiftDamp = 2.0f;
    mAcceleration = 70.0f;
    mMaxSpeed = 9.0f;
    mMaxStrafeSpeed = 7.0f;
    mMaxTurnSpeed = 2.0f;
    mMaxPitchSpeed = 0.5f;
    mMinPitch = -1.0f;
    mMaxPitch = 1.0f;
    mLevelSpring = 10.0f;
    mLevelDamp = 3.0f;
    mCollisionThreshold[0] = 7.5f;
    mCollisionThreshold[1] = 2.5f;
    mCollisionThreshold[2] = 0.0f;
    mCollisionScale[0] = 1.0f;
    mCollisionScale[1] = 0.0f;
    mCollisionScale[2] = 0.0f;
    mCollisionInflict = 1.0f;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = 0;
        mWeaponAmmo[i].uiAmmo = 0;
        mWeaponAmmo[i].uiHeat = 0;
        mWeaponAmmo[i].bFlagAmmoInfinite = 1;
        mWeaponAmmo[i].bFlagShareHeat = 0;
        mWeaponAmmo[i].bFlagShareAmmo = 0;
        mWeaponChannel[i] = i;
    }

    sWeaponSection = 0;

    mWeaponCount = 0;

    mCameraSettings.mCameraEyePointOffset.x = 0.0f;
    mCameraSettings.mCameraEyePointOffset.y = 2.3f;
    mCameraSettings.mCameraEyePointOffset.z = 0.0f;
    mCameraSettings.mCameraTrackCenter.x = 0.0f;
    mCameraSettings.mCameraTrackCenter.y = 2.3f;
    mCameraSettings.mCameraTrackCenter.z = 0.0f;
    mCameraSettings.mCameraTrackOffset.x = 0.0f;
    mCameraSettings.mCameraTrackOffset.y = 0.0f;
    mCameraSettings.mCameraTrackOffset.z = 4.5f;
    mCameraSettings.mCameraSafePoint.x = 0.0f;
    mCameraSettings.mCameraSafePoint.y = 0.15f;
    mCameraSettings.mCameraSafePoint.z = -0.3f;
    mCameraSettings.mCameraMoveTensionL = 12.0f;
    mCameraSettings.mCameraMoveTensionR = 12.0f;
    mCameraSettings.mCameraMoveTensionU = 12.0f;
    mCameraSettings.mCameraMoveTensionD = 12.0f;
    mCameraSettings.mCameraMoveTensionF = 12.0f;
    mCameraSettings.mCameraMoveTensionB = 12.0f;
    mCameraSettings.mCameraAimTension = FLT_MAX;

    mExplosion = 0;

    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;
    mEnergyBarClass.Init(100.0f, 20.0f);

    mEnergyAutoRestore = 0.0f;
    mCameraDestructTrackOffset.x = 0.0f;
    mCameraDestructTrackOffset.y = 0.0f;
    mCameraDestructTrackOffset.z = 0.0f;
    mCameraDestructTiltValue = 0.0f;
    mDestructChargeEffect = 0x23081C56u;
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0049d370
EntityDroidClass::EntityDroidClass(EntityDroidClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash),
      ControllableClass(pBase)
{
    mEnergyBarClass.m_fEnergyMax = 0.0f;
    mEnergyBarClass.m_fEnergyMin = 0.0f;
    mEnergyBarClass.m_fEnergyOverheat = 0.0f;

    sWeaponSection = 0;

    mSetAltitude = pBase->mSetAltitude;
    mGravityScale = pBase->mGravityScale;
    mLiftSpring = pBase->mLiftSpring;
    mLiftDamp = pBase->mLiftDamp;
    mAcceleration = pBase->mAcceleration;
    mMaxSpeed = pBase->mMaxSpeed;
    mMaxStrafeSpeed = pBase->mMaxStrafeSpeed;
    mMaxTurnSpeed = pBase->mMaxTurnSpeed;
    mMaxPitchSpeed = pBase->mMaxPitchSpeed;
    mMinPitch = pBase->mMinPitch;
    mMaxPitch = pBase->mMaxPitch;
    mLevelSpring = pBase->mLevelSpring;
    mLevelDamp = pBase->mLevelDamp;
    for (int i = 0; i < 3; ++i)
    {
        mCollisionThreshold[i] = pBase->mCollisionThreshold[i];
    }
    for (int i = 0; i < 3; ++i)
    {
        mCollisionScale[i] = pBase->mCollisionScale[i];
    }
    mCollisionInflict = pBase->mCollisionInflict;

    for (int i = 0; i < 4; ++i)
    {
        mWeaponClass[i] = pBase->mWeaponClass[i];
        mWeaponAmmo[i] = pBase->mWeaponAmmo[i];
        mWeaponChannel[i] = pBase->mWeaponChannel[i];
    }
    mWeaponCount = pBase->mWeaponCount;
    for (int i = 0; i < 4; ++i)
    {
        mAimerClass[i] = pBase->mAimerClass[i];
    }

    mCameraSettings = pBase->mCameraSettings;
    mExplosion = pBase->mExplosion;
    mEnergyBarClass = pBase->mEnergyBarClass;
    mEnergyAutoRestore = pBase->mEnergyAutoRestore;
    mCameraDestructTrackOffset = pBase->mCameraDestructTrackOffset;
    mCameraDestructTiltValue = pBase->mCameraDestructTiltValue;
    mDestructChargeEffect = pBase->mDestructChargeEffect;
    mNoiseFactor = 2.0f;
    mLODClass = 0;
    mAmbientSound = pBase->mAmbientSound;
    mAmbient2Sound = pBase->mAmbient2Sound;
}

// FUNCTION: SWBF2 0x0049d720
void EntityDroidClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x4F358485:
        sscanf(pszValue, "%f", &mSetAltitude);
        break;
    case 0x97F2E0F1:
        sscanf(pszValue, "%f", &mGravityScale);
        break;
    case 0x1B5071F9:
        sscanf(pszValue, "%f", &mLiftSpring);
        break;
    case 0xA37B2EC6:
        sscanf(pszValue, "%f", &mLiftDamp);
        break;
    case 0x2FA0BA9D:
    case 0x9F8A0040:
        sscanf(pszValue, "%f", &mAcceleration);
        break;
    case 0x86DF0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0x3437A33D:
        sscanf(pszValue, "%f", &mMaxStrafeSpeed);
        break;
    case 0x5433F589:
        sscanf(pszValue, "%f", &mMaxTurnSpeed);
        break;
    case 0xB5534BE0:
        sscanf(pszValue, "%f", &mMaxPitchSpeed);
        break;
    case 0x3403B139:
        sscanf(pszValue, "%f %f", &mMinPitch, &mMaxPitch);
        mMinPitch *= 0.017453292f;
        mMaxPitch *= 0.017453292f;
        break;
    case 0x02E46404:
        sscanf(pszValue, "%f", &mLevelSpring);
        break;
    case 0x252DFAAB:
        sscanf(pszValue, "%f", &mLevelDamp);
        break;
    case 0x2CDB588A:
        sscanf(pszValue, "%f %f %f", &mCollisionThreshold[0], &mCollisionThreshold[1], &mCollisionThreshold[2]);
        break;
    case 0x1A87BACF:
        sscanf(pszValue, "%f %f %f", &mCollisionScale[0], &mCollisionScale[1], &mCollisionScale[2]);
        break;
    case 0x2EF03732:
        sscanf(pszValue, "%f", &mCollisionInflict);
        break;
    case 0x28ADDA49:
        mCombatInterrupts = atoi(pszValue) == 0;
        break;
    case 0x4F9D8BB4:
        sscanf(pszValue, "%f", &mEnergyAutoRestore);
        break;
    case 0x65A920BA:
        mAmbientSound.SetID(pszValue, true, false);
        break;
    case 0x9A108B4A:
        mAmbient2Sound.SetID(pszValue, true, false);
        break;
    case 0x02BB1FE0:
    case 0x9B30E763:
        mExplosion = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0x83E12771:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mCameraDestructTrackOffset.x = x;
        mCameraDestructTrackOffset.y = y;
        mCameraDestructTrackOffset.z = z;
        break;
    }
    case 0xE11CE401:
        sscanf(pszValue, "%f", &mCameraDestructTiltValue);
        mCameraDestructTiltValue *= 0.017453292f;
        break;
    case 0xF9765272:
        mDestructChargeEffect = PblHash(pszValue);
        break;
    case 0xD0329E80:
        sWeaponSection = atoi(pszValue) - 1;
        break;
    case 0x6F332041:
    case 0xFBF47DBA:
    {
        mWeaponClass[sWeaponSection] = WeaponClass::Find(PblHash(pszValue));
        int iCount = sWeaponSection + 1;
        if (mWeaponCount > iCount)
        {
            iCount = mWeaponCount;
        }
        mWeaponCount = iCount;
        break;
    }
    case 0x16472328:
        sscanf(pszValue, "%d", &mWeaponChannel[sWeaponSection]);
        break;
    case 0xDC19BD13:
    {
        int iAmmo = 0;
        sscanf(pszValue, "%d", &iAmmo);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        ammo.bFlagShareAmmo = 0;
        ammo.bFlagAmmoInfinite = iAmmo <= 0;
        if (!ammo.bFlagAmmoInfinite)
        {
            ammo.uiAmmo = iAmmo;
        }
        break;
    }
    case 0x8585BB8A:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagAmmoInfinite = 0;
            ammo.bFlagShareAmmo = 1;
            ammo.uiAmmo = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareAmmo = 0;
            ammo.bFlagAmmoInfinite = 1;
        }
        break;
    }
    case 0x9455A446:
    {
        int iWeapon = 0;
        sscanf(pszValue, "%d", &iWeapon);
        WeaponAmmoDesc& ammo = mWeaponAmmo[sWeaponSection];
        if (iWeapon > 0 && iWeapon <= sWeaponSection)
        {
            ammo.bFlagShareHeat = 1;
            ammo.uiHeat = iWeapon - 1;
        }
        else
        {
            ammo.bFlagShareHeat = 0;
        }
        break;
    }
    case 0x47C86B4A:
        GameObjectClass::SetProperty(0x47C86B4A, pszValue);
        if (mCollisionModel.GetNumBodies() == 0 && mModel)
        {
            const PblSphere& sphere = mModel->_mModel->_BoundingSphere;
            CollisionPrimitive primitive;
            primitive.SetSphere(sphere.GetRadius());
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = sphere.x;
            matrix.m[3][1] = sphere.y;
            matrix.m[3][2] = sphere.z;
            matrix.m[3][3] = 1.0f;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
        break;
    default:
    {
        const RedModel* pModel = mModel ? mModel->GetRedModel(0) : 0;

        if (!mEnergyBarClass.SetProperty(uProperty, pszValue) &&
            !mAimerClass[sWeaponSection].SetProperty(uProperty, pszValue, pModel, 0) &&
            !mCameraSettings.SetProperty(uProperty, pszValue) &&
            !ControllableClass::SetProperty(uProperty, pszValue, pModel))
        {
            GameObjectClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
    }
}

// FUNCTION: SWBF2 0x0049e040
EntityClass* EntityDroidClass::Derive(unsigned int uHash)
{
    return new EntityDroidClass(this, uHash);
}

// FUNCTION: SWBF2 0x00483380
GameObject* Controllable::GetGameObject()
{
    return 0;
}

// FUNCTION: SWBF2 0x00483390
const GameObject* Controllable::GetGameObject() const
{
    return 0;
}

// FUNCTION: SWBF2 0x004833a0
Trackable* Controllable::GetTrackable()
{
    return 0;
}

// FUNCTION: SWBF2 0x00483360
Entity* Controllable::GetEntity()
{
    return 0;
}

// FUNCTION: SWBF2 0x00483370
const Entity* Controllable::GetEntity() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00483420
const ControllableClass* Controllable::GetControllableClass() const
{
    return 0;
}

// FUNCTION: SWBF2 0x00483430
ControllableClass* Controllable::GetControllableClass()
{
    return 0;
}

// FUNCTION: SWBF2 0x00483440
void Controllable::SetupController()
{
}

// FUNCTION: SWBF2 0x00483460
bool Controllable::AIThread::Update(float fDeltaTime)
{
    mOwner->UpdateIndirect(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00483480
void Controllable::AIThread::DeactivateThread()
{
    mOwner->DeactivateIndirect();
    Thread::DeactivateThread();
}

// SYNTHETIC: SWBF2 0x004834a0
// Controllable::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x004834d0
// Controllable::AIThread::~AIThread

// SYNTHETIC: SWBF2 0x004834e0
// Controllable::AIThread::`scalar deleting destructor'

// GLOBAL: SWBF2 0x01ebbbb4
// SYNTHETIC: SWBF2 0x00402a40
// `dynamic initializer for 'EntityDroid::rttiHashEntityDroid''
PblHash EntityDroid::rttiHashEntityDroid("EntityDroid");
char EntityDroid::rttiNameEntityDroid[] = "EntityDroid";

// FUNCTION: SWBF2 0x00499300
void GameObject::SetNetUniqueId(int iNetUniqueId)
{

    mNetUniqueId = iNetUniqueId;
}

// GLOBAL: SWBF2 0x01ebbbb8
// SYNTHETIC: SWBF2 0x00402a20
// `dynamic initializer for 'EntityDroid::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f30
// `dynamic atexit destructor for 'EntityDroid::sMemoryPool''
MemoryPool EntityDroid::sMemoryPool("EntityDroid", 0x460);

// FUNCTION: SWBF2 0x00499370
unsigned int EntityDroid::GetDerivedRtti() const
{
    return rttiHashEntityDroid;
}

// FUNCTION: SWBF2 0x00499380
const char* EntityDroid::GetDerivedRttiName() const
{
    return rttiNameEntityDroid;
}

// FUNCTION: SWBF2 0x004993a0
GameObject* EntityDroid::GetGameObject()
{
    return this;
}

// FUNCTION: SWBF2 0x004993b0
const GameObject* EntityDroid::GetGameObject() const
{
    return this;
}

// FUNCTION: SWBF2 0x004993c0
Entity* EntityDroid::GetEntity()
{
    return this;
}

// FUNCTION: SWBF2 0x004993d0
const Entity* EntityDroid::GetEntity() const
{
    return this;
}

// FUNCTION: SWBF2 0x004993e0
const PblMatrix& EntityDroid::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x004993f0
const PblVector3& EntityDroid::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00499400
void EntityDroid::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x00499420
TargetInfo* EntityDroid::GetTargetInfo()
{
    return &mTargetInfo;
}

// FUNCTION: SWBF2 0x00499430
Trackable* EntityDroid::GetTrackable()
{
    return this;
}

// FUNCTION: SWBF2 0x00499450
const Controllable* EntityDroid::GetControllable() const
{
    return this;
}

// FUNCTION: SWBF2 0x00499460
Controllable* EntityDroid::GetControllable()
{
    return this;
}

// FUNCTION: SWBF2 0x00499470
int EntityDroid::GetWeaponIndex(int iChannel) const
{
    return mWeaponIndex[iChannel];
}

// FUNCTION: SWBF2 0x00499490
Weapon* EntityDroid::GetWeaponPtr(int iIndex) const
{
    return mWeapon[iIndex];
}

// FUNCTION: SWBF2 0x004994b0
const Aimer* EntityDroid::GetAimer(int iIndex) const
{
    return mAimer[iIndex];
}

// FUNCTION: SWBF2 0x004994d0
EnergyBar* EntityDroid::GetEnergyBar()
{
    return reinterpret_cast<EnergyBar*>(mEnergyBar);
}

// FUNCTION: SWBF2 0x004994e0
EntityType EntityDroid::GetEntityType() const
{
    return ENT_DROID;
}

// FUNCTION: SWBF2 0x004994f0
float EntityDroid::GetSmallestCollisionDim() const
{
    return 0.3f;
}

// FUNCTION: SWBF2 0x00499500
bool EntityDroid::PostCollision::Update(float fDeltaTime)
{
    return mOwner->PostCollisionUpdate(fDeltaTime);
}

// SYNTHETIC: SWBF2 0x00499520
// EntityDroid::PostCollision::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00499550
// EntityDroid::PostCollision::~PostCollision

// GLOBAL: SWBF2 0x01ebbaa4
// SYNTHETIC: SWBF2 0x00402a50
// `dynamic initializer for 'EntityDroidClass::rttiHashEntityDroidClass''
PblHash EntityDroidClass::rttiHashEntityDroidClass("EntityDroidClass");
char EntityDroidClass::rttiNameEntityDroidClass[] = "EntityDroidClass";

// FUNCTION: SWBF2 0x00499560
bool EntityDroidClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityDroidClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004995a0
unsigned int EntityDroidClass::GetDerivedRtti()
{
    return rttiHashEntityDroidClass;
}

// FUNCTION: SWBF2 0x004995b0
const char* EntityDroidClass::GetDerivedRttiName()
{
    return rttiNameEntityDroidClass;
}

// FUNCTION: SWBF2 0x004995c0
int EntityDroidClass::GetWeaponCount()
{
    return mWeaponCount;
}

// FUNCTION: SWBF2 0x004995d0
const WeaponClass* EntityDroidClass::GetWeaponClass(int iIndex)
{
    return mWeaponClass[iIndex];
}

// FUNCTION: SWBF2 0x004995f0
const void* EntityDroidClass::GetControllableClass()
{
    return static_cast<ControllableClass*>(this);
}

// SYNTHETIC: SWBF2 0x00499610
// EntityDroid::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0049a390
bool EntityDroid::RenderTrackable(unsigned int uView) const
{
    if (mTargetInfo.mIsAiming)
    {
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0)
        {
            Weapon* pWeapon = mWeapon[iIndex];
            if (pWeapon && pWeapon->ZoomFirstPerson() &&
                NetGame::GetJoystickIndex(uView) == (int)FLRenderer::GetCurrentViewport())
            {
                return false;
            }
        }
    }
    return Trackable::RenderTrackable(uView);
}

// FUNCTION: SWBF2 0x0049a3f0
int EntityDroid::GetFirstPersonIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!mTracker || !mTracker->IsFirstPersonView())
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}

// FUNCTION: SWBF2 0x0049a470
int EntityDroid::GetJoystickIndex()
{
    if (mPlayerId < 0)
    {
        return -1;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsHost() && NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return NetGame::GetJoystickIndex(mPlayerId);
    }
    return -1;
}

// FUNCTION: SWBF2 0x0049ab70
void EntityDroid::TriggerDestructChargeEffect()
{
    if (!mClass->mDestructChargeEffect)
    {
        return;
    }
    if (mDestructChargeEffectHandle)
    {
        return;
    }
    FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mClass->mDestructChargeEffect);
    if (!pClass)
    {
        return;
    }
    FLEffectObject* pEffect = pClass->BuildEffect();
    if (!pEffect)
    {
        return;
    }
    pEffect->AttachEffectToObject(this, true);
    mDestructChargeEffectHandle = pEffect;
}

// FUNCTION: SWBF2 0x0049ac00
void EntityDroid::KillDelay(float fDelay)
{
    if (mState != EXPLODE && mState != KILLTIMER)
    {
        mState = EXPLODE;
        mKillTimer = fDelay;
    }
}

// FUNCTION: SWBF2 0x0049ac30
bool EntityDroid::IsForcedThirdPerson() const
{
    return mState != NORMAL;
}

// FUNCTION: SWBF2 0x0049ac40
void EntityDroid::FreezeMovement()
{
    if (mState != EXPLODE && mState != KILLTIMER)
    {
        mState = FROZEN;
    }
}

// FUNCTION: SWBF2 0x0049e180
template WeaponClass* Factory<Weapon, WeaponClass, WeaponDesc>::Find(unsigned int);

// FUNCTION: SWBF2 0x00499330
bool EntityDroid::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityDroid || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00499640
EntityDroid::EntityDroid(EntityDroidClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_SOFT, &pClass->mModel->_mLodData)
    , Controllable(pClass, desc.mPlayerId)
    , mPostCollision(this)
{
    new (mEnergyBar) EnergyBar(&pClass->mEnergyBarClass);
    mDestructChargeEffectHandle = NULL;

    mControllableList.AddLast(&this->Controllable::mNode, this);
    mMatrix = NormalMatrix(desc.mMatrix.GetZAxis(), PblVector3(0.0f, 1.0f, 0.0f), desc.mMatrix.GetTranslation());
    mClass = pClass;
    mEyeDir = mMatrix.GetZAxis();
    mEyePoint = pClass->mCameraSettings.CalcEyePoint(mMatrix);
    AddToScene();

    mVelocity.z = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.x = 0.0f;
    mPitchValue = 0.0f;
    mGroundRatio = 1.0f;
    for (int i = 0; i < 4; ++i)
    {
        mAimer[i] = 0;
        mWeapon[i] = 0;
    }
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (!mClass->mWeaponClass[i])
        {
            continue;
        }

        mAimer[i] = new Aimer(&pClass->mAimerClass[i], this, &mMatrix, 0, 0);

        WeaponDesc weaponDesc;
        weaponDesc.mOwner = this;
        weaponDesc.mAimer = mAimer[i];
        weaponDesc.mTrigger = &mControlFire[mClass->mWeaponChannel[i]];
        weaponDesc.mReload = &mControlReload;
        weaponDesc.mNumClips = 0;
        weaponDesc.m_pAmmoCounter = 0;
        weaponDesc.m_pEnergyBar = 0;

        const WeaponAmmoDesc& ammo = mClass->mWeaponAmmo[i];
        if (ammo.bFlagShareAmmo)
        {
            weaponDesc.m_pAmmoCounter = mWeapon[ammo.uiAmmo]->m_pAmmoCounter;
        }
        else if (ammo.bFlagAmmoInfinite)
        {
            weaponDesc.mNumClips = 0x7fffffff;
        }
        else
        {
            weaponDesc.mNumClips = ammo.uiAmmo;
        }
        if (ammo.bFlagShareHeat)
        {
            weaponDesc.m_pEnergyBar = mWeapon[ammo.uiHeat]->m_pEnergyBar;
        }

        mWeapon[i] = mClass->mWeaponClass[i]->Build(weaponDesc);
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        mWeaponIndex[iChannel] = -1;
        for (int i = 0; i < mClass->mWeaponCount; ++i)
        {
            if (mWeapon[i] && mClass->mWeaponChannel[i] == iChannel)
            {
                mWeaponIndex[iChannel] = i;
                break;
            }
        }
    }

    TargetManager::AddTarget(this, true);
    ClearControllable();
    NetGame::Created(this);
    mAmbientSound.Play(&pClass->mAmbientSound, &mMatrix.GetTranslation(), &mVelocity, true);
    mAmbient2Sound.Play(&pClass->mAmbient2Sound, &mMatrix.GetTranslation(), &mVelocity, true);
    mKillTimer = 0.0f;
    mState = NORMAL;
    mDestructChargeEffectHandle = NULL;
}

// FUNCTION: SWBF2 0x00499b10
EntityDroid::~EntityDroid()
{
    if (mDestructChargeEffectHandle)
    {
        mDestructChargeEffectHandle->StopAndFinish();
        mDestructChargeEffectHandle = NULL;
    }
    NetGame::Destroyed(this);
    mControllableList.Remove(&this->Controllable::mNode);
    TargetManager::RemoveTarget(this);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        delete mWeapon[i];
        delete mAimer[i];
    }
    mAmbientSound.Stop(true);
    mAmbient2Sound.Stop(true);
    reinterpret_cast<EnergyBar*>(mEnergyBar)->~EnergyBar();
}

// FUNCTION: SWBF2 0x00499ce0
void EntityDroid::Init()
{
    GameObject::Init();
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mWeapon[i])
        {
            mWeapon[i]->Init();
        }
    }
    for (int iChannel = 0; iChannel < 2; ++iChannel)
    {
        int iIndex = mWeaponIndex[iChannel];
        if (iIndex >= 0 && mWeapon[iIndex])
        {
            mWeapon[iIndex]->Select(iChannel, false);
        }
    }
    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        for (PblList<Controllable>::Itor it(GetControllableList()); it; ++it)
        {
            it->SetupController();
        }
    }
    mKillTimer = 0.0f;
    mState = NORMAL;
    if (mDestructChargeEffectHandle)
    {
        mDestructChargeEffectHandle->StopAndFinish();
        mDestructChargeEffectHandle = NULL;
    }
}

// FUNCTION: SWBF2 0x00499f00
void EntityDroid::Kill()
{
    mIsAlive = false;
    if (mDestructChargeEffectHandle)
    {
        mDestructChargeEffectHandle->StopAndFinish();
        mDestructChargeEffectHandle = NULL;
    }
    if (mTracker && mTracker->IsFirstPersonView())
    {
        int iJoystick = GetJoystickIndex();
        if (iJoystick >= 0)
        {
            FirstPerson::Activate(iJoystick, false);
            FirstPerson::Update(iJoystick, TYPE_COCKPIT, -2, 0, 0);
        }
    }
    GameObject::Kill();
    Controllable::Kill();
    mAmbientSound.Stop(true);
    mAmbient2Sound.Stop(true);
}

// FUNCTION: SWBF2 0x00499fe0
void EntityDroid::ApplyRadiusPush(const PblVector3& vPos, float fInnerRadius, float fOuterRadius, float fPush,
                                  bool bArg)
{
    const PblVector3& vCenter = GetCollisionCenter();
    PblVector3 vDiff(vCenter.x - vPos.x, vCenter.y - vPos.y, vCenter.z - vPos.z);
    float fDist = sqrtf(vDiff.x * vDiff.x + vDiff.y * vDiff.y + vDiff.z * vDiff.z);
    if (fDist > 0.0001f)
    {
        float fEffect = CalcRadiusEffect(fDist, fInnerRadius, fOuterRadius);
        if (fEffect > 0.0f)
        {
            PblVector3 vDir;
            D3DXVec3Normalize(&vDir, &vDiff);
            mVelocity.x = vDir.x * fPush * fEffect + mVelocity.x;
            mVelocity.y = vDir.y * fPush * fEffect + mVelocity.y;
            mVelocity.z = vDir.z * fPush * fEffect + mVelocity.z;
        }
    }
}

// FUNCTION: SWBF2 0x0049a100
void EntityDroid::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    PblMatrix mWorld = GetSmoothedMatrix();
    if (!netFrameLock && NetGame::IsEnabled())
    {
        gameObjectInterpolator.GetMatrix(mNetUniqueId, mWorld);
    }

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(int)(fDist * 255.0f);
    if (mClass->m_bDynamicLighting)
    {
        uiFlags &= ~0x80;
    }
    else
    {
        uiFlags |= 0x80;
    }

    PblMatrix mReflection;
    bool bReflected = !(uiFlags & 0x200000) &&
                      FLRenderer::IsReflected(*(const PblVector3*)&_Sphere, _Sphere.w, mReflection, false);
    unsigned int uiReflectFlags = (uiFlags & ~0x10000) | 0x10000100;
    if (bReflected)
    {
        PblMatrix mTemp;
        D3DXMatrixMultiply(&mTemp, &mWorld, &mReflection);
        mReflection = mTemp;
    }

    static PblHash sPlayerShadow("player_shadow");
    unsigned int uShadow = sPlayerShadow;
    if (GetControllable() && GetControllable()->mPilotType == PILOT_REMOTE)
    {
        uShadow = 0;
    }
    SetupShadow(this, mModel, ucViewport, uiFlags, uShadow);

    mModel->Render(ucViewport, fDist, &mWorld, 0, color, uiFlags, 0);
    if (bReflected)
    {
        mModel->Render(ucViewport, fDist, &mReflection, 0, color, uiReflectFlags | 0x40000000, 0);
    }
}

// FUNCTION: SWBF2 0x0049a4e0
bool EntityDroid::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                    Restrictor* pRestrictor)
{
    DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
    if (pOther->GetGameObject() && pOther->GetGameObject()->GetControllable())
    {
        owner.mCharacter = pOther->GetGameObject()->GetControllable()->mCharacter;
        owner.mGameObject = pOther->GetGameObject();
        owner.mTeam = pOther->GetGameObject()->GetTeam();
        owner.mClass = pOther->GetGameObject()->GetGameObjectClass();
    }

    PblVector3 vRelVel = mVelocity;
    if (pOther->GetGameObject())
    {
        const PblVector3& vOtherVel = pOther->GetGameObject()->GetVelocity();
        vRelVel.x -= vOtherVel.x;
        vRelVel.y -= vOtherVel.y;
        vRelVel.z -= vOtherVel.z;
    }
    const PblVector3& vSeparation = pResult->mSeparationNormal;
    float fApproach = vSeparation.y * vRelVel.y + vRelVel.x * vSeparation.x + vSeparation.z * vRelVel.z;

    if (pOther->GetCollisionType() >= GetCollisionType() && fApproach < 0.0f)
    {
        float fUp =
            mMatrix.m[1][1] * vSeparation.y + vSeparation.x * mMatrix.m[1][0] + mMatrix.m[1][2] * vSeparation.z;
        float fFront =
            mMatrix.m[2][1] * vSeparation.y + vSeparation.x * mMatrix.m[2][0] + mMatrix.m[2][2] * vSeparation.z;
        float fThreshold = mClass->mCollisionThreshold[1] * fUp + mClass->mCollisionThreshold[0] +
                           mClass->mCollisionThreshold[2] * fFront;

        FoleyFXCollider::CollisionCallback(pOther->GetCollisionType(), pOther, &GetPosition());
        PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);

        if (-fThreshold > fApproach)
        {
            float fExcess = fThreshold + fApproach;
            float fDamage = (mClass->mCollisionScale[1] * fUp + mClass->mCollisionScale[0] +
                             mClass->mCollisionScale[2] * fFront) *
                            fExcess * fExcess;
            if (pOther->GetGameObject())
            {
                fDamage = pOther->GetGameObject()->GetGameObjectClass()->GetCollisionInflict() * fDamage;
            }
            if (fDamage > 0.0f)
            {
                Damageable::ApplyDamage(DamageDesc(owner, fDamage), 0xffffffff);
            }
        }
    }

    if (pOther->GetCollisionType() == COLL_SOFT || !pRestrictor)
    {
        bool bResult = CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
        if (bResult && pRestrictor)
        {
            pRestrictor->AddRestriction(vSeparation);
        }
        return bResult;
    }

    if (mClass->mSetAltitude <= 0.0f && vSeparation.y > 0.5f)
    {
        mGroundNormal = vSeparation;
        mGroundRatio = 1.0f;
    }

    if (fApproach < 0.0f && IsAlive())
    {
        float fFriction = 0.25f;
        if (mClass->mSetAltitude == 0.0f)
        {
            float fUp =
                mMatrix.m[1][0] * vSeparation.x + mMatrix.m[1][1] * vSeparation.y + mMatrix.m[1][2] * vSeparation.z;
            if (fUp < 0.0f)
            {
                fUp = 0.0f;
            }
            fFriction = (1.0f - fUp * fUp * fUp * fUp) * 0.25f;
        }
        mVelocity.x -= vSeparation.x * fApproach;
        mVelocity.y -= vSeparation.y * fApproach;
        mVelocity.z -= vSeparation.z * fApproach;
        float fSlow = fFriction * fApproach * -1.125f /
                      sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z);
        if (fSlow >= 1.0f)
        {
            fSlow = 1.0f;
        }
        mVelocity.x -= fSlow * mVelocity.x;
        mVelocity.y -= fSlow * mVelocity.y;
        mVelocity.z -= fSlow * mVelocity.z;
        float fBounce = fApproach * 0.125f;
        mVelocity.x -= vSeparation.x * fBounce;
        mVelocity.y -= vSeparation.y * fBounce;
        mVelocity.z -= vSeparation.z * fBounce;
    }

    float fDist = pResult->mOverlap;
    PblVector3 vDir = pRestrictor->RestrictMovement(vSeparation, fDist);
    if (fDist > 5.0f)
    {
        fDist = 0.0f;
        Damageable::InstantDeath(owner, true);
    }
    PblVector3 vMove = pRestrictor->GetRestrictedMovement(vDir, fDist);
    SetPosition(PblVector3(vMove.x + mMatrix.m[3][0], mMatrix.m[3][1] + vMove.y, mMatrix.m[3][2] + vMove.z));
    pRestrictor->AddRestriction(vSeparation);
    return true;
}

// FUNCTION: SWBF2 0x0049ac60
bool EntityDroid::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Controllable::Update(fDeltaTime);

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        float fHeight = mMatrix.m[3][1];
        if (-500.0f > fHeight)
        {
            Kill();
            return false;
        }
        if (TriggerRegionManager::sInstance->mSinkHeight > fHeight)
        {
            KillDelay(1.0f);
            return true;
        }
        if (mState == FROZEN)
        {
            TriggerDestructChargeEffect();
        }
        else if (mDestructChargeEffectHandle)
        {
            mDestructChargeEffectHandle->StopAndFinish();
            mDestructChargeEffectHandle = NULL;
        }
    }

    if (mState != NORMAL)
    {
        mControlFire[0].ClearUntilPressed();
        mControlFire[1].ClearUntilPressed();
    }

    if (mState == EXPLODE)
    {
        RemoveFromScene();
        RemoveFromCollisionManager();
        if (mClass->mExplosion)
        {
            DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0,
                              1.0f);
            mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        mState = KILLTIMER;
        return true;
    }
    if (mState == KILLTIMER)
    {
        mKillTimer -= fDeltaTime;
        if (mKillTimer < 0.0f)
        {
            mKillTimer = 0.0f;
        }
        if (NetGame::IsEnabled() && NetGame::IsClient())
        {
            return true;
        }
        if (mKillTimer != 0.0f && mPlayerId >= 0)
        {
            return true;
        }
        Kill();
        mKillTimer = 0.0f;
        return false;
    }

    if (GetEnergyBar())
    {
        float fRestore = mClass->mEnergyAutoRestore;
        if (fRestore >= 0.0f)
        {
            float fMultiplier = mCharacter && mCharacter->mMedalsMgr &&
                                        mCharacter->mMedalsMgr->IsAwardAvailable(MedalsMgr::ENDURANCE)
                                    ? 1.5f
                                    : 1.0f;
            GetEnergyBar()->AddEnergy(fRestore * fMultiplier * fDeltaTime);
        }
        else
        {
            GetEnergyBar()->SpendEnergy(-(fRestore * fDeltaTime));
            if (GetEnergyBar()->m_fEnergy <= 0.0f)
            {
                KillDelay(1.0f);
                return true;
            }
        }
    }

    if (!IsAlive())
    {
        if (mClass->mExplosion && !(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam, mClass, 0,
                              1.0f);
            mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        RemoveFromCollisionManager();
        RemoveFromScene();
        Kill();
        if (!(NetGame::IsEnabled() && NetGame::IsClient()))
        {
            return false;
        }
    }

    if (!(NetGame::IsEnabled() && NetGame::IsClient()))
    {
        Controllable* pPilot = GetControllablePilot(this);
        if (pPilot && mState == NORMAL)
        {
            if (mControlUse.IsPressed() || IsDisabled())
            {
                if (mPilotType == PILOT_REMOTE)
                {
                    pPilot->ExitRemote();
                }

                else if (mPilotType == PILOT_VEHICLE || mPilotType == PILOT_VEHICLESELF)
                {
                    if (pPilot->mCharacter && pPilot->mCharacter->mRemote)
                    {
                        pPilot->ExitRemote();
                    }
                    pPilot->ExitVehicle(true, true);
                }
            }
            if ((mControlSwitch[0] > 0.0f || mControlSwitch[1] > 0.0f) && mPilotType == PILOT_REMOTE)
            {
                pPilot->ExitRemote();
            }
        }
    }

    if (!IsAlive())
    {
        return true;
    }

    UpdateStatus(fDeltaTime);

    if (mState != FROZEN)
    {
        for (int iChannel = 0; iChannel < 2; ++iChannel)
        {
            int iIndex = mWeaponIndex[iChannel];
            if (iIndex < 0 || mControlSwitch[iChannel] == 0.0f)
            {
                continue;
            }

            int iNew = iIndex;
            do
            {
                if (mControlSwitch[iChannel] < 0.0f)
                {
                    if (--iNew < 0)
                    {
                        iNew = mClass->mWeaponCount - 1;
                    }
                }
                else
                {
                    if (++iNew > mClass->mWeaponCount - 1)
                    {
                        iNew = 0;
                    }
                }
            } while (iNew != mWeaponIndex[iChannel] && mClass->mWeaponChannel[iNew] != iChannel);

            if (iNew != iIndex)
            {
                mWeapon[iIndex]->Deselect(iChannel, false);
                mWeaponIndex[iChannel] = iNew;
                mWeapon[iNew]->Select(iChannel, false);
                mTargetInfo.mIsAiming = false;
            }
            else if (NetGame::GetJoystickIndex(mPlayerId) >= 0)
            {
                WeaponClass::sWeaponChangeUnableSound.Play(&mMatrix.GetTranslation(), 0, false, true);
            }
        }
    }

    if (mPlayerId >= 0)
    {
        if (GameLoop::_bFirstPersonEnabled && netIsLocalTurn && mControlView.IsPressed() && !IsForcedThirdPerson() &&
            !IsForcedFirstPerson() && mClass->mForceMode != 1 && mClass->mForceMode != 2 && mTracker)
        {
            SwitchControlView(mPlayerId);
        }

        int iIndex = mWeaponIndex[0];
        if (mState != FROZEN && iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mClass->mZoomMax > 1.0f)
        {
            if (mControlZoom.IsPressed())
            {
                mTargetInfo.mIsAiming = mWeapon[iIndex]->CycleZoom(mTargetInfo.mIsAiming);
            }
            if (mWeapon[0]->mClass->mZoomRate != 0.0f)
            {
                mWeapon[0]->UpdateZoom(fDeltaTime, mControlMove);
            }
        }
        else
        {
            mTargetInfo.mIsAiming = false;
        }
    }

    float fMove = 0.0f;
    float fStrafe = 0.0f;
    if (!(mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]] &&
          mWeapon[mWeaponIndex[0]]->mClass->mZoomRate != 0.0f) &&
        mState != FROZEN)
    {
        fMove = mControlMove;
        fStrafe = mControlStrafe;
    }
    float fTurn = mControlTurn;
    float fPitch = mControlPitch;

    float fPitchSpeed = (mPlayerId >= 0 && NetGame::IsMouseMode(mPlayerId)) ? mClass->mMaxTurnSpeed
                                                                            : mClass->mMaxPitchSpeed;
    if (mTargetInfo.mIsAiming && mWeaponIndex[0] >= 0 && mWeapon[mWeaponIndex[0]])
    {
        fPitchSpeed /= mWeapon[mWeaponIndex[0]]->mZoomTurnScale;
    }
    float fPitchValue = fPitchSpeed * fPitch * fDeltaTime + mPitchValue;
    if (mClass->mMinPitch > fPitchValue)
    {
        fPitchValue = mClass->mMinPitch;
    }
    else if (fPitchValue > mClass->mMaxPitch)
    {
        fPitchValue = mClass->mMaxPitch;
    }
    mPitchValue = fPitchValue;

    Move(fDeltaTime, fMove, fStrafe, fTurn);

    mEyePoint = mClass->mCameraSettings.CalcEyePoint(mMatrix);
    float fSin;
    float fCos;
    PblMath::SinCos(&fSin, &fCos, mPitchValue);
    PblVector3 vLocal;
    vLocal.x = 0.0f;
    vLocal.y = fSin;
    vLocal.z = fCos;
    PblVector3 vEyeDir;
    D3DXVec3TransformNormal(&vEyeDir, &vLocal, &mMatrix);
    mEyeDir = vEyeDir;

    if (mPlayerId >= 0 && mTracker)
    {
        if (mTargetInfo.mIsAiming || mTracker->IsFirstPersonView())
        {
            mTargetInfo.mAimStart = mEyePoint;
        }
        else
        {

            PblMatrix mLook =
                DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z), mMatrix.GetYAxis(), gVectorZero);
            PblQuaternion qLook = PblQuaternion(mLook);
            float fTiltSin;
            float fTiltCos;
            PblMath::SinCos(&fTiltSin, &fTiltCos, -mClass->mCameraSettings.GetCameraTiltValue() * 0.5f);
            PblQuaternion qTilt;
            qTilt._fX = fTiltSin * gVectorUnitX.x;
            qTilt._fY = fTiltSin * gVectorUnitX.y;
            qTilt._fZ = fTiltSin * gVectorUnitX.z;
            qTilt._fS = fTiltCos;
            PblQuaternion qCamera = qLook * qTilt;

            PblQuaternion qUnit;
            PblMatrix mCamera;
            D3DXMatrixRotationQuaternion(&mCamera, D3DXQuaternionNormalize(&qUnit, &qCamera));
            mCamera.m[3][0] = gVectorZero.x;
            mCamera.m[3][1] = gVectorZero.y;
            mCamera.m[3][2] = gVectorZero.z;
            mCamera.m[3][3] = 1.0f;

            mTargetInfo.mAimStart = mClass->mCameraSettings.CalcCameraPoint(mMatrix, mCamera);
            float fAlong = mEyeDir.y * (mEyePoint.y - mTargetInfo.mAimStart.y) +
                           mEyeDir.x * (mEyePoint.x - mTargetInfo.mAimStart.x) +
                           mEyeDir.z * (mEyePoint.z - mTargetInfo.mAimStart.z);
            mTargetInfo.mAimStart.x = fAlong * mEyeDir.x + mTargetInfo.mAimStart.x;
            mTargetInfo.mAimStart.y = mTargetInfo.mAimStart.y + mEyeDir.y * fAlong;
            mTargetInfo.mAimStart.z = mTargetInfo.mAimStart.z + mEyeDir.z * fAlong;
        }
        const GameObject* pIgnore = this;
        CollisionObject* pHit;
        float fDist =
            CollisionManager::RayHit(mTargetInfo.mAimStart, mEyeDir, 512.0f, pHit, 0, &pIgnore, 1, 0x9a, true) *
            512.0f;
        mTargetInfo.mAimPoint = PblVector3(mTargetInfo.mAimStart.x + mEyeDir.x * fDist,
                                           mTargetInfo.mAimStart.y + mEyeDir.y * fDist,
                                           mTargetInfo.mAimStart.z + mEyeDir.z * fDist);
    }
    else
    {
        mTargetInfo.mAimStart = mEyePoint;
        float fDist = 512.0f;
        int iIndex = mWeaponIndex[0];
        if (iIndex >= 0 && mWeapon[iIndex] && mWeapon[iIndex]->mTarget)
        {
            PblVector3 vTarget = mWeapon[iIndex]->mTarget->GetTargetPoint(mEyePoint, mEyeDir, -1);
            fDist = sqrtf((vTarget.x - mEyePoint.x) * (vTarget.x - mEyePoint.x) +
                          (vTarget.y - mEyePoint.y) * (vTarget.y - mEyePoint.y) +
                          (vTarget.z - mEyePoint.z) * (vTarget.z - mEyePoint.z));
        }
        mTargetInfo.mAimPoint = PblVector3(mEyeDir.x * fDist + mEyePoint.x, mEyePoint.y + mEyeDir.y * fDist,
                                           mEyePoint.z + mEyeDir.z * fDist);
    }

    mGroundRatio = 0.0f;
    mAmbientSound.Update(&mClass->mAmbientSound, &mMatrix.GetTranslation(), &mVelocity);
    mAmbient2Sound.Update(&mClass->mAmbient2Sound, &mMatrix.GetTranslation(), &mVelocity);
    if (!mAmbientSound.GetPlaying())
    {
        mAmbientSound.Play(&mClass->mAmbientSound, &mMatrix.GetTranslation(), &mVelocity, true);
    }
    if (!mAmbient2Sound.GetPlaying())
    {
        mAmbient2Sound.Play(&mClass->mAmbient2Sound, &mMatrix.GetTranslation(), &mVelocity, true);
    }
    return true;
}

// FUNCTION: SWBF2 0x0049baa0
bool EntityDroid::PostCollisionUpdate(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    Smooth(fDeltaTime);
    if (!netFrameLock && NetGame::IsEnabled())
    {
        if (!NetGame::IsClient() || !netIsLocalTurn)
        {
            if (!NetGame::IsHost() || !NetGame::IsOneWorld() || netIsLateTurn)
            {
                return true;
            }
            if (mNetUniqueId >= 0)
            {
                mNetUniqueId = NetGame::GetHostWorldUniqueId(mNetUniqueId, this);
            }
        }
        gameObjectInterpolator.SetNext(mNetUniqueId, &GetSmoothedMatrix());
    }
    return true;
}

// FUNCTION: SWBF2 0x0049bb70
void EntityDroid::Move(float fDeltaTime, float fMove, float fStrafe, float fTurn)
{
    PblVector3 vOmega = gVectorZero;
    const GameObject* pIgnore = this;
    CollisionObject* pHit;
    float fHeight = 0.0f;
    PblVector3 vUp;

    if (mClass->mSetAltitude > 0.0f)
    {
        PblVector3 vStart = GetCollisionCenter();
        PblVector3 vDown(0.0f, -1.0f, 0.0f);
        float fLength = mClass->mSetAltitude * 10.0f + GetCollisionRadius();
        PblVector3 vNormal = gVectorUnitY;
        float fHit = CollisionManager::RayHit(vStart, vDown, fLength, pHit, &vNormal, &pIgnore, 1, 0x190, false);
        mGroundNormal = vNormal;
        fHeight = fHit * fLength + mMatrix.m[3][1] - vStart.y;

        if (mVelocity.x * mVelocity.x + mVelocity.z * mVelocity.z > 25.0f)
        {
            PblVector3 vAhead(vDown.x * fLength + mVelocity.x, mVelocity.y + vDown.y * fLength,
                              mVelocity.z + vDown.z * fLength);
            float fAheadLength = sqrtf(vAhead.y * vAhead.y + vAhead.x * vAhead.x + vAhead.z * vAhead.z);
            float fInv = 1.0f / fAheadLength;
            PblVector3 vAheadDir(vAhead.x * fInv, vAhead.y * fInv, vAhead.z * fInv);
            float fAheadHit =
                CollisionManager::RayHit(vStart, vAheadDir, fAheadLength, pHit, &vNormal, &pIgnore, 1, 0x190, false);
            if (1.0f > fAheadHit)
            {
                float fAheadDist = fAheadHit * fAheadLength;
                float fDrop = -((vAheadDir.x * fAheadDist) * vNormal.x + (vAheadDir.y * fAheadDist) * vNormal.y +
                                vNormal.z * (vAheadDir.z * fAheadDist));
                if (!(fDrop > fHeight))
                {
                    fHeight = fDrop;
                }
                if (vNormal.y > 0.0f)
                {
                    float fBlend = (1.0f - fAheadHit) * vNormal.y;
                    PblVector3 vBlend((vNormal.x - mGroundNormal.x) * fBlend + mGroundNormal.x,
                                      (vNormal.y - mGroundNormal.y) * fBlend + mGroundNormal.y,
                                      (vNormal.z - mGroundNormal.z) * fBlend + mGroundNormal.z);
                    PblVector3 vGround;
                    D3DXVec3Normalize(&vGround, &vBlend);
                    mGroundNormal = vGround;
                }
            }
        }

        float fRatio = 1.0f - (fHeight - mClass->mSetAltitude) / (mClass->mSetAltitude * 9.0f);
        if (0.0f > fRatio)
        {
            fRatio = 0.0f;
        }
        else if (fRatio > 1.0f)
        {
            fRatio = 1.0f;
        }
        mGroundRatio = fRatio;
        float fLift = mClass->mSetAltitude > 0.0f ? 0.1f : 0.0f;
        PblVector3 vTilt(mGroundNormal.x * fRatio, mGroundNormal.y + fLift, mGroundNormal.z * fRatio);
        D3DXVec3Normalize(&vUp, &vTilt);
    }
    else if (mGroundRatio == 1.0f)
    {
        PblVector3 vStart(mMatrix.m[3][0] + 0.0f, mMatrix.m[3][1] + GetCollisionRadius(), mMatrix.m[3][2] + 0.0f);
        PblVector3 vDown(0.0f, -1.0f, 0.0f);
        CollisionManager::RayHit(vStart, vDown, 32.0f, pHit, &mGroundNormal, &pIgnore, 1, 0x90, false);
        vUp = mGroundNormal;
    }
    else
    {
        vUp.x = 0.0f;
        vUp.y = 1.0f;
        vUp.z = 0.0f;
    }

    PblMatrix mGround = NormalMatrix(mMatrix.GetZAxis(), mGroundNormal, gVectorZero);
    mVelocity.y = (-9.8f - (mClass->mGravityScale - 1.0f) * mGroundRatio * 9.8f) * fDeltaTime + mVelocity.y;
    if (mClass->mSetAltitude > 0.0f)
    {
        float fLift = (mClass->mSetAltitude - fHeight) * mClass->mLiftSpring + mGroundRatio * 9.8f -
                      (mGroundNormal.y * mVelocity.y + mVelocity.x * mGroundNormal.x +
                       mGroundNormal.z * mVelocity.z) *
                          mClass->mLiftDamp;
        if (fLift > 0.0f)
        {
            fLift *= fDeltaTime;
            mVelocity.x = fLift * mGroundNormal.x + mVelocity.x;
            mVelocity.y = mVelocity.y + mGroundNormal.y * fLift;
            mVelocity.z = mVelocity.z + mGroundNormal.z * fLift;
        }
    }

    float fDamp = mClass->mLevelDamp * fDeltaTime;
    if (fDamp > 1.0f)
    {
        fDamp = 1.0f;
    }
    float fSpring = mClass->mLevelSpring * fDeltaTime;
    if (fSpring > 1.0f)
    {
        fSpring = 1.0f;
    }
    vOmega.x = (mMatrix.m[2][1] * vUp.y + vUp.x * mMatrix.m[2][0] + mMatrix.m[2][2] * vUp.z) * fSpring +
               (1.0f - fDamp) * vOmega.x;
    vOmega.z = (1.0f - fDamp) * vOmega.z -
               (mMatrix.m[0][1] * vUp.y + vUp.x * mMatrix.m[0][0] + mMatrix.m[0][2] * vUp.z) * fSpring;

    float fClimb = mGroundNormal.y > 0.0f ? mGroundNormal.y : 0.0f;
    float fAccel = fClimb * mGroundRatio * fDeltaTime * mClass->mAcceleration;
    float fStrafeDelta =
        -mClass->mMaxStrafeSpeed * fStrafe -
        (mVelocity.y * mGround.m[0][1] + mVelocity.x * mGround.m[0][0] + mVelocity.z * mGround.m[0][2]);
    float fForwardDelta =
        mClass->mMaxSpeed * fMove -
        (mVelocity.y * mGround.m[2][1] + mGround.m[2][0] * mVelocity.x + mVelocity.z * mGround.m[2][2]);
    if (-fAccel > fForwardDelta)
    {
        fForwardDelta = -fAccel;
    }
    else if (fForwardDelta > fAccel)
    {
        fForwardDelta = fAccel;
    }
    mVelocity.x = fForwardDelta * mGround.m[2][0] + mVelocity.x;
    mVelocity.y = mVelocity.y + fForwardDelta * mGround.m[2][1];
    mVelocity.z = mVelocity.z + fForwardDelta * mGround.m[2][2];
    if (-fAccel > fStrafeDelta)
    {
        fStrafeDelta = -fAccel;
    }
    else if (fStrafeDelta > fAccel)
    {
        fStrafeDelta = fAccel;
    }
    mVelocity.x = fStrafeDelta * mGround.m[0][0] + mVelocity.x;
    mVelocity.y = mVelocity.y + fStrafeDelta * mGround.m[0][1];
    mVelocity.z = mVelocity.z + fStrafeDelta * mGround.m[0][2];

    bool bMouse = false;
    if (mPlayerId >= 0)
    {
        bMouse = NetGame::IsMouseMode(mPlayerId);
    }
    float fTurnSpeed;
    if (mPlayerId >= 0 && !bMouse)
    {
        fTurnSpeed = (mClass->mMaxTurnSpeed - mClass->mMaxPitchSpeed) * mTurnBuildup + mClass->mMaxPitchSpeed;
    }
    else
    {
        fTurnSpeed = mClass->mMaxTurnSpeed;
    }
    vOmega.y = -fTurnSpeed * fTurn;

    PblQuaternion qOrient(mMatrix);
    PblVector3 vPos = mMatrix.GetTranslation();
    float fRate = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fRate > 0.0001f)
    {
        float fInv = 1.0f / fRate;
        PblVector3 vAxis(fInv * vOmega.x, fInv * vOmega.y, fInv * vOmega.z);
        float fSin;
        float fCos;
        PblMath::SinCos(&fSin, &fCos, fRate * fDeltaTime * 0.5f);
        PblQuaternion qDelta;
        qDelta._fX = vAxis.x * fSin;
        qDelta._fY = fSin * vAxis.y;
        qDelta._fZ = fSin * vAxis.z;
        qDelta._fS = fCos;
        qOrient = qOrient * qDelta;
    }

    PblVector3 vNewPos(vPos.x + mVelocity.x * fDeltaTime, vPos.y + mVelocity.y * fDeltaTime,
                       vPos.z + mVelocity.z * fDeltaTime);
    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXMatrixRotationQuaternion(&mNew, D3DXQuaternionNormalize(&qUnit, &qOrient));
    mNew.m[3][0] = vNewPos.x;
    mNew.m[3][1] = vNewPos.y;
    mNew.m[3][2] = vNewPos.z;
    mNew.m[3][3] = 1.0f;
    SetMatrix(mNew);
    if (mClass->mSetAltitude <= 0.0f)
    {
        mGroundRatio = 0.0f;
    }
}

// FUNCTION: SWBF2 0x0049c700
void EntityDroid::RecalculateAABB()
{
    SetCollisionRadius(1.5f);
    Set2DCollisionRadius(1.5f);
    const PblVector3& vCenter = GetCollisionCenter();
    mAABB = PblBox(vCenter.x - 2.5f, vCenter.y - 2.5f, vCenter.z - 2.5f, vCenter.x + 2.5f, vCenter.y + 2.5f,
                   vCenter.z + 2.5f);
}

// FUNCTION: SWBF2 0x0049c7d0
bool EntityDroid::UpdateIndirect(float fDeltaTime)
{
    AILowLevel& lowLevel = UnitControllerPtr(mCtrl)->GetLowLevel();
    if (!lowLevel.UpdateIndirect(fDeltaTime) || !mCtrl)
    {
        return true;
    }

    ControllableHeading heading = lowLevel.mHeading;
    lowLevel.ProcessFire(fDeltaTime, &heading);
    float fPitch = mControlPitch;
    float fStrafe = mControlStrafe;
    float fThrust = mControlMove;
    float fTurn = mControlTurn;
    AIUtil::UpdatePhysics(this, fDeltaTime, lowLevel.mDest, heading, mClass->mMaxSpeed, mClass->mMaxStrafeSpeed,
               mClass->mMaxTurnSpeed, mClass->mMaxPitchSpeed, &fThrust, &fStrafe, &fTurn, &fPitch);
    mControlStrafe = fStrafe;
    mControlMove = fThrust;
    mControlTurn = fTurn;
    mControlPitch = fPitch;
    return true;
}

// FUNCTION: SWBF2 0x0049c900
void EntityDroid::SetupController()
{
    Controller* pController = 0;
    if (mPlayerId >= 0)
    {
        if (!(NetGame::IsEnabled() && NetGame::IsClient()) || NetGame::GetJoystickIndex(mPlayerId) >= 0)
        {
            pController = new PlayerController(this);
        }
        Track(mPlayerId);
    }
    else
    {
        if (!mPilot)
        {
            return;
        }
        if (NetGame::IsEnabled() && NetGame::IsClient())
        {
            return;
        }
        pController = new UnitMobileController(this);
    }
    if (pController)
    {
        AddController(pController, mPlayerId < 0);
    }
}

// FUNCTION: SWBF2 0x0049ce30
const ControllableClass* EntityDroid::GetControllableClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x0049ce50
ControllableClass* EntityDroid::GetControllableClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x0049cea0
int EntityDroid::GetWeaponCount() const
{
    return mClass->mWeaponCount;
}

// FUNCTION: SWBF2 0x0049ceb0
float EntityDroid::GetSoundRadius() const
{
    const PblVector3& vVelocity = GetVelocity();
    float fRadius =
        sqrtf(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z) * mClass->mNoiseFactor;
    if (0.0f > fRadius)
    {
        return 0.0f;
    }
    if (fRadius > 50.0f)
    {
        return 50.0f;
    }
    return fRadius;
}

// FUNCTION: SWBF2 0x0049cf50
void EntityDroid::ActivatePhysics()
{
    ActivateThread(0, PER_TURN);
    mPostCollision.ActivateThread(-15, PER_TURN);
    for (int i = 0; i < mClass->mWeaponCount; ++i)
    {
        if (mAimer[i])
        {
            mAimer[i]->ActivatePhysics();
        }
    }
}

// FUNCTION: SWBF2 0x0049cff0
float EntityDroid::GetMaxSpeed() const
{
    return mClass->mMaxSpeed;
}

// FUNCTION: SWBF2 0x0049e0b0
Entity* EntityDroidClass::Build(const EntityDesc& desc)
{
    if (EntityDroid::sMemoryPool.IsFull())
    {
        return 0;
    }
    EntityDroid* pDroid = new EntityDroid(this, desc);
    if (pDroid)
    {
        pDroid->AddSoftToCollisionManager();
        pDroid->ActivatePhysics();
        pDroid->BuildEffects();
        for (PblList<Controllable>::Itor it(pDroid->GetControllableList()); it; ++it)
        {
            it->SetupController();
        }
    }
    return pDroid;
}

// SYNTHETIC: SWBF2 0x0049e20a
// [thunk]:EntityDroid::GetMatrix`adjustor{564}'

// SYNTHETIC: SWBF2 0x0049e220
// [thunk]:EntityDroid::GetEntity`adjustor{12}'

// SYNTHETIC: SWBF2 0x0049e230
// [thunk]:EntityDroid::GetGameObject`adjustor{320}'

// SYNTHETIC: SWBF2 0x0049e246
// [thunk]:EntityDroid::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x0049e251
// [thunk]:EntityDroid::GetGameObject`adjustor{600}'

// SYNTHETIC: SWBF2 0x0049e25c
// [thunk]:EntityDroid::GetGameObject`adjustor{12}'

// SYNTHETIC: SWBF2 0x0049e26c
// [thunk]:EntityDroid::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x0049e277
// [thunk]:EntityDroid::GetEntity`adjustor{576}'

// SYNTHETIC: SWBF2 0x0049e2cc
// [thunk]:EntityDroid::`scalar deleting destructor'`adjustor{576}'

// SYNTHETIC: SWBF2 0x0049e215
// [thunk]:EntityDroid::`scalar deleting destructor' adjustor{148}
// SYNTHETIC: SWBF2 0x0049e228
// [thunk]:EntityDroid::GetEntity adjustor{12}
// SYNTHETIC: SWBF2 0x0049e23b
// [thunk]:EntityDroid::GetGameObject adjustor{320}
// SYNTHETIC: SWBF2 0x0049e264
// [thunk]:EntityDroid::GetGameObject adjustor{12}
// SYNTHETIC: SWBF2 0x0049e282
// [thunk]:EntityDroid::GetControllable adjustor{600}
// SYNTHETIC: SWBF2 0x0049e28d
// [thunk]:EntityDroid::GetControllable adjustor{600}
// SYNTHETIC: SWBF2 0x0049e298
// [thunk]:EntityDroid::Kill adjustor{256}
// SYNTHETIC: SWBF2 0x0049e2a3
// [thunk]:EntityDroid::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x0049e2ae
// [thunk]:EntityDroid::`scalar deleting destructor' adjustor{600}
// SYNTHETIC: SWBF2 0x0049e2b9
// [thunk]:EntityDroid::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x0049e2c1
// [thunk]:EntityDroid::`scalar deleting destructor' adjustor{512}

// FUNCTION: SWBF2 0x005eedf0
const PblMatrix& EntityDroid::GetSmoothedMatrix() const
{
    if (NetGame::IsEnabled() && mNetUniqueId != -1 && NetGame::IsClient())
    {
        return NetPredict::GetSmoothInfo(mNetUniqueId).GetRenderMatrix();
    }
    return mMatrix;
}

// FUNCTION: SWBF2 0x0049ca60
bool EntityDroid::ShouldShowReticule() const
{
    return mState == NORMAL && mIsAlive;
}

// FUNCTION: SWBF2 0x0049ce70
float EntityDroid::GetAimTurnRate() const
{
    return mClass->mMaxPitchSpeed;
}

// FUNCTION: SWBF2 0x0049ce80
void EntityDroid::SetWeaponIndex(int iIndex, int iChannel)
{
    mWeaponIndex[iChannel] = iIndex;
}

// FUNCTION: SWBF2 0x0049cfc0
void EntityDroid::DeactivatePhysics()
{
    mPostCollision.DeactivateThread();
    static_cast<Thread*>(static_cast<Controllable*>(this))->DeactivateThread();
}

// FUNCTION: SWBF2 0x0049d000
float EntityDroid::GetMaxStrafe() const
{
    return mClass->mMaxStrafeSpeed;
}

// FUNCTION: SWBF2 0x0049cdf0
void EntityDroid::SetupCameraZoomFactor(float& rZoom)
{
    rZoom = 1.0f;
    const int iIndex = mWeaponIndex[0];
    if (mTargetInfo.mIsAiming && iIndex >= 0 && mWeapon[iIndex])
    {
        rZoom = mWeapon[iIndex]->mZoom;
    }
}

// FUNCTION: SWBF2 0x0049ccb0
CameraTrackSetting::CameraTrackSetting(const CameraTrackSetting& other)
    : mCameraEyePointOffset(other.mCameraEyePointOffset),
      mCameraTrackCenter(other.mCameraTrackCenter),
      mCameraTrackOffset(other.mCameraTrackOffset),
      mCameraSafePoint(other.mCameraSafePoint),
      mCameraMoveTensionL(other.mCameraMoveTensionL),
      mCameraMoveTensionR(other.mCameraMoveTensionR),
      mCameraMoveTensionU(other.mCameraMoveTensionU),
      mCameraMoveTensionD(other.mCameraMoveTensionD),
      mCameraMoveTensionF(other.mCameraMoveTensionF),
      mCameraMoveTensionB(other.mCameraMoveTensionB),
      mCameraAimTension(other.mCameraAimTension),
      mCameraTiltValue(other.mCameraTiltValue)
{
}

// FUNCTION: SWBF2 0x00499e10
void EntityDroid::ClientKill()
{
    if (mClass->mExplosion && mState != KILLTIMER)
    {
        DamageOwner owner(mDamageOwner.mCharacter, mDamageOwner.mGameObject, mDamageOwner.mTeam,
                          mClass, 0, 1.0f);
        mClass->mExplosion->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
    }
    if (mDestructChargeEffectHandle)
    {
        mDestructChargeEffectHandle->StopAndFinish();
        mDestructChargeEffectHandle = 0;
    }
}

// FUNCTION: SWBF2 0x0049cd30
void EntityDroid::SetupInterpolatedCameraTrackMatrix(PblMatrix& rMatrix, float)
{
    if (IsAlive() && mTracker && mTracker->IsFirstPersonView())
    {
        const int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateAimer(iFirstPerson, rMatrix);
        }
    }
}

extern float GetDefaultFOV();

// FUNCTION: SWBF2 0x0049cd80
void EntityDroid::SetupCameraFOV(float& rFOV)
{
    if (!IsAlive())
    {
        rFOV = GetDefaultFOV();
        return;
    }
    const ControllableClass* pClass = GetControllableClass();
    if (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView()))
    {
        rFOV = pClass->mFirstPersonFOV;
    }
    else
    {
        rFOV = pClass->mThirdPersonFOV;
    }
}

// FUNCTION: SWBF2 0x0049ca80
void EntityDroid::SetupCameraTrackMatrix(PblMatrix& rMatrix, float fDeltaTime)
{

    static PblVector3 vCameraOffset(0.0f, 0.0f, 8.5f);
    (void)&vCameraOffset;
    CameraTrackSetting settings = mClass->mCameraSettings;
    if (mState == FROZEN || mState == EXPLODE || mState == KILLTIMER)
    {
        settings.mCameraTrackOffset = mClass->mCameraDestructTrackOffset;
        settings.mCameraTiltValue = mClass->mCameraDestructTiltValue;
    }
    if (IsAlive() && (mTargetInfo.mIsAiming || (mTracker && mTracker->IsFirstPersonView())))
    {
        const float fEyeDist = mTargetInfo.mIsAiming ? 0.5f : 0.3f;
        PblVector3 vEye(mEyePoint.x - mEyeDir.x * fEyeDist,
                        mEyePoint.y - mEyeDir.y * fEyeDist,
                        mEyePoint.z - mEyeDir.z * fEyeDist);
        rMatrix = DirectionMatrix(PblVector3(-mEyeDir.x, -mEyeDir.y, -mEyeDir.z),
                                  mMatrix.GetYAxis(), vEye);
        const int iFirstPerson = GetFirstPersonIndex();
        if (iFirstPerson >= 0 && !mTargetInfo.mIsAiming)
        {
            FirstPerson::UpdateCamera(iFirstPerson, rMatrix);
        }
        return;
    }
    settings.SetupCameraTrackMatrix(this, rMatrix, fDeltaTime, mMatrix, mEyeDir,
                                    1.0f, 1.0f, this);
}

// FUNCTION: SWBF2 0x0049e2e0
unsigned int* __fastcall InitializeSpawnCameraFloatPair(unsigned int* pThis)
{
    pThis[0] = 0x3f800000u;
    pThis[1] = 0;
    return pThis;
}
