#pragma once

#include <stddef.h>

class PblFile
{
public:

    enum SeekType
    {
        SeekFromHere = 0,
        SeekFromStart,
        SeekFromEnd
    };

    static void __cdecl SetBlockCallback(void(__cdecl* pFunc)(void));

    PblFile();
    ~PblFile();

    bool Open(const char* pcFileName, bool bAsynchronous, bool bOSBuffer, bool bBuffer);

    bool OpenCreate(const char* pcFileName, bool bAsynchronous, bool bOSBuffer, bool bBuffer);
    void Close();

    bool IsOpen() const { return _bIsOpen; }

    static void __cdecl Delete(const char* pcFileName);
    static bool __cdecl Exists(const char* pcFileName);

    int GetSize();
    int Tell() const;
    void Seek(int iOffset, SeekType eFrom);

    void SeekAlign(int iAlign);

    char Read8();
    short Read16();
    int Read32();
    float ReadFloat();
    void ReadData(void* pBuffer, int iNumBytes);

    void WriteData(void* pBuffer, int iNumBytes);
    void WriteString(const char* pcText);

protected:

    unsigned int MemGetSize() const;
    void MemRead(void* pBuffer, int iNumBytes);
    void MemWrite(void* pBuffer, int iNumBytes);
    unsigned int MemTell() const;
    void MemSeek(int iOffset, SeekType eFrom);

    bool MemOpenFile(void* pBuffer, unsigned int uiSize);
    void MemCloseFile();

public:

    bool OpenReadWrite(const char* pcFileName, bool bAsynchronous, bool bOSBuffer, bool bBuffer);
    void Write8(char cValue);
    void Write16(short sValue);
    void Write32(int iValue);
    void WriteFloat(float fValue);
    void Flush();
    void Suspend();
    void Resume(const char* pcFileName);
    bool OpenMemFile(void* pBuffer, unsigned int uiSize);
    void CloseMemFile();
    static int __cdecl GetSize(const char* pcFileName);

    char* _Buffer;
    unsigned int _iBufferSize;
    unsigned int _iPos;
    bool _bIsOpen;
    bool _bAsynchronous;
    bool _bOSBuffer;
    bool _bBuffer;
    bool _bIsMemFile;

    static void(__cdecl* _blockCallbackFunc)(void);
};
 
 
 
