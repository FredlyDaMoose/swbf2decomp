#include "OrdnanceMeleeThrow.h"
#include "EntityLight.h"
#include "WeaponMeleeInstance.h"
#include "GameObject.h"
#include "BirdEffect.h"
#include "LeafEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "CollisionManager.h"
#include "Damageable.h"
#include "FLEffect.h"
#include "NetGame.h"
#include "WeaponMelee.h"
#include <math.h>
#include <d3dx9math.h>
#include "WeaponMeleeThrow.h"
#include <stdio.h>
#include <string.h>

// FUNCTION: SWBF2 0x00602b40
OrdnanceMeleeThrow::~OrdnanceMeleeThrow()
{
    static_cast<RedSceneObject*>(this)->Deactivate();
    delete m_pLight;
    if (m_pDamageData)
    {
        WeaponMelee::FreeDamageData((WeaponMelee::DamageData*)m_pDamageData);
        m_pDamageData = 0;
    }
}

// SYNTHETIC: SWBF2 0x006048b6
// [thunk]:OrdnanceMeleeThrow::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x006048be
// [thunk]:OrdnanceMeleeThrow::`scalar deleting destructor' adjustor{152}
// SYNTHETIC: SWBF2 0x006021f0
// OrdnanceMeleeThrow::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604820
Ordnance* OrdnanceMeleeThrowClass::Build(const OrdnanceDesc& desc)
{
    if (Ordnance::sMemoryPool.IsFull())
    {
        return 0;
    }
    OrdnanceMeleeThrow* pOrdnance = new OrdnanceMeleeThrow(this, desc);
    pOrdnance->ActivateThread(-40, PER_LOCAL_TURN);
    pOrdnance->AddOrdnanceToCollisionManager();
    return pOrdnance;
}

