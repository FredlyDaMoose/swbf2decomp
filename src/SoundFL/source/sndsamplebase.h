#pragma once

#include "sndallocator.h"
#include "sndsampleinfo.h"

namespace Snd
{

class SampleBase : public Allocator<SampleBase>, public SampleInfo
{
public:

    SampleBase(unsigned int uiHashID, void* pAddress, unsigned int uiLength,
               unsigned int uiLengthSamples, float fFrequency, SampleInfo::Format eFormat,
               SampleBase* pAlias)
    {
        mFormat = (char)eFormat;
        mFrequency = (unsigned int)fFrequency;
        mHashID = uiHashID;
        mLength = uiLength;
        mAddress = pAddress;
        mLengthSamples = uiLengthSamples;
        mAliasSample = pAlias;
    }

    void* GetAddress() const { return mAddress; }
    unsigned int GetLengthSamples() const { return mLengthSamples; }
    SampleBase* GetAlias() const { return mAliasSample; }

protected:

    void SetAddress(void* pAddress) { mAddress = pAddress; }

public:
    void* mAddress;
    unsigned int mLengthSamples;
    SampleBase* mAliasSample;
};
 

}
