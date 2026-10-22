#include <math.h>
#include <stdio.h>

#include "LeafPatch.h"
#include "Battlefront2Lod.h"
#include "CollisionManager.h"
#include "CollisionUtil.h"
#include "LeafEffect.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "CameraManager.h"
#include "NetGame.h"

#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"

// SYNTHETIC: SWBF2 0x0056dbd0
// LeafPatchClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056dcb0
LeafPatchClass::~LeafPatchClass()
{
    if (mParticle)
    {
        delete mParticle;
    }
    DestroyRenderData();
}

// SYNTHETIC: SWBF2 0x0056f41d
// [thunk]:LeafPatchClass::`scalar deleting destructor' adjustor{24}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0056dc00
void LeafPatchClass::CreateClient()
{
    for (PblList<LeafPatch>::Itor it(sObjects); (LeafPatch*)it; ++it)
    {
        it->Deactivate();
    }
    NetGame::EnterClient();
    for (PblList<LeafPatch>::Itor it(sObjects); (LeafPatch*)it; ++it)
    {
        it->Activate();
    }
    NetGame::LeaveClient();

    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->DeactivateThread();
        }
    }
    NetGame::EnterClient();
    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        if (it->mParticle)
        {
            it->ActivateThread(0, PER_FRAME);
        }
    }
    NetGame::LeaveClient();
}

// GLOBAL: SWBF2 0x007ebaec
PblList<LeafPatchClass> LeafPatchClass::sClasses;

// FUNCTION: SWBF2 0x0056d9b0
LeafPatchClass::LeafPatchClass(unsigned int uHash)
    : Thread()
    , EntityClass(uHash)
{
    mBoundingRadius = 10.0f;
    mMaxFallingLeaves = 0;
    mMaxScatterBirds = 0;
    mRadius = 5.0f;
    mHeightScale = 1.0f;
    mHeight = 5.0f;
    mSeed = (int)PblRandom::GetGlobalInt();
    mType = 0;
    mParticle = 0;
    mNumParticles = 50;
    mOffset.x = 0.0f;
    mOffset.y = 0.0f;
    mOffset.z = 0.0f;
    mSizeMin = 10.0f;
    mSizeMax = 15.0f;
    mAlpha = 0.5f;
    mMaxDistance = 100.0f;
    mConeHeight = 0.0f;
    mBoxSize.x = 0.0f;
    mBoxSize.y = 0.0f;
    mBoxSize.z = 0.0f;
    mTextureHash = 0xB26FAC7Du;
    mDarknessMin = 1.0f;
    mDarknessMax = 1.0f;
    mNearFadeDist = 6.0f;
    mNumParts = 4;
    mVineXSpread = 1.0f;
    mVineZSpread = 0.0f;
    mVineLengthMin = 2;
    mVineLengthMax = 4;
    mVineSpread = 0.6f;
    mWiggleSpeed = 1.2f;
    mWiggleAmount = 0.11f;
    mFadeInStep = 1.0f;
    mRenderData = 0;

    sClasses.AddLast(&mClassNode, this);
}

// GLOBAL: SWBF2 0x01e573d4
int sLeafParticleIndex;

