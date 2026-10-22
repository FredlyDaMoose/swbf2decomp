#include "SaveFsm.h"
#include "PebbleFL/Common/PblUnicode.h"

#include "FLInputManager.h"

namespace SaveGameManager { extern unsigned int mReqSaveFile2Data; }
namespace SaveGameManager { extern unsigned int mReqSaveFile2Size; }
namespace SaveGameManager { extern unsigned int mReqSaveFile1Data; }
namespace SaveGameManager { extern unsigned int mReqSaveFile1Size; }

namespace SaveGameManager
{

static const float EPSILON = 0.0001f;

// GLOBAL: SWBF2 0x01faa220
static unsigned short sNextFileName[260];

}

namespace SaveGameManager {
void ResetFileList();
}

namespace SaveGameManager { extern unsigned char mForceNoSaveDevice; }

namespace SaveGameManager
{

// GLOBAL: SWBF2 0x01f3a1b0
// SYNTHETIC: SWBF2 0x00406350
// `dynamic initializer for 'SaveGameManager::mSaveFsm''
// SYNTHETIC: SWBF2 0x00769490
// `dynamic atexit destructor for 'SaveGameManager::mSaveFsm''
SaveFsm mSaveFsm;

// FUNCTION: SWBF2 0x00633050
bool CheckIfOkToExitMemoryMode()
{
    return !SaveGameManager::mForceNoSaveDevice && mSaveFsm.IsDeviceInsertted();
}

// FUNCTION: SWBF2 0x00633110
bool IsMemoryCardUnformatted()
{
    return mSaveFsm.IsMemoryCardUnformatted();
}

// FUNCTION: SWBF2 0x00633120
bool IsMemoryCardInsertted()
{
    return mSaveFsm.IsDeviceInsertted();
}

// FUNCTION: SWBF2 0x00633290
bool Input_Accept(int iAnswer)
{
    return mSaveFsm.InputAccept(iAnswer);
}

extern bool mMemoryMode;

// FUNCTION: SWBF2 0x00633040
void SetMemoryMode(bool bMemoryMode)
{
    mMemoryMode = bMemoryMode;
}

// FUNCTION: SWBF2 0x00633070
void StartQueue()
{
    mSaveFsm.EmptyQueue();
}

// FUNCTION: SWBF2 0x00633080
void QueueLoad(unsigned short const* pusFilename, unsigned int uiType, void* pDest, unsigned int uiSize)
{
    mSaveFsm.QueueFile(pusFilename, pDest, uiSize, uiType, QOP_LOAD);
}

// FUNCTION: SWBF2 0x006330a0
void QueueSave(unsigned short const* pusFilename, unsigned int uiType, void* pSrc, unsigned int uiSize)
{
    mSaveFsm.QueueFile(pusFilename, pSrc, uiSize, uiType, QOP_SAVE);
}

// FUNCTION: SWBF2 0x006330c0
void QueueDelete(unsigned short const* pusFilename, unsigned int uiType, bool bSilent)
{
    if (bSilent)
    {
        mSaveFsm.QueueFile(pusFilename, 0, 0, uiType, QOP_SILENTDELETE);
    }
    else
    {
        mSaveFsm.QueueFile(pusFilename, 0, 0, uiType, QOP_DELETE);
    }
}

// GLOBAL: SWBF2 0x01eaf004
void (*mOnDone)();

// FUNCTION: SWBF2 0x00632fa0
void StartInitialMemcardCheck(void (*pfnDone)())
{
    if (SaveGameManager::mForceNoSaveDevice)
    {
        pfnDone();
    }
    else
    {
        mOnDone = pfnDone;
        mSaveFsm.SetNextState(5);
    }
}

// FUNCTION: SWBF2 0x006330f0
void FinishQueue(void (*pfnDone)())
{
    mOnDone = pfnDone;
    mSaveFsm.ProcessQueuedFiles();
}

void MarkMemoryCard()
{
    mSaveFsm.MarkMemoryCard();
}

// FUNCTION: SWBF2 0x00633100
bool CheckMemoryCardMark()
{
    return mSaveFsm.CheckMemoryCardMark();
}

// FUNCTION: SWBF2 0x00633280
bool CanCreateNewFile(int  )
{
    return true;
}

}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00632f50
void Update(float fDeltaTime)
{
    if (FLInputManager::s_instance.AnyMissing() == true)
    {
        return;
    }

    mSaveFsm.FSMUpdate(fDeltaTime + EPSILON, false);
    if (mSaveFsm.IsDone() && mOnDone)
    {
        void (*pfnOnDone)() = mOnDone;
        mOnDone = 0;
        pfnOnDone();
    }
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00632fc0
void StartPreOp(void (*pfnDone)(), bool bAllowFormat, unsigned int uiCheckFreeBytes,
                unsigned int uiCheckFreeFiles, unsigned short const* pusFailOpStr, bool bSilentFail,
                bool bNoFileList, bool bUseIMCMsg, bool bIsPreLoad)
{
    if (SaveGameManager::mForceNoSaveDevice)
    {
        pfnDone();
        return;
    }
    mOnDone = pfnDone;
    mSaveFsm.StartPreOp(bAllowFormat, uiCheckFreeBytes, uiCheckFreeFiles, pusFailOpStr,
                        bSilentFail, bNoFileList, bUseIMCMsg, bIsPreLoad);
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00633000
bool GetSuccess()
{
    if (SaveGameManager::mForceNoSaveDevice)
    {
        return true;
    }
    return mSaveFsm.GetSuccess();
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00633020
bool InMemoryMode()
{
    if (SaveGameManager::mForceNoSaveDevice)
    {
        return true;
    }
    return mMemoryMode;
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00633130
bool IsUniqueFilename(unsigned short const* pusFilename, int iType)
{
    if (!pusFilename[0])
    {
        return false;
    }

    ResetFileList();
    for (SaveFile* pFile = mSaveFsm.GetNextFile(); pFile; pFile = mSaveFsm.GetNextFile())
    {
        if (pFile->GetType() == iType &&
            !_wcsicmp((wchar_t const*)pFile->GetGameName(), (wchar_t const*)pusFilename))
        {
            return false;
        }
    }

    if (SaveGameManager::mForceNoSaveDevice && !pusFilename[0])
    {
        return false;
    }
    return true;
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x006331a0
int GetUniqueFilename(unsigned short* pusFilename, int iStart, int iArg3, unsigned int uiArg4)
{
    unsigned short ausName[256];
    for (; iStart < 10000; ++iStart)
    {
        PblUnicode::usnprintf(ausName, 256, pusFilename, iStart);
        if (IsUniqueFilename(ausName, 1))
        {
            PblUnicode::ustrlcpy(pusFilename, ausName, 16);
            return iStart;
        }
    }
    return iStart;
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00633210
void ResetFileList()
{
    mSaveFsm.ResetFileIterator();
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00633220
unsigned short const* GetNextFileName(int iType, int* piArg)
{
    SaveFile* pFile;
    do
    {
        pFile = mSaveFsm.GetNextFile();
        if (!pFile)
        {
            return 0;
        }
    } while (pFile->GetType() != iType);

    PblUnicode::ustrlcpy(sNextFileName, pFile->GetGameName(), 260);
    if (piArg)
    {
        *piArg = 0;
    }
    return sNextFileName;
}
}

namespace SaveGameManager {

// FUNCTION: SWBF2 0x00632f20
void Cleanup()
{
    SaveGameManager::mReqSaveFile2Data = 0;
    SaveGameManager::mReqSaveFile1Data = 0;
    SaveGameManager::mReqSaveFile2Size = 0;
    SaveGameManager::mReqSaveFile1Size = 0;
}

}

namespace SaveGameManager {

// GLOBAL: SWBF2 0x01eaf00c
bool mMemoryMode;
}
