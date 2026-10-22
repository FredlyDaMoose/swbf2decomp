#include "sndvoicebase.h"

#include <string.h>

#include "sndallocbitmap.h"

namespace Snd
{

// FUNCTION: SWBF2 0x00745320
bool AllocBitmap::Alloc(unsigned int uiNumEntries, Handle& handle)
{
    unsigned int uiLargestRun = 0;
    unsigned int uiRunStart = 0;
    unsigned int uiRun = 0;
    unsigned int uiByte = mSearchEntry >> 3;

    for (unsigned int i = 0; i < mBitmapSize; i++)
    {

        if (uiByte == mBitmapSize)
        {
            uiByte = 0;
        }

        const unsigned char ucByte = mBitmap[uiByte];
        if (ucByte == 0)
        {
            if (uiRun == 0)
            {
                uiRunStart = uiByte * 8;
            }
            uiRun += 8;
            if (uiRun > uiLargestRun)
            {
                uiLargestRun = uiRun;
            }
        }
        else if (ucByte == 0xFF)
        {
            uiRunStart = uiByte * 8;
            uiRun = 0;
        }
        else
        {
            for (unsigned int b = 0; b < 8; b++)
            {
                if (ucByte & (1 << b))
                {
                    uiRunStart = b + uiByte * 8;
                    uiRun = 0;
                }
                else
                {
                    if (uiRun == 0)
                    {
                        uiRunStart = b + uiByte * 8;
                    }
                    uiRun++;
                }
                if (uiRun == uiNumEntries)
                {
                    break;
                }
            }
            if (uiRun > uiLargestRun)
            {
                uiLargestRun = uiRun;
            }
        }

        if (uiRun >= uiNumEntries)
        {

            uiRun = uiNumEntries;
            break;
        }
        uiByte++;
    }

    if (uiRun < uiNumEntries)
    {
        return false;
    }

    Reserve(uiRunStart, uiRun, true);
    handle.mStartEntry = uiRunStart;
    handle.mNumEntries = uiRun;
    mSearchEntry = uiRunStart + uiRun;
    return true;
}

// FUNCTION: SWBF2 0x00745430
bool MemoryBitmap::Alloc(unsigned int uiSize, MemoryHandle& handle)
{
    return AllocBitmap::Alloc((uiSize - 1 + mEntrySize) / mEntrySize, handle);
}

// FUNCTION: SWBF2 0x00745450
void AllocBitmap::Free(Handle& handle)
{
    Reserve(handle.mStartEntry, handle.mNumEntries, false);

    unsigned int uiByte = 0;
    while (uiByte < handle.mStartEntry && mBitmap[uiByte] == 0xFF)
    {
        uiByte++;
    }
    mSearchEntry = uiByte;

    handle.mStartEntry = 0;
    handle.mNumEntries = 0;
}

// FUNCTION: SWBF2 0x007454a0
unsigned int MemoryBitmap::GetEntrySize() const
{
    return mEntrySize;
}

// FUNCTION: SWBF2 0x007454b0
unsigned int AllocBitmap::GetFreeEntries() const
{
    unsigned int uiFree = 0;

    for (unsigned int i = 0; i < mBitmapSize; i++)
    {
        const unsigned char ucByte = mBitmap[i];
        if (ucByte == 0)
        {
            uiFree += 8;
        }
        else if (ucByte != 0xFF)
        {
            for (unsigned int b = 0; b < 8; b++)
            {
                if (!(ucByte & (1 << b)))
                {
                    uiFree++;
                }
            }
        }
    }

    return uiFree;
}

// FUNCTION: SWBF2 0x00745520
unsigned int MemoryBitmap::GetStartAddress() const
{
    return mStartAddress;
}

// FUNCTION: SWBF2 0x00745530
void AllocBitmap::Initialize(unsigned char* pBitmap, unsigned int uiNumEntries)
{
    mBitmap = pBitmap;
    mBitmapSize = (uiNumEntries + 7) >> 3;
    mSearchEntry = 0;

    if (pBitmap && mBitmapSize)
    {
        memset(pBitmap, 0, mBitmapSize);
    }

    if (uiNumEntries & 7)
    {
        Reserve(uiNumEntries, (mBitmapSize << 3) - uiNumEntries, true);
    }
}

// FUNCTION: SWBF2 0x00745590
void MemoryBitmap::Initialize(unsigned int uiStartAddress, unsigned int uiMemorySize,
                              unsigned int uiEntrySize, unsigned char* pBitmap)
{
    mStartAddress = uiStartAddress;
    mMemorySize = uiMemorySize;
    mEntrySize = uiEntrySize;

    AllocBitmap::Initialize(pBitmap, uiMemorySize / uiEntrySize);
}

// FUNCTION: SWBF2 0x00745600
void AllocBitmap::Reserve(unsigned int uiStartEntry, unsigned int uiNumEntries, bool bReserve)
{
    const unsigned int uiFirstByte = uiStartEntry >> 3;
    const unsigned int uiLastByte = (uiStartEntry + uiNumEntries) >> 3;
    const unsigned char ucFirstBit = (unsigned char)(uiStartEntry & 7);
    const unsigned char ucLastBit = (unsigned char)((uiStartEntry + uiNumEntries) & 7);

    for (unsigned int i = uiFirstByte; i <= uiLastByte; i++)
    {
        const unsigned char ucFrom = (i == uiFirstByte) ? ucFirstBit : 0;
        const unsigned char ucTo = (i == uiLastByte) ? ucLastBit : 8;

        if (ucFrom == 0 && ucTo == 8)
        {
            mBitmap[i] = bReserve ? (unsigned char)0xFF : (unsigned char)0x00;
            continue;
        }

        for (unsigned char b = ucFrom; b < ucTo; b++)
        {
            mBitmap[i] &= (unsigned char)~(1 << b);
            mBitmap[i] |= (unsigned char)(bReserve << b);
        }
    }
}

// FUNCTION: SWBF2 0x007456c0
bool AllocBitmap::Resize(unsigned int uiNumEntries, Handle& handle)
{
    const unsigned int uiOldEntries = handle.mNumEntries;

    Free(handle);

    if (Alloc(uiNumEntries, handle))
    {
        return true;
    }

    Alloc(uiOldEntries, handle);
    return false;
}

// FUNCTION: SWBF2 0x00745740
bool MemoryBitmap::Resize(unsigned int uiSize, MemoryHandle& handle)
{
    return AllocBitmap::Resize((uiSize - 1 + mEntrySize) / mEntrySize, handle);
}

}

namespace
{
struct VoiceBaseDestructorAccess : Snd::VoiceBase
{
    static void Destroy(Snd::VoiceBase* pVoice)
    {
        ((VoiceBaseDestructorAccess*)pVoice)->Snd::VoiceBase::~VoiceBase();
    }
};
}

// FUNCTION: SWBF2 0x00745a80
void __fastcall DestroyVoiceBaseAlias(Snd::VoiceBase* pVoice)
{
    VoiceBaseDestructorAccess::Destroy(pVoice);
}
