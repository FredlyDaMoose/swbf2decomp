#pragma once

#include "Fsm.h"
#include "Battlefront2/Platform/Pc/pcSaveDevice.h"
#include "Battlefront2/Platform/Pc/pcSaveFile.h"

enum SaveFsmQueueOp
{
    QOP_LOAD = 0,
    QOP_SAVE = 1,
    QOP_DELETE = 2,
    QOP_SILENTDELETE = 3
};

// VTABLE: SWBF2 0x007ae3a8
class SaveFsm : public Fsm
{
public:

    SaveFsm();

    void StartPreOp(bool bAllowFormat, unsigned int uiCheckFreeBytes,
                     unsigned int uiCheckFreeFiles, unsigned short const* pusFailOpStr,
                     bool bSilentFail, bool bNoFileList, bool bUseIMCMsg, bool bIsPreLoad);

    void EmptyQueue();

    void QueueFile(unsigned short const* pusFilename, void* pDest, unsigned int uiSize,
                    int iType, SaveFsmQueueOp eOp);

    bool InputAccept(int iAnswer);

    void ProcessQueuedFiles();

    SaveFile* GetNextFile();

protected:

    virtual void EnterState();

    virtual void ExitState();
    virtual void UpdateState(float fDeltaTime);

public:
    virtual bool IsDone() const;

protected:

    void Enter_PreOp();

    void ShowLoadSavePopup(bool bSetup);

    void Update_InitialMemcardCheck_CheckProfile();
    void Update_InitialMemcardCheck_Error();
    void Update_PreOp_Error();
    void Update_PreOp_ConfirmDashboard();
    void Update_PreOp_Filelist();
    void Update_PreOp_Format_Prompt();
    void Update_PreOp_Format();
    void Update_Save_Overwrite();
    void Update_Save_Create();
    void Update_Save_Open();
    void Update_Save_WriteChecksum();
    void Update_Save_WriteData();
    void Update_Save_Close();
    void Update_Save_Error();
    void Update_Save_TooManyFiles();

    void Update_Load_Open();
    void Update_Load_ReadChecksum();
    void Update_Load_ReadData();

    void Update_Load_Close();
    void Update_Load_Error();
    void Update_Delete();
    void Update_Delete_Error();

public:

    bool GetSuccess() const;

    void MarkMemoryCard();
    bool CheckMemoryCardMark();
    bool IsMemoryCardUnformatted();
    bool IsDeviceInsertted();
    void ResetFileIterator();

public:
    SaveDevice mSaveDevice;
    unsigned int mChecksum;
    SaveFile* mCurSaveFile;
    int mLoadQueueCnt;
    unsigned short mLoadFilename[5][66];

    unsigned int mLoadSize[5];
    void* mLoadDest[5];
    int mLoadType[5];
    SaveFsmQueueOp mLoadOp[5];
    unsigned short mMsgStr1[400];
    unsigned short mBtnA[100];
    unsigned short mBtnB[100];
    unsigned short mBtnC[100];
    unsigned char mBtns;

    unsigned char mBtnDefault;
    int mLoadSavePopupAnswer;
    int mXBOXTotalFreeNeeded;
    int mXBOXAdditionalFreeNeeded;
    bool mErrorSuccess;
    bool m_preop_AllowFormat;
    unsigned int m_preop_CheckFreeBytes;
    unsigned int m_preop_CheckFreeFiles;
    unsigned short mFailOpStr[64];
    bool m_preop_SilentFail;
    bool m_preop_NoFileList;
    bool m_preop_UseIMCMsg;
    bool m_preop_bIsPreLoad;
    bool mSaveErrorAfterClose;
    bool mLoadErrorAfterClose;

    static const int CRC_SIZE = 4;
    static const int MAX_LOAD_QUEUE = 5;
};
