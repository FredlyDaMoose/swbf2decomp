#include "Chunk.h"
#include "PebbleFL/Common/PblVector.h"

#include <d3dx9math.h>

#include "CollisionManager.h"
#include "EntityGeometry.h"
#include "FLEffect.h"
#include "GameModel.h"
#include "GameObject.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

static const float DYNAMIC_LIFETIME = 8.0f;
static const float STATIC_LIFETIME = 53.0f;
static const float EPSILON = 0.0001f;

static inline const PblMatrix operator*(const PblMatrix& a, const PblMatrix& b)
{
    PblMatrix result;
    D3DXMatrixMultiply((D3DXMATRIX*)&result, (const D3DXMATRIX*)&a, (const D3DXMATRIX*)&b);
    return result;
}

static inline const PblVector3 TransformCoord(const PblVector3& v, const PblMatrix& m)
{
    PblVector3 result;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&result, (const D3DXVECTOR3*)&v, (const D3DXMATRIX*)&m);
    return result;
}

// FUNCTION: SWBF2 0x00454e30
void Chunk::SetPosition(const PblVector3& vPos)
{
    mMatrix.m[3][0] = vPos.x;
    mMatrix.m[3][1] = vPos.y;
    mMatrix.m[3][2] = vPos.z;
    mMatrix.m[3][3] = 1.0f;
    mCollisionMatrix = mPrimitiveMatrix.GetInverseFast() * mMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
}

