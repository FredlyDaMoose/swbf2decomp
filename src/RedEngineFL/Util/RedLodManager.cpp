#include "RedLodManager.h"
#include "Battlefront2/Source/Battlefront2Lod.h"
#include "RedSceneManager.h"
#include "RedSceneObject.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x007e2484
static float rMinScreenSize = 5.0f;

// GLOBAL: SWBF2 0x007e2480
static float rMaxVertexDensity = 1.0f;

// GLOBAL: SWBF2 0x007e2488
bool bRenderSoldiers = true;

// GLOBAL: SWBF2 0x009cab1c
bool bShowMissingLOD;

static const float rPixelCount = 798720.0f;

// FUNCTION: SWBF2 0x00717380
int GetNextAvailableLod(const RedLodData* pLodData, int iLod)
{
    int i;
    for (i = iLod + 1; i < 4; i++)
    {
        if (pLodData->_LodBitmask & (1 << i))
        {
            break;
        }
    }
    return i;
}

namespace RedLodManager
{

// GLOBAL: SWBF2 0x009cab18
RedLodClass* mClass;

// GLOBAL: SWBF2 0x009cab14
int mNumClass;

// FUNCTION: SWBF2 0x00717270
void Add(RedSceneObject* pObject, float fPriority)
{
    float fKey = fPriority < 100.0f ? fPriority : 100.0f;
    PblHeap<RedSceneObject*, float>* pHeap = mClass[pObject->GetLodData()->GetLodClass()].mHeap;
    if (!pHeap->IsFull())
    {
        pHeap->Push(pObject, fKey);
    }
}

// FUNCTION: SWBF2 0x00717300
void Cleanup()
{
    for (int i = 0; i < mNumClass; i++)
    {
        delete mClass[i].mHeap;
    }
    delete[] mClass;
    mClass = 0;
    mNumClass = 0;
}

// FUNCTION: SWBF2 0x007173b0
void Init(int iNumClass)
{
    mNumClass = iNumClass;
    mClass = new RedLodClass[iNumClass];
    for (int i = 0; i < iNumClass; i++)
    {
        mClass[i].mFadeSpeed = 1.0f;
        mClass[i].mHeap = 0;
        for (int j = 0; j < 4; j++)
        {
            mClass[i].mData[j].mCostHint = -1;
            mClass[i].mData[j].mMaxCost = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00717370
float GetMinScreenSize()
{
    return rMinScreenSize;
}

// FUNCTION: SWBF2 0x00717490
bool IsFull(int iClass)
{
    return mClass[iClass].mHeap->mCount == mClass[iClass].mHeap->mMaxCount;
}

// FUNCTION: SWBF2 0x007174b0
void Render(unsigned int uiFlags)
{
    float fMinSize = rMinScreenSize * rMinScreenSize / rPixelCount;

    for (int iClass = 0; iClass < mNumClass; iClass++)
    {
        int iCost[4] = {0, 0, 0, 0};

        if (!bRenderSoldiers && iClass == 2)
        {
            mClass[iClass].mHeap->Flush();
            continue;
        }

        if (iClass == 3)
        {
            int iCount = 400;
            while (!mClass[3].mHeap->IsEmpty() && iCount--)
            {
                RedSceneObject* pObject = mClass[3].mHeap->Get(0);
                mClass[3].mHeap->Pop();
                RedSceneManager::AddTransparentObject(pObject);
            }
        }
        else
        {
            float fDetail = RedRenderer::pcGetRenderOptions().mNearSceneFadeFactor * 2.0f;

            while (!mClass[iClass].mHeap->IsEmpty())
            {
                RedSceneObject* pObject = mClass[iClass].mHeap->Get(0);
                float fSize = mClass[iClass].mHeap->Value(0);
                mClass[iClass].mHeap->Pop();

                if (fMinSize > fSize)
                {
                    continue;
                }

                float fPriorityMod = pObject->GetPriorityMod();
                if (fPriorityMod > 1.1f)
                {
                    fPriorityMod += 0.0001f;
                }

                const RedLodData* pLodData = pObject->GetLodData();
                float fInvSize = 1.0f / (fPriorityMod * fSize);
                float fTarget = rMaxVertexDensity * rPixelCount * fDetail;

                int iLod;
                for (iLod = 0; iLod < 4; iLod++)
                {
                    if (!pLodData->HasLod(iLod))
                    {
                        continue;
                    }

                    int iNextLod = GetNextAvailableLod(pLodData, iLod);
                    float fDensity = pLodData->GetCost(iLod) * fInvSize;
                    float fNextDensity = fDensity;
                    if (iNextLod < 4)
                    {
                        fNextDensity = pLodData->GetCost(iNextLod) * fInvSize;
                    }

                    bool bCompare = bShowMissingLOD ||
                                    ((pLodData->mLowRezFlags & 1) && iNextLod != 4);

                    if (fDensity < 0.001f)
                    {
                        fDensity = 0.001f;
                    }
                    if (fNextDensity < 0.001f)
                    {
                        fNextDensity = 0.001f;
                    }

                    float fError = fDensity / fTarget;
                    if (fTarget / fDensity > fError)
                    {
                        fError = fTarget / fDensity;
                    }
                    float fNextError = fNextDensity / fTarget;
                    if (fTarget / fNextDensity > fNextError)
                    {
                        fNextError = fTarget / fNextDensity;
                    }

                    if (bCompare && fError > fNextError)
                    {
                        continue;
                    }

                    if (iCost[iLod] + pLodData->GetCost(iLod) <= mClass[iClass].mData[iLod].mMaxCost)
                    {
                        break;
                    }
                }

                if (iLod == 4)
                {
                    if (!(pLodData->mLowRezFlags & 1))
                    {
                        continue;
                    }
                    iLod = 3;
                }

                unsigned int uiObjectFlags = uiFlags;
                if (uiFlags & 0x20000000)
                {
                    const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
                    const PblVector4& vSphere = pObject->_Sphere;
                    float fFront = vSphere.w - ((vSphere.x - mCamera.m[3][0]) * mCamera.m[2][0] +
                                                (vSphere.y - mCamera.m[3][1]) * mCamera.m[2][1] +
                                                (vSphere.z - mCamera.m[3][2]) * mCamera.m[2][2]);
                    if (RedRenderer::pcGetNearSceneStart() > fFront)
                    {
                        uiObjectFlags &= ~0x20000000;
                    }
                }

                pObject->Render((unsigned char)iLod, 1.0f, uiObjectFlags | pObject->_uiRenderFlags);
                iCost[iLod] += pLodData->GetCost(iLod);
            }
        }

        mClass[iClass].mHeap->Flush();
    }
}

// FUNCTION: SWBF2 0x00717830
void SetClassLodHint(int iClass, int iHint0, int iHint1, int iHint2)
{
    mClass[iClass].mData[0].mCostHint = iHint0;
    mClass[iClass].mData[1].mCostHint = iHint1;
    mClass[iClass].mData[2].mCostHint = iHint2;
}

// FUNCTION: SWBF2 0x00717870
void SetClassMaxCost(int iClass, int iHeapSize, int iCost0, int iCost1,
                     int iCost2, int iCost3)
{
    mClass[iClass].mData[0].mMaxCost = iCost0;
    mClass[iClass].mData[1].mMaxCost = iCost1;
    mClass[iClass].mData[2].mMaxCost = iCost2;
    mClass[iClass].mData[3].mMaxCost = iCost3;
    mClass[iClass].mHeap = new PblHeap<RedSceneObject*, float>(
        iHeapSize > 256 ? iHeapSize : 256, 1000000.0f, 0);
}

// FUNCTION: SWBF2 0x00717960
void SetFadeSpeed(int iClass, float fSpeed)
{
    mClass[iClass].mFadeSpeed = fSpeed;
}

}
