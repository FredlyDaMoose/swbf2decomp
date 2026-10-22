#include "SaveFsm.h"
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "LuaHelper.h"
#include "PebbleFL/Common/PblHashTable.h"

extern void* GetNextSaveFile(void* pDevice);
extern void sub_631640();
extern unsigned int CalcCRC(void const* pData, unsigned int uiSize, unsigned int uiSeed);
namespace RedLocalizeDB { const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing); }

extern int SafeLocalizeSPrintf(void* pDest, unsigned int uiSize, char const* pcFormat, int iArg);

extern int SafeLocalizeSPrintf(void* pDest, unsigned int uiSize, char const* pcFormat,
                       void const* pArg1, int iArg2);

// FUNCTION: SWBF2 0x00631550
int SafeLocalizeSPrintf(void* pDest, unsigned int uiSize, char const* pcFormat, int iArg)
{
    const unsigned short* pusFormat = RedLocalizeDB::Find(PblHash(pcFormat), false);
    if (pusFormat)
    {
        return PblUnicode::uvsnprintf((unsigned short*)pDest, unsigned int(-1), pusFormat,
                                      (char*)&iArg);
    }
    PblUnicode::ustrncnv((unsigned short*)pDest, pcFormat, uiSize);
    return 0;
}

int SafeLocalizeSPrintf(void* pDest, unsigned int uiSize, char const* pcFormat,
               void const* pArg1, int iArg2)
{
    const unsigned short* pusFormat = RedLocalizeDB::Find(PblHash(pcFormat), false);
    if (pusFormat)
    {
        return PblUnicode::uvsnprintf((unsigned short*)pDest, unsigned int(-1), pusFormat,
                                      (char*)&pArg1);
    }
    PblUnicode::ustrncnv((unsigned short*)pDest, pcFormat, uiSize);
    return 0;
}
unsigned short* SafeLocalizeStrcpy(unsigned short* pusDest, char const* pcSrc, unsigned int uiSize);

namespace RedLocalizeDB { const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing); }

// FUNCTION: SWBF2 0x00631600
unsigned int Hash16(unsigned short const* pusString)
{
    if (!pusString || !*pusString)
    {
        return 0;
    }
    unsigned int uHash = 0x811C9DC5u;
    while (*pusString)
    {
        uHash = (uHash ^ *pusString++) * 0x01000193u;
    }
    return uHash;
}

// FUNCTION: SWBF2 0x00631650
SaveFsm::SaveFsm()
    : mCurSaveFile(0)
    , mLoadQueueCnt(0)
    , mXBOXTotalFreeNeeded(-1)
    , mXBOXAdditionalFreeNeeded(-1)
{
    mCurState = 1;
    mNextState = 2;
}

// SYNTHETIC: SWBF2 0x006316f0
// SaveFsm::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00631720
bool SaveFsm::IsDone() const
{
    int iState = mCurState;
    return iState == 1 || iState == 100 || iState == 2 || iState == 3 || iState == 4;
}

// FUNCTION: SWBF2 0x00631770
void SaveFsm::StartPreOp(bool bAllowFormat, unsigned int uiCheckFreeBytes,
                          unsigned int uiCheckFreeFiles, unsigned short const* pusFailOpStr,
                          bool bSilentFail, bool bNoFileList, bool bUseIMCMsg, bool bIsPreLoad)
{
    m_preop_AllowFormat = bAllowFormat;
    m_preop_CheckFreeBytes = uiCheckFreeBytes;
    m_preop_CheckFreeFiles = uiCheckFreeFiles;
    PblUnicode::ustrlcpy((unsigned short*)mFailOpStr, pusFailOpStr, 64);
    m_preop_SilentFail = bSilentFail;
    m_preop_NoFileList = bNoFileList;
    m_preop_UseIMCMsg = bUseIMCMsg;
    m_preop_bIsPreLoad = bIsPreLoad;
    SetNextState(8);
}

