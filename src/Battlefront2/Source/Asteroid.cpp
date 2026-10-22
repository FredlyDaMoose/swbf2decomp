#include <stdio.h>
#include "PebbleFL/Common/PblVector.h"

#include "Asteroid.h"
#include "CollisionBody.h"
#include "CollisionMesh.h"
#include "CollisionModel.h"
#include "CollisionObject.h"
#include "CollisionPrimitive.h"
#include "Explosion.h"
#include "GameModel.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"
#include "CameraManager.h"
#include <d3dx9math.h>
#include <math.h>

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x01e2fddc
unsigned int sSoftCollisionMask;
// GLOBAL: SWBF2 0x01e2fdd8
unsigned int sOrdnanceCollisionMask;

// GLOBAL: SWBF2 0x01eb024c
// SYNTHETIC: SWBF2 0x004022d0
// `dynamic initializer for 'AsteroidClass::rttiHashAsteroidClass''
PblHash AsteroidClass::rttiHashAsteroidClass("AsteroidClass");
char AsteroidClass::rttiNameAsteroidClass[] = "AsteroidClass";

// FUNCTION: SWBF2 0x004453d0
AsteroidClass::AsteroidClass(unsigned int uHash)
    : GameObjectClass(uHash)
{
    mLODClass = 1;

    mMinSpeed = 10.0f;
    mMaxSpeed = 30.0f;
    mMinOmega = 5.0f;
    mMaxOmega = 10.0f;
    mCollidable = 1;
    mNumChildren = 0;
    mExplosionBreak = 0;
    mExplosionDestruct = 0;
}

// SYNTHETIC: SWBF2 0x00445450
// AsteroidClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00445560
AsteroidClass::~AsteroidClass()
{
}

// FUNCTION: SWBF2 0x00445480
AsteroidClass::AsteroidClass(AsteroidClass* pBase, unsigned int uHash)
    : GameObjectClass(pBase, uHash)
{
    mMinSpeed = pBase->mMinSpeed;
    mMaxSpeed = pBase->mMaxSpeed;
    mMinOmega = pBase->mMinOmega;
    mMaxOmega = pBase->mMaxOmega;
    mCollidable = pBase->mCollidable;
    mNumChildren = pBase->mNumChildren;
    for (int i = 0; i < 10; ++i)
    {
        mChildren[i] = pBase->mChildren[i];
    }
    mExplosionBreak = pBase->mExplosionBreak;
    mExplosionDestruct = pBase->mExplosionDestruct;
    mLODClass = 1;
}

// FUNCTION: SWBF2 0x00444790
bool AsteroidClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashAsteroidClass || GameObjectClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004447d0
unsigned int AsteroidClass::GetDerivedRtti()
{
    return rttiHashAsteroidClass;
}

// FUNCTION: SWBF2 0x004447e0
const char* AsteroidClass::GetDerivedRttiName()
{
    return rttiNameAsteroidClass;
}

// FUNCTION: SWBF2 0x004441e0
const GameModel* EntityGeometryClass::GetModel()
{
    return mModel;
}

// FUNCTION: SWBF2 0x00444430
int GameObjectClass::GetSoldierType()
{
    return 5;
}

// FUNCTION: SWBF2 0x00444440
int GameObjectClass::GetVehicleType()
{
    return 9;
}

// FUNCTION: SWBF2 0x00444450
const void* GameObjectClass::GetControllableClass()
{
    return 0;
}

// FUNCTION: SWBF2 0x00444460
int GameObjectClass::GetWeaponCount()
{
    return 0;
}

// FUNCTION: SWBF2 0x00444470
const WeaponClass* GameObjectClass::GetWeaponClass(int iIndex)
{
    return 0;
}

// FUNCTION: SWBF2 0x00444480
float GameObjectClass::GetCollisionInflict()
{
    return 1.0f;
}

// FUNCTION: SWBF2 0x00444490
const DamageEffectClass* GameObjectClass::GetDamageEffectClass(int iIndex)
{
    return mDamageEffectClass[iIndex];
}

// FUNCTION: SWBF2 0x004444b0
float GameObjectClass::GetTimeRequiredToEject()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004444c0
float GameObjectClass::GetReboardTime()
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004447f0
const CollisionMesh* CollisionMesh::Find(unsigned int uHash)
{
    return _HashTable.Find(uHash);
}

