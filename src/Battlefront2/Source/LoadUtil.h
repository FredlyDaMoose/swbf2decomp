#pragma once

class PblFile;
class PblFileChunk;
class Entity;
class RedRegion;
class BaseHint;

extern bool gLoading;

void FileBlockCallback();

namespace LoadUtil
{

enum FileType { FILE_TYPE_LEVEL = 0, FILE_TYPE_MOVIE = 1, FILE_TYPE_NONE = 2 };

bool ReadDataFile(char const* pName, void* pUnused, unsigned int const* puiHashes,
                  unsigned int uiCount, char const** ppNames);
bool ReadDataFile(PblFile& file, void* pUnused, unsigned int const* puiHashes,
                  unsigned int uiCount, char const** ppNames);

bool ReadDataFileOnHeap(char const* pName, int iHeap, unsigned int const* puiIds,
                        unsigned int uiIdCount, unsigned int const* puiHashes,
                        unsigned int uiCount, char const** ppNames);

bool ReadDataFileChunk(char const* pName, PblFileChunk& chunk, unsigned int const* puiIds,
                       unsigned int uiIdCount, unsigned int const* puiHashes,
                       unsigned int uiCount, char const** ppNames);

void MakeFullName(char const* pName, FileType eType, char* pBuffer, int iBufferCount,
                  bool bAddPrefix);

void LoadSoundBank(PblFileChunk& chunk, char const* pName, void* pData);

char const* GetDataDirectory();
void ProcessBin(PblFileChunk& chunk);
void ProcessWorld(PblFileChunk& chunk);
Entity* ProcessEntityInfo(PblFileChunk& chunk);
void ProcessEntity(PblFileChunk& chunk);
RedRegion* ProcessRegionInfo(PblFileChunk& chunk);
void ProcessRegion(PblFileChunk& chunk);
void ProcessHint(PblFileChunk& chunk);
void ProcessBarrier(PblFileChunk& chunk);
}

namespace LoadUtil { BaseHint* ProcessHintInfo(PblFileChunk& chunk); }
namespace LoadUtil { void ProcessBarrierInfo(PblFileChunk& chunk); }
