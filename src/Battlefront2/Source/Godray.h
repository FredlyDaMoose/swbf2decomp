#pragma once

#include <stddef.h>
#include "Entity.h"
#include "FLEffect.h"
#include "PebbleFL/Common/PblRandom.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcRedShader;
class PblConfig;

struct GodrayDesc
{
    GodrayDesc() {}

    float fRadius;
    float fLifetimeMin;
    float fLifetimeMax;
    float fAlphaMin;
    float fAlphaMax;
    RedColor color;
};
 

// VTABLE: SWBF2 0x007a011c
class GodrayManager : public FLEffect
{
     
     

public:

    struct Godray
    {
        Godray() {}

        PblVector3 vStartPos;
        PblVector3 vEndPos;
        float fLength;
        unsigned int uiFrameLastVisible;
        float fRadius;
        float fLifetimeMin;
        float fLifetimeMax;
        float fAlphaMin;
        float fAlphaMax;
        RedColor color;
        Godray* pNext;
    };

    struct VisibleGodray
    {
        struct Ray
        {
            float fTime0;
            float fLifeTime;
            float fPosX;
            float fPosZ;
            float fRadius0;
            float fAlpha0;
            float fTexOffsetL0;
            float fTexOffsetW0;
            float fRadius;
            float fAlpha;
        };

        VisibleGodray() {}

        Godray* pGodray;
        float fTime0;
        float fAlphaAnimated;
        int iVisible;
        float fLengthClipped;
        bool bStartClipped;
        PblVector3 vStartPosClipped;
        PblVector3 vEndPosClipped;
        float fAlpha;
        Ray aRays[8];
        unsigned int uiNumRays;
    };

    GodrayManager();

    virtual ~GodrayManager();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static GodrayManager* s_pInstance;

    unsigned int AddGodray(const GodrayDesc& desc, const PblVector3& vPos);
    void RemoveGodray(unsigned int uiId);
    void UpdateGodrayPosition(unsigned int uiId, const PblVector3& vPos);

private:
    Godray* NewGodray();
    void DeleteGodray(Godray* pGodray, Godray* pPrev);

    bool ClipVisibleGodray(VisibleGodray* pVisible) const;
    bool AnimateVisibleGodray(VisibleGodray* pVisible, float fTime, float fDeltaTime);
    bool UpdateVisibleGodray(VisibleGodray* pVisible);
    Godray* ChooseNewGodray();
    VisibleGodray* CreateVisibleGodray(Godray* pGodray);

    void CreateRay(VisibleGodray* pVisible, unsigned int uiRay);
    void PlatformCleanup();
    void PlatformRender(unsigned int uiFlags) const;

public:

    Godray* m_aGodrays;
    unsigned int m_uiNumGodrays;
    Godray* m_pFirstFreeGodray;
    Godray* m_pFirstAllocatedGodray;
    unsigned int m_uiMaxGodrays;
    unsigned int m_uiMaxVisibleGodrays;
    unsigned int m_uiMaxRaysPerGodray;
    unsigned int m_uiMinRaysPerGodray;
    float m_fMaxViewRadius;
    float m_fFadeViewRadius;
    float m_fMaxGodrayLength;
    float m_fRadiusForMaxRays;
    unsigned int m_uiTextureHash;
    float m_fTexScaleL;
    float m_fTexScaleW;
    PblVector3 m_vTextureVelocity;
    float m_fTextureRandomWalkSpeed;
    VisibleGodray* m_aVisibleGodrays;
    unsigned int m_uiNumVisibleGodrays;
    unsigned int m_uiFrame;
    float m_fTime;
    float m_fTimeLastVisibleCheck;
    PblVector3 m_vLengthDir;
    float m_fOffsetAngle;
    RedColor m_color;
    PblRandom m_Rand;
    PblVector3 m_vWidthDir;
    float m_fTexOffsetAngle;
    float m_fTexOffsetW;
    float m_fTexOffsetL;
    bool m_bTowardCamera;
    float m_fNearDist;
    pcRedShader* m_pShader;

private:

    void PlatformInit();
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 // VTABLE: SWBF2 0x007a015c
// VTABLE: SWBF2 0x007a0150
class GodrayClass : public EntityClass, public FLEffectClass
{
public:
    GodrayClass(unsigned int uHash);

    GodrayClass(PblConfig& config);

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);
    virtual FLEffectObject* BuildEffect();

    void ReadConfig(PblConfig& config);

    float m_fLifetimeMin;
    float m_fLifetimeMax;
    float m_fRadius;
    float m_fAlphaMin;
    float m_fAlphaMax;
    RedColor m_color;
    int mNumGodRays;
    float mSpreadRadius;
};
   
 
 
 
 
 
 
 
 

class GodrayEffectObject : public FLEffectObject
{
    friend class GodrayClass;
     
public:
    virtual ~GodrayEffectObject();
    virtual bool IsEffectActive() const;
private:
    GodrayEffectObject(unsigned int hGodray) : m_hGodray(hGodray) {}
    virtual void UpdatedWorldMatrix();
public:
    unsigned int m_hGodray;
};
 
 
