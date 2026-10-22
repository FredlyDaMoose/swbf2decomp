#include "PropGenerator.h"
#include "PebbleFL/Common/PblVector.h"

#include <float.h>
#include <math.h>

#include "CameraManager.h"
#include "PropModelMesh.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Application/RedPath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "Entity.h"

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

namespace PropGenerator
{
// GLOBAL: SWBF2 0x01eaefd0
PropClusterInfo* mClusterMap;
// GLOBAL: SWBF2 0x01eaefd4
float mClusterCellWidth;
// GLOBAL: SWBF2 0x01eaefdc
PropCluster** mClusterObjectArray;
// GLOBAL: SWBF2 0x01eaefe0
int mNumClusterObjects;
// GLOBAL: SWBF2 0x01eaeff0
int mClusterMapWidth;
// GLOBAL: SWBF2 0x01eaefc7
bool mInitFlag;

// GLOBAL: SWBF2 0x01eaeff4
bool mLoaded;

// GLOBAL: SWBF2 0x01f03000
// SYNTHETIC: SWBF2 0x00406060
// `dynamic initializer for 'PropGenerator::mPropLayers''
// SYNTHETIC: SWBF2 0x00769360
// `dynamic atexit destructor for 'PropGenerator::mPropLayers''
PropLayer mPropLayers[4];

// GLOBAL: SWBF2 0x01f03250
// SYNTHETIC: SWBF2 0x00406090
// `dynamic initializer for 'PropGenerator::m_amCamera''

PblMatrix m_amCamera[1];

// GLOBAL: SWBF2 0x01efd044
// SYNTHETIC: SWBF2 0x004060a0
// `dynamic initializer for 'PropGenerator::mThread''
// SYNTHETIC: SWBF2 0x00769380
// `dynamic atexit destructor for 'PropGenerator::mThread''
PropGeneratorThread mThread;

// GLOBAL: SWBF2 0x01eaefcc
unsigned int mSeed;
// GLOBAL: SWBF2 0x01eaefd8
float mLayerCellWidthAspect;
// GLOBAL: SWBF2 0x01eaefe4
int mLayerMapWidth;
// GLOBAL: SWBF2 0x01eaefe8
float mLayerCellWidth;
// GLOBAL: SWBF2 0x01eaefec
float mClusterCellWidthAspect;
}

