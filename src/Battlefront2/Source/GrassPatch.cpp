#include <math.h>
#include <stdio.h>

#include "GrassPatch.h"
#include "Battlefront2Lod.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "CameraManager.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

class PblMatrix;
extern PblMatrix gMatrixIdentity;
extern NoLodData noloddata;

// GLOBAL: SWBF2 0x007eb9bc
PblList<GrassPatchClass> GrassPatchClass::sClasses;

// GLOBAL: SWBF2 0x007eb9e8
PblList<GrassPatch> GrassPatchClass::sObjects;

static bool bGrassPatchEnable = true;

// FUNCTION: SWBF2 0x00540d10
GrassPatchClass::GrassPatchClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
{
    mBoundingRadius = 64.0f;
    mParticle = 0;
    mNumParticles = 50;
    mSizeMin = 10.0f;
    mSizeMax = 15.0f;
    mYOffset = 0.0f;
    mAlpha = 0.5f;
    mMaxDistance = 100.0f;
    mTextureHash = 0xB26FAC7Du;
    mRadiusFadeMin = -1.0f;
    mRadiusFadeMax = -1.0f;
    mDarknessMin = 1.0f;
    mDarknessMax = 1.0f;
    mNumParts = 4;
    mBoxX = -1.0f;
    mBoxZ = -1.0f;
    mFlatHeightMin = 0.2f;
    mFlatHeightMax = 0.2f;
    mFlatSizeMultiplier = 2.0f;
    mFlatFaceFactor = 0.5f;
    mFlatShadowHeight = 0.0f;
    mMaxSkew = 0.1f;
    mFlatCount = 6;
    mSkinnyFactor = 0.3f;
    mFlatGrassSwingFlag = false;
    mRenderData = 0;
    mTransparentFlag = false;

    sClasses.AddLast(&mClassNode, this);
}

// FUNCTION: SWBF2 0x005409e0
GrassPatch::GrassPatch(GrassPatchClass* pClass, const PblMatrix& matrix)
    : RedSceneObject(&noloddata)
    , mClass(pClass)
{
    mMatrix = gMatrixIdentity;

    mMatrix.m[1][0] = matrix.m[1][0];
    mMatrix.m[1][1] = matrix.m[1][1];
    mMatrix.m[1][2] = matrix.m[1][2];
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

    PblVector3 vCenter;
    D3DXVec3TransformCoord(&vCenter, &mClass->mBoundingOffset, &mMatrix);
    SetRenderPos(vCenter);
    SetRenderRadius(mClass->mBoundingRadius);
    Activate();

    GrassPatchClass::sObjects.AddLast(&mObjectNode, this);
}

// FUNCTION: SWBF2 0x00540ca0
GrassPatch::~GrassPatch()
{
}

// SYNTHETIC: SWBF2 0x005421b5
// [thunk]:GrassPatch::`scalar deleting destructor' adjustor{88}
// SYNTHETIC: SWBF2 0x00540c70
// GrassPatch::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00540cc0
bool GrassPatch::IsEnabled()
{
    return bGrassPatchEnable;
}

// FUNCTION: SWBF2 0x00540ef0
GrassPatchClass::~GrassPatchClass()
{
    if (mParticle)
    {
        delete mParticle;
    }
    DestroyRenderData();
}

// SYNTHETIC: SWBF2 0x00540ec0
// GrassPatchClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005421bd
// [thunk]:GrassPatchClass::`scalar deleting destructor' adjustor{24}

// FUNCTION: SWBF2 0x00540f70
void GrassPatchClass::CreateClient()
{
    for (PblList<GrassPatch>::Itor it(sObjects); (GrassPatch*)it; ++it)
    {
        it->Deactivate();
    }
    NetGame::EnterClient();
    for (PblList<GrassPatch>::Itor it(sObjects); (GrassPatch*)it; ++it)
    {
        it->Activate();
    }
    NetGame::LeaveClient();

    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->DeactivateThread();
        }
    }
    NetGame::EnterClient();
    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->ActivateThread(0, PER_FRAME);
        }
    }
    NetGame::LeaveClient();
}

