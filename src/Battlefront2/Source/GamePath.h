#pragma once

class PblFileChunk;

#include <stddef.h>
#include "RedEngineFL/Application/RedPath.h"
#include "PebbleFL/Common/PblList.h"

class GamePath;

class GamePathFactory : public PblList<GamePathFactory>::Node
{
public:

    static void Read(PblFileChunk& chunk);

    static void ClearFactoryList();

    // SYNTHETIC: SWBF2 0x00537b70
    // GamePathFactory::`scalar deleting destructor'

protected:

    GamePathFactory(unsigned int uiClassHash);

    virtual ~GamePathFactory();

    virtual GamePath* CreatePath() const;

    static const GamePathFactory* Find(unsigned int uiClassHash);

public:
    unsigned int mClassHash;

    static PblList<GamePathFactory> sList;
};
 
 

// VTABLE: SWBF2 0x0079b5c0
class GamePath : public RedPath
{
public:
    virtual ~GamePath();
    virtual bool Read(PblConfig& config);
};
 
