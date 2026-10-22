#pragma once

#include <stddef.h>

#include "Battlefront2/Source/LeafPatch.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

class pcRedPrimitive;
class pcRedShader;
class pcRedIndexBuffer;
class pcRedVertexBuffer;

// VTABLE: SWBF2 0x0079655c
class LeafPatchRenderable : public Renderable
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

    LeafPatchRenderable(const LeafPatchClass* pClass);
    virtual ~LeafPatchRenderable();

    // SYNTHETIC: SWBF2 0x0041b900
    // LeafPatchRenderable::`scalar deleting destructor'

    virtual void Render(int value);

    void RenderInstance(const PblMatrix& matrix, const RedColor& color, unsigned int uiFlags) const;

    void Update();
    void Begin();
    void AddParticle(const PblVector3& vPos, float fSize, const RedColor& color, float fRotation,
                     int iParticleIndex, unsigned char ucTexture);

    float m_fDistanceFarShader;
    pcRedPrimitive* m_pPrimitive;
    pcRedShader* m_pShader;
    pcRedPrimitive* m_pPrimitiveFar;
    pcRedShader* m_pShaderFar;
    pcRedIndexBuffer* m_pIBReverse;
    pcRedIndexBuffer* m_pIBForward;
    const LeafPatchClass* m_pClass;
    PblHeap<int, float>* mHeap;
    int m_iNumParticlesAllLODs;
    unsigned int m_startIndexForward;
    unsigned int m_startIndexReverse;
    pcRedVertexBuffer* m_pVB;
    void* m_pVertices;
    PblVector3 m_vUp;
    PblVector3 m_avRight[10];
    PblVector3 m_vFront;
    int m_iTexCoordType;
    bool m_bReflection;

    static const CompressedTexCoord s_CompressedTexCoords[2][4][4];
    static const TexCoord s_TexCoords[2][4][4];
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