// FUNCTION: SWBF2 0x00454f10
const PblVector3& Chunk::GetPosition() const
{
    return mMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x00454f20
const PblMatrix& Chunk::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00454f30
const PblVector3& Chunk::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00454f40
const PblVector3& Chunk::GetOmega() const
{
    return mOmega;
}

// FUNCTION: SWBF2 0x00454f50
bool Chunk::IsAChunk() const
{
    return true;
}

// FUNCTION: SWBF2 0x00454f60
Chunk::Chunk()
    : CollisionObject(COLL_UNKNOWN, NULL, NULL, NULL),
      mIsValid(false),
      mIsMoving(false),
      mMatrix(_IdentityMatrix),
      mModel(NULL),
      mMaxTerrainCollisions(0),
      mNumTerrainCollisions(0)
{
    SetCollisionModel(&mSimpleCollisionModel);
    mTotalUpdateDt = 0.0f;
    mJustRendered = false;
}

// SYNTHETIC: SWBF2 0x00455080
// Chunk::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004550c0
Chunk::~Chunk()
{
}

// FUNCTION: SWBF2 0x00455150
void Chunk::ClearAll()
{
    mIsValid = false;
    mIsMoving = false;
    mMaxTerrainCollisions = 0;
    mNumTerrainCollisions = 0;
    mMatrix = _IdentityMatrix;
    SetCollisionRadius(1.0f);
    Set2DCollisionRadius(1.0f);
    mModel = NULL;
    RemoveFromCollisionManager();
    mSimpleCollisionModel.ClearAll(false);
    mFullCollisionModel = NULL;
    SetCollisionModel(&mSimpleCollisionModel);

    mAngularMomentum.z = 0.0f;
    mAngularMomentum.y = 0.0f;
    mAngularMomentum.x = 0.0f;
    mOmega.z = 0.0f;
    mOmega.y = 0.0f;
    mOmega.x = 0.0f;
    mGravity = PblVector3(0.0f, -9.8f, 0.0f);
    mMaxTerrainCollisions = 0;
    mNumTerrainCollisions = 0;
    mTerrainImpactEffect = 0;
    for (int i = 0; i < 3; i++)
    {
        mTrailEffect[i] = 0;
    }
    for (int i = 0; i < 3; i++)
    {
        mSmokeEffect[i] = 0;
    }
    mPhysicsType = PHYSICS_SIMPLE_RIGID;

    for (int i = 0; i < 3; i++)
    {
        if (mTrailEmitters[i].mObject)
        {
            FLEffectObject* pEffect = mTrailEmitters[i];
            if (pEffect)
            {
                pEffect->StopAndFinish();
            }
            mTrailEmitters[i] = NULL;
        }
        if (mSmokeEmitters[i].mObject)
        {
            FLEffectObject* pEffect = mSmokeEmitters[i];
            if (pEffect)
            {
                pEffect->StopAndFinish();
            }
            mSmokeEmitters[i] = NULL;
        }
    }
    mTotalUpdateDt = 0.0f;
    mJustRendered = false;
}

// FUNCTION: SWBF2 0x00455300
void Chunk::FixPosition()
{
    mLifeTime = 0.0f;
    CreateSmokeEmitters();
    mIsMoving = false;
    if (RedTerrain::_bEnable)
    {
        AddStaticToCollisionManager();
    }
    if (mFullCollisionModel)
    {
        SetCollisionModel(mFullCollisionModel);
    }
}

// FUNCTION: SWBF2 0x00455340
void Chunk::SetData(ChunkDescriptor& desc)
{
    RedPose pose;

    bCollisionOnUpdate = false;
    mIsValid = true;
    mIsMoving = true;
    mLifeTime = 0.0f;
    mMatrix = desc.mMatrix;

    float fRadius = desc.mPrimitive->GetRadius();
    SetCollisionRadius(fRadius);
    Set2DCollisionRadius(fRadius);

    mModel = desc.mModel;
    mModel->GetRedModel(0)->SetupPose(pose);
    mVelocity = desc.mVelocity;

    mBodyPrimitive.Init(desc.mPrimitive->mPrimitive, "", desc.mPrimitive->mMatrix,
                        desc.mPrimitive->mParentHashName);
    mSimpleCollisionModel.AddBody(&mBodyPrimitive);
    mFullCollisionModel = desc.mFullCollisionModel;

    mNumTerrainCollisions = 0;
    mMaxTerrainCollisions = desc.mMaxTerrainCollisions;
    mTerrainImpactEffect = desc.mTerrainImpactEffect;
    for (int i = 0; i < 3; i++)
    {
        mTrailEffect[i] = desc.mTrailEffect[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mSmokeEffect[i] = desc.mSmokeEffect[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mTrailAttachPoints[i] = desc.mTrailAttachPoints[i];
    }
    for (int i = 0; i < 3; i++)
    {
        mSmokeAttachPoints[i] = desc.mSmokeAttachPoints[i];
    }
    mPhysicsType = (ChunkPhysicsType)desc.mPhysicsType;
    mCoefficientOfRestitution = desc.mCoefficientOfRestitution;
    mCoefficientOfFriction = desc.mCoefficientOfFriction;
    mSimpleFrictionConstant = desc.mSimpleFrictionConstant;
    mMinDeathCollisionSpeed = desc.mMinDeathCollisionSpeed;
    mForceZeroYAxisVelocity = desc.mForceZeroYAxisVelocity;
    mMaxStaticLifetime = desc.mMaxStaticLifetime;
    mGravity = desc.mGravity;

    CreateTrailEmitters();

    mMass = 40.0f;
    mOneOverMass = 1.0f / 40.0f;
    PblMatrix bodyInertia = desc.mPrimitive->GetMomentOfInertia();
    mInverseBodyInertiaTensor = _IdentityMatrix;
    bodyInertia.m[0][0] *= 40.0f;
    bodyInertia.m[1][1] *= 40.0f;
    bodyInertia.m[2][2] *= 40.0f;
    if (bodyInertia.m[0][0] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[0][0] = 1.0f / bodyInertia.m[0][0];
    }
    if (bodyInertia.m[1][1] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[1][1] = 1.0f / bodyInertia.m[1][1];
    }
    if (bodyInertia.m[2][2] > 0.0f)
    {
        mInverseBodyInertiaTensor.m[2][2] = 1.0f / bodyInertia.m[2][2];
    }

    PblMatrix localMatrix = desc.mPrimitive->GetLocalMatrix(&pose);
    mCollisionMatrix = localMatrix * mMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
    RecalculateAABB();

    if (mPhysicsType == PHYSICS_STATIC)
    {
        if (mFullCollisionModel)
        {
            SetCollisionModel(mFullCollisionModel);
        }
        AddStaticToCollisionManager();
        mMaxTerrainCollisions = 0;
        mNumTerrainCollisions = 1;
        mIsMoving = false;
    }
    else if (RedTerrain::_bEnable || desc.mKeepSoldierCollisions)
    {
        AddSoftToCollisionManager();
    }

    PblMatrix rotation = mCollisionMatrix;
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    mInverseWorldInertiaTensor = rotation.GetInverseFast() * mInverseBodyInertiaTensor * rotation;
    mAngularMomentum = TransformCoord(desc.mOmega, bodyInertia * rotation);
    mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
    mPrimitiveMatrix = localMatrix.GetInverseFast();

    if (!desc.mKeepSoldierCollisions)
    {
        mSimpleCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(0));
    }
    if (mFullCollisionModel)
    {
        mFullCollisionModel->SetCollisionMask(COLL_SOFT, CollisionMask(0));
    }
    if (desc.mNoBuildingCollisions)
    {
        mSimpleCollisionModel.SetCollisionMask(COLL_STATIC, CollisionMask(0));
    }

    if (mPhysicsType == PHYSICS_COCKPIT)
    {
        const PblMatrix& m = mCollisionMatrix;
        float fX = m.m[0][0] * mVelocity.x + m.m[0][1] * mVelocity.y + m.m[0][2] * mVelocity.z;
        float fY = m.m[1][0] * mVelocity.x + m.m[1][1] * mVelocity.y + m.m[1][2] * mVelocity.z;
        float fZ = m.m[2][0] * mVelocity.x + m.m[2][1] * mVelocity.y + m.m[2][2] * mVelocity.z;
        if (fX > fY)
        {
            mForwardAxis = (fX > fZ) ? 0 : 2;
        }
        else
        {
            mForwardAxis = (fY <= fZ) ? 2 : 1;
        }
    }
    mLinearDamping = desc.mLinearDamping;
    mAngularDamping = desc.mAngularDamping;
}

// FUNCTION: SWBF2 0x00455a90
void Chunk::CreateTrailEmitters()
{
    for (int i = 0; i < 3; i++)
    {
        if (!mTrailEffect[i])
        {
            continue;
        }

        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mTrailEffect[i]);
        if (!pClass)
        {
            continue;
        }
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (!pEffect)
        {
            continue;
        }

        const PblMatrix* pAttach = NULL;
        if (mTrailAttachPoints[i])
        {
            RedPose pose;
            mModel->GetRedModel(0)->SetupPose(pose);
            pAttach = pose._Table.Find(mTrailAttachPoints[i]);
            if (pAttach)
            {
                pEffect->AttachEffectToMatrix(&mMatrix, *pAttach, true);
            }
        }
        if (!pAttach)
        {
            pEffect->AttachEffectToMatrix(&mCollisionMatrix, true);
        }
        mTrailEmitters[i] = pEffect;
    }
}

// FUNCTION: SWBF2 0x00455b90
void Chunk::CreateSmokeEmitters()
{
    for (int i = 0; i < 3; i++)
    {
        if (!mSmokeEffect[i])
        {
            continue;
        }

        FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mSmokeEffect[i]);
        if (!pClass)
        {
            continue;
        }
        FLEffectObject* pEffect = pClass->BuildEffect();
        if (!pEffect)
        {
            continue;
        }

        const PblMatrix* pAttach = NULL;
        if (mSmokeAttachPoints[i])
        {
            RedPose pose;
            mModel->GetRedModel(0)->SetupPose(pose);
            pAttach = pose._Table.Find(mSmokeAttachPoints[i]);
            if (pAttach)
            {
                pEffect->AttachEffectToMatrix(&mMatrix, *pAttach, true);
            }
        }
        if (!pAttach)
        {
            pEffect->AttachEffectToMatrix(&mMatrix, true);
        }
        mSmokeEmitters[i] = pEffect;
    }
}

