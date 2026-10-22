#pragma once

#include "RedEngineFL/Application/Renderable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class PblMatrix;

enum RedBlendMode
{
    BLEND_NORMAL = 0,
    BLEND_ADDITIVE = 1,
    BLEND_BLUR = 2
};

enum ParticleInfoType
{
    PIT_PARTICLE = 0,
    PIT_STREAK_HEAD = 1,
    PIT_STREAK = 2,
    PIT_STREAK_TAIL = 3,
    PIT_LASER_START = 4,
    PIT_LASER_END = 5,
    PIT_FLAT = 6,
    PIT_ANIMATED = 7,
    PIT_GEOMETRY = 8
};

struct RedParticleInfoType
{
    RedParticleInfoType() {}

    ParticleInfoType mPit;
    PblVector3 mPos;
    RedColor mColor;
    float mSize;
    float mRotation;
    float u[2];
    float v[2];
    PblVector3 mRight;
    PblVector3 mUp;
};

 

const PblVector3 CalcStreakSplitVector(const PblVector3& vPrev, const PblVector3& vCur,
                                       const PblVector3& vNext, float fWidth,
                                       const PblMatrix& mCamera);

class RedParticleRenderer
{
public:

    static void Init();
    static void Cleanup();

    struct TexCoords
    {
        unsigned int u0 : 8;
        unsigned int v0 : 8;
        unsigned int u1 : 8;
        unsigned int v1 : 8;
    };

    static void SubmitTexture(unsigned int uTexture, RedBlendMode eBlend, unsigned int);
    static void SubmitBounds(PblVector3 const& vMin, PblVector3 const& vMax);
    static void RenderAll();
    static void SetBlurParams(float fBlurValue, int iBlurParam);
    static bool IsFull();

    static void SubmitParticle(ParticleInfoType eType, PblVector3 const& vPos, RedColor const& color,
                               float, float, TexCoords texCoords,
                               PblVector3 const& vRight, PblVector3 const& vUp);

    // VTABLE: SWBF2 0x00773028
    class Renderable : public ::Renderable
    {
    public:

        Renderable();

        virtual ~Renderable();

        virtual void Render(int value);
    };

    static RedParticleRenderer* currentCache;

    static int s_cacheIndex;

    static RedParticleRenderer s_caches[15];

protected:

    static void SetCurrentCache(unsigned int uTexture, RedBlendMode eBlend, unsigned int uiRenderFlags);

    bool AddParticle(ParticleInfoType eType, const PblVector3& vPos, const RedColor& color,
                     float fSize, float fRotation, TexCoords texCoords,
                     const PblVector3& vRight, const PblVector3& vUp);
    void AddParticleToBuffers(void* pVertices, unsigned short* pIndices);

    RedParticleInfoType m_particleCache[200];
    int m_particleIndex;
    unsigned int m_texHash;
    RedBlendMode m_blendMode;
    unsigned int m_uiRenderFlags;
    unsigned int m_numVerts;
    unsigned int m_numIndices;
    PblBox m_blurBounds;
    float m_blurValue;
    float m_blurParam;
public:
    static RedParticleRenderer::Renderable _FarInstance;
    static RedParticleRenderer::Renderable _Instance;
    static bool s_blurEnable;
};

 
