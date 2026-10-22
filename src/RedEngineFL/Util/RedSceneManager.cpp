#include <float.h>
#include <math.h>

#include "RedSceneManager.h"
#include "RedSceneObject.h"
#include "RedLodManager.h"

#include "RedEngineFL/Portals/Portal.h"
#include "ext/Allocator.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "Battlefront2/Source/Battlefront2Lod.h"
#include "PebbleFL/Common/PblMath.h"

#include "ext/Globals.h"

namespace FLRenderer
{
bool GetFadeAdjustWithZoom();
float GetNearSceneFadeStart();
float GetFarSceneRange();
float GetNearSceneFadeEnd();
}

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

// GLOBAL: SWBF2 0x009a08a8
RedSceneManager* RedSceneManager::sInstance;

void j_nullsub_177();

// FUNCTION: SWBF2 0x006e4410
void RedSceneManager::_Init()
{
    j_nullsub_177();
    util::Globals::GetInstance();
    pvs::IManager::GetInstance();
}

// FUNCTION: SWBF2 0x00725190
void sub_725190()
{
}

// FUNCTION: SWBF2 0x007251a0
void j_nullsub_177()
{
    sub_725190();
}

// FUNCTION: SWBF2 0x006aee30
template bool util::BaseArray<int*,
    util::TypedArrayAllocator<int*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x006aed30
template bool util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x006aef30
template bool util::BaseArray<char const*,
    util::TypedArrayAllocator<char const*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::Reserve_(int, bool);
// FUNCTION: SWBF2 0x006af1e0
template bool util::Vector<int*, util::BaseArray<int*,
    util::TypedArrayAllocator<int*, util::SingletonAllocator<util::DefaultStaticAllocator> > > >::SetSize_(int, bool);
// FUNCTION: SWBF2 0x006af130
template bool util::Vector<unsigned int, util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::SingletonAllocator<util::DefaultStaticAllocator> > > >::SetSize_(int, bool);
// FUNCTION: SWBF2 0x006af290
template bool util::Vector<char const*, util::BaseArray<char const*,
    util::TypedArrayAllocator<char const*, util::SingletonAllocator<util::DefaultStaticAllocator> > > >::SetSize_(int, bool);
// FUNCTION: SWBF2 0x006adb00
template unsigned int util::HashTable_<int*, char const*, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::FindIndex_(unsigned int) const;

// FUNCTION: SWBF2 0x006ad580
template util::BaseArray<unsigned int,
    util::TypedArrayAllocator<unsigned int, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();
// FUNCTION: SWBF2 0x006ad5d0
template util::BaseArray<int*,
    util::TypedArrayAllocator<int*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();
// FUNCTION: SWBF2 0x006ad620
template util::BaseArray<char const*,
    util::TypedArrayAllocator<char const*, util::SingletonAllocator<util::DefaultStaticAllocator> > >::~BaseArray();

// FUNCTION: SWBF2 0x006ae950
template int*& util::HashTable_<int*, char const*, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::ElemVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::KeyVecT, util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::HashVecT>::Get(char const* const&);

// FUNCTION: SWBF2 0x006ae9d0
template int& util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >::GetInt(char const*);

// FUNCTION: SWBF2 0x006aead0
template void util::HashTable<int*, char const*, util::SingletonAllocator<util::DefaultStaticAllocator> >::Reallocate(int);

// GLOBAL: SWBF2 0x009a0878
int gRenderCount;
// GLOBAL: SWBF2 0x009a087c
int gClippedCount;
// GLOBAL: SWBF2 0x009a0880
int gCullingCount;
// GLOBAL: SWBF2 0x009a0884
int gObjCullingCount;
// GLOBAL: SWBF2 0x009a0888
static PblHeap<RedSceneObject*, float>* sFarSceneObjects;

// FUNCTION: SWBF2 0x006e43b0
void RedSceneManager::_Cleanup()
{
    sFarSceneObjects = 0;
    mScene.Cleanup();
    mDoScene = true;
    pvs::IManager::ClearInstance();
    util::HashMultiAllocator::ClearDefault();
}

// GLOBAL: SWBF2 0x009a088c
float g_fCameraNear;
// GLOBAL: SWBF2 0x009a0890
float g_fCameraFar;
// GLOBAL: SWBF2 0x009a0894
float g_fCameraTanHalfFOVX;
// GLOBAL: SWBF2 0x009a0898
float g_fCameraTanHalfFOVY;
// GLOBAL: SWBF2 0x009a089c
float g_fCameraCosHalfFOVX;
// GLOBAL: SWBF2 0x009a08a0
float g_fCameraCosHalfFOVY;

// GLOBAL: SWBF2 0x009a08b0
// SYNTHETIC: SWBF2 0x004089d0
// `dynamic initializer for 'sRedSceneCamera''
static RedSceneCamera sRedSceneCamera;
// GLOBAL: SWBF2 0x007df954
static int sMaxNumFarObjects = 256;

