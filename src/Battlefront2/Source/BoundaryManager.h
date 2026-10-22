#pragma once

#include <stddef.h>

#include "GameSoundStream.h"

class PblFileChunk;
class PblVector3;
class RedPath;

class Boundary
{
public:
    struct Edge
    {
        float x;
        float z;
        float nx;
        float nz;
    };

    Boundary(const RedPath* pPath);
    ~Boundary();

    bool IsInBounds(const PblVector3& vPos) const;

private:
    void AddEdge(int i, const PblVector3& vStart, const PblVector3& vEnd);

public:
    Boundary* mNext;
    int mCount;
    Edge* mEdges;
};
 
 

class BoundaryManager
{
public:

    static BoundaryManager* Instance() { return sInstance; }

    BoundaryManager();

    static void Create();
    static void Destroy();
    ~BoundaryManager();

    void Read(PblFileChunk& chunk);

    bool IsOutOfBounds(const PblVector3& vPos) const;

    void PlayOutOfBoundsVoiceOver(int iTeam);

    void SetOutOfBoundsVoiceOverID(int iTeam, const char* pName);

private:
    void AddPath(unsigned int uPathID);

public:
    Boundary* mBoundaries;
    GameSoundStream mOutOfBounds[2];

    static BoundaryManager* sInstance;
};
 
 
