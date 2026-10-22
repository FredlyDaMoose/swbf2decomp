#include <windows.h>

#include <string.h>

// LIBRARY: SWBF2 0x00752e02
// memcpy

#include "PebbleFL/Common/PblFile.h"

#include "SoundFL/source/sndfile.h"

#include "RedEngineFL/Memory/RedMemory.h"
#include "Battlefront2/Source/GameMemory.h"
#include "Battlefront2/Source/NetGame.h"

namespace RedMissionLog { bool Enabled(); }

// GLOBAL: SWBF2 0x007ec440

int sPreviousHeap;
// GLOBAL: SWBF2 0x01e2fd3c
int _fileOpCompleted;

void(__cdecl* PblFile::_blockCallbackFunc)(void) = 0;

// FUNCTION: SWBF2 0x00425840
static void PblFile_SetHeap(bool bSet)
{
    if (bSet)
    {
        int iHeap;
        if (RedMissionLog::Enabled() || NetGame::IsJournal())
        {
            iHeap = __RedDebugHeap;
        }
        else if (GameMemory::TempLoadHeap == 0 || GameMemory::TempLoadHeap == -1)
        {
            iHeap = GameMemory::RunTimeHeap;
        }
        else
        {
            iHeap = GameMemory::TempLoadHeap;
        }
        sPreviousHeap = RedSetCurrentHeap(iHeap);
    }
    else if (sPreviousHeap != -1)
    {
        RedSetCurrentHeap(sPreviousHeap);
        sPreviousHeap = -1;
    }
}

// FUNCTION: SWBF2 0x004258b0
void PblFile::SetBlockCallback(void(__cdecl* pFunc)(void))
{

    _blockCallbackFunc = pFunc;
}

// FUNCTION: SWBF2 0x004258c0
static void FileOpComplete(Snd::File*, void* pResult, bool bSuccess)
{
    if (pResult)
    {
        *(bool*)pResult = bSuccess;
    }
    _fileOpCompleted = 1;
}

// FUNCTION: SWBF2 0x004258e0
static void FileOpWait()
{

    void(__cdecl* pBlockFunc)(void) = PblFile::_blockCallbackFunc;

    if (_fileOpCompleted == 0)
    {
        HANDLE hThread = GetCurrentThread();
        int nPriority = GetThreadPriority(hThread);
        if (pBlockFunc == 0)
        {
            SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
        }
        while (_fileOpCompleted == 0)
        {
            if (pBlockFunc)
            {
                pBlockFunc();
            }
            Sleep(0);
        }
        SetThreadPriority(hThread, nPriority);
    }
    else if (pBlockFunc)
    {
        pBlockFunc();
    }
}

// FUNCTION: SWBF2 0x00425960
PblFile::PblFile()
{

    _iPos = 0;
    _bIsOpen = false;
    _bAsynchronous = false;
    _bOSBuffer = false;
    _bBuffer = false;
    _bIsMemFile = false;
    _Buffer = 0;
}

// FUNCTION: SWBF2 0x00425980
PblFile::~PblFile()
{
    if (_bIsOpen)
    {
        Close();
    }
}

// FUNCTION: SWBF2 0x00425990
bool PblFile::Open(const char* pcFileName, bool bAsynchronous, bool bOSBuffer, bool bBuffer)
{
    _bAsynchronous = bAsynchronous;
    PblFile_SetHeap(true);
    if (_bAsynchronous)
    {
        _Buffer = (char*)new Snd::File(FileOpComplete, &_bIsOpen);
        if (_Buffer)
        {
            _fileOpCompleted = 0;
            ((Snd::File*)_Buffer)->Open(pcFileName, 1, bBuffer ? 0x20000 : 0, 0x800, 4);
            FileOpWait();
            if (_bIsOpen)
            {
                ((Snd::File*)_Buffer)->SetFileOpCompleteFunc(FileOpComplete, 0);
                PblFile_SetHeap(false);
                return true;
            }
            delete (Snd::File*)_Buffer;
        }
    }
    else
    {
        _Buffer = (char*)new Snd::FilePlatform();
        if (_Buffer)
        {
            _bIsOpen = ((Snd::FilePlatform*)_Buffer)->Open(pcFileName, bOSBuffer ? 8 | 1 : 1,
                                                           bBuffer ? 0x20000 : 0, 0x800, 4);
            if (_bIsOpen)
            {
                PblFile_SetHeap(false);
                return true;
            }
            delete (Snd::FilePlatform*)_Buffer;
        }
    }
    _Buffer = 0;
    PblFile_SetHeap(false);
    return false;
}

