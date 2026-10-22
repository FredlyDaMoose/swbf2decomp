#include <float.h>
#include <string.h>

#include "RedOmniLight.h"
#include "RedCamera.h"
#include "RedColor.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"

class PblMatrix;
extern PblMatrix gMatrixIdentity;

static const float POINT_LIGHT_VISIBILITY_RADIUS_FUDGE = 10.0f;

// GLOBAL: SWBF2 0x009acbb0
// SYNTHETIC: SWBF2 0x00408df0
// `dynamic initializer for 'RedOmniLight::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769b10
// `dynamic atexit destructor for 'RedOmniLight::sMemoryPool''
MemoryPool RedOmniLight::sMemoryPool("RedOmniLight", 0x120);

// GLOBAL: SWBF2 0x009acc00
// SYNTHETIC: SWBF2 0x00408d40
// `dynamic initializer for 'RedOmniLight::BLACK''
// SYNTHETIC: SWBF2 0x00769b00
// `dynamic atexit destructor for 'RedOmniLight::BLACK''
RedOmniLight RedOmniLight::BLACK(PblVector3(0.0f, 0.0f, 0.0f), 0.0f,
                                 RedColorValue(RedColor::BLACK));

// FUNCTION: SWBF2 0x006ee990
RedOmniLight::RedOmniLight(const PblVector3& pos, float fRadius, const RedColorValue& color)
    : RedLight(TYPE_POINT)
{
    memcpy(&m_mProjection, &gMatrixIdentity, sizeof(m_mProjection));
    m_bProjectionDirty = true;
    memset(&m_d3dLight, 0, sizeof(m_d3dLight));
    m_d3dLight.Type = D3DLIGHT_POINT;
    m_d3dLight.Range = 1.0f;
    m_d3dLight.Falloff = 1.0f;
    m_d3dLight.Attenuation2 = 1.0f;

    SetPosition(pos);
    SetRadius(fRadius);
    SetColor(color);
    _RecalculateProjectionMatrix();
}

// FUNCTION: SWBF2 0x006eea60
RedOmniLight::~RedOmniLight()
{
}

// FUNCTION: SWBF2 0x006eeaa0
void RedOmniLight::DebugDraw() const
{
    RedColor color;
    color.r = (unsigned char)(int)(m_color.r * 255.0f);
    color.g = (unsigned char)(int)(m_color.g * 255.0f);
    color.b = (unsigned char)(int)(m_color.b * 255.0f);
    color.a = (unsigned char)(int)(m_color.a * 255.0f);
    RedDebug::DrawSphere(m_pos, m_radius, color);
}

// FUNCTION: SWBF2 0x006eeb20
bool RedOmniLight::IsVisible(const RedCamera* pCamera) const
{
    return pCamera->ClassifySphere(m_pos, m_radius + POINT_LIGHT_VISIBILITY_RADIUS_FUDGE, -1.0f)
        != RedCamera::VISIBILITY_NONE;
}

// SYNTHETIC: SWBF2 0x006eea70
// RedOmniLight::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006eeb60
void RedOmniLight::SetColor(const RedColorValue& color)
{
    float fScale = (m_radius * 0.5f) * (m_radius * 0.5f);
    m_color = color;
    m_d3dLight.Diffuse = *(const D3DCOLORVALUE*)&color;
    m_d3dLight.Diffuse.r *= fScale;
    m_d3dLight.Diffuse.g *= fScale;
    m_d3dLight.Diffuse.b *= fScale;
    m_d3dLight.Specular = m_d3dLight.Diffuse;
}

// FUNCTION: SWBF2 0x006eebc0
void RedOmniLight::SetPosition(const PblVector3& pos)
{
    m_pos = pos;
    m_d3dLight.Position = *(const D3DVECTOR*)&pos;
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006eec00
void RedOmniLight::SetRadius(float fRadius)
{
    m_radius = fRadius;
    m_d3dLight.Range = (fRadius * 0.5f) * (fRadius * 0.5f) * fRadius;
    m_fRadiusInv = (fRadius > 0.0f) ? 1.0f / fRadius : FLT_MAX;
    m_fRadiusInvSq = (fRadius > 0.0f) ? m_fRadiusInv * m_fRadiusInv : FLT_MAX;
    m_bProjectionDirty = true;
}

// FUNCTION: SWBF2 0x006eec90
void RedOmniLight::_RecalculateProjectionMatrix() const
{
    memcpy(&m_mProjection, &gMatrixIdentity, sizeof(m_mProjection));
    if (m_radius > 0.0f)
    {
        float fInv = 1.0f / m_radius;
        m_mProjection.m[0][0] = fInv;
        m_mProjection.m[1][1] = fInv;
        m_mProjection.m[2][2] = fInv;
        m_mProjection.m[3][0] = -m_pos.x * fInv;
        m_mProjection.m[3][1] = -m_pos.y * fInv;
        m_mProjection.m[3][2] = -m_pos.z * fInv;
        m_mProjection.m[3][3] = 1.0f;
    }
    m_bProjectionDirty = false;
}