// FUNCTION: SWBF2 0x00631820
void SaveFsm::EmptyQueue()
{
    mLoadQueueCnt = 0;
}

// FUNCTION: SWBF2 0x00631830
void SaveFsm::QueueFile(unsigned short const* pusFilename, void* pDest, unsigned int uiSize,
                         int iType, SaveFsmQueueOp eOp)
{
    if (mLoadQueueCnt < MAX_LOAD_QUEUE)
    {
        PblUnicode::ustrlcpy((unsigned short*)mLoadFilename[mLoadQueueCnt], pusFilename, 66);
        mLoadDest[mLoadQueueCnt] = pDest;
        mLoadSize[mLoadQueueCnt] = uiSize;
        mLoadType[mLoadQueueCnt] = iType;
        mLoadOp[mLoadQueueCnt] = eOp;
        ++mLoadQueueCnt;
    }
}

// FUNCTION: SWBF2 0x006318a0
void SaveFsm::ProcessQueuedFiles()
{
    SetNextState(31);
}

// FUNCTION: SWBF2 0x006318b0
bool SaveFsm::InputAccept(int iAnswer)
{
    mLoadSavePopupAnswer = iAnswer;
    return true;
}

// FUNCTION: SWBF2 0x00631810
SaveFile* SaveFsm::GetNextFile()
{
    return (SaveFile*)GetNextSaveFile(&mSaveDevice);
}

void SaveFsm::MarkMemoryCard()
{
    mSaveDevice.MarkMemoryCard();
}

// FUNCTION: SWBF2 0x006317d0
bool SaveFsm::CheckMemoryCardMark()
{
    return mSaveDevice.CheckMemoryCardMark();
}

// FUNCTION: SWBF2 0x006317e0
bool SaveFsm::IsMemoryCardUnformatted()
{
    return mSaveDevice.GetStatus() == -4;
}

// FUNCTION: SWBF2 0x006317f0
bool SaveFsm::IsDeviceInsertted()
{
    return mSaveDevice.GetStatus() != -2;
}

// FUNCTION: SWBF2 0x00631800
void SaveFsm::ResetFileIterator()
{
    mSaveDevice.ResetFileIterator();
}

