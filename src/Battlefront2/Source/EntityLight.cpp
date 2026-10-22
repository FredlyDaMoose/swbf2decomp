#include <math.h>
#include <stdio.h>
#include <string.h>

#include "EntityLight.h"
#include "Battlefront2Lod.h"
#include "GameLoop.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedOmniLight.h"
#include "RedEngineFL/Graphics/RedSpotLight.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

#include "LightBeam.h"

extern PblVector3 gVectorZero;

class PblMatrix;
extern PblMatrix gMatrixIdentity;

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                        const PblMatrix* pM);

// GLOBAL: SWBF2 0x01e51de0
EntityLightFXParser* EntityLightFXParser::s_pInstance;

// GLOBAL: SWBF2 0x01ebbe30
// SYNTHETIC: SWBF2 0x00402c00
// `dynamic initializer for 'EntityLight::rttiHashEntityLight''
PblHash EntityLight::rttiHashEntityLight("EntityLight");
char EntityLight::rttiNameEntityLight[] = "EntityLight";
// GLOBAL: SWBF2 0x01ebbe34
// SYNTHETIC: SWBF2 0x00402c10
// `dynamic initializer for 'EntityLightClass::rttiHashEntityLightClass''
PblHash EntityLightClass::rttiHashEntityLightClass("EntityLightClass");
char EntityLightClass::rttiNameEntityLightClass[] = "EntityLightClass";
// GLOBAL: SWBF2 0x01ebbe38
// SYNTHETIC: SWBF2 0x00402be0
// `dynamic initializer for 'EntityLight::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766f70
// `dynamic atexit destructor for 'EntityLight::sMemoryPool''
MemoryPool EntityLight::sMemoryPool("EntityLight", 0x2e0);
// GLOBAL: SWBF2 0x01ebbe88
int EntityLightClass::sLightClassTableNumEntries;
// GLOBAL: SWBF2 0x01ebbe8c
// SYNTHETIC: SWBF2 0x00402c20
// `dynamic initializer for 'EntityLightClass::sLightClassTable''

PblHashTable<EntityLightClass, 32> EntityLightClass::sLightClassTable;

// FUNCTION: SWBF2 0x004ce920
EntityLight* EntityLightClass::Build(unsigned int uLightOdfNameHash)
{
    EntityDesc desc;
    desc.mMatrix = gMatrixIdentity;
    EntityLightClass* pClass = EntityLightClass::sLightClassTable.Find(uLightOdfNameHash);
    if (!pClass)
    {
        return 0;
    }
    return (EntityLight*)pClass->Build(desc);
}

// SYNTHETIC: SWBF2 0x0042d060
// FLEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0042d250
RedColor ReadColor(const PblConfig::Data& data)
{
    RedColor color;
    float f = data.GetFloatArg(0);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.r = (unsigned char)(int)f;
    f = data.GetFloatArg(1);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.g = (unsigned char)(int)f;
    f = data.GetFloatArg(2);
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    color.b = (unsigned char)(int)f;
    if (data.GetNumArgs() > 3)
    {
        f = data.GetFloatArg(3);
        if (f < 0.0f)
        {
            f = 0.0f;
        }
        else if (f > 255.0f)
        {
            f = 255.0f;
        }
        color.a = (unsigned char)(int)f;
    }
    else
    {
        color.a = 0xff;
    }
    return color;
}

