#include "OrdnanceBullet.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"

#include <stdio.h>

#include <math.h>

#include "BirdEffect.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "Controllable.h"
#include "Damageable.h"
#include "EntityLight.h"
#include "GameObject.h"
#include "LeafEffect.h"
#include "NetGame.h"
#include "NetPredict.h"
#include "WaterEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/RedWater.h"

extern bool netIsLocalTurn;
extern bool netFrameLock;
extern bool netIsLateTurn;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

GameObject* ResolveGameObjectHandle(PblHandle<GameObject>& handle);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace
{

    unsigned int GameObjectHandleId(const GameObject* pObject)
    {
        return pObject->mHandleId;
    }

    int GameObjectTeam(const GameObject* pObject)
    {
        return pObject->GetTeam();
    }

    EntityClass* GetEntityClass(GameObject* pObject)
    {
        return pObject->GetEntityClass();
    }

    Controllable* GetControllable(GameObject* pObject)
    {
        return pObject->GetControllable();
    }
}

// GLOBAL: SWBF2 0x01ef2620
// SYNTHETIC: SWBF2 0x00405a60
// `dynamic initializer for 'OrdnanceBulletClass::rttiHashOrdnanceBulletClass''
PblHash OrdnanceBulletClass::rttiHashOrdnanceBulletClass("OrdnanceBulletClass");
char OrdnanceBulletClass::rttiNameOrdnanceBulletClass[] = "OrdnanceBulletClass";

static const RedColor kColorZero = { 0, 0, 0, 0 };

// FUNCTION: SWBF2 0x005fbb60
OrdnanceBulletClass::OrdnanceBulletClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mLifeSpan = 2.0f;

    mModel = 0;
    mStraightBullet = true;
    mFadeColorLight = false;
    mLodData = OrdnanceLodData();
    mVelocity = 150.0f;
    mImpactEffectSoft = 0;
    mImpactEffectRigid = 0;
    mImpactEffectStatic = 0;
    mImpactEffectTerrain = 0x72968E25u;
    mImpactEffectWater = 0x4955BD1Bu;
    mImpactEffectShieldSmall = 0xB3503FC5u;
    mImpactEffectShieldLarge = 0xD4391A69u;
    mImpactDecalStatic = 0;
    mImpactDecalTerrain = 0;
    m_uiLightOdfNameHash = 0;
    m_colorLight = kColorZero;
    m_endColorLight = kColorZero;
    m_fRadiusLight = 0.0f;
}

// FUNCTION: SWBF2 0x005fbca0
OrdnanceBulletClass::OrdnanceBulletClass(OrdnanceBulletClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mModel = pBase->mModel;
    mStraightBullet = pBase->mStraightBullet;
    mFadeColorLight = pBase->mFadeColorLight;
    mVelocity = pBase->mVelocity;
    mLodData = OrdnanceLodData();
    mImpactEffectSoft = pBase->mImpactEffectSoft;
    mImpactEffectRigid = pBase->mImpactEffectRigid;
    mImpactEffectStatic = pBase->mImpactEffectStatic;
    mImpactEffectTerrain = pBase->mImpactEffectTerrain;
    mImpactEffectWater = pBase->mImpactEffectWater;
    mImpactEffectShieldSmall = pBase->mImpactEffectShieldSmall;
    mImpactEffectShieldLarge = pBase->mImpactEffectShieldLarge;
    mImpactDecalTerrain = pBase->mImpactDecalTerrain;
    mImpactDecalStatic = pBase->mImpactDecalStatic;
    m_uiLightOdfNameHash = pBase->m_uiLightOdfNameHash;
    m_colorLight = pBase->m_colorLight;
    m_endColorLight = pBase->m_endColorLight;
    m_fRadiusLight = pBase->m_fRadiusLight;
}

// FUNCTION: SWBF2 0x005f8b30
bool OrdnanceBulletClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceBulletClass || OrdnanceClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005fa870
unsigned int OrdnanceBulletClass::GetDerivedRtti()
{
    return rttiHashOrdnanceBulletClass;
}

// FUNCTION: SWBF2 0x005fa880
const char* OrdnanceBulletClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceBulletClass;
}

