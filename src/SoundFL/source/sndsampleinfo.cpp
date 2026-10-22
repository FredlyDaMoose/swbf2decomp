#include "sndsampleinfo.h"

namespace Snd
{

const unsigned int SampleInfo::smSamplesPerFrame[5] = { 28, 1, 1, 64, 64 };

const unsigned int SampleInfo::smFrameSize[5] = { 16, 2, 1, 36, 36 };

const unsigned char SampleInfo::smBitDepth[5] = { 4, 16, 8, 4, 4 };

// FUNCTION: SWBF2 0x00748f00
unsigned int __cdecl SampleInfo::GetBitDepth(Format eFormat)
{
    return smBitDepth[eFormat - 1];
}

// FUNCTION: SWBF2 0x00748f10
unsigned int __cdecl SampleInfo::GetFrameSize(Format eFormat)
{
    return smFrameSize[eFormat - 1];
}

// FUNCTION: SWBF2 0x00748f20
unsigned int __cdecl SampleInfo::GetSamplesPerFrame(Format eFormat)
{
    return smSamplesPerFrame[eFormat - 1];
}

}