// FUNCTION: SWBF2 0x00425b20
bool PblFile::OpenCreate(const char* pcFileName, bool bAsynchronous, bool bOSBuffer, bool bBuffer)
{

    PblFile_SetHeap(true);
    _bAsynchronous = false;
    _Buffer = (char*)new Snd::FilePlatform();
    if (_Buffer)
    {
        _bIsOpen = ((Snd::FilePlatform*)_Buffer)->Open(pcFileName, 0x0F, 0, 0x800, 4);
    }
    if (!_bIsOpen)
    {
        delete (Snd::FilePlatform*)_Buffer;
        _Buffer = 0;
    }
    PblFile_SetHeap(false);
    return _bIsOpen;
}

// FUNCTION: SWBF2 0x00425bd0
void PblFile::Close()
{
    PblFile_SetHeap(true);
    if (_bAsynchronous)
    {
        if (_Buffer)
        {
            ((Snd::File*)_Buffer)->Close();
            delete (Snd::File*)_Buffer;
            _Buffer = 0;
        }
    }
    else if (_Buffer)
    {
        ((Snd::FilePlatform*)_Buffer)->Close();
        delete (Snd::FilePlatform*)_Buffer;
        _Buffer = 0;
    }
    _bIsOpen = false;
    PblFile_SetHeap(false);
}

// FUNCTION: SWBF2 0x00425c40
void PblFile::Delete(const char* pcFileName)
{
    DeleteFileA(pcFileName);
}