// FUNCTION: SWBF2 0x004708b0
bool PblConfig::Data::IsStringArg(unsigned int i) const
{
    if (i < m_uiNumArgs)
    {
        unsigned int uiOffset = m_auiArgs[i];
        if (uiOffset >= m_uiNumArgs * 4 && uiOffset < 0x200)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004cc320
void ManagedLightBeam::Render(unsigned char, float, unsigned int uiFlags)
{
    LightBeam::Render(uiFlags);
}

// FUNCTION: SWBF2 0x004cc850
void ManagedLightBeam::Unhide()
{
    _uiHideInView = 0;
    _Flare._uiHideInView = 0;
}

// FUNCTION: SWBF2 0x004cc870
void ManagedLightBeam::HideInView(unsigned int uiView, bool bHide)
{
    const unsigned int mask = 1u << uiView;
    if (bHide)
    {
        _uiHideInView |= mask;
        _Flare._uiHideInView |= mask;
    }
    else
    {
        _uiHideInView &= ~mask;
        _Flare._uiHideInView &= ~mask;
    }
}

// FUNCTION: SWBF2 0x004cc8c0
bool ManagedLightBeam::IsHiddenInView(unsigned int uiView)
{
    return (_uiHideInView & (1u << uiView)) != 0;
}

// FUNCTION: SWBF2 0x004cc340
void ManagedLightBeam::SetHeadPosition(const PblVector3& vPos)
{
    LightBeam::SetHeadPosition(vPos);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc3d0
void ManagedLightBeam::SetDirection(const PblVector3& vDir)
{
    LightBeam::SetDirection(vDir);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc450
void ManagedLightBeam::SetLength(float fLength)
{
    LightBeam::SetLength(fLength);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc4c0
void ManagedLightBeam::SetHeadTailPosition(const PblVector3& vHead, const PblVector3& vTail)
{
    LightBeam::SetHeadTailPosition(vHead, vTail);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc530
void ManagedLightBeam::SetPositionOmni(const PblVector3& vPos)
{
    LightBeam::SetPositionOmni(vPos);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc660
void ManagedLightBeam::SetInitialWidth(float fWidthX, float fWidthZ)
{
    LightBeam::SetInitialWidth(fWidthX, fWidthZ);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc5d0
void ManagedLightBeam::SetInitialWidth(float fWidth)
{
    LightBeam::SetInitialWidth(fWidth);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc6f0
void ManagedLightBeam::SetFinalWidth(float fWidth)
{
    LightBeam::SetFinalWidth(fWidth);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc760
void ManagedLightBeam::SetFinalWidth(float fWidthX, float fWidthZ)
{
    LightBeam::SetFinalWidth(fWidthX, fWidthZ);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc7e0
void ManagedLightBeam::SetFadeLength(float fLength)
{
    LightBeam::SetFadeLength(fLength);
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

// FUNCTION: SWBF2 0x004cc8e0
void ManagedLightBeam::UpdatedWorldMatrix()
{
    LightBeam::UpdatedWorldMatrix();
    if (_bActive)
    {
        _UpdateBoundingSphere();
    }
}

void ManagedLightBeam::_UpdateBoundingSphere()
{
    PblVector3 vCenter;
    float fRadius;
    GetLocalBoundingSphere(vCenter, fRadius);
    SetRenderRadius(fRadius);
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &vCenter, &m_mWorldMatrix);
    SetRenderPos(vPos);
}

// FUNCTION: SWBF2 0x004cc940
void ManagedLightBeam::AddToRenderList()
{
    if (!_bActive)
    {
        _UpdateBoundingSphere();
        Activate();
    }
}

// FUNCTION: SWBF2 0x004cc9a0
void ManagedLightBeam::RemoveFromRenderList()
{
    Deactivate();
}

// SYNTHETIC: SWBF2 0x004cf48c
// [thunk]:ManagedLightBeam::`scalar deleting destructor' adjustor{96}
// SYNTHETIC: SWBF2 0x004cf494
// [thunk]:ManagedLightBeam::`scalar deleting destructor' adjustor{136}
// SYNTHETIC: SWBF2 0x004cf4a7
// [thunk]:ManagedLightBeam::`scalar deleting destructor' adjustor{112}
// SYNTHETIC: SWBF2 0x004cc9b0
// ManagedLightBeam::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004cca10
ManagedLightBeam::~ManagedLightBeam()
{
}

ManagedLightBeam::ManagedLightBeam()
    : RedSceneObject(&largeEffectLodData)
    , LightBeam()
    , _uiHideInView(0)
{
}

// FUNCTION: SWBF2 0x004cca60
bool EntityLight::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityLight || Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004cca90
unsigned int EntityLight::GetDerivedRtti() const
{
    return rttiHashEntityLight;
}

// FUNCTION: SWBF2 0x004ccaa0
const char* EntityLight::GetDerivedRttiName() const
{
    return rttiNameEntityLight;
}

// FUNCTION: SWBF2 0x004ccab0
bool EntityLightClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityLightClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x004ccae0
unsigned int EntityLightClass::GetDerivedRtti()
{
    return rttiHashEntityLightClass;
}

// FUNCTION: SWBF2 0x004ccaf0
const char* EntityLightClass::GetDerivedRttiName()
{
    return rttiNameEntityLightClass;
}

// FUNCTION: SWBF2 0x004ccb00
void EntityLightFXParser::Init()
{
}

// FUNCTION: SWBF2 0x004ccb10
void EntityLightFXParser::Cleanup()
{
}

// FUNCTION: SWBF2 0x004cebd0
void EntityLightFXParser::ReadConfig(PblConfig&)
{
}

// FUNCTION: SWBF2 0x004cebe0
FLEffectClass* EntityLightFXParser::BuildClass(PblConfig& config) const
{
    EntityLightClass* pClass = new EntityLightClass(config.m_uiNameHash);
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x09E7F535u:
            if (data.GetBoolArg(0))
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_SHADOWCASTER;
            }
            else
            {
                pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_SHADOWCASTER;
            }
            break;
        case 0x044C2100u:
            if (data.GetBoolArg(0))
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_BIDIRECTIONAL;
            }
            else
            {
                pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_BIDIRECTIONAL;
            }
            break;
        case 0xF946B484u:
            if (data.GetBoolArg(0))
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_SPECULARCASTER;
            }
            else
            {
                pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_SPECULARCASTER;
            }
            break;
        case 0xD290C23Bu:
            if (data.GetBoolArg(0))
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_STATIC;
            }
            else
            {
                pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_STATIC;
            }
            break;
        case 0x7E276E3Du:
            if (data.GetBoolArg(0))
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_SYNCHRONIZE;
            }
            else
            {
                pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_SYNCHRONIZE;
            }
            break;
        case 0x5127F14Du:
            if (PblHash(data.GetStringArg(0)) == 0x2305F67Du)
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_TYPE_SPOT;
            }
            else
            {
                pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_TYPE_OMNI;
            }
            break;
        case 0x8D39BDE6u:
            pClass->_uiName = PblHash(data.GetStringArg(0));
            break;
        case 0x3D7E6258u:
            pClass->_color = ReadColor(data);
            break;

        case 0x9342844Eu:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.2f;
            }
            pClass->_fBeamLength = data.GetFloatArg(0);
            if (pClass->_fBeamLength <= 0.0f)
            {
                pClass->_fBeamLength = 0.0f;
            }
            break;
        case 0x6F7F1016u:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.2f;
            }
            pClass->_fBeamWidthX = (float)fabs(data.GetFloatArg(0));
            if (data.GetNumArgs() >= 2)
            {
                pClass->_fBeamWidthZ = (float)fabs(data.GetFloatArg(1));
            }
            else
            {
                pClass->_fBeamWidthZ = pClass->_fBeamWidthX;
            }
            if (pClass->_fBeamWidthX == 0.0f || pClass->_fBeamWidthZ == 0.0f)
            {
                pClass->_fBeamWidthZ = 0.0f;
                pClass->_fBeamWidthX = 0.0f;
            }
            break;
        case 0x9CBB54E2u:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.2f;
            }
            pClass->_fBeamInitialWidthX = (float)fabs(data.GetFloatArg(0));
            if (data.GetNumArgs() >= 2)
            {
                pClass->_fBeamInitialWidthZ = (float)fabs(data.GetFloatArg(1));
            }
            else
            {
                pClass->_fBeamInitialWidthZ = pClass->_fBeamInitialWidthX;
            }
            if (pClass->_fBeamInitialWidthX == 0.0f || pClass->_fBeamInitialWidthZ == 0.0f)
            {
                pClass->_fBeamInitialWidthZ = 0.0f;
                pClass->_fBeamInitialWidthX = 0.0f;
            }
            break;
        case 0x663A891Du:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.5f;
            }
            pClass->_fBeamWidthZ = pClass->_fBeamWidthX = (float)fabs(data.GetFloatArg(0));
            break;
        case 0x857435A7u:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.2f;
            }
            pClass->_fBeamFadeFactor = data.GetFloatArg(0);
            if (pClass->_fBeamFadeFactor < 0.0f)
            {
                pClass->_fBeamFadeFactor = 0.0f;
            }
            else if (pClass->_fBeamFadeFactor > 1.0f)
            {
                pClass->_fBeamFadeFactor = 1.0f;
            }
            break;
        case 0x8807A05Au:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.5f;
            }
            pClass->_fBeamFadeFactor = data.GetFloatArg(0);
            if (pClass->_fBeamFadeFactor < 0.0f)
            {
                pClass->_fBeamFadeFactor = 0.0f;
            }
            else if (pClass->_fBeamFadeFactor > 1.0f)
            {
                pClass->_fBeamFadeFactor = 1.0f;
            }
            break;
        case 0x2C52F680u:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.2f;
            }
            pClass->_fBeamFadeLength = data.GetFloatArg(0);
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.0f;
            }
            else if (pClass->_fBeamFadeLength > 1.0f)
            {
                pClass->_fBeamFadeLength = 1.0f;
            }
            break;
        case 0xC35D99B9u:
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.5f;
            }
            pClass->_fBeamFadeLength = data.GetFloatArg(0);
            if (pClass->_fBeamFadeLength < 0.0f)
            {
                pClass->_fBeamFadeLength = 0.0f;
            }
            else if (pClass->_fBeamFadeLength > 1.0f)
            {
                pClass->_fBeamFadeLength = 1.0f;
            }
            break;
        case 0x1612C34Cu:
            pClass->_fBeamDrawDistance = data.GetFloatArg(0);
            break;
        case 0x633ED9D4u:
            pClass->_fFlareIntensity = data.GetFloatArg(0);
            if (pClass->_fFlareIntensity < 0.0f)
            {
                pClass->_fFlareIntensity = 0.0f;
            }
            else if (pClass->_fFlareIntensity > 1.0f)
            {
                pClass->_fFlareIntensity = 1.0f;
            }
            break;
        case 0x75096FABu:
            pClass->_fBeamIntensity = data.GetFloatArg(0);
            if (pClass->_fBeamIntensity < 0.0f)
            {
                pClass->_fBeamIntensity = 0.0f;
            }
            else if (pClass->_fBeamIntensity > 1.0f)
            {
                pClass->_fBeamIntensity = 1.0f;
            }
            break;

        case 0x3685BB77u:
            if (data.GetNumArgs() >= 1)
            {
                if (data.IsStringArg(0))
                {
                    switch (PblHash(data.GetStringArg(0)))
                    {
                    case 0xC33D6F68u:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_Strobe;
                        break;
                    case 0xF56C83DFu:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_StrobeRandom;
                        break;
                    case 0x135752DEu:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_StrobeFade;
                        break;
                    case 0x550BFE9Eu:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_Pulse;
                        break;
                    case 0x0D7C428Fu:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_Flicker;
                        break;
                    default:
                        pClass->_eLightFlickerType = EntityLight::LightFlickerType_None;
                        break;
                    }
                }
                else
                {
                    unsigned int uType = (int)data.GetFloatArg(0);
                    pClass->_eLightFlickerType = uType < EntityLight::NumLightFlickerTypes
                                                     ? uType
                                                     : EntityLight::LightFlickerType_None;
                }
            }
            break;
        case 0xD92EA25Eu:
            pClass->_fLightFlickerPeriodOn = data.GetFloatArg(0);
            pClass->_fLightFlickerPeriodOff = data.GetNumArgs() > 1 ? data.GetFloatArg(1) : data.GetFloatArg(0);
            break;
        case 0x65D177CAu:
            pClass->_fLightFadePeriod = data.GetFloatArg(0);
            break;
        case 0xC2C1813Au:
            pClass->_fOmniRadius = data.GetFloatArg(0);
            break;
        case 0xC967096Bu:
            pClass->_fSpotInnerConeAngle = data.GetFloatArg(0) * 3.1415927f / 180.0f;
            break;
        case 0xB22D429Eu:
            pClass->_fSpotOuterConeAngle = data.GetFloatArg(0) * 3.1415927f / 180.0f;
            break;

        case 0x99E6DBFEu:
            if (data.GetNumArgs() < 2)
            {
                pClass->_uiNumFrames = 1;
            }
            pClass->_puiProjectedTextures = new unsigned int[pClass->_uiNumFrames];
            if (pClass->_uiNumFrames == 1)
            {
                pClass->_puiProjectedTextures[0] = PblHash(data.GetStringArg(0));
            }
            else
            {
                for (unsigned int i = 0; i < pClass->_uiNumFrames; ++i)
                {
                    char szFrame[128];
                    sprintf(szFrame, "%s%d", data.GetStringArg(0), pClass->_uiNumFrames);
                    pClass->_puiProjectedTextures[i] = PblHash(szFrame);
                }
            }
            break;
        case 0x9B111AE4u:
            pClass->_fFrameRate = data.GetFloatArg(0);
            if (pClass->_fFrameRate < 0.0f)
            {
                pClass->_fFrameRate = 0.0f;
            }
            break;

        default:
            pClass->EntityClass::SetProperty(data.GetId(), data.GetStringArg(0));
            break;
        }
    }
    return pClass;
}

