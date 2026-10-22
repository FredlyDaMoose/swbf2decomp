#include <math.h>
#include <stdio.h>
#include <d3dx9math.h>

#include "Godray.h"
#include "CameraManager.h"
#include "CollisionManager.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x01e56bcc
GodrayManager* GodrayManager::s_pInstance;

// FUNCTION: SWBF2 0x0053e480
bool GodrayEffectObject::IsEffectActive() const
{
    return true;
}

// FUNCTION: SWBF2 0x0053e490
void GodrayEffectObject::UpdatedWorldMatrix()
{
    GodrayManager::s_pInstance->UpdateGodrayPosition(m_hGodray, *(const PblVector3*)&m_mWorldMatrix.m[3][0]);
}

// SYNTHETIC: SWBF2 0x0053e4b0
// GodrayEffectObject::`scalar deleting destructor'
// SYNTHETIC: SWBF2 0x005409d7
// [thunk]:GodrayEffectObject::`scalar deleting destructor' adjustor{24}

GodrayEffectObject::~GodrayEffectObject()
{
    GodrayManager::s_pInstance->RemoveGodray(m_hGodray);
}

// FUNCTION: SWBF2 0x0053e500
void GodrayManager::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new GodrayManager();
    }
}

// FUNCTION: SWBF2 0x0053e570
void GodrayManager::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0053e590
GodrayManager::GodrayManager()
    : FLEffect(0xBA5B0537u, 3)
{
    Enable(false);
    m_pFirstFreeGodray = 0;
    m_pFirstAllocatedGodray = 0;
    m_aGodrays = 0;
    m_uiNumGodrays = 0;
    m_aVisibleGodrays = 0;
    m_uiNumVisibleGodrays = 0;
    m_pShader = 0;
}

// SYNTHETIC: SWBF2 0x0053e670
// GodrayManager::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0053e6a0
GodrayManager::~GodrayManager()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x0053e710
void GodrayManager::Init()
{
    Enable(false);
    m_vTextureVelocity = PblVector3(0.0f, -0.1f, 0.0f);
    m_uiMaxGodrays = 0;
    m_uiMaxVisibleGodrays = 0;
    m_fMaxViewRadius = 80.0f;
    m_fFadeViewRadius = 70.0f;
    m_fMaxGodrayLength = 40.0f;
    m_fOffsetAngle = 0.0f;
    m_uiTextureHash = 0xAC3632BEu;
    m_fTexScaleW = 1.5f;
    m_fTexScaleL = 1.5f;
    m_fTextureRandomWalkSpeed = 0.05f;
    m_uiMinRaysPerGodray = 3;
    m_uiMaxRaysPerGodray = 6;
    m_fRadiusForMaxRays = 2.0f;
    m_pFirstFreeGodray = 0;
    m_pFirstAllocatedGodray = 0;
    m_aGodrays = 0;
    m_uiNumGodrays = 0;
    m_aVisibleGodrays = 0;
    m_uiNumVisibleGodrays = 0;
    m_uiFrame = 1;
    m_fTime = 0.0f;
    m_fTimeLastVisibleCheck = 0.0f;
    m_fTexOffsetW = 0.0f;
    m_fTexOffsetL = 0.0f;
    PlatformInit();
}

// FUNCTION: SWBF2 0x0053e800
void GodrayManager::Cleanup()
{
    PlatformCleanup();
    delete[] m_aGodrays;
    m_aGodrays = 0;
    m_pFirstAllocatedGodray = 0;
    m_pFirstFreeGodray = 0;
    m_uiNumGodrays = 0;
    delete[] m_aVisibleGodrays;
    m_aVisibleGodrays = 0;
    m_uiNumVisibleGodrays = 0;
}

