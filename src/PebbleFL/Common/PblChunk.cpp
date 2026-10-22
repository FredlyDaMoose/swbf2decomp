#include <string.h>
#include <math.h>

#include "PblFile.h"
#include "PblChunk.h"
#include "PblVector.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

// FUNCTION: SWBF2 0x0072a8a0
PblFileChunk::PblFileChunk(PblFile* pFile)
{
    _pFile = pFile;
    _ReadHeader();
    _iNext = _pFile->Tell();
}

// FUNCTION: SWBF2 0x0072ab70
void PblFileChunk::_ReadHeader()
{
    Realign();
    _iId = _pFile->Read32();
    _iSize = _pFile->Read32();
    _iEnd = _pFile->Tell() + _iSize;
}

// FUNCTION: SWBF2 0x0072a900
void PblFileChunk::GetChunkData(void* pBuffer)
{
    unsigned int* puiOut = (unsigned int*)pBuffer;
    puiOut[0] = _iId;
    puiOut[1] = _iSize;
    _pFile->ReadData(puiOut + 2, _iSize);
}

// FUNCTION: SWBF2 0x0072a930
unsigned int PblFileChunk::PeekNextChildId() const
{
    int iHere = _pFile->Tell();
    _pFile->Seek(_iNext, PblFile::SeekFromStart);
    _pFile->SeekAlign(4);
    unsigned int uiId = _pFile->Read32();
    _pFile->Seek(iHere, PblFile::SeekFromStart);
    return uiId;
}

// FUNCTION: SWBF2 0x0072aa30
PblFileChunk PblFileChunk::ReadNextChild()
{
    _pFile->Seek(_iNext, PblFile::SeekFromStart);
    Realign();
    PblFileChunk Chunk(_pFile);
    _iNext = _pFile->Tell() + Chunk._iSize;
    return Chunk;
}

// FUNCTION: SWBF2 0x0072aad0
char* PblFileChunk::ReadString(char* pcBuffer, int iBufSize)
{
    char* pcOut = pcBuffer;
    char cChar = _pFile->Read8();
    while (cChar != '\0')
    {
        if (iBufSize == 0)
        {
            return 0;
        }
        *pcOut++ = cChar;
        --iBufSize;
        cChar = _pFile->Read8();
    }
    if (iBufSize <= 0)
    {
        return 0;
    }
    *pcOut = '\0';
    return pcBuffer;
}

// FUNCTION: SWBF2 0x0072a8d0
PblMemChunk::PblMemChunk(const void* pData)
{
    _pData = (char*)pData;
    _ReadHeader();
    _pNext = _pData;
}

// FUNCTION: SWBF2 0x0072abc0
void PblMemChunk::_ReadHeader()
{
    Realign();
    _pEnd = _pData + 8;
    _iId = Read32();
    _iSize = Read32();
    _pEnd = _pData + _iSize;
}

// FUNCTION: SWBF2 0x0072aa00
char PblMemChunk::Read8()
{
    return *_pData++;
}

// FUNCTION: SWBF2 0x0072a990
short PblMemChunk::Read16()
{
    int iValue = (unsigned char)Read8();
    iValue |= Read8() << 8;
    return (short)iValue;
}

// FUNCTION: SWBF2 0x0072a9c0
int PblMemChunk::Read32()
{
    int iValue = (unsigned char)Read8();
    iValue |= (unsigned char)Read8() << 8;
    iValue |= (unsigned char)Read8() << 16;
    iValue |= Read8() << 24;
    return iValue;
}

// FUNCTION: SWBF2 0x0072aa10
float PblMemChunk::ReadFloat()
{
    int iValue = Read32();
    return *(float*)&iValue;
}

// FUNCTION: SWBF2 0x0072ab20
char* PblMemChunk::ReadString(char* pcBuffer, int iBufSize)
{
    char* pcOut = pcBuffer;
    char cChar = Read8();
    while (cChar != '\0')
    {
        if (iBufSize == 0)
        {
            return 0;
        }
        *pcOut++ = cChar;
        --iBufSize;
        cChar = Read8();
    }
    if (iBufSize <= 0)
    {
        return 0;
    }
    *pcOut = '\0';
    return pcBuffer;
}

// FUNCTION: SWBF2 0x0072aa90
PblMemChunk PblMemChunk::ReadNextChild()
{
    _pData = _pNext;
    Realign();
    PblMemChunk Chunk(_pData);
    _pNext = _pData + Chunk._iSize + 8;
    return Chunk;
}

void PblFileChunk::Realign()
{
    _pFile->SeekAlign(4);
}

void PblMemChunk::Realign()
{
    _pData = _pData + ((-(int)_pData) & 3);
}

int PblMemChunk::GetSize() const
{
    return _iSize;
}

void PblMemChunk::ReadData(void* pBuffer, int iNumBytes)
{
    memcpy(pBuffer, _pData, iNumBytes);
    _pData += iNumBytes;
}

PblMemChunk::PblMemChunk(const PblMemChunk& other)
{
    _iId = other._iId;
    _iSize = other._iSize;
    _pNext = other._pNext;
    _pEnd = other._pEnd;
    _pData = other._pData;
}

// FUNCTION: SWBF2 0x00578fa0
PblMemChunk::~PblMemChunk()
{
}

// FUNCTION: SWBF2 0x00565060
void AdjustScreenGroupAspect(RedScreenGroupElement* pGroup)
{
    float fHeight = (float)RedInterfaceScreen::GetScreenHeight();
    float fHeight43 = (float)RedInterfaceScreen::GetScreenWidth() * 3.0f / 4.0f;
    if (fabsf(fHeight - fHeight43) >= 2.0f)
    {
        PblVector3 vPos = pGroup->GetPosition();
        if (fHeight43 > fHeight)
        {
            pGroup->SetMatrixPosScale(vPos.x, (fHeight43 - fHeight) * 0.5f, 1.0f,
                                      1.0f - (fHeight43 - fHeight) / fHeight);
        }
    }
}
