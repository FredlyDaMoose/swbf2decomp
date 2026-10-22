#pragma once

#include "SoundFL/source/sndfile.h"

class RedFileAsync
{
public:
    // FUNCTION: SWBF2 0x00726c00
    RedFileAsync();

    ~RedFileAsync();

    bool Open(const char* const pFileName, unsigned int uiPriority, bool bBuffer);

    // FUNCTION: SWBF2 0x00726c30
    void Close();

    bool FileOperationComplete();
    unsigned int GetPos() const;
    void Read(void* pBuffer, unsigned int uiSize);
    void Seek(unsigned int uiPos);

    static void __cdecl FileOpComplete(Snd::File* pFile, void* pData, bool bResult);

    Snd::File mFile;
    bool mFileOpComplete;
    bool mOpenRetVal;
};

 
