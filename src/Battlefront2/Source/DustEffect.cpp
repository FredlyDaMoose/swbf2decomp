#include "DustEffect.h"

#include <math.h>
#include <stdio.h>

#include "Battlefront2Lod.h"
#include "CameraManager.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float FADE_IN_PERCENT = 0.3f;
static const float FADE_OUT_PERCENT = 0.5f;

class CloudClusterObjectLodData : public RedLodData
{
public:
    CloudClusterObjectLodData()
    {
        _LodCost[0] = 0;
        _LodCost[1] = 0;
        _LodCost[2] = 0;
        _LodCost[3] = 0;
        _LodBitmask = 7;
        _LodClass = -1;
    }
};

// GLOBAL: SWBF2 0x007eb79c
CloudClusterObjectLodData cloudClusterObjectLodData;

// GLOBAL: SWBF2 0x007eb7d0
PblList<DustEffectClass> DustEffectClass::sDustEffectClasses;

// GLOBAL: SWBF2 0x007eb7bc
PblList<DustEffect> DustEffectClass::sDustEffects;

// FUNCTION: SWBF2 0x00443f60
bool EntityClass::IsRtti(unsigned int uRtti)
{
    return rttiHashEntityClass == uRtti;
}

// FUNCTION: SWBF2 0x0048ec20
DustEffect::DustEffect(DustEffectClass* pClass, const PblMatrix& matrix)
    : RedSceneObject(&cloudClusterObjectLodData)
    , mClass(pClass)
{
    mMatrix = gMatrixIdentity;

    mMatrix.m[1][0] = matrix.m[1][0] * mClass->mHeightScale;
    mMatrix.m[1][1] = matrix.m[1][1] * mClass->mHeightScale;
    mMatrix.m[1][2] = matrix.m[1][2] * mClass->mHeightScale;
    mMatrix.m[1][3] = 0.0f;

    PblVector3 vBack(-mMatrix.m[2][0], -mMatrix.m[2][1], -mMatrix.m[2][2]);
    PblVector3 vRight(mMatrix.m[1][2] * vBack.y - mMatrix.m[1][1] * vBack.z,
                      mMatrix.m[1][0] * vBack.z - mMatrix.m[1][2] * vBack.x,
                      mMatrix.m[1][1] * vBack.x - mMatrix.m[1][0] * vBack.y);
    PblVector3 vOut;
    D3DXVec3Normalize(&vOut, &vRight);
    mMatrix.m[0][0] = vOut.x;
    mMatrix.m[0][1] = vOut.y;
    mMatrix.m[0][2] = vOut.z;
    mMatrix.m[0][3] = 0.0f;

    PblVector3 vAt(mMatrix.m[1][2] * mMatrix.m[0][1] - mMatrix.m[1][1] * mMatrix.m[0][2],
                   mMatrix.m[1][0] * mMatrix.m[0][2] - mMatrix.m[0][0] * mMatrix.m[1][2],
                   mMatrix.m[0][0] * mMatrix.m[1][1] - mMatrix.m[1][0] * mMatrix.m[0][1]);
    D3DXVec3Normalize(&vOut, &vAt);
    mMatrix.m[2][0] = vOut.x;
    mMatrix.m[2][1] = vOut.y;
    mMatrix.m[2][2] = vOut.z;
    mMatrix.m[2][3] = 0.0f;

    mMatrix.m[3][0] = matrix.m[3][0];
    mMatrix.m[3][1] = matrix.m[3][1];
    mMatrix.m[3][2] = matrix.m[3][2];
    mMatrix.m[3][3] = 1.0f;

    SetRenderPos(*(const PblVector3*)mMatrix.m[3]);
    SetRenderRadius(mClass->mBoundingRadius);
    Activate();

    DustEffectClass::sDustEffects.AddLast(&mEffectNode, this);
}

// SYNTHETIC: SWBF2 0x00490b1d
// [thunk]:DustEffect::`scalar deleting destructor' adjustor{88}
// SYNTHETIC: SWBF2 0x0048eed0
// DustEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048ef00
DustEffect::~DustEffect()
{
}