// FUNCTION: SWBF2 0x0053e850
void GodrayManager::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3C6468F4u:
            m_uiTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x34904329u:
            m_fOffsetAngle = data.GetFloatArg(0) * 0.017453292f;
            break;
        case 0x209F61AFu:
            m_vTextureVelocity = PblVector3(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            break;
        case 0x3F7364CDu:
            m_fRadiusForMaxRays = data.GetFloatArg(0);
            break;
        case 0x5D6614A1u:
            m_fMaxGodrayLength = data.GetFloatArg(0);
            break;
        case 0x6D9B436Eu:
            m_fTexScaleW = data.GetFloatArg(0);
            m_fTexScaleL = data.GetFloatArg(1);
            break;
        case 0x74E6240Du:
            m_uiMaxGodrays = data.GetUint32Arg(0);
            if (m_uiMaxGodrays < 1)
            {
                m_uiMaxGodrays = 1;
            }
            break;
        case 0x7C0DAA5Du:
            m_uiMaxRaysPerGodray = data.GetUint32Arg(0);
            if (m_uiMaxRaysPerGodray > 8)
            {
                m_uiMaxRaysPerGodray = 8;
            }
            else if (m_uiMaxRaysPerGodray < 1)
            {
                m_uiMaxRaysPerGodray = 1;
            }
            break;
        case 0xEF3FAB75u:
            m_fTextureRandomWalkSpeed = data.GetFloatArg(0);
            break;
        case 0xAF8BB8CEu:
            Enable(data.GetUint32Arg(0) != 0);
            break;
        case 0xD760C55Du:
            m_uiMaxVisibleGodrays = data.GetUint32Arg(0);
            if (m_uiMaxVisibleGodrays < 1)
            {
                m_uiMaxVisibleGodrays = 1;
            }
            break;
        case 0xDA62178Fu:
            m_fMaxViewRadius = data.GetFloatArg(0);
            break;
        case 0xF21598CFu:
            m_uiMinRaysPerGodray = data.GetUint32Arg(0);
            if (m_uiMinRaysPerGodray > 8)
            {
                m_uiMinRaysPerGodray = 8;
            }
            else if (m_uiMinRaysPerGodray < 1)
            {
                m_uiMinRaysPerGodray = 1;
            }
            break;
        case 0xF300705Du:
            m_fFadeViewRadius = data.GetFloatArg(0);
            break;
        }
    }

    delete[] m_aGodrays;
    m_aGodrays = 0;
    if (m_uiMaxGodrays)
    {
        m_aGodrays = new Godray[m_uiMaxGodrays];
        for (unsigned int i = 1; i < m_uiMaxGodrays; ++i)
        {
            m_aGodrays[i - 1].pNext = &m_aGodrays[i];
        }
        m_aGodrays[m_uiMaxGodrays - 1].pNext = 0;
    }
    m_pFirstAllocatedGodray = 0;
    m_pFirstFreeGodray = m_aGodrays;
    m_uiNumGodrays = 0;

    delete[] m_aVisibleGodrays;
    m_aVisibleGodrays = 0;
    if (m_uiMaxVisibleGodrays)
    {
        m_aVisibleGodrays = new VisibleGodray[m_uiMaxVisibleGodrays];
    }
    m_uiNumVisibleGodrays = 0;

    PlatformInit();
}

// FUNCTION: SWBF2 0x0053eb80
FLEffectClass* GodrayManager::BuildClass(PblConfig& config) const
{
    return new GodrayClass(config);
}

// FUNCTION: SWBF2 0x0053ebf0
GodrayManager::Godray* GodrayManager::NewGodray()
{
    Godray* pGodray = m_pFirstFreeGodray;
    if (!pGodray)
    {
        return 0;
    }
    m_pFirstFreeGodray = pGodray->pNext;
    pGodray->pNext = m_pFirstAllocatedGodray;
    ++m_uiNumGodrays;
    m_pFirstAllocatedGodray = pGodray;
    return pGodray;
}

