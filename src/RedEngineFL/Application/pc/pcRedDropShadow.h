#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/Renderable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcRedShader;

// VTABLE: SWBF2 0x00783d38
class DropShadowRenderable : public Renderable
{
public:

    struct _DropShadow
    {
        PblVector3 vtxpos[4];
        RedColor color;
    };

    DropShadowRenderable();

    virtual ~DropShadowRenderable();

    void AddShadow(unsigned int uiTexture, const PblVector3& vPos,
                   const PblVector3& vNormal, const PblVector3& vDir,
                   float fStart, float fEnd, float, float fAlpha);

    virtual void Render(int value);

    unsigned int m_uiTexture;
    pcRedShader* m_pShader;
    _DropShadow mShadowsList[32];
    int mShadowCount;
    float mShadowOffset;
    float mPosOffset;
    float mShadowSize;
};
 
 
 
 
 
 
 
 
 

class RedDropShadowManager
{
public:

    static void Init();

    static void Cleanup();

    static void Render(unsigned int uiFlags);

    static void AddShadow(unsigned int uiTexture, const PblVector3& vPos,
                          const PblVector3& vNormal, const PblVector3& vDir,
                          float fStart, float fEnd, float, float fAlpha);

    static DropShadowRenderable* _Instance;
};
