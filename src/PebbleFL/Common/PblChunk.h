#pragma once

#include <stddef.h>

class PblFile;

class PblFileChunk
{
public:

    PblFileChunk(PblFile* pFile);

    PblFileChunk ReadNextChild();

    unsigned int PeekNextChildId() const;

    void GetChunkData(void* pBuffer);

    char* ReadString(char* pcBuffer, int iBufSize);

    void Realign();

    bool HasMoreChildren() const
    {
        return _iNext + ((-_iNext) & 3) < _iEnd;
    }

    int GetSize() const { return _iSize; }

    unsigned int _iId;
    int _iSize;
    int _iEnd;
    int _iNext;
    PblFile* _pFile;

private:
    void _ReadHeader();
};
 
 

class PblMemChunk
{
public:
    PblMemChunk(const void* pData);
    PblMemChunk(const PblMemChunk& other);
    virtual ~PblMemChunk();

    // SYNTHETIC: SWBF2 0x00578fb0
    // PblMemChunk::`scalar deleting destructor'

    char Read8();
    short Read16();
    int Read32();
    float ReadFloat();
    void ReadData(void* pBuffer, int iNumBytes);
    char* ReadString(char* pcBuffer, int iBufSize);
    PblMemChunk ReadNextChild();
    int GetSize() const;
    void Realign();

    unsigned int GetId() const { return _iId; }
    const char* GetData() const { return _pData; }
    bool IsDone() const { return _pNext + ((-(int)_pNext) & 3) >= _pEnd; }

    unsigned int _iId;
    int _iSize;
    char* _pNext;
    char* _pEnd;
    char* _pData;

private:
    void _ReadHeader();
};
 
 