// FUNCTION: SWBF2 0x006318d0
void SaveFsm::EnterState()
{
    switch (mCurState)
    {
    case 5:
        mErrorSuccess = false;
        {
            int iResult = mSaveDevice.GetStatus();
            if (iResult == -2 || iResult == -4)
            {
                SetNextState(3);
            }
            else
            {
                int iNeeded = (int)SaveDevice::BytesToBlocks(0x1800);
                unsigned int uiNeeded = 35;
                if (iNeeded > 35)
                {
                    uiNeeded = iNeeded;
                }
                unsigned int uiFree = mSaveDevice.GetFreeSpace();
                if (uiFree >= uiNeeded)
                {
                    SetNextState(3);
                }
                else
                {
                    mXBOXTotalFreeNeeded = uiNeeded >> 10;
                    mXBOXAdditionalFreeNeeded = uiNeeded - uiFree;
                    SetNextState(6);
                }
            }
        }
        return;

    case 6:
        if (mSaveDevice.StartRefreshFileList())
        {
            SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save08", 400);
            SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
            mBtns = 2;
            mBtnDefault = 2;
            SetNextState(7);
        }
        else
        {
            SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save19", 400);
            mBtns = 0;
            ShowLoadSavePopup(true);
        }
        return;

    case 7:
    case 0xA:
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 8:
        Enter_PreOp();
        return;

    case 9:

        if (m_preop_SilentFail)
        {
            SetNextState(4);
        }
        else
        {
            mLoadSavePopupAnswer = -1;
            ShowLoadSavePopup(true);
        }
        return;

    case 0xC:
        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save10", (int)mFailOpStr);
        SafeLocalizeStrcpy(mBtnB, "common.no", 100);
        SafeLocalizeStrcpy(mBtnC, "common.yes", 100);
        mBtns = 6;
        mBtnDefault = 2;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0xD:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save11", 400);
        mBtns = 0;
        ShowLoadSavePopup(true);
        {
            int iResult = mSaveDevice.GetStatus();
            if (iResult == -4)
            {

                if ((unsigned int)mSaveDevice.StartFormat() < 2)
                {
                    return;
                }

                const unsigned short* pJoy = RedLocalizeDB::Find(0x7CF9A572u, false);
                SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save13", (int)pJoy);
                SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
            }
            else
            {
                const unsigned short* pJoy = RedLocalizeDB::Find(0x7CF9A572u, false);
                SafeLocalizeSPrintf(mMsgStr1, 400, iResult == -2 ? "ifs.loadsave_pc.save04" : "ifs.loadsave_pc.save13", (int)pJoy);
                SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
            }
        }
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
        return;

    case 0xE:
        {
            int iIdx = mLoadQueueCnt - 1;
            mSaveErrorAfterClose = false;
            int iType = mLoadType[iIdx];
            if (!iType || (mCurSaveFile = mSaveDevice.FindFileInList(mLoadFilename[iIdx], iType), sub_631640(), mCurSaveFile != 0))
            {
                SetNextState(0x11);
            }
            else
            {
                SetNextState(0x10);
            }
        }
        return;

    case 0xF:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save14", 400);
        SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.yes", 100);
        SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.no", 100);
        mBtns = 3;
        mBtnDefault = 2;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0x10:
        {
            SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save15", 400);
            mBtns = 0;
            mLoadSavePopupAnswer = -1;
            ShowLoadSavePopup(true);
            int iIdx = mLoadQueueCnt - 1;
            int iResult = mSaveDevice.StartCreateFile(mLoadFilename[iIdx], mLoadType[iIdx]);
            if ((unsigned int)iResult >= 2)
            {
                SetNextState(iResult == -6 ? 22 : 21);
            }
        }
        return;

    case 0x11:
        {
            SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save15", 400);
            mBtns = 0;
            ShowLoadSavePopup(true);
            int iIdx = mLoadQueueCnt - 1;
            mCurSaveFile = mSaveDevice.FindFileInList(mLoadFilename[iIdx], mLoadType[iIdx]);
            sub_631640();
            if (!mCurSaveFile || (unsigned int)mSaveDevice.StartOpenFile(*mCurSaveFile, false) >= 2)
            {
                SetNextState(21);
            }
        }
        return;

    case 0x12:
        {
            int iIdx = mLoadQueueCnt - 1;
            if (mLoadType[iIdx])
            {
                mChecksum = CalcCRC(mLoadDest[iIdx], mLoadSize[iIdx], 0);
                if ((unsigned int)mSaveDevice.StartWriteFile(*mCurSaveFile, &mChecksum, CRC_SIZE) >= 2)
                {
                    mSaveErrorAfterClose = true;
                    SetNextState(0x14);
                }
            }
            else
            {
                SetNextState(0x13);
            }
        }
        return;

    case 0x13:
        {
            int iIdx = mLoadQueueCnt - 1;
            if ((unsigned int)mSaveDevice.StartWriteFile(*mCurSaveFile, mLoadDest[iIdx], mLoadSize[iIdx]) >= 2)
            {
                mSaveErrorAfterClose = true;
                SetNextState(0x14);
            }
        }
        return;

    case 0x14:
        if ((unsigned int)mSaveDevice.StartCloseFile(*mCurSaveFile, false) >= 2)
        {
            SetNextState(21);
        }
        return;

    case 0x15:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save16", 400);
        SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0x16:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save03", 400);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0x17:
        mLoadErrorAfterClose = false;
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save18", 400);
        mBtns = 0;
        ShowLoadSavePopup(true);
        {
            int iIdx = mLoadQueueCnt - 1;
            mCurSaveFile = mSaveDevice.FindFileInList(mLoadFilename[iIdx], mLoadType[iIdx]);
            sub_631640();
            SetNextState(mCurSaveFile ? 24 : 28);
        }
        return;

    case 0x18:
        if ((unsigned int)mSaveDevice.StartOpenFile(*mCurSaveFile, false) >= 2)
        {
            SetNextState(28);
        }
        return;

    case 0x19:
        if ((unsigned int)mSaveDevice.StartReadFile(*mCurSaveFile, &mChecksum, CRC_SIZE) >= 2)
        {
            mLoadErrorAfterClose = true;
            SetNextState(27);
        }
        return;

    case 0x1A:
        {
            int iIdx = mLoadQueueCnt - 1;
            if ((unsigned int)mSaveDevice.StartReadFile(*mCurSaveFile, mLoadDest[iIdx], mLoadSize[iIdx]) >= 2)
            {
                mLoadErrorAfterClose = true;
                SetNextState(27);
            }
        }
        return;

    case 0x1B:
        if ((unsigned int)mSaveDevice.StartCloseFile(*mCurSaveFile, false) >= 2)
        {
            SetNextState(28);
        }
        return;

    case 0x1C:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save17", 400);
        SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.ok", 100);
        mBtns = 1;
        mBtnDefault = 1;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0x1D:
        {

            int iIdx = mLoadQueueCnt - 1;
            char const* pcMsg = mLoadOp[iIdx] != 3 ? "ifs.loadsave_pc.save22" : "ifs.loadsave_pc.save15";
            mLoadErrorAfterClose = false;
            SafeLocalizeStrcpy(mMsgStr1, pcMsg, 400);
            mBtns = 0;
            ShowLoadSavePopup(true);
            mCurSaveFile = mSaveDevice.FindFileInList(mLoadFilename[iIdx], mLoadType[iIdx]);
            sub_631640();

            if (!mCurSaveFile || (unsigned int)mSaveDevice.StartDeleteFile(mLoadFilename[iIdx], mLoadType[iIdx], 0) >= 2)
            {
                SetNextState(30);
            }
        }
        return;

    case 0x1E:
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save23", 400);
        SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.ok", 100);
        mBtns = 1;
        mBtnDefault = 1;
        mLoadSavePopupAnswer = -1;
        ShowLoadSavePopup(true);
        return;

    case 0x1F:
        {

            int iCnt = mLoadQueueCnt;
            if (iCnt)
            {
                switch (mLoadOp[iCnt - 1])
                {
                case 0: SetNextState(23); break;
                case 1: SetNextState(14); break;
                case 2:
                case 3: SetNextState(29); break;
                default: return;
                }
            }
            else
            {
                SetNextState(3);
            }
        }
        return;

    default:
        return;
    }
}