// FUNCTION: SWBF2 0x004ccb20
void EntityLightFXParser::Render(int)
{
}

// SYNTHETIC: SWBF2 0x004ccb30
// EntityLightFXParser::`scalar deleting destructor'

EntityLightFXParser::~EntityLightFXParser()
{
}

// FUNCTION: SWBF2 0x004ccba0
EntityLight::EntityLight()
    : m_bOn(false)
    , _fBeamLength(0.0f)
    , _fBeamWidthX(0.0f)
    , _fBeamWidthZ(0.0f)
    , _fBeamFadeLength(0.0f)
    , _fLightFlickerTimer(0.0f)
    , _fLightFlickerIntensity(1.0f)
{
    _light = new RedOmniLight(PblVector3(0.0f, 0.0f, 0.0f), 0.0f, RedColorValue::BLACK);
    SetOmniLightRadius(10.0f);
    SetColor(RedColor::WHITE);
    _LightBeam.SetDrawDistance(200.0f);
    _LightBeam.SetFlareIntensity(0.0f);
    _LightBeam.SetInitialWidth(0.0f);
    _LightBeam.SetFadeFactor(1.0f);
    _LightBeam.SetFadeLength(0.0f);
    _LightBeam.SetBeamIntensity(1.0f);
    _lightClass = 0;
    RecalculateBeam();
}

