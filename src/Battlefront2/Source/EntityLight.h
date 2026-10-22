#pragma once

#include <stddef.h>
#include "Entity.h"
#include "FLEffect.h"
#include "LightBeam.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class RedLight;
class EntityLightClass;

class EntityLight : public Entity, public FLEffectObject
{
public:

    enum LightFlickerType
    {
        LightFlickerType_None = 0,
        LightFlickerType_Strobe = 1,
        LightFlickerType_StrobeRandom = 2,
        LightFlickerType_StrobeFade = 3,
        LightFlickerType_Pulse = 4,
        LightFlickerType_Flicker = 5,
        NumLightFlickerTypes = 6,
    };

    static void* operator new(unsigned int uiSize) { return sMemoryPool.Allocate(uiSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    EntityLight();
    EntityLight(const EntityLightClass* pClass, const EntityDesc& desc);
    virtual ~EntityLight();

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
protected:
    virtual void Init();
public:
    virtual void Eliminate();

    virtual bool Update(float fDeltaTime);
    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
protected:
    virtual void UpdatedWorldMatrix();

public:

    static unsigned int GetRtti() { return rttiHashEntityLight; }

    void SetColor(const RedColor& color);
    void SetOmniLightRadius(float fRadius);
    void SetHaloRadius(float fRadius);

    void SetFlareIntensity(float fIntensity);
    void TurnOn();
    void TurnOff();

protected:

    void UpdateColor();
    void RecalculateBeam();
    void Deinit();

public:
    RedLight* _light;
    ManagedLightBeam _LightBeam;
    bool m_bOn;
    RedColor _color;
    float _fBeamLength;
    float _fBeamWidthX;
    float _fBeamWidthZ;
    float _fBeamFadeLength;
    float _fLightFlickerTimer;
    float _fLightFlickerIntensity;
    EntityLightClass* _lightClass;

    static PblHash rttiHashEntityLight;
    static char rttiNameEntityLight[];
    static MemoryPool sMemoryPool;
};
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079c120
class EntityLightFXParser : public FLEffect
{
public:

    EntityLightFXParser()
        : FLEffect(0xB71E2FBEu, 3)
    {
    }

    virtual ~EntityLightFXParser();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static EntityLightFXParser* s_pInstance;
};
 
 // VTABLE: SWBF2 0x0079c088
// VTABLE: SWBF2 0x0079c0ac
class EntityLightClass : public EntityClass, public FLEffectClass
{
public:
    static EntityLight* Build(unsigned int uLightOdfNameHash);

    enum ENTITY_LIGHT_FLAGS
    {
        ENTITY_LIGHT_FLAG_STATIC = 1,
        ENTITY_LIGHT_FLAG_SHADOWCASTER = 2,
        ENTITY_LIGHT_FLAG_SPECULARCASTER = 4,
        ENTITY_LIGHT_FLAG_BIDIRECTIONAL = 8,
        ENTITY_LIGHT_FLAG_SYNCHRONIZE = 16,
        ENTITY_LIGHT_FLAG_UPDATEPARITY = 32,
        ENTITY_LIGHT_FLAG_TYPE_OMNI = 64,
        ENTITY_LIGHT_FLAG_TYPE_SPOT = 128,
        ENTITY_LIGHT_FLAGS_FORCE_DWORD = 0x7fffffff
    };

    EntityLightClass(unsigned int uHash);

    EntityLightClass(EntityLightClass* pParent, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual FLEffectObject* BuildEffect();

    static unsigned int GetRtti() { return rttiHashEntityLightClass; }
    static PblHash rttiHashEntityLightClass;
    static char rttiNameEntityLightClass[];

    static int sLightClassTableNumEntries;

    static void Reset();
    static PblHashTable<EntityLightClass, 32> sLightClassTable;

    RedColor _color;
    float _fBeamLength;
    float _fBeamWidthX;
    float _fBeamWidthZ;
    float _fBeamInitialWidthX;
    float _fBeamInitialWidthZ;
    float _fBeamFadeFactor;
    float _fBeamFadeLength;
    float _fBeamDrawDistance;
    float _fFlareIntensity;
    float _fBeamIntensity;
    unsigned int _uiName;
    unsigned int _uiFlags;
    int _eLightFlickerType;
    float _fLightFlickerPeriodOn;
    float _fLightFlickerPeriodOff;
    float _fLightFadePeriod;
    float _fOmniRadius;
    float _fSpotInnerConeAngle;
    float _fSpotOuterConeAngle;
    float _fRandom;
    float _fFrameRate;
    unsigned int _uiNumFrames;
    unsigned int* _puiProjectedTextures;
};
   
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
