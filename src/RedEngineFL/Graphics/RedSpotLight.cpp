#include <float.h>
#include <math.h>
#include <string.h>

#include "RedSpotLight.h"
#include "RedCamera.h"
#include "RedColor.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1,
                                                   const PblMatrix* pM2);

PblMatrix DirectionMatrix(const PblVector3& vDir, const PblVector3& vUp, const PblVector3& vPos);

extern PblVector3 gVectorZero;

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

static const float EPSILON = 0.0001f;
static const float SPOT_LIGHT_VISIBILITY_RADIUS_FUDGE = 10.0f;

// FUNCTION: SWBF2 0x006eed70
RedSpotLight::RedSpotLight(const RedColorValue& color, const PblVector3& vDirection,
                           const PblVector3& vPosition, float fRadius, float fFalloff)
    : RedLight(TYPE_SPOT)
    , m_fFalloff(fFalloff)
    , m_fRadius(fRadius)
    , m_vDirection(vDirection)
    , m_vPos(vPosition)
{
    memcpy(&m_mProjection, &gMatrixIdentity, sizeof(m_mProjection));
    m_bProjectionDirty = true;
    m_bBidirectional = false;

    memset(&m_d3dLight, 0, sizeof(m_d3dLight));
    m_d3dLight.Type = D3DLIGHT_SPOT;
    m_d3dLight.Range = 1.0f;
    m_d3dLight.Falloff = 1.0f;
    m_d3dLight.Attenuation2 = 1.0f;

    SetPosition(vPosition);
    SetRadius(fRadius);
    SetColor(color);
    SetFalloff(fFalloff);
    SetDirection(vDirection);
    SetInnerConeAngle(PblMath::fHalfPi * 0.5f);
    SetOuterConeAngle(PblMath::fHalfPi);

    m_mProjection.m[3][2] = 1.0f;
    m_mProjection.m[3][3] = 0.0f;
    _RecalculateProjectionMatrix();
}

// FUNCTION: SWBF2 0x006eeee0
RedSpotLight::~RedSpotLight()
{
}

// SYNTHETIC: SWBF2 0x006eeef0
// RedSpotLight::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006eef20
bool RedSpotLight::AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const
{
    return true;
}

// FUNCTION: SWBF2 0x006eef30
void RedSpotLight::DebugDraw() const
{
    const PblVector3& vDir = m_vDirection;

    PblVector3 vSide(gVectorUnitX.y * vDir.z - gVectorUnitX.z * vDir.y,
                     gVectorUnitX.z * vDir.x - gVectorUnitX.x * vDir.z,
                     gVectorUnitX.x * vDir.y - gVectorUnitX.y * vDir.x);
    PblVector3 vNormal;
    if (vSide.x * vSide.x + vSide.y * vSide.y + vSide.z * vSide.z > 0.99f)
    {
        D3DXVec3Normalize(&vNormal, &vSide);
    }
    else
    {
        PblVector3 vSideY(gVectorUnitY.y * vDir.z - gVectorUnitY.z * vDir.y,
                          gVectorUnitY.z * vDir.x - gVectorUnitY.x * vDir.z,
                          gVectorUnitY.x * vDir.y - gVectorUnitY.y * vDir.x);
        D3DXVec3Normalize(&vNormal, &vSideY);
    }
    vSide = vNormal;

    PblVector3 vUp;
    D3DXVec3Normalize(&vUp, &vDir);

    PblMatrix mBase;
    mBase.m[0][0] = vSide.x;
    mBase.m[0][1] = vSide.y;
    mBase.m[0][2] = vSide.z;
    mBase.m[0][3] = 0.0f;
    mBase.m[1][0] = vUp.x;
    mBase.m[1][1] = vUp.y;
    mBase.m[1][2] = vUp.z;
    mBase.m[1][3] = 0.0f;
    mBase.m[2][0] = vUp.y * vSide.z - vUp.z * vSide.y;
    mBase.m[2][1] = vUp.z * vSide.x - vUp.x * vSide.z;
    mBase.m[2][2] = vUp.x * vSide.y - vUp.y * vSide.x;
    mBase.m[2][3] = 0.0f;
    mBase.m[3][0] = m_vPos.x;
    mBase.m[3][1] = m_vPos.y;
    mBase.m[3][2] = m_vPos.z;
    mBase.m[3][3] = 1.0f;

    RedColor color;
    color.r = (unsigned char)(int)(m_color.r * 255.0f);
    color.g = (unsigned char)(int)(m_color.g * 255.0f);
    color.b = (unsigned char)(int)(m_color.b * 255.0f);
    color.a = (unsigned char)(int)(m_color.a * 255.0f);
    RedDebug::DrawCone(mBase, m_fRadius, 0.0f, m_fTanHalfOuterConeAngle * m_fRadius, color);
    color.r = (unsigned char)(int)(m_color.r * 255.0f);
    color.g = (unsigned char)(int)(m_color.g * 255.0f);
    color.b = (unsigned char)(int)(m_color.b * 255.0f);
    color.a = (unsigned char)(int)(m_color.a * 255.0f);
    RedDebug::DrawCone(mBase, m_fRadius, 0.0f, m_fTanHalfInnerConeAngle * m_fRadius, color);

    if (m_bBidirectional)
    {
        mBase.m[1][0] = -vUp.x;
        mBase.m[1][1] = -vUp.y;
        mBase.m[1][2] = -vUp.z;
        mBase.m[1][3] = 0.0f;

        color.r = (unsigned char)(int)(m_color.r * 255.0f);
        color.g = (unsigned char)(int)(m_color.g * 255.0f);
        color.b = (unsigned char)(int)(m_color.b * 255.0f);
        color.a = (unsigned char)(int)(m_color.a * 255.0f);
        RedDebug::DrawCone(mBase, m_fRadius, 0.0f, m_fTanHalfOuterConeAngle * m_fRadius, color);
        color.r = (unsigned char)(int)(m_color.r * 255.0f);
        color.g = (unsigned char)(int)(m_color.g * 255.0f);
        color.b = (unsigned char)(int)(m_color.b * 255.0f);
        color.a = (unsigned char)(int)(m_color.a * 255.0f);
        RedDebug::DrawCone(mBase, m_fRadius, 0.0f, m_fTanHalfInnerConeAngle * m_fRadius, color);
    }
}

