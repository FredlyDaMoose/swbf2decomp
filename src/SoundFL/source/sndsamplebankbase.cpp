#include "sndsamplebankbase.h"

#include <stddef.h>
#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "sndenginebase.h"
#include "sndglobals.h"
#include "win/sndsamplebank.h"

namespace Snd
{

// GLOBAL: SWBF2 0x007e3850
// SYNTHETIC: SWBF2 0x0040c060
// `dynamic initializer for 'Snd::SampleBankBase::smSampleBankList''
// SYNTHETIC: SWBF2 0x0076a470
// `dynamic atexit destructor for 'Snd::SampleBankBase::smSampleBankList''
PblListSingle SampleBankBase::smSampleBankList;

// FUNCTION: SWBF2 0x00742510
template <>
void Allocator<SampleBankBase>::operator delete(void* p, void* pMemory, bool bTemp)
{
    (void)pMemory;
    (void)bTemp;
    if (!p)
    {
        return;
    }
    switch (((SampleBankBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00742540
template <>
void Allocator<SampleBase>::operator delete(void* p, void* pMemory, bool bTemp)
{
    (void)pMemory;
    (void)bTemp;
    if (!p)
    {
        return;
    }
    switch (((SampleBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}

template <>
void Allocator<SampleBankBase>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((SampleBankBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}

template <>
void Allocator<SampleBase>::operator delete(void* p, unsigned int uiSize)
{
    (void)uiSize;
    switch (((SampleBase*)p)->GetAllocType())
    {
    case allocType_heap:
        Free(p);
        break;
    case allocType_tempHeap:
        FreeTemp(p);
        break;
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00742370
SampleBankBase::SampleBankBase(unsigned int uiHashID, unsigned int uiSize,
                               SampleInfo::Format eFormat, Sample* pSamples,
                               unsigned int uiNumSamples, bool bTemplate, bool bAllocate)
    : mSamples(pSamples),
      mNumSamples(uiNumSamples),
      mHashID(uiHashID),
      mFormat(eFormat),
      mHandle(),
      mNode(),
      mTemplate(bTemplate)
{
    if (uiSize && bAllocate && !EngineBase::smSoundRAMAllocator.Alloc(uiSize, mHandle))
    {
        mNumSamples = 0;
        mSamples = 0;
    }

    smSampleBankList.AddLast(&mNode);
}

Sample const* SampleBankBase::FindSampleByHashID(unsigned int uiHashID) const
{
    for (unsigned int i = 0; i < mNumSamples; ++i)
    {
        if (mSamples[i].mHashID == uiHashID)
        {
            return &mSamples[i];
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00742570
Sample const* __cdecl SampleBankBase::AllBanksFindSampleByHashID(unsigned int uiHashID,
                                                                 bool bIncludeTemplates)
{
    for (PblListSingle::Node* pNode = smSampleBankList.mHead.mNext;
         pNode != &smSampleBankList.mHead; pNode = pNode->mNext)
    {
        SampleBankBase* const pBank =
            (SampleBankBase*)((char*)pNode - offsetof(SampleBankBase, mNode));
        if (pBank->mTemplate == 1 && !bIncludeTemplates)
        {
            continue;
        }
        Sample const* const pSample = pBank->FindSampleByHashID(uiHashID);
        if (pSample)
        {
            return pSample;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00742a00
bool SampleBankBase::LoadSamples(PblFileChunk& chunk, SampleBankBase const* pBase,
                                 unsigned int** ppOffsets, unsigned int uiNumSamplesInFile)
{
    unsigned int uiRemaining = mNumSamples;
    unsigned int uiFileOffset = 0;
    unsigned int uiRAMSize = 0;
    unsigned int uiIndex = 0;
    unsigned int* puiMissing = 0;
    unsigned int uiNumMissing = 0;

    if (ppOffsets)
    {
        if (pBase)
        {
            mNumSamples = 0;
            unsigned int* const puiOffsets = (unsigned int*)AllocTemp(
                (uiRemaining + uiNumSamplesInFile * 2) * sizeof(unsigned int));
            *ppOffsets = puiOffsets;
            if (!puiOffsets)
            {
                return false;
            }
            puiMissing = puiOffsets + uiRemaining;
            memset(puiOffsets, 0, uiRemaining * sizeof(unsigned int));
        }
        else
        {
            *ppOffsets = 0;
        }
    }

    while (chunk.HasMoreChildren() && uiRemaining)
    {
        if ((unsigned int)chunk._pFile->Read32() != 0x96E382A7u)
        {
            return false;
        }

        unsigned int uiHashID = 0;
        unsigned int uiFrequency = 0;
        bool bAlias = false;
        bool bContinue = true;
        unsigned int uiSize = 0;
        unsigned int uiLengthSamples = 0;
        Sample* pAlias = 0;
        void* pAddress = 0;
        bool bAliasMissing = false;

        while (chunk.HasMoreChildren() && bContinue)
        {
            switch ((unsigned int)chunk._pFile->Read32())
            {
            case 0x2FB31C01u:
                uiFrequency = chunk._pFile->Read32();
                break;
            case 0x1D48FEEFu:
                uiLengthSamples = chunk._pFile->Read32();
                break;
            case 0x23A0D95Cu:
                uiSize = chunk._pFile->Read32();
                break;
            case 0x2E789FB4u:
                bContinue = false;
                break;
            case 0x37386AE0u:
                uiHashID = chunk._pFile->Read32();
                break;
            case 0x7D268157u:
            {
                const unsigned int uiAliasHashID = chunk._pFile->Read32();
                for (unsigned int i = 0; i < uiIndex; ++i)
                {
                    Sample* const pSample = &mSamples[i];
                    if (pSample->mHashID == uiAliasHashID)
                    {
                        uiSize = pSample->mLength;
                        uiLengthSamples = pSample->mLengthSamples;
                        pAddress = pSample->mAddress;
                        uiFrequency = pSample->mFrequency;
                        pAlias = pSample;
                        break;
                    }
                }
                if (puiMissing && !pAlias)
                {
                    for (unsigned int i = 0; i < uiNumMissing; ++i)
                    {
                        if (puiMissing[i * 2] == uiAliasHashID)
                        {
                            bAliasMissing = true;
                            break;
                        }
                    }
                }
                bAlias = true;
                break;
            }
            }
        }

        if (mTemplate == 1)
        {
            uiSize = 0;
            uiLengthSamples = 0;
        }
        if (!bAlias)
        {
            uiFileOffset += uiSize;
        }

        if (pBase && !pBase->FindSampleByHashID(uiHashID))
        {
            if (puiMissing)
            {
                puiMissing[uiNumMissing * 2] = uiHashID;
                puiMissing[uiNumMissing * 2 + 1] = uiFileOffset;
                ++uiNumMissing;
            }
            continue;
        }

        uiRAMSize += (bAlias && !bAliasMissing) ? 0 : uiSize;

        if (ppOffsets && *ppOffsets)
        {
            (*ppOffsets)[uiIndex] = uiFileOffset - uiSize;
        }

        if (pBase)
        {
            const bool bOk = uiIndex
                                 ? EngineBase::smSoundRAMAllocator.Resize(uiRAMSize, mHandle)
                                 : EngineBase::smSoundRAMAllocator.Alloc(uiRAMSize, mHandle);
            if (!bOk)
            {

                while (uiIndex)
                {
                    --uiIndex;
                    delete &mSamples[uiIndex];
                }
                mNumSamples = 0;
                return false;
            }
            ++mNumSamples;
        }

        if (mTemplate == 1)
        {
            pAddress = 0;
        }
        else if (!bAlias || bAliasMissing)
        {
            pAddress = (void*)(GetBaseAddress() - uiSize + uiRAMSize);
        }

        new (&mSamples[uiIndex++], false)
            Sample(uiHashID, pAddress, uiSize, uiLengthSamples, (float)uiFrequency,
                   mFormat, pAlias);

        --uiRemaining;
    }

    return true;
}

// FUNCTION: SWBF2 0x00742670
SampleBankBase* __cdecl SampleBankBase::LoadBank(PblFileChunk& chunk, char const* pcName,
                                                 bool bFirstOnly, unsigned int uiBaseHashID)
{
    unsigned int* puiOffsets = 0;
    bool bFailed = false;
    SampleBank* pBank = 0;

    while (chunk.HasMoreChildren() && !bFailed)
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x5E262256u:
        case 0x9A644D15u:
            pBank = static_cast<SampleBank*>(LoadBank(child, pcName, bFirstOnly, uiBaseHashID));
            if (bFirstOnly)
            {
                return pBank;
            }
            break;

        case 0x0FB40705u:
        {
            unsigned int uiHashID = 0;
            unsigned int uiSize = 0;
            unsigned int uiNumSamples = 0;
            SampleInfo::Format eFormat = SampleInfo::format_Unknown;
            bool bContinue = true;
            bool bTemplate = false;

            while (child.HasMoreChildren() && bContinue)
            {
                switch ((unsigned int)child._pFile->Read32())
                {
                case 0x8D39BDE6u:
                    uiHashID = child._pFile->Read32();
                    break;
                case 0x694AAA0Bu:
                    bTemplate = child._pFile->Read32() == 1;
                    if (bTemplate)
                    {
                        uiSize = 0;
                    }
                    break;
                case 0x23A0D95Cu:
                    uiSize = child._pFile->Read32();
                    break;
                case 0x98B889CEu:
                    uiNumSamples = child._pFile->Read32();
                    break;
                case 0xB99D8552u:
                    eFormat = (SampleInfo::Format)child._pFile->Read32();
                    break;
                case 0x8EBF7143u:
                {
                    SampleBankBase const* const pBase =
                        uiBaseHashID ? FindByHashID(uiBaseHashID) : 0;
                    const unsigned int uiNumSamplesInFile = uiNumSamples;
                    if (pBase)
                    {
                        uiSize = 0;
                        if (pBase->mNumSamples < uiNumSamples)
                        {
                            uiNumSamples = pBase->mNumSamples;
                        }
                    }

                    void* const pMemory = Snd::Alloc(uiNumSamples * sizeof(Sample) + sizeof(SampleBank));
                    pBank = new (pMemory, false)
                        SampleBank(uiHashID, uiSize, eFormat,
                                   (Sample*)((char*)pMemory + sizeof(SampleBank)), uiNumSamples,
                                   bTemplate, pBase == 0);

                    if (!pBank->GetSize() && pBank->mTemplate != 1 && !pBase)
                    {
                        delete pBank;
                        Snd::Free(pBank);
                        return 0;
                    }

                    if (!pBank->LoadSamples(child, pBase, &puiOffsets, uiNumSamplesInFile))
                    {
                        delete pBank;
                        pBank = 0;
                        bFailed = true;
                    }
                    bContinue = false;
                    break;
                }
                }
            }
            break;
        }

        case 0xD872E2A5u:
            pBank->LoadData(child, pcName, puiOffsets);
            if (puiOffsets)
            {
                FreeTemp(puiOffsets);
                puiOffsets = 0;
            }
            break;
        }
    }

    if (puiOffsets)
    {
        FreeTemp(puiOffsets);
    }
    return pBank;
}

// FUNCTION: SWBF2 0x00742640
SampleBankBase* __cdecl SampleBankBase::FindByHashID(unsigned int uiHashID)
{
    for (PblListSingle::Itor it(smSampleBankList); !it.IsDone(); ++it)
    {
        SampleBankBase* pBank =
            (SampleBankBase*)((char*)it.GetNode() - offsetof(SampleBankBase, mNode));
        if (pBank->mHashID == uiHashID)
        {
            return pBank;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00742440
SampleBankBase::~SampleBankBase()
{
    if (mHandle.mNumEntries)
    {
        EngineBase::smSoundRAMAllocator.Free(mHandle);
    }

    for (unsigned int i = 0; i < mNumSamples; ++i)
    {
        delete &mSamples[i];
    }

    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

}

namespace Snd {

// FUNCTION: SWBF2 0x007425e0
void __cdecl SampleBankBase::DestroyAllBanks()
{
    while (!smSampleBankList.IsEmpty())
    {
        SampleBankBase* const pBank =
            (SampleBankBase*)((char*)smSampleBankList.mHead.mNext - offsetof(SampleBankBase, mNode));
        const AllocType eType = pBank->GetAllocType();
        delete pBank;
        if (eType == allocType_custom)
        {
            Snd::Free(pBank);
        }
    }
}
}