// GLOBAL: SWBF2 0x0078ac68
static const float mRandValues[256] = {
    0.288184f, 0.826782f, 0.788031f, 0.086964f, 0.348232f, 0.374394f,
    0.593966f, 0.67877f, 0.297205f, 0.563011f, 0.270326f, 0.445829f,
    0.346563f, 0.373405f, 0.109531f, 0.276601f, 0.580298f, 0.395091f,
    0.53165f, 0.636767f, 0.132068f, 0.081265f, 0.70544f, 0.136522f,
    0.909862f, 0.229348f, 0.975774f, 0.790865f, 0.582175f, 0.997894f,
    0.552188f, 0.064337f, 0.378356f, 0.154947f, 0.7062f, 0.030019f,
    0.874316f, 0.635953f, 0.493055f, 0.845912f, 0.404209f, 0.739388f,
    0.263503f, 0.001823f, 0.988716f, 0.538189f, 0.481244f, 0.039883f,
    0.920558f, 0.335143f, 0.611659f, 0.701829f, 0.397852f, 0.240334f,
    0.097461f, 0.706163f, 0.845955f, 0.198798f, 0.686874f, 0.60832f,
    0.8184f, 0.046489f, 0.629991f, 0.446967f, 0.617565f, 0.422694f,
    0.508908f, 0.129568f, 0.02569f, 0.617019f, 0.717279f, 0.760321f,
    0.786873f, 0.679067f, 0.854909f, 0.363607f, 0.730664f, 0.36894f,
    0.553008f, 0.706077f, 0.677887f, 0.776687f, 0.495739f, 0.170491f,
    0.51932f, 0.688814f, 0.653675f, 0.884437f, 0.107839f, 0.030001f,
    0.994799f, 0.557066f, 0.854674f, 0.958133f, 0.188667f, 0.726957f,
    0.170389f, 0.440775f, 0.06936f, 0.374233f, 0.141208f, 0.391219f,
    0.872848f, 0.100072f, 0.081328f, 0.69223f, 0.514394f, 0.326714f,
    0.851823f, 0.685605f, 0.421626f, 0.221108f, 0.086783f, 0.955685f,
    0.688894f, 0.295316f, 0.717297f, 0.484931f, 0.56854f, 0.886796f,
    0.433918f, 0.5097f, 0.56471f, 0.971699f, 0.97823f, 0.654336f,
    0.645332f, 0.014291f, 0.404137f, 0.171713f, 0.337875f, 0.80564f,
    0.699506f, 0.368343f, 0.724531f, 0.494684f, 0.408788f, 0.542181f,
    0.60243f, 0.264802f, 0.147239f, 0.011254f, 0.324523f, 0.359022f,
    0.147367f, 0.061225f, 0.086754f, 0.406992f, 0.228232f, 0.89004f,
    0.677139f, 0.256821f, 0.82822f, 0.877763f, 0.952395f, 0.764937f,
    0.991924f, 0.75973f, 0.557725f, 0.247079f, 0.451744f, 0.531157f,
    0.655394f, 0.54354f, 0.194847f, 0.161305f, 0.677723f, 0.901798f,
    0.156095f, 0.445353f, 0.370116f, 0.129652f, 0.927581f, 0.572084f,
    0.639752f, 0.721994f, 0.625387f, 0.235517f, 0.975574f, 0.914334f,
    0.304201f, 0.368616f, 0.455181f, 0.866215f, 0.560398f, 0.827182f,
    0.604264f, 0.427626f, 0.214236f, 0.602076f, 0.624209f, 0.191557f,
    0.21177f, 0.387883f, 0.753486f, 0.785809f, 0.137119f, 0.586147f,
    0.778447f, 0.60718f, 0.975714f, 0.0713f, 0.959176f, 0.451172f,
    0.019141f, 0.297416f, 0.885989f, 0.740319f, 0.052215f, 0.573901f,
    0.032234f, 0.30428f, 0.212904f, 0.588264f, 0.018997f, 0.914807f,
    0.165731f, 0.504077f, 0.857353f, 0.028735f, 0.479274f, 0.212256f,
    0.683775f, 0.442086f, 0.348399f, 0.563789f, 0.754341f, 0.808447f,
    0.051835f, 0.662032f, 0.713346f, 0.224721f, 0.785737f, 0.542703f,
    0.401959f, 0.337394f, 0.76384f, 0.819696f, 0.722537f, 0.672422f,
    0.724848f, 0.124836f, 0.32424f, 0.141892f, 0.237667f, 0.419709f,
    0.125547f
};

static const float MAX_CLUSTER_DISTANCE = 100.0f;

// GLOBAL: SWBF2 0x01efce04
// SYNTHETIC: SWBF2 0x004060c0
// `dynamic initializer for 'afScore''
// SYNTHETIC: SWBF2 0x00769390
// `dynamic atexit destructor for 'afScore''
static util::Vector<float> afScore;

// FUNCTION: SWBF2 0x0062b8e0
template bool util::BaseArray<float>::Reserve_(int, bool);

// FUNCTION: SWBF2 0x0062ba60
template float* util::TypedArrayAllocator<float, util::NewAllocator>::Allocate_(int& riCount);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0062a010
void PropGenerator::Init()
{
    m_amCamera[0] = _IdentityMatrix;
    mLayerCellWidth = 16.0f;
    mLayerCellWidthAspect = 1.0f / 16.0f;
    mLayerMapWidth = 512;
    mClusterMap = 0;
    mClusterMapWidth = 0;
    mClusterCellWidth = 0.0f;
    mClusterCellWidthAspect = 0.0f;
    mSeed = 0;
    mThread.ActivateThread(0, PER_FRAME);
    mInitFlag = true;
    mLoaded = false;
}

// FUNCTION: SWBF2 0x0062a0c0
void PropGenerator::Cleanup()
{
    afScore.Clear();
    mThread.DeactivateThread();

    if (mClusterMap)
    {
        delete[] mClusterMap;
        mClusterMap = 0;
    }

    if (mClusterObjectArray)
    {
        while (mNumClusterObjects)
        {
            mClusterObjectArray[--mNumClusterObjects]->DeleteObjects();
        }
        delete[] mClusterObjectArray;
        mClusterObjectArray = 0;
    }

    for (int i = 0; i < 4; i++)
    {
        mPropLayers[i].Clear();
    }

    PropObject::ClearObjectCaches();
    mInitFlag = false;
}