// SYNTHETIC: SWBF2 0x004cf49f
// [thunk]:EntityLight::`scalar deleting destructor' adjustor{40}
// SYNTHETIC: SWBF2 0x004cf4af
// [thunk]:EntityLight::`scalar deleting destructor' adjustor{16}
// SYNTHETIC: SWBF2 0x004ccdf0
// EntityLight::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004cce20
EntityLight::EntityLight(const EntityLightClass* pClass, const EntityDesc& desc)
    : m_bOn(false)
{
    if (pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_TYPE_SPOT)
    {
        _light = new RedSpotLight(RedColorValue::WHITE, gVectorUnitZ, gVectorZero, 1.0f, 1.0f);
        ((RedSpotLight*)_light)->SetInnerConeAngle(pClass->_fSpotInnerConeAngle);
        ((RedSpotLight*)_light)->SetOuterConeAngle(pClass->_fSpotOuterConeAngle);
        ((RedSpotLight*)_light)->SetBidirectional(
            (pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_BIDIRECTIONAL) != 0);
    }
    else
    {
        _light = new RedOmniLight(PblVector3(0.0f, 0.0f, 0.0f), 0.0f, RedColorValue::BLACK);
    }

    _LightBeam.SetDrawDistance(pClass->_fBeamDrawDistance);
    _LightBeam.SetFlareIntensity(pClass->_fFlareIntensity);
    _LightBeam.SetBeamIntensity(pClass->_fBeamIntensity);
    _LightBeam.SetHeadPosition(PblVector3(0.0f, 0.0f, 0.0f));
    _LightBeam.SetInitialWidth(fabs(pClass->_fBeamInitialWidthX), fabs(pClass->_fBeamInitialWidthZ));
    _LightBeam.SetFadeFactor(pClass->_fBeamFadeFactor);

    if (pClass->_fBeamLength > 0.0f)
    {
        _fBeamLength = pClass->_fBeamLength;
        _fBeamWidthX = fabs(pClass->_fBeamWidthX);
        _fBeamWidthZ = fabs(pClass->_fBeamWidthZ);
    }
    else
    {
        _fBeamLength = 0.0f;
        _fBeamWidthX = _fBeamWidthZ = fabs(pClass->_fBeamWidthX);
    }
    _fBeamFadeLength = pClass->_fBeamFadeLength;
    RecalculateBeam();

    SetColor(pClass->_color);
    SetOmniLightRadius(pClass->_fOmniRadius);

    _light->SetStatic((pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_STATIC) != 0);
    _light->SetShadowCaster((pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_SHADOWCASTER) != 0);
    _light->SetSpecularCaster((pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_SPECULARCASTER) != 0);
    _light->SetName(pClass->_uiName);
    if (pClass->_uiNumFrames > 0)
    {
        _light->SetProjectionTexture(pClass->_puiProjectedTextures[0]);
    }

    _fLightFlickerTimer = 0.0f;
    _fLightFlickerIntensity = 1.0f;
    _lightClass = const_cast<EntityLightClass*>(pClass);

    m_uiAttachFlags |= UPDATE_REQUIRED;
    SetMatrix(desc.mMatrix);
    TurnOn();
}

// FUNCTION: SWBF2 0x004cd260
EntityLight::~EntityLight()
{
    Deinit();
    delete _light;
}

