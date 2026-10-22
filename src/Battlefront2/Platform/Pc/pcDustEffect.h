#pragma once

#include <stddef.h>

#include "Battlefront2/Source/DustEffect.h"
#include "PebbleFL/Common/PblHeap.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedColor.h"

class pcParticleShader;
class pcRedPrimitive;

// VTABLE: SWBF2 0x00796490
class DustEffectRenderData : public Renderable
{
public:
    DustEffectRenderData(const DustEffectClass* pClass);
    virtual ~DustEffectRenderData();

    virtual void Render(int value);

    void Begin() const;
    void AddParticle(const PblVector3& vPos, float fSize, const RedColor& color,
                     float fOffset) const;
    void End() const;

public:
    const DustEffectClass* m_pClass;
    pcParticleShader* m_pShader;
    PblHeap<int, float>* mHeap;
    mutable pcRedPrimitive* m_pPrimitive;
    mutable void* m_pVertices;
    mutable PblVector3 m_vUp;
    mutable PblVector3 m_vRight;
};
  
 
 
 
 
 
 
 
