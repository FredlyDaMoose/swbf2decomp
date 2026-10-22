#pragma once

namespace Snd
{

class SampleInfo
{
public:
    enum Format
    {
        format_Unknown = 0,
        format_VAG = 1,
        format_PCM16 = 2,
        format_PCM8 = 3,
        format_XADPCM = 4,
        format_IMAADPCM = 5
    };

    static unsigned int __cdecl GetBitDepth(Format eFormat);
    static unsigned int __cdecl GetFrameSize(Format eFormat);
    static unsigned int __cdecl GetSamplesPerFrame(Format eFormat);

    static const unsigned int smSamplesPerFrame[5];
    static const unsigned int smFrameSize[5];
    static const unsigned char smBitDepth[5];

    char mFormat;
    unsigned int mFrequency;
    unsigned int mLength;
    unsigned int mHashID;
};
 

}