static inline const PblVector3 Cross(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

static inline const PblVector3 Normalize(const PblVector3& v)
{
    PblVector3 result;
    D3DXVec3Normalize((D3DXVECTOR3*)&result, (const D3DXVECTOR3*)&v);
    return result;
}

static inline void SetAxis(PblMatrix& m, int iRow, const PblVector3& v)
{
    m.m[iRow][0] = v.x;
    m.m[iRow][1] = v.y;
    m.m[iRow][2] = v.z;
    m.m[iRow][3] = 0.0f;
}

static inline PblQuaternion PblQuaternion_FromAxisAngle(const PblVector3& vAxis, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle * 0.5f);
    PblQuaternion q;
    q._fX = vAxis.x * fSin;
    q._fY = vAxis.y * fSin;
    q._fZ = vAxis.z * fSin;
    q._fS = fCos;
    return q;
}

static inline PblMatrix MatrixFromQuat(const PblQuaternion& q, const PblVector3& vPos)
{
    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    return mNew;
}

// FUNCTION: SWBF2 0x00455c90
void Chunk::Update(float fDeltaTime)
{
    if (!mIsValid)
    {
        return;
    }

    if (!mIsMoving || !RedTerrain::_bEnable || mMaxTerrainCollisions == 255)
    {
        float fMaxLifeTime =
            (mPhysicsType == PHYSICS_STATIC || mPhysicsType == PHYSICS_COCKPIT) ? STATIC_LIFETIME : DYNAMIC_LIFETIME;
        if (mMaxStaticLifetime >= 0.0f)
        {
            fMaxLifeTime = mMaxStaticLifetime;
        }
        if (mMaxTerrainCollisions == 255)
        {
            fMaxLifeTime = 30.0f;
        }
        if (mLifeTime < fMaxLifeTime)
        {
            mLifeTime += fDeltaTime;
            if (mLifeTime >= fMaxLifeTime)
            {
                ClearAll();
            }
        }
        if (!mIsMoving)
        {
            return;
        }
    }

    if (mJustRendered)
    {
        mTotalUpdateDt = fDeltaTime;
        mJustRendered = false;
    }
    else
    {
        mTotalUpdateDt += fDeltaTime;
    }

    bCollisionOnUpdate = false;
    PblVector3 vPos = mCollisionMatrix.GetTranslation();
    PblVector3 vForce(mGravity.x * mMass + 0.0f - mVelocity.x * mLinearDamping,
                      mGravity.y * mMass + 0.0f - mVelocity.y * mLinearDamping,
                      mGravity.z * mMass + 0.0f - mVelocity.z * mLinearDamping);

    if (mPhysicsType == PHYSICS_COCKPIT)
    {

        PblVector3 vForward = Normalize(mVelocity);
        switch (mForwardAxis)
        {
        case 0:
        {
            PblVector3 vZ = Normalize(Cross(vForward, mCollisionMatrix.GetYAxis()));
            PblVector3 vY = Cross(vZ, vForward);
            SetAxis(mCollisionMatrix, 0, vForward);
            SetAxis(mCollisionMatrix, 1, vY);
            SetAxis(mCollisionMatrix, 2, vZ);
            break;
        }
        case 1:
        {
            PblVector3 vX = Normalize(Cross(vForward, mCollisionMatrix.GetZAxis()));
            PblVector3 vZ = Cross(vX, vForward);
            SetAxis(mCollisionMatrix, 0, vX);
            SetAxis(mCollisionMatrix, 1, vForward);
            SetAxis(mCollisionMatrix, 2, vZ);
            break;
        }
        case 2:
        {
            PblVector3 vY = Normalize(Cross(vForward, mCollisionMatrix.GetXAxis()));
            PblVector3 vX = Cross(vY, vForward);
            SetAxis(mCollisionMatrix, 0, vX);
            SetAxis(mCollisionMatrix, 1, vY);
            SetAxis(mCollisionMatrix, 2, vForward);
            break;
        }
        }
    }
    else if (mPhysicsType != PHYSICS_SIMPLE_RIGID)
    {
        float fDamping = -mAngularDamping;
        mAngularMomentum.x += fDamping * mAngularMomentum.x * fDeltaTime;
        mAngularMomentum.y += mAngularMomentum.y * fDamping * fDeltaTime;
        mAngularMomentum.z += mAngularMomentum.z * fDamping * fDeltaTime;
        mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
    }

    PblQuaternion q = PblQuaternion(mCollisionMatrix);
    PblMatrix mInverse = mCollisionMatrix.GetInverseFast();
    PblVector3 vSpin;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vSpin, (const D3DXVECTOR3*)&mOmega, (const D3DXMATRIX*)&mInverse);
    vSpin.x *= fDeltaTime;
    vSpin.y *= fDeltaTime;
    vSpin.z *= fDeltaTime;
    float fAngle = sqrtf(vSpin.x * vSpin.x + vSpin.y * vSpin.y + vSpin.z * vSpin.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion_FromAxisAngle(PblVector3(vSpin.x * fInv, vSpin.y * fInv, vSpin.z * fInv), fAngle);
    }

    mVelocity.x += mOneOverMass * vForce.x * fDeltaTime;
    mVelocity.y += mOneOverMass * vForce.y * fDeltaTime;
    mVelocity.z += mOneOverMass * vForce.z * fDeltaTime;
    if (mForceZeroYAxisVelocity)
    {
        const PblVector3& vUp = mMatrix.GetYAxis();
        float fDot = mVelocity.y * vUp.y + vUp.x * mVelocity.x + mVelocity.z * vUp.z;
        mVelocity.x -= fDot * vUp.x;
        mVelocity.y -= vUp.y * fDot;
        mVelocity.z -= vUp.z * fDot;
    }
    vPos.x += mVelocity.x * fDeltaTime;
    vPos.y += mVelocity.y * fDeltaTime;
    vPos.z += mVelocity.z * fDeltaTime;

    mCollisionMatrix = MatrixFromQuat(q, vPos);
    mMatrix = mPrimitiveMatrix * mCollisionMatrix;
    SetCollisionCenter(mCollisionMatrix.GetTranslation());
    RecalculateAABB();

    PblMatrix rotation = mCollisionMatrix;
    rotation.m[3][0] = 0.0f;
    rotation.m[3][1] = 0.0f;
    rotation.m[3][2] = 0.0f;
    rotation.m[3][3] = 1.0f;
    mInverseWorldInertiaTensor = rotation.GetInverseFast() * mInverseBodyInertiaTensor * rotation;

    CollisionObject* pHit = NULL;
    PblVector3 vNormal;
    if (mNumTerrainCollisions > mMaxTerrainCollisions)
    {
        float fDist = CollisionManager::RayHit(vPos, PblVector3(0.0f, -1.0f, 0.0f), GetCollisionRadius() * 1.001f,
                                               pHit, &vNormal, NULL, 0, 0x90, false);
        if ((pHit && fDist < (mPhysicsType == PHYSICS_COCKPIT ? 0.3f : 0.75f)) || mPhysicsType == PHYSICS_STATIC ||
            mNumTerrainCollisions > mMaxTerrainCollisions + 5)
        {
            RemoveFromCollisionManager();
            EntityGeometryClass* pClass = mParentCluster->mParentClass;
            if (pClass && NetGame::IsEnabled() && !NetGame::IsClient())
            {
                NetGame::CreateEvent_CorrectChunkCluster(mParentCluster, pClass, mID, (unsigned __int64)-1);
            }
            FixPosition();
            if (!mParentCluster->mParentClass)
            {
                if (mMaxStaticLifetime >= 0.0f)
                {
                    mLifeTime = mMaxStaticLifetime - 5.0f;
                }
                mLifeTime = 3.0f;
            }
        }
    }

    if (RedTerrain::_bEnable)
    {
        float fHeight = RedTerrain::GetHoleHeight(vPos.x, vPos.z);
        if (fHeight > vPos.y + GetCollisionRadius())
        {
            pHit = NULL;
            CollisionManager::RayHit(vPos, PblVector3(0.0f, -1.0f, 0.0f), 256.0f, pHit, &vNormal, NULL, 0, 0x10,
                                     false);
            if (!pHit)
            {
                ClearAll();
            }
        }
    }
}

