#include "sndfileplatformbase.h"
#include "sndglobals.h"

#include <string.h>

namespace Snd
{

// FUNCTION: SWBF2 0x0072fe40
FilePlatformBase::FilePlatformBase()
    : mAccess(0),
      mFlags(0),
      mBufferAlloc(0),
      mBuffer(),
      mSectorSize(0),
      mPos(0)
{
}

// FUNCTION: SWBF2 0x0072fe80
FilePlatformBase::~FilePlatformBase()
{
}

// FUNCTION: SWBF2 0x0072fe90
FilePlatformBase& FilePlatformBase::operator=(FilePlatformBase& rOther)
{
    memcpy(this, &rOther, sizeof(FilePlatformBase));
    rOther.mBufferAlloc = 0;
    rOther.mBuffer = Buffer();
    rOther.Close();
    return *this;
}

// FUNCTION: SWBF2 0x0072ff70
unsigned int FilePlatformBase::BufferedReadWrite(void* pBuffer, unsigned int uiSize, bool bRead)
{
    unsigned int uiDone = 0;

    while (uiDone < uiSize)
    {
        unsigned int uiLeft = uiSize - uiDone;
        unsigned int uiSectors = uiLeft & ~(mSectorSize - 1);

        if ((uiSectors && !((unsigned int)pBuffer & (mAlignment - 1)) && !(GetPos() & (mSectorSize - 1)))
            || (mAccess & 8))
        {

            unsigned int uiDirect = (mAccess & 8) ? uiLeft : uiSectors;
            if (bRead)
            {
                uiDirect = SeekRead(pBuffer, uiDirect);
            }
            else
            {
                uiDirect = SeekWrite(pBuffer, uiDirect);
            }

            Seek(RealGetPos() + uiDirect, true);
            uiDone += uiDirect;
            pBuffer = (char*)pBuffer + uiDirect;
            if (uiDirect != uiSectors)
            {
                return uiDone;
            }
        }
        else if (bRead)
        {
            if (mFlags & 4)
            {
                unsigned int uiPos = GetPos();
                Flush();
                Seek(uiPos, false);
            }

            if (mBuffer.IsEmpty())
            {
                mBuffer.UpdateWritePos(SeekRead(mBuffer.GetWritePos(), mBuffer.GetSize()));
                if (mBuffer.GetUsed() < uiLeft && uiLeft < mBuffer.GetSize())
                {
                    return uiDone;
                }
            }

            unsigned int uiRead = mBuffer.Read(pBuffer, uiLeft);
            if (!uiRead)
            {
                return 0;
            }

            if (mBuffer.IsEmpty())
            {
                Seek(mPos + mBuffer.GetSize(), true);
            }

            uiDone += uiRead;
            pBuffer = (char*)pBuffer + uiRead;
        }
        else
        {
            if (mBuffer.IsFull())
            {
                Flush();
            }

            if (mBuffer.IsEmpty() && (mPos < GetSize() || uiLeft < mBuffer.GetSize()))
            {
                CommitSeek(false);
                RealRead(mBuffer.GetWritePos(), mBuffer.GetSize());
                mBuffer.UpdateWritePos(mBuffer.GetReadOffset());
            }

            unsigned int uiWritten = mBuffer.Write(pBuffer, uiLeft);
            mFlags |= 4;

            uiDone += uiWritten;
            pBuffer = (char*)pBuffer + uiWritten;
        }
    }

    return uiDone;
}

// FUNCTION: SWBF2 0x00730230
void FilePlatformBase::Close()
{
    Flush();
    mFlags &= ~1;
    mAccess = 0;
    if (mBufferAlloc)
    {
        Free(mBufferAlloc);
        mBufferAlloc = 0;
        mBuffer = Buffer();
    }
    mPos = 0;
    mSectorSize = 0;
}

// FUNCTION: SWBF2 0x00730290
void FilePlatformBase::Flush()
{
    if (mFlags & 4)
    {

        if (mPos == GetSize())
        {
            unsigned int uiPad = mBuffer.GetWriteLeft();
            if (uiPad)
            {
                memset(mBuffer.GetWritePos(), 0, uiPad);
                mBuffer.UpdateWritePos(uiPad);
            }
        }

        CommitSeek(true);
        RealWrite(mBuffer.GetBuffer(), mBuffer.GetSize());

        mFlags &= ~4;
        Seek(mPos + mBuffer.GetUsed(), false);
    }

    unsigned int uiPos = GetPos();
    mBuffer.Flush();
    Seek(uiPos, false);
}

// FUNCTION: SWBF2 0x00730360
unsigned int FilePlatformBase::GetPos() const
{
    if (mFlags & 4)
    {
        return mPos + mBuffer.GetUsed();
    }
    return mPos + mBuffer.GetReadOffset();
}

// FUNCTION: SWBF2 0x00730380
bool FilePlatformBase::IsDataBuffered(unsigned int uiPos, unsigned int uiSize)
{
    return !mBuffer.IsEmpty() && uiPos >= mPos && uiPos + uiSize <= mPos + mBuffer.GetUsed();
}

// FUNCTION: SWBF2 0x007303c0
bool FilePlatformBase::IsOpen() const
{
    return (mFlags & 1) != 0;
}

// FUNCTION: SWBF2 0x007303d0
bool FilePlatformBase::Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
                            unsigned int uiSectorSize, unsigned int uiAlignment)
{
    if (uiBufferSize)
    {
        mBufferAlloc = Alloc(uiBufferSize + uiAlignment);
        if (!mBufferAlloc)
        {
            return false;
        }
        mBuffer.Init((void*)(((unsigned int)mBufferAlloc + uiAlignment - 1) / uiAlignment * uiAlignment),
                     uiBufferSize);
    }

    mPos = 0;

    if (!uiBufferSize && (uiAccessFlags & 8))
    {
        mAlignment = 1;
        mSectorSize = 1;
    }
    else
    {
        mAlignment = uiAlignment;
        mSectorSize = uiSectorSize;
    }

    if (mBuffer.GetSize() && mSectorSize < mBuffer.GetSize())
    {
        mSectorSize = mBuffer.GetSize();
    }

    mAccess |= uiAccessFlags;
    mFlags |= 1;
    return true;
}

// FUNCTION: SWBF2 0x00730470
unsigned int FilePlatformBase::Read(void* pBuffer, unsigned int uiSize)
{
    if (!pBuffer)
    {
        return 0;
    }
    return BufferedReadWrite(pBuffer, uiSize, true);
}

// FUNCTION: SWBF2 0x00730490
void FilePlatformBase::Seek(unsigned int uiPos)
{
    Seek(uiPos, true);
}

// FUNCTION: SWBF2 0x007304e0
unsigned int FilePlatformBase::Write(void* pBuffer, unsigned int uiSize)
{
    if (!pBuffer)
    {
        return 0;
    }
    return BufferedReadWrite(pBuffer, uiSize, false);
}

}