// FUNCTION: SWBF2 0x005fbde0
void OrdnanceBulletClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x746CE73E)
    {
        int r, g, b, a;
        int iCount = sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a);
        RedColor color;
        if (iCount >= 4)
        {
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
        }
        else
        {
            return;
        }
        color.a = 255;
        m_colorLight = color;
    }
    else if (uProperty == 0x96E0CCB1)
    {
        int r, g, b, a;
        int iCount = sscanf(pszValue, "%d %d %d %d", &r, &g, &b, &a);
        RedColor color;
        if (iCount >= 4)
        {
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
        }
        else if (iCount == 3)
        {
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
        }
        else
        {
            return;
        }
        color.a = 255;
        m_endColorLight = color;
    }
    else if (uProperty == 0x32741C32)
    {
        sscanf(pszValue, "%f", &mVelocity);
    }
    else if (uProperty == 0x2776564D)
    {
        sscanf(pszValue, "%f", &m_fRadiusLight);
    }
    else if (uProperty == 0x087EA2F0)
    {
        mImpactEffectStatic = PblHash(pszValue);
    }
    else if (uProperty == 0x2BBD32B1)
    {
        mImpactDecalTerrain = PblHash(pszValue);
    }
    else if (uProperty == 0x3F30370C)
        ;
    else if (uProperty == 0x47C86B4A)
    {
        if (*pszValue)
        {
            mModel = RedModel::_HashTable.Find(PblHash(pszValue));
            mLodData = ModelLodData(0, 0, 0, mModel, 0);
        }
        else
        {
            mModel = 0;
        }
    }
    else if (uProperty == 0x5A554273)
    {
        mImpactEffectTerrain = PblHash(pszValue);
    }
    else if (uProperty == 0x7A140FC6)
    {
        mImpactDecalStatic = PblHash(pszValue);
    }
    else if (uProperty == 0x8FC79EBE)
    {
        mImpactEffectShieldSmall = PblHash(pszValue);
    }
    else if (uProperty == 0xA3091269)
    {
        mImpactEffectRigid = PblHash(pszValue);
    }
    else if (uProperty == 0xA58ACBD0)
    {
        int iValue;
        sscanf(pszValue, "%d", &iValue);
        mFadeColorLight = iValue != 0;
    }
    else if (uProperty == 0xD6220FE6)
    {
        mImpactEffectSoft = PblHash(pszValue);
    }
    else if (uProperty == 0xE369687E)
    {
        m_uiLightOdfNameHash = PblHash(pszValue);
    }
    else if (uProperty == 0xECAAA59D)
    {
        mImpactEffectWater = PblHash(pszValue);
    }
    else if (uProperty == 0xF73EC52A)
    {
        mImpactEffectShieldSmall = PblHash(pszValue);
    }
    else
    {
        OrdnanceClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x005fc1c0
OrdnanceClass* OrdnanceBulletClass::Derive(unsigned int uHash)
{
    return new OrdnanceBulletClass(this, uHash);
}

// FUNCTION: SWBF2 0x005fc230
Ordnance* OrdnanceBulletClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.mUsed >= Ordnance::sMemoryPool.mCount)
    {
        return 0;
    }

    OrdnanceBullet* pBullet = new OrdnanceBullet(this, desc);
    pBullet->ActivateThread(-40, PER_LOCAL_TURN);
    pBullet->AddOrdnanceToCollisionManager();
    return pBullet;
}

// GLOBAL: SWBF2 0x01ef261c
// SYNTHETIC: SWBF2 0x00405a50
// `dynamic initializer for 'OrdnanceBullet::rttiHashOrdnanceBullet''
PblHash OrdnanceBullet::rttiHashOrdnanceBullet("OrdnanceBullet");
char OrdnanceBullet::rttiNameOrdnanceBullet[] = "OrdnanceBullet";

// SYNTHETIC: SWBF2 0x005fc2c6
// [thunk]:OrdnanceBullet::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x005fc2d1
// [thunk]:OrdnanceBullet::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x005fa840
// OrdnanceBullet::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005f8ae0
bool OrdnanceBullet::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceBullet || Ordnance::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x005fa820
unsigned int OrdnanceBullet::GetDerivedRtti() const
{
    return rttiHashOrdnanceBullet;
}