// FUNCTION: SWBF2 0x00456960
void Chunk::DoTerrainImpactEffect(const PblVector3& vPos)
{
    FLEffectClass* pClass = FLEffect::s_EffectClasses.Find(mTerrainImpactEffect);
    if (!pClass)
    {
        return;
    }
    FLEffectObject* pEffect = pClass->BuildEffect();
    if (pEffect)
    {
        pEffect->AttachEffectToVector(&vPos, false);
    }
}

PblVector3* __cdecl CrossProduct(PblVector3* pResult, const PblVector3* pA, const PblVector3* pB);

// FUNCTION: SWBF2 0x004569a0
bool Chunk::DoSimpleRigidCollision(const CollisionResult* pResult, CollisionObject* pOther,
                                   Restrictor* pRestrictor)
{
    const CollisionObjectType eType = pOther->GetCollisionType();
    if (!mIsMoving)
    {
        return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    }

    if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType == COLL_TERRAIN)
    {
        if (mNumTerrainCollisions == mMaxTerrainCollisions)
        {
            PblVector3 vImpact = pResult->mCollisionPosition;
            vImpact.y = RedTerrain::GetHoleHeight(vImpact.x, vImpact.z);
            DoTerrainImpactEffect(vImpact);
            ++mNumTerrainCollisions;
        }
        return true;
    }

    const PblVector3& vNormal = pResult->mSeparationNormal;
    const float fImpactSpeed = -(mVelocity.x * vNormal.x + mVelocity.y * vNormal.y +
                                 mVelocity.z * vNormal.z);
    if (fImpactSpeed > 0.0f)
    {
        FoleyFXCollider::CollisionCallback(eType, static_cast<FoleyFXCollidee*>(pOther), &GetPosition());
        PlayImpactEffects(&mMatrix.GetTranslation(), &mVelocity);

        if ((eType & (COLL_SOFT | COLL_RIGID)) && mMinDeathCollisionSpeed > 0.0f &&
            mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y + mVelocity.z * mVelocity.z >
                mMinDeathCollisionSpeed * mMinDeathCollisionSpeed)
        {
            GameObject* pObject = pOther->GetGameObject();
            if (pObject)
            {
                DamageOwner owner(0, 0, 0, 0, 0, 1.0f);
                pObject->InstantDeath(owner, true);
                return true;
            }
        }

        const float fImpulse = (mCoefficientOfRestitution + 1.0f) * fImpactSpeed;
        mVelocity.x += vNormal.x * fImpulse;
        mVelocity.y += vNormal.y * fImpulse;
        mVelocity.z += vNormal.z * fImpulse;

        if (mSimpleFrictionConstant > 0.0f)
        {

            PblVector3 vTangent(
                vNormal.y * (mVelocity.x * vNormal.y - mVelocity.y * vNormal.x) -
                    vNormal.z * (mVelocity.z * vNormal.x - mVelocity.x * vNormal.z),
                vNormal.z * (mVelocity.y * vNormal.z - mVelocity.z * vNormal.y) -
                    vNormal.x * (mVelocity.x * vNormal.y - mVelocity.y * vNormal.x),
                vNormal.x * (mVelocity.z * vNormal.x - mVelocity.x * vNormal.z) -
                    vNormal.y * (mVelocity.y * vNormal.z - mVelocity.z * vNormal.y));
            D3DXVec3Normalize((D3DXVECTOR3*)&vTangent, (const D3DXVECTOR3*)&vTangent);
            const float fSlip = (mVelocity.x * vTangent.x + mVelocity.y * vTangent.y +
                                 mVelocity.z * vTangent.z) * mSimpleFrictionConstant * mOneOverMass;
            mVelocity.x -= vTangent.x * fSlip;
            mVelocity.y -= vTangent.y * fSlip;
            mVelocity.z -= vTangent.z * fSlip;

            const PblVector3& vAxis = mCollisionMatrix.GetYAxis();
            const PblVector3& vCenter = mCollisionMatrix.GetTranslation();
            PblVector3 vRadius(pResult->mCollisionPosition.x - vCenter.x,
                               pResult->mCollisionPosition.y - vCenter.y,
                               pResult->mCollisionPosition.z - vCenter.z);
            const float fAlongAxis = vRadius.x * vAxis.x + vRadius.y * vAxis.y + vRadius.z * vAxis.z;
            vRadius.x -= vAxis.x * fAlongAxis;
            vRadius.y -= vAxis.y * fAlongAxis;
            vRadius.z -= vAxis.z * fAlongAxis;
            const float fRadius = sqrtf(vRadius.x * vRadius.x + vRadius.y * vRadius.y + vRadius.z * vRadius.z);
            if (fRadius > 0.0f)
            {
                float fOmega = sqrtf(mVelocity.x * mVelocity.x + mVelocity.y * mVelocity.y +
                                      mVelocity.z * mVelocity.z) / fRadius;
                if (vAxis.z * mVelocity.x - vAxis.x * mVelocity.z > 0.0f)
                {
                    fOmega = -fOmega;
                }
                mOmega.x = vAxis.x * fOmega;
                mOmega.y = vAxis.y * fOmega;
                mOmega.z = vAxis.z * fOmega;
            }
        }
    }

    if (eType == COLL_TERRAIN || vNormal.y > 0.8f)
    {
        ++mNumTerrainCollisions;
    }
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x00456ef0
bool Chunk::DoFullRigidCollision(const CollisionResult* pResult, CollisionObject* pOther, Restrictor* pRestrictor)
{
    CollisionObjectType eType = pOther->GetCollisionType();
    if (eType == COLL_SOFT)
    {
        return true;
    }
    if ((eType == COLL_RIGID || eType == COLL_STATIC) && pOther->IsAChunk())
    {
        return true;
    }

    if (mIsMoving && !bCollisionOnUpdate)
    {
        bCollisionOnUpdate = true;
        PblVector3 vImpact = pResult->mCollisionPosition;
        vImpact.y = RedTerrain::GetHoleHeight(vImpact.x, vImpact.z);

        const PblVector3& vCenter = mCollisionMatrix.GetTranslation();
        PblVector3 vR(pResult->mCollisionPosition.x - vCenter.x, pResult->mCollisionPosition.y - vCenter.y,
                      pResult->mCollisionPosition.z - vCenter.z);
        PblVector3 vNormal = pResult->mSeparationNormal;

        PblVector3 vSpinVel = Cross(mOmega, vR);
        PblVector3 vVel(mVelocity.x + vSpinVel.x, mVelocity.y + vSpinVel.y, mVelocity.z + vSpinVel.z);
        float fDot = vNormal.y * vVel.y + vVel.x * vNormal.x + vNormal.z * vVel.z;
        PblVector3 vSlip = Normalize(PblVector3(vVel.x - vNormal.x * fDot, vVel.y - vNormal.y * fDot,
                                                vVel.z - vNormal.z * fDot));
        PblVector3 vFriction(-vSlip.x, -vSlip.y, -vSlip.z);

        if (fDot < 0.0f && (eType == COLL_TERRAIN || eType == COLL_STATIC || eType == COLL_RIGID))
        {
            if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType != COLL_STATIC && eType != COLL_RIGID)
            {
                if (mNumTerrainCollisions == mMaxTerrainCollisions)
                {
                    if (eType == COLL_TERRAIN)
                    {
                        DoTerrainImpactEffect(vImpact);
                    }
                    mNumTerrainCollisions++;
                }
                return true;
            }

            float fNum = -((mCoefficientOfRestitution + 1.0f) * fDot);
            PblVector3 vA = TransformCoord(Cross(vR, vNormal), mInverseWorldInertiaTensor);
            PblVector3 vAxR = Cross(vA, vR);
            float fJ = fNum / (vAxR.x * vNormal.x + vNormal.y * vAxR.y + vNormal.z * vAxR.z + mOneOverMass);
            PblVector3 vJ(vNormal.x * fJ, vNormal.y * fJ, vNormal.z * fJ);
            mVelocity.z += vJ.z * mOneOverMass;
            mVelocity.x += vJ.x * mOneOverMass;
            mVelocity.y += vJ.y * mOneOverMass;
            PblVector3 vL = Cross(vR, vJ);
            mAngularMomentum.y += vL.y;
            mAngularMomentum.x += vL.x;
            mAngularMomentum.z += vL.z;
            mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);

            PblVector3 vNewSpinVel;
            const PblVector3& vW = *CrossProduct(&vNewSpinVel, &mOmega, &vR);
            float fSlip = (mVelocity.y + vW.y) * vFriction.y + (mVelocity.x + vW.x) * vFriction.x +
                          (mVelocity.z + vW.z) * vFriction.z;
            if (fSlip < 0.0f && (eType == COLL_TERRAIN || eType == COLL_STATIC || eType == COLL_RIGID))
            {
                if (mNumTerrainCollisions >= mMaxTerrainCollisions && eType != COLL_STATIC && eType != COLL_RIGID)
                {
                    if (mNumTerrainCollisions == mMaxTerrainCollisions)
                    {
                        if (eType == COLL_TERRAIN)
                        {
                            DoTerrainImpactEffect(vImpact);
                        }
                        mNumTerrainCollisions++;
                    }
                    return true;
                }

                float fFNum = -(mCoefficientOfFriction * fSlip);
                PblVector3 vB = TransformCoord(Cross(vR, vFriction), mInverseWorldInertiaTensor);
                PblVector3 vBxR = Cross(vB, vR);
                float fK = fFNum / (vBxR.x * vFriction.x + vFriction.y * vBxR.y + vFriction.z * vBxR.z + mOneOverMass);
                PblVector3 vF(vFriction.x * fK, vFriction.y * fK, vFriction.z * fK);
                PblVector3 vLF = Cross(vR, vF);
                mAngularMomentum.x += vLF.x;
                mAngularMomentum.y += vLF.y;
                mAngularMomentum.z += vLF.z;
                mOmega = TransformCoord(mAngularMomentum, mInverseWorldInertiaTensor);
            }
        }
    }
    mNumTerrainCollisions++;
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x00457570
bool Chunk::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther, Restrictor* pRestrictor)
{
    switch (mPhysicsType)
    {
    case PHYSICS_FULL_RIGID:
        return DoFullRigidCollision(pResult, pOther, pRestrictor);
    case PHYSICS_SIMPLE_RIGID:
    case PHYSICS_COCKPIT:
        return DoSimpleRigidCollision(pResult, pOther, pRestrictor);
    case PHYSICS_STATIC:
        return false;
    default:
        return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
    }
}

