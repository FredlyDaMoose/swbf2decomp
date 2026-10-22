#include "PropModel.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

class PropObjectMesh
{
public:
    static void ClearCache();
};

// FUNCTION: SWBF2 0x0062bf80
PblVector3 PropObject::GetPos()
{

    const float fScale = -16384.0f;
    return PblVector3(gVectorUnitY.x * fScale, gVectorUnitY.y * fScale, gVectorUnitY.z * fScale);
}

// FUNCTION: SWBF2 0x0062bfd0
const PblMatrix& PropObject::GetMatrix()
{
    return gMatrixIdentity;
}

// FUNCTION: SWBF2 0x0062bfe0
void PropModel::CreateCollisionObject(float, float, float)
{
}

// FUNCTION: SWBF2 0x0062bff0
PropObject::PropObject()
{
}

// FUNCTION: SWBF2 0x0062c050
PropObject::~PropObject()
{
}

// FUNCTION: SWBF2 0x0062c060
void PropObject::ClearObjectCaches()
{
    PropObjectMesh::ClearCache();
}

// FUNCTION: SWBF2 0x0062c070
PropModel::PropModel()
    : mMaxDist(1000.0f),
      mFrequency(1.0f),
      mStiffness(0.0f)
{
}

// FUNCTION: SWBF2 0x0062c0c0
PropModel::~PropModel()
{
}

// FUNCTION: SWBF2 0x0062c0d0
void PropModel::ReadData(unsigned int uiHash, int iVersion, PblFileChunk& chunk)
{
    switch (uiHash)
    {
    case 0x2fb31c01:
        mFrequency = chunk._pFile->ReadFloat();
        break;
    case 0x39f85b7d:
        mMaxDist = chunk._pFile->ReadFloat();
        break;
    case 0x495f91ae:
        mStiffness = chunk._pFile->ReadFloat();
        break;
    }
}

// SYNTHETIC: SWBF2 0x0062c020
// PropObject::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0062c090
// PropModel::`scalar deleting destructor'
