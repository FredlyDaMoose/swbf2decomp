#pragma once

#include <stddef.h>

#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblHashTable.h"

class PblFileChunk;
class PblMatrix;
class PblVector3;
class RedColor;
class RedModel;
class RedPose;
class RedShadingPose;
class RedSkeleton;

class GameModel
{
public:

    GameModel();

    virtual void Render(unsigned char, float, const PblMatrix *, const RedPose *, const RedColor &, unsigned int, RedShadingPose *) const;

    bool _Read(PblFileChunk& chunk);

    static void Read(PblFileChunk& chunk);

    static void PrintDebugInfo(const RedModel* pModel, const PblVector3& vPos);

    static void Reset();

    static const GameModel* Find(unsigned int uHash);

    const RedModel* GetRedModel(unsigned char lod) const { return _mLod[lod]; }

    unsigned int _uiNameHash;
    unsigned int _iLodType;
    float _fLodBias;
    unsigned int _iLodCost[4];
    RedModel* _mModel;
    RedModel* _mLod[4];
    RedLodData _mLodData;
    RedSkeleton* _pSkeleton;
    unsigned int mClothCount;
    unsigned int mClothList[8];

    static PblHashTable<GameModel, 1024> _HashTable;
};
 
 
 
 
 
 
 
