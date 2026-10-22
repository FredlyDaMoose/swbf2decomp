#ifndef SWBF2_APPLICATION_PC_PCREDTERRAIN_H
#define SWBF2_APPLICATION_PC_PCREDTERRAIN_H

#include "RedEngineFL/Application/Renderable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"

class pcRedPrimitive;
class pcRedIndexBuffer;
class pcTerrainShader;
class pcTerrainShader2;
class PblFileChunk;

// VTABLE: SWBF2 0x00773070
class TerrainRenderable : public Renderable
{
public:
    struct LowResPatch
    {
        pcRedPrimitive* pPrim;
        PblBox bbox;
    };

    struct Patch
    {
        unsigned int uiNumPasses;
        pcRedPrimitive** apPrim;
        pcTerrainShader** apShader;
        pcTerrainShader2** apShader2;
        short iHeightMin;
        short iHeightMax;
    };

    TerrainRenderable();
    virtual ~TerrainRenderable();

    virtual void Deactivate();

    virtual void Render(int value);

    void Deinit();
    void Read(PblFileChunk& chunk);

    void _Reset();

    void Init();
    void _SetQuality();
    void _InitPixelLighting(bool bEnable);

    void SetFarTerrainMaxDistance(float fDist) { m_fFarTerrainMaxDistance = fDist; }

    void SetBumpDetailTexture(unsigned int uiHash, float fScale, float fStrength)
    {
        m_fBumpLuminosityScale = fScale;
        m_bBumpTextureAvailable = uiHash != 0;
        m_uiBumpLuminosityTextureHash = uiHash;
        m_fStaticBumpFactor = fStrength;
    }

    void SetFarTerrainTexture(unsigned int uiHash, float fA, unsigned int uiB, float fC);

    static int _CalcPatchX(float fX);
    static int _CalcPatchZ(float fZ);

    void _RenderLowRes(unsigned int uiFlags) const;

    Patch* m_aPatches;
    LowResPatch* m_aLowResPatches;
    pcRedIndexBuffer* m_pIBDefault;
    pcTerrainShader* m_pFarShader;
    pcTerrainShader2* m_pFarShader2;
    bool m_bEnableFarDetail;
    float m_fFarTerrainMaxDistance;
    float m_fLowResMapSize;
    int m_iLowResPatchSize;
    int m_iLowResPatchesPerDimension;
    float m_fLowResPointsPerMeter;
    float m_fDetailMaxDistance;
    bool m_bEnablePixelLighting;
    bool m_bBumpTextureAvailable;
    unsigned int m_uiBumpLuminosityTextureHash;
    float m_fBumpLuminosityScale;
    float m_fStaticBumpFactor;
    pcTerrainShader* m_pPrePixelLightingShader;
    pcTerrainShader2* m_pPrePixelLightingShader2;
    QualityT m_eQuality;
    bool m_bEnableBumpMapping;
};

 
 
 
 
 extern TerrainRenderable _TerrainRenderable;

#endif