// FUNCTION: SWBF2 0x006031f0
bool OrdnanceMeleeThrow::Collide(CollisionObject* pObject, const PblVector3& vPos,
                                const PblVector3& vNormal, int iBodyId)
{
    if (mLifeTime <= 0.0f)
    {
        return false;
    }

    GameObject* pVictim = pObject->GetGameObject();
    if (m_pLight)
    {
        PblVector3 vMid(vPos.x + vNormal.x * 0.5f,
                        vPos.y + vNormal.y * 0.5f,
                        vPos.z + vNormal.z * 0.5f);
        static_cast<FLEffectObject*>(m_pLight)->SetPosition(vMid);
    }

    int iReturn = -1;
    if (pObject->GetCollisionType() == COLL_SHIELD)
    {
        if (pObject->mParent)
        {
            pVictim = pObject->mParent->GetGameObject();
            iReturn = 1;
            iBodyId = -1;
        }
    }
    else if (pObject->mParent)
    {
        if (pObject->mParent->GetCollisionRadius() > pObject->GetCollisionRadius() * 1.5f)
        {
            iReturn = 2;
        }
    }

    if (pVictim)
    {
        PblVector3 vDirection;
        D3DXVec3Normalize((D3DXVECTOR3*)&vDirection, (const D3DXVECTOR3*)&mVelocity);
        if (pVictim->Deflect(this, vPos, vDirection))
        {
            Return();
            return false;
        }
    }

    const CollisionObjectType eType = pObject->GetCollisionType();
    if (eType == COLL_SOFT || eType == COLL_ATTACHED)
    {
        if (iReturn == 1)
        {
            Return();
        }
    }
    else if (eType == COLL_LEAFPATCH && mOffscreenTime < 0.5f)
    {
        PblVector3 vHorizontalNormal(vNormal.x, 0.0f, vNormal.z);
        PblVector3 vUnit, vWind;
        D3DXVec3Normalize((D3DXVECTOR3*)&vUnit, (const D3DXVECTOR3*)&vHorizontalNormal);
        vWind = PblVector3(vUnit.x * 2.0f, vUnit.y * 2.0f, vUnit.z * 2.0f);
        for (int i = 0; i < 3; ++i)
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

    Ordnance::Collide(pObject, vPos, vNormal, iBodyId);
    return false;
}

class RedPose;

namespace
{

    void RenderWeapon(WeaponMelee* pWeapon, const PblMatrix& mat, const RedPose* pPose,
                      const RedColor& color, unsigned int uiFlags, bool bArg)
    {
        typedef void(__thiscall * RenderFn)(WeaponMelee*, const PblMatrix&, const RedPose*,
                                            const RedColor&, unsigned int, bool);
        pWeapon->Render(mat, pPose, color, uiFlags, bArg);
    }
}

// GLOBAL: SWBF2 0x01ef2654
// SYNTHETIC: SWBF2 0x00405b20
// `dynamic initializer for 'OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow''
PblHash OrdnanceMeleeThrow::rttiHashOrdnanceMeleeThrow("OrdnanceMeleeThrow");
char OrdnanceMeleeThrow::rttiNameOrdnanceMeleeThrow[] = "OrdnanceMeleeThrow";

// GLOBAL: SWBF2 0x01ef2650
// SYNTHETIC: SWBF2 0x00405b30
// `dynamic initializer for 'OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass''
PblHash OrdnanceMeleeThrowClass::rttiHashOrdnanceMeleeThrowClass("OrdnanceMeleeThrowClass");
char OrdnanceMeleeThrowClass::rttiNameOrdnanceMeleeThrowClass[] = "OrdnanceMeleeThrowClass";

// FUNCTION: SWBF2 0x00602180
bool OrdnanceMeleeThrow::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashOrdnanceMeleeThrow || Ordnance::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x006021b0
unsigned int OrdnanceMeleeThrow::GetDerivedRtti() const
{
    return rttiHashOrdnanceMeleeThrow;
}

// FUNCTION: SWBF2 0x006021c0
const char* OrdnanceMeleeThrow::GetDerivedRttiName() const
{
    return rttiNameOrdnanceMeleeThrow;
}

// FUNCTION: SWBF2 0x006021d0
const PblVector3& OrdnanceMeleeThrow::GetOldPosition() const
{
    return mOldPosition;
}

// FUNCTION: SWBF2 0x006021e0
const PblVector3& OrdnanceMeleeThrow::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00602220
bool OrdnanceMeleeThrowClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashOrdnanceMeleeThrowClass || OrdnanceClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00602250
unsigned int OrdnanceMeleeThrowClass::GetDerivedRtti()
{
    return rttiHashOrdnanceMeleeThrowClass;
}

// FUNCTION: SWBF2 0x00602260
const char* OrdnanceMeleeThrowClass::GetDerivedRttiName()
{
    return rttiNameOrdnanceMeleeThrowClass;
}

// FUNCTION: SWBF2 0x00602270
float OrdnanceMeleeThrowClass::GetVelocity()
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00602280
float OrdnanceMeleeThrowClass::GetRange()
{
    return mVelocity * mLifeSpan;
}

// FUNCTION: SWBF2 0x006034e0
void OrdnanceMeleeThrow::Render(unsigned char, float, unsigned int uiFlags)
{
    if (m_pOwner.mObject && m_pOwner.mObject->mHandleId != m_pOwner.mSavedHandleId)
    {
        return;
    }
    if (!m_pWeapon || !m_pWeapon->m_pPrimaryWeapon)
    {
        return;
    }

    mOffscreenTime = 0.0f;
    PblMatrix mat = m_mMatrix;
    RenderWeapon(m_pWeapon->m_pPrimaryWeapon, mat, 0, RedColor::WHITE, uiFlags | 1, true);
}

// FUNCTION: SWBF2 0x00603550
void OrdnanceMeleeThrow::Return()
{
    if (mLifeTime > 0.0f)
    {
        m_fReturnTime -= mLifeTime;
        mLifeTime = 0.0f;
    }
}

extern bool netIsLateTurn;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00602290
OrdnanceMeleeThrow::OrdnanceMeleeThrow(OrdnanceMeleeThrowClass* pClass, const OrdnanceDesc& desc)
    : Ordnance(pClass, desc),
      RedSceneObject(&((OrdnanceMeleeThrowClass*)mClass)->mLodData),
      m_pOwner(desc.mOwner)
{
    mOldPosition = desc.mPosition;
    OrdnanceMeleeThrowClass* pCharge = (OrdnanceMeleeThrowClass*)mClass;
    const float fSpeed =
        (((OrdnanceMeleeThrowClass*)pCharge->mNextCharge)->mVelocity - pCharge->mVelocity) * mRatio +
        pCharge->mVelocity;
    mVelocity.x = fSpeed * desc.mDirection.x;
    mVelocity.y = desc.mDirection.y * fSpeed;
    mVelocity.z = desc.mDirection.z * fSpeed;
    m_pDamageData = 0;
    m_pLight = 0;
    mOffscreenTime = 0.0f;
    m_fReturnTime = 0.0f;

    m_pWeapon = (WeaponMeleeThrow*)desc.mFireWeapon;
    m_pWeapon->m_pPrimaryWeapon->TurnOn(0.01f);
    m_pWeapon->m_pPrimaryWeapon->m_uiTrailOnMask = 0xf;
    m_pWeapon->NotifyThrow();
    m_bLastFrameIndicator = false;

    D3DXMatrixMultiply((D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&m_pWeapon->m_mThrow0Local,
                       (const D3DXMATRIX*)&m_pOwner->GetMatrix());

    const WeaponMeleeClass* pWeaponClass = (const WeaponMeleeClass*)m_pWeapon->m_pPrimaryWeapon->mClass;
    PblVector3 vEdgeDir = gVectorUnitZ;
    if (pWeaponClass->m_uiNumDamageEdges)
    {
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vEdgeDir, (const D3DXVECTOR3*)&pWeaponClass->m_aDamageEdge[0].vOffsetDir,
                                (const D3DXMATRIX*)&m_mMatrix);
    }
    const PblVector3& vDir = desc.mDirection;
    PblVector3 vAxis(vEdgeDir.y * vDir.z - vEdgeDir.z * vDir.y, vEdgeDir.z * vDir.x - vEdgeDir.x * vDir.z,
                     vEdgeDir.x * vDir.y - vEdgeDir.y * vDir.x);
    D3DXVec3Normalize((D3DXVECTOR3*)&vEdgeDir, (const D3DXVECTOR3*)&vAxis);
    m_vRotationAxis = vEdgeDir;

    mCheckStaticCollision = true;
    mLifeTime = mClass->mLifeSpan;
    m_fReturnTime = mLifeTime;

    float fRadius2 = 1.0f;
    for (unsigned int i = 0; i < pWeaponClass->m_uiNumDamageEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[i];
        if (edge.uiAttachToWeapon)
        {
            continue;
        }
        const float fLength =
            pClass->m_bDamageLengthFromEdge ? edge.fLength * pClass->m_fDamageLength : pClass->m_fDamageLength;
        const float fWidth =
            pClass->m_bDamageWidthFromEdge ? edge.fWidth * pClass->m_fDamageWidth : pClass->m_fDamageWidth;
        const PblVector3 vTip(edge.vOffsetPos.x + edge.vOffsetDir.x * fLength,
                              edge.vOffsetPos.y + edge.vOffsetDir.y * fLength,
                              edge.vOffsetPos.z + edge.vOffsetDir.z * fLength);
        const float fTip2 = vTip.y * vTip.y + vTip.x * vTip.x + vTip.z * vTip.z + fWidth * fWidth;
        if (fTip2 > fRadius2)
        {
            fRadius2 = fTip2;
        }
        const float fBase2 = edge.vOffsetPos.x * edge.vOffsetPos.x + edge.vOffsetPos.y * edge.vOffsetPos.y +
                             edge.vOffsetPos.z * edge.vOffsetPos.z;
        if (fBase2 > fRadius2)
        {
            fRadius2 = fBase2;
        }
    }

    SetRenderPos(mPosition);
    SetRenderRadius((float)sqrt((double)fRadius2));
    AddToScene();

    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->SetOrientation(mVelocity);
    }

    mDecrementLifeTime = false;
    if (!NetGame::IsEnabled() || NetGame::IsClient())
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

// FUNCTION: SWBF2 0x00602bf0
bool OrdnanceMeleeThrow::Update(float fDeltaTime)
{
    if (netIsLateTurn)
    {
        return true;
    }

    OrdnanceMeleeThrowClass* pClass = (OrdnanceMeleeThrowClass*)mClass;
    if ((m_pOwner.mObject && m_pOwner.mObject->mHandleId != m_pOwner.mSavedHandleId) || m_bLastFrameIndicator)
    {
        return false;
    }

    if (mLifeTime > 0.0f)
    {
        if (mCheckStaticCollision)
        {
            const float fSpeed = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                       mVelocity.z * mVelocity.z);
            const float fInvSpeed = 1.0f / fSpeed;
            const PblVector3 vDir(mVelocity.x * fInvSpeed, mVelocity.y * fInvSpeed, mVelocity.z * fInvSpeed);
            CollisionObject* pHit;
            PblVector3 vNormal;
            const float fHit =
                CollisionManager::RayHit(mPosition, vDir, mLifeTime * fSpeed, pHit, &vNormal, 0, 0, 0x90, true);
            if (fHit < 1.0f)
            {
                m_fReturnTime *= fHit;
                mLifeTime *= fHit;
            }
            mCheckStaticCollision = false;
        }

        float fStep = fDeltaTime;
        mLifeTime -= fDeltaTime;
        if (mLifeTime <= 0.0f)
        {
            if (mLifeTime + fDeltaTime > 0.0f && m_pDamageData)
            {
                ((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects = 0;
            }
            fStep = mLifeTime;
        }
        mOffscreenTime += fStep;
        ApplyDamage(fStep);

        mOldPosition = mPosition;
        mPosition.x += mVelocity.x * fStep;
        mPosition.y += mVelocity.y * fStep;
        mPosition.z += mVelocity.z * fStep;
        PblMatrix mRotate = _IdentityMatrix;
        D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis, pClass->m_fOmega * fStep);
        D3DXMatrixMultiply((D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&mRotate);
        m_mMatrix.m[3][0] = mPosition.x;
        m_mMatrix.m[3][1] = mPosition.y;
        m_mMatrix.m[3][2] = mPosition.z;
        m_mMatrix.m[3][3] = 1.0f;

        if (!Ordnance::Update(fStep) && mLifeTime > 0.0f)
        {
            Return();
        }

        m_pWeapon->m_pPrimaryWeapon->UpdateEffects(fStep, mPosition, mVelocity,
                                                   pClass->m_OnSound.GetProperties() ? &pClass->m_OnSound : 0);
        SetRenderPos(mPosition);
        if (m_pLight)
        {
            const PblVector3 vMid((mPosition.x + mOldPosition.x) * 0.5f, (mPosition.y + mOldPosition.y) * 0.5f,
                                  (mPosition.z + mOldPosition.z) * 0.5f);
            static_cast<FLEffectObject*>(m_pLight)->SetPosition(vMid);
        }
        if (mLifeTime <= 0.0f)
        {
            Return();
        }
        return true;
    }

    mLifeTime -= fDeltaTime;
    const float fReturnDuration = m_fReturnTime / pClass->mLifeSpan * pClass->m_fReturnDuration;
    if (-mLifeTime >= fReturnDuration)
    {
        m_pWeapon->NotifyReturned();
        m_pWeapon->m_pPrimaryWeapon->m_uiTrailOnMask = 0;
        m_bLastFrameIndicator = true;
        return true;
    }

    const float fPrev = (-mLifeTime - fDeltaTime) / fReturnDuration;
    const float fT = (-(mLifeTime / fReturnDuration) - fPrev) / (1.0f - fPrev);
    PblVector3 vTarget;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vTarget, (const D3DXVECTOR3*)&m_pWeapon->m_mThrow0Local.GetTranslation(),
                           (const D3DXMATRIX*)&mOwner->GetMatrix());
    mOldPosition = mPosition;
    mPosition.x = (vTarget.x - mOldPosition.x) * fT + mOldPosition.x;
    mPosition.y = (vTarget.y - mOldPosition.y) * fT + mOldPosition.y;
    mPosition.z = (vTarget.z - mOldPosition.z) * fT + mOldPosition.z;
    PblMatrix mRotate(_IdentityMatrix);
    D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                           pClass->m_fOmega * fDeltaTime * 0.5f);
    D3DXMatrixMultiply((D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&mRotate);
    m_mMatrix.m[3][0] = mPosition.x;
    m_mMatrix.m[3][1] = mPosition.y;
    m_mMatrix.m[3][2] = mPosition.z;
    m_mMatrix.m[3][3] = 1.0f;
    m_pWeapon->m_pPrimaryWeapon->UpdateEffects(fDeltaTime, mPosition, mVelocity, 0);
    SetRenderPos(mPosition);
    ApplyDamage(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00603580
void OrdnanceMeleeThrow::ApplyDamage(float fDeltaTime)
{
    OrdnanceMeleeThrowClass* pClass = (OrdnanceMeleeThrowClass*)mClass;
    const WeaponMeleeClass* pWeaponClass = (const WeaponMeleeClass*)m_pWeapon->m_pPrimaryWeapon->mClass;
    const unsigned int uiNumEdges = pWeaponClass->m_uiNumDamageEdges;
    if (!uiNumEdges)
    {
        return;
    }

    float fMaxLength = 0.0f;
    if (!m_pDamageData)
    {
        m_pDamageData = WeaponMelee::AllocDamageData();
        if (!m_pDamageData)
        {
            return;
        }
        ((WeaponMelee::DamageData*)m_pDamageData)->pAttack = 0;
        ((WeaponMelee::DamageData*)m_pDamageData)->pNext = 0;
        ((WeaponMelee::DamageData*)m_pDamageData)->fTime = 0.0f;
        ((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects = 0;
    }

    float fStepTime = 0.5f / (pClass->mVelocity + pClass->m_fOmega);
    if (fStepTime > 0.0333f)
    {
        fStepTime = 0.0333f;
    }
    const int iNumSteps = (int)(fDeltaTime / fStepTime) + 1;
    const float fStep = fDeltaTime / (float)iNumSteps;

    PblMatrix mRotate = _IdentityMatrix;
    D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis, fStep * pClass->m_fOmega);
    PblMatrix mFirst;
    D3DXMatrixMultiply((D3DXMATRIX*)&mFirst, (const D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&mRotate);
    mFirst.m[3][0] = m_mMatrix.m[3][0] + mVelocity.x * fStep;
    mFirst.m[3][1] = m_mMatrix.m[3][1] + mVelocity.y * fStep;
    mFirst.m[3][2] = m_mMatrix.m[3][2] + mVelocity.z * fStep;
    mFirst.m[3][3] = 1.0f;
    PblMatrix mLast;
    if (fDeltaTime > fStep)
    {
        D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                               fDeltaTime * pClass->m_fOmega);
        D3DXMatrixMultiply((D3DXMATRIX*)&mLast, (const D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&mRotate);
        mLast.m[3][0] = mVelocity.x * fDeltaTime + m_mMatrix.m[3][0];
        mLast.m[3][1] = m_mMatrix.m[3][1] + mVelocity.y * fDeltaTime;
        mLast.m[3][2] = m_mMatrix.m[3][2] + mVelocity.z * fDeltaTime;
        mLast.m[3][3] = 1.0f;
    }
    else
    {
        mLast = mFirst;
    }

    PblBox box(mLast.GetTranslation());
    float fMaxWidth = 0.0f;
    for (unsigned int i = 0; i < uiNumEdges; ++i)
    {
        const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[i];
        if (edge.uiAttachToWeapon)
        {
            continue;
        }
        if (edge.fLength > fMaxLength)
        {
            fMaxLength = edge.fLength;
        }
        if (edge.fWidth > fMaxWidth)
        {
            fMaxWidth = edge.fWidth;
        }
        PblVector3 vPos;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vPos, (const D3DXVECTOR3*)&edge.vOffsetPos, (const D3DXMATRIX*)&mFirst);
        box.AddPoint(vPos);
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vPos, (const D3DXVECTOR3*)&edge.vOffsetPos, (const D3DXMATRIX*)&mLast);
        box.AddPoint(vPos);
    }
    const float fLength = pClass->m_bDamageLengthFromEdge ? fMaxLength * pClass->m_fDamageLength
                                                          : pClass->m_fDamageLength;
    const float fWidth = pClass->m_bDamageWidthFromEdge ? pClass->m_fDamageWidth * fMaxWidth
                                                        : pClass->m_fDamageWidth;
    const PblVector3 vCenter((box._vMax.x + box._vMin.x) * 0.5f, (box._vMax.y + box._vMin.y) * 0.5f,
                             (box._vMax.z + box._vMin.z) * 0.5f);
    const float dx = box._vMax.x - box._vMin.x;
    const float dy = box._vMax.y - box._vMin.y;
    const float dz = box._vMax.z - box._vMin.z;
    const float fRadius = sqrtf(dy * dy + dx * dx + dz * dz) * 0.5f + (fWidth + fLength) * 1.1f;

    GameObject* apObjects[8];
    int iNumObjects =
        CollisionManager::GetObjectsInRange(vCenter, fRadius, apObjects, 8, mOwner->GetGameObject(), 0x3a);
    for (int i = 0; i < iNumObjects;)
    {
        GameObject* pObject = apObjects[i];
        if (pObject->GetCollisionObject())
        {
            const WeaponMelee::DamageData* pData = (const WeaponMelee::DamageData*)m_pDamageData;
            int k;
            for (k = 0; k < pData->iNumObjects; ++k)
            {
                if (pData->apObject[k] == pObject)
                {
                    break;
                }
            }
            if (k >= pData->iNumObjects)
            {
                ++i;
                continue;
            }
        }
        apObjects[i] = apObjects[--iNumObjects];
    }

    PblVector3 vDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&mVelocity);
    GameObject* pFirstHit = 0;
    FLEffectClass* pHitEffectClass = 0;
    for (int iStep = 0; iStep < iNumSteps && iNumObjects; ++iStep)
    {
        const float fTime = (float)iStep * fStep;
        if (fTime > m_fReturnTime)
        {
            break;
        }
        PblMatrix mStep;
        if (iStep < iNumSteps - 1)
        {
            D3DXMatrixRotationAxis((D3DXMATRIX*)&mRotate, (const D3DXVECTOR3*)&m_vRotationAxis,
                                   fTime * pClass->m_fOmega);
            D3DXMatrixMultiply((D3DXMATRIX*)&mStep, (const D3DXMATRIX*)&m_mMatrix, (const D3DXMATRIX*)&mRotate);
            mStep.m[3][0] = mVelocity.x * fTime + m_mMatrix.m[3][0];
            mStep.m[3][1] = m_mMatrix.m[3][1] + mVelocity.y * fTime;
            mStep.m[3][2] = m_mMatrix.m[3][2] + mVelocity.z * fTime;
            mStep.m[3][3] = 1.0f;
        }
        else
        {
            mStep = mLast;
        }

        for (unsigned int e = 0; e < uiNumEdges && iNumObjects; ++e)
        {
            const WeaponMeleeClass::DamageEdge& edge = pWeaponClass->m_aDamageEdge[e];
            if (edge.uiAttachToWeapon)
            {
                continue;
            }
            const float fEdgeLength =
                pClass->m_bDamageLengthFromEdge ? edge.fLength * pClass->m_fDamageLength : pClass->m_fDamageLength;
            const float fEdgeWidth =
                pClass->m_bDamageWidthFromEdge ? edge.fWidth * pClass->m_fDamageWidth : pClass->m_fDamageWidth;
            PblVector3 vEdgeDir;
            D3DXVec3TransformNormal((D3DXVECTOR3*)&vEdgeDir, (const D3DXVECTOR3*)&edge.vOffsetDir,
                                    (const D3DXMATRIX*)&mStep);
            const float fBackX = vEdgeDir.x * 0.2f;
            const float fBackY = vEdgeDir.y * 0.2f;
            const float fBackZ = vEdgeDir.z * 0.2f;
            PblVector3 vEdgePos;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vEdgePos, (const D3DXVECTOR3*)&edge.vOffsetPos,
                                   (const D3DXMATRIX*)&mStep);
            const PblVector3 vStart(vEdgePos.x - fBackX, vEdgePos.y - fBackY, vEdgePos.z - fBackZ);

            for (int i = 0; i < iNumObjects;)
            {
                GameObject* pObject = apObjects[i];
                const float t =
                    pObject->GetCollisionObject()->RayHit(vStart, vEdgeDir, fEdgeLength, 0, 0, true, fEdgeWidth);
                if (t >= 1.0f)
                {
                    ++i;
                    continue;
                }
                apObjects[i] = apObjects[--iNumObjects];

                WeaponMelee::DamageData* pData = (WeaponMelee::DamageData*)m_pDamageData;
                if (pData->iNumObjects >= WeaponMelee::DamageData::MAX_OBJECTS_PER_ATTACK)
                {
                    continue;
                }
                pData->apObject[pData->iNumObjects] = pObject;
                ++((WeaponMelee::DamageData*)m_pDamageData)->iNumObjects;
                const float fAlong = t * fEdgeLength;
                const PblVector3 vHit(vEdgeDir.x * fAlong + vStart.x, vEdgeDir.y * fAlong + vStart.y,
                                      vEdgeDir.z * fAlong + vStart.z);

                if (pObject->Deflect(0, vDir, vHit))
                {
                    continue;
                }
                static_cast<Damageable*>(pObject)->ApplyDamage(GetDamage(), vDir, vHit, -1, 0xffffffff);

                if (pWeaponClass->m_uiNumHitEffects > 0 && pWeaponClass->m_uiHitEffects[0] && !pHitEffectClass)
                {
                    pHitEffectClass = FLEffect::s_EffectClasses.Find(pWeaponClass->m_uiHitEffects[0]);
                }
                if (pHitEffectClass)
                {
                    FLEffectObject* pEffect = pHitEffectClass->BuildEffect();
                    if (pEffect)
                    {
                        pEffect->AttachEffectToVector(&vHit, false);
                    }
                }
                if (!pFirstHit)
                {
                    pFirstHit = pObject;
                }
            }
        }
    }

    if (pFirstHit)
    {
        pWeaponClass->m_HitSound.Play(&pFirstHit->GetMatrix().GetTranslation(), 0, false, true);
    }
}

static const RedColor kColorZero = { 0, 0, 0, 0 };

// FUNCTION: SWBF2 0x006042d0
OrdnanceMeleeThrowClass::OrdnanceMeleeThrowClass(unsigned int uHash)
    : OrdnanceClass(uHash)
{
    mLifeSpan = 3.0f;

    mLodData = OrdnanceLodData();
    mVelocity = 20.0f;
    m_fReturnDuration = 1.0f;
    m_fOmega = 6.2831855f * 2.0f;
    m_fDamageLength = 1.0f;
    m_fDamageWidth = 1.0f;
    m_bDamageLengthFromEdge = 1;
    m_bDamageWidthFromEdge = 1;
    m_uiLightOdfNameHash = 0;
    m_colorLight = kColorZero;
    m_fRadiusLight = 0.0f;
}

// FUNCTION: SWBF2 0x006043e0
OrdnanceMeleeThrowClass::OrdnanceMeleeThrowClass(OrdnanceMeleeThrowClass* pBase, unsigned int uHash)
    : OrdnanceClass(pBase, uHash)
{
    mLodData = OrdnanceLodData();
    mVelocity = pBase->mVelocity;
    m_fReturnDuration = pBase->m_fReturnDuration;
    m_fOmega = pBase->m_fOmega;
    m_fDamageLength = pBase->m_fDamageLength;
    m_fDamageWidth = pBase->m_fDamageWidth;
    m_bDamageLengthFromEdge = pBase->m_bDamageLengthFromEdge;
    m_bDamageWidthFromEdge = pBase->m_bDamageWidthFromEdge;
    m_uiLightOdfNameHash = pBase->m_uiLightOdfNameHash;
    m_colorLight = pBase->m_colorLight;
    m_fRadiusLight = pBase->m_fRadiusLight;
    m_OnSound = pBase->m_OnSound;
}

// FUNCTION: SWBF2 0x00604530
void OrdnanceMeleeThrowClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x32741C32)
    {
        sscanf(pszValue, "%f", &mVelocity);
    }
    else if (uProperty == 0x2776564D)
    {
        sscanf(pszValue, "%f", &m_fRadiusLight);
    }
    else if (uProperty == 0x0DDA9B5B)
    {
        sscanf(pszValue, "%f", &m_fReturnDuration);
    }
    else if (uProperty == 0x746CE73E)
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
    else if (uProperty == 0xDE71B19F)
    {
        m_OnSound.SetID(pszValue, false, false);
    }
    else if (uProperty == 0xC3B311DC)
    {
        char acEdge[256];
        m_bDamageLengthFromEdge = 0;
        if (sscanf(pszValue, "%f %s", &m_fDamageLength, acEdge) == 2 && !_stricmp(acEdge, "fromedge"))
        {
            m_bDamageLengthFromEdge = 1;
        }
    }
    else if (uProperty == 0xE369687E)
    {
        m_uiLightOdfNameHash = PblHash(pszValue);
    }
    else if (uProperty == 0xE6989765)
    {
        float fRate;
        sscanf(pszValue, "%f", &fRate);
        m_fOmega = 6.2831855f * fRate;
    }
    else if (uProperty == 0xE4B667AC)
    {
        char acEdge[256];
        m_bDamageWidthFromEdge = 0;
        if (sscanf(pszValue, "%f %s", &m_fDamageWidth, acEdge) == 2 && !_stricmp(acEdge, "fromedge"))
        {
            m_bDamageWidthFromEdge = 1;
        }
    }
    else
    {
        OrdnanceClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x006047b0
OrdnanceClass* OrdnanceMeleeThrowClass::Derive(unsigned int uHash)
{
    return new OrdnanceMeleeThrowClass(this, uHash);
}

// SYNTHETIC: SWBF2 0x006043b0
// OrdnanceMeleeThrowClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00604520
OrdnanceMeleeThrowClass::~OrdnanceMeleeThrowClass()
{
}
