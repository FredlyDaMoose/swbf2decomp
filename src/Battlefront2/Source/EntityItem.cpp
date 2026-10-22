#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "EntityItem.h"
#include "Character.h"
#include "CollisionBody.h"
#include "CollisionModel.h"
#include "CollisionPrimitive.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "EntityLight.h"
#include "NetGame.h"
#include "NetPacket.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Portals/Portal.h"

// FUNCTION: SWBF2 0x004cbb10
bool EntityItem::NoDisplayLifeTimer() const
{
    return mClass->mNoLifeSpanDisplay;
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);

extern PblVector3 gVectorZero;

extern bool netFrameLock;
extern bool netIsLocalTurn;
extern bool netIsLateTurn;

PblMatrix NormalMatrix(const PblVector3& vDir, const PblVector3& vNormal, const PblVector3& vPos);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01ebbe2c
// SYNTHETIC: SWBF2 0x00402bd0
// `dynamic initializer for 'EntityItemClass::rttiHashEntityItemClass''
PblHash EntityItemClass::rttiHashEntityItemClass("EntityItemClass");
char EntityItemClass::rttiNameEntityItemClass[] = "EntityItemClass";

// FUNCTION: SWBF2 0x004cabd0
bool EntityItemClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityItemClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004cac10
unsigned int EntityItemClass::GetDerivedRtti()
{
    return rttiHashEntityItemClass;
}

// FUNCTION: SWBF2 0x004cac20
const char* EntityItemClass::GetDerivedRttiName()
{
    return rttiNameEntityItemClass;
}

// FUNCTION: SWBF2 0x004cbb20
EntityItemClass::EntityItemClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    mNoLifeSpanDisplay = false;

    m_colorLight.b = 0;
    m_colorLight.g = 0;
    m_colorLight.r = 0;
    m_colorLight.a = 0;

    m_bDynamicLighting = true;

    mLifeSpan = FLT_MAX;
    mGravity = 1.0f;
    mRebound = 0.3f;
    mFriction = 0.5f;
    m_uiLightOdfNameHash = 0;
    m_fRadiusLight = 0.0f;
    mTrailEffect = 0;
}

// SYNTHETIC: SWBF2 0x004cbbd0
// EntityItemClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004cbc00
EntityItemClass::EntityItemClass(EntityItemClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash)
{
    mLifeSpan = pBase->mLifeSpan;
    mNoLifeSpanDisplay = pBase->mNoLifeSpanDisplay;
    mGravity = pBase->mGravity;
    mRebound = pBase->mRebound;
    mFriction = pBase->mFriction;
    m_uiLightOdfNameHash = pBase->m_uiLightOdfNameHash;
    m_colorLight = pBase->m_colorLight;
    m_fRadiusLight = pBase->m_fRadiusLight;
    mTrailEffect = pBase->mTrailEffect;
    mCollisionSound = pBase->mCollisionSound;
    mCollisionFoliageSound = pBase->mCollisionFoliageSound;
    mCollisionWaterSound = pBase->mCollisionWaterSound;
}

// FUNCTION: SWBF2 0x004cbd20
EntityItemClass::~EntityItemClass()
{
}

