#include <math.h>
#include <stddef.h>
#include <d3dx9math.h>

#include "LeafEffect.h"
#include "Battlefront2Lod.h"
#include "NetGame.h"
#include "CameraManager.h"
#include "WeatherEffect.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblRandom.h"

#include "RedEngineFL/Application/RedTerrain.h"

extern NoLodData noloddata;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float EPSILON = 0.0001f;

class GameObject;
class CollisionManager
{
public:
    static int GetObjectsInRect(const PblVector3& vMin, const PblVector3& vMax, GameObject** const ppObjects,
                                int iMaxObjects, const GameObject* pIgnore, int iMask);
};

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

// FUNCTION: SWBF2 0x0056b370
FallingLeafParticle::FallingLeafParticle()
    : RedSceneObject(&noloddata)
{
    mIsValid = false;
    mTextureHash = 0x2648CD55u;
    PlatformInit();
}

// SYNTHETIC: SWBF2 0x0056b3d0
// FallingLeafParticle::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056b400
FallingLeafParticle::~FallingLeafParticle()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x0056b450
void FallingLeafParticle::RespawnLeaf(const PblVector3& vPos, LeafSpawner* pSpawner, LeafState eState)
{
    mRenderTimer = 0.0f;
    mGroundHeight = RedTerrain::GetHoleHeight(vPos);
    mVelocity.SetAll(0.0f);
    mWindVelocity.SetAll(0.0f);
    mConstantWindVelocity.SetAll(0.0f);

    PblQuaternion q = PblQuaternion(gMatrixIdentity);
    PblVector3 vOmega(0.0f, PblRandom::GetGlobalFloat() * PblMath::fTwoPi, 0.0f);
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion_FromAxisAngle(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;
    mInitMatrix = mNew;
    mIsValid = true;

    SetRenderRadius(0.125f);
    SetRenderPos(vPos);
    Activate();

    mFallTime = PblRandom::GetGlobalFloat() * PblMath::fTwoPi;
    mRandomness = PblRandom::GetGlobalFloat();
    if (eState == LEAF_NONE)
    {
        eState = (PblRandom::GetGlobalFloat() < 0.2f) ? LEAF_TUMBLE : LEAF_PERIODIC_FLUTTER;
    }
    mState = eState;
    mSpawner = pSpawner;

    LeafParticleManager::sInstance->mLeafFallSound.Play(&vPos, NULL, false, true);
}

// FUNCTION: SWBF2 0x0056b810
void FallingLeafParticle::SetConstantWindVelocity(const PblVector3& vVelocity)
{
    mConstantWindVelocity = vVelocity;
}

// FUNCTION: SWBF2 0x0056b830
void FallingLeafParticle::DoFlutterUpdate(float fDeltaTime)
{

    static PblVector3 sFlutterSinkVelocity(0.0f, -0.5f, 0.0f);

    static float sFlutterMaxRockAngle = PblMath::fPi * 0.25f;

    float fSin = PblMath::Sin(mFallTime * 2.0f);
    PblVector3 vPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);

    PblQuaternion q = PblQuaternion(mInitMatrix);
    PblVector3 vOmega(0.0f, 0.0f, -(mRandomness * sFlutterMaxRockAngle * -fSin));
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion_FromAxisAngle(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }
    if (mRandomness < 0.15f)
    {
        q = q * PblQuaternion_FromAxisAngle(PblVector3(0.0f, 1.0f, 0.0f), (mRandomness - 1.0f) * 10.0f * mFallTime);
    }

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;

    float fSlide = fSin * -1.8f;
    PblVector3 vSlide(mMatrix.m[0][0] * fSlide, mMatrix.m[0][1] * fSlide, mMatrix.m[0][2] * fSlide);
    mVelocity = Interpolate(sFlutterSinkVelocity, vSlide, (float)fabs(mRandomness * fSin));

    mMatrix.m[3][0] = mVelocity.x * fDeltaTime + vPos.x;
    mMatrix.m[3][1] = mVelocity.y * fDeltaTime + vPos.y;
    mMatrix.m[3][2] = mVelocity.z * fDeltaTime + vPos.z;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0056bc10
void FallingLeafParticle::DoTumbleUpdate(float fDeltaTime)
{
    PblVector3 vPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);

    PblQuaternion q = PblQuaternion(mInitMatrix);
    PblVector3 vOmega((mRandomness + 2.0f) * PblMath::fTwoPi * mFallTime, 0.0f, 0.0f);
    float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
    if (fAngle > EPSILON)
    {
        float fInv = 1.0f / fAngle;
        q = q * PblQuaternion_FromAxisAngle(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
    }

    PblQuaternion qUnit;
    PblMatrix mNew;
    D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
    mNew.m[3][0] = vPos.x;
    mNew.m[3][1] = vPos.y;
    mNew.m[3][2] = vPos.z;
    mNew.m[3][3] = 1.0f;
    mMatrix = mNew;

    float fSpeed = mRandomness * 0.3f + 0.3f;
    vPos.x -= 0.0f;
    vPos.y -= (mRandomness + 1.0f) * fDeltaTime;
    vPos.z -= 0.0f;
    mMatrix.m[3][0] = mInitMatrix.m[2][0] * fSpeed * fDeltaTime + vPos.x;
    mMatrix.m[3][1] = mInitMatrix.m[2][1] * fSpeed * fDeltaTime + vPos.y;
    mMatrix.m[3][2] = mInitMatrix.m[2][2] * fSpeed * fDeltaTime + vPos.z;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0056be80
void FallingLeafParticle::DoGroundedUpdate(float)
{
    PblVector3 vPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
    mGroundHeight = RedTerrain::GetHoleHeight(vPos);
    mMatrix.m[3][0] = vPos.x;
    mMatrix.m[3][1] = mGroundHeight + 0.1f;
    mMatrix.m[3][2] = vPos.z;
    mMatrix.m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0056bf10
bool FallingLeafParticle::Update(float fDeltaTime)
{
    if (!mIsValid)
    {
        return true;
    }

    mRenderTimer += fDeltaTime;
    switch (mState)
    {
    case LEAF_PERIODIC_FLUTTER:
        DoFlutterUpdate(fDeltaTime);
        break;
    case LEAF_TUMBLE:
        DoTumbleUpdate(fDeltaTime);
        break;
    }

    mFallTime += fDeltaTime;
    if (mFallTime > PblMath::fTwoPi)
    {
        mFallTime -= PblMath::fTwoPi;
    }

    PblVector3 vPos(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
    PblVector3 vWind(mConstantWindVelocity.x + mWindVelocity.x,
                     mWindVelocity.y + mConstantWindVelocity.y,
                     mWindVelocity.z + mConstantWindVelocity.z);
    if (sqrtf(vWind.y * vWind.y + vWind.x * vWind.x + vWind.z * vWind.z) > 0.1f)
    {
        PblQuaternion q = PblQuaternion(mMatrix);

        float fFade = 1.0f - mFallTime / PblMath::fTwoPi;
        PblVector3 vSwirl((mWindVelocity.x + mConstantWindVelocity.x * 16.0f * fFade) * PblMath::fTwoPi,
                          (mWindVelocity.y + mConstantWindVelocity.y * 16.0f * fFade) * PblMath::fTwoPi,
                          (mWindVelocity.z + mConstantWindVelocity.z * 16.0f * fFade) * PblMath::fTwoPi);

        PblVector3 vOmega(vSwirl.y * 0.0f - vSwirl.z,
                          vSwirl.z * 0.0f - vSwirl.x * 0.0f,
                          vSwirl.x - vSwirl.y * 0.0f);
        PblVector3 vLocal;
        PblMatrix mInverse = mMatrix.GetInverseFast();
        D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocal, (const D3DXVECTOR3*)&vOmega, (const D3DXMATRIX*)&mInverse);

        float fAngle = sqrtf(vOmega.x * vOmega.x + vOmega.y * vOmega.y + vOmega.z * vOmega.z);
        if (fAngle > EPSILON)
        {
            float fInv = 1.0f / fAngle;
            q = q * PblQuaternion_FromAxisAngle(PblVector3(vOmega.x * fInv, vOmega.y * fInv, vOmega.z * fInv), fAngle);
        }

        vPos.x += (mConstantWindVelocity.x + mWindVelocity.x) * fDeltaTime;
        vPos.y += (mWindVelocity.y + mConstantWindVelocity.y) * fDeltaTime;
        vPos.z += (mWindVelocity.z + mConstantWindVelocity.z) * fDeltaTime;

        PblQuaternion qUnit;
        PblMatrix mNew;
        D3DXQuaternionNormalize((D3DXQUATERNION*)&qUnit, (const D3DXQUATERNION*)&q);
        D3DXMatrixRotationQuaternion((D3DXMATRIX*)&mNew, (const D3DXQUATERNION*)&qUnit);
        mNew.m[3][0] = vPos.x;
        mNew.m[3][1] = vPos.y;
        mNew.m[3][2] = vPos.z;
        mNew.m[3][3] = 1.0f;
        mMatrix = mNew;

        float fDamp = fDeltaTime * 2.0f;
        mWindVelocity.x -= mWindVelocity.x * fDamp;
        mWindVelocity.y -= mWindVelocity.y * fDamp;
        mWindVelocity.z -= mWindVelocity.z * fDamp;
        if (!(sqrtf(mWindVelocity.x * mWindVelocity.x + mWindVelocity.y * mWindVelocity.y +
                    mWindVelocity.z * mWindVelocity.z) > 0.1f))
        {
            mGroundHeight = RedTerrain::GetHoleHeight(vPos);
        }
    }

    if (mState == LEAF_GROUNDED)
    {
        DoGroundedUpdate(fDeltaTime);
        vPos = PblVector3(mMatrix.m[3][0], mMatrix.m[3][1], mMatrix.m[3][2]);
    }

    SetRenderPos(vPos);
    if (mGroundHeight - 0.5f > vPos.y || mRenderTimer > 3.0f)
    {
        Deactivate();
        mIsValid = false;
        if (mSpawner)
        {
            mSpawner->DeactivateLeaf();
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0056c470
void FallingLeafParticle::AddWindVelocity(const PblVector3& vVelocity)
{
    float fSpeed = sqrtf(vVelocity.x * vVelocity.x + vVelocity.y * vVelocity.y + vVelocity.z * vVelocity.z);
    mWindVelocity.x += vVelocity.x * 0.9f;
    mWindVelocity.y += vVelocity.y * 0.9f;
    mWindVelocity.z += vVelocity.z * 0.9f;
    mWindVelocity.x += 0.0f;
    mWindVelocity.y += fSpeed * 0.4f;
    mWindVelocity.z += 0.0f;
}

// GLOBAL: SWBF2 0x01e573d0
LeafParticleManager* LeafParticleManager::sInstance;

// FUNCTION: SWBF2 0x0056c570
LeafParticleManager::LeafParticleManager()
    : Thread()
{
    mNextFreeLeaf = mLeafParticles;
    mGroundLeavesTimer = 0.0f;
}

// SYNTHETIC: SWBF2 0x0056c5f0
// LeafParticleManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056c620
LeafParticleManager::~LeafParticleManager()
{
    DeactivateThread();
}

// FUNCTION: SWBF2 0x0056c690
bool LeafParticleManager::Create()
{
    if (sInstance)
    {
        return false;
    }
    sInstance = new LeafParticleManager;
    sInstance->ActivateThread(0, PER_FRAME);
    return true;
}

// FUNCTION: SWBF2 0x0056c710
bool LeafParticleManager::Destroy()
{
    if (!sInstance)
    {
        return false;
    }
    delete sInstance;
    sInstance = NULL;
    return true;
}

// FUNCTION: SWBF2 0x0056c730
FallingLeafParticle* LeafParticleManager::GetFreeLeaf()
{
    if (!sInstance)
    {
        return NULL;
    }
    FallingLeafParticle* pLeaf = sInstance->mNextFreeLeaf;
    if (pLeaf)
    {
        for (int i = 0; i < 100; ++i)
        {
            FallingLeafParticle* pOther = &sInstance->mLeafParticles[i];
            if (!pOther->IsValid() && pOther != pLeaf)
            {
                sInstance->mNextFreeLeaf = pOther;
            }
        }
    }
    return pLeaf;
}

// FUNCTION: SWBF2 0x0056c770
bool LeafParticleManager::CreateClient()
{
    if (sInstance)
    {
        for (int i = 0; i < 100; ++i)
        {
            if (sInstance->mLeafParticles[i].IsValid())
            {
                sInstance->mLeafParticles[i].Deactivate();
            }
        }
        NetGame::EnterClient();
        for (int i = 0; i < 100; ++i)
        {
            if (sInstance->mLeafParticles[i].IsValid())
            {
                sInstance->mLeafParticles[i].Activate();
            }
        }
        NetGame::LeaveClient();

        sInstance->DeactivateThread();
        NetGame::EnterClient();
        sInstance->ActivateThread(0, PER_FRAME);
        NetGame::LeaveClient();
    }
    return true;
}

// FUNCTION: SWBF2 0x0056c810
bool LeafParticleManager::Update(float fDeltaTime)
{
    for (int i = 0; i < 100; ++i)
    {
        mLeafParticles[i].Update(fDeltaTime);
        if (!mLeafParticles[i].IsValid())
        {
            mNextFreeLeaf = &mLeafParticles[i];
        }
    }

    mGroundLeavesTimer += fDeltaTime;
    if (mGroundLeavesTimer > 30.0f)
    {
        int iNumCameras = CameraManager::Instance()->GetNumCameras();
        for (int iCam = 0; iCam < iNumCameras; ++iCam)
        {
            PblMatrix mCamera = CameraManager::Instance()->GetCamera(iCam)->GetMatrix();
            const PblVector3& vDir = *(const PblVector3*)&mCamera.m[2][0];
            const PblVector3& vPos = *(const PblVector3*)&mCamera.m[3][0];

            PblVector3 vWind;
            D3DXVec3Normalize((D3DXVECTOR3*)&vWind, (const D3DXVECTOR3*)&FogCloudEffect::Instance()->GetVelocity());

            PblVector3 vBack(vPos.x - (vDir.x + vWind.x) * 30.0f + 0.0f,
                             vPos.y - (vDir.y + vWind.y) * 30.0f + 1024.0f,
                             vPos.z - (vDir.z + vWind.z) * 30.0f + 0.0f);
            PblVector3 vMin(vBack.x > vPos.x ? vPos.x : vBack.x,
                            vBack.y > vPos.y ? vPos.y : vBack.y,
                            vBack.z > vPos.z ? vPos.z : vBack.z);
            PblVector3 vMax(vPos.x > vBack.x ? vPos.x : vBack.x,
                            vPos.y > vBack.y ? vPos.y : vBack.y,
                            vPos.z > vBack.z ? vPos.z : vBack.z);

            GameObject* pObject;
            if (CollisionManager::GetObjectsInRect(vMin, vMax, &pObject, 1, NULL, 0x200) > 0)
            {
                for (int i = 0; i < 20; ++i)
                {
                    FallingLeafParticle* pLeaf = GetFreeLeaf();
                    if (!pLeaf)
                    {
                        continue;
                    }
                    float fWindDist = (PblRandom::GetGlobalFloat() - 0.5f) * 30.0f + 30.0f;
                    float fDirDist = (PblRandom::GetGlobalFloat() - 0.5f) * 30.0f + 30.0f;
                    PblVector3 vSpawn;
                    vSpawn.x = vPos.x - vDir.x * fDirDist - fWindDist * vWind.x;
                    vSpawn.z = vPos.z - vDir.z * fDirDist - vWind.z * fWindDist;
                    vSpawn.y = RedTerrain::GetInterpolatedHeight(vSpawn.x, vSpawn.z);
                    PblVector3 vDrop(vSpawn.x + 0.0f,
                                     PblRandom::GetGlobalFloat() + 1.0f + vSpawn.y,
                                     vSpawn.z + 0.0f);
                    pLeaf->RespawnLeaf(vDrop, NULL, LEAF_GROUNDED);
                    pLeaf->AddWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                    pLeaf->SetConstantWindVelocity(PblVector3(vWind.x * 5.0f, vWind.y * 5.0f, vWind.z * 5.0f));
                }
                mGroundLeavesTimer = 0.0f;
            }
        }
    }
    return true;
}
