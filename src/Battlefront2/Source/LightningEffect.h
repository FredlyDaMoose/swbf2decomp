#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "GameSound.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedMaterial.h"

// VTABLE: SWBF2 0x007a3ce4
class LightningEffect : public FLEffect
{
public:

    LightningEffect();

    virtual ~LightningEffect();
    virtual void Render(int value);
    virtual bool IsReflected();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static void PlaySound(GameSound& sound);

    static LightningEffect* s_pInstance;

    RedColorValue m_colorLightning;
    float m_fSunlightFadeMax;
    float m_fSkyDomeDarken;
    float m_fBrightnessMin;
    float m_fTimeToFade;
    float m_fTimeBetweenFlashesMin;
    float m_fTimeBetweenFlashesMax;
    float m_fTimeBetweenSubFlashesMin;
    float m_fTimeBetweenSubFlashesMax;
    int m_iNumSubFlashesMin;
    int m_iNumSubFlashesMax;
    float m_fHorizonAngleMin;
    float m_fHorizonAngleMax;
    float m_fTimer;
    float m_fTimeNextFlash;
    int m_iNumSubFlashesRemaining;
    PblVector3 m_vDir;
    float m_fBrightness;
    float m_fSunlightBrightness;
    bool m_bInitRestore;
    RedColorValue m_colorSunlight_Restore;
    RedColorValue m_colorAmbient_Restore;
    RedColorValue m_colorDirectionalAmbient_Restore[2];
    PblVector3 m_vSunlightDir_Restore;
    RedColor m_colorFog_Restore;
    GameSound m_soundCrack;
    GameSound m_soundSubCrack;
    float mBoltFrequencyMin;
    float mBoltFrequencyMax;
    float mBoltTimer;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 