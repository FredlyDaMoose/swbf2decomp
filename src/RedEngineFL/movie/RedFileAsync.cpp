#include "RedFileAsync.h"
#include <windows.h>

// FUNCTION: SWBF2 0x00726c00
RedFileAsync::RedFileAsync()
{
    mFileOpComplete = true;
    mOpenRetVal = false;
    mFile.SetFileOpCompleteFunc(FileOpComplete, this);
}

// FUNCTION: SWBF2 0x00726c30
void RedFileAsync::Close()
{
    mFile.Close();
}

// FUNCTION: SWBF2 0x00726c60
bool RedFileAsync::FileOperationComplete()
{
    return mFileOpComplete;
}

// FUNCTION: SWBF2 0x00726c20
RedFileAsync::~RedFileAsync()
{
}

// FUNCTION: SWBF2 0x00726c40
void __cdecl RedFileAsync::FileOpComplete(Snd::File* pFile, void* pData, bool bResult)
{
    RedFileAsync* pThis = (RedFileAsync*)pData;
    pThis->mOpenRetVal = bResult;
    pThis->mFileOpComplete = true;
}

// FUNCTION: SWBF2 0x00726c80
bool RedFileAsync::Open(const char* const pFileName, unsigned int uiPriority, bool bBuffer)
{
    mFileOpComplete = false;
    mFile.SetPriority(uiPriority);
    mFile.Open(pFileName, 1, bBuffer ? 0x800 : 0, 0x800, 1);
    while (!mFileOpComplete)
    {
        Sleep(0);
    }
    return mOpenRetVal;
}