// GLOBAL: SWBF2 0x009a092c
static bool sDrawFullyVisibleBoxes;

// GLOBAL: SWBF2 0x007df9d0
static bool sQueueTransparentLodSegments = true;

void SetupMinMaxIndex(const PblVector3& v, int& iMin, int& iMax);
void SetupPlanes(const PblMatrix& m, float fNear, float fWidth, float fHeight,
                 PblVector3* pPlanes, int* pMinIndex, int* pMaxIndex);

bool RenderObject(RedSceneObject* pObject, unsigned int uiFlags, int iClip);

PblVector3 GetCornerFromIndex(const PblVector3& v, int i);

// FUNCTION: SWBF2 0x006e22f0
RedScene::RedScene()
{
    mObject = 0;
}

// FUNCTION: SWBF2 0x006e2330
RedSceneManager::RedSceneManager()
{
    sInstance = this;
    mDoScene = true;
    mTransparentObjects = new PblHeap<RedSceneObject*, float>(1024, FLT_MAX, 0);
    mTransparentPhase = false;
    mState = 0;
}

// FUNCTION: SWBF2 0x006e2400
RedScene::~RedScene()
{
}

// FUNCTION: SWBF2 0x006e2410
void RedScene::Add(RedSceneObject* pObject)
{
    mStaticWorldBox.Add(pObject);
}

// FUNCTION: SWBF2 0x006e2420
void RedSceneAABox::Add(RedSceneObject* pObject)
{
    if (mChild && mChild->IsInside(pObject))
    {
        mChild->Add(pObject);
        return;
    }
    if (mSibling && mSibling->IsInside(pObject))
    {
        mSibling->Add(pObject);
        return;
    }
    if (pObject->mSceneAABox != this)
    {
        if (pObject->mSceneAABox)
        {
            pObject->mSceneAABox->Remove(pObject);
        }
        pObject->mNextSceneObject = mObjects;
        if (mObjects)
        {
            mObjects->mPrevSceneObject = pObject;
        }
        pObject->mPrevSceneObject = 0;
        mObjects = pObject;
        pObject->mSceneAABox = this;
    }
}

// FUNCTION: SWBF2 0x006e24a0
void RedSceneAABox::AddSibling(RedSceneAABox* pBox)
{
    if (mSibling)
    {
        mSibling->AddSibling(pBox);
    }
    else
    {
        mSibling = pBox;
    }
    pBox->mParent = mParent;
}

// FUNCTION: SWBF2 0x006e24e0
void RedSceneManager::AddTransparentObject(RedSceneObject* pObject)
{
    const PblMatrix& m = RedRenderer::GetCamera()->GetMatrix();
    float fDist = m.m[2][1] * (m.m[3][1] - pObject->_Sphere.y)
                + m.m[2][0] * (m.m[3][0] - pObject->_Sphere.x)
                + m.m[2][2] * (m.m[3][2] - pObject->_Sphere.z)
                - pObject->_Sphere.w;
    sInstance->_AddTransparentObject(pObject, fDist);
}

// FUNCTION: SWBF2 0x006e2550
int RedSceneCamera::ClassifyAABB(unsigned int uiFlags, const PblVector3& vPos,
                                 const PblVector3& vSize, int iMask)
{
    // GLOBAL: SWBF2 0x007df9b0
    static int sPlaneBit[4] = { 4, 8, 16, 32 };

    float dx = vPos.x - mPosition.x;
    float dy = vPos.y - mPosition.y;
    float dz = vPos.z - mPosition.z;
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);

    if (!(iMask & 1) || !(iMask & 2))
    {
        PblVector3 c = GetCornerFromIndex(vSize, mFrontMaxIndex);
        float fMax = mFront.y * (c.y + dy) + mFront.x * (c.x + dx) + mFront.z * (c.z + dz);
        if (mNearPlane > fMax)
        {
            return -1;
        }
        c = GetCornerFromIndex(vSize, mFrontMinIndex);
        float fMin = mFront.y * (c.y + dy) + (c.x + dx) * mFront.x + mFront.z * (c.z + dz);
        if (fMin > mFarPlane)
        {
            return -1;
        }
        if (fMin > mNearPlane)
        {
            if (uiFlags & 0x40000)
            {
                if (fNear > fMin)
                {
                    iMask |= 1;
                }
                else
                {
                    fFar = 0;
                }
            }
            else
            {
                iMask |= 1;
            }
        }
        if (mFarPlane > fMax)
        {
            iMask |= 2;
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (!(iMask & sPlaneBit[i]))
        {
            PblVector3 c = GetCornerFromIndex(vSize, mMaxIndex[i]);
            if (mPlane[i].y * (c.y + dy) + mPlane[i].x * (c.x + dx) + mPlane[i].z * (c.z + dz) < 0.0f)
            {
                return -1;
            }
        }
    }
    for (int i = 0; i < 4; ++i)
    {
        if (!(iMask & sPlaneBit[i]))
        {
            PblVector3 c = GetCornerFromIndex(vSize, mMinIndex[i]);
            if (mPlane[i].y * (c.y + dy) + mPlane[i].x * (c.x + dx) + mPlane[i].z * (c.z + dz) > 0.0f)
            {
                iMask |= sPlaneBit[i];
            }
        }
    }
    return iMask;
}

