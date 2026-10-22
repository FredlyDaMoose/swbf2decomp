#include "EntityGeometry.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <d3dx9math.h>

#include "AttachedEffects.h"
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"
#include "GameModel.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "ProceduralAnimation.h"
#include "RedEngineFL/Application/pc/pcRedDropShadow.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Util/RedSceneObject.h"
#include "Chunk.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblRandom.h"

// GLOBAL: SWBF2 0x01ebbcbc
// SYNTHETIC: SWBF2 0x00402b30
// `dynamic initializer for 'EntityGeometryClass::rttiHashEntityGeometryClass''
PblHash EntityGeometryClass::rttiHashEntityGeometryClass("EntityGeometryClass");
char EntityGeometryClass::rttiNameEntityGeometryClass[] = "EntityGeometryClass";

// GLOBAL: SWBF2 0x01ebbcc0
// SYNTHETIC: SWBF2 0x00402b20
// `dynamic initializer for 'EntityGeometry::rttiHashEntityGeometry''
PblHash EntityGeometry::rttiHashEntityGeometry("EntityGeometry");
char EntityGeometry::rttiNameEntityGeometry[] = "EntityGeometry";

// FUNCTION: SWBF2 0x004440d0
bool EntityGeometry::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityGeometry || EntityEx::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004441b0
bool EntityGeometryClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityGeometryClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00444190
void EntityGeometry::SetupPose(RedPose& pose) const
{
    mModel->GetRedModel(0)->SetupPose(pose);
}

// FUNCTION: SWBF2 0x004ba080
unsigned int EntityGeometry::GetDerivedRtti() const
{
    return rttiHashEntityGeometry;
}

// FUNCTION: SWBF2 0x004ba090
const char* EntityGeometry::GetDerivedRttiName() const
{
    return rttiNameEntityGeometry;
}

// SYNTHETIC: SWBF2 0x004bd7fe
// [thunk]:EntityGeometry::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004bd806
// [thunk]:EntityGeometry::`scalar deleting destructor' adjustor{148}
// SYNTHETIC: SWBF2 0x004ba0a0
// EntityGeometry::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004ba0d0
unsigned int EntityGeometryClass::GetDerivedRtti()
{
    return rttiHashEntityGeometryClass;
}

// FUNCTION: SWBF2 0x004ba0e0
const char* EntityGeometryClass::GetDerivedRttiName()
{
    return rttiNameEntityGeometryClass;
}

// FUNCTION: SWBF2 0x004ba0f0
EntityGeometry::EntityGeometry(EntityGeometryClass* pClass, const EntityDesc& desc,
                               CollisionObjectType eType, const RedLodData* pLodData)
    : EntityEx(desc.mNameId, pClass),
      CollisionObject(eType, &pClass->mCollisionModel, pClass, pClass),
      RedSceneObject(pLodData)
{
    m_pAttachedEffects = 0;
    if (pClass)
    {
        _fPriorityMod = pClass->_fPriorityMod;
        if (_fPriorityMod > 1.0f)
        {
            _fPriorityMod += 0.001f;
        }
    }
    mModel = pClass->mModel;
    mMatrix = desc.mMatrix;

    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);

    float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
    if (!(fRadius > pClass->mPotentialCollisionRadius))
    {
        fRadius = pClass->mPotentialCollisionRadius;
    }
    SetCollisionRadius(fRadius);
    Set2DCollisionRadius(fRadius);
    RecalculateAABB();

    const PblBox& box = mModel->_mModel->_TotalBoundingBox;
    float fDimY = box._vMax.y - box._vMin.y;
    float fDimX = box._vMax.x - box._vMin.x;
    float fDimZ = box._vMax.z - box._vMin.z;
    SetMass(fDimY * fDimX * fDimZ + 0.0001f);

    SetRenderPos(GetCollisionCenter());
    SetRenderRadius(GetCollisionRadius());

    float fSmallest = fDimZ > fDimY ? fDimY : fDimZ;
    mSmallestCollisionDim = fSmallest > fDimX ? fDimX : fSmallest;
}

// FUNCTION: SWBF2 0x004ba370
EntityGeometry::~EntityGeometry()
{
    PAAnimationGroup::GeometryDeleted(this);
    delete m_pAttachedEffects;
}

