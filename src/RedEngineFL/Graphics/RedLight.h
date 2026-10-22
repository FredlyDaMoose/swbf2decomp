#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedMaterial.h"

class PblConfig;
class PblFileChunk;
class PblMatrix;
class RedCamera;
class RedAmbientLight;
class RedDirectionalLight;
class RedShadowRegionFactory;
class RedOmniLight;
class RedSpotLight;
class RedTexture;
class PblSphere;
struct RedLightingState;

class RedLight
{
public:

    static void UpdateAllVisibility(const RedCamera* pCamera, unsigned int uiView);

    enum TypeT
    {
        TYPE_AMBIENT = 0,
        TYPE_DIRECTIONAL = 1,
        TYPE_POINT = 2,
        TYPE_SPOT = 3,
        TYPE_FORCE_DWORD = 0x7fffffff
    };

    enum LIGHT_FLAGS
    {
        LIGHT_FLAG_STATIC = 1,
        LIGHT_FLAG_SHADOWCASTER = 2,
        LIGHT_FLAG_ACTIVE = 4,
        LIGHT_FLAG_SPECULARCASTER = 8,
        LIGHT_FLAG_OVERBRIGHT = 16,
        LIGHT_FLAG_INTERNAL_PROJMATDIRTY = 65536,
        LIGHT_FLAGS_ALL = 16777215
    };

    RedLight(TypeT eType);
    virtual ~RedLight();

    virtual void Activate();
    virtual void Deactivate();
    virtual void DebugDraw() const = 0;
    virtual bool IsVisible(const RedCamera* pCamera) const = 0;
    virtual bool SphereIntersect(const PblVector3& vCenter, float fRadius) const = 0;
    virtual bool AABBIntersect(const PblVector3& vMin, const PblVector3& vMax) const = 0;
    virtual const PblMatrix& GetProjectionMatrix() const;

    bool IsSpecularCaster() const { return (m_uiFlags & LIGHT_FLAG_SPECULARCASTER) != 0; }

    bool IsShadowCaster() const { return (m_uiFlags & LIGHT_FLAG_SHADOWCASTER) != 0; }

    void SetShadowCaster(bool bCaster)
    {
        if (bCaster)
        {
            m_uiFlags |= LIGHT_FLAG_SHADOWCASTER;
        }
        else
        {
            m_uiFlags &= ~LIGHT_FLAG_SHADOWCASTER;
        }
    }

    void SetStatic(bool bStatic)
    {
        if (bStatic)
        {
            m_uiFlags |= LIGHT_FLAG_STATIC;
        }
        else
        {
            m_uiFlags &= ~LIGHT_FLAG_STATIC;
        }
    }
    void SetName(unsigned int uiName) { m_uiName = uiName; }

    void SetOverbright(bool bOverbright)
    {
        if (bOverbright)
        {
            m_uiFlags |= LIGHT_FLAG_OVERBRIGHT;
        }
        else
        {
            m_uiFlags &= ~LIGHT_FLAG_OVERBRIGHT;
        }
    }

    void SetSpecularCaster(bool bCaster)
    {
        if (bCaster)
        {
            m_uiFlags |= LIGHT_FLAG_SPECULARCASTER;
        }
        else
        {
            m_uiFlags &= ~LIGHT_FLAG_SPECULARCASTER;
        }
    }

    static void InitSys();
    static void DeinitSys();

    static void PostLoad();
    static void GetGlobalDirectionalAmbientColors(RedColorValue& topColor,
                                                  RedColorValue& bottomColor);

    static void Read(PblFileChunk& chunk);
private:
     
     
    static void ReadGlobals(PblConfig& config);
    static void ReadLight(PblConfig& config, unsigned int uiName);
public:

    static RedAmbientLight* GetGlobalAmbientLight() { return m_sGlobalAmbient; }
    static RedDirectionalLight* GetGlobalDirectionalLight(int i) { return m_sGlobalDirectionals[i]; }

    static void SetGlobalAmbientLightColor(RedColorValue& color);
    static void SetGlobalDirectionalAmbientColors(RedColorValue& topColor,
                                                  RedColorValue& bottomColor);
    static void SetGlobalDirectionalLightColor(RedColorValue& color, int i);

    static const RedLight* Find(unsigned int uiName);

    static const RedLightingState* GetLightingState(bool bStatic);
    static const RedLightingState* GetLightingState(const PblSphere& sphere, int iMaxOmniLights,
                                                    bool bNoShadowRegions, bool bStatic,
                                                    bool bNoProjectedLights);
    static const RedLightingState* GetLightingState(const PblBox& box, int iMaxOmniLights,
                                                    bool bNoShadowRegions, bool bStatic,
                                                    bool bNoProjectedLights);

    float GetIntensity() const
    {
        return m_color.r * 0.3f + m_color.g * 0.45f + m_color.b * 0.25f;
    }

    void SetProjectionTexture(unsigned int uiTexture) { m_uiProjTexHash = uiTexture; }
    void SetProjectionTextureWrapMode(unsigned int uiMode) { m_uiProjTexWrapMode = uiMode; }
    void SetProjectionTextureBlendMode(unsigned int uiMode) { m_uiProjTexBlendMode = uiMode; }

    unsigned int GetProjectionTextureWrapMode() const { return m_uiProjTexWrapMode; }

    unsigned int m_uiHideInView : 8;
    unsigned int m_uiFlags : 24;
    unsigned int m_uiName;
    TypeT m_eType;
    unsigned int m_uiProjTexHash;
    unsigned int m_uiProjTexWrapMode;
    unsigned int m_uiProjTexBlendMode;
    float m_fProjTexTile;
    RedColorValue m_color;
    PblList<RedLight>::Node m_GlobalNode;
    PblList<RedLight>::Node m_VisibleNode;

    static PblList<RedLight> s_GlobalList;
    static PblList<RedLight> s_VisibleOmniLightList;
    static PblList<RedLight> s_VisibleSpotLightList;
    static PblList<RedLight> s_VisibleDirectionalLightList;
    static RedShadowRegionFactory* s_pShadowRegionFactory;
    static RedDirectionalLight* m_sGlobalDirectionals[2];
    static RedAmbientLight* m_sGlobalAmbient;
    static unsigned int m_sGlobalEnvMapHash;
    static float m_sGlobalDirectionalFactor[2];
};
 
 
 
 
 
 

class RedShadowMap
{
public:
    static void InitSys();
    static void SetScreenSpaceShadowMap(RedTexture* pTexture, const PblMatrix& matrix);

    static bool IsShadowMapEnabled();
    static RedTexture* GetShadowMapTexture() { return _pShadowMapTexture; }
    static unsigned int GetShadowMapWrapMode() { return _uiShadowMapWrapMode; }
    static RedTexture* _pShadowMapTexture;

    static float _fShadowMapScale;
    static unsigned int _uiShadowMapWrapMode;
    static PblVector4 _vShadowMapTransformU;
    static PblVector4 _vShadowMapTransformV;
    static PblVector4 _vShadowMapTransformW;
    static bool _bShadowMapEnable;
public:
    static unsigned int _uiShadowMapHash;
    static unsigned int _fShadowMapOffsetX;
    static unsigned int _fShadowMapOffsetZ;
};

#include "RedAmbientLight.h"
