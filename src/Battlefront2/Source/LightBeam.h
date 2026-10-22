#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "LensFlare.h"
#include "RedEngineFL/Util/RedSceneObject.h"

class LightBeam;
class pcRedShader;

// VTABLE: SWBF2 0x007a3b54
class LightBeamEffect : public FLEffect
{
public:

    LightBeamEffect()
        : FLEffect(0xE8651328u, 3)
    {
        m_pShader = 0;
    }

    virtual ~LightBeamEffect();
    virtual void Render(int value);
    virtual void Init();
    virtual void Cleanup();
    virtual void ReadConfig(PblConfig& config);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static LightBeamEffect* s_pInstance;

    static LightBeamEffect* Instance() { return s_pInstance; }

    void RenderLightBeam(const LightBeam* pBeam, unsigned int uiFlags) const;

    pcRedShader* m_pShader;

private:

    void _PlatformInit();

    void _PlatformCleanup();
};
 
 
 class LightBeam : public FLEffectClass, public FLEffectObject
{
public:
    LightBeam();
    LightBeam(const LightBeam& that);
    virtual ~LightBeam();
    virtual FLEffectObject* BuildEffect();
    virtual void SetHeadPosition(const PblVector3& vPos);
    virtual void SetDirection(const PblVector3& vDir);
    virtual void SetLength(float fLength);
    virtual void SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail);
    virtual void SetPositionOmni(const PblVector3& vPos);
    virtual void SetInitialWidth(float fWidth);
    virtual void SetInitialWidth(float fWidthX, float fWidthZ);
    virtual void SetFinalWidth(float fWidth);
    virtual void SetFinalWidth(float fWidthX, float fWidthZ);
    virtual void SetFadeLength(float fLength);
    virtual void Unhide();
    virtual void HideInView(unsigned int uiView, bool bHide);
    virtual bool IsHiddenInView(unsigned int uiView);
    virtual void AddToRenderList();
    virtual void RemoveFromRenderList();

    void SetColor(const RedColor& color);

    void Render(unsigned int uiFlags) const { LightBeamEffect::Instance()->RenderLightBeam(this, uiFlags); }
    void SetDrawDistance(float fDistance)
    {
        _fSquaredDrawDistance = fDistance * fDistance;
        _Flare.SetDrawDistance(fDistance);
    }
    void SetFadeFactor(float fFactor) { _fFadeAmount = fFactor; }
    void SetFlareIntensity(float fIntensity)
    {
        _fFlareIntensity = fIntensity;
        RedColor flareColor = _color;
        flareColor.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
        _Flare.SetColor(flareColor);
    }

    virtual bool IsEffectActive() const;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
protected:
    virtual void UpdatedWorldMatrix();
public:

    void GetLocalBoundingSphere(PblVector3& vCenter, float& fRadius) const;

    void SetBeamIntensity(float fIntensity) { _fBeamIntensity = fIntensity; }

    LensFlare _Flare;
    PblVector3 _vHeadPosition;
    PblVector3 _vDirection;
    RedColor _color;
    float _fLength;
    float _fHeadSizeX;
    float _fHeadSizeZ;
    float _fTailSizeX;
    float _fTailSizeZ;
    float _fFadeLength;
    float _fFadeAmount;
    float _fSquaredDrawDistance;
    float _fFlareIntensity;
    float _fBeamIntensity;
    float _fFlareCosAngle;
};
 
 
 
 
 class ManagedLightBeam : public RedSceneObject, public LightBeam
{
public:

    ManagedLightBeam();

    ManagedLightBeam(const LightBeam& beam);
    virtual ~ManagedLightBeam();
    virtual void Render(unsigned char ucViewport, float fDist, unsigned int uiFlags);
    virtual void SetHeadPosition(const PblVector3& vPos);
    virtual void SetDirection(const PblVector3& vDir);
    virtual void SetLength(float fLength);
    virtual void SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail);
    virtual void SetPositionOmni(const PblVector3& vPos);
    virtual void SetInitialWidth(float fWidth);
    virtual void SetInitialWidth(float fWidthX, float fWidthZ);
    virtual void SetFinalWidth(float fWidth);
    virtual void SetFinalWidth(float fWidthX, float fWidthZ);
    virtual void SetFadeLength(float fLength);
    virtual void Unhide();
    virtual void HideInView(unsigned int uiView, bool bHide);
    virtual bool IsHiddenInView(unsigned int uiView);
    virtual void AddToRenderList();
    virtual void RemoveFromRenderList();
protected:
    virtual void UpdatedWorldMatrix();
private:

    void _UpdateBoundingSphere();
public:
    unsigned int _uiHideInView;
};
 
 