// FUNCTION: SWBF2 0x006e27f0
int RedSceneCamera::ClassifySphere(const PblVector3& vPos, float fRadius, int iMask)
{
    // GLOBAL: SWBF2 0x007df9c0
    static int sPlaneBit[4] = { 4, 8, 16, 32 };

    float dx = vPos.x - mPosition.x;
    float dy = vPos.y - mPosition.y;
    float dz = vPos.z - mPosition.z;

    if (!(iMask & 1) || !(iMask & 2))
    {
        float fDist = mFront.y * dy + mFront.x * dx + mFront.z * dz;
        if (mNearPlane - fRadius > fDist)
        {
            return -1;
        }
        if (fDist > mFarPlane + fRadius)
        {
            return -1;
        }
        if (fDist > mNearPlane + fRadius)
        {
            iMask |= 1;
        }
        if (mFarPlane - fRadius > fDist)
        {
            iMask |= 2;
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (!(iMask & sPlaneBit[i]))
        {
            float fDist = mPlane[i].x * dx + mPlane[i].y * dy + dz * mPlane[i].z;
            if (-fRadius > fDist)
            {
                return -1;
            }
            if (fDist > fRadius)
            {
                iMask |= sPlaneBit[i];
            }
        }
    }
    return iMask;
}

// FUNCTION: SWBF2 0x006e2900
PblVector3 GetCornerFromIndex(const PblVector3& v, int i)
{
    float z = v.z;
    if (i & 2)
    {
        z = -z;
    }
    float y = v.y;
    if (i >= 4)
    {
        y = -y;
    }
    float x = v.x;
    if (!(i & 1))
    {
        x = -x;
    }
    return PblVector3(x, y, z);
}

// FUNCTION: SWBF2 0x006e2950
bool RedSceneAABox::IsInside(RedSceneObject* pObject)
{
    float fRadius = pObject->_Sphere.w;
    return mSize.x > fabsf(pObject->_Sphere.x - mMidPos.x) + fRadius
        && mSize.y > fabsf(pObject->_Sphere.y - mMidPos.y) + fRadius
        && mSize.z > fabsf(pObject->_Sphere.z - mMidPos.z) + fRadius;
}

// FUNCTION: SWBF2 0x006e29c0
void RedSceneAABox::Move(RedSceneObject* pObject)
{
    RedSceneAABox* pBox = this;
    while (pBox->mParent && !pBox->IsInside(pObject))
    {
        pBox = pBox->mParent;
    }
    pBox->Add(pObject);
}

// FUNCTION: SWBF2 0x006e2a00
template void PblHeap<RedSceneAABox*, float>::Pop();
// FUNCTION: SWBF2 0x006e2a90
template void PblHeap<RedSceneObject*, float>::Pop();

// FUNCTION: SWBF2 0x006e2b20
template void PblHeap<RedSceneObject*, float>::Push(RedSceneObject*&, float);

// FUNCTION: SWBF2 0x006e2b80
void RedSceneManager::Read_(PblFileChunk& chunk)
{
}

// FUNCTION: SWBF2 0x006e2b90
void RedSceneAABox::Remove(RedSceneObject* pObject)
{
    if (mObjects == pObject)
    {
        mObjects = pObject->mNextSceneObject;
    }
    else if (pObject->mPrevSceneObject)
    {
        pObject->mPrevSceneObject->mNextSceneObject = pObject->mNextSceneObject;
    }
    if (pObject->mNextSceneObject)
    {
        pObject->mNextSceneObject->mPrevSceneObject = pObject->mPrevSceneObject;
    }
    pObject->mSceneAABox = 0;
}

// FUNCTION: SWBF2 0x006e2bd0
void RedScene::Render(unsigned int uiFlags) const
{
    const_cast<RedSceneAABox&>(mStaticWorldBox).Render(uiFlags, 0);
}

// FUNCTION: SWBF2 0x006e2bf0
void RedSceneAABox::Render(unsigned int uiFlags, int iClip)
{
    RedSceneObject* pObject = mObjects;
    bool bPvs = pvs::IManager::GetInstance().IsActive();
    while (pObject)
    {
        if (!bPvs || !pObject->PortalActive() || pObject->IsActive(pvs::GetCurrentFrame_()))
        {
            if ((uiFlags & 0x260000) || !pObject->IsOccluded())
            {
                RenderObject(pObject, uiFlags, iClip);
            }
            if (!(uiFlags & 0x260000))
            {
                pObject->UpdateOcclusionQuery(iClip);
            }
        }
        pObject = pObject->mNextSceneObject;
    }

    if (mChild)
    {
        PblHeap<RedSceneAABox*, float>::RECORD pool[5];
        PblHeap<RedSceneAABox*, float> heap(4, FLT_MAX, pool);
        heap.Push(mChild, mChild->SetupRenderOrder(heap));
        while (!heap.IsEmpty())
        {
            RedSceneAABox* pBox = heap.Get(0);
            heap.Pop();
            int iBoxClip = iClip;
            if (iClip != 63)
            {
                iBoxClip = sRedSceneCamera.ClassifyAABB(uiFlags, pBox->mMidPos, pBox->mSize, iClip);
                ++gCullingCount;
            }
            if (sDrawFullyVisibleBoxes && iClip != 63 && !(uiFlags & 0x200000) && iBoxClip == 63)
            {
                pBox->RenderBox(0, false);
            }
            if (iBoxClip != -1)
            {
                pBox->Render(uiFlags, iBoxClip);
            }
        }
    }
}

// FUNCTION: SWBF2 0x006e2de0
void RedSceneAABox::RenderBox(int iDepth, bool bSiblings)
{
    if (bSiblings && mSibling)
    {
        mSibling->RenderBox(iDepth, true);
    }

    PblBox box;
    box._vMin.x = mMidPos.x - mSize.x;
    box._vMin.y = mMidPos.y - mSize.y;
    box._vMin.z = mMidPos.z - mSize.z;
    box._vMax.x = mMidPos.x + mSize.x;
    box._vMax.y = mMidPos.y + mSize.y;
    box._vMax.z = mMidPos.z + mSize.z;
    RedColor color;
    color.b = RedColor::WHITE.b;
    color.g = RedColor::WHITE.g;
    color.r = RedColor::WHITE.r;
    color.a = 0x80;
    RedDebug::DrawAABB(box, color);

    if (iDepth > 0 && mChild)
    {
        RenderBox(iDepth - 1, true);
    }
}

// FUNCTION: SWBF2 0x006e2ed0
void RenderFarObjects()
{
    if (!sFarSceneObjects)
    {
        return;
    }

    float fMinScreenSize = RedLodManager::GetMinScreenSize();
    RedSceneObject* objects[256];
    int iCount = 0;
    int iMax = 256;
    if (sMaxNumFarObjects < iMax)
    {
        iMax = sMaxNumFarObjects;
    }
    int iClass2Budget = 10;
    sMaxNumFarObjects = iMax;
    float fMinRatio = fMinScreenSize * fMinScreenSize / 798720.0f;

    for (; iCount < iMax; ++iCount)
    {
        if (sFarSceneObjects->IsEmpty())
        {
            break;
        }
        objects[iCount] = sFarSceneObjects->Get(0);
        sFarSceneObjects->Pop();
        if (objects[iCount]->_pLodData->_LodClass == 2)
        {
            if (iClass2Budget > 0)
            {
                --iClass2Budget;
            }
            else
            {
                --iCount;
            }
        }
    }
    sFarSceneObjects->Flush();
    gRenderCount += iCount;

    float fZoom = 1.0f;
    if (FLRenderer::GetFadeAdjustWithZoom())
    {
        fZoom = RedRenderer::GetCamera()->GetZoom();
    }
    float fFadeStart = FLRenderer::GetNearSceneFadeStart() * fZoom;
    float fFarRange = FLRenderer::GetFarSceneRange();

    for (int i = 0; i < iCount; ++i)
    {
        RedSceneObject* p = objects[i];
        float fRadius = p->_Sphere.w;
        float fDist = (p->_Sphere.x - sRedSceneCamera.mPosition.x) * sRedSceneCamera.mFront.x
                    + (p->_Sphere.y - sRedSceneCamera.mPosition.y) * sRedSceneCamera.mFront.y
                    + (p->_Sphere.z - sRedSceneCamera.mPosition.z) * sRedSceneCamera.mFront.z
                    + fRadius;
        float fLimit = fRadius + fFarRange;
        if (fDist < fLimit && !(fMinRatio > fRadius * fRadius / (fDist * fDist)))
        {
            if (fDist >= fLimit - 20.0f)
            {
                continue;
            }
            if (p->_pLodData->mLowRezFlags & 2)
            {
                if (!(fFadeStart > fDist))
                {
                    if (fFadeStart + 20.0f > fDist)
                    {
                        continue;
                    }
                    p->Render(3, 1.0f, p->_uiRenderFlags | 0x20000);
                }
            }
            else if (fFadeStart < fDist)
            {
                p->Render(3, 1.0f, p->_uiRenderFlags | 0x20000);
            }
        }
        objects[i] = 0;
    }

    for (int i = iCount - 1; i >= 0; --i)
    {
        RedSceneObject* p = objects[i];
        if (!p)
        {
            continue;
        }
        float fRadius = p->_Sphere.w;
        float fDist = (p->_Sphere.x - sRedSceneCamera.mPosition.x) * sRedSceneCamera.mFront.x
                    + (p->_Sphere.y - sRedSceneCamera.mPosition.y) * sRedSceneCamera.mFront.y
                    + (p->_Sphere.z - sRedSceneCamera.mPosition.z) * sRedSceneCamera.mFront.z
                    + fRadius;
        float fAlpha = (fDist - fFadeStart) / 20.0f;
        if (!(1.0f > fAlpha))
        {
            fAlpha = 1.0f;
        }
        float fLimit = fRadius + fFarRange;
        if (fDist >= fLimit - 20.0f)
        {
            fAlpha = (fLimit - fDist) * fAlpha / 20.0f;
        }
        p->Render(3, fAlpha, p->_uiRenderFlags | 0x20000);
    }
}

// FUNCTION: SWBF2 0x006e31c0
bool RenderObject(RedSceneObject* pObject, unsigned int uiFlags, int iClip)
{
    if (iClip != 63)
    {
        int iMask = sRedSceneCamera.ClassifySphere(*(const PblVector3*)&pObject->_Sphere,
                                                   pObject->_Sphere.w, 0);
        if (iMask == -1)
        {
            return false;
        }
        ++gObjCullingCount;
        if (iMask != 63)
        {
            ++gClippedCount;
            pObject->SetClipped(true);
        }
        else
        {
            pObject->SetClipped(false);
        }
    }
    else
    {
        pObject->SetClipped(false);
    }

    if ((uiFlags & 0x200000) && !pObject->IsReflected())
    {
        return false;
    }

    if (uiFlags & 0x40000)
    {
        if (sFarSceneObjects->IsFull() || !pObject->_pLodData->HasLowRes())
        {
            return false;
        }
        float fZoom = 1.0f;
        if (FLRenderer::GetFadeAdjustWithZoom())
        {
            fZoom = RedRenderer::GetCamera()->GetZoom();
        }
        float fFadeStart = FLRenderer::GetNearSceneFadeStart() * fZoom;
        float fRadius = pObject->_Sphere.w;
        float fDist = (pObject->_Sphere.y - sRedSceneCamera.mPosition.y) * sRedSceneCamera.mFront.y
                    + (pObject->_Sphere.x - sRedSceneCamera.mPosition.x) * sRedSceneCamera.mFront.x
                    + (pObject->_Sphere.z - sRedSceneCamera.mPosition.z) * sRedSceneCamera.mFront.z
                    + fRadius - fFadeStart;
        if (0.0f > fDist)
        {
            return false;
        }
        if (FLRenderer::GetFarSceneRange() > fFadeStart)
        {
            sFarSceneObjects->Push(pObject, -(fDist - fRadius * 4.0f));
        }
        return true;
    }

    const RedLodData* pLodData = pObject->_pLodData;
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fZoom = 1.0f;
    if (FLRenderer::GetFadeAdjustWithZoom())
    {
        fZoom = pCamera->GetZoom();
    }
    PblVector3 vView;
    D3DXVec3TransformCoord(&vView, &*(const PblVector3*)&pObject->_Sphere, &pCamera->GetMatrixInverse());

    if (pLodData->HasLowRes() && !pLodData->CrossFadeLowRes())
    {
        float fFadeStart = FLRenderer::GetNearSceneFadeStart();
        if (FLRenderer::GetFarSceneRange() > fFadeStart)
        {
            if (FLRenderer::GetFadeAdjustWithZoom())
            {
                fFadeStart = RedRenderer::GetCamera()->GetZoom() * fFadeStart;
            }
            if (-vView.z - pObject->_Sphere.w >= fFadeStart)
            {
                return false;
            }
        }
    }

    if (pLodData->GetLodClass() < 0 || (sQueueTransparentLodSegments && pLodData->HasTransparentSegment()))
    {
        RedSceneManager::AddTransparentObject(pObject, -vView.z - pObject->_Sphere.w);
        return true;
    }
    if (RedLodManager::IsFull(pLodData->GetLodClass()))
    {
        return true;
    }

    float fRadius = pObject->_Sphere.w * fZoom;
    float fLength2 = vView.y * vView.y + vView.x * vView.x + vView.z * vView.z;
    float fPriority = fRadius * fRadius;
    if (fLength2 > 1.0f)
    {
        fPriority /= fLength2;
    }

    if (pObject->IsClipped())
    {
        float fDepth = -vView.z;
        float fInvRadius = 1.0f / fRadius;
        float fNear = g_fCameraNear - fDepth;
        if (fNear > 0.0f)
        {
            float f = 1.0f - fNear * fInvRadius;
            fPriority *= f * f;
        }
        if (fDepth > 0.0f)
        {
            float fOutX = fabsf(vView.x) - g_fCameraTanHalfFOVX * fDepth;
            float fAbsY = fabsf(vView.y);
            if (fOutX > 0.0f)
            {
                float f = 1.0f - g_fCameraCosHalfFOVX * fOutX * fInvRadius;
                fPriority = f * f * fPriority;
            }
            float fOutY = fAbsY - g_fCameraTanHalfFOVY * fDepth;
            if (fOutY > 0.0f)
            {
                float f = 1.0f - g_fCameraCosHalfFOVY * fOutY * fInvRadius;
                fPriority = f * f * fPriority;
            }
        }
    }

    RedLodManager::Add(pObject, fPriority);
    ++gRenderCount;
    return true;
}

// FUNCTION: SWBF2 0x006e3610
void RedSceneCamera::Setup(const RedCamera* pCamera, float fNear, float fFar)
{
    const PblMatrix& m = pCamera->GetMatrix();
    mNearPlane = fNear;
    mFarPlane = fFar;
    mPosition = PblVector3(m.m[3][0], m.m[3][1], m.m[3][2]);
    mFront = PblVector3(-m.m[2][0], -m.m[2][1], -m.m[2][2]);
    SetupMinMaxIndex(mFront, mFrontMinIndex, mFrontMaxIndex);

    float fCamNear, fCamFar, fWidth, fHeight;
    pCamera->GetFrustum(fCamNear, fCamFar, fWidth, fHeight);
    SetupPlanes(m, fCamNear, fWidth, fHeight, mPlane, mMinIndex, mMaxIndex);
}

// FUNCTION: SWBF2 0x006e36e0
bool RedSceneAABox::SetupBox()
{
    RedSceneObject* pObject = mObjects;
    if (!pObject)
    {
        return false;
    }

    PblVector3 vMin(FLT_MAX, FLT_MAX, FLT_MAX);
    PblVector3 vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    float fSumX = 0.0f;
    float fSumZ = 0.0f;
    int iCount = 0;
    do
    {
        PblVector3 vPos = *(const PblVector3*)&pObject->_Sphere;
        PblBox box;
        if (pObject->_uiRenderFlags & 2)
        {
            const PblBox& bbox = pObject->GetBBox();
            PblVector3 vHalf((bbox._vMax.x - bbox._vMin.x) * 0.5f,
                             (bbox._vMax.y - bbox._vMin.y) * 0.5f,
                             (bbox._vMax.z - bbox._vMin.z) * 0.5f);
            PblBox local;
            local._vMin = PblVector3(-vHalf.x, -vHalf.y, -vHalf.z);
            local._vMax = vHalf;
            PblMatrix m = pObject->GetBBoxMatrix();
            m.m[3][0] = vPos.x;
            m.m[3][1] = vPos.y;
            m.m[3][2] = vPos.z;
            m.m[3][3] = 1.0f;
            box = local.GetTransformedBox(m);
        }
        else
        {
            float fRadius = pObject->_Sphere.w;
            box._vMin = PblVector3(vPos.x - fRadius, vPos.y - fRadius, vPos.z - fRadius);
            box._vMax = PblVector3(vPos.x + fRadius, vPos.y + fRadius, vPos.z + fRadius);
        }
        if (!(box._vMin.z > vMin.z))
        {
            vMin.z = box._vMin.z;
        }
        if (!(box._vMin.y > vMin.y))
        {
            vMin.y = box._vMin.y;
        }
        if (!(box._vMin.x > vMin.x))
        {
            vMin.x = box._vMin.x;
        }
        if (!(vMax.z > box._vMax.z))
        {
            vMax.z = box._vMax.z;
        }
        if (!(vMax.y > box._vMax.y))
        {
            vMax.y = box._vMax.y;
        }
        if (!(vMax.x > box._vMax.x))
        {
            vMax.x = box._vMax.x;
        }
        fSumZ += vPos.z;
        fSumX += vPos.x;
        ++iCount;
        pObject = pObject->mNextSceneObject;
    } while (pObject);

    float fInv = 1.0f / (float)iCount;
    float fMidZ = fSumZ * fInv;
    float fMidX = fInv * fSumX;
    mMidPos.x = (vMax.x + vMin.x) * 0.5f;
    mMidPos.y = (vMax.y + vMin.y) * 0.5f;
    mMidPos.z = (vMax.z + vMin.z) * 0.5f;
    mSize.x = (vMax.x - vMin.x) * 0.5f;
    mSize.y = (vMax.y - vMin.y) * 0.5f;
    mSize.z = (vMax.z - vMin.z) * 0.5f;
    float fMaxSize = mSize.x > mSize.y ? mSize.x : mSize.y;
    if (!(fMaxSize > mSize.z))
    {
        fMaxSize = mSize.z;
    }

    if (iCount > 10)
    {
        RedSceneAABox* pBoxes[4];
        for (int i = 0; i < 4; ++i)
        {
            pBoxes[i] = new RedSceneAABox;
        }

        pObject = mObjects;
        if (pObject)
        {
            fMaxSize *= 0.75f;
            do
            {
                RedSceneObject* pNext = pObject->mNextSceneObject;
                RedSceneAABox* pBox;
                if (pObject->_Sphere.w > fMaxSize)
                {
                    pBox = this;
                }
                else if (fMidX > pObject->_Sphere.x)
                {
                    pBox = fMidZ > pObject->_Sphere.z ? pBoxes[0] : pBoxes[1];
                }
                else
                {
                    pBox = fMidZ > pObject->_Sphere.z ? pBoxes[2] : pBoxes[3];
                }
                pBox->Add(pObject);
                pObject = pNext;
            } while (pObject);
        }

        for (int i = 0; i < 4; ++i)
        {
            RedSceneAABox* pBox = pBoxes[i];
            if (pBox->SetupBox())
            {
                AddChild(pBox);
            }
            else
            {
                delete pBox;
            }
        }
    }
    return true;
}

void RedSceneAABox::AddChild(RedSceneAABox* pBox)
{
    if (mChild)
    {
        mChild->AddSibling(pBox);
    }
    else
    {
        mChild = pBox;
    }
    pBox->mParent = this;
}

// FUNCTION: SWBF2 0x006e3ce0
void SetupMinMaxIndex(const PblVector3& v, int& iMin, int& iMax)
{
    iMin = 0;
    iMax = 0;
    if (v.y > 0.0f)
    {
        iMin += 4;
    }
    else
    {
        iMax += 4;
    }
    if (0.0f > v.x)
    {
        iMin += 1;
    }
    else
    {
        iMax += 1;
    }
    if (v.z > 0.0f)
    {
        iMin += 2;
    }
    else
    {
        iMax += 2;
    }
}

// FUNCTION: SWBF2 0x006e3d30
void SetupPlanes(const PblMatrix& m, float fNear, float fWidth, float fHeight,
                 PblVector3* pPlanes, int* pMinIndex, int* pMaxIndex)
{
    const float* x = m.m[0];
    const float* z = m.m[2];
    float fHalfW = fWidth * 0.5f;
    float fHalfH = fHeight * 0.5f;
    PblVector3 vFront(z[0] * -fNear, z[1] * -fNear, z[2] * -fNear);
    PblVector3 vRight(x[0] * fHalfW, x[1] * fHalfW, x[2] * fHalfW);
    PblVector3 vUp((x[2] * z[1] - x[1] * z[2]) * fHalfH,
                   (x[0] * z[2] - x[2] * z[0]) * fHalfH,
                   (x[1] * z[0] - x[0] * z[1]) * fHalfH);

    PblVector3 v, n0, n1, n2, n3;
    v = PblVector3(vFront.x - vRight.x + vUp.x, vFront.y - vRight.y + vUp.y, vFront.z - vRight.z + vUp.z);
    D3DXVec3Normalize(&n0, &v);
    v = PblVector3(vRight.x + vFront.x + vUp.x, vRight.y + vFront.y + vUp.y, vRight.z + vFront.z + vUp.z);
    D3DXVec3Normalize(&n1, &v);
    v = PblVector3(vFront.x - vRight.x - vUp.x, vFront.y - vRight.y - vUp.y, vFront.z - vRight.z - vUp.z);
    D3DXVec3Normalize(&n2, &v);
    v = PblVector3(vRight.x + vFront.x - vUp.x, vRight.y + vFront.y - vUp.y, vRight.z + vFront.z - vUp.z);
    D3DXVec3Normalize(&n3, &v);

    v = PblVector3(n0.z * n2.y - n0.y * n2.z, n2.z * n0.x - n0.z * n2.x, n0.y * n2.x - n2.y * n0.x);
    D3DXVec3Normalize(&pPlanes[0], &v);
    v = PblVector3(n3.z * n1.y - n3.y * n1.z, n1.z * n3.x - n3.z * n1.x, n3.y * n1.x - n1.y * n3.x);
    D3DXVec3Normalize(&pPlanes[1], &v);
    v = PblVector3(n1.z * n0.y - n1.y * n0.z, n0.z * n1.x - n1.z * n0.x, n1.y * n0.x - n0.y * n1.x);
    D3DXVec3Normalize(&pPlanes[2], &v);
    v = PblVector3(n3.y * n2.z - n3.z * n2.y, n3.z * n2.x - n2.z * n3.x, n2.y * n3.x - n3.y * n2.x);
    D3DXVec3Normalize(&pPlanes[3], &v);

    for (int i = 0; i < 4; ++i)
    {
        SetupMinMaxIndex(pPlanes[i], pMinIndex[i], pMaxIndex[i]);
    }
}

// FUNCTION: SWBF2 0x006e4170
float RedSceneAABox::SetupRenderOrder(PblHeap<RedSceneAABox*, float>& heap)
{
    if (mSibling)
    {
        heap.Push(mSibling, mSibling->SetupRenderOrder(heap));
    }
    return -(sRedSceneCamera.mFront.x * (mMidPos.x - sRedSceneCamera.mPosition.x)
           + sRedSceneCamera.mFront.y * (mMidPos.y - sRedSceneCamera.mPosition.y)
           + sRedSceneCamera.mFront.z * (mMidPos.z - sRedSceneCamera.mPosition.z));
}

// FUNCTION: SWBF2 0x006e4240
void RedScene::SetupStaticWorld()
{
    mStaticWorldBox.SetupBox();
    sFarSceneObjects = new PblHeap<RedSceneObject*, float>(512, FLT_MAX, 0);
}

// FUNCTION: SWBF2 0x006e42f0
void RedSceneManager::Update(const RedCamera& camera)
{
    const pvs::ICamera* pCamera = &camera;
    pvs::IManager::GetInstance().Update(*pCamera, 0);
}

// FUNCTION: SWBF2 0x006e4330
void RedSceneManager::_Add(RedSceneObject* pObject)
{
    if (mDoScene)
    {
        mScene.Add(pObject);
    }
}

// FUNCTION: SWBF2 0x006e4350
void RedSceneManager::_AddTransparentObject(RedSceneObject* pObject, float fDist)
{
    PblHeap<RedSceneObject*, float>* pHeap =
        static_cast<PblHeap<RedSceneObject*, float>*>(mTransparentObjects);
    if (!pHeap->IsFull())
    {
        pHeap->Push(pObject, fDist);
    }
}

// FUNCTION: SWBF2 0x006e43f0
void RedSceneManager::_LeaveClient()
{
    mDoScene = false;
}

// FUNCTION: SWBF2 0x006e4400
void RedSceneManager::_EnterClient()
{
    mDoScene = true;
}

// FUNCTION: SWBF2 0x006e4420
void RedSceneManager::_Remove(RedSceneObject* pObject)
{
    if (pObject->mSceneAABox)
    {
        pObject->mSceneAABox->Remove(pObject);
    }
}

// FUNCTION: SWBF2 0x006e4440
void RedSceneManager::_Render(unsigned int uiFlags, const RedCamera* pCamera)
{
    BeginSceneRender();
    if (pCamera)
    {
        float fNear, fFar, fWidth, fHeight;
        pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
        sRedSceneCamera.Setup(pCamera, fNear, fFar);
    }
    else if (uiFlags & 0x40000)
    {
        if (!sFarSceneObjects)
        {
            return;
        }
        float fZoom = 1.0f;
        if (FLRenderer::GetFadeAdjustWithZoom())
        {
            fZoom = RedRenderer::GetCamera()->GetZoom();
        }
        float fFadeStart = FLRenderer::GetNearSceneFadeStart() * fZoom;
        float fFarRange = FLRenderer::GetFarSceneRange();
        if (fFadeStart > fFarRange)
        {
            return;
        }
        sRedSceneCamera.Setup(RedRenderer::GetCamera(), fFadeStart, fFarRange);
    }
    else
    {
        float fNear, fFar, fWidth, fHeight;
        RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
        float fZoom = RedRenderer::GetCamera()->GetZoom();
        float fFadeEnd = FLRenderer::GetNearSceneFadeEnd() * fZoom;
        sRedSceneCamera.Setup(RedRenderer::GetCamera(), fNear, fFadeEnd);
    }

    mState = (uiFlags & 0x40000) ? 2 : 1;
    gRenderCount = 0;
    gClippedCount = 0;
    gCullingCount = 0;
    gObjCullingCount = 0;
    RedRenderer::GetCamera()->GetFrustum(g_fCameraNear, g_fCameraFar,
                                         g_fCameraTanHalfFOVX, g_fCameraTanHalfFOVY);
    float fScale = 0.5f / g_fCameraNear;
    g_fCameraTanHalfFOVX *= fScale;
    g_fCameraTanHalfFOVY *= fScale;
    g_fCameraCosHalfFOVX = 1.0f / sqrtf(g_fCameraTanHalfFOVX * g_fCameraTanHalfFOVX + 1.0f);
    g_fCameraCosHalfFOVY = 1.0f / sqrtf(g_fCameraTanHalfFOVY * g_fCameraTanHalfFOVY + 1.0f);

    mScene.Render(uiFlags);
    if (uiFlags & 0x40000)
    {
        RenderFarObjects();
    }
    else
    {
        RedLodManager::Render(uiFlags);
    }
    _RenderTransparentObjects(uiFlags);
    EndSceneRender();
    mState = 0;
}

// FUNCTION: SWBF2 0x006e46a0
void RedSceneManager::_RenderTransparentObjects(unsigned int uiFlags)
{
    mTransparentPhase = true;
    PblHeap<RedSceneObject*, float>* pHeap =
        static_cast<PblHeap<RedSceneObject*, float>*>(mTransparentObjects);
    while (!pHeap->IsEmpty())
    {
        RedSceneObject* pObject = pHeap->Get(0);
        pHeap->Pop();
        pObject->Render(0, 1.0f, pObject->_uiRenderFlags | uiFlags);
    }
    mTransparentPhase = false;
}

// FUNCTION: SWBF2 0x006e4780
void RedSceneManager::_SetupStaticWorld()
{
    mScene.SetupStaticWorld();
}

// FUNCTION: SWBF2 0x006e4790
void RedSceneManager::_Update(RedSceneObject* pObject)
{
    if (pObject->mSceneAABox)
    {
        pObject->mSceneAABox->Move(pObject);
    }
}

// SYNTHETIC: SWBF2 0x00408990
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x004089b0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''
