#pragma once

namespace Snd
{

class AllocBitmap
{
public:

    class Handle
    {
    public:

        Handle() : mStartEntry(0), mNumEntries(0) {}

        unsigned int mStartEntry;
        unsigned int mNumEntries;
    };

    bool Alloc(unsigned int uiNumEntries, Handle& handle);
    void Free(Handle& handle);
    bool Resize(unsigned int uiNumEntries, Handle& handle);

    unsigned int GetFreeEntries() const;

    void Initialize(unsigned char* pBitmap, unsigned int uiNumEntries);
    void Reserve(unsigned int uiStartEntry, unsigned int uiNumEntries, bool bReserve);

    unsigned char* mBitmap;
    unsigned int mBitmapSize;

    unsigned int mSearchEntry;
};

class MemoryBitmap : public AllocBitmap
{
public:

    class MemoryHandle : public AllocBitmap::Handle
    {
    public:

        unsigned int GetAddress(MemoryBitmap& bitmap) const
        {
            return mStartEntry * bitmap.GetEntrySize() + bitmap.GetStartAddress();
        }

        unsigned int GetSize(MemoryBitmap& bitmap) const
        {
            return mNumEntries ? mNumEntries * bitmap.GetEntrySize() : 0;
        }
    };

    bool Alloc(unsigned int uiSize, MemoryHandle& handle);
    bool Resize(unsigned int uiSize, MemoryHandle& handle);

    unsigned int GetEntrySize() const;
    unsigned int GetStartAddress() const;

    void Initialize(unsigned int uiStartAddress, unsigned int uiMemorySize,
                    unsigned int uiEntrySize, unsigned char* pBitmap);

    unsigned int mStartAddress;
    unsigned int mMemorySize;
    unsigned int mEntrySize;
};

 
 
 
 

}
