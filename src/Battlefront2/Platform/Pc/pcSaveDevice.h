#pragma once

#include "pcSaveFile.h"

#include <windows.h>
#undef GetFreeSpace

class SaveDevice
{
public:
    SaveDevice();
    int StartCreateFile(const unsigned short* pName, int iType);
    int StartRefreshFileList();
    int CycleRefreshFileList();

    int GetStatus();
    unsigned int GetFreeFiles();
    void ResetFileIterator();
    SaveFile* GetNextFile();

    unsigned int GetFreeSpace();

    static unsigned int BytesToBlocks(unsigned int uiBytes);
    void MarkMemoryCard();
    bool CheckMemoryCardMark();
    int CycleCreateFile(const unsigned short* pName, int iType);
    int StartOpenFile(SaveFile& file, bool bArg);
    int CycleOpenFile(SaveFile& file, bool bArg);
    int StartWriteFile(SaveFile& file, void* pBuf, unsigned int uiSize);
    int CycleWriteFile(SaveFile& file, void* pBuf, unsigned int uiSize);
    int StartReadFile(SaveFile& file, void* pBuf, unsigned int uiSize);
    int CycleReadFile(SaveFile& file, void* pBuf, unsigned int uiSize);
    int StartCloseFile(SaveFile& file, bool bArg);
    int CycleCloseFile(SaveFile& file, bool bArg);
    int StartDeleteFile(const unsigned short* pName, int iType, int iArg);
    int CycleDeleteFile(const unsigned short* pName, int iType);
    int StartFormat();
    int CycleFormat();
    SaveFile* FindFileInList(const unsigned short* pName, int iType);
private:
    void EmptyFileList();
public:

    SaveFile mFileList[200];
    int mNumFiles;
    int mCurFile;
    int mLastAsyncResult;
};