// FUNCTION: SWBF2 0x006ef3d0
const PblMatrix& RedSpotLight::GetProjectionMatrix() const
{
    if (m_bProjectionDirty)
    {
        _RecalculateProjectionMatrix();
    }
    return m_mProjection;
}

// FUNCTION: SWBF2 0x006ef3f0
bool RedSpotLight::IsVisible(const RedCamera* pCamera) const
{
    return pCamera->ClassifySphere(m_vPos, m_fRadius + SPOT_LIGHT_VISIBILITY_RADIUS_FUDGE, -1.0f)
        != RedCamera::VISIBILITY_NONE;
}

// FUNCTION: SWBF2 0x006ef430
void RedSpotLight::SetColor(const RedColorValue& color)
{
    float fScale = (m_fRadius * 0.5f) * (m_fRadius * 0.5f);
    m_color = color;
    m_d3dLight.Diffuse = *(const D3DCOLORVALUE*)&color;
    m_d3dLight.Diffuse.r *= fScale;
    m_d3dLight.Diffuse.g *= fScale;
    m_d3dLight.Diffuse.b *= fScale;
    m_d3dLight.Specular = m_d3dLight.Diffuse;
}

// FUNCTION: SWBF2 0x006ef4b0
void RedSpotLight::SetDirection(const PblVector3& vDirection)
{
    float fLengthSq = vDirection.x * vDirection.x + vDirection.y * vDirection.y
                    + vDirection.z * vDirection.z;

    if (EPSILON >= fabs(fLengthSq - 1.0f))
    {
        m_vDirection = vDirection;
    }
    else
    {
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &vDirection);
        m_vDirection = vNormal;
    }

    m_d3dLight.Direction = *(const D3DVECTOR*)&m_vDirection;
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006ef560
void RedSpotLight::SetFalloff(float fFalloff)
{
    m_fFalloff = fFalloff;
    m_d3dLight.Falloff = fFalloff;
}