extern bool netFrameLock;

// FUNCTION: SWBF2 0x004575c0
void Chunk::Render(unsigned char ucAlpha, float fDist, unsigned int uiFlags)
{
    float fMaxLifeTime =
        (mPhysicsType == PHYSICS_STATIC || mPhysicsType == PHYSICS_COCKPIT) ? STATIC_LIFETIME : DYNAMIC_LIFETIME;
    if (mMaxStaticLifetime >= 0.0f)
    {
        fMaxLifeTime = mMaxStaticLifetime;
    }
    if (mMaxTerrainCollisions == 255)
    {
        fMaxLifeTime = 30.0f;
    }
    if (mLifeTime > fMaxLifeTime - 2.0f)
    {
        fDist *= (fMaxLifeTime - mLifeTime) * 0.5f;
    }

    RedColor color;
    color.b = 0xff;
    color.g = 0xff;
    color.r = 0xff;
    color.a = (unsigned char)(fDist * 255.0f);

    RedPose pose;
    mModel->GetRedModel(0)->SetupPose(pose);
    PblMatrix mRender = mMatrix;

    if (mIsMoving && NetGame::IsEnabled() && !netFrameLock)
    {
        mJustRendered = true;
        float fDt = (1.0f - NetGame::GetTurnRatio()) * mTotalUpdateDt;

        PblQuaternion q = PblQuaternion(mCollisionMatrix);
        PblVector3 vBack(-mOmega.x, -mOmega.y, -mOmega.z);
        PblMatrix mInverse = mCollisionMatrix.GetInverseFast();
        PblVector3 vSpin;
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vSpin, (const D3DXVECTOR3*)&vBack, (const D3DXMATRIX*)&mInverse);
        vSpin.x *= fDt;
        vSpin.y *= fDt;
        vSpin.z *= fDt;
        PblVector3 vPos = mCollisionMatrix.GetTranslation();
        float fAngle = sqrtf(vSpin.x * vSpin.x + vSpin.y * vSpin.y + vSpin.z * vSpin.z);
        if (fAngle > EPSILON)
        {
            float fInv = 1.0f / fAngle;
            q = q * PblQuaternion_FromAxisAngle(PblVector3(vSpin.x * fInv, vSpin.y * fInv, vSpin.z * fInv), fAngle);
        }
        vPos.x -= mVelocity.x * fDt;
        vPos.y -= mVelocity.y * fDt;
        vPos.z -= mVelocity.z * fDt;
        mRender = mPrimitiveMatrix * MatrixFromQuat(q, vPos);
    }

    mModel->GetRedModel(0)->Render(&mRender, &pose, color, uiFlags | 8, NULL);
}

