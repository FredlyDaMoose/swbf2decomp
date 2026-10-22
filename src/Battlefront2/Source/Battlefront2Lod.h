#pragma once

#include <stddef.h>

class RedLodData
{
public:

    RedLodData()
    {
        _LodBitmask = 0;
        mLowRezFlags = 0;
        mMaxDist = 1000.0f;
    }

    int GetCost(int iLod) const { return _LodCost[iLod]; }
    bool HasLod(int iLod) const { return (_LodBitmask & (1 << iLod)) != 0; }

    int GetLodClass() const { return _LodClass; }
    int HasLowRes() const { return mLowRezFlags & 1; }
    int CrossFadeLowRes() const { return mLowRezFlags & 2; }
    int HasTransparentSegment() const { return mLowRezFlags & 4; }

    int _LodCost[4];
    int _LodBitmask;
    int _LodClass;
    unsigned char mLowRezFlags;
    float mMaxDist;
};
 
 
 
 
 
 

class OrdnanceLodData : public RedLodData
{
public:
    OrdnanceLodData();
};
 

class NoLodData : public RedLodData
{
public:
    NoLodData();
};
 

class SmallEffectLodData : public RedLodData
{
public:
    SmallEffectLodData();
};
class MediumEffectLodData : public RedLodData
{
public:
    MediumEffectLodData();
};
class LargeEffectLodData : public RedLodData
{
public:
    LargeEffectLodData();
};
extern LargeEffectLodData largeEffectLodData;
 

class RedModel;

class ModelLodData : public RedLodData
{
public:
    ModelLodData(int iLodType, const RedModel* pLod0, const RedModel* pLod1,
                 const RedModel* pLod2, const RedModel* pLod3);
};
 

class SoldierLodData : public RedLodData
{
public:
    SoldierLodData(const RedModel* pModel, const RedModel* pLowResModel);
};
