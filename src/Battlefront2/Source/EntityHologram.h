#pragma once

#include <stddef.h>

#include "Entity.h"
#include "Thread.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class LightBeam;
class RedModel;
class RedOmniLight;

enum HOLOGRAM_TYPE
{
    TYPE_REAL = 0,
    TYPE_ICON_ALIGNED = 1,
    TYPE_ICON_BUILD = 2,
    NUM_TYPES = 3
};

// VTABLE: SWBF2 0x0079b7b4
class EntityHologramClass : public EntityClass
{
public:
    EntityHologramClass(unsigned int uHash);

    EntityHologramClass(EntityHologramClass* pParent, unsigned int uHash);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    static EntityHologramClass* Find(unsigned int uHash);

    static PblHash rttiHashEntityHologramClass;
    static char rttiNameEntityHologramClass[];

    static int sHologramClassTableNumEntries;
    static void Reset();
    static PblHashTable<EntityHologramClass, 32> sHologramClassTable;

    RedModel* m_pHoloModel;
    RedColor m_color;
    float m_fVisibleDistance;
    float m_fHeight;
    float m_fSize;
    float m_fBeamInitialWidth;
    float m_fBeamIntensity;
    float m_fLightIntensity;
    float m_fImageIntensity;
    float m_fFlareIntensity;
    float m_fLightRadius;
    float m_fRotateRate;
    float m_fPopRate;
    float m_fFlickerRate;
    float m_fFlickerAlphaMin;
    float m_fFlickerAlphaMax;
    float m_fFadeInTime;
    float m_fTurnOnDistance;
    int m_eType;
};
  
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x0079b820
class EntityHologram : public EntityEx, public RedSceneObject
{
public:

    // VTABLE: SWBF2 0x0079b7d8
    class MyThread : public Thread
    {
    public:
        virtual ~MyThread();
        virtual bool Update(float fDeltaTime);

        void SetOwner(EntityHologram* pOwner) { _pOwner = pOwner; }

        EntityHologram* _pOwner;
    };

    EntityHologram(EntityHologramClass* pClass, const EntityDesc& desc);

    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityHologram();
protected:
    virtual void Init();
    void Deinit();

public:
    virtual bool Update(float fDeltaTime);
    virtual void SetStoredMatrix(const PblMatrix& mMatrix);
    virtual void Eliminate();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);

    void SetTeam(int iTeam);

    int GetTeam() const { return m_iTeam; }
    void SetColor(const RedColor& color);
    void SetModel(const RedModel* pModel);
    void SetIntensity(float fIntensity);

    void TurnOnNow();

    void TurnOff();

    void TurnOn();

    PblMatrix m_WorldMatrix;
    RedColor m_color;
    LightBeam* m_pLightBeam;
    RedOmniLight* m_apOmniLight[1];
    RedModel* m_pHoloModel;
    float m_fRotateTimer;
    float m_fPopTimer;
    float m_fAspect;
    float m_fFlickerTimer;
    float m_fFadeInTimer;
    float m_fIntensity;
    float m_fTilt;
    float m_fCurrentIntensity;
    float m_fCurrentFadeInFactor;
    unsigned char m_uiHideInView;
    bool m_bTurnOn;
    bool m_bActive;
    int m_iTeam;
    MyThread _Thread;

    static PblHash rttiHashEntityHologram;
    static char rttiNameEntityHologram[];
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