extern NoLodData noloddata;

// FUNCTION: SWBF2 0x00457a30
ChunkCluster::ChunkCluster()
    : RedSceneObject(&noloddata)
{
    SetRenderRadius(1.0f);
    SetRenderPos(PblVector3(0.0f, 0.0f, 0.0f));
}

// SYNTHETIC: SWBF2 0x00457ad0
// ChunkCluster::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00457b00
ChunkCluster::~ChunkCluster()
{
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            mChunks[i].ClearAll();
        }
    }
}

// FUNCTION: SWBF2 0x00457b90
void ChunkCluster::RecalculateCenterAndRadius()
{
    PblVector3 vCenter(0.0f, 0.0f, 0.0f);
    int iCount = 0;
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            const PblVector3& vPos = mChunks[i].GetCollisionCenter();
            vCenter.x += vPos.x;
            vCenter.y += vPos.y;
            vCenter.z += vPos.z;
            iCount++;
        }
    }
    if (iCount <= 0)
    {
        return;
    }

    float fInv = 1.0f / iCount;
    vCenter.x *= fInv;
    vCenter.y *= fInv;
    vCenter.z *= fInv;

    float fRadius = 0.0f;
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            const PblVector3& vPos = mChunks[i].GetCollisionCenter();
            float fX = vPos.x - vCenter.x;
            float fY = vPos.y - vCenter.y;
            float fZ = vPos.z - vCenter.z;
            float fReach = sqrtf(fY * fY + fX * fX + fZ * fZ) + mChunks[i].GetCollisionRadius() + 1.0f;
            if (fReach > fRadius)
            {
                fRadius = fReach;
            }
        }
    }
    SetRenderRadius(fRadius);
    SetRenderPos(vCenter);
}