// FUNCTION: SWBF2 0x00425c50
bool PblFile::Exists(const char* pcFileName)
{
    WIN32_FIND_DATAA FindFileData;
    HANDLE hFindFile = FindFirstFileA(pcFileName, &FindFileData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    FindClose(hFindFile);
    return true;
}

// FUNCTION: SWBF2 0x00425c90
int PblFile::GetSize()
{
    if (_bIsMemFile)
    {
        return MemGetSize();
    }
    if (_Buffer == 0)
    {
        return 0;
    }
    if (_bAsynchronous)
    {
        return ((Snd::File*)_Buffer)->GetSize();
    }
    return ((Snd::FilePlatform*)_Buffer)->GetSize();
}

// FUNCTION: SWBF2 0x00425d00
char PblFile::Read8()
{
    char cValue;
    ReadData(&cValue, 1);
    return cValue;
}

// FUNCTION: SWBF2 0x00425ce0
short PblFile::Read16()
{
    short sValue;
    ReadData(&sValue, 2);
    return sValue;
}

// FUNCTION: SWBF2 0x00425cc0
int PblFile::Read32()
{
    int iValue;
    ReadData(&iValue, 4);
    return iValue;
}

// FUNCTION: SWBF2 0x00425d20
float PblFile::ReadFloat()
{
    float fValue;
    ReadData(&fValue, 4);
    return fValue;
}

// FUNCTION: SWBF2 0x00425d40
void PblFile::ReadData(void* pBuffer, int iNumBytes)
{
    if (_bIsMemFile)
    {
        MemRead(pBuffer, iNumBytes);
        return;
    }
    if (_bAsynchronous)
    {
        _fileOpCompleted = 0;
        ((Snd::File*)_Buffer)->Read(pBuffer, iNumBytes);
        FileOpWait();
    }
    else
    {
        ((Snd::FilePlatform*)_Buffer)->Read(pBuffer, iNumBytes);
    }
}

// FUNCTION: SWBF2 0x00425db0
void PblFile::WriteData(void* pBuffer, int iNumBytes)
{
    if (_bIsMemFile)
    {
        MemWrite(pBuffer, iNumBytes);
        return;
    }

    ((Snd::FilePlatform*)_Buffer)->Write(pBuffer, iNumBytes);
}

// FUNCTION: SWBF2 0x00425e10
void PblFile::WriteString(const char* pcText)
{
    WriteData((void*)pcText, strlen(pcText));
}

// FUNCTION: SWBF2 0x00425e40
void PblFile::Seek(int iOffset, SeekType eFrom)
{
    if (_bIsMemFile)
    {
        MemSeek(iOffset, eFrom);
        return;
    }
    if (_bAsynchronous)
    {
        switch (eFrom)
        {
        case SeekFromHere:
            ((Snd::File*)_Buffer)->Seek(Tell() + iOffset);
            break;
        case SeekFromStart:
            ((Snd::File*)_Buffer)->Seek(iOffset);
            break;
        case SeekFromEnd:
            ((Snd::File*)_Buffer)->Seek(((Snd::File*)_Buffer)->GetSize() - iOffset);
            break;
        }
    }
    else
    {
        switch (eFrom)
        {
        case SeekFromHere:
            ((Snd::FilePlatform*)_Buffer)->Seek(Tell() + iOffset);
            break;
        case SeekFromStart:
            ((Snd::FilePlatform*)_Buffer)->Seek(iOffset);
            break;
        case SeekFromEnd:
            ((Snd::FilePlatform*)_Buffer)->Seek(
                ((Snd::FilePlatform*)_Buffer)->GetSize() - iOffset);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00425f20
int PblFile::Tell() const
{
    if (_bIsMemFile)
    {
        return MemTell();
    }
    if (_bAsynchronous)
    {
        return ((Snd::File*)_Buffer)->GetPos();
    }
    return ((Snd::FilePlatform*)_Buffer)->GetPos();
}

void PblFile::SeekAlign(int iAlign)
{
    int iPad = -Tell() & (iAlign - 1);
    if (iPad)
    {
        Seek(iPad, SeekFromHere);
    }
}

unsigned int PblFile::MemGetSize() const
{
    return _iBufferSize;
}

void PblFile::MemRead(void* pBuffer, int iNumBytes)
{
    if (_iPos + iNumBytes > _iBufferSize)
    {
        iNumBytes = _iBufferSize - _iPos;
    }
    if (iNumBytes != 0)
    {
        memcpy(pBuffer, _Buffer + _iPos, iNumBytes);
        _iPos += iNumBytes;
    }
}

void PblFile::MemWrite(void* pBuffer, int iNumBytes)
{
    if (_iPos + iNumBytes > _iBufferSize)
    {
        iNumBytes = _iBufferSize - _iPos;
    }
    if (iNumBytes != 0)
    {
        memcpy(_Buffer + _iPos, pBuffer, iNumBytes);
        _iPos += iNumBytes;
    }
}

unsigned int PblFile::MemTell() const
{
    return _iPos;
}

void PblFile::MemSeek(int iOffset, SeekType eFrom)
{
    switch (eFrom)
    {
    case SeekFromHere:
        _iPos += iOffset;
        break;
    case SeekFromStart:
        _iPos = iOffset;
        break;
    case SeekFromEnd:
        _iPos = _iBufferSize - iOffset;
        break;
    }
}

// GLOBAL: SWBF2 0x00964f78
namespace RedMissionLog { bool mEnabled; }

// FUNCTION: SWBF2 0x006ccca0
bool RedMissionLog::Enabled()
{
    return RedMissionLog::mEnabled;
}