// FUNCTION: SWBF2 0x004cd300
bool EntityLight::Update(float fDeltaTime)
{
    EntityLightClass* pClass = _lightClass;
    if (!pClass || !m_bOn)
    {
        return true;
    }

    if (pClass->_uiNumFrames > 1)
    {
        _light->SetProjectionTexture(
            pClass->_puiProjectedTextures[(int)(GameLoop::GetMissionTime() * pClass->_fFrameRate) %
                                          pClass->_uiNumFrames]);
    }

    float fRandom;
    if (pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_SYNCHRONIZE)
    {
        if ((pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_UPDATEPARITY) &&
            (RedRenderer::GetFrameNumber() & 1))
        {
            pClass->_uiFlags &= ~EntityLightClass::ENTITY_LIGHT_FLAG_UPDATEPARITY;
            pClass->_fRandom = PblRandom::GetGlobalFloat();
        }
        else if (!(pClass->_uiFlags & EntityLightClass::ENTITY_LIGHT_FLAG_UPDATEPARITY) &&
                 !(RedRenderer::GetFrameNumber() & 1))
        {
            pClass->_uiFlags |= EntityLightClass::ENTITY_LIGHT_FLAG_UPDATEPARITY;
            pClass->_fRandom = PblRandom::GetGlobalFloat();
        }
        fRandom = pClass->_fRandom;
    }
    else
    {
        fRandom = PblRandom::GetGlobalFloat();
    }

    switch (pClass->_eLightFlickerType)
    {
    case LightFlickerType_Strobe:
        _fLightFlickerTimer -= fDeltaTime;
        if (_fLightFlickerTimer <= 0.0f)
        {
            if (_fLightFlickerIntensity > 0.0f)
            {
                _fLightFlickerTimer = pClass->_fLightFlickerPeriodOff;
                _fLightFlickerIntensity = 0.0f;
            }
            else
            {
                _fLightFlickerTimer = pClass->_fLightFlickerPeriodOn;
                _fLightFlickerIntensity = 1.0f;
            }
            UpdateColor();
        }
        break;

    case LightFlickerType_StrobeRandom:
        _fLightFlickerTimer -= fDeltaTime;
        if (_fLightFlickerTimer <= 0.0f)
        {
            _fLightFlickerTimer = pClass->_fLightFlickerPeriodOn * fRandom;
            _fLightFlickerIntensity = 1.0f;
        }
        else if (_fLightFlickerTimer < 0.25f)
        {
            _fLightFlickerIntensity = (0.25f - _fLightFlickerTimer) * 4.0f;
        }
        UpdateColor();
        break;

    case LightFlickerType_StrobeFade:
    {
        _fLightFlickerTimer += fDeltaTime;
        const float fPeriod = pClass->_fLightFlickerPeriodOn + pClass->_fLightFlickerPeriodOff;
        if (_fLightFlickerTimer > fPeriod)
        {
            _fLightFlickerTimer = fmod(_fLightFlickerTimer, fPeriod);
        }
        const float fOn = pClass->_fLightFlickerPeriodOn;
        const float fFade = pClass->_fLightFadePeriod;
        if (_fLightFlickerTimer > fOn)
        {
            _fLightFlickerIntensity = 0.0f;
        }
        else if (_fLightFlickerTimer < fFade)
        {
            _fLightFlickerIntensity = _fLightFlickerTimer / fFade;
        }
        else if (_fLightFlickerTimer + fFade > fOn)
        {
            _fLightFlickerIntensity = (fOn - _fLightFlickerTimer) / fFade;
        }
        else
        {
            _fLightFlickerIntensity = 1.0f;
        }
        UpdateColor();
        break;
    }

    case LightFlickerType_Pulse:
    {
        _fLightFlickerTimer += fDeltaTime;
        if (_fLightFlickerTimer > pClass->_fLightFlickerPeriodOn)
        {
            _fLightFlickerTimer = fmod(_fLightFlickerTimer, pClass->_fLightFlickerPeriodOn);
        }
        const float fHalf = pClass->_fLightFlickerPeriodOn * 0.5f;
        if (_fLightFlickerTimer > fHalf)
        {
            _fLightFlickerIntensity = (pClass->_fLightFlickerPeriodOn - _fLightFlickerTimer) / fHalf;
        }
        else
        {
            _fLightFlickerIntensity = _fLightFlickerTimer / fHalf;
        }
        UpdateColor();
        break;
    }

    case LightFlickerType_Flicker:
    {
        _fLightFlickerTimer += fDeltaTime;
        const float fTarget = fRandom * 0.5f + 0.5f;
        if (_fLightFlickerTimer >= pClass->_fLightFlickerPeriodOn)
        {
            _fLightFlickerIntensity = fTarget;
            _fLightFlickerTimer = 0.0f;
        }
        else
        {
            const float t = _fLightFlickerTimer * 0.4f / pClass->_fLightFlickerPeriodOn;
            _fLightFlickerIntensity = (1.0f - t) * _fLightFlickerIntensity + t * fTarget;
        }
        UpdateColor();
        break;
    }
    }
    return true;
}

namespace
{

inline RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * (1.0f / 255.0f);
    v.g = c.g * (1.0f / 255.0f);
    v.b = c.b * (1.0f / 255.0f);
    v.a = c.a * (1.0f / 255.0f);
    return v;
}
}

// FUNCTION: SWBF2 0x004cd6f0
void EntityLight::UpdateColor()
{
    if (m_bOn && _fLightFlickerIntensity > 0.0f)
    {
        if (!_LightBeam.IsEffectActive())
        {
            _LightBeam.ActivateEffect();
            float fRadius = _light->m_eType == RedLight::TYPE_SPOT ? ((RedSpotLight*)_light)->m_fRadius
                                                                   : ((RedOmniLight*)_light)->m_radius;
            if (fRadius > 0.0f)
            {
                _light->Activate();
            }
        }

        if (_fLightFlickerIntensity >= 1.0f)
        {
            if (_light->m_eType == RedLight::TYPE_SPOT)
            {
                ((RedSpotLight*)_light)->SetColor(ToColorValue(_color));
            }
            else
            {
                ((RedOmniLight*)_light)->SetColor(ToColorValue(_color));
            }
            _LightBeam.SetColor(_color);
        }
        else
        {
            RedColor color = Interpolate(RedColor::BLACK, _color, _fLightFlickerIntensity);
            if (_light->m_eType == RedLight::TYPE_SPOT)
            {
                ((RedSpotLight*)_light)->SetColor(ToColorValue(color));
            }
            else
            {
                ((RedOmniLight*)_light)->SetColor(ToColorValue(color));
            }
            _LightBeam.SetColor(color);
        }
    }
    else if (_LightBeam.IsEffectActive())
    {
        _LightBeam.DeactivateEffect();
        if (_light->m_uiFlags & RedLight::LIGHT_FLAG_ACTIVE)
        {
            _light->Deactivate();
        }
    }
}

// FUNCTION: SWBF2 0x004cd940
void EntityLight::SetColor(const RedColor& color)
{
    _color = color;
    UpdateColor();
}

// FUNCTION: SWBF2 0x004cd9b0
void EntityLight::SetOmniLightRadius(float fRadius)
{
    if (_light->m_eType == RedLight::TYPE_SPOT)
    {
        ((RedSpotLight*)_light)->SetRadius(fRadius > 0.0f ? fRadius : 0.0f);
    }
    else
    {
        ((RedOmniLight*)_light)->SetRadius(fRadius > 0.0f ? fRadius : 0.0f);
    }
    UpdateColor();
}