// FUNCTION: SWBF2 0x00457ff0
void ChunkCluster::AddChunk(ChunkDescriptor& desc)
{
    for (int i = 0; i < 10; i++)
    {
        if (!mChunks[i].IsValid())
        {
            mChunks[i].SetData(desc);
            RecalculateCenterAndRadius();
            return;
        }
    }
}

// FUNCTION: SWBF2 0x00458040
bool ChunkCluster::Update(float fDeltaTime)
{
    bool bMoved = false;
    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsMoving())
        {
            bMoved = true;
        }
        mChunks[i].Update(fDeltaTime);
    }
    if (bMoved)
    {
        RecalculateCenterAndRadius();
    }
    if (!IsValid())
    {
        mID = -1;
    }
    return true;
}

// FUNCTION: SWBF2 0x004580c0
void ChunkCluster::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblVector3& vCamDir = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    PblVector3 vNear(vCamDir.x * fNear, vCamDir.y * fNear, vCamDir.z * fNear);
    const PblVector3& vCamPos = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    float fX = _Sphere.x - vCamPos.x + vNear.x;
    float fY = _Sphere.y - vCamPos.y + vNear.y;
    float fZ = _Sphere.z - vCamPos.z + vNear.z;

    if (fY * fY + fX * fX + fZ * fZ < 800.0f && RedStencilShadow::GetShadowEnable())
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
            const PblVector3& vDir = pLight->m_vDirection;
            float fMinLength = _Sphere.w * -2.0f / vDir.y;
            uiFlags |= 0x10000;
            PblVector3 vStart(_Sphere.x, _Sphere.w + _Sphere.y, _Sphere.z);
            const GameObject* pIgnore = NULL;
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
        RedRenderer::SetStencilShadowLength(fLength);

        const PblMatrix& mView = pCamera->GetMatrix();
        float fDX = mView.m[3][0] - mView.m[2][0] * fNear - _Sphere.x;
        float fDY = mView.m[3][1] - mView.m[2][1] * fNear - _Sphere.y;
        float fDZ = mView.m[3][2] - mView.m[2][2] * fNear - _Sphere.z;
        float fReach = _Sphere.w + fLength + fWidth * 1.33f;
        RedRenderer::SetStencilShadowZFail(fReach * fReach > fDX * fDX + fDY * fDY + fDZ * fDZ);
    }

    for (int i = 0; i < 10; i++)
    {
        if (mChunks[i].IsValid())
        {
            mChunks[i].Render(ucViewport, fDist, uiFlags);
        }
    }
}