// FUNCTION: SWBF2 0x00445570
void AsteroidClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x47c86b4a:
    {
        EntityGeometryClass::SetProperty(uProperty, pszValue);
        sSoftCollisionMask = 0;
        sOrdnanceCollisionMask = 0;
        int iNumBodies = mCollisionModel.GetNumBodies();
        const CollisionMesh* pMesh = CollisionMesh::Find(PblHash(pszValue));
        if (pMesh)
        {
            mCollisionModel.AddBody(new CollisionBodyMesh(pMesh, "CollisionMesh", _IdentityMatrix, 0));
            int iMaskId = mCollisionModel.GetMaskId(0x29173102);
            if (iMaskId >= 0)
            {
                sSoftCollisionMask |= 1 << iMaskId;
                sOrdnanceCollisionMask |= 1 << iMaskId;
            }
        }
        if (iNumBodies == 0 && mModel)
        {
            const PblBox& box = mModel->_mModel->_TotalBoundingBox;
            float fCenterX = (box._vMin.x + box._vMax.x) * 0.5f;
            float fCenterY = (box._vMin.y + box._vMax.y) * 0.5f;
            float fCenterZ = (box._vMin.z + box._vMax.z) * 0.5f;
            float fHalfX = (box._vMax.x - box._vMin.x) * 0.5f;
            float fHalfY = (box._vMax.y - box._vMin.y) * 0.5f;
            float fHalfZ = (box._vMax.z - box._vMin.z) * 0.5f;
            CollisionPrimitive primitive;
            PblMatrix matrix = _IdentityMatrix;
            primitive.SetAACylinder((fHalfZ + fHalfX) * 0.5f, fHalfY * 2.0f);
            matrix.m[3][0] = fCenterX;
            matrix.m[3][1] = fCenterY;
            matrix.m[3][2] = fCenterZ;
            matrix.m[3][3] = 1.0f;
            mCollisionModel.AddBody(new CollisionBodyPrimitive(primitive, "main_body", matrix, 0));
        }
        if (pMesh)
        {
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        mCollisionModel.mIsFlyerFlag = false;
        break;
    }

    case 0xfbe2eb62:
        sscanf(pszValue, "%f", &mMinSpeed);
        break;
    case 0x86df0364:
        sscanf(pszValue, "%f", &mMaxSpeed);
        break;
    case 0x8ec2719e:
        sscanf(pszValue, "%f", &mMinOmega);
        break;
    case 0x38970514:
        sscanf(pszValue, "%f", &mMaxOmega);
        break;
    case 0x74e9dbae:
        sscanf(pszValue, "%d", &mCollidable);
        break;

    case 0xa20a3388:
        if (mNumChildren < 10)
        {
            char acClass[256];
            char acHardPoint[256];
            sscanf(pszValue, "%s %s", acClass, acHardPoint);
            mChildren[mNumChildren].mClassHash = PblHash(acClass);
            mChildren[mNumChildren].mHardPointHash = PblTEMPHash(acHardPoint);
            ++mNumChildren;
        }
        break;

    case 0x0e15a87f:
        mExplosionBreak = ExplosionClass::Find(PblHash(pszValue));
        break;
    case 0x02bb1fe0:
    case 0x8a323f32:
    case 0x9b30e763:
        mExplosionDestruct = ExplosionClass::Find(PblHash(pszValue));
        break;

    case 0x5dfdc07f:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sSoftCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_SOFT, CollisionMask(sSoftCollisionMask));
        }
        break;
    }
    case 0xfb2bdf07:
    {
        int iMaskId = mCollisionModel.GetMaskId(PblHash(pszValue));
        if (iMaskId >= 0)
        {
            sOrdnanceCollisionMask |= 1 << iMaskId;
            mCollisionModel.SetCollisionMask(COLL_ORDNANCE, CollisionMask(sOrdnanceCollisionMask));
        }
        break;
    }

    default:
        GameObjectClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// SYNTHETIC: SWBF2 0x00445af0
// CollisionPrimitive::CollisionPrimitive