// FUNCTION: SWBF2 0x004ba410
void EntityGeometry::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        const GameModel* pModel = GameModel::Find(PblHash(pszValue));
        if (pModel)
        {
            SetModel(pModel);
            PblVector3 vCenter;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                                   (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                                   (const D3DXMATRIX*)&mMatrix);
            SetCollisionCenter(vCenter);
            float fRadius = mModel->_mModel->_BoundingSphere.GetRadius();
            EntityGeometryClass* pClass = static_cast<EntityGeometryClass*>(mEntityClass);
            if (!(fRadius > pClass->mPotentialCollisionRadius))
            {
                fRadius = pClass->mPotentialCollisionRadius;
            }
            SetCollisionRadius(fRadius);
            Set2DCollisionRadius(fRadius);
            RecalculateAABB();
            SetRenderPos(GetCollisionCenter());
            SetRenderRadius(GetCollisionRadius());
        }
        break;
    }
    case 0x235baa54:
        _fPriorityMod = (float)atof(pszValue);
        break;
    case 0x6ca8a6d1:
        if (atoi(pszValue))
        {
            if (!_bActive)
            {
                Activate();
            }
        }
        else if (_bActive)
        {
            Deactivate();
        }
        break;
    case 0xbc85e3be:
        if (atoi(pszValue))
        {
            if (!IsInCollisionList())
            {
                switch (GetCollisionType())
                {
                case COLL_ITEM:     AddItemToCollisionManager(); break;
                case COLL_SOFT:     CollisionManager::AddSoftBody(this); break;
                case COLL_ASTEROID: AddAsteroidToCollisionManager(); break;
                case COLL_RIGID:    CollisionManager::AddRigidBody(this); break;
                case COLL_STATIC:   AddStaticToCollisionManager(); break;
                case COLL_ATTACHED: AddAttachedToCollisionManager(); break;
                case COLL_SHIELD:   AddShieldToCollisionManager(); break;
                }
            }
        }
        else if (IsInCollisionList())
        {
            CollisionManager::RemoveBody(this);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x004ba660
void EntityGeometry::BuildEffects()
{
    EntityGeometryClass* pClass = static_cast<EntityGeometryClass*>(mEntityClass);
    delete m_pAttachedEffects;
    m_pAttachedEffects = 0;
    if (pClass->m_pAttachedEffectsClass)
    {
        m_pAttachedEffects = pClass->m_pAttachedEffectsClass->BuildEffects(this);
    }
}

// FUNCTION: SWBF2 0x004ba6a0
float EntityGeometry::GetSmallestCollisionDim() const
{
    if (mModel)
    {
        return mSmallestCollisionDim;
    }
    return GetCollisionRadius() * 2.0f;
}

// FUNCTION: SWBF2 0x004ba6f0
int EntityGeometry::GetTargetBodyID(const PblVector3& vPos, const PblVector3& vDir) const
{
    if (mCollisionModel->GetNumBodies() > 0 &&
        !mCollisionModel->GetCollisionMask(COLL_TARGETABLE).IsEmpty())
    {
        return mCollisionModel->GetTargetBodyID(vPos, vDir, mMatrix, GetPose());
    }
    return -1;
}

// FUNCTION: SWBF2 0x004ba750
PblVector3 EntityGeometry::GetTargetPoint(const PblVector3& vPos, const PblVector3& vDir,
                                          int iBodyId) const
{
    if (iBodyId >= 0)
    {
        return mCollisionModel->GetWorldMatrix(iBodyId, mMatrix, GetPose()).GetTranslation();
    }

    if (mCollisionModel->GetNumBodies() > 0 &&
        !mCollisionModel->GetCollisionMask(COLL_TARGETABLE).IsEmpty())
    {
        return mCollisionModel->GetTargetPoint(vPos, vDir, mMatrix, GetPose());
    }

    PblVector3 vOffset;
    D3DXVec3TransformNormal(
        (D3DXVECTOR3*)&vOffset,
        (const D3DXVECTOR3*)&static_cast<EntityGeometryClass*>(mEntityClass)->GetTargetPointOffset(),
        (const D3DXMATRIX*)&GetMatrix());
    const PblVector3& vCenter = GetCollisionCenter();
    return PblVector3(vCenter.x + vOffset.x, vCenter.y + vOffset.y, vCenter.z + vOffset.z);
}

// FUNCTION: SWBF2 0x004ba870
PblVector3 EntityGeometry::GetTargetPointMissVector() const
{
    return PblVector3(0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x004bb220
bool EntityGeometry::SetupShadow(const GameObject* pObject, const GameModel* pModel, unsigned char ucLod,
                                 unsigned int& uFlags, unsigned int uShadowTexture)
{
    RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
    if (!pLight->IsShadowCaster())
    {
        pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                     ? RedLight::GetGlobalDirectionalLight(1)
                     : NULL;
    }
    if (!pLight)
    {
        uFlags &= ~0x10000;
        return false;
    }

    if (ucLod == 0 && !(uFlags & 0x220000) && pModel->GetRedModel(0)->_pStencilShadow &&
        RedStencilShadow::GetShadowEnable())
    {
        const PblVector3& vDir = pLight->m_vDirection;
        float fMinLength = _Sphere.w * -2.0f / vDir.y;
        if (fMinLength < 6.0f)
        {
            fMinLength = 6.0f;
        }
        uFlags |= 0x10000;

        float fBack = _Sphere.w * 2.0f;
        PblVector3 vStart(_Sphere.x - vDir.x * fBack, _Sphere.y - vDir.y * fBack, _Sphere.z - vDir.z * fBack);
        CollisionObject* pHit;
        float fLength = CollisionManager::RayHit(vStart, vDir, 50.0f, pHit, NULL, &pObject, pObject != NULL,
                                                 0x90, false) * 50.0f;
        if (fMinLength > fLength)
        {
            fLength = fMinLength;
        }

        float fNear, fFar, fWidth, fHeight;
        RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
        const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
        const PblVector3& vCamPos = mCamera.GetTranslation();
        const PblVector3& vCamDir = mCamera.GetZAxis();
        float fDX = vCamPos.x - vCamDir.x * fNear - _Sphere.x;
        float fDY = vCamPos.y - vCamDir.y * fNear - _Sphere.y;
        float fDZ = vCamPos.z - vCamDir.z * fNear - _Sphere.z;
        float fReach = _Sphere.w + fLength + fWidth * 1.33f;
        RedRenderer::SetStencilShadowLength(fLength);
        RedRenderer::SetStencilShadowZFail(fReach * fReach > fDX * fDX + fDY * fDY + fDZ * fDZ);
        uFlags |= 0x10000;
        return true;
    }

    uFlags &= ~0x10000;
    if (uShadowTexture)
    {
        PblVector3 vDown(-gVectorUnitY.x, -gVectorUnitY.y, -gVectorUnitY.z);
        PblVector3 vPos(_Sphere.x, _Sphere.y, _Sphere.z);
        float fRadius = GetCollisionRadius();
        if (const RedModel* pRedModel = pModel->GetRedModel(ucLod))
        {
            fRadius = pRedModel->_BoundingSphere.w;
        }
        PblVector3 vNormal = gVectorUnitY;
        CollisionObject* pHit;
        float fHit = CollisionManager::RayHit(vPos, vDown, 5.0f, pHit, &vNormal, &pObject, pObject != NULL,
                                              0x90, false);
        if (fHit < 1.0f)
        {
            float fAlpha = 1.0f - RedRenderer::GetShadowIntensity();
            float fDist = fHit * 5.0f;
            PblVector3 vHit(vPos.x + vDown.x * fDist, vPos.y + vDown.y * fDist, vPos.z + vDown.z * fDist);
            const PblVector3& vDir = pLight->m_vDirection;
            PblVector3 vShadowNormal(0.0f - vDir.x * 0.2f, 0.8f - vDir.y * 0.2f, 0.0f - vDir.z * 0.2f);
            float fStart = vPos.y - fRadius - vHit.y;
            if (-vDir.x * vShadowNormal.x - vShadowNormal.z * vDir.z > -vDir.x * vNormal.x - vNormal.z * vDir.z)
            {
                RedDropShadowManager::AddShadow(uShadowTexture, vHit, vShadowNormal, vDir, fStart,
                                                fStart + fRadius * 2.0f, fRadius * 2.0f, fAlpha);
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004bb700
void EntityGeometry::SetupLightingState(unsigned int& uFlags)
{
    PblSphere sphere;
    sphere.SetCenter(GetCollisionCenter());
    sphere.SetRadius(GetCollisionRadius());
    RedRenderer::SetLightingState(*RedLight::GetLightingState(sphere, 4, (uFlags & 0x80) != 0, false, false));
    uFlags |= 0x80000000;
}

// FUNCTION: SWBF2 0x004bb790
EntityGeometryClass::EntityGeometryClass(unsigned int uHash)
    : EntityClass(uHash)
{
    _fPriorityMod = 1.0f;
    mModel = 0;

    for (int i = 0; i < 10; ++i)
    {
        mChunkAttachPoints[i] = 0;
    }

    mLODClass = 1;
    mHitLocations = 0;
    mNumHitLocations = 0;

    for (int i = 0; i < 10; ++i)
    {
        mChunkDescriptors[i] = 0;
        mFullChunkCollisions[i] = 0;
    }

    for (int i = 0; i < 10; ++i)
    {
        mInitialChunkSpeed[i] = -1.0f;
        mChunkVelocityFactor[i] = 1.0f;
        mChunkUpFactor[i] = 0.25f;
    }

    mPotentialCollisionRadius = 0.0f;
    m_bDynamicLighting = false;
    m_pAttachedEffectsClass = 0;
    mMaxShadowLength = -1.0f;

    mTargetPointOffset.z = 0.0f;
    mTargetPointOffset.y = 0.0f;
    mTargetPointOffset.x = 0.0f;
}

// FUNCTION: SWBF2 0x004b9f50
void ChunkDescriptor::Init()
{
    mMatrix.Reset();

    mVelocity.x = 0.0f;
    mVelocity.y = 0.0f;
    mVelocity.z = 0.0f;
    mModel = 0;
    mPrimitive = 0;
    mMaxTerrainCollisions = 0;
    mTerrainImpactEffect = 0;
    mPhysicsType = 0;
    mOmega.x = 0.0f;
    mOmega.y = 0.0f;
    mOmega.z = 0.0f;
    mGravity.x = 0.0f;
    mGravity.y = -9.8f;
    mGravity.z = 0.0f;
    mFullCollisionModel = 0;
    mCoefficientOfRestitution = 0.0f;
    mCoefficientOfFriction = 0.0f;
    mSimpleFrictionConstant = 0.0f;
    mMinDeathCollisionSpeed = 0.0f;
    mForceZeroYAxisVelocity = false;
    mMaxStaticLifetime = -1.0f;
    for (int i = 0; i < 3; i++)
    {
        mTrailEffect[i] = 0;
        mSmokeEffect[i] = 0;
        mTrailAttachPoints[i] = 0;
        mSmokeAttachPoints[i] = 0;
    }
    mNoBuildingCollisions = false;
    mKeepSoldierCollisions = false;
    mLinearDamping = 0.0f;
    mAngularDamping = 0.0f;
}

// SYNTHETIC: SWBF2 0x004bb0b0
// ChunkDescriptor::ChunkDescriptor

// GLOBAL: SWBF2 0x007e650c
static int sCurrentChunk = -1;
// GLOBAL: SWBF2 0x007e6510
static int sCurrentTrail = -1;
// GLOBAL: SWBF2 0x007e6508
static int sCurrentSmoke = -1;

// GLOBAL: SWBF2 0x01e51db0
static unsigned int sOrdnanceCollisionMask;
// GLOBAL: SWBF2 0x01e51db4
static unsigned int sStaticCollisionMask;
// GLOBAL: SWBF2 0x01e51db8
static unsigned int sTerrainCollisionMask;
// GLOBAL: SWBF2 0x01e51dbc
static unsigned int sTargetableCollisionMask;
// GLOBAL: SWBF2 0x01e51dc0
static unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e51dc4
static unsigned int sRigidCollisionMask;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x004bb9e0
EntityGeometryClass::EntityGeometryClass(EntityGeometryClass* pBase, unsigned int uHash)
    : EntityClass(pBase, uHash),
      FoleyFXCollideeClass(pBase),
      FoleyFXColliderClass(pBase)
{
    _fPriorityMod = 1.0f;
    mModel = pBase->mModel;

    for (int i = 0; i < 10; ++i)
    {
        mChunkDescriptors[i] = pBase->mChunkDescriptors[i];
    }
    for (int i = 0; i < 10; ++i)
    {
        mFullChunkCollisions[i] = pBase->mFullChunkCollisions[i];
    }
    for (int i = 0; i < 10; ++i)
    {
        mChunkAttachPoints[i] = pBase->mChunkAttachPoints[i];
    }

    mLODClass = pBase->mLODClass;

    for (int i = 0; i < 10; ++i)
    {
        mInitialChunkSpeed[i] = pBase->mInitialChunkSpeed[i];
    }
    for (int i = 0; i < 10; ++i)
    {
        mChunkVelocityFactor[i] = pBase->mChunkVelocityFactor[i];
    }
    for (int i = 0; i < 10; ++i)
    {
        mChunkUpFactor[i] = pBase->mChunkUpFactor[i];
    }

    mNumHitLocations = pBase->mNumHitLocations;
    mHitLocations = 0;
    if (mNumHitLocations > 0)
    {
        mHitLocations = new HitLocationData[10];
        memcpy(mHitLocations, pBase->mHitLocations, mNumHitLocations * sizeof(HitLocationData));
    }

    mPotentialCollisionRadius = pBase->mPotentialCollisionRadius;
    m_bDynamicLighting = pBase->m_bDynamicLighting;
    m_pAttachedEffectsClass = pBase->m_pAttachedEffectsClass;
    mMaxShadowLength = pBase->mMaxShadowLength;
    mTargetPointOffset = pBase->mTargetPointOffset;
}

// FUNCTION: SWBF2 0x004bbc30
EntityGeometryClass::~EntityGeometryClass()
{
    for (int i = 0; i < 10; ++i)
    {
        if (mChunkDescriptors[i])
        {
            if (mChunkDescriptors[i]->mPrimitive)
            {
                delete mChunkDescriptors[i]->mPrimitive;
                mChunkDescriptors[i]->mPrimitive = 0;
            }
            delete mChunkDescriptors[i];
        }
        delete mFullChunkCollisions[i];
    }
    delete m_pAttachedEffectsClass;
}

// SYNTHETIC: SWBF2 0x004bb9b0
// EntityGeometryClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004bbce0
void EntityGeometryClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xda1de03d)
    {
        switch (PblHash(pszValue))
        {
        case 0x80bf3dc2:
        case 0xa00e8589:
            sCurrentChunk = 0;
            break;
        case 0x9d0e80d0: sCurrentChunk = 1; break;
        case 0x9e0e8263: sCurrentChunk = 2; break;
        case 0xa30e8a42: sCurrentChunk = 3; break;
        case 0xa40e8bd5: sCurrentChunk = 4; break;
        case 0xa10e871c: sCurrentChunk = 5; break;
        case 0xa20e88af: sCurrentChunk = 6; break;
        case 0xa70e908e: sCurrentChunk = 7; break;
        case 0xa80e9221: sCurrentChunk = 8; break;
        default: sCurrentChunk = -1; break;
        }
    }

    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        mModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));

        CollisionBodyPrimitive* pPrimitive;
        int i = 0;
        do
        {
            char acName[128];
            _snprintf(acName, 127, "%s%d", pszValue, i);
            acName[127] = 0;
            pPrimitive = CollisionBodyPrimitive::_HashTable.Find(PblHash(acName));
            if (pPrimitive)
            {
                mCollisionModel.AddBody(pPrimitive);
            }
            ++i;
        } while (pPrimitive);

        sTerrainCollisionMask = 0;
        sStaticCollisionMask = 0;
        sRigidCollisionMask = 0;
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        sTargetableCollisionMask = 0;
        break;
    }

    case 0x235baa54:
        _fPriorityMod = (float)atof(pszValue);
        break;
    case 0x2c88f5bd:
        sscanf(pszValue, "%f", &mMaxShadowLength);
        break;

    case 0x827eb1c9:
    {
        size_t uLength = strlen(pszValue);
        if (uLength)
        {
            if (!_strnicmp(pszValue, "dynamic", uLength))
            {
                m_bDynamicLighting = true;
            }
            else if (!_strnicmp(pszValue, "static", uLength))
            {
                m_bDynamicLighting = false;
            }
        }
        break;
    }

    case 0xe3019fc9:
    {
        float x, y, z;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mTargetPointOffset.x = x;
        mTargetPointOffset.y = y;
        mTargetPointOffset.z = z;
        break;
    }

    case 0xced77479:
    {
        float fMultiplier = 1.0f;
        char acName[64];
        sscanf(pszValue, "%s %f", acName, &fMultiplier);
        if (mNumHitLocations < 10)
        {
            int iBodyId = mCollisionModel.GetMaskId(PblHash(acName));
            if (iBodyId >= 0 && fMultiplier != 1.0f)
            {
                if (!mHitLocations)
                {
                    mHitLocations = new HitLocationData[10];
                }
                mHitLocations[mNumHitLocations].mBodyId = iBodyId;
                mHitLocations[mNumHitLocations].mDamageMultiplier = fMultiplier;
                ++mNumHitLocations;
            }
        }
        break;
    }

    case 0x6d8fe606:
        mCollisionModel.mDenyFlyerLand = atoi(pszValue) != 0;
        break;
    case 0xd9d02e03:
        mCollisionModel.mDeathOnFlyerLand = atoi(pszValue) != 0;
        break;

    case 0x9828f43f:
        AddBuildingCollision(pszValue);
        break;
    case 0xde5365a1:
        AddVehicleCollision(pszValue);
        break;
    case 0xafe693ce:
        AddTerrainCollision(pszValue);
        break;
    case 0xfb2bdf07:
        AddOrdnanceCollision(pszValue);
        break;
    case 0x5dfdc07f:
        AddSoldierCollision(pszValue);
        break;
    case 0xd23dbaf8:
        AddTargetableCollision(pszValue);
        break;

    case 0x74e63ee8:
    case 0x07d00083:
    case 0x330acaab:
    case 0x8cf29543:
    case 0x7cdccf2a:
    case 0xfec8cea9:
    {
        PblVector3 vCenter = mModel->_mModel->_BoundingSphere.GetCenter();

        char acName[32];
        char acType[32];
        char acParent[32];
        float x, y, z, a, b, c;
        sscanf(pszValue, "%s %s %s %f %f %f %f %f %f", acName, acType, acParent, &x, &y, &z, &a, &b, &c);

        PblMatrix matrix = _IdentityMatrix;
        matrix.m[3][0] = x;
        matrix.m[3][1] = y;
        matrix.m[3][2] = z;
        matrix.m[3][3] = 1.0f;

        CollisionPrimitive primitive;

        float fDX = vCenter.x - x;
        float fDY = vCenter.y - y;
        float fDZ = vCenter.z - z;
        float fRadius;
        switch (PblTEMPHash(acType))
        {
        case 0x1ccafdcc:
            primitive.SetBox(a, b, c, -1.0f);
            fRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
            fRadius = (float)sqrt((double)(a * a + b * b + c * c)) + fRadius;
            break;
        case 0x4a182b8f:
            primitive.SetCylinder(a, b);
            fRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
            fRadius = (float)sqrt((double)(a * 2.0f * a + b * 0.25f * b)) + fRadius;
            break;
        case 0x73f51491:
            primitive.SetAACylinder(a, b);
            fRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ);
            fRadius = (float)sqrt((double)(a * 2.0f * a + b * 0.25f * b)) + fRadius;
            break;
        default:
            primitive.SetSphere(a);
            fRadius = sqrtf(fDX * fDX + fDY * fDY + fDZ * fDZ) + a;
            break;
        }
        mPotentialCollisionRadius = __max(mPotentialCollisionRadius, fRadius);

        CollisionMask terrainMask = mCollisionModel.GetCollisionMask(COLL_TERRAIN);
        CollisionMask staticMask = mCollisionModel.GetCollisionMask(COLL_STATIC);
        CollisionMask rigidMask = mCollisionModel.GetCollisionMask(COLL_RIGID);
        CollisionMask softMask = mCollisionModel.GetCollisionMask(COLL_SOFT);
        CollisionMask ordnanceMask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);
        CollisionMask targetableMask = mCollisionModel.GetCollisionMask(COLL_TARGETABLE);

        mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, acName, matrix, PblTEMPHash(acParent)));

        int iMaskId = mCollisionModel.GetMaskId(PblHash(acName));
        if (iMaskId >= 0)
        {
            switch (uProperty)
            {
            case 0x74e63ee8: targetableMask.SetId(iMaskId); break;
            case 0x330acaab: ordnanceMask.SetId(iMaskId); break;
            case 0x07d00083: softMask.SetId(iMaskId); break;
            case 0x7cdccf2a: terrainMask.SetId(iMaskId); break;
            case 0x8cf29543: staticMask.SetId(iMaskId); break;
            case 0xfec8cea9: rigidMask.SetId(iMaskId); break;
            }
            mCollisionModel.SetCollisionMask(COLL_TERRAIN, terrainMask);
            mCollisionModel.SetCollisionMask(COLL_STATIC, staticMask);
            mCollisionModel.SetCollisionMask(COLL_RIGID, rigidMask);
            mCollisionModel.SetCollisionMask(COLL_SOFT, softMask);
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, ordnanceMask);
            mCollisionModel.SetCollisionMask(COLL_TARGETABLE, targetableMask);
        }
        break;
    }

    case 0xda328455:
    {
        if ((unsigned int)sCurrentChunk > 9)
        {
            break;
        }
        mChunkDescriptors[sCurrentChunk] = new ChunkDescriptor;
        mFullChunkCollisions[sCurrentChunk] = new CollisionModel;
        mChunkDescriptors[sCurrentChunk]->Init();
        mChunkDescriptors[sCurrentChunk]->mCoefficientOfRestitution = 0.6f;
        mChunkDescriptors[sCurrentChunk]->mCoefficientOfFriction = 0.7f;
        mChunkDescriptors[sCurrentChunk]->mSimpleFrictionConstant = 0.0f;
        mChunkDescriptors[sCurrentChunk]->mMinDeathCollisionSpeed = -1.0f;
        mChunkDescriptors[sCurrentChunk]->mForceZeroYAxisVelocity = false;
        mChunkDescriptors[sCurrentChunk]->mLinearDamping = 0.001f;
        mChunkDescriptors[sCurrentChunk]->mAngularDamping = 0.004f;
        mChunkDescriptors[sCurrentChunk]->mModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));

        char acName[128];
        CollisionBodyPrimitive* pPrimitive;
        int i = 0;
        do
        {
            _snprintf(acName, 127, "%s%d", pszValue, i);
            acName[127] = 0;
            pPrimitive = CollisionBodyPrimitive::Find(PblHash(acName));
            if (pPrimitive)
            {
                mFullChunkCollisions[sCurrentChunk]->AddBody(pPrimitive);
            }
            ++i;
        } while (pPrimitive);

        if (!mFullChunkCollisions[sCurrentChunk]->GetNumBodies())
        {
            const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
            mFullChunkCollisions[sCurrentChunk]->AddBody(
                new CollisionBodyMesh(pMesh, pszValue, _IdentityMatrix, 0));
        }

        _snprintf(acName, 127, "%s0", pszValue);
        acName[127] = 0;
        mChunkDescriptors[sCurrentChunk]->mPrimitive = CollisionBodyPrimitive::Find(PblHash(acName));
        _snprintf(acName, 127, "%s1", pszValue);
        acName[127] = 0;
        if (!mChunkDescriptors[sCurrentChunk]->mPrimitive || CollisionBodyPrimitive::Find(PblHash(acName)))
        {
            PblBox box = mChunkDescriptors[sCurrentChunk]->mModel->_mModel->_TotalBoundingBox;
            float fCenterX = (box._vMin.x + box._vMax.x) * 0.5f;
            float fCenterY = (box._vMin.y + box._vMax.y) * 0.5f;
            float fCenterZ = (box._vMin.z + box._vMax.z) * 0.5f;

            CollisionPrimitive primitive;
            PblMatrix matrix = _IdentityMatrix;
            float fHalfX = (float)fabs(box._vMax.x - box._vMin.x) * 0.5f;
            float fHalfY = (float)fabs(box._vMax.y - box._vMin.y) * 0.5f;
            float fHalfZ = (float)fabs(box._vMax.z - box._vMin.z) * 0.5f;
            primitive.SetSphere(sqrtf(fHalfX * fHalfX + fHalfY * fHalfY + fHalfZ * fHalfZ));
            matrix.m[3][0] = fCenterX;
            matrix.m[3][1] = fCenterY;
            matrix.m[3][2] = fCenterZ;
            matrix.m[3][3] = 1.0f;
            mChunkDescriptors[sCurrentChunk]->mPrimitive = new CollisionBodyPrimitive(primitive, "", matrix, 0);
        }
        sCurrentSmoke = 0;
        sCurrentTrail = 0;
        break;
    }

    case 0x0a157be2:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            unsigned short usCount;
            sscanf(pszValue, "%hu", &usCount);
            mChunkDescriptors[sCurrentChunk]->mMaxTerrainCollisions = (unsigned char)usCount;
        }
        break;
    case 0x048f1226:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            mChunkDescriptors[sCurrentChunk]->mTerrainImpactEffect = PblHash(pszValue);
        }
        break;
    case 0xc8e32dc3:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            switch (PblHash(pszValue))
            {
            case 0xff79b33c: mChunkDescriptors[sCurrentChunk]->mPhysicsType = 0; break;
            case 0xd290c23b: mChunkDescriptors[sCurrentChunk]->mPhysicsType = 2; break;
            case 0x2648cd55: mChunkDescriptors[sCurrentChunk]->mPhysicsType = 3; break;
            case 0x493ea232: mChunkDescriptors[sCurrentChunk]->mPhysicsType = 4; break;
            default: mChunkDescriptors[sCurrentChunk]->mPhysicsType = 1; break;
            }
        }
        break;
    case 0x166f6939:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            ChunkDescriptor* pDesc = mChunkDescriptors[sCurrentChunk];
            pDesc->mOmega.x = x;
            pDesc->mOmega.y = y;
            pDesc->mOmega.z = z;
        }
        break;
    case 0xd0259d88:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            float x, y, z;
            sscanf(pszValue, "%f %f %f", &x, &y, &z);
            ChunkDescriptor* pDesc = mChunkDescriptors[sCurrentChunk];
            pDesc->mGravity.x = x;
            pDesc->mGravity.y = y;
            pDesc->mGravity.z = z;
        }
        break;
    case 0xc2f5ffd5:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mCoefficientOfRestitution);
        }
        break;
    case 0xb94ae34e:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mCoefficientOfFriction);
        }
        break;
    case 0x1cdaebc2:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mSimpleFrictionConstant);
        }
        break;
    case 0xed35b79b:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mMinDeathCollisionSpeed);
        }
        break;
    case 0x4f17fc11:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mMaxStaticLifetime);
        }
        break;
    case 0xfeb5ed07:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mLinearDamping);
        }
        break;
    case 0x0e3bec8e:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkDescriptors[sCurrentChunk]->mAngularDamping);
        }
        break;
    case 0xc74f3df1:
        mChunkDescriptors[sCurrentChunk]->mKeepSoldierCollisions = atoi(pszValue) != 0;
        break;
    case 0x945d1d15:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mInitialChunkSpeed[sCurrentChunk]);
        }
        break;
    case 0xb3574d92:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkVelocityFactor[sCurrentChunk]);
        }
        break;
    case 0xc316ae10:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            sscanf(pszValue, "%f", &mChunkUpFactor[sCurrentChunk]);
        }
        break;
    case 0xebf014fd:
        if ((unsigned int)sCurrentChunk <= 9)
        {
            mChunkAttachPoints[sCurrentChunk] = PblTEMPHash(pszValue);
        }
        break;

    case 0x94f0544b:
        if ((unsigned int)sCurrentChunk <= 9 && (unsigned int)sCurrentTrail <= 2)
        {
            mChunkDescriptors[sCurrentChunk]->mTrailEffect[sCurrentTrail] = PblHash(pszValue);
            ++sCurrentTrail;
        }
        break;
    case 0xa8fb54e9:
        if ((unsigned int)sCurrentChunk <= 9 && (unsigned int)(sCurrentTrail - 1) <= 2)
        {
            mChunkDescriptors[sCurrentChunk]->mTrailAttachPoints[sCurrentTrail - 1] = PblTEMPHash(pszValue);
        }
        break;
    case 0xd350c3de:
        if ((unsigned int)sCurrentChunk <= 9 && (unsigned int)sCurrentSmoke <= 2)
        {
            mChunkDescriptors[sCurrentChunk]->mSmokeEffect[sCurrentSmoke] = PblHash(pszValue);
            ++sCurrentSmoke;
        }
        break;
    case 0x472e0410:
        if ((unsigned int)sCurrentChunk <= 9 && (unsigned int)(sCurrentSmoke - 1) <= 2)
        {
            mChunkDescriptors[sCurrentChunk]->mSmokeAttachPoints[sCurrentSmoke - 1] = PblTEMPHash(pszValue);
        }
        break;

    default:
        if (!AttachedEffectsClass::SetProperty(uProperty, pszValue) &&
            !FoleyFXCollideeClass::SetProperty(uProperty, pszValue) &&
            !FoleyFXColliderClass::SetProperty(uProperty, pszValue))
        {
            EntityClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x004bd370
void EntityGeometryClass::PostReadSetup()
{
    m_pAttachedEffectsClass = AttachedEffectsClass::BuildAttachedEffectsClass();
}

// FUNCTION: SWBF2 0x004bd380
int EntityGeometryClass::GetNumChunks()
{
    int iNum = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (mChunkDescriptors[i])
        {
            ++iNum;
        }
    }
    return iNum;
}

