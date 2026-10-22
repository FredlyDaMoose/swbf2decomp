#include "EntityProp.h"
#include "PebbleFL/Common/PblVector.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <d3dx9math.h>

#include "Cloth.h"
#include "CollisionBody.h"
#include "CollisionManager.h"
#include "CollisionMesh.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"
#include "EntityLight.h"
#include "GameModel.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "NetGame.h"
#include "ProceduralAnimation.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Graphics/RedModel.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01e51de4
static unsigned int sTerrainCollisionMask;
// GLOBAL: SWBF2 0x01e51de8
static unsigned int sTargetableCollisionMask;
// GLOBAL: SWBF2 0x01e51dec
static unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e51df0
static unsigned int sRigidCollisionMask;
// GLOBAL: SWBF2 0x01e51df4
static unsigned int sOrdnanceCollisionMask;
// GLOBAL: SWBF2 0x01e51df8
static unsigned int sStaticCollisionMask;

// GLOBAL: SWBF2 0x01faa45c
static EntityClass* sPendingAttachedOdfClass;

// GLOBAL: SWBF2 0x01ebbfec
// SYNTHETIC: SWBF2 0x00402cc0
// `dynamic initializer for 'EntityProp::rttiHashEntityProp''
PblHash EntityProp::rttiHashEntityProp("EntityProp");
char EntityProp::rttiNameEntityProp[] = "EntityProp";

// GLOBAL: SWBF2 0x01ebc040
// SYNTHETIC: SWBF2 0x00402cd0
// `dynamic initializer for 'EntityPropClass::rttiHashEntityPropClass''
PblHash EntityPropClass::rttiHashEntityPropClass("EntityPropClass");
char EntityPropClass::rttiNameEntityPropClass[] = "EntityPropClass";

// FUNCTION: SWBF2 0x00497e50
bool EntityProp::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityProp ||
           reinterpret_cast<const EntityGeometry*>(this)->EntityGeometry::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00497ec0
const PblMatrix& EntityProp::GetBBoxMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00497ed0
const PblBox& EntityProp::GetBBox() const
{
    return GetModel()->_mModel->_TotalBoundingBox;
}

// FUNCTION: SWBF2 0x00497ef0
bool EntityPropClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityPropClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004d8d50
unsigned int EntityProp::GetDerivedRtti() const
{
    return rttiHashEntityProp;
}

// FUNCTION: SWBF2 0x004d8d60
const char* EntityProp::GetDerivedRttiName() const
{
    return rttiNameEntityProp;
}

// FUNCTION: SWBF2 0x004d8da0
unsigned int EntityPropClass::GetDerivedRtti()
{
    return rttiHashEntityPropClass;
}

// FUNCTION: SWBF2 0x004d8db0
const char* EntityPropClass::GetDerivedRttiName()
{
    return rttiNameEntityPropClass;
}

// FUNCTION: SWBF2 0x004d8dc0
void ConvertHSVtoRGB(float* pR, float* pG, float* pB, float fH, float fS, float fV)
{
    fH *= 6.0f;
    if (fS == 0.0f)
    {
        *pB = fV;
        *pG = fV;
        *pR = fV;
        return;
    }

    int i = (int)fH;
    float f = fH - (float)i;
    float p = fV * (1.0f - fS);
    float q = fV * (1.0f - fS * f);
    float t = fV * (1.0f - fS * (1.0f - f));

    switch (i)
    {
    case 0:
        *pR = fV;
        *pG = t;
        *pB = p;
        break;
    case 1:
        *pR = q;
        *pG = fV;
        *pB = p;
        break;
    case 2:
        *pR = p;
        *pG = fV;
        *pB = t;
        break;
    case 3:
        *pR = p;
        *pG = q;
        *pB = fV;
        break;
    case 4:
        *pR = t;
        *pG = p;
        *pB = fV;
        break;
    default:
        *pR = fV;
        *pG = p;
        *pB = q;
        break;
    }
}

