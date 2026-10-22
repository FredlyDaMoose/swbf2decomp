#include <math.h>
#include <d3dx9math.h>

#include "LightningBoltEffect.h"
#include "Battlefront2Lod.h"
#include "Aimer.h"
#include "Controllable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"

// GLOBAL: SWBF2 0x01e573e0
LightningBoltEffect* LightningBoltEffect::sInstance;

// FUNCTION: SWBF2 0x00570e50
void LightningBoltEffect::Create()
{
    if (!sInstance)
    {
        sInstance = new LightningBoltEffect();
        sInstance->Activate(0);
    }
}

// GLOBAL: SWBF2 0x01ec63c0
// SYNTHETIC: SWBF2 0x00404100
// `dynamic initializer for 'LightningBoltEffectObject::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768c50
// `dynamic atexit destructor for 'LightningBoltEffectObject::sMemoryPool''
MemoryPool LightningBoltEffectObject::sMemoryPool("LightningBoltEffectObject", 0x120);

namespace
{

inline Controllable* GetControllable(GameObject* pObject)
{
    typedef Controllable*(__thiscall * GetControllableFn)(GameObject*);
    return pObject->GetControllable();
}

inline unsigned char ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    return (unsigned char)(int)f;
}

inline void ReadColor(const PblConfig::Data& data, RedColor& color)
{
    unsigned char a = data.GetNumArgs() > 3 ? ClampColorComponent(data.GetFloatArg(3)) : 0xff;
    unsigned char b = ClampColorComponent(data.GetFloatArg(2));
    unsigned char g = ClampColorComponent(data.GetFloatArg(1));
    unsigned char r = ClampColorComponent(data.GetFloatArg(0));
    color.a = a;
    color.r = r;
    color.g = g;
    color.b = b;
}
}

// SYNTHETIC: SWBF2 0x00570e00
// LightningBoltEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00570e40
bool LightningBoltEffectObject::IsEffectActive() const
{
    return true;
}

// FUNCTION: SWBF2 0x00570f00
void LightningBoltEffect::Destroy()
{
    if (sInstance)
    {
        delete sInstance;
        sInstance = 0;
    }
}

// FUNCTION: SWBF2 0x00570f20
LightningBoltEffect::~LightningBoltEffect()
{
}

// FUNCTION: SWBF2 0x00570f70
FLEffectClass* LightningBoltEffect::BuildClass(PblConfig& config) const
{
    LightningBoltEffectClass* pClass = new LightningBoltEffectClass(config);
    const_cast<PblList<LightningBoltEffectClass>&>(mClasses).AddLast(&pClass->mNode, pClass);
    return pClass;
}

// FUNCTION: SWBF2 0x00570ff0
void LightningBoltEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x00571000
void LightningBoltEffect::CreateClient()
{
    if (sInstance)
    {
        for (PblList<LightningBoltEffectClass>::Itor it(sInstance->mClasses);
             (LightningBoltEffectClass*)it; ++it)
        {
            LightningBoltEffectClass* pClass = it;
            pClass->DeactivateThread();
            NetGame::EnterClient();
            pClass->ActivateThread(0, PER_FRAME);
            NetGame::LeaveClient();
        }
    }
}

// FUNCTION: SWBF2 0x00571050
LightningBoltEffectClass::LightningBoltEffectClass(PblConfig& config)
{
    mColor.b = mColor.g = mColor.r = mColor.a = 0xff;
    mChildColor.b = mChildColor.g = mChildColor.r = mChildColor.a = 0xff;
    mTexture = 0xE812F405u;
    mWidth = 2.0f;
    mFadeTime = 1.0f;
    mBreakDistance = 2.0f;
    mTextureSize = 5.0f;
    mSpreadFactor = 1.0f;
    mMaxBranches = 5;
    mBranchFactor = 0.3f;
    mBranchSpreadFactor = 0.8f;
    mBranchLength = 4.0f;
    mInterpolationSpeed = 10.0f;
    mNumChildren = 3;
    mChildBreakDistance = 1.5f;
    mChildTextureSize = 3.0f;
    mChildWidth = 0.3f;
    mChildSpreadFactor = 1.0f;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3D7E6258u:
            ReadColor(data, mColor);
            break;
        case 0x33271FF0u:
            ReadColor(data, mChildColor);
            break;
        case 0x3C6468F4u:
            mTexture = PblHash(data.GetStringArg(0));
            break;
        case 0x95876E1Fu:
            mWidth = data.GetFloatArg(0);
            break;
        case 0xA40F930Au:
            mFadeTime = data.GetFloatArg(0);
            break;
        case 0xDBE276E9u:
            mSpreadFactor = data.GetFloatArg(0);
            break;
        case 0x9B2BB6E8u:
            mInterpolationSpeed = data.GetFloatArg(0);
            break;
        case 0xCEC9B326u:
            mBranchFactor = data.GetFloatArg(0);
            break;
        case 0x43523A69u:
            mBranchSpreadFactor = data.GetFloatArg(0);
            break;
        case 0xF609F595u:
            mBranchLength = data.GetFloatArg(0);
            break;
        case 0xFD127D8Du:
            mMaxBranches = (int)data.GetFloatArg(0);
            break;
        case 0x8F813993u:
            mBreakDistance = data.GetFloatArg(0);
            break;
        case 0x309878ADu:
            mTextureSize = data.GetFloatArg(0);
            break;
        case 0xC37B68ABu:
            mChildBreakDistance = data.GetFloatArg(0);
            break;
        case 0x17E40BE5u:
            mChildTextureSize = data.GetFloatArg(0);
            break;
        case 0x30BB2F27u:
            mChildWidth = data.GetFloatArg(0);
            break;
        case 0x8366BEF1u:
            mChildSpreadFactor = data.GetFloatArg(0);
            break;
        case 0xD80BDB3Cu:
            mNumChildren = (int)data.GetFloatArg(0);
            break;
        }
    }

    if (mMaxBranches > 10)
    {
        mMaxBranches = 10;
    }

    Thread::ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x00571510
// LightningBoltEffectClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00571540
LightningBoltEffectClass::~LightningBoltEffectClass()
{
    mObjects.RemoveAll();
    mNode.GetList()->Remove(&mNode);
}

// SYNTHETIC: SWBF2 0x00572827
// [thunk]:LightningBoltEffectClass::`scalar deleting destructor' adjustor{12}

// FUNCTION: SWBF2 0x005715d0
FLEffectObject* LightningBoltEffectClass::BuildEffect()
{
    LightningBoltEffectObject* pObject = LightningBoltEffectObject::Create(this);
    if (pObject)
    {
        mObjects.AddLast(&pObject->mNode, pObject);
    }
    return pObject;
}

// FUNCTION: SWBF2 0x00571610
bool LightningBoltEffectClass::Update(float fDeltaTime)
{
    PblList<LightningBoltEffectObject>::Itor it(mObjects);
    while (LightningBoltEffectObject* pObject = it)
    {
        ++it;
        if (!pObject->UpdateBolt(fDeltaTime))
        {
            delete pObject;
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00571650
LightningBoltEffectObject* LightningBoltEffectObject::Create(const LightningBoltEffectClass* pClass)
{
    if (sMemoryPool.mUsed >= sMemoryPool.GetCount())
    {
        return 0;
    }
    if (!NetGame::IsEnabled() || NetGame::IsClient() ||
        (NetGame::IsOneWorld() && !NetGame::IsDedicated()))
    {
        return new LightningBoltEffectObject(pClass);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00571700
LightningBoltEffectObject::LightningBoltEffectObject(const LightningBoltEffectClass* pClass)
    : RedSceneObject(&largeEffectLodData)
{
    mGameObject.mObject = 0;
    mGameObject.mSavedHandleId = 0;
    mClass = const_cast<LightningBoltEffectClass*>(pClass);
    mLifeTime = pClass->mFadeTime;
    mRandSeed = PblRandom::GetGlobalInt();
    mRandSeed2 = PblRandom::GetGlobalInt();
    mInterpolation = 0.0f;
    mStrength = 1.0f;
}

// SYNTHETIC: SWBF2 0x00572814
// [thunk]:LightningBoltEffectObject::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x0057281c
// [thunk]:LightningBoltEffectObject::`scalar deleting destructor' adjustor{128}
// SYNTHETIC: SWBF2 0x00571880
// LightningBoltEffectObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005718b0
LightningBoltEffectObject::~LightningBoltEffectObject()
{
    mClass->mObjects.Remove(&mNode);
}

// FUNCTION: SWBF2 0x00571950
bool LightningBoltEffectObject::UpdateBolt(float fDeltaTime)
{
    mLifeTime -= fDeltaTime;
    if (mLifeTime <= 0.0f)
    {
        return false;
    }

    mInterpolation += mClass->mInterpolationSpeed * fDeltaTime;
    if (mInterpolation > 1.0f)
    {
        mRandSeed = mRandSeed2;
        mInterpolation -= 1.0f;
        mRandSeed2 = PblRandom::GetGlobalInt();
    }

    if (mGameObject && GetControllable(mGameObject) && GetControllable(mGameObject)->GetAimer(0))
    {
        PblVector3 vFirePos = GetControllable(mGameObject)->GetAimer(0)->mFirePos;
        AttachEffectToVector(&vFirePos, false);
    }

    if (!_bActive)
    {
        const PblVector3& vStart = *(const PblVector3*)&m_mWorldMatrix.m[3][0];
        PblVector3 vDelta(vStart.x - mTargetPos.x, vStart.y - mTargetPos.y, vStart.z - mTargetPos.z);
        PblVector3 vCenter(mTargetPos.x + vDelta.x * 0.5f, mTargetPos.y + vDelta.y * 0.5f,
                           mTargetPos.z + vDelta.z * 0.5f);
        SetRenderPos(vCenter);
        SetRenderRadius(sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z) * 0.5f);
        Activate();
    }
    return true;
}

// FUNCTION: SWBF2 0x00571bb0
bool LightningBoltEffectObject::SetOrdnanceOwner(GameObject* pObject)
{
    if (!pObject || !GetControllable(pObject))
    {
        return false;
    }

    mGameObject = pObject;
    PblVector3 vFirePos = GetControllable(mGameObject)->GetAimer(0)->mFirePos;
    AttachEffectToVector(&vFirePos, false);
    return true;
}

// FUNCTION: SWBF2 0x00571c60
void LightningBoltEffectObject::Render(unsigned char, float, unsigned int)
{
    PblVector3 vStart = *(const PblVector3*)&m_mWorldMatrix.m[3][0];
    PblVector3 vMin((mTargetPos.x > vStart.x ? vStart.x : mTargetPos.x) - 128.0f,
                    (mTargetPos.y > vStart.y ? vStart.y : mTargetPos.y) - 128.0f,
                    (mTargetPos.z > vStart.z ? vStart.z : mTargetPos.z) - 128.0f);
    PblVector3 vMax((vStart.x > mTargetPos.x ? vStart.x : mTargetPos.x) + 128.0f,
                    (vStart.y > mTargetPos.y ? vStart.y : mTargetPos.y) + 128.0f,
                    (vStart.z > mTargetPos.z ? vStart.z : mTargetPos.z) + 128.0f);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::SubmitTexture(mClass->mTexture, BLEND_ADDITIVE, 0);

    float fFade = mLifeTime * 2.0f / mClass->mFadeTime;
    if (fFade >= 1.0f)
    {
        fFade = 1.0f;
    }
    float fAlpha = (mStrength * 0.5f + 0.5f) * fFade;
    float fWidthScale = mStrength * 0.7f + 0.3f;

    RedColor color = mClass->mColor;
    color.a = (unsigned char)(int)(mClass->mColor.a * fAlpha);
    Render(vStart, mTargetPos, mClass->mWidth * fWidthScale, mClass->mSpreadFactor, mRandSeed,
           mRandSeed2, mInterpolation, color, mClass->mBreakDistance, mClass->mTextureSize,
           mClass->mBranchFactor, mClass->mMaxBranches, mClass->mBranchSpreadFactor,
           mClass->mBranchLength);

    if (mClass->mNumChildren > 0)
    {
        for (int i = 0;; ++i)
        {
            int iChildren = (int)(mClass->mNumChildren * mStrength);
            if (iChildren < 1)
            {
                iChildren = 1;
            }
            if (i >= iChildren)
            {
                break;
            }
            color = mClass->mChildColor;
            color.a = (unsigned char)(int)(mClass->mChildColor.a * fAlpha);
            Render(vStart, mTargetPos, mClass->mChildWidth * fWidthScale,
                   mClass->mChildSpreadFactor, PblRandom::GetGlobalInt(),
                   PblRandom::GetGlobalInt(), 0.0f, color, mClass->mChildBreakDistance,
                   mClass->mChildTextureSize, mClass->mBranchFactor, mClass->mMaxBranches,
                   mClass->mBranchSpreadFactor, mClass->mBranchLength);
        }
    }

    RedParticleRenderer::RenderAll();
}

// FUNCTION: SWBF2 0x00571fc0
void LightningBoltEffectObject::Render(const PblVector3& vStart, const PblVector3& vEnd, float fWidth,
                                       float fSpread, unsigned int uiSeed, unsigned int uiSeed2,
                                       float fInterpolation, const RedColor& color,
                                       float fBreakDistance, float fTextureSize, float fBranchFactor,
                                       int iMaxBranches, float fBranchSpread,
                                       float fBranchLength) const
{
    PblRandom rand1;
    rand1.SetSeed(uiSeed ^ 0xC71C2830);
    PblRandom rand2;
    rand2.SetSeed(uiSeed2 ^ 0xC71C2830);

    PblVector3 avBranchStart[10];
    PblVector3 avBranchDir[10];
    float afBranchWidth[10];
    int iBranches = 0;

    PblVector3 vDir(vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z);
    float fLength = sqrtf(vDir.x * vDir.x + vDir.y * vDir.y + vDir.z * vDir.z);
    float fTexCoord = 0.0f;
    if (fLength <= 0.0f)
    {
        return;
    }

    float fInvLength = 1.0f / fLength;
    vDir.x *= fInvLength;
    vDir.y *= fInvLength;
    vDir.z *= fInvLength;

    int iSegments = (int)(fLength / fBreakDistance);
    if (iSegments < 3)
    {
        iSegments = 3;
    }
    if (iSegments > 100)
    {
        return;
    }

    float fSegLength = fLength / iSegments;
    float fWidthStep = fWidth / (iSegments + 1);
    float fTexStep = fSegLength / fTextureSize;

    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;

    RedColor capColor;
    capColor.b = color.b;
    capColor.g = color.g;
    capColor.r = color.r;
    capColor.a = 0;

    PblVector3 vPos = vStart;
    RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vPos, capColor, fWidth, 0.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);

    PblVector3 vStep(vDir.x * fSegLength, vDir.y * fSegLength, vDir.z * fSegLength);
    for (int i = 1; i < iSegments; ++i)
    {
        fTexCoord += fTexStep;
        vPos.x += vStep.x;
        vPos.y += vStep.y;
        vPos.z += vStep.z;

        PblVector3 vFrom((rand1.GetFloat() - 0.5f) * fSpread, (rand1.GetFloat() - 0.5f) * fSpread,
                         (rand1.GetFloat() - 0.5f) * fSpread);
        float fDot = vFrom.x * vDir.x + vFrom.y * vDir.y + vFrom.z * vDir.z;
        vFrom.x -= vDir.x * fDot;
        vFrom.y -= vDir.y * fDot;
        vFrom.z -= vDir.z * fDot;

        PblVector3 vTo((rand2.GetFloat() - 0.5f) * fSpread, (rand2.GetFloat() - 0.5f) * fSpread,
                       (rand2.GetFloat() - 0.5f) * fSpread);
        fDot = vTo.x * vDir.x + vTo.y * vDir.y + vTo.z * vDir.z;
        vTo.x -= vDir.x * fDot;
        vTo.y -= vDir.y * fDot;
        vTo.z -= vDir.z * fDot;
        rand2.GetInt();

        PblVector3 vOffset((vTo.x - vFrom.x) * fInterpolation + vFrom.x,
                           (vTo.y - vFrom.y) * fInterpolation + vFrom.y,
                           (vTo.z - vFrom.z) * fInterpolation + vFrom.z);
        PblVector3 vJoint(vPos.x + vOffset.x, vPos.y + vOffset.y, vPos.z + vOffset.z);
        RedParticleRenderer::SubmitParticle(PIT_STREAK, vJoint, color, fWidth, fTexCoord, texCoords,
                                            gVectorUnitX, gVectorUnitY);

        if (rand1.GetFloat() < fBranchFactor && iBranches < iMaxBranches)
        {
            avBranchStart[iBranches] = vJoint;
            PblVector3 vBranch(vOffset.x + vDir.x * fBranchSpread, vOffset.y + vDir.y * fBranchSpread,
                               vOffset.z + vDir.z * fBranchSpread);
            D3DXVec3Normalize((D3DXVECTOR3*)&avBranchDir[iBranches], (const D3DXVECTOR3*)&vBranch);
            afBranchWidth[iBranches] = fWidth;
            ++iBranches;
        }
        fWidth -= fWidthStep;
    }

    RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vEnd, capColor, fWidth, fTexCoord + fTexStep,
                                        texCoords, gVectorUnitX, gVectorUnitY);

    for (int i = iBranches - 1; i >= 0; --i)
    {
        PblVector3 vBranchEnd(avBranchStart[i].x + avBranchDir[i].x * fBranchLength,
                              avBranchStart[i].y + avBranchDir[i].y * fBranchLength,
                              avBranchStart[i].z + avBranchDir[i].z * fBranchLength);
        Render(avBranchStart[i], vBranchEnd, afBranchWidth[i], fSpread, uiSeed + i + 1,
               uiSeed2 + i + 1, fInterpolation, color, fBreakDistance, fTextureSize,
               fBranchFactor * 0.5f, 0, fBranchSpread, fBranchLength * 0.5f);
    }
}

// FUNCTION: SWBF2 0x005727d0
void LightningBoltEffectObject::SetStrength(float fStrength)
{
    mStrength = fStrength;
}

// FUNCTION: SWBF2 0x005727e0
template void PblList<LightningBoltEffectObject>::RemoveAll();