// FUNCTION: SWBF2 0x00458370
bool ChunkCluster::IsValid()
{
    bool bValid = false;
    for (int i = 0; i < 10; i++)
    {
        bValid |= mChunks[i].IsValid();
    }
    return bValid;
}

// FUNCTION: SWBF2 0x004583b0
bool ChunkCluster::HasStaticChunks()
{
    bool bStatic = false;
    for (int i = 0; i < 10; ++i)
    {
        if (mChunks[i].mIsValid)
        {
            bStatic |= mChunks[i].mPhysicsType == PHYSICS_STATIC;
        }
    }
    return bStatic;
}

// FUNCTION: SWBF2 0x00458340
void ChunkCluster::ClearChunks()
{
    RemoveFromScene();
    for (int i = 0; i < 10; ++i)
    {
        mChunks[i].ClearAll();
    }
}

// GLOBAL: SWBF2 0x01e30348
ChunkClusterManager* ChunkClusterManager::sInstance;
// GLOBAL: SWBF2 0x01e3034c
ChunkClusterManager* ChunkClusterManager::sClientInstance;
// GLOBAL: SWBF2 0x01e30350
ChunkClusterManager* ChunkClusterManager::sHostInstance;

// FUNCTION: SWBF2 0x004586a0
bool ChunkClusterManager::CreateClient(bool bDeactivate)
{
    if (NetGame::IsOneWorld())
    {
        sClientInstance = 0;
        sHostInstance->DeactivateThread();
        sHostInstance->ActivateThread(-10, PER_LOCAL_TURN);
    }
    else
    {
        sClientInstance = new ChunkClusterManager;
        if (bDeactivate)
        {
            sClientInstance->DeactivateThread();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00458750
bool ChunkClusterManager::DestroyClient()
{
    if (sClientInstance)
    {
        delete sClientInstance;
        sClientInstance = 0;
    }
    return true;
}

// FUNCTION: SWBF2 0x00458770
void ChunkClusterManager::EnterClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sClientInstance;
    }
}

// FUNCTION: SWBF2 0x00458790
void ChunkClusterManager::LeaveClient()
{
    if (!NetGame::IsOneWorld())
    {
        sInstance = sHostInstance;
    }
}

// FUNCTION: SWBF2 0x00458490
ChunkClusterManager::ChunkClusterManager()
    : mOldestChunkCluster(0)
{
    ActivateThread(-10, PER_LOCAL_TURN);
    for (int i = 0; i < 8; i++)
    {
        mChunkClusters[i].mID = -1;
        for (unsigned char j = 0; j < 10; j++)
        {
            mChunkClusters[i].mChunks[j].mID = j;
            mChunkClusters[i].mChunks[j].mParentCluster = &mChunkClusters[i];
        }
    }
}

// SYNTHETIC: SWBF2 0x00458540
// ChunkClusterManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00458570
ChunkClusterManager::~ChunkClusterManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x004585e0
bool ChunkClusterManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = sHostInstance = new ChunkClusterManager;
    return true;
}

// FUNCTION: SWBF2 0x00458660
bool ChunkClusterManager::Destroy()
{
    if (!sInstance)
    {
        return false;
    }
    delete sHostInstance;
    sHostInstance = 0;
    sInstance = 0;
    return true;
}

// FUNCTION: SWBF2 0x00458800
ChunkCluster* ChunkClusterManager::GetFreeCluster(bool bArg)
{
    for (int i = 0; i < 8; i++)
    {
        if (!sInstance->mChunkClusters[i].IsValid())
        {
            ChunkCluster* pCluster = &sInstance->mChunkClusters[i];
            pCluster->mID = bArg ? i : -1;
            pCluster->Activate();
            return &sInstance->mChunkClusters[i];
        }
    }

    int iOldest = sInstance->mOldestChunkCluster;
    int i;
    for (i = iOldest; i < 8; i++)
    {
        if (!sInstance->mChunkClusters[i].HasStaticChunks())
        {
            break;
        }
    }
    if (i == 8)
    {
        for (i = 0; i < iOldest; i++)
        {
            if (!sInstance->mChunkClusters[i].HasStaticChunks())
            {
                break;
            }
        }
    }
    sInstance->mChunkClusters[i].ClearChunks();
    if (++sInstance->mOldestChunkCluster >= 8)
    {
        sInstance->mOldestChunkCluster = 0;
    }
    ChunkCluster* pCluster = &sInstance->mChunkClusters[i];
    pCluster->mID = bArg ? i : -1;
    pCluster->Activate();
    return pCluster;
}

// FUNCTION: SWBF2 0x004587b0
ChunkCluster* ChunkClusterManager::GetClusterByID(unsigned char ucID, bool bArg)
{
    if (ucID >= 8)
    {
        return 0;
    }
    for (int i = 0; i < 8; ++i)
    {
        if (sInstance->mChunkClusters[i].mID == ucID)
        {
            return &sInstance->mChunkClusters[i];
        }
    }
    if (bArg)
    {
        ChunkCluster* pCluster = GetFreeCluster(true);
        if (pCluster)
        {
            pCluster->mID = ucID;
        }
        return pCluster;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00458920
bool ChunkClusterManager::Update(float fDeltaTime)
{
    for (int i = 0; i < 8; i++)
    {
        mChunkClusters[i].Update(fDeltaTime);
    }
    return true;
}
