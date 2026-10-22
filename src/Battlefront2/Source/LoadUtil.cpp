#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include "GameMovie.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "LoadUtil.h"

#include "BoundaryManager.h"
#include "CollisionBody.h"
#include "Entity.h"
#include "FoleyFX.h"
#include "GamePath.h"
#include "HintManager.h"
#include "LoadDisplay.h"
#include "LuaScript.h"
#include "PathFinder.h"
#include "ProceduralAnimation.h"
#include "StringDB.h"
#include "TriggerSoundRegion.h"
#include "Weapon.h"
#include "ConnectivityGraph.h"
#include "Explosion.h"
#include "FLEffect.h"
#include "Ordnance.h"
#include "SoldierAnimatorClass.h"
#include "WeaponMelee.h"
#include "Cloth.h"
#include "CollisionMesh.h"
#include "Music.h"
#include "RedEngineFL/Util/RedAnimation.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/movie/RedMovie.h"
#include "RedEngineFL/Portals/Portal.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/RedRegion.h"
#include "RedEngineFL/Util/RedSceneManager.h"
#include "RedEngineFL/Util/RedStack.h"
#include "SoundFL/source/sndenginebase.h"
#include "SoundFL/source/sndsamplebankbase.h"

#include "RedEngineFL/Application/RedTerrain.h"

#include "PortalReader.h"

void PumpWaitingMessages();
extern HWND hGlobalWnd;

namespace DownloadableContent
{
bool IsDownloadableContent();
char const* GetContentDirectory();
}

namespace ConnectivityGraphManager
{
void ReadWorldGraph(PblFileChunk& chunk);
}

namespace LoadUtil
{
void ProcessHint(PblFileChunk& chunk);
void ProcessBarrier(PblFileChunk& chunk);
}

namespace GameLoop
{
void SetWorldFilename(char const* pcName);
void SetTerrainFilename(char const* pcName);
}

// GLOBAL: SWBF2 0x01faaa78

int sUnknownChunkSeen;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

namespace HUD { namespace Manager { void Load(PblFileChunk& chunk); } }
namespace PropGenerator { void Read(PblFileChunk& chunk); }
namespace SkyManager { void Read(PblFileChunk& chunk); }

extern bool sLoadNestedLevels;

// GLOBAL: SWBF2 0x01e56fa3
bool gLoading;

// GLOBAL: SWBF2 0x009a1a40
char RedStack::sMem[40960];

// GLOBAL: SWBF2 0x009aba40
int RedStack::sMemIdx;

pvs::PortalReader::~PortalReader()
{
}

// FUNCTION: SWBF2 0x00578ff0
void FileBlockCallback()
{
    PumpWaitingMessages();
    if (LoadDisplay::sInstance && GetForegroundWindow() == hGlobalWnd)
    {
        LoadDisplay::sInstance->Update();
    }
}

// FUNCTION: SWBF2 0x00579020
void LoadUtil::ProcessBin(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x454D414E)
        {
            char acName[256];
            child.ReadString(acName, sizeof(acName));
            PblHash hName(acName);
            (void)hName;
        }
    }
}

// FUNCTION: SWBF2 0x00579090
char const* LoadUtil::GetDataDirectory()
{
    return "data\\_lvl_pc\\";
}

// FUNCTION: SWBF2 0x005790a0
void LoadUtil::MakeFullName(char const* pName, FileType eType, char* pBuffer, int iBufferCount,
                            bool bAddPrefix)
{
    char acRoot[300];
    strlcpy(acRoot, "data\\", sizeof(acRoot) - 1);
    if (bAddPrefix)
    {
        _snprintf(acRoot, sizeof(acRoot), "%s\\Data\\", DownloadableContent::GetContentDirectory());
    }

    int iLength;
    switch (eType)
    {
    case FILE_TYPE_LEVEL:
        iLength = _snprintf(pBuffer, iBufferCount, "%s%s%s.%s", acRoot, "_lvl_pc\\", pName, "lvl");
        break;
    case FILE_TYPE_MOVIE:
        iLength = _snprintf(pBuffer, iBufferCount, "%s%s%s.%s", acRoot, "_lvl_pc\\movies\\", pName, "bik");
        break;
    case FILE_TYPE_NONE:
        iLength = _snprintf(pBuffer, iBufferCount, "%s%s%s", acRoot, "_lvl_pc\\", pName);
        break;
    default:
        return;
    }
    if (iLength < 0)
    {
        pBuffer[iBufferCount - 1] = 0;
    }
}

