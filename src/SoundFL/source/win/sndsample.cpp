#include "sndsamplebank.h"

#include "sndengine.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0074c2e0
Sample::Sample(unsigned int uiHashID, void* pAddress, unsigned int uiLength,
               unsigned int uiLengthSamples, float fFrequency, SampleInfo::Format eFormat,
               Sample* pAlias)
    : SampleBase(uiHashID, pAddress, uiLength, uiLengthSamples, fFrequency, eFormat, pAlias),
      mBuffer2DAlias(0),
      mBuffer2DLFEAlias(0),
      mBuffer3DAlias(0)
{
    if ((uiLength || uiLengthSamples) && Engine::GetMixConfig() == Engine::mixConfig_Disabled)
    {
        SetAddress(0);
    }
}

// FUNCTION: SWBF2 0x0074c420
bool Sample::UploadData(void* const pData, unsigned int uiSize)
{
    (void)pData;
    (void)uiSize;
    Engine::GetMixConfig();
    return true;
}

}

namespace Snd {

// FUNCTION: SWBF2 0x0074c3b0
Sample::~Sample()
{
    Engine::GetMixConfig();
}
}
