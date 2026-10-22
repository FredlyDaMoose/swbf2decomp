#include "LightFlash.h"

#include "PebbleFL/Common/PblVector.h"

// GLOBAL: SWBF2 0x01ec6370
// SYNTHETIC: SWBF2 0x004040e0
// `dynamic initializer for 'LightFlash::sMemoryPool''
// SYNTHETIC: SWBF2 0x00768c40
// `dynamic atexit destructor for 'LightFlash::sMemoryPool''
MemoryPool LightFlash::sMemoryPool("LightFlash", 0x150);

static inline RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * (1.0f / 255.0f);
    v.g = c.g * (1.0f / 255.0f);
    v.b = c.b * (1.0f / 255.0f);
    v.a = c.a * (1.0f / 255.0f);
    return v;
}

// FUNCTION: SWBF2 0x00570a40
LightFlash::LightFlash(const PblSphere& sphere, const RedColor& color, float fLifeTime)
    : RedOmniLight(sphere.GetCenter(), sphere.GetRadius(), ToColorValue(color))
{
    m_color0 = color;
    m_fLifeTime = fLifeTime;
    m_fTimer = 0.0f;
    Activate();
    ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x00570b60
// LightFlash::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00570b90
LightFlash::~LightFlash()
{
    Deactivate();
    DeactivateThread();
}

// SYNTHETIC: SWBF2 0x00570df7
// [thunk]:LightFlash::`scalar deleting destructor' adjustor{32}

// FUNCTION: SWBF2 0x00570c00
bool LightFlash::Update(float fDeltaTime)
{
    m_fTimer += fDeltaTime;
    if (m_fTimer < m_fLifeTime)
    {
        float t = m_fTimer / m_fLifeTime;
        RedColor color;
        color.b = (unsigned char)(int)(RedColor::BLACK.b * t + m_color0.b * (1.0f - t) + 0.5f);
        color.g = (unsigned char)(int)(m_color0.g * (1.0f - t) + RedColor::BLACK.g * t + 0.5f);
        color.r = (unsigned char)(int)(m_color0.r * (1.0f - t) + RedColor::BLACK.r * t + 0.5f);
        color.a = (unsigned char)(int)(m_color0.a * (1.0f - t) + RedColor::BLACK.a * t + 0.5f);
        SetColor(ToColorValue(color));
        return true;
    }

    SetColor(ToColorValue(RedColor::BLACK));
    return false;
}
