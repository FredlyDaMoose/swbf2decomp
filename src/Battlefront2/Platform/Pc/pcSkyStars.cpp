#include "Battlefront2/Source/SkyStars.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

#include <string.h>

// FUNCTION: SWBF2 0x00428ef0
void SkyStars::_PlatformInit()
{
    m_pShader = pcRedShader::Create(0xe8651328u, 0x102, 0, 0, 0x40,
                                    RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x00428f30
void SkyStars::_PlatformCleanup()
{
    if (m_pShader)
    {
        m_pShader->Release();
        m_pShader = 0;
    }
}

// FUNCTION: SWBF2 0x00428f90
void SkyStars::_PlatformRenderField(const StarField& field, unsigned int,
                                    const PblMatrix** ppMatrices, int iNumMatrices) const
{
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_POINTLIST, 0x102, 16,
                                                         field.uiNumStars, 0, false);
    pPrimitive->m_bbox = PblBox(-1.5f, -0.5f, -1.5f, 1.5f, 1.5f, 1.5f);
    Star* pVertices = (Star*)pPrimitive->BeginVertexData();
    if (!pVertices)
    {
        return;
    }

    memcpy(pVertices, field.aStar, field.uiNumStars * sizeof(Star));
    if (m_fTwinkleFactor > 0.0f)
    {
        float fCycle = m_fTwinkleTimer * m_fTwinkleFrequency;
        unsigned int uCycle = (unsigned int)fCycle;
        float fFraction = fCycle - (float)uCycle;
        unsigned int uSeed0 = uCycle ^ 0xc71c2830u;
        unsigned int uSeed1 = (uCycle + 1) ^ 0xc71c2830u;
        for (unsigned int i = 0; i < field.uiNumStars; ++i)
        {
            uSeed0 = uSeed0 * 1664525u + 1013904223u;
            unsigned int uRand0 = uSeed0 >> 16;
            uSeed0 = uSeed0 * 1664525u + 1013904223u;
            uRand0 = (uRand0 | (uSeed0 & 0xffff01ffu)) >> 9 | 0x3f800000u;
            float fRand0;
            memcpy(&fRand0, &uRand0, sizeof(fRand0));

            uSeed1 = uSeed1 * 1664525u + 1013904223u;
            unsigned int uRand1 = uSeed1 >> 16;
            uSeed1 = uSeed1 * 1664525u + 1013904223u;
            uRand1 = (uRand1 | (uSeed1 & 0xffff01ffu)) >> 9 | 0x3f800000u;
            float fRand1;
            memcpy(&fRand1, &uRand1, sizeof(fRand1));

            float fRandom = (fRand0 - 1.0f) * (1.0f - fFraction)
                          + (fRand1 - 1.0f) * fFraction;
            float fDim = fRandom * m_fTwinkleFactor * (1.0f - field.aStar[i].vPos.y * 0.5f);
            pVertices[i].color.a = (unsigned char)(int)((1.0f - fDim) * pVertices[i].color.a);
        }
    }

    pPrimitive->EndVertexData();
    for (int i = 0; i < iNumMatrices; ++i)
    {
        RedRenderer::pcRenderPrimitive(pPrimitive,
                                       m_pShader,
                                       *ppMatrices[i],
                                       m_color,
                                       0x20211u, 0, 0, 0);
    }
    pPrimitive->Release();
}