// FUNCTION: SWBF2 0x006319e0
void SaveFsm::ExitState()
{
}

// FUNCTION: SWBF2 0x006319f0
void SaveFsm::UpdateState(float fDeltaTime)
{
    switch (mCurState)
    {
    case 6:    Update_InitialMemcardCheck_CheckProfile(); break;
    case 7:    Update_InitialMemcardCheck_Error();        break;
    case 9:    Update_PreOp_Error();                      break;
    case 0xA:  Update_PreOp_ConfirmDashboard();           break;
    case 0xB:  Update_PreOp_Filelist();                   break;
    case 0xC:  Update_PreOp_Format_Prompt();              break;
    case 0xD:  Update_PreOp_Format();                     break;
    case 0xF:  Update_Save_Overwrite();                   break;
    case 0x10: Update_Save_Create();                      break;
    case 0x11: Update_Save_Open();                        break;
    case 0x12: Update_Save_WriteChecksum();                break;
    case 0x13: Update_Save_WriteData();                    break;
    case 0x14: Update_Save_Close();                        break;
    case 0x15: Update_Save_Error();                        break;
    case 0x16: Update_Save_TooManyFiles();                 break;
    case 0x18: Update_Load_Open();                          break;
    case 0x19: Update_Load_ReadChecksum();                  break;
    case 0x1A: Update_Load_ReadData();                      break;
    case 0x1B: Update_Load_Close();                        break;
    case 0x1C: Update_Load_Error();                         break;
    case 0x1D: Update_Delete();                             break;
    case 0x1E: Update_Delete_Error();                       break;
    default:   return;
    }
}