// FUNCTION: SWBF2 0x004d8ed0
EntityProp::EntityProp(EntityPropClass* pClass, const EntityDesc& desc)
    : EntityGeometry(pClass, desc, COLL_STATIC, &pClass->mModel->_mLodData)
{
    mMoving = false;
    mClass = pClass;

    PblBox box;
    box._vMin = PblVector3(FLT_MAX, FLT_MAX, FLT_MAX);
    box._vMax = PblVector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    bool bHaveBody = false;
    for (int i = 0; i < mCollisionModel->GetNumBodies(); ++i)
    {
        CollisionBody* pBody = mCollisionModel->GetBody(i);
        if (!pBody)
        {
            continue;
        }
        bHaveBody = true;
        PblBox bodyBox;
        if (pBody->bIsMesh)
        {
            bodyBox = static_cast<CollisionBodyMesh*>(pBody)->mMesh->GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, 0));
        }
        else
        {
            bodyBox = static_cast<CollisionBodyPrimitive*>(pBody)->mPrimitive.GetWorldspaceAABB(
                pBody->GetWorldMatrix(mMatrix, 0));
        }
        box._vMin.x = bodyBox._vMin.x > box._vMin.x ? box._vMin.x : bodyBox._vMin.x;
        box._vMin.y = bodyBox._vMin.y > box._vMin.y ? box._vMin.y : bodyBox._vMin.y;
        box._vMin.z = bodyBox._vMin.z > box._vMin.z ? box._vMin.z : bodyBox._vMin.z;
        box._vMax.x = box._vMax.x > bodyBox._vMax.x ? box._vMax.x : bodyBox._vMax.x;
        box._vMax.y = box._vMax.y > bodyBox._vMax.y ? box._vMax.y : bodyBox._vMax.y;
        box._vMax.z = box._vMax.z > bodyBox._vMax.z ? box._vMax.z : bodyBox._vMax.z;
    }
    if (bHaveBody)
    {
        mAABB = box;
    }

    float fHalfX = (mAABB._vMax.x - mAABB._vMin.x) * 0.5f;
    float fHalfZ = (mAABB._vMax.z - mAABB._vMin.z) * 0.5f;
    float f2dRadius = sqrtf(fHalfX * fHalfX + fHalfZ * fHalfZ);
    if (GetCollisionRadius() > f2dRadius)
    {
        Set2DCollisionRadius(f2dRadius);
    }
    SetTestBBox(GetCollisionRadius() > 1.0f);
    AddToScene();

    mShadowProxy = 0;
    const RedModel* pRedModel = mModel->GetRedModel(0);
    if (pRedModel && pRedModel->_pStencilShadow)
    {
        mShadowProxy = new RedShadowProxy;
        mShadowProxy->BuildData(mModel->GetRedModel(0), &mMatrix, GetCollisionRadius(),
                                GetCollisionCenter());
    }

    const RedColor& cMin = pClass->mGeometryColorMin;
    const RedColor& cMax = pClass->mGeometryColorMax;
    PblVector4 vColor;
    vColor.w = (cMax.a - cMin.a) * PblRandom::GetGlobalFloat() + cMin.a;
    vColor.z = (cMax.b - cMin.b) * PblRandom::GetGlobalFloat() + cMin.b;
    vColor.y = (cMax.g - cMin.g) * PblRandom::GetGlobalFloat() + cMin.g;
    vColor.x = (cMax.r - cMin.r) * PblRandom::GetGlobalFloat() + cMin.r;
    if (pClass->mUseHSVFlag)
    {
        ConvertHSVtoRGB(&vColor.x, &vColor.y, &vColor.z, vColor.x, vColor.y, vColor.z);
    }
    mGeometryColor.r = (unsigned char)(int)vColor.x;
    mGeometryColor.g = (unsigned char)(int)vColor.y;
    mGeometryColor.b = (unsigned char)(int)vColor.z;
    mGeometryColor.a = (unsigned char)(int)vColor.w;

    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        EntityClass* pClothClass = EntityClass::Find(mModel->mClothList[i]);
        if (pClothClass)
        {
            mClothPiece[i] = static_cast<EntityCloth*>(pClothClass->Build(desc));
        }
    }

    PAAnimationGroup::GeometryCreated(desc.mNameId, this, mMatrix);

    PblVector3 vOffset;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vOffset, (const D3DXVECTOR3*)&mClass->mSoundOffset,
                            (const D3DXMATRIX*)&mMatrix);
    const PblVector3& vPos = mMatrix.GetTranslation();
    mPrevPos = PblVector3(vOffset.x + vPos.x, vOffset.y + vPos.y, vOffset.z + vPos.z);
}

