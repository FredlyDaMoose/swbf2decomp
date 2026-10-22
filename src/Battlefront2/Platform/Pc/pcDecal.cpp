#include <string.h>

#include "Battlefront2/Source/Decal.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

// FUNCTION: SWBF2 0x00410960
void DecalManager::PlatformRender(unsigned int uFlags) const
{
    if (uFlags & 0x200000)
    {
        return;
    }

    for (PblList<DecalClass>::Itor itClass(m_decalClasses); itClass != m_decalClasses.End();
         ++itClass)
    {
        if (!itClass->m_totalFrameVertices)
        {
            continue;
        }

        pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
            D3DPT_TRIANGLESTRIP, 0x282, sizeof(Decal::DecalVertex), itClass->m_totalFrameVertices,
            itClass->m_totalFrameIndices + 2 * itClass->m_totalVisible, false);
        Decal::DecalVertex* pVertices = (Decal::DecalVertex*)pPrimitive->BeginVertexData();
        if (!pVertices)
        {
            return;
        }
        unsigned short* pIndices = pPrimitive->BeginIndexData();

        unsigned short uBase = 0;
        for (PblList<Decal>::Itor it(itClass->m_decals); it != itClass->m_decals.End(); ++it)
        {
            Decal* pDecal = it;
            if (pDecal->visibility == RedCamera::VISIBILITY_NONE || !(pDecal->fadeVal > 0.0f))
            {
                continue;
            }

            RedColor color;
            color.b = pDecal->color.b;
            color.g = pDecal->color.g;
            color.r = pDecal->color.r;
            color.a = (unsigned char)(int)((float)pDecal->color.a * pDecal->fadeVal);

            memcpy(pVertices, pDecal->vertices, pDecal->numVertices * sizeof(Decal::DecalVertex));
            for (unsigned int i = 0; i < pDecal->numVertices; ++i)
            {
                pVertices->color = color;
                ++pVertices;
            }

            *pIndices++ = uBase + pDecal->indices[0];
            for (unsigned int i = 0; i < pDecal->numIndices; ++i)
            {
                *pIndices++ = uBase + pDecal->indices[i];
            }
            *pIndices = pIndices[-1];
            ++pIndices;
            uBase += (unsigned short)pDecal->numVertices;
        }

        pPrimitive->EndVertexData();
        pPrimitive->EndIndexData();
        RedRenderer::pcRenderPrimitive(pPrimitive,
                                       itClass->m_pShader,
                                       gMatrixIdentity,
                                       RedColor::WHITE, 0x210, 0, 0, 0);
        pPrimitive->Release();
    }
}