// FUNCTION: SWBF2 0x00631dd0
void SaveFsm::Enter_PreOp()
{
    SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save19", 400);
    mBtns = 0;
    ShowLoadSavePopup(true);

    int iResult = mSaveDevice.GetStatus();
    if (iResult == -2)
    {
        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save04", (int)mFailOpStr);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
        return;
    }
    if (iResult == -4)
    {
        if (!m_preop_AllowFormat)
        {
            SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save05", (int)mFailOpStr);
            SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
            mBtns = 2;
            mBtnDefault = 2;
            SetNextState(9);
        }
        else
        {
            SetNextState(0xC);
        }
        return;
    }

    if (m_preop_CheckFreeBytes)
    {
        int iNeeded = (int)SaveDevice::BytesToBlocks(m_preop_CheckFreeBytes);
        unsigned int uiNeeded = 3;
        if (iNeeded > 3)
        {
            uiNeeded = iNeeded;
        }
        unsigned int uiFree = mSaveDevice.GetFreeSpace();
        if (uiFree < uiNeeded)
        {
            mXBOXAdditionalFreeNeeded = uiNeeded - uiFree;
            mXBOXTotalFreeNeeded = 3 * (uiNeeded >> 10);
            if (!m_preop_UseIMCMsg)
            {

                SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save06", mFailOpStr, mXBOXAdditionalFreeNeeded);
                SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
                mBtns = 2;
                mBtnDefault = 2;
            }
            else
            {
                SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save02", uiNeeded - uiFree);
                SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.freemoreblocks", 100);
                SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.continue", 100);
                mBtns = 3;
                mBtnDefault = 2;
            }
            SetNextState(9);
            return;
        }
    }

    if (m_preop_NoFileList)
    {
        SetNextState(3);
    }
    else if (mSaveDevice.StartRefreshFileList())
    {
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save08", 400);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
    }
    else
    {
        SetNextState(0xB);
    }
}

// FUNCTION: SWBF2 0x00631c00
void SaveFsm::Update_InitialMemcardCheck_CheckProfile()
{
    int iResult = mSaveDevice.CycleRefreshFileList();
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save09", 400);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(7);
        return;
    }

    mSaveDevice.ResetFileIterator();
    void* pSlot = GetNextSaveFile(&mSaveDevice);
    bool bFlag1 = false, bFlag2 = false, bFlag3 = false;
    while (pSlot)
    {
        if (((SaveFile*)pSlot)->GetType() == 1)
        {
            bFlag1 = true;
        }
        if (((SaveFile*)pSlot)->GetType() == 2)
        {
            bFlag2 = true;
        }
        if (((SaveFile*)pSlot)->GetType() == 3)
        {
            bFlag3 = true;
        }
        pSlot = GetNextSaveFile(&mSaveDevice);
    }

    if (bFlag1 && bFlag2 && bFlag3)
    {
        SetNextState(3);
    }
    else
    {
        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save02", mXBOXAdditionalFreeNeeded);
        SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.freemoreblocks", 100);
        SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.continue", 100);
        mBtns = 3;
        mBtnDefault = 1;
        mErrorSuccess = true;
        SetNextState(7);
    }
}

// FUNCTION: SWBF2 0x00631d80
void SaveFsm::Update_InitialMemcardCheck_Error()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (mLoadSavePopupAnswer == 1)
    {
        SetNextState(5);
    }
    else if (mErrorSuccess)
    {
        SetNextState(3);
    }
    else
    {
        SetNextState(4);
    }
}

