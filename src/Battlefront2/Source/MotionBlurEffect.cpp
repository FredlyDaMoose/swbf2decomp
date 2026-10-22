#include "MotionBlurEffect.h"
#include "PebbleFL/Common/PblConfig.h"

// GLOBAL: SWBF2 0x01e579d8
MotionBlurEffect* MotionBlurEffect::s_pInstance;

// GLOBAL: SWBF2 0x01e579d4
float MotionBlurEffect::m_curBlurAmount[1];

// GLOBAL: SWBF2 0x01e579dc
int MotionBlurEffect::m_state[1];

// FUNCTION: SWBF2 0x005a4ea0
void MotionBlurEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new MotionBlurEffect();
    }
}

// FUNCTION: SWBF2 0x005a4f30
MotionBlurEffect::MotionBlurEffect()
    : FLEffect(0xF744F3B2u, 9)
{
    Enable(true);
    SetRenderPriority(0x271B);
}

// FUNCTION: SWBF2 0x005a5040
void MotionBlurEffect::Init()
{
    m_state[0] = 0;
    m_curBlurAmount[0] = 0.0f;
    PlatformInit();
}

// FUNCTION: SWBF2 0x005a4e90
bool MotionBlurEffect::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x005a4f10
void MotionBlurEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x005a4fc0
// MotionBlurEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x005a4ff0
MotionBlurEffect::~MotionBlurEffect()
{
}

// FUNCTION: SWBF2 0x005a5060
void MotionBlurEffect::Cleanup()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x005a5070
void MotionBlurEffect::ReadConfig(PblConfig& config)
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

// FUNCTION: SWBF2 0x005a50f0
void MotionBlurEffect::Start(int, float fBlurAmount, int iCamera)
{
    m_state[iCamera] = 1;
    m_curBlurAmount[iCamera] = fBlurAmount;
}

// FUNCTION: SWBF2 0x005a5110
void MotionBlurEffect::Stop(int, int iCamera)
{
    m_state[iCamera] = 0;
    m_curBlurAmount[iCamera] = 0.0f;
}

// FUNCTION: SWBF2 0x005a5130
void MotionBlurEffect::Update(float)
{
}

// FUNCTION: SWBF2 0x005a5140
void MotionBlurEffect::Render(int value)
{
    if (!((unsigned int)value & 0x200000))
    {
        PlatformRender((unsigned int)value);
    }
}