// FUNCTION: SWBF2 0x00497ea0
void EntityProp::Kill()
{
    CollisionManager::RemoveBody(this);
    Deactivate();
}

// SYNTHETIC: SWBF2 0x004d8d70
// EntityProp::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x004db282
// [thunk]: EntityProp::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x004db28a
// [thunk]: EntityProp::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x004d9660
EntityProp::~EntityProp()
{
    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        delete mClothPiece[i];
    }
    delete mShadowProxy;
}

// FUNCTION: SWBF2 0x004d9710
void EntityProp::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xb2e7fe34)
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        mGeometryColor.r = (unsigned char)r;
        mGeometryColor.g = (unsigned char)g;
        mGeometryColor.b = (unsigned char)b;
        mGeometryColor.a = (unsigned char)a;
    }
    else
    {
        reinterpret_cast<EntityGeometry*>(this)->EntityGeometry::SetProperty(uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x004d9790
void EntityProp::RecalculateAABB()
{
    mAABB = mModel->_mModel->_TotalBoundingBox.GetTransformedBox(mMatrix);
}

// FUNCTION: SWBF2 0x004d97d0
void EntityProp::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    const EntityPropClass* pClass = static_cast<const EntityPropClass*>(mEntityClass);
    if (pClass->m_bDynamicLighting)
    {
        uiFlags &= ~0x80;
    }
    else
    {
        uiFlags |= 0x80;
    }
    if (mShadowProxy && mShadowProxy->IsInScene())
    {
        uiFlags &= ~0x10000;
    }

    mModel->Render(ucViewport, fDist, &mMatrix, NULL, mGeometryColor, uiFlags | 0x1000000, NULL);

    for (unsigned int i = 0; i < mModel->mClothCount; ++i)
    {
        if (mClothPiece[i])
        {
            RedColor color = RedColor::WHITE;
            color.a = (unsigned char)(int)(fDist * 255.0f);
            mClothPiece[i]->Render(mMatrix, NULL, ucViewport, color, uiFlags, 1);
        }
    }

    if (pClass->mReflected && !(uiFlags & 0x200000))
    {
        PblMatrix mReflection;
        if (FLRenderer::IsReflected(*(const PblVector3*)&_Sphere, _Sphere.w, mReflection, false))
        {
            PblMatrix mWorld;
            D3DXMatrixMultiply((D3DXMATRIX*)&mWorld, (const D3DXMATRIX*)&mMatrix,
                               (const D3DXMATRIX*)&mReflection);
            mReflection = mWorld;
            mModel->Render(ucViewport, fDist, &mReflection, NULL, mGeometryColor,
                           (uiFlags & ~0x10000) | 0x10000100, NULL);
        }
    }

    UpdateSound();
}

// FUNCTION: SWBF2 0x004d99a0
void EntityProp::UpdateSound()
{
    bool bMovementSounds = mClass->mStopMovementSound.GetProperties() ||
                           mClass->mStartMovementSound.GetProperties() ||
                           mClass->mAttachedSoundOnlyMoving;
    if (!mClass->mAttachedSound.GetProperties() && !bMovementSounds)
    {
        return;
    }

    PblVector3 vOffset;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vOffset, (const D3DXVECTOR3*)&mClass->mSoundOffset,
                            (const D3DXMATRIX*)&mMatrix);
    PblVector3 vPos;
    vPos.x = mMatrix.m[3][0] + vOffset.x;
    vPos.y = mMatrix.m[3][1] + vOffset.y;
    vPos.z = mMatrix.m[3][2] + vOffset.z;

    float fMove;
    if (bMovementSounds)
    {
        const float fLength2EstAlpha = 0.94805944f;
        const float fLength2EstBeta = 0.39269909f;

        float fA = (float)fabs(vPos.x - mPrevPos.x);
        float fB = (float)fabs(vPos.y - mPrevPos.y);
        float fEst = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                             : fA * fLength2EstBeta + fB * fLength2EstAlpha;

        fA = (float)fabs(fEst);
        fB = (float)fabs(vPos.z - mPrevPos.z);
        fMove = fA > fB ? fA * fLength2EstAlpha + fB * fLength2EstBeta
                        : fA * fLength2EstBeta + fB * fLength2EstAlpha;
    }
    else
    {
        fMove = 0.0f;
    }

    mPrevPos.x = (vPos.x - mPrevPos.x) * 0.5f + mPrevPos.x;
    mPrevPos.y = (vPos.y - mPrevPos.y) * 0.5f + mPrevPos.y;
    mPrevPos.z = (vPos.z - mPrevPos.z) * 0.5f + mPrevPos.z;

    if (mAttachedSound.mFlags & GameSoundControllable::flags_playing)
    {
        mAttachedSound.Update(&mClass->mAttachedSound, &mPrevPos, 0);
    }
    else if (!mClass->mAttachedSoundOnlyMoving)
    {
        mAttachedSound.Play(&mClass->mAttachedSound, &mPrevPos, 0, true);
    }

    if (mMoving)
    {
        if (fMove < 0.0001f)
        {
            mClass->mStopMovementSound.Play(&mPrevPos, 0, false, true);
            if (mClass->mAttachedSoundOnlyMoving &&
                !(mAttachedSound.mFlags & GameSoundControllable::flags_playing))
            {
                mAttachedSound.Play(&mClass->mAttachedSound, &mPrevPos, 0, true);
            }
            mMoving = false;
        }
    }
    else if (fMove > 0.0001f)
    {
        mClass->mStartMovementSound.Play(&mPrevPos, 0, false, true);
        if (mClass->mAttachedSoundOnlyMoving)
        {
            mAttachedSound.Stop(true);
        }
        mMoving = true;
    }
}