// FUNCTION: SWBF2 0x00631ff0
void SaveFsm::Update_PreOp_Error()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    SetNextState(mLoadSavePopupAnswer == 1 ? 8 : 4);
}

// FUNCTION: SWBF2 0x00632040
void SaveFsm::Update_PreOp_ConfirmDashboard()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (m_preop_UseIMCMsg)
    {
        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save02", mXBOXAdditionalFreeNeeded);
        SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.continue", 100);
    }
    else
    {

        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save06", mFailOpStr, mXBOXAdditionalFreeNeeded);
        SafeLocalizeStrcpy(mBtnB, "ifs.loadsave_pc.cancel", 100);
    }
    SafeLocalizeStrcpy(mBtnA, "ifs.loadsave_pc.freemoreblocks", 100);
    mBtns = 3;
    mBtnDefault = 2;
    SetNextState(9);
}

// FUNCTION: SWBF2 0x006320f0
void SaveFsm::Update_PreOp_Filelist()
{
    int iResult = mSaveDevice.CycleRefreshFileList();
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save09", 400);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
        return;
    }

    unsigned int uiFreeFiles = mSaveDevice.GetFreeFiles();
    unsigned int uiCheckFiles = m_preop_CheckFreeFiles;
    if (uiCheckFiles && uiFreeFiles < uiCheckFiles)
    {
        mXBOXTotalFreeNeeded = uiCheckFiles - uiFreeFiles;
        SafeLocalizeStrcpy(mMsgStr1, "ifs.loadsave_pc.save07", 400);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
        return;
    }

    SetNextState(3);
}

// FUNCTION: SWBF2 0x006321e0
void SaveFsm::Update_PreOp_Format_Prompt()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (mLoadSavePopupAnswer == 1)
    {
        SetNextState(8);
    }
    else if (mLoadSavePopupAnswer == 2)
    {
        SetNextState(4);
    }
    else
    {
        SetNextState(0xD);
    }
}

// FUNCTION: SWBF2 0x00632310
void SaveFsm::Update_PreOp_Format()
{
    int iResult = mSaveDevice.CycleFormat();
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {

        const unsigned short* pJoy = RedLocalizeDB::Find(0x7CF9A572u, false);
        SafeLocalizeSPrintf(mMsgStr1, 400, "ifs.loadsave_pc.save13", (int)pJoy);
        SafeLocalizeStrcpy(mBtnB, "common.ok", 100);
        mBtns = 2;
        mBtnDefault = 2;
        SetNextState(9);
    }
    else
    {
        SetNextState(8);
    }
}

// FUNCTION: SWBF2 0x00632440
void SaveFsm::Update_Save_Overwrite()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (mLoadSavePopupAnswer == 1)
    {
        SetNextState(0x11);
    }
    else
    {
        SetNextState(4);
        mLoadQueueCnt = 0;
    }
}

// FUNCTION: SWBF2 0x00632500
void SaveFsm::Update_Save_Create()
{
    int iResult = mSaveDevice.CycleCreateFile(mLoadFilename[mLoadQueueCnt - 1], mLoadType[mLoadQueueCnt - 1]);
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        SetNextState(iResult == -6 ? 22 : 21);
    }
    else
    {
        SetNextState(0x11);
    }
}

// FUNCTION: SWBF2 0x006325c0
void SaveFsm::Update_Save_Open()
{
    int iResult = mSaveDevice.CycleOpenFile(*mCurSaveFile, false);
    if (iResult == 1)
    {
        return;
    }
    if (!iResult)
    {
        SetNextState(0x12);
    }
    else
    {
        mSaveErrorAfterClose = true;
        SetNextState(0x14);
    }
}