// FUNCTION: SWBF2 0x00579160
void LoadUtil::LoadSoundBank(PblFileChunk& chunk, char const* pName, void* pData)
{
    (void)pData;

    Snd::SampleBankBase* const pBank = Snd::SampleBankBase::LoadBank(chunk, pName, false, 0);
    if (pBank && pBank->mTemplate == 1)
    {
        PblFile file;
        const bool bOpened = file.Open("data\\_lvl_pc\\sound\\common.bnk", true, false, true);
        PblFileChunk commonChunk(&file);
        if (bOpened)
        {
            Snd::SampleBankBase::LoadBank(commonChunk, pName, false, pBank->mHashID);
            file.Close();
        }
    }
}

// FUNCTION: SWBF2 0x00579210
bool LoadUtil::ReadDataFileChunk(char const* pName, PblFileChunk& chunk,
                                 unsigned int const* puiIds, unsigned int uiIdCount,
                                 unsigned int const* puiHashes, unsigned int uiCount,
                                 char const** ppNames)
{
    unsigned int uiLvlsRead = 0;
    bool abFound[256];

    bool bAllLvls = false;
    if (uiCount == 1 && puiHashes[0] == 0x2F0C9F3D)
    {
        bAllLvls = true;
    }

    while (chunk.HasMoreChildren())
    {
        FileBlockCallback();
        PblFileChunk child = chunk.ReadNextChild();

        Snd::EngineBase::CallbackOverride aSoundOverrides[2] = {
            { 0x5E262256, LoadSoundBank, 0 },
            { 0x9A644D15, LoadSoundBank, 0 },
        };

        if (puiIds)
        {
            unsigned int i;
            for (i = 0; i < uiIdCount; ++i)
            {
                if (puiIds[i] == child._iId)
                {
                    break;
                }
            }
            if (i == uiIdCount)
            {
                continue;
            }
            if (i < 256)
            {
                abFound[i] = true;
            }
        }

        switch (child._iId)
        {
        case 0x48544C43:
            ClothData::Read(child);
            break;
        case 0x464C4E54:
        case 0x52444853:
            RedRenderer::Read(child);
            break;
        case 0x5E262256:
            Snd::EngineBase::LoadConfiguration(child, pName, true, aSoundOverrides, 2);
            break;
        case 0x5F4E4942:
            ProcessBin(child);
            break;
        case 0x5F5F7866:
            FLEffect::Read(child);
            break;
        case 0x5F61617A:
        {
            void* pData = RedAllocFromHeap(__RedTempHeap, child._iSize + 8, 0);
            child.GetChunkData(pData);
            PblMemChunk memChunk(pData);
            RedAnimation::ReadZaa(memChunk);
            RedFreeToHeap(__RedTempHeap, pData, 0);
            break;
        }
        case 0x5F646E62:
            BoundaryManager::Instance()->Read(child);
            break;
        case 0x5F646E73:
            Snd::EngineBase::LoadConfiguration(child, 0, true, 0, 0);
            break;
        case 0x5F647568:
            HUD::Manager::Load(child);
            break;
        case 0x5F66617A:
        {
            void* pData = RedAllocFromHeap(__RedTempHeap, child._iSize + 8, 0);
            child.GetChunkData(pData);
            PblMemChunk memChunk(pData);
            RedAnimation::ReadZaf(memChunk);
            RedFreeToHeap(__RedTempHeap, pData, 0);
            break;
        }
        case 0x5F6C766C:
            while (child.HasMoreChildren())
            {
                PblFileChunk lvl = child.ReadNextChild();
                bool bRead = bAllLvls;
                if (uiCount && !bAllLvls)
                {
                    for (unsigned int i = 0; i < uiCount; ++i)
                    {
                        bRead = puiHashes[i] == lvl._iId;
                        if (bRead)
                        {
                            if (i < 256)
                            {
                                abFound[i] = true;
                            }
                            break;
                        }
                    }
                }
                if (bRead)
                {
                    ++uiLvlsRead;
                    sLoadNestedLevels = false;
                    const bool bOk = ReadDataFileChunk(pName, lvl, puiIds, uiIdCount, 0, 0, ppNames);
                    sLoadNestedLevels = true;
                    if (!bOk)
                    {
                        return false;
                    }
                }
            }
            break;
        case 0x5F707270:
            PropGenerator::Read(child);
            break;
        case 0x5F726373:
            LuaScriptFactory::Read(child);
            break;
        case 0x5F727374:
            TriggerSoundRegion::Properties::Load(child);
            break;
        case 0x5F73756D:
            Music::Load(child);
            break;
        case 0x5F786574:
            RedTexture::Read(child);
            break;
        case 0x5F786666:
            FoleyFX::Load(child);
            break;
        case 0x5F796B73:
            SkyManager::Read(child);
            break;
        case 0x626D6F63:
            WeaponMeleeClass::ReadCombo(child);
            break;
        case 0x6364726F:
            OrdnanceClass::Read(child);
            break;
        case 0x63707865:
            ExplosionClass::Read(child);
            break;
        case 0x636E7077:
            WeaponClass::Read(child);
            break;
        case 0x63746E65:
            EntityClass::Read(child);
            break;
        case 0x646C7277:
            ProcessWorld(child);
            break;
        case 0x646F6D67:
            GameModel::Read(child);
            break;
        case 0x6766636D:
            if (GameMovie::sUseMovies)
            {
                RedMovie::Load(child, false);
            }
            break;
        case 0x68746170:
            GamePathFactory::Read(child);
            break;
        case 0x6C636F4C:
            RedLocalizeDB::Read(child);
            break;
        case 0x6C646F6D:
            RedModel::Read(child);
            break;
        case 0x6C656B73:
            RedSkeleton::Read(child);
            break;
        case 0x6C6C6F63:
            CollisionMesh::Read(child);
            break;
        case 0x6D697270:
            CollisionBodyPrimitive::Read(child);
            break;
        case 0x6D6E6173:
            SoldierAnimatorClass::Read(child);
            break;
        case 0x6E616C70:
            ConnectivityGraphManager::ReadWorldGraph(child);
            break;
        case 0x6E726574:
            RedTerrain::Read(child);
            break;
        case 0x706E6C70:
            ConnectivityGraphManager::ReadWorldPatch(child);
            break;
        case 0x7468676C:
            RedLight::Read(child);
            break;
        case 0x746E6F66:
            RedFont::Read(child);
            break;
        case 0x74726F70:
        {
            RedSceneManager::Read(child);
            pvs::IManager& manager = pvs::IManager::GetInstance();
            PblConfig config(child);
            pvs::PortalReader reader(manager);
            reader.Read(manager, config);
            break;
        }
        }
    }

    return uiLvlsRead >= uiCount;
}