// FUNCTION: SWBF2 0x0048ef20
void DustEffect::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (uiFlags & 0x200000)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const PblMatrix& camMatrix = pCamera->GetMatrix();

    if (mClass->mCameraDistance > 0.0f)
    {
        mMatrix.m[3][0] = camMatrix.m[3][0];
        mMatrix.m[3][2] = camMatrix.m[3][2];
        mMatrix.m[3][3] = 1.0f;
        SetRenderPos(*(const PblVector3*)mMatrix.m[3]);
    }

    float fDistance = camMatrix.m[2][1] * (camMatrix.m[3][1] - mMatrix.m[3][1]) +
                      camMatrix.m[2][0] * (camMatrix.m[3][0] - mMatrix.m[3][0]) +
                      camMatrix.m[2][2] * (camMatrix.m[3][2] - mMatrix.m[3][2]) +
                      mClass->mBoundingRadius;

    if (fDistance < mClass->mMinDistance || fDistance > mClass->mMaxDistance)
    {
        return;
    }

    float fAlpha = (mClass->mMaxDistance - fDistance) * 0.005f;
    if (fAlpha >= 1.0f)
    {
        fAlpha = 1.0f;
    }
    if (mClass->mMinDistance > 0.0f && mClass->mMinDistance + 100.0f > fDistance)
    {
        fAlpha = (fDistance - mClass->mMinDistance) * 0.01f;
    }
    if (fAlpha > 0.0f)
    {
        mClass->Render(mMatrix, fAlpha, uiFlags);
    }
}

