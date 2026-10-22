#pragma once

#include "SoundFL/source/sndsamplebankbase.h"
#include "SoundFL/source/sndsamplebase.h"
#include "snddsbuffer.h"

class PblFileChunk;

namespace Snd
{

class Sample : public SampleBase
{
public:

    Sample(unsigned int uiHashID, void* pAddress, unsigned int uiLength,
           unsigned int uiLengthSamples, float fFrequency, SampleInfo::Format eFormat,
           Sample* pAlias);

    ~Sample();

    bool UploadData(void* const pData, unsigned int uiSize);

    DSBuffer* mBuffer2DAlias;
    DSBuffer* mBuffer2DLFEAlias;
    DSBuffer* mBuffer3DAlias;
    DSBuffer mBuffer2D;
    DSBuffer mBuffer2DLFE;
    DSBuffer mBuffer3D;
};
 

class SampleBank : public SampleBankBase
{
public:

    SampleBank(unsigned int uiHashID, unsigned int uiSize, SampleInfo::Format eFormat,
               Sample* pSamples, unsigned int uiNumSamples, bool bTemplate, bool bAllocate)
        : SampleBankBase(uiHashID, uiSize, eFormat, pSamples, uiNumSamples, bTemplate, bAllocate)
    {
    }

    void LoadData(PblFileChunk& chunk, char const* pcName, unsigned int const* puiOffsets);

    AllocBitmap::Handle mSampleSlotHandle;
};
 

}
