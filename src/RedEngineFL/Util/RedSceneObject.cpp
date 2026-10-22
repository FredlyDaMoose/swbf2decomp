#include <math.h>
#include <d3dx9math.h>

#include "RedSceneObject.h"
#include "RedSceneManager.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "Battlefront2/Source/Battlefront2Lod.h"

class RedNoLodData : public RedLodData
{
public:
    RedNoLodData();
};

// FUNCTION: SWBF2 0x006e1eb0
RedNoLodData::RedNoLodData()
{
    _LodCost[0] = -1;
    _LodCost[1] = -1;
    _LodCost[2] = -1;
    _LodClass = -1;
}

// GLOBAL: SWBF2 0x009a0854
// SYNTHETIC: SWBF2 0x00408980
// `dynamic initializer for 'rednoloddata''
RedNoLodData rednoloddata;

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// FUNCTION: SWBF2 0x006e2000
bool BoxClipsNearPlane(const RedCamera* pCamera, const PblVector3& vCenter,
                       const PblVector3& vExtents)
{
    const PblMatrix& mView = pCamera->_MatrixInverse;
    D3DXVECTOR3 vViewCenter;
    D3DXVec3TransformCoord(&vViewCenter, (const D3DXVECTOR3*)&vCenter, (const D3DXMATRIX*)&mView);

    const float fRadius = (float)fabs(mView.m[0][2]) * vExtents.x
                        + (float)fabs(mView.m[1][2]) * vExtents.y
                        + (float)fabs(mView.m[2][2]) * vExtents.z;

    return pCamera->_fNear + vViewCenter.z > -fRadius;
}

// FUNCTION: SWBF2 0x006e1ee0
RedSceneObject::RedSceneObject(const RedLodData* pLodData)
    : mNextSceneObject(0)
    , mPrevSceneObject(0)
    , mSceneAABox(0)
    , _pLodData(const_cast<RedLodData*>(pLodData))
    , _uiRenderFlags(0)
    , _bActive(false)
    , _fPriorityMod(1.0f)
{
    _Sphere.x = 0.0f;
    _Sphere.y = 0.0f;
    _Sphere.z = 0.0f;
    _Sphere.w = -1.0f;
    _iFrameNum[0] = (int)0xFAFAFAFA;
}

// FUNCTION: SWBF2 0x006e1f50
RedSceneObject::~RedSceneObject()
{
    Deactivate();
}

// SYNTHETIC: SWBF2 0x006e1fb0
// RedSceneObject::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006e1fe0
void RedSceneObject::Activate()
{
    if (!_bActive)
    {
        _bActive = true;
        RedSceneManager::Add(this);
    }
}

// FUNCTION: SWBF2 0x006e20b0
void RedSceneObject::Deactivate()
{
    if (_bActive)
    {
        RedSceneManager::Remove(this);
        _bActive = false;
    }
}

// FUNCTION: SWBF2 0x006e20d0
bool RedSceneObject::IsClipped() const
{
    return (_uiRenderFlags & 1) != 0;
}

// FUNCTION: SWBF2 0x006e20e0
bool RedSceneObject::IsOccluded() const
{
    if (RedRenderer::GetFrameNumber() - _iFrameNum[FLRenderer::GetCurrentViewport()] == 1)
    {
        unsigned int uiPixelCount;
        if (RedRenderer::GetOcclusionQueryPixelCount(
                _uiOcclusionIndex[FLRenderer::GetCurrentViewport()], uiPixelCount) &&
            uiPixelCount == 0)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006e2130
void RedSceneObject::SetRenderPos(const PblVector3& vPos)
{
    if (!_bActive)
    {
        _Sphere.x = vPos.x;
        _Sphere.y = vPos.y;
        _Sphere.z = vPos.z;
    }
    else if (vPos.x != _Sphere.x || vPos.y != _Sphere.y || vPos.z != _Sphere.z)
    {
        _Sphere.x = vPos.x;
        _Sphere.y = vPos.y;
        _Sphere.z = vPos.z;
        RedSceneManager::Update(this);
    }
}

// FUNCTION: SWBF2 0x006e21a0
void RedSceneObject::SetRenderRadius(float fRadius)
{
    if (!_bActive)
    {
        _Sphere.w = fRadius;
    }
    else if (fRadius != _Sphere.w)
    {
        _Sphere.w = fRadius;
        RedSceneManager::Update(this);
    }
}

class PblMatrix;
extern PblMatrix gMatrixIdentity;

// GLOBAL: SWBF2 0x007df8f4
float MAX_OCCLUSION_RADIUS = 12.0f;
// GLOBAL: SWBF2 0x009a0848
float MIN_OCCLUSION_RADIUS;

// FUNCTION: SWBF2 0x006e21e0
void RedSceneObject::UpdateOcclusionQuery(unsigned int uiViewport)
{
    float fRadius = _Sphere.w;
    if (fRadius > MAX_OCCLUSION_RADIUS || MIN_OCCLUSION_RADIUS > fRadius)
    {
        return;
    }
    PblVector3 vExtents(fRadius, fRadius, fRadius);
    const PblVector3& vCenter = GetRenderPos();
    if (BoxClipsNearPlane(RedRenderer::GetCamera(), vCenter, vExtents))
    {
        return;
    }
    _iFrameNum[FLRenderer::GetCurrentViewport()] = RedRenderer::GetFrameNumber();
    PblMatrix mWorld = gMatrixIdentity;
    mWorld.m[3][0] = vCenter.x;
    mWorld.m[3][1] = vCenter.y;
    mWorld.m[3][2] = vCenter.z;
    mWorld.m[3][3] = 1.0f;
    _uiOcclusionIndex[FLRenderer::GetCurrentViewport()] =
        RedRenderer::SubmitOcclusionQuery(mWorld, OCCLUSION_QUERY_BOX, vExtents, false);
}