// FUNCTION: SWBF2 0x00445b30
EntityClass* AsteroidClass::Derive(unsigned int uHash)
{
    return new AsteroidClass(this, uHash);
}

// GLOBAL: SWBF2 0x01eb0500
// SYNTHETIC: SWBF2 0x004022a0
// `dynamic initializer for 'Asteroid::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766d80
// `dynamic atexit destructor for 'Asteroid::sMemoryPool''
MemoryPool Asteroid::sMemoryPool("Asteroid", 0x280);

// GLOBAL: SWBF2 0x01eb0248
// SYNTHETIC: SWBF2 0x004022c0
// `dynamic initializer for 'Asteroid::rttiHashAsteroid''
PblHash Asteroid::rttiHashAsteroid("Asteroid");
char Asteroid::rttiNameAsteroid[] = "Asteroid";

class CollisionManager
{
public:
    static bool AddAsteroidBody(CollisionObject* pObject);
};

// FUNCTION: SWBF2 0x00445ba0
Entity* AsteroidClass::Build(const EntityDesc& desc)
{
    if (NetGame::IsEnabled() && NetGame::IsOneWorld() && !NetGame::IsHost() && !NetGame::IsClient())
    {
        return 0;
    }
    if (Asteroid::sMemoryPool.mUsed >= Asteroid::sMemoryPool.mCount)
    {
        return 0;
    }

    Asteroid* pAsteroid = new Asteroid(this, desc);
    if (pAsteroid)
    {
        if (mCollidable)
        {
            CollisionManager::AddAsteroidBody(pAsteroid);
        }

        pAsteroid->ActivatePhysics();
    }
    return pAsteroid;
}

// FUNCTION: SWBF2 0x00444390
void GameObject::ReserveForPlayer(float fTime)
{
}

static void AsteroidCollisionFollows(Asteroid* pAsteroid)
{
    PblVector3 vCenter;
    D3DXVec3TransformCoord((D3DXVECTOR3*)&vCenter,
                           (const D3DXVECTOR3*)&pAsteroid->mModel->_mModel->_BoundingSphere.GetCenter(),
                           (const D3DXMATRIX*)&pAsteroid->mMatrix);
    pAsteroid->SetCollisionCenter(vCenter);
    pAsteroid->RecalculateAABB();
}

// FUNCTION: SWBF2 0x00444820
Asteroid::Asteroid(AsteroidClass* pClass, const EntityDesc& desc)
    : GameObject(pClass, desc, COLL_ASTEROID, &pClass->mModel->_mLodData)
{
    mClass = pClass;
    Activate();

    const float fOmega =
        (mClass->mMaxOmega - mClass->mMinOmega) * PblRandom::GetGlobalFloat() + mClass->mMinOmega;
    const PblVector3 vAxis = PblRandom::GetGlobalVector3();
    mOmega = PblVector3(vAxis.x * fOmega, vAxis.y * fOmega, vAxis.z * fOmega);

    const float fSpeed =
        (mClass->mMaxSpeed - mClass->mMinSpeed) * PblRandom::GetGlobalFloat() + mClass->mMinSpeed;
    mMass = 10.0f;
    mVelocity = PblVector3(mMatrix.m[2][0] * fSpeed, mMatrix.m[2][1] * fSpeed, mMatrix.m[2][2] * fSpeed);
    NetGame::Created(this);
}

// FUNCTION: SWBF2 0x00444a30
Asteroid::~Asteroid()
{
    NetGame::Destroyed(this);
}

// SYNTHETIC: SWBF2 0x00444760
// Asteroid::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x00446dca
// [thunk]:Asteroid::`scalar deleting destructor' adjustor{320}
// SYNTHETIC: SWBF2 0x00446dd5
// [thunk]:Asteroid::`scalar deleting destructor' adjustor{12}
// SYNTHETIC: SWBF2 0x00446ddd
// [thunk]:Asteroid::`scalar deleting destructor' adjustor{512}
// SYNTHETIC: SWBF2 0x00446de8
// [thunk]:Asteroid::`scalar deleting destructor' adjustor{576}
// SYNTHETIC: SWBF2 0x00446df3
// [thunk]:Asteroid::`scalar deleting destructor' adjustor{148}