// FUNCTION: SWBF2 0x004d9c60
EntityPropClass::EntityPropClass(unsigned int uHash)
    : EntityGeometryClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    mSoundOffset.z = 0.0f;
    mSoundOffset.y = 0.0f;
    mSoundOffset.x = 0.0f;
    mUseHSVFlag = false;
    mReflected = false;
    mAttachedSoundOnlyMoving = false;

    mGeometryColorMax.b = 0xFF;
    mGeometryColorMax.g = 0xFF;
    mGeometryColorMax.r = 0xFF;
    mGeometryColorMax.a = 0xFF;
    mGeometryColorMin.b = 0xFF;
    mGeometryColorMin.g = 0xFF;
    mGeometryColorMin.r = 0xFF;
    mGeometryColorMin.a = 0xFF;

    mLODClass = 0;
    m_bDynamicLighting = false;
    mDestroyedModel = 0;
    mLowResModel = 0;
    mAttachedOdfs = 0;
    mNumAttachedOdfs = 0;
    mSwingAmount = 0.004f;
    mSwingSpeed = 0.0f;
}

// FUNCTION: SWBF2 0x004d9f40
EntityPropClass::~EntityPropClass()
{
    delete[] mAttachedOdfs;
}

// SYNTHETIC: SWBF2 0x004d9d40
// EntityPropClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004d9d70
EntityPropClass::EntityPropClass(EntityPropClass* pBase, unsigned int uHash)
    : EntityGeometryClass(pBase, uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    m_bDynamicLighting = pBase->m_bDynamicLighting;
    mDestroyedModel = pBase->mDestroyedModel;
    mLowResModel = pBase->mLowResModel;
    mNumAttachedOdfs = pBase->mNumAttachedOdfs;
    if (mNumAttachedOdfs > 0)
    {
        mAttachedOdfs = new AttachedOdfData[16];
        memcpy(mAttachedOdfs, pBase->mAttachedOdfs, mNumAttachedOdfs * sizeof(AttachedOdfData));
    }
    else
    {
        mAttachedOdfs = 0;
    }
    mGeometryColorMin = pBase->mGeometryColorMin;
    mGeometryColorMax = pBase->mGeometryColorMax;
    mUseHSVFlag = pBase->mUseHSVFlag;
    mSwingAmount = pBase->mSwingAmount;
    mSwingSpeed = pBase->mSwingSpeed;
    mReflected = pBase->mReflected;
    FoleyFXCollideeClass::mGroup = pBase->GetFoleyFXGroup();
    FoleyFXColliderClass::mClassID = pBase->GetFoleyFXClassID();
    mAttachedSoundOnlyMoving = pBase->mAttachedSoundOnlyMoving;
    mAttachedSound = pBase->mAttachedSound;
    mStartMovementSound = pBase->mStartMovementSound;
    mStopMovementSound = pBase->mStopMovementSound;
    mSoundOffset = pBase->mSoundOffset;
}

