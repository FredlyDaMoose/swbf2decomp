#include <windows.h>

#include "sndfileplatform.h"

namespace Snd
{

// FUNCTION: SWBF2 0x0072fb20
FilePlatform::FilePlatform()
{
    mHandle = INVALID_HANDLE_VALUE;
}

// FUNCTION: SWBF2 0x0072fb40
FilePlatform::~FilePlatform()
{
    if (IsOpen())
    {
        Close();
    }
}

// SYNTHETIC: SWBF2 0x0072fbf0
// Snd::FilePlatform::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0072fbc0
FilePlatform& FilePlatform::operator=(FilePlatform& rOther)
{
    FilePlatformBase::operator=(rOther);
    mHandle = rOther.mHandle;
    rOther.mHandle = INVALID_HANDLE_VALUE;
    return *this;
}

// FUNCTION: SWBF2 0x0072fc80
void FilePlatform::Close()
{
    FilePlatformBase::Close();
    CloseHandle(mHandle);
    mHandle = INVALID_HANDLE_VALUE;
}

// FUNCTION: SWBF2 0x0072fca0
unsigned int FilePlatform::GetSize() const
{
    DWORD dwSizeHigh;
    return GetFileSize(mHandle, &dwSizeHigh);
}

// FUNCTION: SWBF2 0x0072fcc0
bool FilePlatform::Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
                        unsigned int uiSectorSize, unsigned int uiAlignment)
{
    DWORD dwAccess = ((uiAccessFlags & 1) ? GENERIC_READ : 0) | ((uiAccessFlags & 2) ? GENERIC_WRITE : 0);
    DWORD dwDisposition = (uiAccessFlags & 4) ? CREATE_ALWAYS : OPEN_EXISTING;

    if (uiAccessFlags & 6)
    {

        uiAccessFlags |= 8;
        uiSectorSize = 1;
        uiAlignment = 1;
    }
    else
    {

        DWORD dwSectorsPerCluster, dwBytesPerSector, dwFreeClusters, dwTotalClusters;
        if (GetDiskFreeSpaceA(NULL, &dwSectorsPerCluster, &dwBytesPerSector, &dwFreeClusters,
                              &dwTotalClusters))
        {
            uiBufferSize = (uiBufferSize - 1 + dwBytesPerSector) / dwBytesPerSector * dwBytesPerSector;
            uiAlignment = (uiAlignment - 1 + dwBytesPerSector) / dwBytesPerSector * dwBytesPerSector;
            uiSectorSize = dwBytesPerSector;
        }
    }

    if (FilePlatformBase::Open(pcFileName, uiAccessFlags, uiBufferSize, uiSectorSize, uiAlignment))
    {
        DWORD dwShare;
        DWORD dwAttributes;
        if (uiAccessFlags & 6)
        {
            dwShare = 0;
            dwAttributes = FILE_FLAG_RANDOM_ACCESS | FILE_ATTRIBUTE_NORMAL;
        }
        else
        {
            dwShare = FILE_SHARE_READ;
            dwAttributes = FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL
                           | ((mAccess & 8) ? 0 : FILE_FLAG_NO_BUFFERING);
        }

        mHandle = CreateFileA(pcFileName, dwAccess, dwShare, 0, dwDisposition, dwAttributes, 0);
        if (mHandle != INVALID_HANDLE_VALUE)
        {
            return true;
        }
    }

    FilePlatformBase::Close();
    return false;
}

// FUNCTION: SWBF2 0x0072fdc0
unsigned int FilePlatform::RealRead(void* pBuffer, unsigned int uiSize)
{
    DWORD dwRead;
    return ReadFile(mHandle, pBuffer, uiSize, &dwRead, 0) ? dwRead : 0;
}

// FUNCTION: SWBF2 0x0072fdf0
void FilePlatform::RealSeek(unsigned int uiPos)
{
    SetFilePointer(mHandle, uiPos, 0, FILE_BEGIN);
}

// FUNCTION: SWBF2 0x0072fe10
unsigned int FilePlatform::RealWrite(void* pBuffer, unsigned int uiSize)
{
    DWORD dwWritten;
    return WriteFile(mHandle, pBuffer, uiSize, &dwWritten, 0) ? dwWritten : 0;
}

}
