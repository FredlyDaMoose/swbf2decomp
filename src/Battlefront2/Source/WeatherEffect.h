#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblTimeBase.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedRegion.h"

class FLEffectClass;
class PrecipitationData;
class RedRegionDesc;
class pcRedShader;

// VTABLE: SWBF2 0x007b1d90
class FogCloudEffect : public FLEffect
{
public:

    FogCloudEffect();

    virtual ~FogCloudEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    void PlatformInit();
    void PlatformCleanup();

    void SetTexture(unsigned int uiTexture) { TEXTURE_HASH = uiTexture; }
    void SetVelocity(const PblVector3& vVelocity) { VELOCITY = vVelocity; }
    void SetRotationRate(float fRate) { ROTATION_RATE = fRate; }
    void SetParticleDensity(int iDensity) { PARTICLES_PER_BOX = iDensity; }
    void SetParticleSize(float fSize) { PARTICLE_SIZE = fSize; }
    void SetParticleHeight(float fHeight) { HEIGHT_ABOVE_GROUND = fHeight; }
    void SetRange(float fNear, float fFar)
    {
        DISTANCE_NEAR = fNear;
        DISTANCE_FAR = fFar;
    }
    void SetColor(const RedColor& color) { COLOR = color; }

    static FogCloudEffect* s_pInstance;

    static FogCloudEffect* Instance() { return s_pInstance; }
    const PblVector3& GetVelocity() const { return VELOCITY; }

    PblTimeBase::Stamp _TimeStamp;
    pcRedShader* m_pShader;
    unsigned int TEXTURE_HASH;
    float ROTATION_RATE;
    PblVector3 VELOCITY;
    float BOX_SIZE;
    float BOX_SIZEINV;
    unsigned int PARTICLES_PER_BOX;
    float PARTICLE_SIZE;
    float HEIGHT_ABOVE_GROUND;
    float DISTANCE_NEAR;
    float DISTANCE_FAR;
    RedColor COLOR;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 struct PrecipitationData
{
    PblVector3 vPosRelative;
    PblVector3 vVelocityLocal;
    float fAlpha;
    float fRotationRate;
    float fRotation;
};
 

enum PRECIPITATION_TYPE
{
    PRECIPITATION_QUADS = 0,
    PRECIPITATION_STREAKS = 1
};

// VTABLE: SWBF2 0x007b1dc4
// VTABLE: SWBF2 0x007b1d80
class PrecipitationEffect : public FLEffect, public RedRegionFactory
{
public:
    class RainShadowRegion;

    PrecipitationEffect();

    virtual ~PrecipitationEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);
    virtual void* CreateRegion(RedRegionDesc& desc, const char* pcName) const;

    static void Create();

    static void Destroy();

    void PlatformCleanup();

    void PlatformInit();

private:

    void CreatePrecipitationData();

public:

    static PrecipitationEffect* s_pInstance;

    pcRedShader* m_pShader;
    float BOX_SIZE;
    float BOX_SIZEINV;
    unsigned int TEXTURE_HASH;
    PRECIPITATION_TYPE PARTICLE_TYPE;
    float PARTICLE_SIZE;
    float RENDER_DISTANCE;
    RedColor COLOR;
    float ALPHA_MIN;
    float ALPHA_MAX;
    float PARTICLES_PER_BOX;
    float PARTICLES_PER_BOX_RANGE;
    float VELOCITY;
    float VELOCITY_RANGE;
    float STREAK_TIME;
    float CAMERA_CROSSVELOCITY_SCALE;
    float CAMERA_AXIALVELOCITY_SCALE;
    float ROTATION_RATE_MEAN;
    float ROTATION_RATE_RANGE;
    float PROPERTY_DRIFT_RATE;
    float m_fDriftTime;
    float m_fParticleDensity;
    unsigned int MAX_PARTICLES_PER_BOX;
    PrecipitationData* m_pData;
    FLEffectClass* mGroundEffect;
    float mNumGroundEffectsPerSec;
    float mGroundEffectSpread;
    float mGroundEffectTimer[1];
    PblList<RainShadowRegion> m_RainShadows;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
  class PrecipitationEffect::RainShadowRegion : public RedRegion
{
public:

    RainShadowRegion(RedRegionDesc& desc) : RedRegion(desc), m_bVisible(false) {}

    ~RainShadowRegion() {}

    bool IsVisible() const { return m_bVisible; }

    void UpdateVisibility(const PblBox& box);

    void UpdateVisibility(const RedCamera* pCamera);

    PblList<RainShadowRegion>::Node m_RainShadowNode;
    bool m_bVisible;
};
 
 
 

// VTABLE: SWBF2 0x007b1df8
class WindEffect : public FLEffect
{
public:

    WindEffect();

    virtual ~WindEffect();
    virtual void Render(int value);
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);

    static void Create();

    static void Destroy();

    static WindEffect* s_pInstance;

    static WindEffect* Instance() { return s_pInstance; }
    const PblVector3& GetVelocity() const { return m_vVelocity; }

    PblVector3 m_vBaseVelocity;
    float m_fVelocityRange;
    float m_fVelocityChangeRate;
    float m_fTimer;
    PblVector3 m_vVelocityPrevTarget;
    PblVector3 m_vVelocityTarget;
    PblVector3 m_vVelocity;
};
 
 
 
 
 
 
 
 
 