// FUNCTION: SWBF2 0x004d9fa0
void EntityPropClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x0df468ca:
        mLowResModel = RedModel::_HashTable.Find(PblHash(pszValue));
        mModel->_mLod[3] = mLowResModel;
        break;

    case 0x1b2cc185:
        mStartMovementSound.SetID(pszValue, false, false);
        break;
    case 0x2f78e0b7:
        mStopMovementSound.SetID(pszValue, false, false);
        break;
    case 0xf350b4e5:
        mAttachedSound.SetID(pszValue, false, false);
        break;

    case 0x35b1cfc2:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        mGeometryColorMax.r = (unsigned char)r;
        mGeometryColorMax.g = (unsigned char)g;
        mGeometryColorMax.b = (unsigned char)b;
        mGeometryColorMax.a = (unsigned char)a;
        break;
    }
    case 0x3f9cc4c8:
    {
        int r, g, b, a;
        sscanf(pszValue, "%i %i %i %i", &r, &g, &b, &a);
        mGeometryColorMin.r = (unsigned char)r;
        mGeometryColorMin.g = (unsigned char)g;
        mGeometryColorMin.b = (unsigned char)b;
        mGeometryColorMin.a = (unsigned char)a;
        break;
    }

    case 0x3be7b80a:
        if (!sPendingAttachedOdfClass)
        {
            EntityGeometryClass::SetProperty(uProperty, pszValue);
        }
        else if (mNumAttachedOdfs < 16)
        {
            if (mNumAttachedOdfs == 0)
            {
                mAttachedOdfs = new AttachedOdfData[16];
            }
            mAttachedOdfs[mNumAttachedOdfs].mClass = sPendingAttachedOdfClass;
            mAttachedOdfs[mNumAttachedOdfs].mDistance = -1.0f;
            char acBone[260];
            sscanf(pszValue, "%s %f", acBone, &mAttachedOdfs[mNumAttachedOdfs].mDistance);
            if (mModel->_mLod[0]->GetBoneMatrix(PblTEMPHash(acBone), mAttachedOdfs[mNumAttachedOdfs].mMatrix))
            {
                ++mNumAttachedOdfs;
            }
        }
        break;

    case 0x47c86b4a:
    {
        mModel = const_cast<GameModel*>(GameModel::Find(PblHash(pszValue)));
        if (mModel && mModel->_mModel->_BoundingSphere.w > 10.0f)
        {
            mLODClass = 1;
        }

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

        const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
        if (pMesh)
        {
            mCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", _IdentityMatrix, 0));
        }

        sTerrainCollisionMask = 0;
        sStaticCollisionMask = 0;
        sRigidCollisionMask = 0;
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        sTargetableCollisionMask = 0;
        break;
    }

    case 0x5dc6e917:
        mDestroyedModel = RedModel::_HashTable.Find(PblHash(pszValue));
        if (mModel && mModel->_mModel->_BoundingSphere.w > 10.0f)
        {
            mLODClass = 1;
        }
        break;

    case 0x5dfdc07f:
    {
        unsigned int uName = PblHash(pszValue);
        if (uName == 0xada7afdb || uName == 0x5c6e1222)
        {
            sSoftCollisionMask = 0;
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
        }
        else
        {
            int iMaskId = mCollisionModel.GetMaskId(uName);
            if (iMaskId >= 0)
            {
                sSoftCollisionMask |= 1 << iMaskId;
                mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
            }
        }
        break;
    }

    case 0x6a6c7e0d:
        sPendingAttachedOdfClass = 0;
        EntityGeometryClass::SetProperty(uProperty, pszValue);
        break;

    case 0x6d8fe606:
        mCollisionModel.mDenyFlyerLand = atoi(pszValue) != 0;
        break;

    case 0x8123a629:
        mUseHSVFlag = true;
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

    case 0x862be6b4:
        sscanf(pszValue, "%f", &mSwingSpeed);
        break;
    case 0xbebced85:
        sscanf(pszValue, "%f", &mSwingAmount);
        break;

    case 0x9828f43f:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sStaticCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_STATIC, CollisionMask(sStaticCollisionMask));
        }
        break;
    }

    case 0xa9d0d48b:
    {
        sPendingAttachedOdfClass = 0;
        EntityClass* pClass = EntityClass::Find(PblHash(pszValue));
        if (pClass)
        {
            if (pClass->IsRtti(EntityLightClass::GetRtti()))
            {
                EntityGeometryClass::SetProperty(uProperty, pszValue);
            }
            else
            {
                sPendingAttachedOdfClass = pClass;
            }
        }
        break;
    }

    case 0xafe693ce:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sTerrainCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_TERRAIN, CollisionMask(sTerrainCollisionMask));
        }
        break;
    }

    case 0xba67f8d2:
        mAttachedSoundOnlyMoving = atol(pszValue) != 0;
        break;

    case 0xc3f25cbb:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sRigidCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_RIGID, CollisionMask(sRigidCollisionMask));
            CollisionMask softMask = mCollisionModel.GetCollisionMask(COLL_SOFT);
            CollisionMask ordnanceMask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);
            softMask.ClearId(iMaskId);
            ordnanceMask.ClearId(iMaskId);
            mCollisionModel.SetCollisionMask(COLL_SOFT, softMask);
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, ordnanceMask);
        }
        break;
    }

    case 0xcb00fa3b:
    {
        int iReflected = 0;
        sscanf(pszValue, "%i", &iReflected);
        mReflected = iReflected != 0;
        break;
    }

    case 0xd23dbaf8:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sTargetableCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_TARGETABLE, CollisionMask(sTargetableCollisionMask));
        }
        break;
    }

    case 0xd9d02e03:
        mCollisionModel.mDeathOnFlyerLand = atoi(pszValue) != 0;
        break;

    case 0xde5365a1:
    {
        unsigned int uName = PblHash(pszValue);
        if (uName == 0xada7afdb || uName == 0x5c6e1222)
        {
            sRigidCollisionMask = 0;
            mCollisionModel.SetCollisionMask(COLL_RIGID, CollisionMask(sRigidCollisionMask));
        }
        else
        {
            int iMaskId = mCollisionModel.GetMaskId(uName);
            if (iMaskId >= 0)
            {
                sRigidCollisionMask |= 1 << iMaskId;
                mCollisionModel.SetCollisionMask(COLL_RIGID, CollisionMask(sRigidCollisionMask));
            }
        }
        break;
    }

    case 0xe6dedb8f:
    {
        float x = 0.0f, y = 0.0f, z = 0.0f;
        sscanf(pszValue, "%f %f %f", &x, &y, &z);
        mSoundOffset.x = x;
        mSoundOffset.y = y;
        mSoundOffset.z = z;
        break;
    }

    case 0xf31d1478:
        mCollisionModel.mUseVehicleCollisionAgainstFlyers = true;
        break;

    case 0xfb2bdf07:
    {
        unsigned int uName = PblHash(pszValue);
        if (uName == 0xada7afdb || uName == 0x5c6e1222)
        {
            sOrdnanceCollisionMask = 0;
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        else
        {
            int iMaskId = mCollisionModel.GetMaskId(uName);
            if (iMaskId >= 0)
            {
                sOrdnanceCollisionMask |= 1 << iMaskId;
                mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
            }
        }
        break;
    }

    case 0x07d00083:
    case 0x330acaab:
    case 0x7cdccf2a:
    case 0x8cf29543:
    case 0xfec8cea9:
    {
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
        switch (PblTEMPHash(acType))
        {
        case 0x1ccafdcc:
            primitive.SetBox(a, b, c, -1.0f);
            break;
        case 0x4a182b8f:
            primitive.SetCylinder(a, b);
            break;
        case 0x73f51491:
            primitive.SetAACylinder(a, b);
            break;
        default:
            primitive.SetSphere(a);
            break;
        }

        CollisionMask terrainMask = mCollisionModel.GetCollisionMask(COLL_TERRAIN);
        CollisionMask staticMask = mCollisionModel.GetCollisionMask(COLL_STATIC);
        CollisionMask rigidMask = mCollisionModel.GetCollisionMask(COLL_RIGID);
        CollisionMask softMask = mCollisionModel.GetCollisionMask(COLL_SOFT);
        CollisionMask ordnanceMask = mCollisionModel.GetCollisionMask(COLL_ORDNANCE);

        mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, acName, matrix, PblTEMPHash(acParent)));

        int iMaskId = mCollisionModel.GetMaskId(PblHash(acName));
        if (iMaskId >= 0)
        {
            switch (uProperty)
            {
            case 0x7cdccf2a: terrainMask.SetId(iMaskId); break;
            case 0x07d00083: softMask.SetId(iMaskId); break;
            case 0x330acaab: ordnanceMask.SetId(iMaskId); break;
            case 0x8cf29543: staticMask.SetId(iMaskId); break;
            case 0xfec8cea9: rigidMask.SetId(iMaskId); break;
            }
            mCollisionModel.SetCollisionMask(COLL_TERRAIN, terrainMask);
            mCollisionModel.SetCollisionMask(COLL_STATIC, staticMask);
            mCollisionModel.SetCollisionMask(COLL_RIGID, rigidMask);
            mCollisionModel.SetCollisionMask(COLL_SOFT, softMask);
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, ordnanceMask);
        }
        break;
    }

    default:
        if (!FoleyFXCollideeClass::SetProperty(uProperty, pszValue) &&
            !FoleyFXColliderClass::SetProperty(uProperty, pszValue))
        {
            EntityGeometryClass::SetProperty(uProperty, pszValue);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x004daff0
EntityClass* EntityPropClass::Derive(unsigned int uHash)
{
    return new EntityPropClass(uHash);
}

// FUNCTION: SWBF2 0x004db050
Entity* EntityPropClass::Build(const EntityDesc& desc)
{
    if (!mModel)
    {
        return 0;
    }

    EntityProp* pProp = new EntityProp(this, desc);
    if (pProp)
    {
        static_cast<CollisionObject*>(pProp)->AddStaticToCollisionManager();
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            for (int i = 0; i < mNumAttachedOdfs; ++i)
            {
                EntityDesc attachDesc;
                attachDesc.mMatrix = _IdentityMatrix;
                PblMatrix matrix;
                D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (const D3DXMATRIX*)&mAttachedOdfs[i].mMatrix,
                                   (const D3DXMATRIX*)&desc.mMatrix);
                attachDesc.mMatrix = matrix;
                Entity* pEntity = mAttachedOdfs[i].mClass->Build(attachDesc);
                if (pEntity && mAttachedOdfs[i].mDistance > -0.0001f)
                {
                    char acDistance[20];
                    _snprintf(acDistance, 19, "%f", mAttachedOdfs[i].mDistance);
                    acDistance[19] = 0;
                    pEntity->SetProperty(0xe6915cf6, acDistance);
                }
            }
            ((EntityGeometry*)pProp)->BuildEffects();
        }
    }
    return (Entity*)pProp;
}
