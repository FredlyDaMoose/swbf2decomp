#include "GameVoiceChat.h"

// FUNCTION: SWBF2 0x0053c5c0
unsigned int GameVoiceChat::CalculatePlayCaptureBufferSizeSamples(float fMilliseconds)
{
    const unsigned int uiSamples = (unsigned int)(fMilliseconds / 1000.0f * 8000.0f);

    return ((uiSamples + 159u) / 160u) * 160u;
}

// FUNCTION: SWBF2 0x0053c5f0
unsigned int GameVoiceChat::CalculateEncodedPacketSize(unsigned int uiSamples)
{
    return (uiSamples / 160u) * 33u;
}