// FUNCTION: SWBF2 0x0053ec20
void GodrayManager::DeleteGodray(Godray* pGodray, Godray* pPrev)
{
    if (pPrev)
    {
        pPrev->pNext = pGodray->pNext;
    }
    else
    {
        m_pFirstAllocatedGodray = pGodray->pNext;
    }
    pGodray->pNext = m_pFirstFreeGodray;
    --m_uiNumGodrays;
    m_pFirstFreeGodray = pGodray;

    if (pGodray->uiFrameLastVisible == 0)
    {
        for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
        {
            if (m_aVisibleGodrays[i].pGodray == pGodray)
            {
                pGodray->uiFrameLastVisible = m_uiFrame;
                --m_uiNumVisibleGodrays;
                m_aVisibleGodrays[i] = m_aVisibleGodrays[m_uiNumVisibleGodrays];
                return;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0053eca0
unsigned int GodrayManager::AddGodray(const GodrayDesc& desc, const PblVector3& vPos)
{
    Godray* pGodray = NewGodray();
    if (!pGodray)
    {
        return 0;
    }
    pGodray->fLength = -1.0f;
    pGodray->vStartPos = vPos;
    pGodray->uiFrameLastVisible = 1;
    pGodray->fLifetimeMin = desc.fLifetimeMin;
    pGodray->fLifetimeMax = desc.fLifetimeMax;
    pGodray->fRadius = desc.fRadius;
    pGodray->color = desc.color;
    pGodray->fAlphaMin = desc.fAlphaMin;
    pGodray->fAlphaMax = desc.fAlphaMax;
    return (unsigned int)(pGodray - m_aGodrays) + 1;
}

// FUNCTION: SWBF2 0x0053ed20
void GodrayManager::UpdateGodrayPosition(unsigned int uiId, const PblVector3& vPos)
{
    if (uiId == 0 || uiId > m_uiMaxGodrays || !m_aGodrays)
    {
        return;
    }
    Godray* pGodray = &m_aGodrays[uiId - 1];
    const float fDX = pGodray->vStartPos.x - vPos.x;
    const float fDY = pGodray->vStartPos.y - vPos.y;
    const float fDZ = pGodray->vStartPos.z - vPos.z;
    if (fDX * fDX + fDY * fDY + fDZ * fDZ <= 0.0001f)
    {
        return;
    }
    pGodray->vStartPos = vPos;
    pGodray->fLength = -1.0f;
    if (pGodray->uiFrameLastVisible == 0)
    {
        for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
        {
            if (m_aVisibleGodrays[i].pGodray == pGodray)
            {
                UpdateVisibleGodray(&m_aVisibleGodrays[i]);
                return;
            }
        }
    }
}

// FUNCTION: SWBF2 0x0053edf0
void GodrayManager::RemoveGodray(unsigned int uiId)
{
    if (uiId == 0 || uiId > m_uiMaxGodrays || !m_aGodrays)
    {
        return;
    }
    Godray* pTarget = &m_aGodrays[uiId - 1];
    Godray* pPrev = 0;
    for (Godray* pGodray = m_pFirstAllocatedGodray; pGodray; pGodray = pGodray->pNext)
    {
        if (pGodray == pTarget)
        {
            DeleteGodray(pGodray, pPrev);
        }
        pPrev = pGodray;
    }
}

// FUNCTION: SWBF2 0x0053ee40
void GodrayManager::Update(float fDeltaTime)
{
    if (!m_uiNumGodrays)
    {
        return;
    }

    if (++m_uiFrame == 0)
    {
        m_uiFrame = 1;
    }
    m_fTime += fDeltaTime;

    const PblVector3 vSunDir = RedLight::GetGlobalDirectionalLight(0)->m_vDirection;

    PblVector3 vAxisIn(vSunDir.y * 0.0f - vSunDir.z, vSunDir.z * 0.0f - vSunDir.x * 0.0f,
                       vSunDir.x - vSunDir.y * 0.0f);
    PblVector3 vAxis;
    D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)&vAxisIn);
    D3DXMATRIX mRotation;
    D3DXMatrixRotationAxis(&mRotation, (const D3DXVECTOR3*)&vAxis, m_fOffsetAngle);
    PblVector3 vLengthDir;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vLengthDir,
                            (const D3DXVECTOR3*)&RedLight::GetGlobalDirectionalLight(0)->m_vDirection,
                            &mRotation);
    m_vLengthDir = vLengthDir;

    m_Rand.SetSeed((unsigned int)(m_fTime * 4099.0f));

    for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
    {
        VisibleGodray* pVisible = &m_aVisibleGodrays[i];
        if (pVisible->iVisible && AnimateVisibleGodray(pVisible, m_fTime, fDeltaTime))
        {
            m_aVisibleGodrays[i].iVisible = 0;
        }
        else
        {
            m_aVisibleGodrays[i].pGodray->uiFrameLastVisible = m_uiFrame;
            --m_uiNumVisibleGodrays;
            m_aVisibleGodrays[i] = m_aVisibleGodrays[m_uiNumVisibleGodrays];
            --i;
        }
    }

    if (m_uiNumVisibleGodrays < m_uiMaxVisibleGodrays && m_fTime > m_fTimeLastVisibleCheck + 2.0f)
    {
        Godray* pGodray = ChooseNewGodray();
        if (pGodray)
        {
            VisibleGodray* pVisible = CreateVisibleGodray(pGodray);
            pVisible->iVisible = 1;
            AnimateVisibleGodray(pVisible, m_fTime, fDeltaTime);
        }
        m_fTimeLastVisibleCheck = m_fTime;
    }

    if (m_uiNumVisibleGodrays)
    {
        const RedColorValue& color = RedLight::GetGlobalDirectionalLight(0)->m_color;
        RedColor c;
        c.b = (unsigned char)(int)(color.b * 255.0f);
        c.g = (unsigned char)(int)(color.g * 255.0f);
        c.r = (unsigned char)(int)(color.r * 255.0f);
        c.a = (unsigned char)(int)(color.a * 255.0f);
        m_color = c;
    }
}

// FUNCTION: SWBF2 0x0053f0b0
void GodrayManager::Render(int value)
{
    if (!m_uiNumVisibleGodrays)
    {
        return;
    }

    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    const float* vFront = pCamera->GetMatrix().m[2];
    const float* vPos = pCamera->GetMatrix().m[3];

    const float fViewDot =
        -(m_vLengthDir.y * vFront[1] + m_vLengthDir.x * vFront[0] + m_vLengthDir.z * vFront[2]);
    m_bTowardCamera = fViewDot < 0.0f;
    m_fNearDist = fNear - (vPos[1] * vFront[1] + vPos[0] * vFront[0] + vFront[2] * vPos[2]);

    bool bAnyVisible = false;
    for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
    {
        m_aVisibleGodrays[i].fAlpha = m_aVisibleGodrays[i].fAlphaAnimated;
        if (ClipVisibleGodray(&m_aVisibleGodrays[i]))
        {
            bAnyVisible = true;
            ++m_aVisibleGodrays[i].iVisible;
        }
    }
    if (!bAnyVisible)
    {
        return;
    }

    PblVector3 vWidth(m_vLengthDir.y * vFront[2] - m_vLengthDir.z * vFront[1],
                      m_vLengthDir.z * vFront[0] - m_vLengthDir.x * vFront[2],
                      m_vLengthDir.x * vFront[1] - m_vLengthDir.y * vFront[0]);
    PblVector3 vWidthDir;
    D3DXVec3Normalize((D3DXVECTOR3*)&vWidthDir, (const D3DXVECTOR3*)&vWidth);
    m_vWidthDir = vWidthDir;

    const float fNX = m_vLengthDir.z * m_vWidthDir.y - m_vLengthDir.y * m_vWidthDir.z;
    const float fNY = m_vLengthDir.x * m_vWidthDir.z - m_vWidthDir.x * m_vLengthDir.z;
    const float fNZ = m_vWidthDir.x * m_vLengthDir.y - m_vLengthDir.x * m_vWidthDir.y;
    const float fViewDot2 = fViewDot * fViewDot;
    const float fSideFactor = 1.0f - fViewDot2 * fViewDot2;
    for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
    {
        VisibleGodray& visible = m_aVisibleGodrays[i];
        const float fSide = (vPos[0] - visible.vStartPosClipped.x) * fNX +
                            (vPos[1] - visible.vStartPosClipped.y) * fNY +
                            (vPos[2] - visible.vStartPosClipped.z) * fNZ;
        float fFade = 1.0f;
        if (fSide <= 0.0f)
        {
            fFade = 0.0f;
        }
        else if (fSide < visible.pGodray->fRadius)
        {
            fFade = fSide / visible.pGodray->fRadius;
        }
        visible.fAlpha = fFade * fSideFactor * visible.fAlpha;
    }

    m_fTexOffsetAngle = (float)atan2((double)-vFront[0], (double)-vFront[2]);
    m_fTexOffsetW -= (float)(int)m_fTexOffsetW;
    m_fTexOffsetL -= (float)(int)m_fTexOffsetL;
    PlatformRender((unsigned int)value);
}

// FUNCTION: SWBF2 0x0053f430
bool GodrayManager::ClipVisibleGodray(VisibleGodray* pVisible) const
{
    const float* vCamPos = RedRenderer::GetCamera()->GetMatrix().m[3];
    const float* vFront = RedRenderer::GetCamera()->GetMatrix().m[2];
    const Godray* pGodray = pVisible->pGodray;
    const PblVector3& vStart = pGodray->vStartPos;
    const PblVector3& vEnd = pGodray->vEndPos;
    float fDist2;

    if (m_bTowardCamera)
    {
        const float fStartDot = -(vStart.x * vFront[0] + vStart.y * vFront[1] + vStart.z * vFront[2]);
        if (m_fNearDist > fStartDot)
        {
            pVisible->fAlpha = 0.0f;
            return false;
        }
        const float fDX = vEnd.x - vCamPos[0];
        const float fDY = vEnd.y - vCamPos[1];
        const float fDZ = vEnd.z - vCamPos[2];
        fDist2 = fDY * fDY + fDX * fDX + fDZ * fDZ;
        if (fDist2 > m_fMaxViewRadius * m_fMaxViewRadius)
        {
            pVisible->fAlpha = 0.0f;
            return false;
        }
        pVisible->vStartPosClipped = vStart;
        const float fEndDot = -(vFront[0] * vEnd.x + vEnd.y * vFront[1] + vEnd.z * vFront[2]);
        if (m_fNearDist > fEndDot)
        {
            const float fT = (fStartDot - m_fNearDist) / (fStartDot - fEndDot);
            pVisible->vEndPosClipped = PblVector3((vEnd.x - vStart.x) * fT + vStart.x,
                                                  (vEnd.y - vStart.y) * fT + vStart.y,
                                                  (vEnd.z - vStart.z) * fT + vStart.z);
            pVisible->fLengthClipped = pGodray->fLength * fT;
        }
        else
        {
            pVisible->vEndPosClipped = vEnd;
            pVisible->fLengthClipped = pGodray->fLength;
        }
        pVisible->bStartClipped = false;
    }
    else
    {
        const float fEndDot = -(vEnd.x * vFront[0] + vEnd.y * vFront[1] + vEnd.z * vFront[2]);
        if (m_fNearDist > fEndDot)
        {
            pVisible->fAlpha = 0.0f;
            return false;
        }
        const float fDX = vStart.x - vCamPos[0];
        const float fDY = vStart.y - vCamPos[1];
        const float fDZ = vStart.z - vCamPos[2];
        fDist2 = fDY * fDY + fDX * fDX + fDZ * fDZ;
        if (fDist2 > m_fMaxViewRadius * m_fMaxViewRadius)
        {
            pVisible->fAlpha = 0.0f;
            return false;
        }
        pVisible->vEndPosClipped = vEnd;
        const float fStartDot = -(vFront[0] * vStart.x + vStart.y * vFront[1] + vStart.z * vFront[2]);
        if (m_fNearDist > fStartDot)
        {
            const float fT = (fStartDot - m_fNearDist) / (fStartDot - fEndDot);
            pVisible->vStartPosClipped = PblVector3((vEnd.x - vStart.x) * fT + vStart.x,
                                                    (vEnd.y - vStart.y) * fT + vStart.y,
                                                    (vEnd.z - vStart.z) * fT + vStart.z);
            pVisible->bStartClipped = true;
            pVisible->fLengthClipped = (1.0f - fT) * pGodray->fLength;
        }
        else
        {
            pVisible->vStartPosClipped = vStart;
            pVisible->fLengthClipped = pGodray->fLength;
            pVisible->bStartClipped = false;
        }
    }

    const float fFade2 = m_fFadeViewRadius * m_fFadeViewRadius;
    if (fDist2 > fFade2)
    {
        const float fMax2 = m_fMaxViewRadius * m_fMaxViewRadius;
        pVisible->fAlpha = (fMax2 - fDist2) / (fMax2 - fFade2) * pVisible->fAlpha;
    }
    return true;
}

// FUNCTION: SWBF2 0x0053f790
bool GodrayManager::AnimateVisibleGodray(VisibleGodray* pVisible, float fTime, float fDeltaTime)
{
    const float fAge = fTime - pVisible->fTime0;
    if (fAge < 1.0f)
    {
        pVisible->fAlphaAnimated = fAge / 1.0f;
    }

    for (unsigned int i = 0; i < pVisible->uiNumRays; ++i)
    {
        VisibleGodray::Ray& ray = pVisible->aRays[i];
        float fT = (fTime - ray.fTime0) / ray.fLifeTime;
        if (fT >= 1.0f)
        {
            CreateRay(pVisible, i);
            fT = 0.0f;
        }
        float fAlpha;
        if (fT < 0.4f)
        {
            fAlpha = fT * 2.5f;
        }
        else if (fT > 0.6f)
        {
            fAlpha = (1.0f - fT) * 2.5f;
        }
        else
        {
            fAlpha = 1.0f;
        }
        ray.fAlpha = ray.fAlpha0 * fAlpha;
        ray.fRadius = ((1.0f - fAlpha) * 0.5f + fAlpha) * ray.fRadius0;
        ray.fTexOffsetL0 += (m_Rand.GetFloat() * 2.0f - 1.0f) * (m_fTextureRandomWalkSpeed * fDeltaTime);
        ray.fTexOffsetW0 += (m_Rand.GetFloat() * 2.0f - 1.0f) * (m_fTextureRandomWalkSpeed * fDeltaTime);
    }
    return true;
}

// FUNCTION: SWBF2 0x0053f980
bool GodrayManager::UpdateVisibleGodray(VisibleGodray* pVisible)
{
    return AnimateVisibleGodray(pVisible, m_fTime, 0.0f);
}

// FUNCTION: SWBF2 0x0053f9a0
GodrayManager::Godray* GodrayManager::ChooseNewGodray()
{
    if (m_uiNumVisibleGodrays == m_uiNumGodrays)
    {
        return 0;
    }

    Godray* pBest = 0;
    unsigned int uiBestScore = 0;
    const int iNumCameras = CameraManager::Instance()->GetNumCameras();
    Godray* pPrev = 0;
    for (Godray* pGodray = m_pFirstAllocatedGodray; pGodray; pPrev = pGodray, pGodray = pGodray->pNext)
    {
        if (pGodray->uiFrameLastVisible == 0)
        {
            continue;
        }

        if (pGodray->fLength < 0.0f)
        {
            const float fMaxLength = m_fMaxGodrayLength;
            CollisionObject* pHitObject;
            float fLength = CollisionManager::RayHit(pGodray->vStartPos, m_vLengthDir, fMaxLength,
                                                     pHitObject, 0, 0, 0, 0x80, false) *
                            fMaxLength;
            if (fLength <= 0.0f)
            {
                DeleteGodray(pGodray, pPrev);
                continue;
            }
            if (fLength > fMaxLength)
            {
                fLength = fMaxLength;
            }
            pGodray->fLength = fLength;
            pGodray->vEndPos = PblVector3(pGodray->vStartPos.x + m_vLengthDir.x * fLength,
                                          pGodray->vStartPos.y + m_vLengthDir.y * fLength,
                                          pGodray->vStartPos.z + m_vLengthDir.z * fLength);
        }

        unsigned int uiScore = m_uiFrame - pGodray->uiFrameLastVisible;
        if (uiScore < uiBestScore)
        {
            continue;
        }

        for (int iCam = 0; iCam < iNumCameras; ++iCam)
        {
            RedCamera* pCamera = CameraManager::Instance()->GetCamera(iCam);
            float fNear, fFar, fWidth, fHeight;
            pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
            const float* vFront = pCamera->GetMatrix().m[2];
            const float* vPos = pCamera->GetMatrix().m[3];
            const float fNearDist = fNear - (vFront[0] * vPos[0] + vFront[1] * vPos[1] + vFront[2] * vPos[2]);
            const PblVector3& vStart = pGodray->vStartPos;
            const PblVector3& vEnd = pGodray->vEndPos;

            float fDot;
            if (-(m_vLengthDir.y * vFront[1] + m_vLengthDir.x * vFront[0] + m_vLengthDir.z * vFront[2]) < 0.0f)
            {
                const float fDX = vEnd.x - vPos[0];
                const float fDY = vEnd.y - vPos[1];
                const float fDZ = vEnd.z - vPos[2];
                if (fDX * fDX + fDY * fDY + fDZ * fDZ > m_fMaxViewRadius * m_fMaxViewRadius)
                {
                    continue;
                }
                fDot = vStart.x * vFront[0] + vStart.y * vFront[1] + vStart.z * vFront[2];
            }
            else
            {
                const float fDX = vStart.x - vPos[0];
                const float fDY = vStart.y - vPos[1];
                const float fDZ = vStart.z - vPos[2];
                if (fDX * fDX + fDY * fDY + fDZ * fDZ > m_fMaxViewRadius * m_fMaxViewRadius)
                {
                    continue;
                }
                fDot = vEnd.x * vFront[0] + vEnd.y * vFront[1] + vEnd.z * vFront[2];
            }
            if (fNearDist > -fDot)
            {
                continue;
            }

            const PblVector3 vCenter((vStart.x + vEnd.x) * 0.5f, (vEnd.y + vStart.y) * 0.5f,
                                     (vEnd.z + vStart.z) * 0.5f);
            if (pCamera->ClassifySphere(vCenter, pGodray->fLength * 0.5f, -1.0f) == RedCamera::VISIBILITY_NONE)
            {
                continue;
            }

            if (uiScore > 1800)
            {
                uiScore = 1800;
            }
            for (unsigned int i = 0; i < m_uiNumVisibleGodrays; ++i)
            {
                const PblVector3& vOther = m_aVisibleGodrays[i].pGodray->vStartPos;
                const float fDX = vOther.x - vStart.x;
                const float fDY = vOther.y - vStart.y;
                const float fDZ = vOther.z - vStart.z;
                const float fDist2 = fDY * fDY + fDX * fDX + fDZ * fDZ;
                if (fDist2 < 400.0f)
                {
                    uiScore = (unsigned int)((1.0f - fDist2 * 0.0025f) * (float)uiScore);
                }
            }
            if (uiScore > uiBestScore)
            {
                uiBestScore = uiScore;
                pBest = pGodray;
            }
            break;
        }
    }
    return pBest;
}

// FUNCTION: SWBF2 0x0053fdd0
GodrayManager::VisibleGodray* GodrayManager::CreateVisibleGodray(Godray* pGodray)
{
    VisibleGodray* pVisible = &m_aVisibleGodrays[m_uiNumVisibleGodrays++];
    pGodray->uiFrameLastVisible = 0;
    pVisible->pGodray = pGodray;
    pVisible->fTime0 = m_fTime;

    unsigned int uiMaxRays;
    if (pGodray->fRadius > m_fRadiusForMaxRays)
    {
        uiMaxRays = m_uiMaxRaysPerGodray;
    }
    else
    {
        uiMaxRays = (unsigned int)((float)(m_uiMaxRaysPerGodray - m_uiMinRaysPerGodray) *
                                       (pGodray->fRadius / m_fRadiusForMaxRays) +
                                   (float)m_uiMinRaysPerGodray);
    }
    pVisible->uiNumRays = (unsigned int)(((float)(uiMaxRays - m_uiMinRaysPerGodray) + 0.99f) * m_Rand.GetFloat() +
                                         (float)m_uiMinRaysPerGodray);

    for (unsigned int i = 0; i < pVisible->uiNumRays; ++i)
    {
        CreateRay(pVisible, i);
        pVisible->aRays[i].fTime0 -= m_Rand.GetFloat() * pVisible->aRays[i].fLifeTime * 0.7f;
    }
    return pVisible;
}

// FUNCTION: SWBF2 0x0053ff90
void GodrayManager::CreateRay(VisibleGodray* pVisible, unsigned int uiRay)
{
    VisibleGodray::Ray& ray = pVisible->aRays[uiRay];
    const Godray* pGodray = pVisible->pGodray;

    ray.fTime0 = m_fTime;
    ray.fLifeTime = pGodray->fLifetimeMin + (pGodray->fLifetimeMax - pGodray->fLifetimeMin) * m_Rand.GetFloat();

    const float r = m_Rand.GetFloat();
    const float fDist = (1.0f - r * r) * (pGodray->fRadius - ray.fRadius);
    const float fAngle = m_Rand.GetFloat() * PblMath::fTwoPi;
    ray.fPosX = PblMath::Cos(fAngle) * fDist;
    ray.fPosZ = PblMath::Sin(fAngle) * fDist;

    ray.fRadius0 = (pGodray->fRadius * 0.5f - 0.1f) * m_Rand.GetFloat() + 0.1f;
    ray.fAlpha0 = pGodray->fAlphaMin + (pGodray->fAlphaMax - pGodray->fAlphaMin) * m_Rand.GetFloat();
    ray.fTexOffsetL0 = m_Rand.GetFloat();
    ray.fTexOffsetW0 = m_Rand.GetFloat();
    ray.fRadius = 0.0f;
    ray.fAlpha = 0.0f;
}

// FUNCTION: SWBF2 0x00540260
GodrayClass::GodrayClass(unsigned int uHash)
    : EntityClass(uHash)
    , FLEffectClass()
{
    m_fLifetimeMin = 2.0f;
    m_fLifetimeMax = 5.0f;
    m_fRadius = 1.0f;
    m_fAlphaMin = 0.5f;
    m_fAlphaMax = 1.0f;
    m_color = RedColor::WHITE;
    mNumGodRays = 1;
    mSpreadRadius = 0.0f;
}

// SYNTHETIC: SWBF2 0x005402d0
// GodrayClass::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x005409cf
// [thunk]:GodrayClass::`scalar deleting destructor' adjustor{96}

// FUNCTION: SWBF2 0x00540330
GodrayClass::GodrayClass(PblConfig& config)
    : EntityClass(config.m_uiNameHash)
    , FLEffectClass()
{
    m_fLifetimeMin = 2.0f;
    m_fLifetimeMax = 5.0f;
    m_fRadius = 1.0f;
    m_fAlphaMin = 0.5f;
    m_fAlphaMax = 1.0f;
    m_color = RedColor::WHITE;
    mNumGodRays = 1;
    mSpreadRadius = 0.0f;
    ReadConfig(config);
}

// FUNCTION: SWBF2 0x005403e0
EntityClass* GodrayClass::Derive(unsigned int uHash)
{
    return new GodrayClass(uHash);
}

// FUNCTION: SWBF2 0x00540440
FLEffectObject* GodrayClass::BuildEffect()
{
    GodrayDesc desc;
    desc.fRadius = m_fRadius;
    desc.fLifetimeMin = m_fLifetimeMin;
    desc.fLifetimeMax = m_fLifetimeMax;
    desc.fAlphaMin = m_fAlphaMin;
    desc.fAlphaMax = m_fAlphaMax;
    desc.color = m_color;
    unsigned int hGodray = GodrayManager::s_pInstance->AddGodray(desc, PblVector3(0.0f, 0.0f, 0.0f));
    return new GodrayEffectObject(hGodray);
}

// FUNCTION: SWBF2 0x00540550
Entity* GodrayClass::Build(const EntityDesc& desc)
{
    const PblVector3& vPos = *(const PblVector3*)&desc.mMatrix.m[3][0];
    for (int i = 0; i < mNumGodRays; ++i)
    {
        GodrayDesc godrayDesc;
        godrayDesc.fRadius = m_fRadius;
        godrayDesc.fLifetimeMin = m_fLifetimeMin;
        godrayDesc.fLifetimeMax = m_fLifetimeMax;
        godrayDesc.fAlphaMin = m_fAlphaMin;
        godrayDesc.fAlphaMax = m_fAlphaMax;
        godrayDesc.color = m_color;
        const float fSpread = mSpreadRadius * 2.0f;

        const float fZ = vPos.z + (PblRandom::GetGlobalFloat() - 0.5f) * fSpread;
        const float fY = vPos.y + (PblRandom::GetGlobalFloat() - 0.5f) * fSpread;
        const float fX = vPos.x + (PblRandom::GetGlobalFloat() - 0.5f) * fSpread;
        PblVector3 vGodrayPos(fX, fY, fZ);
        GodrayManager::s_pInstance->AddGodray(godrayDesc, vGodrayPos);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00540700
void GodrayClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x3D7E6258u:
    {
        unsigned int r = 255, g = 255, b = 255, a = 255;
        if (sscanf(pszValue, "%u %u %u %u", &r, &g, &b, &a) >= 3)
        {
            RedColor color;
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
            color.a = (unsigned char)a;
            m_color = color;
        }
        break;
    }
    case 0x0DBA4CB3u:
        sscanf(pszValue, "%f", &m_fRadius);
        break;
    case 0x7DB59929u:
        sscanf(pszValue, "%f", &m_fAlphaMin);
        break;
    case 0x9D399B92u:
        sscanf(pszValue, "%f", &m_fLifetimeMax);
        break;
    case 0x82A0379Cu:
        sscanf(pszValue, "%f", &mSpreadRadius);
        break;
    case 0xB1E439AFu:
        sscanf(pszValue, "%f", &m_fAlphaMax);
        break;
    case 0xBD9397ECu:
        sscanf(pszValue, "%f", &m_fLifetimeMin);
        break;
    case 0xF357466Au:
        sscanf(pszValue, "%i", &mNumGodRays);
        break;
    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

namespace
{

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
}

// FUNCTION: SWBF2 0x00540830
void GodrayClass::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3D7E6258u:
        {
            RedColor color;
            color.r = ClampColorComponent(data.GetFloatArg(0));
            color.g = ClampColorComponent(data.GetFloatArg(1));
            color.b = ClampColorComponent(data.GetFloatArg(2));
            if (data.GetNumArgs() > 3)
            {
                color.a = ClampColorComponent(data.GetFloatArg(3));
            }
            else
            {
                color.a = 0xFF;
            }
            m_color = color;
            break;
        }
        case 0x0DBA4CB3u:
            m_fRadius = data.GetFloatArg(0);
            break;
        case 0x7DB59929u:
            m_fAlphaMin = data.GetFloatArg(0);
            break;
        case 0x9D399B92u:
            m_fLifetimeMax = data.GetFloatArg(0);
            break;
        case 0xB1E439AFu:
            m_fAlphaMax = data.GetFloatArg(0);
            break;
        case 0xBD9397ECu:
            m_fLifetimeMin = data.GetFloatArg(0);
            break;
        }
    }
}