// FUNCTION: SWBF2 0x00579930
bool LoadUtil::ReadDataFileOnHeap(char const* pName, int iHeap,
                                  unsigned int const* puiIds, unsigned int uiIdCount,
                                  unsigned int const* puiHashes, unsigned int uiCount,
                                  char const** ppNames)
{
    PblFile file;
    RedStack stack;
    char* pcFullName = (char*)stack.Alloc(0x100);
    char* pcFileName = (char*)stack.Alloc(0x100);
    char* pcLvlName = (char*)stack.Alloc(0x100);
    pcLvlName[0] = '\0';
    unsigned int uiLvlHash = 0;
    gLoading = true;

    int i = 0;
    for (; pName[i] != '\0' && pName[i] != ';'; ++i)
    {
        pcFileName[i] = pName[i];
    }
    pcFileName[i] = '\0';
    if (pName[i++] == ';')
    {
        int j = 0;
        for (; pName[i + j] != '\0'; ++j)
        {
            pcLvlName[j] = pName[i + j];
        }
        pcLvlName[j] = '\0';
    }

    if (!_stricmp(pcFileName, "mission.lvl") && DownloadableContent::IsDownloadableContent())
    {
        strlcpy(pcFullName, DownloadableContent::GetContentDirectory(), 0x100);
        strlcat(pcFullName, "\\Data\\_lvl_pc\\", 0x100);
        strlcat(pcFullName, pcFileName, 0x100);
    }
    else if (!_strnicmp(pcFileName, "dc:", 3))
    {
        strlcpy(pcFullName, DownloadableContent::GetContentDirectory(), 0x100);
        strlcat(pcFullName, "\\Data\\_lvl_pc\\", 0x100);
        strlcat(pcFullName, pcFileName + 3, 0x100);
    }
    else if (!_strnicmp(pcFileName, "raw:", 4))
    {
        strlcpy(pcFullName, pcFileName, 0x100);
    }
    else
    {
        strlcpy(pcFullName, "data\\_lvl_pc\\", 0x100);
        strlcat(pcFullName, pcFileName, 0x100);
    }

    int iOldHeap = 0;
    if (iHeap != -1)
    {
        iOldHeap = RedSetCurrentHeap(iHeap);
    }
    PblFile::SetBlockCallback(FileBlockCallback);
    bool bOpened = file.Open(pcFullName, true, false, true);
    if (iHeap != -1)
    {
        RedSetCurrentHeap(iOldHeap);
    }

    if (!bOpened)
    {
        gLoading = false;
        PblFile::SetBlockCallback(0);
        return false;
    }

    if ((!uiCount || !puiHashes) && pcLvlName[0] != '\0')
    {
        uiLvlHash = PblHash(pcLvlName);
    }

    PblFileChunk chunk(&file);
    if (uiLvlHash)
    {
        ReadDataFileChunk(pcFullName, chunk, puiIds, uiIdCount, &uiLvlHash, 1, ppNames);
    }
    else
    {
        ReadDataFileChunk(pcFullName, chunk, puiIds, uiIdCount, puiHashes, uiCount, ppNames);
    }

    if (iHeap != -1)
    {
        iOldHeap = RedSetCurrentHeap(iHeap);
    }
    file.Close();
    if (iHeap != -1)
    {
        RedSetCurrentHeap(iOldHeap);
    }

    PblFile::SetBlockCallback(0);
    gLoading = false;
    return true;
}