// FUNCTION: SWBF2 0x0056dd30
void LeafPatchClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0xD133F7DA:
        if (sscanf(pszValue, "%i", &mNumParticles))
        {
            mNumParticles /= CameraManager::Instance()->GetNumCameras();
        }
        mNumVisible = mNumParticles;
        break;
    case 0x1ABD7877:
        sscanf(pszValue, "%i", &mNumVisible);
        mFadeInStep = 1.0f / ((float)mNumVisible * 0.2f);
        break;
    case 0x0DBA4CB3:
        sscanf(pszValue, "%f", &mRadius);
        break;
    case 0xCAAA3778:
        sscanf(pszValue, "%f", &mHeightScale);
        break;
    case 0xD5BDBB42:
        sscanf(pszValue, "%f", &mHeight);
        break;
    case 0x5045BCAC:
        sscanf(pszValue, "%i", &mSeed);
        break;
    case 0x14C8D3CA:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mOffset.x = fX;
        mOffset.y = fY;
        mOffset.z = fZ;
        break;
    }
    case 0xEAEBCF6E:
        sscanf(pszValue, "%f", &mSizeMin);
        break;
    case 0x80BA1308:
        sscanf(pszValue, "%f", &mSizeMax);
        break;
    case 0x5D8B6DAB:
        sscanf(pszValue, "%f", &mAlpha);
        break;
    case 0xE6915CF6:
        sscanf(pszValue, "%f", &mMaxDistance);
        break;
    case 0xEAAAA7A9:
        sscanf(pszValue, "%f", &mConeHeight);
        break;
    case 0x35A4393B:
    {
        float fX, fY, fZ;
        sscanf(pszValue, "%f %f %f", &fX, &fY, &fZ);
        mBoxSize.x = fX;
        mBoxSize.y = fY;
        mBoxSize.z = fZ;
        mType = 2;
        break;
    }
    case 0x3C6468F4:
        mTextureHash = PblHash(pszValue);
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
    case 0xFD7FAE90:
    {
        int iCount;
        sscanf(pszValue, "%i", &iCount);
        mMaxFallingLeaves = (unsigned char)(iCount > 50 ? 50 : iCount);
        break;
    }
    case 0x3B1948F3:
    {
        int iCount;
        sscanf(pszValue, "%i", &iCount);
        mMaxScatterBirds = (unsigned char)(iCount > 5 ? 5 : iCount);
        break;
    }
    case 0xCDBC383D:
        sscanf(pszValue, "%f %f", &mVineXSpread, &mVineZSpread);
        mType = 3;
        break;
    case 0xA8C0D41D:
        sscanf(pszValue, "%i %i", &mVineLengthMin, &mVineLengthMax);
        break;
    case 0x45328DC6:
        sscanf(pszValue, "%f", &mVineSpread);
        break;
    case 0xD4CBD35B:
        sscanf(pszValue, "%f", &mWiggleSpeed);
        break;
    case 0x7CBB796C:
        sscanf(pszValue, "%f", &mWiggleAmount);
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x0056e110
EntityClass* LeafPatchClass::Derive(unsigned int uHash)
{
    return new LeafPatchClass(uHash);
}

// FUNCTION: SWBF2 0x0056e180
void LeafPatchClass::CreateVineBranch(PblVector3 vPos, PblVector3 vDir, int iCount, PblRandom& rand)
{
    int iLeft = iCount - 1;
    if (iLeft < 0)
    {
        return;
    }

    mParticle[sLeafParticleIndex].mPos = vPos;
    float fScale = ((float)iLeft + 1.0f) * 0.25f;
    float fSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
    if (fScale >= 1.0f)
    {
        fScale = 1.0f;
    }
    fSize = fScale * fSize;
    mParticle[sLeafParticleIndex++].mSize = fSize;

    int iLength = (int)((float)(mVineLengthMax - mVineLengthMin) * rand.GetFloat() + (float)mVineLengthMin);
    if (iLength > iLeft)
    {
        iLength = iLeft;
    }

    for (; iLength != 0; --iLength)
    {
        const PblVector3 vWander((rand.GetFloat() - rand.GetFloat()) * 0.5f * mVineXSpread, 0.2f,
                                 (rand.GetFloat() - rand.GetFloat()) * 0.5f * mVineZSpread);
        vDir.x += vWander.x;
        vDir.y += vWander.y;
        vDir.z += vWander.z;
        PblVector3 vNorm;
        D3DXVec3Normalize(&vNorm, &vDir);
        vDir = vNorm;

        vPos.x += vDir.x * fSize * mVineSpread;
        vPos.y += vDir.y * fSize * mVineSpread;
        vPos.z += vDir.z * fSize * mVineSpread;

        fScale = ((float)iLeft + 1.0f) * 0.25f;
        fSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
        if (fScale >= 1.0f)
        {
            fScale = 1.0f;
        }
        --iLeft;
        fSize = fScale * fSize;
        mParticle[sLeafParticleIndex].mPos = vPos;
        mParticle[sLeafParticleIndex++].mSize = fSize;
    }

    if (iLeft > 0)
    {
        PblVector3 vTarget((rand.GetFloat() - rand.GetFloat()) * mVineXSpread, 0.0f,
                           (rand.GetFloat() - rand.GetFloat()) * mVineZSpread);
        PblVector3 vBranch = Interpolate(vDir, vTarget, 0.8f);
        PblVector3 vNorm;
        D3DXVec3Normalize(&vNorm, &vBranch);
        vBranch = vNorm;
        const int iSplit = (int)((rand.GetFloat() * 0.5f + 0.25f) * (float)iLeft);
        CreateVineBranch(PblVector3(vBranch.x * fSize * mVineSpread + vPos.x,
                                    vBranch.y * fSize * mVineSpread + vPos.y,
                                    vBranch.z * fSize * mVineSpread + vPos.z),
                         vBranch, iSplit, rand);

        vTarget = PblVector3((rand.GetFloat() - rand.GetFloat()) * mVineXSpread, 0.0f,
                             (rand.GetFloat() - rand.GetFloat()) * mVineZSpread);
        vBranch = Interpolate(vDir, vTarget, 0.8f);
        D3DXVec3Normalize(&vNorm, &vBranch);
        vBranch = vNorm;
        CreateVineBranch(PblVector3(vBranch.x * fSize * mVineSpread + vPos.x,
                                    vBranch.y * fSize * mVineSpread + vPos.y,
                                    vBranch.z * fSize * mVineSpread + vPos.z),
                         vBranch, iLeft - iSplit, rand);
    }
}

