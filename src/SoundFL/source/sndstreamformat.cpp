#include "sndstreamformat.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0073f6f0
StreamFormat::StreamFormat(SampleInfo::Format eFormat, unsigned int uiFrequency,
                           unsigned short uNumChannels)
{
    mFrequency = uiFrequency;
    mNumChannels = uNumChannels;
    mFormat = (unsigned short)eFormat;
}

// FUNCTION: SWBF2 0x0073f710
StreamFormat::StreamFormat()
    : mFrequency(0),
      mNumChannels(0),
      mFormat(0)
{
}

// FUNCTION: SWBF2 0x0073f720
SampleInfo::Format StreamFormat::GetFormat() const
{
    return (SampleInfo::Format)mFormat;
}

// FUNCTION: SWBF2 0x0073f730
unsigned int StreamFormat::GetFrequency() const
{
    return mFrequency;
}

// FUNCTION: SWBF2 0x0073f740
unsigned short StreamFormat::GetNumChannels() const
{
    return mNumChannels;
}

// FUNCTION: SWBF2 0x0073f750
void StreamFormat::SetFrequency(unsigned int uiFrequency)
{
    mFrequency = uiFrequency;
}

}