// FUNCTION: SWBF2 0x00579c30
bool LoadUtil::ReadDataFile(char const* pName, void* pUnused, unsigned int const* puiHashes,
                            unsigned int uiCount, char const** ppNames)
{
    (void)pUnused;
    return ReadDataFileOnHeap(pName, __RedTempHeap, 0, 0, puiHashes, uiCount, ppNames);
}

// FUNCTION: SWBF2 0x0057aa60
bool LoadUtil::ReadDataFile(PblFile& file, void* pUnused, unsigned int const* puiHashes,
                            unsigned int uiCount, char const** ppNames)
{
    (void)pUnused;
    PblFile::SetBlockCallback(FileBlockCallback);
    PblFileChunk chunk(&file);
    ReadDataFileChunk("Memory File", chunk, 0, 0, puiHashes, uiCount, ppNames);
    PblFile::SetBlockCallback(0);
    return true;
}

static void ReadTransform(PblFile* pFile, PblMatrix& mMatrix)
{
    for (int iRow = 0; iRow < 4; iRow++)
    {
        const float fX = pFile->ReadFloat();
        const float fY = pFile->ReadFloat();
        const float fZ = pFile->ReadFloat();
        mMatrix.m[iRow][0] = fX;
        mMatrix.m[iRow][1] = fY;
        mMatrix.m[iRow][2] = fZ;
        mMatrix.m[iRow][3] = (iRow == 3) ? 1.0f : 0.0f;
    }
}