// FUNCTION: SWBF2 0x0056e970
void LeafPatchClass::Init()
{
    if (mParticle)
    {
        return;
    }

    mParticle = new LeafParticle[mNumParticles];

    const float fInvHeightScale = 1.0f / mHeightScale;
    const PblVector3 vOffset(mOffset.x, mOffset.y * fInvHeightScale, mOffset.z);

    PblRandom rand(mSeed ^ 0xC71C2830u);
    rand.GetInt();

    char cQuadrant = 0;

    if (mType == 3)
    {
        sLeafParticleIndex = 0;
        CreateVineBranch(PblVector3(0.0f, 0.0f, 0.0f), PblVector3(0.0f, 1.0f, 0.0f), mNumParticles, rand);
    }

    PblVector3 vMin(100.0f, 100.0f, 100.0f);
    PblVector3 vMax(-100.0f, -100.0f, -100.0f);

    for (int i = 0; i < mNumParticles; ++i)
    {
        LeafParticle& particle = mParticle[i];

        if (mType != 3)
        {
            if (mType == 2)
            {
                particle.mPos = PblVector3((rand.GetFloat() - 0.5f) * mBoxSize.x,
                                           (rand.GetFloat() - 0.5f) * mBoxSize.y,
                                           (rand.GetFloat() - 0.5f) * mBoxSize.z);
            }
            else
            {
                do
                {
                    particle.mPos = PblVector3(rand.GetFloat() * mRadius * 1.5f, rand.GetFloat() * mRadius,
                                               rand.GetFloat() * mRadius * 1.5f);
                    if (cQuadrant < 2)
                    {
                        particle.mPos.x = -particle.mPos.x;
                    }
                    if (cQuadrant == 1 || cQuadrant == 2)
                    {
                        particle.mPos.z = -particle.mPos.z;
                    }
                    cQuadrant = (char)((cQuadrant + 1) % 4);
                } while (particle.mPos.x * particle.mPos.x + particle.mPos.y * particle.mPos.y
                             + particle.mPos.z * particle.mPos.z
                         > mRadius * mRadius);

                particle.mPos.y = fInvHeightScale * mHeight / mRadius * particle.mPos.y;
                if (mConeHeight > 0.0f && particle.mPos.y > mHeight - mConeHeight)
                {
                    const float fPinch = (mHeight - particle.mPos.y) / mConeHeight;
                    particle.mPos.x *= fPinch;
                    particle.mPos.z *= fPinch;
                }
            }
            particle.mSize = (mSizeMax - mSizeMin) * rand.GetFloat() + mSizeMin;
        }

        const float fDarknessRange = mDarknessMax - mDarknessMin;
        const RedDirectionalLight* pLight = RedLight::m_sGlobalDirectionals[0];
        const float fDarkness = rand.GetFloat() * fDarknessRange * 0.5f + mDarknessMin;
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &particle.mPos);
        particle.mColor.a = mAlpha;
        const float fShade = fDarkness
                             - (pLight->m_vDirection.y * vNormal.y + vNormal.x * pLight->m_vDirection.x
                                + pLight->m_vDirection.z * vNormal.z)
                                   * fDarknessRange * 0.5f;
        particle.mColor.r = fShade;
        particle.mColor.g = fShade;
        particle.mColor.b = fShade;
        particle.mTime = rand.GetFloat() * 100.0f;
        particle.mTexture = (unsigned char)(i % 4);
        particle.mPos.x += vOffset.x;
        particle.mPos.y += vOffset.y;
        particle.mPos.z += vOffset.z;

        const float fHalfSize = particle.mSize * 0.5f;
        if (particle.mPos.x - fHalfSize <= vMin.x)
        {
            vMin.x = particle.mPos.x - fHalfSize;
        }
        if ((particle.mPos.y - fHalfSize) * mHeightScale <= vMin.y)
        {
            vMin.y = (particle.mPos.y - fHalfSize) * mHeightScale;
        }
        if (particle.mPos.z - fHalfSize <= vMin.z)
        {
            vMin.z = particle.mPos.z - fHalfSize;
        }
        if (fHalfSize + particle.mPos.x >= vMax.x)
        {
            vMax.x = fHalfSize + particle.mPos.x;
        }
        if ((fHalfSize + particle.mPos.y) * mHeightScale >= vMax.y)
        {
            vMax.y = (fHalfSize + particle.mPos.y) * mHeightScale;
        }
        if (fHalfSize + particle.mPos.z >= vMax.z)
        {
            vMax.z = fHalfSize + particle.mPos.z;
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
        const LeafParticle& particle = mParticle[i];
        const float fX = particle.mPos.x - mBoundingOffset.x;
        const float fY = particle.mPos.y * mHeightScale - mBoundingOffset.y;
        const float fZ = particle.mPos.z - mBoundingOffset.z;
        const float fHalfSize = particle.mSize * 0.5f;
        const float fDist = sqrtf(fX * fX + fY * fY + fZ * fZ);
        const float fScaled = mHeightScale * fHalfSize;
        const float fRadius = fDist + (fHalfSize > fScaled ? fHalfSize : fScaled);
        mBoundingRadius = fRadius > mBoundingRadius ? fRadius : mBoundingRadius;
    }

    ActivateThread(0, PER_FRAME);
    CreateRenderData();
}