// FUNCTION: SWBF2 0x0062a170
bool PropGenerator::PropGeneratorThread::Update(float fDeltaTime)
{
    PropGenerator::Update();
    return true;
}

// SYNTHETIC: SWBF2 0x00629fe0
// PropGenerator::PropGeneratorThread::`scalar deleting destructor'

// GLOBAL: SWBF2 0x007e9039
bool flushClusterObjs_ = true;

// FUNCTION: SWBF2 0x0062a180
void PropGenerator::SortClusters()
{
    int iNumCameras = CameraManager::Instance()->GetNumCameras();

    int iFirst = 0;
    for (int i = 0; i < mNumClusterObjects; i++)
    {
        if (mClusterObjectArray && mClusterObjectArray[i]->mMapIndex == -1)
        {
            if (i != iFirst)
            {
                PropCluster* pSwap = mClusterObjectArray[iFirst];
                mClusterObjectArray[iFirst] = mClusterObjectArray[i];
                mClusterObjectArray[i] = pSwap;
            }
            iFirst++;
        }
    }

    int iNum = mNumClusterObjects - iFirst;
    if (iNum <= 1)
    {
        return;
    }

    if (afScore.GetSize() < iNum)
    {
        afScore.SetSize(iNum);
    }

    PropCluster** ppClusters = mClusterObjectArray + iFirst;
    for (int i = 0; i < iNum; i++)
    {
        PropCluster* pCluster = ppClusters[i];
        afScore[i] = FLT_MAX;
        for (int iCam = 0; iCam < iNumCameras; iCam++)
        {
            if (!(pCluster->mWasRenderedFlag & (1 << iCam)))
            {
                continue;
            }
            const PblVector3& vPos = pCluster->GetRenderPos();
            float dx = vPos.x - m_amCamera[iCam].m[3][0];
            float dy = vPos.y - m_amCamera[iCam].m[3][1];
            float dz = vPos.z - m_amCamera[iCam].m[3][2];
            float fDist = dx * dx + dy * dy + dz * dz;
            if (fDist < afScore[i])
            {
                afScore[i] = fDist;
            }
        }
    }

    for (int i = 0; i < iNum - 1; i++)
    {
        for (int j = i; j < iNum; j++)
        {
            float fI = afScore[i];
            float fJ = afScore[j];
            if (fJ > fI)
            {
                afScore[i] = fJ;
                afScore[j] = fI;
                PropCluster* pSwap = ppClusters[i];
                ppClusters[i] = ppClusters[j];
                ppClusters[j] = pSwap;
            }
        }
    }

    afScore.Clear();
}

// FUNCTION: SWBF2 0x0062a330
float PropGenerator::CalcDistScore_(const PropCluster* pCluster, const PblVector3& vPos, bool bAllCameras)
{
    float fBest = 10000000.0f;
    int iNumCameras = CameraManager::Instance()->GetNumCameras();
    for (int iCam = 0; iCam < iNumCameras; iCam++)
    {
        if (!bAllCameras && !(pCluster->mWasRenderedFlag & (1 << iCam)))
        {
            continue;
        }
        float dx = vPos.x - m_amCamera[iCam].m[3][0];
        float dy = vPos.y - m_amCamera[iCam].m[3][1];
        float dz = vPos.z - m_amCamera[iCam].m[3][2];
        float fDist = dx * dx + dy * dy + dz * dz;
        if (fDist < fBest)
        {
            fBest = fDist;
        }
    }
    return fBest;
}