// FUNCTION: SWBF2 0x004bd3e0
bool EntityGeometryClass::AddBuildingCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sStaticCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sStaticCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_STATIC, CollisionMask(sStaticCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004bd490
bool EntityGeometryClass::AddVehicleCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sRigidCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sRigidCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_RIGID, CollisionMask(sRigidCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004bd540
bool EntityGeometryClass::AddTerrainCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sTerrainCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sTerrainCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_TERRAIN, CollisionMask(sTerrainCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004bd5f0
bool EntityGeometryClass::AddOrdnanceCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sOrdnanceCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sOrdnanceCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004bd6a0
bool EntityGeometryClass::AddSoldierCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sSoftCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sSoftCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004bd750
bool EntityGeometryClass::AddTargetableCollision(const char* pszValue)
{
    unsigned int uName = PblHash(pszValue);
    if (uName == 0xada7afdb || uName == 0x5c6e1222)
    {
        sTargetableCollisionMask = 0;
    }
    else
    {
        int iMaskId = mCollisionModel.GetMaskId(uName);
        if (iMaskId < 0)
        {
            return false;
        }
        sTargetableCollisionMask |= 1 << iMaskId;
    }
    mCollisionModel.SetCollisionMask(COLL_TARGETABLE, CollisionMask(sTargetableCollisionMask));
    return true;
}

// FUNCTION: SWBF2 0x004920e0
void EntityGeometry::SetPosition(const PblVector3& vPos)
{
    *(PblVector3*)&mMatrix.m[3][0] = vPos;
    mMatrix.m[3][3] = 1.0f;

    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);
    RecalculateAABB();
    SetRenderPos(GetCollisionCenter());
}

// FUNCTION: SWBF2 0x004921c0
void EntityGeometry::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;

    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&mMatrix);
    SetCollisionCenter(vCenter);
    RecalculateAABB();
    SetRenderPos(GetCollisionCenter());
}