// FUNCTION: SWBF2 0x00541020
void GrassPatchClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xD133F7DA:
        if (sscanf(pszValue, "%i", &mNumParticles))
        {
            mNumParticles /= CameraManager::Instance()->GetNumCameras();
        }
        break;
    case 0xEAEBCF6E:
        sscanf(pszValue, "%f", &mSizeMin);
        break;
    case 0x80BA1308:
        sscanf(pszValue, "%f", &mSizeMax);
        break;
    case 0x3D5D0AEF:
        sscanf(pszValue, "%f", &mYOffset);
        break;
    case 0x5D8B6DAB:
        sscanf(pszValue, "%f", &mAlpha);
        break;
    case 0xE6915CF6:
        sscanf(pszValue, "%f", &mMaxDistance);
        break;
    case 0x3C6468F4:
        mTextureHash = PblHash(pszValue);
        break;
    case 0x80887A6F:
        sscanf(pszValue, "%f", &mRadiusFadeMin);
        break;
    case 0x6E74D661:
        sscanf(pszValue, "%f", &mRadiusFadeMax);
        break;
    case 0x13FAC25E:
        sscanf(pszValue, "%f", &mDarknessMin);
        break;
    case 0x25E5C3FC:
        sscanf(pszValue, "%f", &mDarknessMax);
        break;
    case 0x5AC02207:
        sscanf(pszValue, "%i", &mNumParts);
        break;
    case 0x35A4393B:
        sscanf(pszValue, "%f %f", &mBoxX, &mBoxZ);
        break;
    case 0x231FCC79:
        sscanf(pszValue, "%f %f", &mFlatHeightMin, &mFlatHeightMax);
        break;
    case 0xA9B14958:
        sscanf(pszValue, "%f", &mFlatSizeMultiplier);
        break;
    case 0xF6582B9C:
        sscanf(pszValue, "%f", &mFlatFaceFactor);
        break;
    case 0xC75263E1:
        sscanf(pszValue, "%f", &mFlatShadowHeight);
        break;
    case 0x97133EEB:
        sscanf(pszValue, "%f", &mMaxSkew);
        break;
    case 0x4B6D5171:
        sscanf(pszValue, "%i", &mFlatCount);
        break;
    case 0xFF7BB39A:
        sscanf(pszValue, "%f", &mSkinnyFactor);
        break;
    case 0x604DB82E:
        mFlatGrassSwingFlag = true;
        break;
    case 0x9C3F7A19:
        mTransparentFlag = true;
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00541280
EntityClass* GrassPatchClass::Derive(unsigned int uHash)
{
    return new GrassPatchClass(uHash);
}