// FUNCTION: SWBF2 0x004444d0
bool Asteroid::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashAsteroid || GameObject::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00444510
unsigned int Asteroid::GetDerivedRtti() const
{
    return rttiHashAsteroid;
}

// FUNCTION: SWBF2 0x00444520
const char* Asteroid::GetDerivedRttiName() const
{
    return "Asteroid";
}

// FUNCTION: SWBF2 0x00444540
const PblMatrix& Asteroid::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00444550
const PblVector3& Asteroid::GetVelocity() const
{
    return mVelocity;
}

// FUNCTION: SWBF2 0x00444560
void Asteroid::SetPosition(const PblVector3& vPos)
{
    *(PblVector3*)&mMatrix.m[3][0] = vPos;
    mMatrix.m[3][3] = 1.0f;
    AsteroidCollisionFollows(this);
}

// FUNCTION: SWBF2 0x00444600
void Asteroid::SetMatrix(const PblMatrix& matrix)
{
    mMatrix = matrix;
    AsteroidCollisionFollows(this);
}

// FUNCTION: SWBF2 0x004446b0
float Asteroid::GetMaxSpeed() const
{
    return mVelocity.y * mMatrix.m[2][1] + mVelocity.x * mMatrix.m[2][0] +
           mVelocity.z * mMatrix.m[2][2];
}

// FUNCTION: SWBF2 0x004446e0
float Asteroid::GetMaxStrafe() const
{
    return 0.0f;
}

// FUNCTION: SWBF2 0x004446f0
bool Asteroid::AddToTeamManager() const
{
    return false;
}

