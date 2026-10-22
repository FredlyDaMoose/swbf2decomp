#pragma once

#include <stddef.h>

#include "Battlefront2/Source/GrassPatch.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcRedPrimitive;
class pcRedShader;
class pcRedIndexBuffer;
class pcRedVertexBuffer;

// VTABLE: SWBF2 0x0079653c
class GrassPatchRenderable : public Renderable
{
public:

    struct CompressedTexCoord
    {
        short u, v;
    };
    struct TexCoord
    {
        float u, v;
    };

    GrassPatchRenderable(const GrassPatchClass* pClass);
    virtual ~GrassPatchRenderable();

    // SYNTHETIC: SWBF2 0x00418de0
    // GrassPatchRenderable::`scalar deleting destructor'

    virtual void Render(int value);

    void RenderInstance(const PblMatrix& matrix, const RedColor& color, unsigned int uiFlags) const;

    void Update();
    void Begin();
    void AddParticle(const GrassPatchClass::GrassParticle& particle, int iParticleIndex,
                     const GrassPatchClass* pClass);

    float m_fDistanceFarShader;
    pcRedPrimitive* m_pPrimitive;
    pcRedShader* m_pShader;
    pcRedPrimitive* m_pPrimitiveFar;
    pcRedShader* m_pShaderFar;
    pcRedIndexBuffer* m_pIBReverse;
    pcRedIndexBuffer* m_pIBForward;
    pcRedShader* m_pShaderTransparent;
    GrassPatchClass* m_pClass;
    PblHeap<int, float>* mHeap;
    pcRedVertexBuffer* m_pVB;
    unsigned int m_startIndexForward;
    unsigned int m_startIndexReverse;
    void* m_pVertices;
    PblVector3 m_vUp;
    PblVector3 m_avRight[10];
    PblVector3 m_vFlatFront;
    int m_iTexCoordType;
    bool m_bReflection;

    static const CompressedTexCoord s_CompressedTexCoords[3][6][4];
    static const TexCoord s_TexCoords[3][6][4];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
