#pragma once

#include "SoundFL/source/sndfileplatformbase.h"

namespace Snd
{

// VTABLE: SWBF2 0x00785c9c
class FilePlatform : public FilePlatformBase
{
public:
    FilePlatform();
    virtual ~FilePlatform();
    FilePlatform& operator=(FilePlatform& rOther);

    bool Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
              unsigned int uiSectorSize, unsigned int uiAlignment);
    void Close();

    virtual unsigned int GetSize() const;

protected:
    virtual void RealSeek(unsigned int uiPos);
    virtual unsigned int RealRead(void* pBuffer, unsigned int uiSize);
    virtual unsigned int RealWrite(void* pBuffer, unsigned int uiSize);

public:
    void* mHandle;
};
 

}