// FUNCTION: SWBF2 0x00444700
void Asteroid::SetVelocity(const PblVector3& vVelocity)
{
    mVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x00444720
GameObjectType Asteroid::GetGameObjectType()
{
    return GO_UNKNOWN;
}

// FUNCTION: SWBF2 0x00444730
EntityType Asteroid::GetEntityType() const
{
    return ENT_ASTEROID;
}

// FUNCTION: SWBF2 0x00444740
void Asteroid::AddSends(void (*)(GameObject*, int), int)
{
}

// FUNCTION: SWBF2 0x00444750
const PblMatrix& Asteroid::GetSmoothedMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x00444ad0
void Asteroid::Init()
{
    SetRenderPos(mMatrix.GetTranslation());
    AsteroidCollisionFollows(this);
}

// FUNCTION: SWBF2 0x00444b70
void Asteroid::ClientKill()
{
    Kill();
}

// FUNCTION: SWBF2 0x00444b80
void Asteroid::Kill()
{
    mIsAlive = false;
    Deactivate();
    RemoveFromCollisionManager();

    AsteroidClass* pClass = mClass;
    if (pClass->mNumChildren > 0)
    {
        EntityDesc desc;
        desc.mMatrix = gMatrixIdentity;
        desc.mPlayerId = 0;
        for (int i = 0; i < pClass->mNumChildren; ++i)
        {
            EntityClass* pChildClass = EntityClass::Find(pClass->mChildren[i].mClassHash);
            PblMatrix mBone;
            if (pChildClass &&
                mModel->_mLod[0]->GetBoneMatrix(pClass->mChildren[i].mHardPointHash, mBone))
            {
                PblMatrix mChild;
                D3DXMatrixMultiply((D3DXMATRIX*)&mChild, (const D3DXMATRIX*)&mBone,
                                   (const D3DXMATRIX*)&mMatrix);
                desc.mMatrix = mChild;
                Entity* pChild = pChildClass->Build(desc);
                if (pChild)
                {
                    pChild->Init();
                    const PblVector3& vPos = mMatrix.GetTranslation();
                    PblVector3 vDir(desc.mMatrix.m[3][0] - vPos.x, desc.mMatrix.m[3][1] - vPos.y,
                                    desc.mMatrix.m[3][2] - vPos.z);
                    D3DXVec3Normalize((D3DXVECTOR3*)&vDir, (const D3DXVECTOR3*)&vDir);
                    static_cast<EntityGeometry*>(pChild)->SetVelocity(
                        PblVector3(vDir.x * 50.0f, vDir.y * 50.0f, vDir.z * 50.0f));
                }
            }
        }
        if (pClass->mExplosionBreak)
        {
            DamageOwner owner(0, 0, mTeam, mClass, 0, 1.0f);
            pClass->mExplosionBreak->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
        }
        return;
    }

    if (pClass->mExplosionDestruct)
    {
        DamageOwner owner(0, 0, mTeam, pClass, 0, 1.0f);
        pClass->mExplosionDestruct->CreateExplosion(mMatrix.GetTranslation(), owner, false, COLL_UNKNOWN);
    }
    CreateChunkCluster(1.0f);
}

// FUNCTION: SWBF2 0x00444e60
bool Asteroid::Update(float fDeltaTime)
{
    if (NetGame::IsSkipped(this, fDeltaTime))
    {
        return true;
    }

    if (mCurHealth <= 0.0f)
    {
        mCurHealth = 0.0f;
        if (!NetGame::IsEnabled() || !NetGame::IsClient())
        {
            Kill();
            return false;
        }
    }

    CameraManager* pCameras = CameraManager::Instance();
    for (unsigned int i = 0; i < pCameras->GetNumCameras(); ++i)
    {
        const PblMatrix& mView = pCameras->GetCamera(i)->GetMatrixInverse();
        const PblVector3& vPos = static_cast<CollisionObject*>(this)->GetPosition();
        const float fZ = vPos.x * mView.m[0][2] + vPos.y * mView.m[1][2] +
                         vPos.z * mView.m[2][2] + mView.m[3][2];
        PblVector3 vView;
        D3DXVec3TransformCoord((D3DXVECTOR3*)&vView, (const D3DXVECTOR3*)&vPos,
                               (const D3DXMATRIX*)&mView);
        if (fZ < -2000.0f || fZ > 0.0f)
        {
            return true;
        }
    }

    PblQuaternion qRotation(mMatrix);
    PblVector3 vPos = mMatrix.GetTranslation();
    const float fOmega = sqrtf(mOmega.x * mOmega.x + mOmega.y * mOmega.y + mOmega.z * mOmega.z);
    if (fOmega > 0.0001f)
    {
        const float fInv = 1.0f / fOmega;
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, fOmega * fDeltaTime * 0.5f);
        const PblQuaternion qStep(mOmega.x * fInv * fSin, mOmega.y * fInv * fSin,
                                  mOmega.z * fInv * fSin, fCos);
        qRotation = qRotation * qStep;
    }
    vPos.x += mVelocity.x * fDeltaTime;
    vPos.y += mVelocity.y * fDeltaTime;
    vPos.z += mVelocity.z * fDeltaTime;

    D3DXQUATERNION qNormalized;
    D3DXQuaternionNormalize(&qNormalized, (const D3DXQUATERNION*)&qRotation);
    PblMatrix mNew;
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, &qNormalized);
    *(PblVector3*)&mNew.m[3][0] = vPos;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;
    SetRenderPos(vPos);
    AsteroidCollisionFollows(this);
    return true;
}

// FUNCTION: SWBF2 0x004452c0
void Asteroid::Render(unsigned char ucLod, float fDist, unsigned int uMask)
{
    const PblMatrix& mRender = GetSmoothedMatrix();
    const PblMatrix mInverse = mRender.GetInverseFast();
    SetupShadow(this, mModel, ucLod, uMask, 0);
    mModel->Render(ucLod, fDist, &mRender, 0, RedColor::WHITE, uMask & ~0x10u, 0);
}

// FUNCTION: SWBF2 0x00445350
bool Asteroid::CollisionCallback(const CollisionResult* pResult, CollisionObject* pOther,
                                 Restrictor* pRestrictor)
{
    if (pOther->GetGameObject() && !pOther->GetGameObject()->IsRtti(rttiHashAsteroid))
    {
        mCurHealth = 0.0f;
    }
    return CollisionObject::CollisionCallback(pResult, pOther, pRestrictor);
}

// FUNCTION: SWBF2 0x004453a0
void Asteroid::ActivatePhysics()
{
    ActivateThread(0, (ThreadType)2);
}

// FUNCTION: SWBF2 0x004453c0
void Asteroid::DeactivatePhysics()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x00446d40
template ExplosionClass* Factory<Ordnance, ExplosionClass, OrdnanceDesc>::Find(unsigned int);
