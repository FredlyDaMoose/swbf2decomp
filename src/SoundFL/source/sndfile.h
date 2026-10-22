#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "sndfileplatformbase.h"
#include "sndsemaphorebase.h"
#include "sndthreadbase.h"
#include "win/sndfileplatform.h"
#include "win/sndsemaphore.h"
#include "win/sndthread.h"

namespace Snd
{

class File
{
public:
    enum FileOp
    {
        fileOp_nothing = 0,
        fileOp_open,
        fileOp_read,
        fileOp_write
    };

    static bool __cdecl SystemOpen(unsigned int uiPriority);
    static void __cdecl SystemClose();

    File();
    File(void(__cdecl* pfnOpComplete)(File*, void*, bool), void* pCallbackData);
    ~File();
    File& operator=(File& rOther);

    void Open(FilePlatform& rFile);
    void Open(char const* pcFileName, unsigned int uiAccessFlags, unsigned int uiBufferSize,
              unsigned int uiSectorSize, unsigned int uiAlignment);
    void Close();

    bool IsOpen() const { return mFile.IsOpen(); }
    void Read(void* pBuffer, unsigned int uiSize);
    void Write(void* pBuffer, unsigned int uiSize);
    void Seek(unsigned int uiPos);
    unsigned int GetPos() const;
    void SetPriority(unsigned int uiPriority);

    unsigned int GetSize() const
    {
        return mFile.GetSize();
    }

    void SetFileOpCompleteFunc(void(__cdecl* pfnOpComplete)(File*, void*, bool),
                               void* pCallbackData)
    {
        mOpCompleteFunc = pfnOpComplete;
        mCallbackData = pCallbackData;
    }

private:
    void CompleteFileOp();
    static void __cdecl FileThread(void* pStartedSignal);
    static bool __cdecl InsertFile(File* pFile);

public:
    FilePlatform mFile;
    void(__cdecl* mOpCompleteFunc)(File*, void*, bool);
    void* mCallbackData;
    unsigned int mFilePos;
    union
    {
        char* filename;
        void* dataBlock;
    } mFileOpData;
    unsigned int mBlockSize;
    FileOp mFileOp;
    unsigned int mHashID;
    PblListDoubleSorted::Node mNode;
    unsigned int mAccessFlags;
    unsigned int mBufferSize;
    unsigned int mSectorSize;
    unsigned int mAlignment;
    Semaphore mLock;

    static Thread smFileThread;
    static Semaphore smQueuedCommands;
    static Semaphore smQueuedFileOpListLock;
    static PblListDoubleSorted smQueuedFileOpList;
    static bool smExitFileThread;
    static bool smFileSystemOpen;
    static PblListDoubleSorted smFileList;
};
 

}
