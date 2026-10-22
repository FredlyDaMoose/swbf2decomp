#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "LightningBoltEffect.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcRedPrimitive;
class pcShieldShader;
class PblBox;

class ShieldEffectClass : public FLEffectClass
{
    friend class ShieldEffectManager;
     
     

public:

    virtual ~ShieldEffectClass();
    // SYNTHETIC: SWBF2 0x00636ba0
    // ShieldEffectClass::`scalar deleting destructor'

    virtual FLEffectObject* BuildEffect();

    PblList<ShieldEffectClass>::Node m_Node;
    int m_type;
    unsigned int m_uiTextureHash;
    RedColor m_color;
    RedColor m_colorSpecular;
    float m_fRadius;
    float m_fHeight;
    float m_fViewAngleFactor;
    float m_fTurnOnTime;
    float m_fTurnOffTime;
    float m_fScrollRate;
    pcRedPrimitive* m_pPrim;

private:

    ShieldEffectClass(PblConfig& config);

    void PlatformInit();
    void PlatformCleanup();
};
 
 
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x007ae594
// VTABLE: SWBF2 0x007ae520
class ShieldEffect : public FLEffectObject, public RedSceneObject
{
     
     

public:

    ShieldEffect(const ShieldEffectClass* pClass);
    virtual ~ShieldEffect();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* pBlock) { sMemoryPool.Free(pBlock); }

    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
    virtual void StopAndFinish();

    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual const PblMatrix& GetBBoxMatrix() const;

    virtual const PblBox& GetBBox() const;
    virtual bool IsReflected() const;

    void TurnOff();

    static MemoryPool sMemoryPool;

private:
    virtual bool Update(float fDeltaTime);
    virtual void UpdatedWorldMatrix();

    void PlatformInit();
    void PlatformCleanup();
    void PlatformRender(const PblMatrix& mMatrix, unsigned char ucViewport, unsigned int uiFlags);

public:
    ShieldEffectClass* m_pClass;
    pcShieldShader* m_pShader;
    float m_fScrollTimer;
    float m_fTurnOnTimer;
    float m_fTurnOnFactor;
    bool m_bTurnOn;
    bool m_bStopAndFinish;
};
 
 
 
 
 
 

// VTABLE: SWBF2 0x007ae5c4
class ShieldEffectManager : public FLEffect
{
public:

    ShieldEffectManager();

    virtual ~ShieldEffectManager();
    // SYNTHETIC: SWBF2 0x00636690
    // ShieldEffectManager::`scalar deleting destructor'
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static ShieldEffectManager* s_pInstance;

    PblList<ShieldEffectClass> m_listClasses;
};
 
 
 