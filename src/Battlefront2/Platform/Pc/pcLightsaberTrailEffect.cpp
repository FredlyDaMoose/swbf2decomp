#include "Battlefront2/Source/LightsaberTrailEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

// FUNCTION: SWBF2 0x004213e0
void LightsaberTrailEffect::_PlatformInit()
{
    m_pShader = pcRedShader::Create(
        PblHash("LightBeam"),
        RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_COMPRESSED ? 0x1082 : 0x82,
        0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00421440
void LightsaberTrailEffect::_PlatformCleanup()
{
    m_pShader->Release();
    m_pShader = 0;
}