// FUNCTION: SWBF2 0x0062a3c0
void PropGenerator::Update()
{
    if (!mLoaded)
    {
        return;
    }

    if (flushClusterObjs_)
    {
        FlushClusterObjects_();
    }

    CameraManager* pCameraManager = CameraManager::Instance();
    int iNumCameras = pCameraManager->GetNumCameras();

    for (int iCam = 0; iCam < iNumCameras; iCam++)
    {
        const PblMatrix& mCamera = pCameraManager->GetCamera(iCam)->GetMatrix();
        PblMatrix& mLast = m_amCamera[iCam];

        float dx = mCamera.m[3][0] - mLast.m[3][0];
        float dy = mCamera.m[3][1] - mLast.m[3][1];
        float dz = mCamera.m[3][2] - mLast.m[3][2];
        if (dx * dx + dy * dy + dz * dz > 40000.0f ||
            mLast.m[2][0] * mCamera.m[2][0] + mLast.m[2][1] * mCamera.m[2][1] +
                    mLast.m[2][2] * mCamera.m[2][2] < 0.7f)
        {
            for (int i = 0; i < mNumClusterObjects; i++)
            {
                mClusterObjectArray[i]->mWasRenderedFlag &= ~(1 << iCam);
                PropCluster* pCluster = mClusterObjectArray[i];
                if (!pCluster->mWasRenderedFlag && pCluster->mMapIndex != -1)
                {
                    mClusterMap[pCluster->mMapIndex].mOwner = 0;
                    mClusterObjectArray[i]->DeleteObjects();
                }
            }
        }

        mLast = mCamera;
    }

    SortClusters();

    PblVector3 avStart[4];
    PblVector3 avDir[4];
    PblVector3 avSide[4];
    float afSlope[4];
    float afOffset[4];
    float afDirLength[4];
    float afSideLength[4];
    float afNextDist[4];
    int iNumLines = 0;

    for (int iCam = 0; iCam < iNumCameras; iCam++)
    {
        RedCamera* pCamera = pCameraManager->GetCamera(iCam);
        float fNear, fFar, fWidth, fHeight;
        pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);

        float fScale = 1.0f / (fNear * 2.0f * pCamera->GetZoom());
        float fTanHeight = fScale * fHeight;
        float fTanWidth = fScale * fWidth;
        float fSecWidth = sqrtf(1.0f / (fTanWidth * fTanWidth) + 1.0f);
        float fSecHeight = sqrtf(1.0f / (fTanHeight * fTanHeight) + 1.0f);

        const PblMatrix& m = m_amCamera[iCam];
        const float* vRight = m.m[0];
        const float* vUp = m.m[1];
        const float* vAt = m.m[2];
        const float* vPos = m.m[3];

        float fWidthDist = mClusterCellWidth * fSecWidth * 0.5f;
        PblVector3 vWidthStart(vPos[0] + vAt[0] * fWidthDist,
                               vPos[1] + vAt[1] * fWidthDist,
                               vPos[2] + vAt[2] * fWidthDist);
        float fHeightDist = mClusterCellWidth * fSecHeight * 0.5f;
        PblVector3 vHeightStart(vPos[0] + vAt[0] * fHeightDist,
                                vPos[1] + vAt[1] * fHeightDist,
                                vPos[2] + vAt[2] * fHeightDist);
        float fWidthOffset = (fSecWidth - 1.0f) * mClusterCellWidth * 0.5f;
        float fHeightOffset = (fSecHeight - 1.0f) * mClusterCellWidth * 0.5f;
        bool bAtUp = vAt[1] > 0.0f;

        if ((vUp[1] + vAt[1] * fTanHeight > 0.0f) == bAtUp)
        {
            avStart[iNumLines] = vWidthStart;
            avDir[iNumLines] = PblVector3(vUp[0] * fTanHeight - vAt[0],
                                          vUp[1] * fTanHeight - vAt[1],
                                          vUp[2] * fTanHeight - vAt[2]);
            avSide[iNumLines] = PblVector3(vRight[0], vRight[1], vRight[2]);
            afSlope[iNumLines] = fTanWidth;
            afOffset[iNumLines] = fWidthOffset;
            iNumLines++;
        }
        if ((vAt[1] * fTanHeight - vUp[1] > 0.0f) == bAtUp)
        {
            avStart[iNumLines] = vWidthStart;
            avDir[iNumLines] = PblVector3(-vAt[0] - vUp[0] * fTanHeight,
                                          -vAt[1] - vUp[1] * fTanHeight,
                                          -vAt[2] - vUp[2] * fTanHeight);
            avSide[iNumLines] = PblVector3(-vRight[0], -vRight[1], -vRight[2]);
            afSlope[iNumLines] = fTanWidth;
            afOffset[iNumLines] = fWidthOffset;
            iNumLines++;
        }
        if ((vRight[1] + vAt[1] * fTanWidth > 0.0f) == bAtUp)
        {
            avStart[iNumLines] = vHeightStart;
            avDir[iNumLines] = PblVector3(vRight[0] * fTanWidth - vAt[0],
                                          vRight[1] * fTanWidth - vAt[1],
                                          vRight[2] * fTanWidth - vAt[2]);
            avSide[iNumLines] = PblVector3(-vUp[0], -vUp[1], -vUp[2]);
            afSlope[iNumLines] = fTanHeight;
            afOffset[iNumLines] = fHeightOffset;
            iNumLines++;
        }
        if ((vAt[1] * fTanWidth - vRight[1] > 0.0f) == bAtUp)
        {
            avStart[iNumLines] = vHeightStart;
            avDir[iNumLines] = PblVector3(-vAt[0] - vRight[0] * fTanWidth,
                                          -vAt[1] - vRight[1] * fTanWidth,
                                          -vAt[2] - vRight[2] * fTanWidth);
            avSide[iNumLines] = PblVector3(vUp[0], vUp[1], vUp[2]);
            afSlope[iNumLines] = fTanHeight;
            afOffset[iNumLines] = fHeightOffset;
            iNumLines++;
        }
    }

    for (int i = 0; i < iNumLines; i++)
    {
        afNextDist[i] = 0.0f;
        avDir[i].y = 0.0f;
        avStart[i].y = 0.0f;
        avSide[i].y = 0.0f;
        afDirLength[i] = sqrtf(avDir[i].x * avDir[i].x + avDir[i].z * avDir[i].z);
        afSideLength[i] = sqrtf(avSide[i].x * avSide[i].x + avSide[i].z * avSide[i].z);
    }

    float fStep = mClusterCellWidth * 0.7071f;
    float fMaxDist;
    if (pCameraManager->GetNumCameras() > 1)
    {
        fMaxDist = mClusterCellWidth + 50.0f;
    }
    else
    {
        fMaxDist = mClusterCellWidth * 2.0f + 100.0f;
    }

    int iNextCluster = 0;
    for (float fDist = 0.0f; fDist < fMaxDist; fDist += fStep)
    {
        for (int i = 0; i < iNumLines; i++)
        {
            if (afNextDist[i] > fDist)
            {
                continue;
            }

            float fLineDist = afOffset[i] + fDist;
            float fHalfWidth = afSlope[i] * fLineDist;
            float fX = avStart[i].x + avDir[i].x * fLineDist;
            float fZ = avStart[i].z + avDir[i].z * fLineDist;
            int iNumSteps = (int)(afSideLength[i] * fHalfWidth * 2.0f / fStep + 0.999f);
            if (iNumSteps < 1)
            {
                iNumSteps = 1;
            }
            float fSideStep = fHalfWidth * 2.0f / (float)iNumSteps;

            for (int iStep = 0; iStep <= iNumSteps; iStep++)
            {
                float fSide = (float)iStep * fSideStep - fHalfWidth;
                int iIndex = GetClusterIndex(fX + avSide[i].x * fSide, -(fZ + avSide[i].z * fSide));
                if (iIndex < 0 || !mClusterMap[iIndex].mLayerInfo || mClusterMap[iIndex].mOwner)
                {
                    continue;
                }

                PropCluster* pCluster = mClusterObjectArray[iNextCluster++];
                float fHalfMap = (float)mClusterMapWidth * 0.5f;
                float fCellX = ((float)(iIndex % mClusterMapWidth) - fHalfMap + 0.5f) * mClusterCellWidth;
                float fCellZ = ((float)(iIndex / mClusterMapWidth) - fHalfMap + 0.5f) * mClusterCellWidth;

                if (pCluster->mMapIndex != -1)
                {

                    PblVector3 vCellPos(fCellX, 10.0f, -fCellZ);
                    float fNewScore = CalcDistScore_(pCluster, vCellPos, true);
                    if (CalcDistScore_(pCluster, pCluster->GetRenderPos(), false) < fNewScore)
                    {
                        continue;
                    }
                    mClusterMap[pCluster->mMapIndex].mOwner = 0;
                    pCluster->DeleteObjects();
                }

                pCluster->Setup(iIndex, fCellX, fCellZ);
                mClusterMap[iIndex].mOwner = pCluster->mId;
                if (iNextCluster >= mNumClusterObjects)
                {
                    return;
                }
            }

            if (afDirLength[i] * fMaxDist > fStep * 0.5f)
            {
                afNextDist[i] = fStep * 0.5f / afDirLength[i] + fDist;
            }
            else
            {
                afNextDist[i] = fMaxDist + 1.0f;
            }
        }
    }

    ResetClusterRenderedFlags();
}