// FUNCTION: SWBF2 0x005fa830
const char* OrdnanceBullet::GetDerivedRttiName() const
{
    return rttiNameOrdnanceBullet;
}

// FUNCTION: SWBF2 0x005fb900
void OrdnanceBullet::CreateRicochet(const PblVector3& vPos, const PblVector3& vNormal,
                                    GameObject* pObject) const
{
    OrdnanceDesc desc;
    desc.mStrength = 1.0f;
    desc.mPosition = vPos;
    desc.mOwner = pObject;
    desc.mDamageOwner.mGameObject.mObject = pObject;
    desc.mDamageOwner.mGameObject.mSavedHandleId = pObject ? GameObjectHandleId(pObject) : 0;
    desc.mDirection = vNormal;
    if (pObject)
    {
        Controllable* pControllable = GetControllable(pObject);
        desc.mDamageOwner.mCharacter = pControllable->mCharacter;
        desc.mDamageOwner.mTeam = GameObjectTeam(pObject);
        desc.mDamageOwner.mClass = GetEntityClass(pObject);
        desc.mDamageOwner.mGameObject.mObject = pObject;
        desc.mDamageOwner.mGameObject.mSavedHandleId = GameObjectHandleId(pObject);
        desc.mDamageOwner.mDamageMultiplier = pControllable->GetOffenseMultiplier();
    }
    else
    {
        desc.mDamageOwner.mCharacter = 0;
        desc.mDamageOwner.mTeam = 0;
        desc.mDamageOwner.mClass = 0;
        desc.mDamageOwner.mGameObject.mObject = 0;
        desc.mDamageOwner.mGameObject.mSavedHandleId = 0;
        desc.mDamageOwner.mDamageMultiplier = 1.0f;
    }
    desc.mDamageOwner.mWeapon = 0;
    desc.mTarget = 0;
    desc.mBodyID = -1;

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        NetGame::CreateEvent_CreateOrdnance(mClass, desc);
    }
    if (!NetGame::IsEnabled() || !NetGame::IsClient() || netIsLocalTurn)
    {
        mClass->Build(desc);
    }
}