// FUNCTION: SWBF2 0x00579c50
void LoadUtil::ProcessWorld(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        char acName[260];
        switch (child._iId)
        {
        case 0x454D414E:
            child.ReadString(acName, 256);
            GameLoop::SetWorldFilename(acName);
            break;
        case 0x4D414E54:
            child.ReadString(acName, 256);
            GameLoop::SetTerrainFilename(acName);
            break;
        case 0x52524142:
            ProcessBarrier(child);
            break;
        case 0x676D6E61:
            PAAnimationGroup::Read(child);
            break;
        case 0x686D6E61:
            PAAnimationGroup::ReadHierarchy(child);
            break;
        case 0x6D696E61:
            PAAnimation::Read(child);
            break;
        case 0x6E676572:
            ProcessRegion(child);
            break;
        case 0x746E6948:
            ProcessHint(child);
            break;
        case 0x74736E69:
            ProcessEntity(child);
            break;
        default:
            sUnknownChunkSeen = 1;
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00579d80
Entity* LoadUtil::ProcessEntityInfo(PblFileChunk& chunk)
{
    EntityDesc desc;
    desc.mMatrix = gMatrixIdentity;
    EntityClass* pClass = 0;
    char acName[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x454D414E:
            desc.mNameId = PblHash(child.ReadString(acName, sizeof(acName)));
            StringDB::Store(desc.mNameId, acName);
            break;
        case 0x45505954:
            pClass = Factory<Entity, EntityClass, EntityDesc>::Find(
                PblHash(child.ReadString(acName, sizeof(acName))));
            break;
        case 0x4D524658:
            ReadTransform(child._pFile, desc.mMatrix);
            break;
        }
    }

    if (!pClass)
    {
        return 0;
    }
    return pClass->Build(desc);
}

// FUNCTION: SWBF2 0x0057a000
void LoadUtil::ProcessEntity(PblFileChunk& chunk)
{
    Entity* pEntity = 0;
    char acValue[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            pEntity = ProcessEntityInfo(child);
            if (!pEntity)
            {
                return;
            }
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pEntity->SetProperty(uProperty, child.ReadString(acValue, sizeof(acValue)));
            break;
        }
        }
    }
    pEntity->Init();
}

// FUNCTION: SWBF2 0x0057a0a0
RedRegion* LoadUtil::ProcessRegionInfo(PblFileChunk& chunk)
{
    RedRegionDesc desc;
    desc.mType = PblRegion::type_invalid;
    char acName[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x45505954:
            desc.mTypeId = PblHash(child.ReadString(acName, sizeof(acName)));
            break;
        case 0x454D414E:
            desc.mNameId = PblHash(child.ReadString(acName, sizeof(acName)));
            StringDB::Store(desc.mNameId, acName);
            break;
        case 0x455A4953:
        {
            desc.mSize.x = child._pFile->ReadFloat();
            desc.mSize.y = child._pFile->ReadFloat();
            desc.mSize.z = child._pFile->ReadFloat();
            desc.mSize.w = sqrtf(desc.mSize.x * desc.mSize.x + desc.mSize.y * desc.mSize.y +
                                 desc.mSize.z * desc.mSize.z);
            break;
        }
        case 0x4D524658:
            ReadTransform(child._pFile, desc.mMatrix);
            break;
        }
    }

    const RedRegionFactory* pFactory = RedRegionFactory::Find(acName);
    void* pRegion = pFactory->CreateRegion(desc, acName);
    if (pFactory == RedRegionFactory::GetDefault())
    {
        return (RedRegion*)(pRegion != 0);
    }
    return (RedRegion*)pRegion;
}

// FUNCTION: SWBF2 0x0057a360
void LoadUtil::ProcessRegion(PblFileChunk& chunk)
{
    RedRegion* pRegion = 0;
    char acValue[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            pRegion = ProcessRegionInfo(child);
            break;
        case 0x504F5250:
            if ((unsigned int)pRegion > 1)
            {
                const unsigned int uProperty = child._pFile->Read32();
                pRegion->SetProperty(uProperty, child.ReadString(acValue, sizeof(acValue)));
            }
            break;
        }
    }
}

bool __cdecl IsCoverHintFilteringEnabled();

void* BaseHint::operator new(unsigned int uSize)
{
    return sMemoryPool.Allocate(uSize);
}

namespace LoadUtil {

// FUNCTION: SWBF2 0x0057a3e0
BaseHint* ProcessHintInfo(PblFileChunk& chunk)
{
    HintDesc desc;
    unsigned int uType = 0;
    char acName[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x454D414E:
            desc.mNameId = PblHash(child.ReadString(acName, sizeof(acName)));
            break;
        case 0x45505954:
            uType = PblHash(child.ReadString(acName, sizeof(acName)));
            break;
        case 0x4D524658:
            ReadTransform(child._pFile, desc.mMatrix);
            break;
        }
    }

