#include "RedEngineFL/Util/RedCommandConsole.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "CommandPost.h"
#include "GameObject.h"
#include "RedEngineFL/Util/RedConsole.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"

int __cdecl DumpHardware(RedConsole* pConsole, unsigned int uiArgc, char const* pcArgs);

// FUNCTION: SWBF2 0x00483220
int __cdecl DumpHardware(RedConsole* pConsole, unsigned int, char const*)
{
    D3DADAPTER_IDENTIFIER9 identifier = RedRenderer::pcGetAdaptorIdentifier();
    pConsole->PrintfMsg("Vendor Id 0x%X\n", identifier.VendorId);
    pConsole->PrintfMsg("Device Id 0x%X\n", identifier.DeviceId);
    if (RedAutoSettings::DeviceOptionsFound())
    {
        pConsole->PrintfMsg("Device match found in device.def ... override options used \n");
    }
    else
    {
        pConsole->PrintfMsg("No match found in device.def ... override options not used! \n");
    }

    if (RedAutoSettings::IsAthlonProcessor())
    {
        int effectiveSpeed = RedAutoSettings::GetEffectiveProcessorSpeed();
        pConsole->PrintfMsg("Processor Speed %d Mhz (Athlon ~%d Mhz)\n",
                            RedAutoSettings::GetProcessorSpeed(), effectiveSpeed);
    }
    else
    {
        pConsole->PrintfMsg("Processor Speed %d Mhz\n", RedAutoSettings::GetProcessorSpeed());
    }
    return 1;
}

// SYNTHETIC: SWBF2 0x00402770
// `dynamic initializer for 'ABDumpHardware''
RedCommandConsole ABDumpHardware("Hardware", DumpHardware);

// FUNCTION: SWBF2 0x00411c50
const RedModel* RedModel::Find(unsigned int uiName)
{
    return _HashTable.Find(uiName);
}

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0043ed70
const PblMatrix& CommandPost::GetMatrix() const
{
    if (!(GameObject*)mObject)
    {
        return _IdentityMatrix;
    }
    return ((GameObject*)mObject)->GetMatrix();
}
