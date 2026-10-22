#include <windows.h>

#include "pcSaveDevice.h"
#include "PebbleFL/Common/PblUnicode.h"

#include <direct.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>

unsigned short* SAVE_DIRECTORY = (unsigned short*)L"SaveGames";

// GLOBAL: SWBF2 0x007886c0
unsigned short* SFTPrefix[4] = {
    (unsigned short*)L"",
    (unsigned short*)L"profile",
    (unsigned short*)L"gc",
    (unsigned short*)L"rote",
};

// FUNCTION: SWBF2 0x00426200
int SaveDevice::StartCreateFile(const unsigned short* pName, int iType)
{
    wchar_t path[_MAX_PATH];
    wchar_t search[_MAX_PATH];
    _wfinddata_t findData;

    _wmakepath(search, 0, (const wchar_t*)SAVE_DIRECTORY, L"*", L"*");
    _wfullpath(path, search, _MAX_PATH);
    intptr_t hFind = _wfindfirst(path, &findData);
    if (hFind == -1 && errno == ENOENT)
    {
        _wfullpath(path, (const wchar_t*)SAVE_DIRECTORY, _MAX_PATH);
        if (_wmkdir(path) == -1)
        {
            return -1;
        }
    }
    else
    {
        _findclose(hFind);
    }

    _wmakepath(search, 0, (const wchar_t*)SAVE_DIRECTORY, (const wchar_t*)pName,
               (const wchar_t*)SFTPrefix[iType]);
    _wfullpath(path, search, _MAX_PATH);
    FILE* pFile = _wfopen(path, L"wb+");
    if (pFile == 0)
    {
        return -1;
    }
    fclose(pFile);

    int iResult = StartRefreshFileList();
    if (iResult == 0)
    {
        do
            iResult = CycleRefreshFileList();
        while (iResult == 1);
    }
    return iResult;
}

// FUNCTION: SWBF2 0x00425f40
SaveDevice::SaveDevice()
    : mCurFile(0)
    , mLastAsyncResult(0)
{
}

// FUNCTION: SWBF2 0x00425f90
unsigned int SaveDevice::GetFreeSpace()
{
    ULARGE_INTEGER freeToCaller;
    ULARGE_INTEGER total;
    ULARGE_INTEGER totalFree;
    GetDiskFreeSpaceExW(L"\\", &freeToCaller, &total, &totalFree);
    if ((__int64)totalFree.QuadPart > 0xffffffff)
    {
        return 0xffffffff;
    }
    return totalFree.LowPart;
}

// FUNCTION: SWBF2 0x00425fd0
unsigned int SaveDevice::BytesToBlocks(unsigned int uiBytes)
{
    return uiBytes;
}

void SaveDevice::MarkMemoryCard()
{
}

// FUNCTION: SWBF2 0x00425ff0
bool SaveDevice::CheckMemoryCardMark()
{
    return true;
}

// FUNCTION: SWBF2 0x00426320
int SaveDevice::CycleCreateFile(const unsigned short* pName, int iType)
{
    return 0;
}

// FUNCTION: SWBF2 0x00426330
int SaveDevice::StartOpenFile(SaveFile& file, bool bArg)
{
    wchar_t path[_MAX_PATH];
    wchar_t search[_MAX_PATH];

    _wmakepath(search, 0, (const wchar_t*)SAVE_DIRECTORY, (const wchar_t*)file.mFileName,
               (const wchar_t*)SFTPrefix[file.GetType()]);
    _wfullpath(path, search, _MAX_PATH);
    FILE* pFile = _wfopen(path, L"rb+");
    if (pFile == 0)
    {
        return -1;
    }
    file.mFileHandle = pFile;
    mLastAsyncResult = 0;
    return 0;
}

// FUNCTION: SWBF2 0x004263c0
int SaveDevice::CycleOpenFile(SaveFile& file, bool bArg)
{
    return 0;
}

// FUNCTION: SWBF2 0x004263d0
int SaveDevice::StartWriteFile(SaveFile& file, void* pBuf, unsigned int uiSize)
{
    unsigned int uiWritten = fwrite(pBuf, 1, uiSize, file.mFileHandle);
    if (uiWritten != uiSize)
    {
        return -1;
    }
    mLastAsyncResult = uiWritten;
    return 0;
}

// FUNCTION: SWBF2 0x00426410
int SaveDevice::CycleWriteFile(SaveFile& file, void* pBuf, unsigned int uiSize)
{
    return 0;
}