// FUNCTION: SWBF2 0x006ef580
void RedSpotLight::SetInnerConeAngle(float fAngle)
{
    float fHalfAngle = fAngle * 0.5f;
    m_fCosHalfInnerConeAngle = cosf(fHalfAngle);
    m_fTanHalfInnerConeAngle = tanf(fHalfAngle);
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006ef5d0
void RedSpotLight::SetOuterConeAngle(float fAngle)
{
    float fHalfAngle = fAngle * 0.5f;
    m_fCosHalfOuterConeAngle = cosf(fHalfAngle);
    m_fTanHalfOuterConeAngle = tanf(fHalfAngle);
    m_bProjectionDirty = true;
    m_d3dLight.Phi = fAngle;
    m_fShadowRadius = m_fRadius / m_fCosHalfOuterConeAngle;
}

// FUNCTION: SWBF2 0x006ef650
void RedSpotLight::SetPosition(const PblVector3& vPosition)
{
    m_vPos = vPosition;
    m_d3dLight.Position = *(const D3DVECTOR*)&vPosition;
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006ef690
void RedSpotLight::SetRadius(float fRadius)
{
    m_fRadius = fRadius;
    m_d3dLight.Range = (fRadius * 0.5f) * (fRadius * 0.5f) * fRadius;
    m_fRadiusInv = (fRadius > 0.0f) ? 1.0f / fRadius : FLT_MAX;
    m_fRadiusInvSq = (fRadius > 0.0f) ? m_fRadiusInv * m_fRadiusInv : FLT_MAX;
    m_fShadowRadius = fRadius / m_fCosHalfOuterConeAngle;
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006ef710
bool RedSpotLight::SphereIntersect(const PblVector3& vCenter, float fRadius) const
{
    PblVector3 vDir = m_vDirection;
    PblVector3 vToCenter;
    float fAxisDist = 0.0f;

    if (m_bBidirectional)
    {
        vToCenter.x = vCenter.x - m_vPos.x;
        vToCenter.y = vCenter.y - m_vPos.y;
        vToCenter.z = vCenter.z - m_vPos.z;
        fAxisDist = vDir.x * vToCenter.x + vDir.y * vToCenter.y + vDir.z * vToCenter.z;
        if (fAxisDist < 0.0f)
        {
            vDir.x *= -1.0f;
            vDir.y *= -1.0f;
            vDir.z *= -1.0f;
        }
    }

    float fSin = m_fTanHalfOuterConeAngle * m_fCosHalfOuterConeAngle;
    float fPullBack = fRadius / fSin;
    PblVector3 vApex(vCenter.x - vDir.x * fPullBack,
                     vCenter.y - vDir.y * fPullBack,
                     vCenter.z - vDir.z * fPullBack);
    PblVector3 vD(vCenter.x - vApex.x, vCenter.y - vApex.y, vCenter.z - vApex.z);
    float fDot = vD.x * vDir.x + vD.y * vDir.y + vD.z * vDir.z;

    if (fDot > 0.0f
        && fDot * fDot >= (vD.x * vD.x + vD.y * vD.y + vD.z * vD.z)
                              * (m_fCosHalfOuterConeAngle * m_fCosHalfOuterConeAngle))
    {
        if (!m_bBidirectional)
        {
            vToCenter.x = vCenter.x - m_vPos.x;
            vToCenter.y = vCenter.y - m_vPos.y;
            vToCenter.z = vCenter.z - m_vPos.z;
            fAxisDist = -(vDir.x * vToCenter.x + vDir.y * vToCenter.y + vDir.z * vToCenter.z);
        }

        float fLengthSq = vToCenter.x * vToCenter.x + vToCenter.y * vToCenter.y
                        + vToCenter.z * vToCenter.z;
        if (fAxisDist > 0.0f && fAxisDist * fAxisDist >= fSin * fSin * fLengthSq)
        {
            return fRadius * fRadius >= fLengthSq;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006ef980
void RedSpotLight::_RecalculateProjectionMatrix() const
{
    PblMatrix mProj;
    memcpy(&mProj, &gMatrixIdentity, sizeof(mProj));
    mProj.m[3][3] = 0.0f;

    const float fScale = (1.0f / m_fTanHalfOuterConeAngle) * 0.5f;
    mProj.m[0][0] = fScale;
    mProj.m[1][1] = fScale;
    mProj.m[2][0] = 0.5f;
    mProj.m[2][1] = 0.5f;
    mProj.m[2][2] = 0.0f;
    mProj.m[2][3] = 1.0f;

    const float fDot = m_vDirection.y * gVectorUnitY.y + m_vDirection.x * gVectorUnitY.x
                     + m_vDirection.z * gVectorUnitY.z;
    const PblVector3& vUp = ((float)fabs((double)fDot) > 0.99f) ? gVectorUnitX : gVectorUnitY;

    const PblMatrix mView = DirectionMatrix(m_vDirection, vUp, m_vPos);
    const PblMatrix mInvView = mView.GetInverseFast();
    PblMatrix mResult;
    D3DXMatrixMultiply(&mResult, &mInvView, &mProj);
    m_mProjection = mResult;
    m_bProjectionDirty = false;
}

// GLOBAL: SWBF2 0x009acd30
// SYNTHETIC: SWBF2 0x00408e50
// `dynamic initializer for 'RedSpotLight::BLACK''
// SYNTHETIC: SWBF2 0x00769b20
// `dynamic atexit destructor for 'RedSpotLight::BLACK''
RedSpotLight RedSpotLight::BLACK(RedColorValue::BLACK, gVectorUnitZ, gVectorZero, 1.0f, 1.0f);