// FUNCTION: SWBF2 0x005412f0
void GrassPatchClass::Init()
{
    if (mParticle)
    {
        return;
    }

    mParticle = new GrassParticle[mNumParticles];

    PblVector3 vMin(100.0f, 100.0f, 100.0f);
    PblVector3 vMax(-100.0f, -100.0f, -100.0f);

    for (int i = 0; i < mNumParticles; ++i)
    {
        GrassParticle& particle = mParticle[i];
        float fFade;

        if (mBoxX > 0.0f || mBoxZ > 0.0f)
        {
            const float fZ = (PblRandom::GetGlobalFloat() - 0.5f) * mBoxZ;
            const float fX = (PblRandom::GetGlobalFloat() - 0.5f) * mBoxX;
            particle.mPos = PblVector3(fX, 0.0f, fZ);
            fFade = 1.0f;
        }
        else
        {
            fFade = 0.0f;
            while (fFade == 0.0f)
            {
                const float fZ = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mRadiusFadeMax;
                const float fX = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mRadiusFadeMax;
                particle.mPos = PblVector3(fX, 0.0f, fZ);

                const float fDist = sqrtf(particle.mPos.x * particle.mPos.x
                                          + particle.mPos.y * particle.mPos.y
                                          + particle.mPos.z * particle.mPos.z);
                if (fDist < mRadiusFadeMin)
                {
                    fFade = 1.0f;
                }
                else if (fDist < mRadiusFadeMax)
                {
                    fFade = 1.0f - (fDist - mRadiusFadeMin) / (mRadiusFadeMax - mRadiusFadeMin);
                }
            }
        }

        particle.mSize = (mSizeMax - mSizeMin) * PblRandom::GetGlobalFloat() + mSizeMin;

        const float fDarkness = (mDarknessMax - mDarknessMin) * PblRandom::GetGlobalFloat() + mDarknessMin;
        particle.mColor.a = mAlpha;
        particle.mColor.r = fDarkness;
        particle.mColor.g = fDarkness;
        particle.mColor.b = fDarkness;

        float fA, fB;
        do
        {
            fA = PblRandom::GetGlobalFloat() - 0.5f;
            fB = PblRandom::GetGlobalFloat() - 0.5f;
        } while (fA * fA + fB * fB == 0.0f);

        particle.mTime = PblRandom::GetGlobalFloat() * 10.0f;
        particle.mOffset = 0.0f;
        particle.mLODPriority = 0.0f;
        particle.mFlatType = false;
        particle.mSkew = (PblRandom::GetGlobalFloat() - 0.5f) * (mMaxSkew * 2.0f);

        if (mNumParts == 3 || mNumParts > 4)
        {
            bool bFlat = false;
            if (mNumParts == 3)
            {
                particle.mTexture = (unsigned char)(i % (mFlatCount + 2));
                if (particle.mTexture > 1)
                {
                    particle.mTexture = 2;
                    bFlat = true;
                }
            }
            else
            {
                particle.mTexture = (unsigned char)(i % (mFlatCount + 4));
                while (particle.mTexture > 5)
                {
                    particle.mTexture -= 2;
                }
                bFlat = particle.mTexture > 3;
            }

            particle.mPos.y = mYOffset;
            if (bFlat)
            {
                particle.mHeight = particle.mSize * mFlatSizeMultiplier;
                particle.mSize = mFlatSizeMultiplier * particle.mSize;
                particle.mUpFactor = PblRandom::GetGlobalFloat() * 0.1f + 0.05f;
                particle.mPos.y += ((mFlatHeightMax - mFlatHeightMin) * PblRandom::GetGlobalFloat()
                                    * (1.0f - fFade) + mFlatHeightMin) * mSizeMin;
                particle.mFlatType = true;

                do
                {
                    const float fZ = PblRandom::GetGlobalFloat() - 0.5f;
                    const float fX = PblRandom::GetGlobalFloat() - 0.5f;
                    particle.mUp = PblVector3(fX, 0.0f, fZ);
                } while (fabsf(particle.mUp.x) < 0.01f && fabsf(particle.mUp.z) < 0.01f);
                D3DXVec3Normalize(&particle.mUp, &particle.mUp);
                particle.mUp.y = (PblRandom::GetGlobalFloat() - 0.5f) * 0.3f;
                D3DXVec3Normalize(&particle.mUp, &particle.mUp);
            }
            else
            {
                particle.mHeight = particle.mSize;
                particle.mSize = mSkinnyFactor * particle.mSize;
                particle.mUpFactor = PblRandom::GetGlobalFloat() * 0.2f + 0.8f;
                particle.mLODPriority = 10000.0f;
            }
        }
        else
        {
            particle.mUpFactor = 1.0f;
            particle.mHeight = particle.mSize * 0.9f;
            particle.mSize = particle.mSize * 1.1f;
            particle.mTexture = (unsigned char)(i % 4);
            particle.mPos.y = mYOffset;
            particle.mPos.y += (1.0f - fFade) * (mSizeMin * -0.5f);
        }

        const float fHalfSize = particle.mSize * 0.5f;
        if (particle.mPos.x - fHalfSize <= vMin.x)
        {
            vMin.x = particle.mPos.x - fHalfSize;
        }
        if (particle.mPos.y - particle.mSize <= vMin.y)
        {
            vMin.y = particle.mPos.y - particle.mSize;
        }
        if (particle.mPos.z - fHalfSize <= vMin.z)
        {
            vMin.z = particle.mPos.z - fHalfSize;
        }
        if (particle.mPos.x + fHalfSize >= vMax.x)
        {
            vMax.x = particle.mPos.x + fHalfSize;
        }
        if (particle.mPos.y + particle.mSize >= vMax.y)
        {
            vMax.y = particle.mPos.y + particle.mSize;
        }
        if (particle.mPos.z + fHalfSize >= vMax.z)
        {
            vMax.z = particle.mPos.z + fHalfSize;
        }
    }

    mBoundingOffset.x = (vMax.x + vMin.x) * 0.5f;
    mBoundingOffset.y = (vMax.y + vMin.y) * 0.5f;
    mBoundingOffset.z = (vMax.z + vMin.z) * 0.5f;
    mBoundingRadius = 0.0f;
    mBoundingBoxRadius.x = vMax.x - mBoundingOffset.x;
    mBoundingBoxRadius.y = vMax.y - mBoundingOffset.y;
    mBoundingBoxRadius.z = vMax.z - mBoundingOffset.z;

    for (int i = 0; i < mNumParticles; ++i)
    {
        const PblVector3& vPos = mParticle[i].mPos;
        const float fX = vPos.x - mBoundingOffset.x;
        const float fY = vPos.y - mBoundingOffset.y;
        const float fZ = vPos.z - mBoundingOffset.z;
        const float fRadius = sqrtf(fX * fX + fY * fY + fZ * fZ) + mParticle[i].mSize;
        if (fRadius > mBoundingRadius)
        {
            mBoundingRadius = fRadius;
        }
    }

    ActivateThread(0, PER_FRAME);
    CreateRenderData();
}

