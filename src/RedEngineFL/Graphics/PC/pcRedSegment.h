#ifndef SWBF2_GRAPHICS_PC_PCSHADERSEGMENT_H
#define SWBF2_GRAPHICS_PC_PCSHADERSEGMENT_H

#include "RedEngineFL/Graphics/RedSegment.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

class pcRedPrimitive;
class pcRedShader;

// VTABLE: SWBF2 0x007858dc
class pcShaderSegment : public RedSegment
{
public:

    pcShaderSegment()
        : m_pPrimitive(0), m_pShader(0), m_uiSegmentRenderFlags(0)
    {
    }

    virtual ~pcShaderSegment();

    // SYNTHETIC: SWBF2 0x00717a40
    // pcShaderSegment::`scalar deleting destructor'

    virtual void Render(const PblMatrix* pWorld, const PblMatrix* pBoneMatrices,
                        int iNumBones, unsigned int uiFlags, const RedColor& color,
                        RedShadingState* pState);

    virtual unsigned int GetRenderTypeHash() const;

    virtual void Free();

    virtual RedShadingState* CreateShadingState() const;

    virtual bool IsTransparent() const;

    virtual bool IsLit() const;

    virtual bool IsVertexLit() const;

    pcRedPrimitive* m_pPrimitive;
    pcRedShader* m_pShader;
    unsigned int m_uiSegmentRenderFlags;
};

 

 #endif

#ifndef SWBF2_GRAPHICS_PC_PCSOFTWARESKINNEDSHADERSEGMENT_H
#define SWBF2_GRAPHICS_PC_PCSOFTWARESKINNEDSHADERSEGMENT_H

#include "RedEngineFL/Graphics/RedSegment.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblRef.h"

// VTABLE: SWBF2 0x00785904
class pcSoftwareSkinnedShaderSegment : public RedSegment
{
public:

    pcSoftwareSkinnedShaderSegment();

    virtual ~pcSoftwareSkinnedShaderSegment();

    // SYNTHETIC: SWBF2 0x00717ac0
    // pcSoftwareSkinnedShaderSegment::`scalar deleting destructor'

    virtual void Render(const PblMatrix* pWorld, const PblMatrix* pBoneMatrices,
                        int iNumBones, unsigned int uiFlags, const RedColor& color,
                        RedShadingState* pState);

    virtual unsigned int GetRenderTypeHash() const;

    virtual void Free();

    virtual RedShadingState* CreateShadingState() const;

    virtual bool IsTransparent() const;

    virtual bool IsLit() const;

    virtual bool IsVertexLit() const;

    pcRedPrimitive* _CreateSkinnedPrimitive(const PblMatrix* pBoneMatrices, int iNumBones,
                                            unsigned int count);

    unsigned char* m_pVertexData;
    unsigned char* m_puiSkinData;
    unsigned int m_uiSkinNumBones;
    unsigned int m_uiVertexSize;
    PblRef* m_pIndexBuffer;
    int m_PrimType;
    int m_uiStartIndex;
    int m_uiPrimCount;
    unsigned int m_uiNumVertices;
    int m_uiNumIndices;
    unsigned int m_uiVertexFormat;
    PblBox m_bbox;
    pcRedShader* m_pShader;
    unsigned int m_uiSegmentRenderFlags;
};
 
 
 

 #endif
