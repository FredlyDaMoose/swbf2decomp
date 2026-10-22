#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"

class PblMatrix;
class PblFileChunk;
class RedCamera;

class PropObject
{
public:
    PropObject();

    virtual void Render(const RedCamera* pCamera, float, unsigned int) = 0;
    virtual float GetDist() = 0;
    virtual void Release() = 0;
    virtual PblVector3 GetPos();
    virtual const PblMatrix& GetMatrix();

    static void ClearObjectCaches();

protected:
    virtual ~PropObject();

public:
    PblList<PropObject>::Node mNode;
};
 
 

class PropModel
{
public:
    PropModel();
    virtual ~PropModel();

    virtual PropObject* CreateObject(float, float, float) = 0;
    virtual void CreateCollisionObject(float, float, float);

protected:

    void ReadData(unsigned int uiHash, int iVersion, PblFileChunk& chunk);

public:

    float mMaxDist;
    float mFrequency;
    float mStiffness;
};
 
 