namespace TreeLine { void CreateTreeLine(PblFileChunk& chunk); }
void CreateTreeLineOnPath(unsigned int uPathName, PblFileChunk& chunk);

// FUNCTION: SWBF2 0x0065c890
void CreateTreeLineOnPath(unsigned int uPathName, PblFileChunk& chunk)
{
    EntityClass* aClasses[10];
    EntityClass* aOtherClasses[5];
    int iNumClasses = 0;
    int iNumOtherClasses = 0;
    float fSpacing = 18.0f;
    bool bReverse = false;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        unsigned int uProperty = child._pFile->Read32();
        child._pFile->Read8();
        switch (uProperty)
        {
        case 0x6850acc6:
        {
            EntityClass* pClass = EntityClass::Find(child._pFile->Read32());
            if (pClass)
            {
                aClasses[iNumClasses++] = pClass;
            }
            break;
        }
        case 0xef838cc7:
        {
            EntityClass* pClass = EntityClass::Find(child._pFile->Read32());
            if (pClass)
            {
                aOtherClasses[iNumOtherClasses++] = pClass;
            }
            break;
        }
        case 0xbad71c66:
            bReverse = true;
            break;
        case 0x2eb31462:
            fSpacing = child._pFile->ReadFloat();
            break;
        }
    }

    const RedPath* pPath = RedPath::Find(uPathName);
    if (!pPath || pPath->GetNumPoints() < 2)
    {
        return;
    }

    PblVector3 vPrevious = pPath->GetPointPosition(0);
    const float fSpacingSquared = fSpacing * fSpacing;
    for (int iPoint = 1; iPoint < pPath->GetNumPoints(); ++iPoint)
    {
        const PblVector3& vEnd = pPath->GetPointPosition(iPoint);
        PblVector3 vDelta(vEnd.x - vPrevious.x, vEnd.y - vPrevious.y, vEnd.z - vPrevious.z);
        while (vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z > fSpacingSquared)
        {
            const float fInvLength = 1.0f / sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
            const PblVector3 vDirection(vDelta.x * fInvLength, vDelta.y * fInvLength, vDelta.z * fInvLength);
            if (iNumClasses)
            {
                const float fSign = bReverse ? -1.0f : 1.0f;
                const PblVector3 vCenter(vPrevious.x + vDirection.x * fSpacing * 0.5f,
                                         vPrevious.y + vDirection.y * fSpacing * 0.5f,
                                         vPrevious.z + vDirection.z * fSpacing * 0.5f);
                const PblVector3 vForward(-vDirection.z * fSign, 0.0f, vDirection.x * fSign);
                EntityDesc desc;
                desc.mMatrix = DirectionMatrix(vForward, PblVector3(0.0f, 1.0f, 0.0f), vCenter);
                aClasses[PblRandom::_Global.GetInt() % iNumClasses]->Build(desc);
            }
            vPrevious.x += vDirection.x * fSpacing;
            vPrevious.y += vDirection.y * fSpacing;
            vPrevious.z += vDirection.z * fSpacing;
            vDelta = PblVector3(vEnd.x - vPrevious.x, vEnd.y - vPrevious.y, vEnd.z - vPrevious.z);
        }
    }
}