// FUNCTION: SWBF2 0x00632660
void SaveFsm::Update_Save_WriteChecksum()
{
    int iResult = mSaveDevice.CycleWriteFile(*mCurSaveFile, &mChecksum, CRC_SIZE);
    if (iResult == 1)
    {
        return;
    }
    if (!iResult)
    {
        SetNextState(0x13);
    }
    else
    {
        mSaveErrorAfterClose = true;
        SetNextState(0x14);
    }
}

// FUNCTION: SWBF2 0x006326d0
void SaveFsm::Update_Save_WriteData()
{
    int iResult = mSaveDevice.CycleWriteFile(*mCurSaveFile, mLoadDest[mLoadQueueCnt - 1], mLoadSize[mLoadQueueCnt - 1]);
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        mSaveErrorAfterClose = true;
    }
    SetNextState(0x14);
}

// FUNCTION: SWBF2 0x00632720
void SaveFsm::Update_Save_Close()
{
    int iResult = mSaveDevice.CycleCloseFile(*mCurSaveFile, false);
    if (iResult == 1)
    {
        return;
    }
    if (iResult || mSaveErrorAfterClose)
    {
        SetNextState(21);
    }
    else
    {
        ShowLoadSavePopup(false);
        int iIdx = mLoadQueueCnt - 1;

        PblUnicode::ustrncnv((unsigned short*)mLoadFilename[iIdx], "", 66);
        mLoadDest[iIdx] = 0;
        --mLoadQueueCnt;
        SetNextState(0x1F);
    }
}

// FUNCTION: SWBF2 0x006327e0
void SaveFsm::Update_Save_Error()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (mLoadSavePopupAnswer == 1)
    {
        SetNextState(0xE);
    }
    else
    {
        SetNextState(4);
        mLoadQueueCnt = 0;
    }
}

// FUNCTION: SWBF2 0x00632870
void SaveFsm::Update_Save_TooManyFiles()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    if (mLoadSavePopupAnswer == 1)
    {
        SetNextState(4);
    }
    else
    {
        SetNextState(4);
        mLoadQueueCnt = 0;
    }
}

// FUNCTION: SWBF2 0x00632960
void SaveFsm::Update_Load_Open()
{
    int iResult = mSaveDevice.CycleOpenFile(*mCurSaveFile, false);
    if (iResult == 1)
    {
        return;
    }
    SetNextState(iResult ? 28 : 25);
}

// FUNCTION: SWBF2 0x006329c0
void SaveFsm::Update_Load_ReadChecksum()
{
    int iResult = mSaveDevice.CycleReadFile(*mCurSaveFile, &mChecksum, CRC_SIZE);
    if (iResult == 1)
    {
        return;
    }
    if (!iResult)
    {
        SetNextState(26);
    }
    else
    {
        mLoadErrorAfterClose = true;
        SetNextState(27);
    }
}

// FUNCTION: SWBF2 0x00632a30
void SaveFsm::Update_Load_ReadData()
{
    int iResult = mSaveDevice.CycleReadFile(*mCurSaveFile, mLoadDest[mLoadQueueCnt - 1], mLoadSize[mLoadQueueCnt - 1]);
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        mLoadErrorAfterClose = true;
        SetNextState(27);
    }
    else
    {
        int iIdx = mLoadQueueCnt - 1;
        void* pDest = mLoadDest[iIdx];
        unsigned int uiSize = mLoadSize[iIdx];
        PblJournal* pJournal = PblJournal::Get((PblJournalType)0);
        pJournal->RecordLow(pDest, (int)uiSize);
        SetNextState(27);
    }
}

// FUNCTION: SWBF2 0x00632ac0
void SaveFsm::Update_Load_Close()
{
    int iResult = mSaveDevice.CycleCloseFile(*mCurSaveFile, false);
    if (iResult == 1)
    {
        return;
    }
    if (iResult || mLoadErrorAfterClose)
    {
        SetNextState(28);
    }
    else
    {
        ShowLoadSavePopup(false);
        int iIdx = mLoadQueueCnt - 1;

        PblUnicode::ustrncnv((unsigned short*)mLoadFilename[iIdx], "", 66);
        mLoadDest[iIdx] = 0;
        --mLoadQueueCnt;
        SetNextState(0x1F);
    }
}

