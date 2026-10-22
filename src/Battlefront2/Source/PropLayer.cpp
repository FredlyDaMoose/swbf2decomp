#include "PropGenerator.h"
#include "PropModelMesh.h"
#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

// FUNCTION: SWBF2 0x0062bab0
PropLayer::PropLayer()
{
    mLodData = 0;
    mNumModels = 0;
    mSpreadFactor = 1;
    Clear();
}

// FUNCTION: SWBF2 0x0062bae0
PropLayer::~PropLayer()
{
    Clear();
}

// FUNCTION: SWBF2 0x0062bd10
void PropLayer::Clear()
{
    mNumModels = 0;
}

// FUNCTION: SWBF2 0x0062baf0
void PropLayer::Read(PblFileChunk& chunk)
{
    mObjectsPerCell = 1;
    mNumModels = 0;
    mSpreadFactor = 1;
    mMaxDistance = 1.0f;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        const unsigned int uiHash = child._pFile->Read32();
        child._pFile->Read8();

        if (uiHash == 0xa100bebc)
        {
            PropModelMesh* pModel = new PropModelMesh;
            PblFileChunk modelChunk = chunk.ReadNextChild();
            pModel->Read(modelChunk);
            if (pModel->mMaxDist > mMaxDistance)
            {
                mMaxDistance = pModel->mMaxDist;
            }
            mModels[mNumModels++] = pModel;
        }
        else if (uiHash == 0xdbe276e9)
        {
            const float fSpread = child._pFile->ReadFloat();
            mSpreadFactor = (int)fSpread;
            if (mSpreadFactor == 0)
            {
                mSpreadFactor = 1;
                if (fSpread > 0.0f)
                {
                    mObjectsPerCell = (int)(1.0f / fSpread);
                }
            }
        }
    }

    float fTotal = 0.0f;
    float fScale = 1.0f;
    for (int i = 0; i < mNumModels; i++)
    {
        fTotal += mModels[i]->mFrequency;
    }
    if (fTotal != 0.0f)
    {
        fScale = 1.0f / fTotal;
    }

    float fSum = 0.0f;
    for (int i = 0; i < mNumModels; i++)
    {
        fSum += mModels[i]->mFrequency * fScale;
        mModels[i]->mFrequency = fSum;
    }

    mLodData = new PropLayerLodData(mMaxDistance + 22.624f);
}

// FUNCTION: SWBF2 0x0062be50
void PropLayer::CreateCollisionObjects(int x, int z, int iOffset, float)
{
    if (!mNumModels)
    {
        return;
    }

    if (mSpreadFactor > 1 && (x % mSpreadFactor > 0 || z % mSpreadFactor > 0))
    {
        return;
    }

    const float fRand = PropGenerator::GetRand();
    for (int i = 0; i < mNumModels; i++)
    {
        if (mModels[i]->mFrequency >= fRand)
        {
            const float fRandX = PropGenerator::GetRand();
            const float fRandZ = PropGenerator::GetRand();
            const float fRotation = PropGenerator::GetRand() * 6.1415f;
            const float fSpread = (float)mSpreadFactor;
            const float fHalf = fSpread * 0.45f;
            const float fX = ((float)(x - iOffset) + 0.5f - fHalf + fSpread * fRandX * 0.9f) * PropGenerator::mLayerCellWidth;
            const float fZ = ((float)(z - iOffset) + 0.5f - fHalf + fSpread * fRandZ * 0.9f) * PropGenerator::mLayerCellWidth;
            mModels[i]->CreateCollisionObject(fX, fZ, fRotation);
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0062bd20
PropObject* PropLayer::CreateObject(int x, int z, int iOffset, float)
{
    if (!mNumModels)
    {
        return 0;
    }

    if (mSpreadFactor > 1 && (x % mSpreadFactor > 0 || z % mSpreadFactor > 0))
    {
        return 0;
    }

    const float fRand = PropGenerator::GetRand();
    for (int i = 0; i < mNumModels; i++)
    {
        if (mModels[i]->mFrequency >= fRand)
        {
            const float fRandX = PropGenerator::GetRand();
            const float fRandZ = PropGenerator::GetRand();
            const float fRotation = PropGenerator::GetRand() * 6.1415f;
            const float fSpread = (float)mSpreadFactor;
            const float fHalf = fSpread * 0.45f;
            const float fX = ((float)(x - iOffset) + 0.5f - fHalf + fSpread * fRandX * 0.9f) * PropGenerator::mLayerCellWidth;
            const float fZ = ((float)(z - iOffset) + 0.5f - fHalf + fSpread * fRandZ * 0.9f) * PropGenerator::mLayerCellWidth;
            return mModels[i]->CreateObject(fX, fZ, fRotation);
        }
    }
    return 0;
}