// FUNCTION: SWBF2 0x0048f060
void DustEffectClass::CreateClient()
{
    for (PblList<DustEffect>::Itor it(sDustEffects); it; ++it)
    {
        it->Deactivate();
    }
    NetGame::EnterClient();
    for (PblList<DustEffect>::Itor it(sDustEffects); it; ++it)
    {
        it->Activate();
    }
    NetGame::LeaveClient();

    for (PblList<DustEffectClass>::Itor it(sDustEffectClasses); it; ++it)
    {
        if (it->mParticle)
        {
            it->Deactivate();
            it->DeactivateThread();
        }
    }
    NetGame::EnterClient();
    for (PblList<DustEffectClass>::Itor it(sDustEffectClasses); it; ++it)
    {
        if (it->mParticle)
        {
            it->Activate(10000);
            it->ActivateThread(0, PER_FRAME);
        }
    }
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x0048f120
void DustEffectClass::Render(int value)
{
}

// FUNCTION: SWBF2 0x0048f130
bool DustEffectClass::UpdateParticle(DustParticle& particle, float fDeltaTime,
                                     const PblVector3& vWind)
{
    particle.mTime += fDeltaTime;
    if (particle.mTime > particle.mLifeTime)
    {
        return false;
    }

    particle.mColor.a = mColor.a;
    particle.mPos.x += vWind.x + particle.mVel.x * fDeltaTime;
    particle.mPos.y += vWind.y + particle.mVel.y * fDeltaTime;
    particle.mPos.z += vWind.z + particle.mVel.z * fDeltaTime;

    float fFadeIn = particle.mLifeTime * FADE_IN_PERCENT;
    if (particle.mTime < fFadeIn)
    {
        particle.mColor.a *= particle.mTime / fFadeIn;
    }
    else
    {
        float fFadeOut = particle.mLifeTime * FADE_OUT_PERCENT;
        float fRemaining = particle.mLifeTime - particle.mTime;
        if (fRemaining < fFadeOut)
        {
            particle.mColor.a *= fRemaining / fFadeOut;
        }
    }

    if (mRadiusFadeMin > 0.0f)
    {
        float fDist2 = particle.mPos.x * particle.mPos.x + particle.mPos.z * particle.mPos.z;
        if (fDist2 > mRadiusFadeMin * mRadiusFadeMin)
        {
            if (fDist2 > mRadiusFadeMax * mRadiusFadeMax)
            {
                return false;
            }
            float fDist = (float)sqrt((double)fDist2);
            particle.mColor.a *= 1.0f - (fDist - mRadiusFadeMin) / (mRadiusFadeMax - mRadiusFadeMin);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0048f2b0
void DustEffectClass::SpawnParticle(DustParticle& particle)
{
    {
        const float fZ = (mPosMax.z - mPosMin.z) * PblRandom::GetGlobalFloat() + mPosMin.z;
        const float fY = (mPosMax.y - mPosMin.y) * PblRandom::GetGlobalFloat() + mPosMin.y;
        const float fX = (mPosMax.x - mPosMin.x) * PblRandom::GetGlobalFloat() + mPosMin.x;
        particle.mPos = PblVector3(fX, fY, fZ);
    }

    if (mCameraDistance > 0.0f)
    {
        const PblMatrix& camMatrix = CameraManager::sInstance->mRedCamera[0]->GetMatrix();
        float fForward = mCameraDistance + particle.mPos.z;
        float fSide = particle.mPos.x;
        particle.mPos.x = camMatrix.m[0][0] * fSide + -camMatrix.m[2][0] * fForward;
        particle.mPos.z = camMatrix.m[0][2] * fSide + -camMatrix.m[2][2] * fForward;
    }

    particle.mColor.a = 0.0f;
    {
        const float fZ = (mVelMax.z - mVelMin.z) * PblRandom::GetGlobalFloat() + mVelMin.z;
        const float fY = (mVelMax.y - mVelMin.y) * PblRandom::GetGlobalFloat() + mVelMin.y;
        const float fX = (mVelMax.x - mVelMin.x) * PblRandom::GetGlobalFloat() + mVelMin.x;
        particle.mVel = PblVector3(fX, fY, fZ);
    }
    particle.mSize = (mSizeMax - mSizeMin) * PblRandom::GetGlobalFloat() + mSizeMin;
    particle.mLifeTime = (mLifeTimeMax - mLifeTimeMin) * PblRandom::GetGlobalFloat() + mLifeTimeMin;
    particle.mTime = 0.0f;

    mDustSound.Play(&particle.mPos, 0, false, true);
}

// FUNCTION: SWBF2 0x0048f6e0
bool DustEffectClass::Update(float fDeltaTime)
{
    PblVector3 vWind(0.0f, 0.0f, 0.0f);
    if (mCameraDistance > 0.0f)
    {
        const PblMatrix& camMatrix = CameraManager::sInstance->mRedCamera[0]->GetMatrix();
        vWind = PblVector3(mOldCamPos.x - camMatrix.m[3][0], mOldCamPos.y - camMatrix.m[3][1],
                           mOldCamPos.z - camMatrix.m[3][2]);
        if (vWind.x * vWind.x + vWind.y * vWind.y + vWind.z * vWind.z > 100.0f)
        {
            vWind = PblVector3(0.0f, 0.0f, 0.0f);
        }
        mOldCamPos = *(const PblVector3*)camMatrix.m[3];
    }

    mTimer += fDeltaTime;
    for (int i = 0; i < mNumParticles; ++i)
    {
        if (!UpdateParticle(mParticle[i], fDeltaTime, vWind))
        {
            SpawnParticle(mParticle[i]);
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x0048f810
DustEffectClass::DustEffectClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
    , Renderable()
{
    mParticle = 0;
    mNumParticles = 50;
    mBoundingRadius = 64.0f;
    mPosMin.x = -20.0f;
    mPosMin.y = -1.0f;
    mPosMin.z = -20.0f;
    mPosMax.x = 20.0f;
    mPosMax.y = 1.0f;
    mPosMax.z = 20.0f;
    mVelMin.x = -3.0f;
    mVelMin.y = -1.0f;
    mVelMin.z = -3.0f;
    mVelMax.x = 3.0f;
    mVelMax.y = 1.0f;
    mVelMax.z = 3.0f;
    mSizeMin = 10.0f;
    mSizeMax = 15.0f;
    mLifeTimeMin = 2.0f;
    mLifeTimeMax = 3.0f;
    mColor.r = 1.0f;
    mColor.g = 1.0f;
    mColor.b = 1.0f;
    mColor.a = 1.0f;
    mMinDistance = 0.0f;
    mTextureHash = 0xAA3E3A1Fu;
    mRadiusFadeMin = -1.0f;
    mRadiusFadeMax = -1.0f;
    mHeightScale = 0.2f;
    mTimer = 0.0f;
    mCameraDistance = -1.0f;
    mOldCamPos.x = 0.0f;
    mOldCamPos.y = 0.0f;
    mOldCamPos.z = 0.0f;
    mRenderData = 0;

    sDustEffectClasses.AddLast(&mClassNode, this);
}

// SYNTHETIC: SWBF2 0x00490b0d
// [thunk]:DustEffectClass::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x00490b15
// [thunk]:DustEffectClass::`scalar deleting destructor' adjustor{120}
// SYNTHETIC: SWBF2 0x0048fa10
// DustEffectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048fa40
DustEffectClass::~DustEffectClass()
{
    if (mParticle)
    {
        delete mParticle;
    }
    DestroyRenderData();
}

// FUNCTION: SWBF2 0x0048fad0
void DustEffectClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    float fX, fY, fZ;

    switch (uProperty)
    {
    case 0x3c6468f4:
        mTextureHash = PblHash(pszValue);
        break;

    case 0x38b0b50d:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mPosMax.x = fX;
        mPosMax.y = fY;
        mPosMax.z = fZ;
        break;

    case 0x3d7e6258:
        sscanf(pszValue, "%f %f %f %f", &mColor.r, &mColor.g, &mColor.b, &mColor.a);
        break;

    case 0x597c1bb7:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mPosMin.x = fX;
        mPosMin.y = fY;
        mPosMin.z = fZ;
        break;

    case 0x4e2c3b35:
        sscanf(pszValue, "%f", &mCameraDistance);
        break;

    case 0x5d8b6dab:
        sscanf(pszValue, "%f", &mColor.a);
        break;

    case 0x80ba1308:
        sscanf(pszValue, "%f", &mSizeMax);
        break;

    case 0x80887a6f:
        sscanf(pszValue, "%f", &mRadiusFadeMin);
        break;

    case 0x6e74d661:
        sscanf(pszValue, "%f", &mRadiusFadeMax);
        break;

    case 0x9d399b92:
        sscanf(pszValue, "%f", &mLifeTimeMax);
        break;

    case 0xb10f20ca:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mVelMin.x = fX;
        mVelMin.y = fY;
        mVelMin.z = fZ;
        break;

    case 0xa7bcb86c:
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mVelMax.x = fX;
        mVelMax.y = fY;
        mVelMax.z = fZ;
        break;

    case 0xb4368857:
        mDustSound.SetID(pszValue, false, false);
        break;

    case 0xcaaa3778:
        sscanf(pszValue, "%f", &mHeightScale);
        break;

    case 0xbd9397ec:
        sscanf(pszValue, "%f", &mLifeTimeMin);
        break;

    case 0xb835cf50:
        sscanf(pszValue, "%f", &mMinDistance);
        break;

    case 0xd133f7da:
        if (sscanf(pszValue, "%i", &mNumParticles))
        {
            mNumParticles = (unsigned int)mNumParticles / CameraManager::sInstance->m_iNumCam;
        }
        break;

    case 0xeaebcf6e:
        sscanf(pszValue, "%f", &mSizeMin);
        break;

    case 0xe6915cf6:
        sscanf(pszValue, "%f", &mMaxDistance);
        break;

    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0048fdf0
EntityClass* DustEffectClass::Derive(unsigned int uHash)
{
    return new DustEffectClass(uHash);
}

// FUNCTION: SWBF2 0x0048fe60
void DustEffectClass::CalcBoundingRadius()
{
    float fHalfSize = mSizeMax * 0.5f;
    mBoundingRadius = fHalfSize;
    if (mRadiusFadeMin > 0.0f)
    {
        mBoundingRadius = mRadiusFadeMax + fHalfSize;
        return;
    }

    float fPosMin = sqrtf(mPosMin.x * mPosMin.x + mPosMin.y * mPosMin.y + mPosMin.z * mPosMin.z);
    float fPosMax = sqrtf(mPosMax.x * mPosMax.x + mPosMax.y * mPosMax.y + mPosMax.z * mPosMax.z);
    float fPos = fPosMax > fPosMin ? fPosMax : fPosMin;
    float fVelMin = sqrtf(mVelMin.x * mVelMin.x + mVelMin.y * mVelMin.y + mVelMin.z * mVelMin.z);
    float fVelMax = sqrtf(mVelMax.x * mVelMax.x + mVelMax.y * mVelMax.y + mVelMax.z * mVelMax.z);
    float fVel = fVelMax > fVelMin ? fVelMax : fVelMin;
    mBoundingRadius = mLifeTimeMax * fVel + fPos + fHalfSize;
}

// FUNCTION: SWBF2 0x0048fff0
bool DustEffectClass::Init()
{
    if (IsThreadActive())
    {
        return true;
    }
    if (mNumParticles <= 0)
    {
        return false;
    }

    mParticle = new DustParticle[mNumParticles];
    for (int i = 0; i < mNumParticles; ++i)
    {
        SpawnParticle(mParticle[i]);
        mParticle[i].mLifeTime *= PblRandom::GetGlobalFloat();
        mParticle[i].mColor = mColor;
        mParticle[i].mTime = 0.0f;
    }

    CalcBoundingRadius();
    CreateRenderData();
    ActivateThread(0, PER_FRAME);
    Activate(10000);
    mRenderHeap = new PblHeap<DustEffect*, float>(80, 100000.0f, 0);
    return true;
}

// FUNCTION: SWBF2 0x004901a0
Entity* DustEffectClass::Build(const EntityDesc& desc)
{
    if (!Init())
    {
        return 0;
    }
    return new DustEffect(this, desc.mMatrix);
}

// FUNCTION: SWBF2 0x00490220
void DustEffectClass::Reset()
{
    for (PblList<DustEffectClass>::Itor it(sDustEffectClasses); it; ++it)
    {
        it->DestroyRenderData();
    }
    sDustEffectClasses.RemoveAll();
    sDustEffects.RemoveAll();
}

// FUNCTION: SWBF2 0x00490ac0
template void PblList<DustEffectClass>::RemoveAll();

// FUNCTION: SWBF2 0x00490a70
template void PblList<DustEffect>::RemoveAll();

// FUNCTION: SWBF2 0x00490250
CloudClusterClass::CloudClusterClass(unsigned int uHash)
    : EntityClass(uHash)
{
    mNumDustEffectsInCluster = 0;
    mHorizontalSpread = 60.0f;
    mVerticalSpread = 30.0f;
    mBoundingRadius = 200.0f;
}

// SYNTHETIC: SWBF2 0x00490290
// CloudClusterClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004902c0
CloudClusterClass::~CloudClusterClass()
{
}

// FUNCTION: SWBF2 0x004902d0
void CloudClusterClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xab409a38:
        ++mNumDustEffectsInCluster;
        mDustEffects[mNumDustEffectsInCluster - 1] =
            static_cast<DustEffectClass*>(EntityClass::Find(PblHash(pszValue)));
        break;

    case 0x458975f0:
        sscanf(pszValue, "%d", &mNumDustObjectsInEffects[mNumDustEffectsInCluster - 1]);
        break;

    case 0x26b090c2:
        sscanf(pszValue, "%f", &mHorizontalSpread);
        break;

    case 0xb0d0544c:
        sscanf(pszValue, "%f", &mVerticalSpread);
        break;

    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004903a0
EntityClass* CloudClusterClass::Derive(unsigned int uHash)
{
    return new CloudClusterClass(uHash);
}

// FUNCTION: SWBF2 0x00490400
Entity* CloudClusterClass::Build(const EntityDesc& desc)
{
    return new CloudClusterObject(this, *(const PblVector3*)desc.mMatrix.m[3]);
}

// FUNCTION: SWBF2 0x00490470
void CloudClusterClass::RenderDustEffects(const PblVector3& vPos, unsigned int uSeed, unsigned int uiFlags)
{
    PblMatrix matrix = gMatrixIdentity;
    static PblHeap<int, float> sHeap(20, 100000.0f, 0);
    static PblVector3 sPositions[20];
    PblRandom random = { uSeed ^ 0xc71c2830 };
    const PblMatrix& camMatrix = RedRenderer::GetCamera()->GetMatrix();

    for (int i = 0; i < mNumDustEffectsInCluster; ++i)
    {
        DustEffectClass* pEffect = mDustEffects[i];
        if (!pEffect)
        {
            continue;
        }
        matrix.m[1][0] = 0.0f;
        matrix.m[1][1] = pEffect->mHeightScale;
        matrix.m[1][2] = 0.0f;
        matrix.m[1][3] = 0.0f;
        if (pEffect->Init())
        {
            for (int j = 0; j < mNumDustObjectsInEffects[i]; ++j)
            {
                PblVector3 vOffset((random.GetFloat() - 0.5f) * mHorizontalSpread * 2.0f,
                                   (random.GetFloat() - 0.5f) * mVerticalSpread * 2.0f,
                                   (random.GetFloat() - 0.5f) * mHorizontalSpread * 2.0f);
                sPositions[j].x = vOffset.x;
                sPositions[j].y = vOffset.y;
                sPositions[j].z = vOffset.z;
                sPositions[j].x += vPos.x;
                sPositions[j].y += vPos.y;
                sPositions[j].z += vPos.z;
                const float fDistance = (camMatrix.m[3][0] - sPositions[j].x) * camMatrix.m[2][0] +
                                        (camMatrix.m[3][1] - sPositions[j].y) * camMatrix.m[2][1] +
                                        (camMatrix.m[3][2] - sPositions[j].z) * camMatrix.m[2][2];
                if (pEffect->mMaxDistance > fDistance &&
                    (pEffect->mMinDistance <= 0.0f || fDistance > pEffect->mMinDistance))
                {
                    sHeap.Push(j, fDistance);
                }
            }
        }
        while (!sHeap.IsEmpty())
        {
            const float fDistance = sHeap.Value(0);
            const PblVector3& vObject = sPositions[sHeap.Get(0)];
            matrix.m[3][0] = vObject.x;
            matrix.m[3][1] = vObject.y;
            matrix.m[3][2] = vObject.z;
            matrix.m[3][3] = 1.0f;
            float fAlpha = (pEffect->mMaxDistance - fDistance) * 0.002f;
            if (!(1.0f > fAlpha))
            {
                fAlpha = 1.0f;
            }
            if (pEffect->mMinDistance > 0.0f && pEffect->mMinDistance + 200.0f > fDistance)
            {
                fAlpha = (fDistance - pEffect->mMinDistance) * 0.005f * fAlpha;
            }
            pEffect->Render(matrix, fAlpha * 0.9f, uiFlags);
            sHeap.Pop();
        }
    }
}

// FUNCTION: SWBF2 0x00490910
CloudClusterObject::CloudClusterObject(CloudClusterClass* pClass, const PblVector3& vPosition)
    : RedSceneObject(&cloudClusterObjectLodData)
{
    mPosition = vPosition;
    mSeed = PblRandom::GetGlobalInt();
    mClass = pClass;
    SetRenderPos(mPosition);
    SetRenderRadius(mClass->mBoundingRadius);
    Activate();
}

// FUNCTION: SWBF2 0x00490a10
CloudClusterObject::~CloudClusterObject()
{
}

// SYNTHETIC: SWBF2 0x00490b05
// [thunk]:CloudClusterObject::`scalar deleting destructor' adjustor{88}
// SYNTHETIC: SWBF2 0x004909e0
// CloudClusterObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00490a30
void CloudClusterObject::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (uiFlags & 0x200000)
    {
        return;
    }
    if (RedSceneManager::InTransparentPhase())
    {
        mClass->RenderDustEffects(mPosition, mSeed, uiFlags);
    }
    else
    {
        RedSceneManager::AddTransparentObject(this);
    }
}
