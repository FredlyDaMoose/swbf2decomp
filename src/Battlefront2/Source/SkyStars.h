#pragma once

#include <stddef.h>

#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblVector.h"
#include "Thread.h"

class PblConfig;
class PblMatrix;
class pcRedShader;
 

// VTABLE: SWBF2 0x007ae670
class SkyStars : public Thread
{
     
     

public:

    struct Star
    {

        Star() {}

        PblVector3 vPos;
        RedColor color;
    };

    struct StarField
    {
        unsigned int uiNumStars;
        Star* aStar;
    };

    static const int NUM_STAR_FIELDS = 36;

    SkyStars();
    virtual ~SkyStars();

    virtual bool Update(float fDeltaTime);

    void Read(PblConfig& config);
    void Render(unsigned int uiFlags) const;

private:
    void _BuildStarFields();
    RedColor _GetStarColor(float fBrightness, float fColor) const;

    void _PlatformInit();
    void _PlatformCleanup();
    void _PlatformRenderField(const StarField& field, unsigned int uiFlags,
                              const PblMatrix** ppMatrices, int iNumMatrices) const;

public:
    StarField m_aStarFields[NUM_STAR_FIELDS];
    unsigned int m_uiNumStars;
    unsigned int m_uiRandomSeed;
    float m_fBrightStarRatio;
    float m_fMinBrightness;
    float m_fColorSaturation;
    RedColor m_color;
    float m_fTwinkleFactor;
    float m_fTwinkleFrequency;
    float m_fTwinkleTimer;
    float m_fSinAngleMin;
    float m_fSinAngleFade;
    bool m_bRenderBottom;
    pcRedShader* m_pShader;
};

 
 
 
 
 
 
 
 
