#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"

class SunFlare;
class RedTexture;
struct FlareVertex;
struct FlareCompressedVertex;

// VTABLE: SWBF2 0x007969ac
class SunFlareEffect : public FLEffect
{
public:

    class SunFlareList : public PblList<SunFlare>
    {
    public:

        void Add(SunFlare* pFlare);
        void Remove(SunFlare* pFlare);
        void Cleanup();
    };

    SunFlareEffect()
        : FLEffect(0xBDF357B3u, 6)
    {
    }

    virtual ~SunFlareEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual FLEffectClass* BuildClass(PblConfig& config) const;
    virtual void RenderSky(unsigned int uiPass) const;

    static void Create();

    static void Destroy();

    static SunFlareEffect* s_pInstance;

    SunFlareList _List;
};
 

// VTABLE: SWBF2 0x007969e4
class SunSkyRenderable : public Renderable
{
public:
    SunSkyRenderable()
        : Renderable(1)
    {
    }

    // SYNTHETIC: SWBF2 0x0042d220
    // SunSkyRenderable::`scalar deleting destructor'

    virtual void Render(int value);
};
 

 
 class SunFlare : public FLEffectClass, public FLEffectObject
{
    friend class SunFlareEffect;

public:

    struct Rect;

    template <class VERTEX>
    void EmitSunRings(VERTEX*& pVertices, const PblVector3& vPosition, float fTexU, float fTexV) const;

    SunFlare();
    SunFlare(const SunFlare& that);
    virtual ~SunFlare();
    virtual FLEffectObject* BuildEffect();
    virtual const PblMatrix& GetMatrix() const;
    virtual const PblVector3& GetPosition() const;
    virtual float GetSize() const;
    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();

    void SetSize(float fSize) { _fSize = fSize; }
    void SetColor(const RedColor& color) { _color = color; }
    void SetDirection(const PblVector3& vDirection) { _vDirection = vDirection; }
    void SetFlareOutSize(float fSize) { _fFlareOutSize = fSize; }
    void SetNumFlareOuts(int iNum) { _iNumFlareOuts = iNum; }
    void SetHaloInnerRing(float fSize, const RedColor& color)
    {
        _sunHaloSize[0] = fSize;
        _sunHaloColor[0] = color;
    }
    void SetHaloMiddleRing(float fSize, const RedColor& color)
    {
        _sunHaloSize[1] = fSize;
        _sunHaloColor[1] = color;
    }
    void SetHaloOutterRing(float fSize, const RedColor& color)
    {
        _sunHaloSize[2] = fSize;
        _sunHaloColor[2] = color;
    }
    void SetSpikeColor(const RedColor& color) { _spikeColor = color; }
    void SetSpikeSize(float fSize) { _spikeSize = fSize; }
    void SetFlareEnable(bool bEnable) { _flareEnable = bEnable; }

protected:
    static float GetDistance();
    void _Render() const;
    template <class VERTEX, int RVF>
    void RenderSunAndHalo(unsigned int uiPass) const;

    template <class VERTEX, int RVF>
    void RenderSunRings(const PblVector3& vPosition, float, float, float) const;

public:
    RedTexture* m_sunTex;
    float _fSize;
    RedColor _color;
    PblVector3 _vDirection;
    float _fFlareOutSize;
    int _iNumFlareOuts;
    unsigned char _initialFlareOutAlpha;
    float _sunHaloSize[3];
    RedColor _sunHaloColor[3];
    RedColor _spikeColor;
    float _spikeSize;
    bool _flareEnable;
    PblList<SunFlare>::Node _Node;
};

struct SunFlare::Rect
{
    float minX;
    float minY;
    float maxX;
    float maxY;
};

 
 
 
 
