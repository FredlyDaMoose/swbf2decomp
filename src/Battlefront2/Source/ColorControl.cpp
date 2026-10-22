#include "ColorControl.h"

// GLOBAL: SWBF2 0x01e30738
ColorControl* ColorControl::s_pInstance;

// FUNCTION: SWBF2 0x0046c040
void ColorControl::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new ColorControl();
    }
}

// FUNCTION: SWBF2 0x0046c0d0
ColorControl::ColorControl()
    : FLEffect(0x952BD04Du, 9)
{
    PlatformEnable(true);
}

// FUNCTION: SWBF2 0x0046c270
void ColorControl::Init()
{
    PlatformInit();
}

// FUNCTION: SWBF2 0x0046c0b0
void ColorControl::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// SYNTHETIC: SWBF2 0x0046c190
// ColorControl::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0046c1c0
ColorControl::~ColorControl()
{
}

// FUNCTION: SWBF2 0x0046c030
bool ColorControl::IsReflected()
{
    return false;
}

// FUNCTION: SWBF2 0x0046c280
void ColorControl::Cleanup()
{
}

// FUNCTION: SWBF2 0x0046c290
void ColorControl::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        PlatformReadConfigData(data);
    }
}