// FUNCTION: SWBF2 0x0056f240
Entity* LeafPatchClass::Build(const EntityDesc& desc)
{
    if (!mParticle)
    {
        Init();
    }
    return new LeafPatch(this, desc.mMatrix);
}

// FUNCTION: SWBF2 0x0056f2d0
bool LeafPatchClass::Update(float fDeltaTime)
{
    for (int i = 0; i < mNumParticles; ++i)
    {
        mParticle[i].mTime += mWiggleSpeed * fDeltaTime;
        mParticle[i].mRotation = PblMath::Sin(mParticle[i].mTime) * mWiggleAmount;
    }
    return true;
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;
extern NoLodData noloddata;
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// FUNCTION: SWBF2 0x0056cf20
LeafPatch::LeafPatch(LeafPatchClass* pClass, const PblMatrix& matrix)
    : RedSceneObject(&noloddata)
    , CollisionObject(COLL_LEAFPATCH, 0, 0, 0)
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

    SetCollisionCenter(*(const PblVector3*)mMatrix.m[3]);
    SetCollisionRadius(mClass->mRadius);
    Set2DCollisionRadius(mClass->mRadius);

    PblVector3 vCenter;
    D3DXVec3TransformCoord(&vCenter, &mClass->mBoundingOffset, &mMatrix);
    SetRenderPos(vCenter);
    SetRenderRadius(mClass->mBoundingRadius);
    Activate();

    mLod = 0.0f;
    mNumFallingLeaves = 0;
    const float fShade = PblRandom::_Global.GetFloat() * 0.2f;

    mColor.b = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    mColor.g = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    mColor.r = PblRandom::_Global.GetFloat() * 0.1f + fShade + 0.7f;
    mColor.a = 1.0f;

    if (mClass->mType != 3 && mClass->mMaxFallingLeaves > 0)
    {
        CollisionManager::AddLeafPatchBody(this);
    }

    mMaxDistance = mClass->mMaxDistance;
    LeafPatchClass::sObjects.AddLast(&mObjectNode, this);
}

// FUNCTION: SWBF2 0x0056d430
LeafPatch::~LeafPatch()
{
}

// SYNTHETIC: SWBF2 0x0056f415
// [thunk]:LeafPatch::`scalar deleting destructor' adjustor{88}
// SYNTHETIC: SWBF2 0x0056f425
// [thunk]:LeafPatch::`scalar deleting destructor' adjustor{96}
// SYNTHETIC: SWBF2 0x0056d400
// LeafPatch::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0056cef0
void LeafPatch::DeactivateLeaf()
{
    --mNumFallingLeaves;
}

// FUNCTION: SWBF2 0x0056cf00
const PblVector3& LeafPatch::GetPosition() const
{
    return *(const PblVector3*)mMatrix.m[3];
}

// FUNCTION: SWBF2 0x0056cf10
const PblMatrix& LeafPatch::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x0056d4a0
void LeafPatch::SetProperty(unsigned int uProperty, const char* pszValue)
{
    if (uProperty == 0xE6915CF6)
    {
        sscanf(pszValue, "%f", &mMaxDistance);
    }
}