// FUNCTION: SWBF2 0x004cbd30
void EntityItemClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0x7C7C544B)
    {
        sscanf(pszValue, "%f", &mLifeSpan);
    }
    else if (uProperty == 0x039975BE)
    {
        mNoLifeSpanDisplay = atoi(pszValue) != 0;
    }
    else if (uProperty == 0xC28F0C96 ||
             uProperty == 0xEDC6251A)
    {
        mCollisionSound.SetID(pszValue, false, false);
    }
    else if (uProperty == 0x2297E47D)
    {
        mCollisionWaterSound.SetID(pszValue, false, false);
    }
    else if (uProperty == 0x7E71E2C6)
    {
        if (*pszValue)
        {
            mTrailEffect = FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        }
        else
        {
            mTrailEffect = 0;
        }
    }
    else if (uProperty == 0xD209FF45)
    {
        sscanf(pszValue, "%f", &mGravity);
    }
    else if (uProperty == 0x678DE338)
    {
        sscanf(pszValue, "%f", &mRebound);
    }
    else if (uProperty == 0xA51BE2BB)
    {
        sscanf(pszValue, "%f", &mFriction);
    }
    else if (uProperty == 0xE369687E)
    {
        m_uiLightOdfNameHash = PblHash(pszValue);
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
    else if (uProperty == 0x2776564D)
    {
        sscanf(pszValue, "%f", &m_fRadiusLight);
    }
    else if (uProperty == 0x47C86B4A)
    {
        GameObjectClass::SetProperty(0x47C86B4A, pszValue);
        if (mCollisionModel.GetNumBodies() == 0 && mModel)
        {
            const PblBox& box = mModel->_mModel->_TotalBoundingBox;
            CollisionPrimitive primitive;
            float fX = box._vMax.x - box._vMin.x;
            float fY = box._vMax.y - box._vMin.y;
            float fZ = box._vMax.z - box._vMin.z;
            float fLength = sqrtf(fY * fY + fX * fX + fZ * fZ);
            primitive.SetBox(fX * 0.5f, fY * 0.5f, fZ * 0.5f, fLength * 0.5f);
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = (box._vMin.x + box._vMax.x) * 0.5f;
            matrix.m[3][1] = (box._vMin.y + box._vMax.y) * 0.5f;
            matrix.m[3][2] = (box._vMin.z + box._vMax.z) * 0.5f;
            matrix.m[3][3] = 1.0f;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
    }
    else
    {
        GameObjectClass::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x004cc1f0
EntityClass* EntityItemClass::Derive(unsigned int uHash)
{
    return new EntityItemClass(this, uHash);
}

// GLOBAL: SWBF2 0x01ebbe28
// SYNTHETIC: SWBF2 0x00402bc0
// `dynamic initializer for 'EntityItem::rttiHashEntityItem''
PblHash EntityItem::rttiHashEntityItem("EntityItem");
char EntityItem::rttiNameEntityItem[] = "EntityItem";

class CollisionManager
{
public:
    static bool AddItemBody(CollisionObject* pObject);
    static bool DeactivateItem(CollisionObject* pObject);
};

// FUNCTION: SWBF2 0x004cc260
Entity* EntityItemClass::Build(const EntityDesc& desc)
{
    EntityItem* pItem = new EntityItem(this, desc);
    if (pItem)
    {
        CollisionManager::AddItemBody(pItem);
        pItem->ActivatePhysics();
        pItem->BuildEffects();
    }
    return pItem;
}

// FUNCTION: SWBF2 0x004cac40
EntityItem::EntityItem(EntityItemClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_ITEM, &pClass->mModel->_mLodData)
{
    mVelocity = gVectorZero;
    mStationary = false;
    mClass = pClass;
    mLifeTimer = pClass->mLifeSpan;
    mOwner = 0;
    m_pLight = 0;
    mTrailEmitterObj.mObject = 0;
    mTrailEmitterObj.mSavedHandleId = 0;

    Activate();

    if (!NetGame::IsEnabled() || NetGame::IsClient())
    {
        if (mClass->m_uiLightOdfNameHash)
        {
            m_pLight = EntityLightClass::Build(mClass->m_uiLightOdfNameHash);
            if (mClass->m_fRadiusLight > 0.0f)
            {
                m_pLight->SetOmniLightRadius(mClass->m_fRadiusLight);
            }
            if (mClass->m_colorLight.a > 0)
            {
                m_pLight->SetColor(mClass->m_colorLight);
            }
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = mMatrix.m[3][0];
            matrix.m[3][1] = mMatrix.m[3][1];
            matrix.m[3][2] = mMatrix.m[3][2];
            matrix.m[3][3] = 1.0f;
            if (m_pLight->_fBeamLength > 0.0f)
            {
                *(PblVector3*)matrix.m[0] = gVectorUnitX;
                matrix.m[0][3] = 0.0f;
                matrix.m[1][0] = -gVectorUnitZ.x;
                matrix.m[1][1] = -gVectorUnitZ.y;
                matrix.m[1][2] = -gVectorUnitZ.z;
                matrix.m[1][3] = 0.0f;
                *(PblVector3*)matrix.m[2] = gVectorUnitY;
                matrix.m[2][3] = 0.0f;
            }
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
        else if (mClass->m_fRadiusLight > 0.0f && mClass->m_colorLight.a > 0)
        {
            m_pLight = new EntityLight();
            m_pLight->SetColor(mClass->m_colorLight);
            m_pLight->SetOmniLightRadius(mClass->m_fRadiusLight);
            m_pLight->SetHaloRadius(0.0f);
            m_pLight->SetFlareIntensity(0.0f);
            PblMatrix matrix = _IdentityMatrix;
            matrix.m[3][0] = mMatrix.m[3][0];
            matrix.m[3][1] = mMatrix.m[3][1];
            matrix.m[3][2] = mMatrix.m[3][2];
            matrix.m[3][3] = 1.0f;
            m_pLight->SetMatrix(matrix);
            m_pLight->TurnOn();
        }
    }

    if (mClass->mTrailEffect &&
        (!NetGame::IsEnabled() || NetGame::IsClient() || (NetGame::IsOneWorld() && NetGame::IsHost())))
    {
        mTrailEmitterObj = mClass->mTrailEffect->BuildEffect();
        if (mTrailEmitterObj)
        {
            mTrailEmitterObj->AttachEffectToVector((const PblVector3*)mMatrix.m[3], true);
            mTrailEmitterObj->SetOrientation(*(const PblVector3*)mMatrix.m[2]);
        }
    }
    else
    {
        mTrailEmitterObj = 0;
    }

    SetPortalActive(false);
    pvs::IManager::GetInstance().UnregisterDynamic(*this);
}

// FUNCTION: SWBF2 0x004cb100
EntityItem::~EntityItem()
{
    delete m_pLight;
    if (mTrailEmitterObj)
    {
        mTrailEmitterObj->DeactivateEffect();
    }
    Deactivate();
}

// SYNTHETIC: SWBF2 0x004caba0
// EntityItem::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004cc2df
// [thunk]:EntityItem::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x004cc2ea
// [thunk]:EntityItem::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004cc2f2
// [thunk]:EntityItem::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x004cc2fd
// [thunk]:EntityItem::`scalar deleting destructor' adjustor{576}
// SYNTHETIC: SWBF2 0x004cc308
// [thunk]:EntityItem::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x004cab40
const PblVector3& EntityItem::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x004cab50
void EntityItem::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x004cb210
bool EntityItem::Update(float fDeltaTime)
{
    mLifeTimer -= fDeltaTime;
    if (mLifeTimer > -1.0f && IsAlive())
    {
        if (!mStationary)
        {
            float fHalfDrop = mClass->mGravity * -9.8f * 0.5f * fDeltaTime;
            mVelocity.y += fHalfDrop;
            const PblVector3& vPos = GetPosition();
            PblVector3 vNew(vPos.x + mVelocity.x * fDeltaTime, vPos.y + mVelocity.y * fDeltaTime,
                            vPos.z + mVelocity.z * fDeltaTime);
            SetPosition(vNew);
            mVelocity.y += fHalfDrop;
            if (!(NetGame::IsEnabled() && NetGame::IsClient()) && mMatrix.m[3][1] < -1000.0f)
            {
                Kill();
                return false;
            }
        }

        if (netFrameLock || !NetGame::IsEnabled())
        {
            return true;
        }
        if (!(NetGame::IsClient() && netIsLocalTurn))
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
        gameObjectInterpolator.SetNext(mNetUniqueId, mStationary ? 0 : &mMatrix);
        return true;
    }
    return NetGame::IsEnabled() && NetGame::IsClient();
}

// FUNCTION: SWBF2 0x004cb450
void EntityItem::Render(unsigned char ucLod, float fAlpha, unsigned int uiFlags)
{
    if (!(mLifeTimer > -1.0f))
    {
        return;
    }

    float fFade = 1.0f;
    if (mLifeTimer + 1.0f <= 1.0f)
    {
        fFade = mLifeTimer + 1.0f;
    }
    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(int)(fAlpha * 255.0f * fFade);

    if (mClass->m_bDynamicLighting)
    {
        uiFlags &= ~0x80;
    }
    else
    {
        uiFlags |= 0x80;
    }
    static PblHash sPlayerShadow("player_shadow");
    SetupShadow(this, mModel, ucLod, uiFlags, sPlayerShadow);

    const PblMatrix* pMatrix = &mMatrix;
    PblMatrix mInterpolated;
    if (!netFrameLock && NetGame::IsEnabled())
    {
        mInterpolated = mMatrix;
        gameObjectInterpolator.GetMatrix(mNetUniqueId, mInterpolated);
        pMatrix = &mInterpolated;
    }
    mModel->Render(ucLod, fAlpha, pMatrix, 0, color, uiFlags, 0);
    if (uiFlags & 0x200000)
    {
        return;
    }

    PblMatrix mReflection;
    if (FLRenderer::IsReflected(GetRenderPos(), _Sphere.w, mReflection, false))
    {
        PblMatrix mWorld;
        D3DXMatrixMultiply(&mWorld, pMatrix, &mReflection);
        mReflection = mWorld;
        mModel->Render(ucLod, fAlpha, &mReflection, 0, color, (uiFlags & ~0x10000) | 0x10000100, 0);
    }
}

// FUNCTION: SWBF2 0x004cb6e0
bool EntityItem::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    if (mStationary)
    {
        return false;
    }

    if (pOther->GetCollisionType() == COLL_LEAFPATCH)
    {
        mClass->mCollisionFoliageSound.Play((const PblVector3*)mMatrix.m[3], 0, false, true);
    }

    if (pOther->GetCollisionType() != COLL_SOFT && pOther->GetCollisionType() != COLL_RIGID)
    {
        const PblVector3& vNormal = pResult->mSeparationNormal;
        float fDot = vNormal.x * mVelocity.x + mVelocity.y * vNormal.y + mVelocity.z * vNormal.z;
        if (fDot < 0.0f)
        {
            (pOther->GetCollisionType() == COLL_WATER ? mClass->mCollisionWaterSound
                                                      : mClass->mCollisionSound)
                .Play((const PblVector3*)mMatrix.m[3], 0, false, true);
            if (fDot > -1.0f)
            {
                mMatrix = NormalMatrix(*(const PblVector3*)mMatrix.m[2], vNormal,
                                     *(const PblVector3*)mMatrix.m[3]);
                mVelocity = gVectorZero;
                mStationary = true;
                CollisionManager::DeactivateItem(EntityGeometry::GetCollisionObject());
            }
            else
            {
                float fRebound = mClass->mRebound;
                float fFriction = mClass->mFriction * fDot * (fRebound + 1.0f);
                mVelocity.x -= vNormal.x * fDot;
                mVelocity.y -= vNormal.y * fDot;
                mVelocity.z -= vNormal.z * fDot;
                float fScale = -(fFriction / sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                                   mVelocity.z * mVelocity.z));
                if (!(1.0f > fScale))
                {
                    fScale = 1.0f;
                }
                mVelocity.x -= fScale * mVelocity.x;
                mVelocity.y -= mVelocity.y * fScale;
                mVelocity.z -= mVelocity.z * fScale;
                float fBounce = fRebound * fDot;
                mVelocity.x -= vNormal.x * fBounce;
                mVelocity.y -= vNormal.y * fBounce;
                mVelocity.z -= vNormal.z * fBounce;
            }
        }
    }
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x004cba60
void EntityItem::ActivatePhysics()
{
    ActivateThread(-1, PER_TURN);
}

// FUNCTION: SWBF2 0x004cba90
void EntityItem::SetPosition(const PblVector3& vPosition)
{
    EntityGeometry::SetPosition(vPosition);
    if (m_pLight)
    {
        m_pLight->FLEffectObject::SetPosition(GetPosition());
    }
}

// FUNCTION: SWBF2 0x004caae0
bool EntityItem::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityItem || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004cab20
unsigned int EntityItem::GetDerivedRtti() const
{
    return rttiHashEntityItem;
}

// FUNCTION: SWBF2 0x004cab30
const char* EntityItem::GetDerivedRttiName() const
{
    return rttiNameEntityItem;
}

// FUNCTION: SWBF2 0x004cab70
float EntityItem::GetMaxSpeed() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004cab80
float EntityItem::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004cab90
bool EntityItem::IsSuitableEnemy() const
{
    return false;
}

// FUNCTION: SWBF2 0x004cba80
void EntityItem::DeactivatePhysics()
{
    static_cast<Thread*>(this)->DeactivateThread();
}

// FUNCTION: SWBF2 0x004cbad0
void EntityItem::SetMatrix(const PblMatrix& matrix)
{
    EntityGeometry::SetMatrix(matrix);
    if (m_pLight)
    {
        m_pLight->FLEffectObject::SetPosition(GetPosition());
    }
}
