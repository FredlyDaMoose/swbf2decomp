#pragma once

#include "sndsampleinfo.h"

namespace Snd
{

class StreamFormat
{
public:

    StreamFormat();
    StreamFormat(SampleInfo::Format eFormat, unsigned int uiFrequency,
                 unsigned short uNumChannels);
    SampleInfo::Format GetFormat() const;
    unsigned int GetFrequency() const;
    unsigned short GetNumChannels() const;
    void SetFrequency(unsigned int uiFrequency);

    unsigned int mFrequency;
    unsigned short mNumChannels;
    unsigned short mFormat;
};

}