// FUNCTION: SWBF2 0x0065cd80
void TreeLine::CreateTreeLine(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        const unsigned int uProperty = child._pFile->Read32();
        child._pFile->Read8();
        if (uProperty == 0x84874d36)
        {
            const unsigned int uPathName = child._pFile->Read32();
            PblFileChunk pathChunk = chunk.ReadNextChild();
            CreateTreeLineOnPath(uPathName, pathChunk);
        }
    }
}

// FUNCTION: SWBF2 0x0062b110
void PropGenerator::FlushClusterObjects_()
{
    if (!mClusterObjectArray)
    {
        return;
    }

    flushClusterObjs_ = false;
    for (int i = 0; i < mNumClusterObjects; i++)
    {
        if (!mClusterObjectArray[i]->mWasRenderedFlag)
        {
            for (int iLayer = 0; iLayer < 4; iLayer++)
            {
                mClusterObjectArray[i]->UncreateObjects(iLayer);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0062b170
void PropGenerator::ResetClusterRenderedFlags()
{
    if (!mClusterObjectArray)
    {
        return;
    }

    for (int i = 0; i < mNumClusterObjects; i++)
    {
        mClusterObjectArray[i]->mWasRenderedFlag = 0;
    }
}

namespace PropGenerator
{

// FUNCTION: SWBF2 0x0062b1b0
static unsigned char GetLayers(int x, int z)
{
    if (x < 0 || x >= mLayerMapWidth || z < 0 || z >= mLayerMapWidth)
    {
        return 0;
    }

    const unsigned char* pFoliage = RedTerrain::_Foliage;
    if (!pFoliage)
    {
        return 0;
    }

    const unsigned char ucLayers = pFoliage[(x + z * mLayerMapWidth) >> 1];
    if (x & 1)
    {
        return ucLayers & 0xf;
    }
    return ucLayers >> 4;
}
}

// FUNCTION: SWBF2 0x0062b200
int PropGenerator::GetClusterIndex(float x, float z)
{
    int iX = (int)(x * mClusterCellWidthAspect + (float)(mClusterMapWidth >> 1));
    if (iX < 0 || iX >= mClusterMapWidth)
    {
        return -1;
    }
    int iZ = (int)(z * mClusterCellWidthAspect + (float)(mClusterMapWidth >> 1));
    if (iZ < 0 || iZ >= mClusterMapWidth)
    {
        return -1;
    }
    return iZ * mClusterMapWidth + iX;
}

// FUNCTION: SWBF2 0x0062b250
void PropGenerator::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        const unsigned int uiHash = child._pFile->Read32();
        child._pFile->Read8();

        if (uiHash == 0x07a640f6)
        {
            PropLayer& layer = mPropLayers[(int)child._pFile->ReadFloat()];
            layer.Clear();
            PblFileChunk layerChunk = chunk.ReadNextChild();
            layer.Read(layerChunk);
            mLoaded = true;
        }
        else if (uiHash == 0x40b06ff7)
        {
            PblFileChunk treeLineChunk = chunk.ReadNextChild();
            TreeLine::CreateTreeLine(treeLineChunk);
        }
    }

    if (RedTerrain::_Foliage)
    {
        mLayerMapWidth = RedTerrain::_iPointsPerDimension / 2;
        mLayerCellWidth = RedTerrain::_fMetersPerPoint * 2.0f;
        mLayerCellWidthAspect = 1.0f / mLayerCellWidth;
        SetupClusters();
    }
    else
    {
        mLoaded = false;
    }
}

// FUNCTION: SWBF2 0x0062b370
void PropGenerator::SetupClusters()
{
    mClusterMapWidth = (mLayerMapWidth + 1) / 2;
    mClusterCellWidth = mLayerCellWidth * 2.0f;
    mClusterCellWidthAspect = 1.0f / mClusterCellWidth;
    mClusterMap = new PropClusterInfo[mClusterMapWidth * mClusterMapWidth];

    for (int z = 0; z < mClusterMapWidth; z++)
    {
        for (int x = 0; x < mClusterMapWidth; x++)
        {
            const int iIndex = z * mClusterMapWidth + x;
            mClusterMap[iIndex].Setup(x, z);

            for (int iLayer = 0; iLayer < 4; iLayer++)
            {
                if (!((mClusterMap[iIndex].mLayerInfo >> iLayer) & 1))
                {
                    continue;
                }

                SetRandSeed(iIndex + z * 2 + iLayer);
                PropLayer& layer = mPropLayers[iLayer];
                for (int cz = z * 2; cz < z * 2 + 2; cz++)
                {
                    for (int cx = x * 2; cx < x * 2 + 2; cx++)
                    {
                        if (!((GetLayers(cx, cz) >> iLayer) & 1))
                        {
                            continue;
                        }

                        for (int i = 0; i < layer.mObjectsPerCell; i++)
                        {
                            layer.CreateCollisionObjects(cx, cz, mLayerMapWidth >> 1, 0.0f);
                        }
                    }
                }
            }
        }
    }

    const float fCells = MAX_CLUSTER_DISTANCE / mClusterCellWidth + 5.0f;
    mNumClusterObjects = (int)(fCells * fCells);
    if (mNumClusterObjects > 254)
    {
        mNumClusterObjects = 254;
    }
    mClusterObjectArray = new PropCluster*[mNumClusterObjects];
    for (int i = 0; i < mNumClusterObjects; i++)
    {
        mClusterObjectArray[i] = new PropCluster;
        mClusterObjectArray[i]->Init((unsigned char)(i + 1), mClusterCellWidth * 0.707f);
    }
}

// FUNCTION: SWBF2 0x0062b000
void PropGenerator::AddObjectsFromClusterIndex(int iClusterIndex, int iLayer, PblList<PropObject>& objects)
{
    if (!((mClusterMap[iClusterIndex].mLayerInfo >> iLayer) & 1))
    {
        return;
    }

    const int x = iClusterIndex % mClusterMapWidth;
    const int z = iClusterIndex / mClusterMapWidth;
    SetRandSeed(iClusterIndex + z * 2 + iLayer);
    PropLayer& layer = mPropLayers[iLayer];
    for (int cz = z * 2; cz < z * 2 + 2; cz++)
    {
        for (int cx = x * 2; cx < x * 2 + 2; cx++)
        {
            if (!((GetLayers(cx, cz) >> iLayer) & 1))
            {
                continue;
            }
            for (int i = 0; i < layer.mObjectsPerCell; i++)
            {
                PropObject* pObject = layer.CreateObject(cx, cz, mLayerMapWidth >> 1, 0.0f);
                if (pObject)
                {
                    objects.AddLast(&pObject->mNode, pObject);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0062b5d0
void PropClusterInfo::Setup(int x, int z)
{
    mLayerInfo = 0;
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            mLayerInfo |= PropGenerator::GetLayers(x * 2 + i, z * 2 + j);
        }
    }
    mOwner = mLayerInfo ? 0 : 0xff;
}

// FUNCTION: SWBF2 0x0062b630
void PropGenerator::SetRandSeed(unsigned int uiSeed)
{
    mSeed = uiSeed;
}

// FUNCTION: SWBF2 0x0062b640
float PropGenerator::GetRand()
{
    return mRandValues[(unsigned char)++mSeed];
}

// FUNCTION: SWBF2 0x0062b6d0
void PropGenerator::GetTerrainY(float x, float z, float& y)
{
    y = RedTerrain::GetInterpolatedHeight(x, z);
}

// FUNCTION: SWBF2 0x0062b6f0
float PropGenerator::GetLayerMaxDist(int iLayer)
{
    return mPropLayers[iLayer].mMaxDistance;
}

// FUNCTION: SWBF2 0x0062b700
const PropObject* PropGenerator::FindPropObject(const PblVector3& vPos, bool bParam)
{
    const PropObject* pBest = 0;
    float fBestVis = FLT_MAX;
    for (int i = 0; i < mNumClusterObjects; ++i)
    {
        const PropCluster* pCluster = mClusterObjectArray[i];
        if (!pCluster->IsInScene())
        {
            continue;
        }
        const PblVector3& vCenter = pCluster->GetRenderPos();
        float dx = vPos.x - vCenter.x;
        float dy = vPos.y - vCenter.y;
        float dz = vPos.z - vCenter.z;
        if (dx * dx + dy * dy + dz * dz < 900.0f)
        {
            const PropObjectMesh* pObject =
                static_cast<const PropObjectMesh*>(pCluster->FindPropObject(vPos, bParam));
            if (pObject)
            {
                float fVis = pObject->mModel->mAIVisFactorCrouch + pObject->mModel->mAIVisFactorStand;
                if (fVis < fBestVis)
                {
                    fBestVis = fVis;
                    pBest = pObject;
                }
            }
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0062b7c0
const RedLodData* PropGenerator::GetLodData(int iIndex)
{
    unsigned char ucLayers = mClusterMap[iIndex].mLayerInfo;
    const RedLodData* pBest = 0;
    for (int i = 0; i < 4; i++)
    {
        if (!((ucLayers >> i) & 1))
        {
            continue;
        }
        const RedLodData* pLodData = mPropLayers[i].mLodData;
        if (pLodData && (!pBest || pLodData->mMaxDist > pBest->mMaxDist))
        {
            pBest = pLodData;
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0062b810
template bool util::Vector<float>::SetSize(int);

namespace PropGenerator {

// FUNCTION: SWBF2 0x0062b660
void GetTerrainYAndNormal(float x, float z, float& y, PblVector3& vNormal)
{
    vNormal = RedTerrain::GetInterpolatedNormal(x, z);
    y = RedTerrain::GetInterpolatedHeight(x, z);
}

}
