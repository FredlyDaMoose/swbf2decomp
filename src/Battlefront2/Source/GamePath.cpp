#include "GamePath.h"

#include <stdio.h>
#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

// FUNCTION: SWBF2 0x004b99a0
bool GamePath::Read(PblConfig& config)
{
    return RedPath::Read(config);
}

// SYNTHETIC: SWBF2 0x004b99b0
// GamePath::`scalar deleting destructor'

GamePath::~GamePath()
{
}

PblList<GamePathFactory> GamePathFactory::sList;

// FUNCTION: SWBF2 0x00537b10
GamePathFactory::GamePathFactory(unsigned int uiClassHash)
    : mClassHash(uiClassHash)
{
    sList.AddLast(this, this);
}

// FUNCTION: SWBF2 0x00537ba0
GamePathFactory::~GamePathFactory()
{
    sList.Remove(this);
}

// FUNCTION: SWBF2 0x00537be0
GamePath* GamePathFactory::CreatePath() const
{
    return new GamePath;
}

// FUNCTION: SWBF2 0x00537c50
void GamePathFactory::ClearFactoryList()
{
    sList.RemoveAll();
}

// FUNCTION: SWBF2 0x00537ea0
template void PblList<GamePathFactory>::RemoveAll();

// FUNCTION: SWBF2 0x00537c60
void GamePathFactory::Read(PblFileChunk& chunk)
{
    PblConfig config(chunk);
    while (!config.IsDone())
    {
        PblConfig pathConfig(config, true);
        const PblFileChunk mark = chunk;
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() != 0x84874d36)
        {
            continue;
        }

        unsigned int uiHash = 0;
        char acType[256];
        acType[0] = 0;
        if (data.GetNumArgs() && data.GetStringArg(0))
        {
            strlcpy(acType, data.GetStringArg(0), 256);
            char* pSpace = strchr(acType, ' ');
            if (pSpace)
            {
                *pSpace = 0;
            }
            uiHash = PblHash(acType);
        }

        GamePathFactory defaultFactory(0);
        const GamePathFactory* pFactory = 0;
        if (uiHash)
        {
            pFactory = Find(uiHash);
            if (!pFactory)
            {
                char acBuffer[256];
                _snprintf(acBuffer, 256, "type_%s", acType);
                uiHash = PblHash(acBuffer);
                pFactory = Find(uiHash);
            }
        }
        if (!pFactory)
        {
            pFactory = &defaultFactory;
        }

        GamePath* pPath = pFactory->CreatePath();
        chunk = mark;
        if (pPath->Read(pathConfig))
        {
            RedPath::AddToPathList(pPath);
        }

        if (uiHash != 0x87499d36)
        {
            const char* pcName = (pFactory == &defaultFactory || !uiHash)
                                     ? acType
                                     : acType + strlen(acType) + 1;
            pPath->SetName(PblHash(pcName));
        }
    }
}

// FUNCTION: SWBF2 0x00537e70
const GamePathFactory* GamePathFactory::Find(unsigned int uiClassHash)
{
    for (PblList<GamePathFactory>::Itor it(sList); it; ++it)
    {
        if (it->mClassHash == uiClassHash)
        {
            return it;
        }
    }
    return 0;
}
