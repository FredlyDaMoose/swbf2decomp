#include "ScreenFilterEffect.h"
#include "GameState.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

// GLOBAL: SWBF2 0x01eaf024
ScreenFilterEffect* ScreenFilterEffect::s_pInstance;

// FUNCTION: SWBF2 0x00633e60
void ScreenFilterEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ScreenFilterEffect();
    }
}

// FUNCTION: SWBF2 0x00633ef0
ScreenFilterEffect::ScreenFilterEffect()
    : FLEffect(0xE32EAA23u, 9)
{
    Enable(true);
}

// FUNCTION: SWBF2 0x00634000
void ScreenFilterEffect::Init()
{
    m_data[0].uiState = 0;
    m_filterTimeStamp.Mark(&GameState::_TimeBase);
    PlatformInit();
}

// FUNCTION: SWBF2 0x00633ed0
void ScreenFilterEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x00633f70
// ScreenFilterEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00633fa0
ScreenFilterEffect::~ScreenFilterEffect()
{
}

// FUNCTION: SWBF2 0x00634240
void ScreenFilterEffect::Render(int value)
{
    if (!((unsigned int)value & 0x200000) && m_data[FLRenderer::GetCurrentViewport()].uiState)
    {
        PlatformRender((unsigned int)value);
    }
}

// FUNCTION: SWBF2 0x00633e50
bool ScreenFilterEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00634030
void ScreenFilterEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00634040
void ScreenFilterEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0xAF8BB8CEu)
        {
            Enable(data.GetUint32Arg(0) != 0);
        }
    }
}

// FUNCTION: SWBF2 0x006340c0
void ScreenFilterEffect::TimedFilter(int iViewport, float fDuration, float fFadeInTime,
                                     float fFadeOutTime, RedColor color, unsigned int uiTextureHash)
{
    FilterData& data = m_data[iViewport];
    if (fFadeInTime < 0.0f)
    {
        data.uiState = 1;
        data.fIntensity = 1.0f;
    }
    else
    {
        data.uiState = 2;
        data.fIntensity = 0.0f;
    }
    float fNow = m_filterTimeStamp.GetElapsed();
    data.fStartFadeInTime = fNow;
    data.uiTextureHash = uiTextureHash;
    data.fStopFadeOutTime = fNow + fDuration;
    data.fStopFadeInTime = fNow + fFadeInTime;
    data.filterColor = color;
    data.fStartFadeOutTime = data.fStopFadeOutTime - fFadeOutTime;
}

// FUNCTION: SWBF2 0x00634160
void ScreenFilterEffect::Update(float)
{
    float fNow = m_filterTimeStamp.GetElapsed();
    FilterData& data = m_data[0];
    if (!data.uiState)
    {
        return;
    }
    if (data.uiState == 1 && data.fStartFadeOutTime > 0.0f && fNow >= data.fStartFadeOutTime)
    {
        data.fIntensity = 1.0f;
        data.uiState = 4;
    }
    else if (data.uiState == 2)
    {
        if (fNow < data.fStopFadeInTime)
        {
            data.fIntensity = (fNow - data.fStartFadeInTime) / (data.fStopFadeInTime - data.fStartFadeInTime);
        }
        else
        {
            data.fIntensity = 1.0f;
            data.uiState = 1;
        }
    }
    else if (data.uiState == 4)
    {
        if (fNow < data.fStopFadeOutTime)
        {
            data.fIntensity = 1.0f - (fNow - data.fStartFadeOutTime) / (data.fStopFadeOutTime - data.fStartFadeOutTime);
        }
        else
        {
            data.fIntensity = 0.0f;
            data.uiState = 0;
        }
    }
}
