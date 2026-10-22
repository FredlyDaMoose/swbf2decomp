#pragma once

#include "PebbleFL/Common/PblListSingle.h"
#include "sndallocator.h"
#include "sndallocbitmap.h"
#include "sndenginebase.h"
#include "sndsampleinfo.h"

class PblFileChunk;

namespace Snd
{

class Sample;

class SampleBankBase : public Allocator<SampleBankBase>
{
public:

    static SampleBankBase* __cdecl LoadBank(PblFileChunk& chunk, char const* pcName,
                                            bool bFirstOnly, unsigned int uiBaseHashID);

    static SampleBankBase* __cdecl FindByHashID(unsigned int uiHashID);

    static void __cdecl DestroyAllBanks();

    static Sample const* __cdecl AllBanksFindSampleByHashID(unsigned int uiHashID,
                                                             bool bIncludeTemplates);

    Sample const* FindSampleByHashID(unsigned int uiHashID) const;

    static PblListSingle smSampleBankList;

protected:

    SampleBankBase(unsigned int uiHashID, unsigned int uiSize, SampleInfo::Format eFormat,
                   Sample* pSamples, unsigned int uiNumSamples, bool bTemplate,
                   bool bAllocate);

    ~SampleBankBase();

    bool LoadSamples(PblFileChunk& chunk, SampleBankBase const* pBase, unsigned int** ppOffsets,
                     unsigned int uiNumSamplesInFile);

    unsigned int GetSize() const
    {
        return mHandle.GetSize(EngineBase::smSoundRAMAllocator);
    }
    unsigned int GetBaseAddress() const
    {
        return mHandle.GetAddress(EngineBase::smSoundRAMAllocator);
    }

public:
    Sample* mSamples;
    unsigned int mNumSamples;
    unsigned int mHashID;
    SampleInfo::Format mFormat;
    MemoryBitmap::MemoryHandle mHandle;
    PblListSingle::Node mNode;
    unsigned char mTemplate;
};
 

}
