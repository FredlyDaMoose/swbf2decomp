#include "HeatShimmerEffect.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x007e68f8
unsigned int HeatShimmerEffect::s_tessellationX = 1;

// GLOBAL: SWBF2 0x007e68fc
unsigned int HeatShimmerEffect::s_tessellationY = 1;

// GLOBAL: SWBF2 0x01e56bd8
float HeatShimmerEffect::s_height;
// GLOBAL: SWBF2 0x01e56bdc
float HeatShimmerEffect::s_scrollSpeed;
// GLOBAL: SWBF2 0x01e56be0
float HeatShimmerEffect::s_posY;

// GLOBAL: SWBF2 0x01e56be4
float HeatShimmerEffect::s_distortionScale;

// GLOBAL: SWBF2 0x01ec6080
PblTimeBase::Stamp HeatShimmerEffect::s_shimmerTimeStamp;

// GLOBAL: SWBF2 0x01e56bd4
HeatShimmerEffect* HeatShimmerEffect::s_pInstance;

// FUNCTION: SWBF2 0x00542570
void HeatShimmerEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new HeatShimmerEffect();
    }
}

// FUNCTION: SWBF2 0x00542600
HeatShimmerEffect::HeatShimmerEffect()
    : FLEffect(0xEAAAAE76u, 5)
{
    Enable(false);
    SetRenderPriority(0x2710);
}

// FUNCTION: SWBF2 0x00542710
void HeatShimmerEffect::Init()
{
    s_height = 0.0f;
    s_posY = 10.0f;
    s_tessellationX = 5;
    s_tessellationY = 2;
    s_shimmerTimeStamp.Mark(RedRenderer::GetTimeBase());
    PlatformPreReadInit();
}

// FUNCTION: SWBF2 0x005425e0
void HeatShimmerEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x00542690
// HeatShimmerEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005426c0
HeatShimmerEffect::~HeatShimmerEffect()
{
}

// FUNCTION: SWBF2 0x00542890
void HeatShimmerEffect::Render(int value)
{
    if (!((unsigned int)value & 0x200000))
    {
        PlatformRender((unsigned int)value);
    }
}

// FUNCTION: SWBF2 0x00542560
bool HeatShimmerEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x00542760
void HeatShimmerEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00542880
void HeatShimmerEffect::Update(float)
{
}

// FUNCTION: SWBF2 0x00542770
void HeatShimmerEffect::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3654D7E0u:
            s_distortionScale = *(float*)&data.m_auiArgs[0];
            break;
        case 0x44FA1876u:
            s_posY = *(float*)&data.m_auiArgs[0];
            break;
        case 0x99000D04u:
            s_tessellationX = data.GetUint32Arg(0);
            s_tessellationY = data.m_uiNumArgs > 1 ? data.GetUint32Arg(1) : 2;
            break;
        case 0xAF8BB8CEu:
            break;
        case 0xB099CA7Cu:
            s_height = *(float*)&data.m_auiArgs[0];
            break;
        case 0xDFC96107u:
            s_scrollSpeed = *(float*)&data.m_auiArgs[0];
            break;
        default:
            PlatformReadConfigData(data);
            break;
        }
    }
    PlatformPostReadInit();
}