// FUNCTION: SWBF2 0x00632b80
void SaveFsm::Update_Load_Error()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    SetNextState(4);
    mLoadQueueCnt = 0;
}

// FUNCTION: SWBF2 0x00632c70
void SaveFsm::Update_Delete()
{
    int iResult = mSaveDevice.CycleDeleteFile(mLoadFilename[mLoadQueueCnt - 1], mLoadType[mLoadQueueCnt - 1]);
    if (iResult == 1)
    {
        return;
    }
    if (iResult)
    {
        SetNextState(30);
    }
    else
    {
        ShowLoadSavePopup(false);
        int iIdx = mLoadQueueCnt - 1;

        PblUnicode::ustrncnv((unsigned short*)mLoadFilename[iIdx], "", 66);
        mLoadDest[iIdx] = 0;
        --mLoadQueueCnt;
        SetNextState(0x1F);
    }
}

// FUNCTION: SWBF2 0x00632d30
void SaveFsm::Update_Delete_Error()
{
    if (mLoadSavePopupAnswer == -1)
    {
        return;
    }
    ShowLoadSavePopup(false);
    SetNextState(4);
    mLoadQueueCnt = 0;
}

// FUNCTION: SWBF2 0x00632dc0
void SaveFsm::ShowLoadSavePopup(bool bSetup)
{

    RedInterfaceElement* pTitle = RedInterfaceElement::Find(0x9E227C3Fu);
    RedInterfaceElement* pBtnA = RedInterfaceElement::Find(0x4DB5F949u);
    RedInterfaceElement* pBtnB = RedInterfaceElement::Find(0x597FBABCu);
    RedInterfaceElement* pBtnC = RedInterfaceElement::Find(0x3C76CD7Bu);

    if (bSetup)
    {
        ((RedTextElement*)pBtnA)->SetText((unsigned short const*)mBtnA, true);
        ((RedTextElement*)pBtnB)->SetText((unsigned short const*)mBtnB, true);
        ((RedTextElement*)pBtnC)->SetText((unsigned short const*)mBtnC, true);
        pTitle->Enable(true);
        pBtnA->Enable((mBtns & 1) != 0);
        pBtnB->Enable((mBtns >> 1 & 1) != 0);
        pBtnC->Enable((mBtns >> 2 & 1) != 0);
    }

    LuaHelper::CallGlobalProc("Popup_LoadSave2_ResizeButtons");
    LuaHelper::CallGlobalProc("Popup_LoadSave2_SelectButton", (float)mBtnDefault);
    RedInterfaceElement* pPopup = RedInterfaceElement::Find(0xE65C8183u);
    pPopup->Enable(bSetup);
    LuaHelper::CallGlobalProc("gPopup_Loadsave2_fnSetTitleStr", mMsgStr1);
}

// FUNCTION: SWBF2 0x006315a0
unsigned short* SafeLocalizeStrcpy(unsigned short* pusDest, const char* pcSrc, unsigned int uiSize)
{
    const unsigned short* pusText = RedLocalizeDB::Find(PblHash(pcSrc), false);
    if (pusText)
    {
        wcscpy((wchar_t*)pusDest, (const wchar_t*)pusText);
        return pusDest;
    }
    PblUnicode::ustrncnv(pusDest, pcSrc, uiSize);
    return 0;
}

// FUNCTION: SWBF2 0x00631750
bool SaveFsm::GetSuccess() const
{
    return mCurState == 3;
}

// FUNCTION: SWBF2 0x00631630
void* GetNextSaveFile(void* pDevice)
{
    return ((SaveDevice*)pDevice)->GetNextFile();
}

// FUNCTION: SWBF2 0x00631640
void sub_631640()
{
}