// FUNCTION: SWBF2 0x0056d6f0
bool LeafPatch::RayTest(const PblVector3& vStart, const PblVector3& vDir, float fMaxT)
{
    const float fRadiusSq = GetCollisionRadius() * GetCollisionRadius();
    const PblVector3 vEnd(vStart.x + vDir.x * fMaxT, vStart.y + vDir.y * fMaxT, vStart.z + vDir.z * fMaxT);
    const PblVector3& vCenter = GetCollisionCenter();
    float dx = vEnd.x - vCenter.x;
    float dy = vEnd.y - vCenter.y;
    float dz = vEnd.z - vCenter.z;
    if (dy * dy + dx * dx + dz * dz > fRadiusSq)
    {
        return false;
    }
    const PblVector3& vCenter2 = GetCollisionCenter();
    dx = vStart.x - vCenter2.x;
    dy = vStart.y - vCenter2.y;
    dz = vStart.z - vCenter2.z;
    return fRadiusSq <= dy * dy + dx * dx + dz * dz;
}

// FUNCTION: SWBF2 0x0056d4d0
float LeafPatch::RayHit(const PblVector3& vStart, const PblVector3& vDir, float fMaxT,
                        PblVector3* pNormal, int* pMaterial, bool)
{
    const PblMatrix inverse = mMatrix.GetInverseFast();
    PblVector3 vLocalStart;
    D3DXVec3TransformCoord(&vLocalStart, &vStart, &inverse);
    PblVector3 vLocalDir;
    D3DXVec3TransformNormal(&vLocalDir, &vDir, &inverse);
    if (0.0f >= mClass->mRadius || 0.0f >= mClass->mHeight)
    {
        return 1.0f;
    }
    vLocalStart.x /= mClass->mRadius;
    vLocalStart.y /= mClass->mHeight;
    vLocalStart.z /= mClass->mRadius;
    vLocalDir.x /= mClass->mRadius;
    vLocalDir.y /= mClass->mHeight;
    vLocalDir.z /= mClass->mRadius;
    const float fLength = (float)sqrt((double)(vLocalDir.x * vLocalDir.x + vLocalDir.y * vLocalDir.y +
                                               vLocalDir.z * vLocalDir.z));
    const float fInv = 1.0f / fLength;
    const PblVector3 vUnit(vLocalDir.x * fInv, vLocalDir.y * fInv, vLocalDir.z * fInv);
    const float fHit = CollisionUtil::RayHitSphere(vLocalStart, vUnit, fLength * fMaxT,
                                                   PblVector3(0.0f, 0.0f, 0.0f), 1.0f, pNormal);
    if (pMaterial)
    {
        *pMaterial = 0;
    }
    if (pNormal)
    {
        pNormal->x *= mClass->mRadius;
        pNormal->y *= mClass->mHeight;
        pNormal->z *= mClass->mRadius;
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, pNormal);
        *pNormal = vNormal;
    }
    return fHit;
}

// FUNCTION: SWBF2 0x0056d800
void LeafPatch::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (!RedSceneManager::InTransparentPhase())
    {
        RedSceneManager::AddTransparentObject(this);
        return;
    }
    if (!mClass->RenderMaxDist(mMatrix, fDist, uiFlags, FLRenderer::GetNearSceneFadeEnd(), mColor))
    {
        return;
    }
    if (mNumFallingLeaves >= mClass->mMaxFallingLeaves)
    {
        return;
    }
    for (int i = mClass->mMaxFallingLeaves - mNumFallingLeaves; i > 0; --i)
    {
        FallingLeafParticle* pLeaf = LeafParticleManager::GetFreeLeaf();
        if (!pLeaf)
        {
            continue;
        }
        PblVector3 vPos = *(const PblVector3*)mMatrix.m[3];
        const float fX = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mRadius;
        const float fY = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mHeight * mClass->mHeightScale;
        const float fZ = (PblRandom::GetGlobalFloat() * 2.0f - 1.0f) * mClass->mRadius;
        vPos.x += fX;
        vPos.y += fY;
        vPos.z += fZ;
        pLeaf->RespawnLeaf(vPos, this, LEAF_NONE);
        ++mNumFallingLeaves;
    }
}

// FUNCTION: SWBF2 0x0056f350
void LeafPatchClass::Reset()
{
    for (PblList<LeafPatchClass>::Itor it(sClasses); (LeafPatchClass*)it; ++it)
    {
        it->DestroyRenderData();
    }

    sClasses.RemoveAll();
    sObjects.RemoveAll();
}

// FUNCTION: SWBF2 0x0056f3d0
template void PblList<LeafPatchClass>::RemoveAll();

// GLOBAL: SWBF2 0x007ebad8
PblList<LeafPatch> LeafPatchClass::sObjects;