// FUNCTION: SWBF2 0x00426420
int SaveDevice::StartReadFile(SaveFile& file, void* pBuf, unsigned int uiSize)
{
    unsigned int uiRead = fread(pBuf, 1, uiSize, file.mFileHandle);
    if (uiRead != uiSize)
    {
        return -5;
    }
    mLastAsyncResult = uiRead;
    return 0;
}

// FUNCTION: SWBF2 0x00426460
int SaveDevice::CycleReadFile(SaveFile& file, void* pBuf, unsigned int uiSize)
{
    return 0;
}

// FUNCTION: SWBF2 0x00426470
int SaveDevice::StartCloseFile(SaveFile& file, bool bArg)
{
    int iResult = fclose(file.mFileHandle);
    file.mFileHandle = 0;
    if (iResult != 0)
    {
        return -1;
    }
    mLastAsyncResult = 0;
    return 0;
}

// FUNCTION: SWBF2 0x004264c0
int SaveDevice::CycleCloseFile(SaveFile& file, bool bArg)
{
    return 0;
}

// FUNCTION: SWBF2 0x004264d0
int SaveDevice::StartDeleteFile(const unsigned short* pName, int iType, int iArg)
{
    wchar_t path[_MAX_PATH];
    wchar_t search[_MAX_PATH];

    _wmakepath(search, 0, (const wchar_t*)SAVE_DIRECTORY, (const wchar_t*)pName,
               (const wchar_t*)SFTPrefix[iType]);
    _wfullpath(path, search, _MAX_PATH);
    _wremove(path);

    int iResult = StartRefreshFileList();
    if (iResult == 0)
    {
        do
            iResult = CycleRefreshFileList();
        while (iResult == 1);
    }
    return iResult;
}

// FUNCTION: SWBF2 0x00426550
int SaveDevice::CycleDeleteFile(const unsigned short* pName, int iType)
{
    return 0;
}

// FUNCTION: SWBF2 0x00426560
int SaveDevice::StartFormat()
{
    return 0;
}

// FUNCTION: SWBF2 0x00426570
int SaveDevice::CycleFormat()
{
    return 0;
}

// FUNCTION: SWBF2 0x00426580
SaveFile* SaveDevice::FindFileInList(const unsigned short* pName, int iType)
{
    ResetFileIterator();
    for (SaveFile* pFile = GetNextFile(); pFile; pFile = GetNextFile())
    {
        if (pFile->GetType() == iType && PblUnicode::ustrcmp(pName, pFile->GetGameName()) == 0)
        {
            return pFile;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00425f80
int SaveDevice::GetStatus()
{
    return 0;
}

// FUNCTION: SWBF2 0x00425fe0
unsigned int SaveDevice::GetFreeFiles()
{
    return 100;
}

// FUNCTION: SWBF2 0x00426000
int SaveDevice::StartRefreshFileList()
{
    _wfinddata_t findData;
    wchar_t ext[256];
    wchar_t name[256];
    wchar_t path[_MAX_PATH];
    wchar_t search[_MAX_PATH];

    EmptyFileList();
    ResetFileIterator();

    _wmakepath(search, 0, (const wchar_t*)SAVE_DIRECTORY, L"*", L"*");
    _wfullpath(path, search, _MAX_PATH);
    intptr_t hFind = _wfindfirst(path, &findData);
    if (hFind != -1)
    {
        do
        {
            if (wcscmp(L".", findData.name) != 0 && wcscmp(L"..", findData.name) != 0)
            {
                _wsplitpath(findData.name, 0, 0, name, ext);

                int iType = 0;
                if (ext[0])
                {
                    for (int i = 1; i < 4; ++i)
                    {
                        if (wcscmp((const wchar_t*)SFTPrefix[i], ext + 1) == 0)
                        {
                            iType = i;
                            break;
                        }
                    }
                }

                if (mNumFiles < 200)
                {
                    mFileList[mNumFiles++].SetFileInfo((const unsigned short*)name, iType);
                }
            }
        } while (_wfindnext(hFind, &findData) != -1);
    }
    _findclose(hFind);

    mLastAsyncResult = 0;
    return 0;
}

// FUNCTION: SWBF2 0x004261f0
int SaveDevice::CycleRefreshFileList()
{
    return 0;
}

// FUNCTION: SWBF2 0x004265e0
void SaveDevice::ResetFileIterator()
{
    mCurFile = 0;
}

// FUNCTION: SWBF2 0x004265f0
SaveFile* SaveDevice::GetNextFile()
{
    if (mCurFile >= mNumFiles)
    {
        return 0;
    }
    return &mFileList[mCurFile++];
}

// FUNCTION: SWBF2 0x00426620
void SaveDevice::EmptyFileList()
{
    mNumFiles = 0;
}