// FUNCTION: SWBF2 0x00541f80
Entity* GrassPatchClass::Build(const EntityDesc& desc)
{
    if (!mParticle)
    {
        Init();
    }
    return new GrassPatch(this, desc.mMatrix);
}

// FUNCTION: SWBF2 0x00542000
bool GrassPatchClass::Update(float fDeltaTime)
{
    const float fSwing = fDeltaTime * 0.8f;
    for (int i = 0; i < mNumParticles; ++i)
    {
        mParticle[i].mTime += ((float)(i % 5) * 0.2f + 1.0f) * fSwing;
        mParticle[i].mOffset = PblMath::Sin(mParticle[i].mTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x005420c0
void GrassPatchClass::Render(const PblMatrix& matrix, const GrassPatch*, unsigned int uiFlags) const
{
    const GrassPatchClass* pClass = (const GrassPatchClass*)this;
    const RedColorValue color(1.0f, 1.0f, 1.0f, 1.0f);
    pClass->RenderMaxDist(matrix, pClass->mMaxDistance, color, uiFlags);
}

// FUNCTION: SWBF2 0x00540cd0
void GrassPatch::Render(unsigned char, float, unsigned int uiFlags)
{
    if (RedSceneManager::InTransparentPhase())
    {
        ((GrassPatchClass*)mClass)->Render(mMatrix, this, uiFlags);
    }
    else
    {
        RedSceneManager::AddTransparentObject(this);
    }
}

// FUNCTION: SWBF2 0x005420f0
void GrassPatchClass::Reset()
{
    for (PblList<GrassPatchClass>::Itor it(sClasses); (GrassPatchClass*)it; ++it)
    {
        it->DestroyRenderData();
    }

    sClasses.RemoveAll();
    sObjects.RemoveAll();
}

// FUNCTION: SWBF2 0x00542170
template void PblList<GrassPatchClass>::RemoveAll();