// FUNCTION: SWBF2 0x004cd920
void EntityLight::SetHaloRadius(float fRadius)
{
    _fBeamLength = 0.0f;
    _fBeamWidthX = _fBeamWidthZ = fRadius;
    RecalculateBeam();
}

// FUNCTION: SWBF2 0x004cda00
void EntityLight::RecalculateBeam()
{
    _LightBeam.SetFinalWidth(_fBeamWidthX, _fBeamWidthZ);
    _LightBeam.SetLength((1.0f - _fBeamFadeLength) * _fBeamLength);
    _LightBeam.SetFadeLength(_fBeamLength * _fBeamFadeLength);
}

// FUNCTION: SWBF2 0x004cda80
void EntityLight::TurnOn()
{
    if (!m_bOn)
    {
        m_bOn = true;
        UpdateColor();
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x004cdab0
void EntityLight::TurnOff()
{
    if (m_bOn)
    {
        m_bOn = false;
        UpdateColor();
        FLEffectObject::DeactivateEffect();
    }
}

// FUNCTION: SWBF2 0x004cdae0
bool EntityLight::IsEffectActive() const
{
    return m_bOn;
}

// FUNCTION: SWBF2 0x004cdaf0
void EntityLight::ActivateEffect()
{
    TurnOn();
}

// FUNCTION: SWBF2 0x004cdb00
void EntityLight::DeactivateEffect()
{
    TurnOff();
}

// FUNCTION: SWBF2 0x004cdb10
void EntityLight::Eliminate()
{
    Deinit();
}

// FUNCTION: SWBF2 0x004cdb20
void EntityLight::Init()
{
    _fLightFlickerTimer = 0.0f;
    _fLightFlickerIntensity = 1.0f;
    _LightBeam.SetDrawDistance(200.0f);
    _LightBeam.SetFlareIntensity(0.0f);
    _LightBeam.SetInitialWidth(0.0f);
    _LightBeam.SetFadeFactor(1.0f);
    _LightBeam.SetFadeLength(0.0f);
    _fBeamLength = 0.0f;
    _fBeamWidthX = 0.0f;
    _fBeamWidthZ = 0.0f;
    RecalculateBeam();
    SetColor(RedColor::WHITE);
    SetOmniLightRadius(10.0f);
    m_uiAttachFlags |= UPDATE_REQUIRED;
    SetMatrix(gMatrixIdentity);
    TurnOff();
}

// FUNCTION: SWBF2 0x004cdc30
void EntityLight::Deinit()
{
    m_uiAttachFlags &= ~UPDATE_REQUIRED;
    TurnOff();
}

// FUNCTION: SWBF2 0x004cdc40
void EntityLight::UpdatedWorldMatrix()
{
    if (_light->m_eType == RedLight::TYPE_SPOT)
    {
        ((RedSpotLight*)_light)->SetPosition(m_mWorldMatrix.GetTranslation());
        ((RedSpotLight*)_light)->SetDirection(m_mWorldMatrix.GetZAxis());
    }
    else
    {
        ((RedOmniLight*)_light)->SetPosition(m_mWorldMatrix.GetTranslation());
    }
    RecalculateBeam();
    _LightBeam.AttachEffectToMatrix(&m_mWorldMatrix, false);
}

// FUNCTION: SWBF2 0x004cdc90
EntityLightClass::EntityLightClass(unsigned int uHash)
    : EntityClass(uHash)
    , FLEffectClass()
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    mType = 2;
    _color = RedColor::WHITE;
    _fBeamLength = 0.0f;
    _fBeamWidthX = 0.0f;
    _fBeamWidthZ = 0.0f;
    _fBeamInitialWidthX = 1.0f;
    _fBeamInitialWidthZ = 1.0f;
    _fBeamFadeFactor = 0.2f;
    _fBeamFadeLength = -1.0f;
    _fBeamDrawDistance = 200.0f;
    _fFlareIntensity = 0.0f;
    _fBeamIntensity = 1.0f;
    _uiName = 0;
    _uiFlags = 0x40;
    _eLightFlickerType = 0;
    _fLightFlickerPeriodOn = 0.0f;
    _fLightFlickerPeriodOff = 0.0f;
    _fLightFadePeriod = 1.0f;
    _fOmniRadius = 10.0f;
    _fSpotInnerConeAngle = 1.5707964f * 0.5f;
    _fSpotOuterConeAngle = 1.5707964f;
    _fRandom = 0.0f;
    _fFrameRate = 0.0f;
    _uiNumFrames = 0;
    _puiProjectedTextures = 0;
}

// SYNTHETIC: SWBF2 0x004cf484
// [thunk]:EntityLightClass::`scalar deleting destructor' adjustor{96}
// SYNTHETIC: SWBF2 0x004cddd0
// EntityLightClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x004cde30
EntityLightClass::EntityLightClass(EntityLightClass* pParent, unsigned int uHash)
    : EntityClass(pParent, uHash)
    , FLEffectClass()
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    mType = 2;
    _color = pParent->_color;
    _fBeamLength = pParent->_fBeamLength;
    _fBeamWidthX = pParent->_fBeamWidthX;
    _fBeamWidthZ = pParent->_fBeamWidthZ;
    _fBeamInitialWidthX = pParent->_fBeamInitialWidthX;
    _fBeamInitialWidthZ = pParent->_fBeamInitialWidthZ;
    _fBeamFadeFactor = pParent->_fBeamFadeFactor;
    _fBeamFadeLength = pParent->_fBeamFadeLength;
    _fBeamDrawDistance = pParent->_fBeamDrawDistance;
    _fFlareIntensity = pParent->_fFlareIntensity;
    _fBeamIntensity = pParent->_fBeamIntensity;
    _uiName = pParent->_uiName;
    _uiFlags = pParent->_uiFlags;
    _eLightFlickerType = pParent->_eLightFlickerType;
    _fLightFlickerPeriodOn = pParent->_fLightFlickerPeriodOn;
    _fLightFlickerPeriodOff = pParent->_fLightFlickerPeriodOff;
    _fLightFadePeriod = pParent->_fLightFadePeriod;
    _fOmniRadius = pParent->_fOmniRadius;
    _fSpotInnerConeAngle = pParent->_fSpotInnerConeAngle;
    _fSpotOuterConeAngle = pParent->_fSpotOuterConeAngle;
    _fRandom = pParent->_fRandom;
    _fFrameRate = pParent->_fFrameRate;
    _uiNumFrames = pParent->_uiNumFrames;
    if (pParent->_uiNumFrames)
    {
        _puiProjectedTextures = new unsigned int[pParent->_uiNumFrames];
        memcpy(_puiProjectedTextures, pParent->_puiProjectedTextures,
               pParent->_uiNumFrames * sizeof(unsigned int));
    }
    else
    {
        _puiProjectedTextures = 0;
    }

    if (sLightClassTable.Store(uHash, this))
    {
        ++sLightClassTableNumEntries;
    }
}

