#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblList.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedColor.h"

class DecalClass;
class pcRedShader;

class Decal : public PblList<Decal>::Node
{
public:

    class DecalVertex
    {
    public:
        float pos[3];
        RedColor color;
        float uv[2];
    };

    ~Decal();

    static void operator delete(void* p) { sMemoryPool.Free(p); }

    static MemoryPool sMemoryPool;

    float fTime;
    PblVector3 vPos;
    float fRadius;
    RedCamera::VisibilityT visibility;
    RedColor color;
    unsigned int numVertices;
    unsigned int numIndices;
    float fadeVal;
    DecalVertex* vertices;
    unsigned short* indices;
};
 
 
 
 
 

// VTABLE: SWBF2 0x00798f1c
class DecalClass : public FLEffectClass, public PblList<DecalClass>::Node
{
public:

    DecalClass(PblConfig& config);

    virtual ~DecalClass();
    // SYNTHETIC: SWBF2 0x0048b0f0
    // DecalClass::`scalar deleting destructor'

    virtual FLEffectObject* BuildEffect();

    void ReadConfig(PblConfig& config);

    bool IsBumped();

    float m_lifeTime;
    RedColor m_color;
    float m_fadeInTime;
    float m_fadeOutTime;
    unsigned int m_textureHash;
    unsigned int m_bumpTextureHash;
    PblList<Decal> m_decals;
    int m_maxDecals;
    unsigned int m_totalFrameVertices;
    unsigned int m_totalFrameIndices;
    unsigned int m_totalVisible;
    pcRedShader* m_pShader;
};
 
 
 
 
 

// VTABLE: SWBF2 0x00798ee8
class DecalManager : public FLEffect
{
public:

    DecalManager();

    virtual ~DecalManager();
    // SYNTHETIC: SWBF2 0x0048ac20
    // DecalManager::`scalar deleting destructor'

    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void Update(float fDeltaTime);
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    void AddClass(DecalClass* pClass);
    void RemoveClass(DecalClass* pClass);

    void PlatformRender(unsigned int uFlags) const;

    static DecalManager* s_pInstance;

    unsigned int m_numDecals;
    PblList<DecalClass> m_decalClasses;
};
 
 
 
 