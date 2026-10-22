#include "pcRedShadowProxy.h"

#include "Battlefront2/Source/Battlefront2Lod.h"

#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedStencilShadow.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"

#include <math.h>
#include <d3dx9math.h>

#include "RedEngineFL/Application/RedTerrain.h"

// GLOBAL: SWBF2 0x007e12a0
PblList<RedShadowProxy> RedShadowProxy::_sShadowItemsList;

class RedNoLodData : public RedLodData
{
public:
    RedNoLodData();
};
extern RedNoLodData rednoloddata;

// FUNCTION: SWBF2 0x006ed690
RedShadowProxy::RedShadowProxy()
    : RedSceneObject(&rednoloddata)
{
    m_fShadowLength = 0.0f;
    mMatrix = 0;
    mModel = 0;
}

// FUNCTION: SWBF2 0x006ed6e0
RedShadowProxy::~RedShadowProxy()
{
    if (mObjectNode.IsInList())
    {
        _sShadowItemsList.Remove(&mObjectNode);
    }
}

// FUNCTION: SWBF2 0x006ed780
void RedShadowProxy::BuildData(const RedModel* pModel, const PblMatrix* pMatrix, float fLength,
                               const PblVector3& vPos)
{
    if (!pModel || !pModel->_pStencilShadow)
    {
        return;
    }

    RedDirectionalLight* pLight = RedLight::GetGlobalDirectionalLight(0);
    if (!pLight->IsShadowCaster())
    {
        pLight = RedLight::GetGlobalDirectionalLight(1)->IsShadowCaster()
                     ? RedLight::GetGlobalDirectionalLight(1)
                     : NULL;
    }

    const PblBox& box = pModel->_BoundingBox;
    const D3DXMATRIX* pMat = (const D3DXMATRIX*)pMatrix;

    if (pLight)
    {
        float fMaxHit = 0.0f;
        for (int i = 0; i < 8; ++i)
        {
            D3DXVECTOR3 vCorner((i & 4) ? box._vMax.x : box._vMin.x,
                                (i & 2) ? box._vMax.y : box._vMin.y,
                                (i & 1) ? box._vMax.z : box._vMin.z);
            PblVector3 vStart;
            D3DXVec3TransformCoord((D3DXVECTOR3*)&vStart, &vCorner, pMat);
            PblVector3 vEnd(vStart.x + pLight->m_vDirection.x * 500.0f,
                            vStart.y + pLight->m_vDirection.y * 500.0f,
                            vStart.z + pLight->m_vDirection.z * 500.0f);
            float fHit = RedTerrain::RayHit(vStart, vEnd, 0.05f, 5.0f);
            if (fHit <= 1.0f && fHit > fMaxHit)
            {
                fMaxHit = fHit;
            }
        }

        float fHeight = box._vMax.y - box._vMin.y;
        float fLen;
        if (pLight->m_vDirection.y < -0.5f)
        {
            fLen = fHeight / -pLight->m_vDirection.y;
        }
        else
        {
            fLen = fHeight * 2.0f;
        }

        if (fMaxHit == 1.0f)
        {
            m_fShadowLength = fLen + 10.0f;
        }
        else
        {
            m_fShadowLength = fMaxHit * 500.0f + 2.0f;
        }
    }
    else
    {
        float dx = box._vMin.x - box._vMax.x;
        float dy = box._vMin.y - box._vMax.y;
        float dz = box._vMin.z - box._vMax.z;
        m_fShadowLength = (float)sqrt(dx * dx + dy * dy + dz * dz) * 2.0f;
    }

    float fShadowLength = m_fShadowLength;
    float dx = box._vMin.x - box._vMax.x;
    float dy = box._vMin.y - box._vMax.y;
    float dz = box._vMin.z - box._vMax.z;
    float fMaxLength = (float)sqrt(dx * dx + dy * dy + dz * dz) * 3.0f;
    if (fMaxLength > fShadowLength)
    {
        fMaxLength = fShadowLength;
    }
    m_fShadowLength = fMaxLength;

    D3DXVECTOR3 vCenter((box._vMax.x + box._vMin.x) * 0.5f, (box._vMax.y + box._vMin.y) * 0.5f,
                        (box._vMax.z + box._vMin.z) * 0.5f);
    D3DXVec3TransformCoord((D3DXVECTOR3*)&m_vCasterCenter, &vCenter, pMat);

    float hx = (box._vMax.x - box._vMin.x) * 0.5f;
    float hy = (box._vMax.y - box._vMin.y) * 0.5f;
    float hz = (box._vMax.z - box._vMin.z) * 0.5f;
    m_fCasterRadius = (float)sqrt(hx * hx + hy * hy + hz * hz);

    if (pLight)
    {
        float fHalf = m_fShadowLength * 0.5f;
        PblVector3 vRenderPos(m_vCasterCenter.x + fHalf * pLight->m_vDirection.x,
                              m_vCasterCenter.y + pLight->m_vDirection.y * fHalf,
                              m_vCasterCenter.z + pLight->m_vDirection.z * fHalf);
        SetRenderPos(vRenderPos);
    }
    else
    {
        SetRenderPos(m_vCasterCenter);
    }
    SetRenderRadius(m_fShadowLength * 0.5f + m_fCasterRadius);

    _uiRenderFlags &= ~2u;
    Activate();

    mModel = const_cast<RedModel*>(pModel);
    mMatrix = const_cast<PblMatrix*>(pMatrix);
    _sShadowItemsList.Add(&mObjectNode, this);
}

// FUNCTION: SWBF2 0x006ed750
void RedShadowProxy::AddToScene2()
{
    for (PblList<RedShadowProxy>::Itor it(_sShadowItemsList); (RedShadowProxy*)it; ++it)
    {
        it->Activate();
    }
}

// FUNCTION: SWBF2 0x006edc20
void RedShadowProxy::RemoveFromScene2()
{
    for (PblList<RedShadowProxy>::Itor it(_sShadowItemsList); (RedShadowProxy*)it; ++it)
    {
        it->Deactivate();
    }
}

// FUNCTION: SWBF2 0x006edc50
void RedShadowProxy::Render(unsigned char ucViewport, float fDist, unsigned int uiFlags)
{
    if (RedStencilShadow::GetShadowEnable() && !(uiFlags & 0x200000) &&
        mModel && mModel->_pStencilShadow)
    {
        RedStencilShadow::VisibilityTestT test = RedStencilShadow::ShadowVolumeVisibilityTest(
            m_vCasterCenter, m_fCasterRadius, m_fShadowLength);
        if (test != RedStencilShadow::TEST_NOT_VISIBLE)
        {
            RedRenderer::SetStencilShadowZFail(test == RedStencilShadow::TEST_CONTAINS_CAMERA);
            RedRenderer::SetStencilShadowLength(m_fShadowLength);
            mModel->_pStencilShadow->Render(mMatrix, NULL, NULL, true);
            RedRenderer::SetStencilShadowZFail(true);
        }
    }
}

// FUNCTION: SWBF2 0x006edbd0
void RedShadowProxy::CleanUp()
{
}

// FUNCTION: SWBF2 0x006edd00
void RedShadowProxy::Reset()
{
    _sShadowItemsList.RemoveAll();
}