// FUNCTION: SWBF2 0x005fbaa0
void OrdnanceBullet::Render(unsigned char, float, unsigned int uiFlags)
{
    if (!((OrdnanceBulletClass*)mClass)->mModel)
    {
        return;
    }

    PblVector3 vPos = mPosition;
    if (!netFrameLock && NetGame::IsEnabled() &&
        netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
    {
        netInterpolator.GetVec(mInterpIndex, vPos);
    }
    mOffscreenTime = 0.0f;
    const PblMatrix mat = DirectionMatrix(mVelocity, gVectorUnitY, vPos);
    ((OrdnanceBulletClass*)mClass)->mModel->Render(&mat, 0, mRenderColor, uiFlags, 0);
}

// FUNCTION: SWBF2 0x005fa890
OrdnanceBullet::OrdnanceBullet(OrdnanceBulletClass* pClass, const OrdnanceDesc& desc)
    : Ordnance(pClass, desc),
      RedSceneObject(&((OrdnanceBulletClass*)mClass)->mLodData)
{
    mOldPosition = desc.mPosition;
    OrdnanceBulletClass* pCharge = (OrdnanceBulletClass*)mClass;
    const float fSpeed =
        (((OrdnanceBulletClass*)pCharge->mNextCharge)->mVelocity - pCharge->mVelocity) * mRatio +
        pCharge->mVelocity;
    mVelocity.x = fSpeed * desc.mDirection.x;
    mVelocity.y = desc.mDirection.y * fSpeed;
    mVelocity.z = desc.mDirection.z * fSpeed;
    mFinalCollision = 0;
    mFinalCollisionOnNextUpdate = false;
    m_pLight = 0;
    mOffscreenTime = 0.0f;
    mCheckStaticCollision = true;

    SetRenderPos(mPosition);
    SetRenderRadius(5.0f);
    AddToScene();

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    if (!NetGame::IsEnabled() || NetGame::IsClient() || NetGame::IsOneWorld())
    {
        if (pClass->m_uiLightOdfNameHash)
        {
            m_pLight = EntityLightClass::Build(pClass->m_uiLightOdfNameHash);
            if (pClass->m_fRadiusLight > 0.0f)
            {
                m_pLight->SetOmniLightRadius(pClass->m_fRadiusLight);
            }
            if (pClass->m_colorLight.a > 0)
            {
                m_pLight->SetColor(pClass->m_colorLight);
            }
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = mPosition.x;
            matrix.m[3][1] = mPosition.y;
            matrix.m[3][2] = mPosition.z;
            matrix.m[3][3] = 1.0f;
            if (m_pLight->_fBeamLength > 0.0f)
            {
                const PblVector3& vDir = desc.mDirection;
                *(PblVector3*)matrix.m[1] = vDir;
                matrix.m[1][3] = 0.0f;
                PblVector3 vFront(vDir.x * vDir.y - gVectorUnitY.x, vDir.y * vDir.y - gVectorUnitY.y,
                                  vDir.z * vDir.y - gVectorUnitY.z);
                float fLength2 = vFront.x * vFront.x + vFront.y * vFront.y + vFront.z * vFront.z;
                if (fLength2 <= 0.0001f)
                {
                    vFront = PblVector3(gVectorUnitZ.x - vDir.x * vDir.z, gVectorUnitZ.y - vDir.y * vDir.z,
                                        gVectorUnitZ.z - vDir.z * vDir.z);
                    fLength2 = vFront.x * vFront.x + vFront.y * vFront.y + vFront.z * vFront.z;
                }
                const float fInv = 1.0f / (float)sqrt((double)fLength2);
                vFront.x *= fInv;
                vFront.y *= fInv;
                vFront.z *= fInv;
                matrix.m[0][0] = vDir.y * vFront.z - vDir.z * vFront.y;
                matrix.m[0][1] = vDir.z * vFront.x - vDir.x * vFront.z;
                matrix.m[0][2] = vDir.x * vFront.y - vDir.y * vFront.x;
                matrix.m[0][3] = 0.0f;
                *(PblVector3*)matrix.m[2] = vFront;
                matrix.m[2][3] = 0.0f;
            }
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
        else if (pClass->m_fRadiusLight > 0.0f && pClass->m_colorLight.a > 0)
        {
            m_pLight = new EntityLight();
            m_pLight->SetColor(pClass->m_colorLight);
            m_pLight->SetOmniLightRadius(pClass->m_fRadiusLight);
            m_pLight->SetHaloRadius(0.0f);
            m_pLight->SetFlareIntensity(0.0f);
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = mPosition.x;
            matrix.m[3][1] = mPosition.y;
            matrix.m[3][2] = mPosition.z;
            matrix.m[3][3] = 1.0f;
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
    }
}

// FUNCTION: SWBF2 0x005fae50
OrdnanceBullet::~OrdnanceBullet()
{
    RemoveFromScene();
    delete m_pLight;
}

// FUNCTION: SWBF2 0x005faee0
bool OrdnanceBullet::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    if (mFinalCollisionOnNextUpdate)
    {
        if (mFinalCollision &&
            !(mFinalCollisionHandle.mObject &&
              mFinalCollisionHandle.mObject->mHandleId != mFinalCollisionHandle.mSavedHandleId))
        {
            Collide(mFinalCollision, mPosition, mFinalCollisionNormal, 0);
        }
        return false;
    }

    if (((OrdnanceBulletClass*)mClass)->mStraightBullet && mCheckStaticCollision)
    {
        const float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                   mVelocity.z * mVelocity.z);
        const float fInv = 1.0f / fSpeed;
        const PblVector3 vDir(mVelocity.x * fInv, mVelocity.y * fInv, mVelocity.z * fInv);
        const GameObject* pOwner = ResolveGameObjectHandle(mOwner);
        CollisionObject* pHitObject;
        const float fHit = CollisionManager::RayHit(mPosition, vDir, mLifeTime * fSpeed, pHitObject,
                                                    &mFinalCollisionNormal, pOwner ? &pOwner : 0,
                                                    pOwner != 0, 0x90, true);
        if (pHitObject)
        {
            mLifeTime *= fHit;
            mFinalCollision = pHitObject;
            mFinalCollisionHandle = pHitObject->GetGameObject();
        }
        mCheckStaticCollision = false;
    }

    mOldPosition = mPosition;
    const float fStep = mLifeTime > fDeltaTime ? fDeltaTime : mLifeTime;
    mPosition.x = mPosition.x + mVelocity.x * fStep;
    mPosition.y = mVelocity.y * fStep + mPosition.y;
    mPosition.z = mVelocity.z * fStep + mPosition.z;
    mOffscreenTime += fDeltaTime;
    if (!Ordnance::Update(fDeltaTime))
    {
        mFinalCollisionOnNextUpdate = true;
    }
    SetRenderPos(mPosition);

    if (m_pLight)
    {
        const PblVector3 vMid((mPosition.x + mOldPosition.x) * 0.5f, (mPosition.y + mOldPosition.y) * 0.5f,
                              (mPosition.z + mOldPosition.z) * 0.5f);
        m_pLight->SetPosition(vMid);
        OrdnanceBulletClass* pClass = (OrdnanceBulletClass*)mClass;
        if (pClass->mFadeColorLight)
        {
            const float fAge = mLifeElapsed - pClass->mDamageTransitionDelay;
            if (fAge > 0.0f)
            {
                float fT;
                if (pClass->mDamageTransitionPeriod > 0.0f)
                {
                    fT = fAge / pClass->mDamageTransitionPeriod;
                    if (0.0f > fT)
                    {
                        fT = 0.0f;
                    }
                    else if (fT > 1.0f)
                    {
                        fT = 1.0f;
                    }
                }
                else
                {
                    fT = 1.0f;
                }
                m_pLight->SetColor(Interpolate(pClass->m_colorLight, pClass->m_endColorLight, fT));
            }
        }
    }

    static const PblBox sWorldBox(-5000.0f, -5000.0f, -5000.0f, 5000.0f, 5000.0f, 5000.0f);
    const PblVector3& vRenderPos = GetRenderPos();
    if (vRenderPos.x < sWorldBox._vMin.x || sWorldBox._vMax.x < vRenderPos.x ||
        vRenderPos.y < sWorldBox._vMin.y || sWorldBox._vMax.y < vRenderPos.y ||
        vRenderPos.z < sWorldBox._vMin.z || sWorldBox._vMax.z < vRenderPos.z)
    {
        return false;
    }

    if (!netFrameLock && NetGame::IsEnabled() && NetGame::IsClient() &&
        netInterpolator.GetIndex(mInterpIndex, 0, PblHandled::mHandleId))
    {
        netInterpolator.SetNext(mInterpIndex, &mPosition);
    }
    return true;
}

// FUNCTION: SWBF2 0x005fb330
bool OrdnanceBullet::Collide(CollisionObject* pObject, const PblVector3& vPos, const PblVector3& vNormal,
                             int iBodyId)
{
    unsigned int uDamageFlags = 0xFFFFFFFF;
    GameObject* pVictim = pObject->GetGameObject();
    if (m_pLight)
    {
        m_pLight->SetPosition(PblVector3(vPos.x + vNormal.x * 0.5f, vPos.y + vNormal.y * 0.5f,
                                         vPos.z + vNormal.z * 0.5f));
    }

    int iBody = iBodyId;
    if (pObject->GetCollisionType() == COLL_SHIELD)
    {
        if (pObject->mParent)
        {
            pVictim = pObject->mParent->GetGameObject();
            iBody = -1;
            uDamageFlags = 1;
        }
    }
    else if (pObject->mParent)
    {
        uDamageFlags = pObject->mParent->GetCollisionRadius() > pObject->GetCollisionRadius() * 1.5f
                           ? 2
                           : 0xFFFFFFFF;
    }

    if (pVictim)
    {
        PblVector3 vDir;
        D3DXVec3Normalize(&vDir, &mVelocity);
        if (pVictim->Deflect(this, vPos, vDir))
        {
            return true;
        }
        static_cast<Damageable*>(pVictim)->ApplyDamage(GetDamage(), vDir, vPos, iBody, uDamageFlags);
    }

    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        if (!NetGame::IsOneWorld() || !NetGame::IsHost() ||
            !CameraManager::sInstance->IsInsideAnyCamera(vPos, 2.0f, 70.0f))
        {
            return Ordnance::Collide(pObject, vPos, vNormal, iBody);
        }
    }

    OrdnanceBulletClass* pClass = (OrdnanceBulletClass*)mClass;
    bool bShield = false;
    unsigned int uEffect = 0;
    if (pVictim && pVictim->mIsShielded && pVictim->mCurShield > 0.0f)
    {
        bShield = true;
    }
    else
    {
        switch (pObject->GetCollisionType())
        {
        case COLL_SOFT:
        case COLL_ATTACHED:
            uEffect = pClass->mImpactEffectSoft;
            break;
        case COLL_ASTEROID:
        case COLL_RIGID:
            uEffect = pClass->mImpactEffectRigid;
            break;
        case COLL_STATIC:
            uEffect = pClass->mImpactEffectStatic;
            break;
        case COLL_TERRAIN:
            uEffect = pClass->mImpactEffectTerrain;
            break;
        case COLL_WATER:
            RedWater::AddWaterRing(vPos, 1.5f, 0.01f);
            WaterEffect::AddWaterEffect(pClass->mImpactEffectWater, vPos);
            break;
        case COLL_LEAFPATCH:
            if (mOffscreenTime < 0.5f)
            {
                const PblVector3 vFlat(vNormal.x, 0.0f, vNormal.z);
                PblVector3 vWind;
                D3DXVec3Normalize(&vWind, &vFlat);
                vWind = PblVector3(vWind.x * 2.0f, vWind.y * 2.0f, vWind.z * 2.0f);
                for (int i = 3; i; --i)
                {
                    FallingLeafParticle* pLeaf = LeafParticleManager::GetFreeLeaf();
                    if (pLeaf)
                    {
                        pLeaf->RespawnLeaf(vPos, 0, LEAF_NONE);
                        pLeaf->AddWindVelocity(vWind);
                    }
                }
                if (PblRandom::GetGlobalFloat() < 0.08f)
                {
                    BirdParticleManager::ScatterRandomBirds(2, vPos);
                }
            }
            break;
        case COLL_SHIELD:
            bShield = true;
            break;
        default:
            break;
        }
    }
    if (bShield)
    {
        uEffect = pObject->GetCollisionRadius() >= 16.0f ? pClass->mImpactEffectShieldLarge
                                                         : pClass->mImpactEffectShieldSmall;
    }

    if (uEffect)
    {
        FLEffectClass* pEffectClass = FLEffect::s_EffectClasses.Find(uEffect);
        if (pEffectClass)
        {
            FLEffectObject* pEffect = pEffectClass->BuildEffect();
            if (pEffect)
            {
                if (pVictim)
                {
                    PblMatrix mWorld = _IdentityMatrix;
                    if (bShield)
                    {
                        mWorld = DirectionMatrix(vNormal, gVectorUnitY, vPos);
                    }
                    else
                    {
                        *(PblVector3*)mWorld.m[3] = vPos;
                        mWorld.m[3][3] = 1.0f;
                    }
                    const PblMatrix mInverse = pVictim->GetMatrix().GetInverseFast();
                    PblMatrix mOffset;
                    D3DXMatrixMultiply(&mOffset, &mWorld, &mInverse);
                    pEffect->AttachEffectToObject(pVictim, mOffset, true);
                }
                else if (bShield)
                {
                    pEffect->AttachEffectToVector(&vPos, vNormal, false);
                }
                else
                {
                    pEffect->AttachEffectToVector(&vPos, false);
                }
            }
        }
    }
    return Ordnance::Collide(pObject, vPos, vNormal, iBody);
}

// SYNTHETIC: SWBF2 0x005fbc70
// OrdnanceBulletClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005fbdd0
// OrdnanceBulletClass::~OrdnanceBulletClass