// FUNCTION: SWBF2 0x004ba890
void EntityGeometry::CreateChunkCluster(float fScale)
{
    if (NetGame::IsEnabled() && NetGame::IsClient())
    {
        return;
    }

    const RedPose* pPose = GetPose();
    RedPose kPose;
    if (!pPose)
    {
        SetupPose(kPose);
        pPose = &kPose;
    }

    EntityGeometryClass* pClass = static_cast<EntityGeometryClass*>(mEntityClass);
    if (!pClass)
    {
        return;
    }

    int iNumChunks = 0;
    float fCenterX = 0.0f;
    float fCenterY = 0.0f;
    float fCenterZ = 0.0f;
    for (int i = 0; i < 10; ++i)
    {
        ChunkDescriptor* pDesc = pClass->mChunkDescriptors[i];
        if (!pDesc)
        {
            continue;
        }
        pDesc->mMatrix = mMatrix;
        if (pClass->mChunkAttachPoints[i] && pPose)
        {
            if (const PblMatrix* pAttach = pPose->Find(pClass->mChunkAttachPoints[i]))
            {
                PblMatrix mAttached;
                D3DXMatrixMultiply((D3DXMATRIX*)&mAttached, (const D3DXMATRIX*)pAttach, (const D3DXMATRIX*)&mMatrix);
                pDesc->mMatrix = mAttached;
            }
        }
        PblMatrix mChunk = pDesc->mPrimitive->GetWorldMatrix(mMatrix, pPose);
        fCenterX += mChunk.m[3][0];
        fCenterY += mChunk.m[3][1];
        fCenterZ += mChunk.m[3][2];
        ++iNumChunks;
    }
    if (iNumChunks <= 0)
    {
        return;
    }

    float fInvCount = 1.0f / (float)iNumChunks;
    fCenterZ *= fInvCount;
    fCenterX *= fInvCount;
    ChunkCluster* pCluster = ChunkClusterManager::GetFreeCluster(true);
    if (!pCluster)
    {
        return;
    }
    pCluster->mParentClass = pClass;
    GetGameObject();
    fCenterY = GetPosition().y;

    for (int i = 0; i < 10; ++i)
    {
        ChunkDescriptor* pDesc = pClass->mChunkDescriptors[i];
        if (!pDesc)
        {
            continue;
        }
        pDesc->mFullCollisionModel =
            pClass->mFullChunkCollisions[i]->GetNumBodies() ? pClass->mFullChunkCollisions[i] : 0;
        pDesc->mMatrix = mMatrix;
        const PblVector3& vVelocity = GetVelocity();
        float fFactor = pClass->mChunkVelocityFactor[i];
        pDesc->mVelocity.x = vVelocity.x * fFactor;
        pDesc->mVelocity.y = vVelocity.y * fFactor;
        pDesc->mVelocity.z = vVelocity.z * fFactor;
        if (pClass->mChunkAttachPoints[i] && pPose)
        {
            if (const PblMatrix* pAttach = pPose->Find(pClass->mChunkAttachPoints[i]))
            {
                PblMatrix mAttached;
                D3DXMatrixMultiply((D3DXMATRIX*)&mAttached, (const D3DXMATRIX*)pAttach, (const D3DXMATRIX*)&mMatrix);
                pDesc->mMatrix = mAttached;
            }
        }

        PblMatrix mChunk = pDesc->mPrimitive->GetWorldMatrix(mMatrix, pPose);
        float fDX = mChunk.m[3][0] - fCenterX;
        float fDY = mChunk.m[3][1] - fCenterY;
        float fDZ = mChunk.m[3][2] - fCenterZ;
        float fDistSq = fDX * fDX + fDY * fDY + fDZ * fDZ;
        float fUp = sqrtf(fDistSq) * pClass->mChunkUpFactor[i];
        PblVector3 vOut(fDX + 0.0f, fUp + fDY, fDZ + 0.0f);
        PblVector3 vDir;
        D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vOut);

        float fSpeed = pClass->mInitialChunkSpeed[i];
        if (0.0f > fSpeed)
        {
            if (fDistSq > 1.0f && GetCollisionRadius() > 0.0001f)
            {
                float fRadius = GetCollisionRadius();
                float fKick = PblRandom::GetSim().GetFloat() * 15.0f + 15.0f;
                float fInvRadius = 1.0f / fRadius;
                pDesc->mVelocity.x += vDir.x * fKick * fInvRadius;
                pDesc->mVelocity.y += vDir.y * fKick * fInvRadius;
                pDesc->mVelocity.z += vDir.z * fKick * fInvRadius;
            }
            if (pDesc->mPhysicsType == PHYSICS_LEAF)
            {
                pDesc->mVelocity.x *= 0.25f;
                pDesc->mVelocity.y *= 0.25f;
                pDesc->mVelocity.z *= 0.25f;
            }
            const PblVector3& v = pDesc->mVelocity;
            if (5.0f > sqrtf(v.x * v.x + v.y * v.y + v.z * v.z))
            {
                float fPush = PblRandom::GetSim().GetFloat() * 2.0f + 4.0f;
                pDesc->mVelocity.x = vDir.x * fPush;
                pDesc->mVelocity.y = vDir.y * fPush;
                pDesc->mVelocity.z = vDir.z * fPush;
                if (2.0f > pDesc->mVelocity.y)
                {
                    pDesc->mVelocity.y = PblRandom::GetSim().GetFloat() + 2.0f;
                }
            }
        }
        else
        {
            pDesc->mVelocity.x += vDir.x * fSpeed;
            pDesc->mVelocity.y += vDir.y * fSpeed;
            pDesc->mVelocity.z += vDir.z * fSpeed;
        }
        pDesc->mVelocity.x *= fScale;
        pDesc->mVelocity.y *= fScale;
        pDesc->mVelocity.z *= fScale;

        ChunkDescriptor kChunk(*pDesc);
        pCluster->AddChunk(kChunk);
    }

    if (NetGame::IsEnabled())
    {
        NetGame::CreateEvent_CreateChunkCluster(pCluster, pClass, (unsigned __int64)-1);
        for (unsigned int i = 0; i < 10; ++i)
        {
            if (!pCluster->mChunks[i].IsMoving())
            {
                NetGame::CreateEvent_CorrectChunkCluster(pCluster, pClass, i, (unsigned __int64)-1);
            }
        }
    }
}

// SYNTHETIC: SWBF2 0x00446e24
// [thunk]:EntityGeometry::GetEntity`adjustor{12}'

// FUNCTION: SWBF2 0x004bd360
void* __fastcall ReturnObjectPointer(void* pObject)
{
    return pObject;
}

// SYNTHETIC: SWBF2 0x004992d2
// [thunk]:EntityGeometry::GetGameObject`adjustor{12}'
