#ifndef SWBF2_GRAPHICS_PC_PCDYNAMICSPRITE_H
#define SWBF2_GRAPHICS_PC_PCDYNAMICSPRITE_H

#include "RedEngineFL/Application/Renderable.h"

// VTABLE: SWBF2 0x007964d0
class pcDynamicSprite : public Renderable
{
public:

    // VTABLE: SWBF2 0x007964b0
    class ClearViewportsCallback : public Renderable
    {
    public:

        ClearViewportsCallback(pcDynamicSprite* pOwner)
            : Renderable(2), m_pOwner(pOwner)
        {
        }

        // FUNCTION: SWBF2 0x004118f0
        virtual ~ClearViewportsCallback()
        {
            Deactivate();
        }

        // SYNTHETIC: SWBF2 0x00411950
        // pcDynamicSprite::ClearViewportsCallback::`scalar deleting destructor'

        virtual void Render(int value);

        pcDynamicSprite* m_pOwner;
    };

    pcDynamicSprite();
    virtual ~pcDynamicSprite();

    // SYNTHETIC: SWBF2 0x00411a80
    // pcDynamicSprite::`scalar deleting destructor'

    virtual void Activate(int value);

    virtual void Deactivate();

    virtual void Render(int value);

    void ClearViewports();

    static void Init();

    static pcDynamicSprite s_Instance;

    int m_iNumViewports;
    int m_iNumViewportsX;

    float m_fViewportWidth;
    float m_fViewportHeight;

    int m_iNumTempBufferSprites;
    bool m_bViewportsLocked;

    ClearViewportsCallback m_ClearViewportsCallback;
};
 
 
 
 
 
 
 
 
 
 

  #endif