// FUNCTION: SWBF2 0x004ce010
void EntityLightClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    int iValue;

    switch (uProperty)
    {
    case 0x09E7F535u:
        if (sscanf(pszValue, "%d", &iValue) == 1 && iValue)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_SHADOWCASTER;
        }
        else
        {
            _uiFlags &= ~ENTITY_LIGHT_FLAG_SHADOWCASTER;
        }
        break;
    case 0x044C2100u:
        if (sscanf(pszValue, "%d", &iValue) == 1 && iValue)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_BIDIRECTIONAL;
        }
        else
        {
            _uiFlags &= ~ENTITY_LIGHT_FLAG_BIDIRECTIONAL;
        }
        break;
    case 0xF946B484u:
        if (sscanf(pszValue, "%d", &iValue) == 1 && iValue)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_SPECULARCASTER;
        }
        else
        {
            _uiFlags &= ~ENTITY_LIGHT_FLAG_SPECULARCASTER;
        }
        break;
    case 0xD290C23Bu:
        if (sscanf(pszValue, "%d", &iValue) == 1 && iValue)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_STATIC;
        }
        else
        {
            _uiFlags &= ~ENTITY_LIGHT_FLAG_STATIC;
        }
        break;
    case 0x7E276E3Du:
        if (sscanf(pszValue, "%d", &iValue) == 1 && iValue)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_SYNCHRONIZE;
        }
        else
        {
            _uiFlags &= ~ENTITY_LIGHT_FLAG_SYNCHRONIZE;
        }
        break;
    case 0x5127F14Du:
        if (PblHash(pszValue) == 0x2305F67Du)
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_TYPE_SPOT;
        }
        else
        {
            _uiFlags |= ENTITY_LIGHT_FLAG_TYPE_OMNI;
        }
        break;
    case 0x8D39BDE6u:
        _uiName = PblHash(pszValue);
        break;

    case 0x3D7E6258u:
    {
        unsigned int r = 255, g = 255, b = 255, a = 255;
        int iCount = sscanf(pszValue, "%u %u %u %u", &r, &g, &b, &a);
        if (iCount == 4)
        {
            RedColor color;
            color.b = (unsigned char)(b * a / 255);
            color.g = (unsigned char)(g * a / 255);
            color.r = (unsigned char)(r * a / 255);
            color.a = 255;
            _color = color;
        }
        else if (iCount == 3)
        {
            RedColor color;
            color.b = (unsigned char)b;
            color.g = (unsigned char)g;
            color.r = (unsigned char)r;
            color.a = 255;
            _color = color;
        }
        break;
    }

    case 0x9342844Eu:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.2f;
        }
        sscanf(pszValue, "%f", &_fBeamLength);
        if (_fBeamLength <= 0.0f)
        {
            _fBeamLength = 0.0f;
        }
        break;
    case 0x6F7F1016u:
    {
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.2f;
        }
        int iCount = sscanf(pszValue, "%f %f", &_fBeamWidthX, &_fBeamWidthZ);
        if (iCount == 2)
        {
            if (_fBeamWidthX == 0.0f || _fBeamWidthZ == 0.0f)
            {
                _fBeamWidthZ = 0.0f;
                _fBeamWidthX = 0.0f;
            }
            else
            {
                _fBeamWidthX = (float)fabs(_fBeamWidthX);
                _fBeamWidthZ = (float)fabs(_fBeamWidthZ);
            }
        }
        else if (iCount == 1)
        {
            _fBeamWidthZ = _fBeamWidthX = (float)fabs(_fBeamWidthX);
        }
        break;
    }
    case 0x9CBB54E2u:
    {
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.2f;
        }
        int iCount = sscanf(pszValue, "%f %f", &_fBeamInitialWidthX, &_fBeamInitialWidthZ);
        if (iCount == 2)
        {
            if (_fBeamInitialWidthX == 0.0f || _fBeamInitialWidthZ == 0.0f)
            {
                _fBeamInitialWidthZ = 0.0f;
                _fBeamInitialWidthX = 0.0f;
            }
            else
            {
                _fBeamInitialWidthX = (float)fabs(_fBeamInitialWidthX);
                _fBeamInitialWidthZ = (float)fabs(_fBeamInitialWidthZ);
            }
        }
        else if (iCount == 1)
        {
            _fBeamInitialWidthZ = _fBeamInitialWidthX = (float)fabs(_fBeamInitialWidthX);
        }
        break;
    }
    case 0x663A891Du:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.5f;
        }
        if (sscanf(pszValue, "%f", &_fBeamWidthX) == 1)
        {
            _fBeamWidthX = _fBeamWidthZ = (float)fabs(_fBeamWidthX);
        }
        break;
    case 0x857435A7u:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.2f;
        }
        sscanf(pszValue, "%f", &_fBeamFadeFactor);
        if (_fBeamFadeFactor < 0.0f)
        {
            _fBeamFadeFactor = 0.0f;
        }
        else if (_fBeamFadeFactor > 1.0f)
        {
            _fBeamFadeFactor = 1.0f;
        }
        break;
    case 0x8807A05Au:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.5f;
        }
        sscanf(pszValue, "%f", &_fBeamFadeFactor);
        if (_fBeamFadeFactor < 0.0f)
        {
            _fBeamFadeFactor = 0.0f;
        }
        else if (_fBeamFadeFactor > 1.0f)
        {
            _fBeamFadeFactor = 1.0f;
        }
        break;
    case 0x2C52F680u:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.2f;
        }
        sscanf(pszValue, "%f", &_fBeamFadeLength);
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.0f;
        }
        else if (_fBeamFadeLength > 1.0f)
        {
            _fBeamFadeLength = 1.0f;
        }
        break;
    case 0xC35D99B9u:
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.5f;
        }
        sscanf(pszValue, "%f", &_fBeamFadeLength);
        if (_fBeamFadeLength < 0.0f)
        {
            _fBeamFadeLength = 0.0f;
        }
        else if (_fBeamFadeLength > 1.0f)
        {
            _fBeamFadeLength = 1.0f;
        }
        break;
    case 0x1612C34Cu:
        sscanf(pszValue, "%f", &_fBeamDrawDistance);
        break;
    case 0x633ED9D4u:
        sscanf(pszValue, "%f", &_fFlareIntensity);
        if (_fFlareIntensity < 0.0f)
        {
            _fFlareIntensity = 0.0f;
        }
        else if (_fFlareIntensity > 1.0f)
        {
            _fFlareIntensity = 1.0f;
        }
        break;
    case 0x75096FABu:
        sscanf(pszValue, "%f", &_fBeamIntensity);
        if (_fBeamIntensity < 0.0f)
        {
            _fBeamIntensity = 0.0f;
        }
        else if (_fBeamIntensity > 1.0f)
        {
            _fBeamIntensity = 1.0f;
        }
        break;

    case 0x3685BB77u:
        if (pszValue[0] >= '0' && pszValue[0] <= '9')
        {
            unsigned int uType;
            sscanf(pszValue, "%d", &uType);
            _eLightFlickerType = uType < EntityLight::NumLightFlickerTypes
                                     ? uType
                                     : EntityLight::LightFlickerType_None;
        }
        else
        {
            switch (PblHash(pszValue))
            {
            case 0xC33D6F68u:
                _eLightFlickerType = EntityLight::LightFlickerType_Strobe;
                break;
            case 0xF56C83DFu:
                _eLightFlickerType = EntityLight::LightFlickerType_StrobeRandom;
                break;
            case 0x135752DEu:
                _eLightFlickerType = EntityLight::LightFlickerType_StrobeFade;
                break;
            case 0x550BFE9Eu:
                _eLightFlickerType = EntityLight::LightFlickerType_Pulse;
                break;
            case 0x0D7C428Fu:
                _eLightFlickerType = EntityLight::LightFlickerType_Flicker;
                break;
            default:
                _eLightFlickerType = EntityLight::LightFlickerType_None;
                break;
            }
        }
        break;
    case 0xD92EA25Eu:
        sscanf(pszValue, "%f %f", &_fLightFlickerPeriodOn, &_fLightFlickerPeriodOff);
        break;
    case 0x65D177CAu:
        sscanf(pszValue, "%f", &_fLightFadePeriod);
        break;
    case 0xC2C1813Au:
        sscanf(pszValue, "%f", &_fOmniRadius);
        break;
    case 0xC967096Bu:
        sscanf(pszValue, "%f", &_fSpotInnerConeAngle);
        _fSpotInnerConeAngle = _fSpotInnerConeAngle * 3.1415927f / 180.0f;
        break;
    case 0xB22D429Eu:
        sscanf(pszValue, "%f", &_fSpotOuterConeAngle);
        _fSpotOuterConeAngle = _fSpotOuterConeAngle * 3.1415927f / 180.0f;
        break;

    case 0x99E6DBFEu:
    {
        char szName[128];
        char szFrame[128];
        if (sscanf(pszValue, "%s %d", szName, &_uiNumFrames) == 1)
        {
            _uiNumFrames = 1;
        }
        _puiProjectedTextures = new unsigned int[_uiNumFrames];
        if (_uiNumFrames == 1)
        {
            _puiProjectedTextures[0] = PblHash(szName);
        }
        else
        {
            for (unsigned int i = 0; i < _uiNumFrames; ++i)
            {
                sprintf(szFrame, "%s%d", szName, _uiNumFrames);
                _puiProjectedTextures[i] = PblHash(szFrame);
            }
        }
        break;
    }
    case 0x9B111AE4u:
        sscanf(pszValue, "%f", &_fFrameRate);
        if (_fFrameRate < 0.0f)
        {
            _fFrameRate = 0.0f;
        }
        break;

    default:
        EntityClass::SetProperty(uProperty, pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x004ce8b0
Entity* EntityLightClass::Build(const EntityDesc& desc)
{
    return new EntityLight(this, desc);
}

// FUNCTION: SWBF2 0x004cea30
FLEffectObject* EntityLightClass::BuildEffect()
{
    EntityDesc desc;
    return new EntityLight(this, desc);
}

// FUNCTION: SWBF2 0x004ce9c0
EntityClass* EntityLightClass::Derive(unsigned int uHash)
{
    return new EntityLightClass(this, uHash);
}

// FUNCTION: SWBF2 0x004ceb00
void EntityLightClass::Reset()
{
    sLightClassTable.Clear();
    sLightClassTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x004ceb20
void EntityLightFXParser::Create()
{
    s_pInstance = new EntityLightFXParser();
}

// FUNCTION: SWBF2 0x004cebb0
void EntityLightFXParser::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x004cd960
void EntityLight::SetFlareIntensity(float fIntensity)
{
    _LightBeam.SetFlareIntensity(fIntensity);
}

// LIBRARY: SWBF2 0x00410860
// fabs