    switch (uType)
    {
    case 0x350CA8AF:
        return new SnipeHint(desc);
    case 0x340CA71C:
        return new PatrolHint(desc);
    case 0x370CABD5:
        return new CoverHint(desc);
    case 0x310CA263:
        return new JetJumpHint(desc);
    case 0x300CA0D0:
        return new MinefieldHint(desc);
    case 0x330CA589:
        return new LandHint(desc);
    case 0x320CA3F6:
        return new FortificationHint(desc);
    case 0x3D0CB547:
        return new VehicleCoverHint(desc);
    }
    return 0;
}
}

// FUNCTION: SWBF2 0x0057a6e0
void LoadUtil::ProcessHint(PblFileChunk& chunk)
{
    BaseHint* pHint = 0;
    char acValue[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            pHint = LoadUtil::ProcessHintInfo(child);
            if (!pHint)
            {
                return;
            }
            break;
        case 0x504F5250:
        {
            const unsigned int uProperty = child._pFile->Read32();
            pHint->SetProperty(uProperty, child.ReadString(acValue, sizeof(acValue)));
            break;
        }
        }
    }

    if (pHint)
    {
        if (!IsCoverHintFilteringEnabled() || pHint->GetType() != BaseHint::HINT_COVER)
        {
            HintManager::AddHint(pHint);
        }
    }
}

namespace LoadUtil {

// FUNCTION: SWBF2 0x0057a790
void ProcessBarrierInfo(PblFileChunk& chunk)
{
    PblMatrix mMatrix = gMatrixIdentity;
    PblVector4 vSize;
    vSize.x = vSize.y = vSize.z = vSize.w = 0.0f;
    int iFlag = 0;
    char acName[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x47414C46:
            iFlag = child._pFile->Read32();
            break;
        case 0x454D414E:
            child.ReadString(acName, sizeof(acName));
            break;
        case 0x455A4953:
        {
            const float fX = child._pFile->ReadFloat();
            const float fY = child._pFile->ReadFloat();
            const float fZ = child._pFile->ReadFloat();
            const float fLength = sqrtf(fY * fY + fX * fX + fZ * fZ);
            vSize.x = fX;
            vSize.y = fY;
            vSize.z = fZ;
            vSize.w = fLength;
            break;
        }
        case 0x4D524658:
            ReadTransform(child._pFile, mMatrix);
            mMatrix.m[3][2] = -mMatrix.m[3][2];
            break;
        }
    }

    PathFinder::AddObstacle(mMatrix, vSize, iFlag, PblHash(acName));
}
}

// FUNCTION: SWBF2 0x0057a9e0
void LoadUtil::ProcessBarrier(PblFileChunk& chunk)
{
    char acValue[128];

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x4F464E49:
            LoadUtil::ProcessBarrierInfo(child);
            break;
        case 0x504F5250:
            child._pFile->Read32();
            child.ReadString(acValue, sizeof(acValue));
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00446c00
template EntityClass* Factory<Entity, EntityClass, EntityDesc>::Find(unsigned int);

// TEMPLATE: SWBF2 0x00578f10
// util::HashTable_<pvs::ISector *,char const *,util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > > >::~HashTable_<pvs::ISector *,char const *,util::Vector<pvs::ISector *,util::BaseArray<pvs::ISector *,util::TypedArrayAllocator<pvs::ISector *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<char const *,util::BaseArray<char const *,util::TypedArrayAllocator<char const *,util::TNewArrayAlloc<pvs::ISector *> > > >,util::Vector<unsigned int,util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<pvs::ISector *> > > > >
template util::HashTable_<pvs::ISector*, char const*, util::HashTable<pvs::ISector*, char const*, util::TNewArrayAlloc<pvs::ISector*> >::ElemVecT, util::HashTable<pvs::ISector*, char const*, util::TNewArrayAlloc<pvs::ISector*> >::KeyVecT, util::HashTable<pvs::ISector*, char const*, util::TNewArrayAlloc<pvs::ISector*> >::HashVecT>::~HashTable_();
