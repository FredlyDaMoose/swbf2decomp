#include "LightBeam.h"
#include "Battlefront2Lod.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMath.h"

// GLOBAL: SWBF2 0x01e573dc
LightBeamEffect* LightBeamEffect::s_pInstance;

// FUNCTION: SWBF2 0x0056fcc0
void LightBeamEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new LightBeamEffect();
    }
}

// FUNCTION: SWBF2 0x0056fd70
void LightBeamEffect::Init()
{
    _PlatformInit();
}

// FUNCTION: SWBF2 0x0056fa90
void LightBeamEffect::Render(int)
{
}

// FUNCTION: SWBF2 0x0056faa0
LightBeamEffect::~LightBeamEffect()
{
}

// FUNCTION: SWBF2 0x0056fd50
void LightBeamEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0056fd80
void LightBeamEffect::Cleanup()
{
    _PlatformCleanup();
}

// FUNCTION: SWBF2 0x0056fd90
void LightBeamEffect::ReadConfig(PblConfig&)
{
}

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

namespace
{

inline unsigned char ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    return (unsigned char)(int)f;
}
}

// FUNCTION: SWBF2 0x0056fda0
FLEffectClass* LightBeamEffect::BuildClass(PblConfig& config) const
{
    LightBeam* pBeam = new LightBeam();

    bool bHeadTail = false;
    bool bOmni = true;
    PblVector3 vPosition(0.0f, 0.0f, 0.0f);
    PblVector3 vTailPosition(0.0f, 0.0f, 0.0f);

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x934F4E0Au:
            vPosition = PblVector3(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            pBeam->SetHeadPosition(vPosition);
            break;
        case 0x81932130u:
            bHeadTail = true;
            bOmni = false;
            vTailPosition = PblVector3(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            break;
        case 0x83D03615u:
            pBeam->SetLength(data.GetFloatArg(0));
            if (data.GetFloatArg(0) > 0.0f)
            {
                bOmni = false;
            }
            break;
        case 0xDF6DC76Au:
            pBeam->SetDirection(PblVector3(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2)));
            bOmni = false;
            break;
        case 0xA939A4BCu:
            pBeam->_fFadeAmount = data.GetFloatArg(0);
            break;
        case 0xAA671039u:
            if (data.GetNumArgs() > 1)
            {
                pBeam->SetFinalWidth(data.GetFloatArg(0), data.GetFloatArg(1));
            }
            else
            {
                pBeam->SetFinalWidth(data.GetFloatArg(0));
            }
            break;
        case 0x75096FABu:
            pBeam->_fBeamIntensity = data.GetFloatArg(0);
            break;
        case 0x5AE03A9Eu:
            pBeam->_fFlareCosAngle = PblMath::Cos(data.GetFloatArg(0) * 0.017453292f);
            break;
        case 0x633ED9D4u:
        {
            pBeam->_fFlareIntensity = data.GetFloatArg(0);
            RedColor color = pBeam->_color;
            color.a = (unsigned char)(int)(pBeam->_color.a * pBeam->_fFlareIntensity);
            pBeam->_Flare._color = color;
            break;
        }
        case 0x3D7E6258u:
        {
            RedColor color;
            color.r = ClampColorComponent(data.GetFloatArg(0));
            color.g = ClampColorComponent(data.GetFloatArg(1));
            color.b = ClampColorComponent(data.GetFloatArg(2));
            color.a = data.GetNumArgs() > 3 ? ClampColorComponent(data.GetFloatArg(3)) : 0xff;
            pBeam->_color = color;
            color.a = (unsigned char)(int)(pBeam->_color.a * pBeam->_fFlareIntensity);
            pBeam->_Flare._color = color;
            break;
        }
        case 0x1414BF95u:
            if (data.GetNumArgs() > 1)
            {
                pBeam->SetInitialWidth(data.GetFloatArg(0), data.GetFloatArg(1));
            }
            else
            {
                pBeam->SetInitialWidth(data.GetFloatArg(0));
            }
            break;
        case 0x1612C34Cu:
            pBeam->_fSquaredDrawDistance = data.GetFloatArg(0) * data.GetFloatArg(0);
            pBeam->_Flare._fSquaredDrawDistance = pBeam->_fSquaredDrawDistance;
            break;
        case 0x20C351FBu:
            pBeam->SetFadeLength(data.GetFloatArg(0));
            break;
        }
    }

    if (bHeadTail)
    {
        pBeam->SetHeadTailPosition(vPosition, vTailPosition);
    }
    else if (bOmni)
    {
        pBeam->SetPositionOmni(vPosition);
    }
    return pBeam;
}

// FUNCTION: SWBF2 0x0056fb10
LightBeam::LightBeam(const LightBeam& that)
    : _Flare(that._Flare)
    , _vHeadPosition(that._vHeadPosition)
    , _vDirection(that._vDirection)
    , _color(that._color)
    , _fLength(that._fLength)
    , _fHeadSizeX(that._fHeadSizeX)
    , _fHeadSizeZ(that._fHeadSizeZ)
    , _fTailSizeX(that._fTailSizeX)
    , _fTailSizeZ(that._fTailSizeZ)
    , _fFadeLength(that._fFadeLength)
    , _fFadeAmount(that._fFadeAmount)
    , _fSquaredDrawDistance(that._fSquaredDrawDistance)
    , _fFlareIntensity(that._fFlareIntensity)
    , _fBeamIntensity(that._fBeamIntensity)
    , _fFlareCosAngle(0.5f)
{
}

ManagedLightBeam::ManagedLightBeam(const LightBeam& beam)
    : RedSceneObject(&largeEffectLodData)
    , LightBeam(beam)
    , _uiHideInView(0)
{
}

// FUNCTION: SWBF2 0x00570260
FLEffectObject* LightBeam::BuildEffect()
{
    ManagedLightBeam* pBeam = new ManagedLightBeam(*this);
    pBeam->ActivateEffect();
    return pBeam;
}

// FUNCTION: SWBF2 0x00570310
bool LightBeam::IsEffectActive() const
{
    return _Flare.IsEffectActive();
}

// FUNCTION: SWBF2 0x00570330
void LightBeam::ActivateEffect()
{
    if (!IsEffectActive())
    {
        _Flare.ActivateEffect();
        if (_fLength > 0.0f || (_fTailSizeX > 0.0f && _fTailSizeZ > 0.0f))
        {
            AddToRenderList();
            FLEffectObject::ActivateEffect();
        }
        else
        {
            RedColor color = _color;
            color.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
            _Flare._color = color;
            _Flare._fSize = _fHeadSizeZ * 0.5f + _fHeadSizeX * 0.5f;
        }
    }
}

// FUNCTION: SWBF2 0x00570410
void LightBeam::DeactivateEffect()
{
    if (IsEffectActive())
    {
        _Flare.DeactivateEffect();
        if (_fLength > 0.0f || (_fTailSizeX > 0.0f && _fTailSizeZ > 0.0f))
        {
            RemoveFromRenderList();
            FLEffectObject::DeactivateEffect();
        }
    }
}

// FUNCTION: SWBF2 0x00570470
void LightBeam::SetLength(float fLength)
{
    if (IsEffectActive() && !(_fTailSizeX > 0.0f && _fTailSizeZ > 0.0f))
    {
        bool bWasBeam = _fLength > 0.0f;
        bool bIsBeam = fLength > 0.0f;
        if (bWasBeam)
        {
            if (!bIsBeam)
            {
                RedColor color = _color;
                color.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
                _Flare._color = color;
                _Flare._fSize = _fHeadSizeZ * 0.5f + _fHeadSizeX * 0.5f;
                RemoveFromRenderList();
                FLEffectObject::DeactivateEffect();
            }
        }
        else if (bIsBeam)
        {
            AddToRenderList();
            FLEffectObject::ActivateEffect();
        }
    }
    _fLength = fLength > 0.0f ? fLength : 0.0f;
}

// FUNCTION: SWBF2 0x00570590
void LightBeam::SetFinalWidth(float fWidth)
{
    bool bIsBeam = fWidth > 0.0f;
    if (IsEffectActive() && !(_fLength > 0.0f))
    {
        if (_fTailSizeX > 0.0f && _fTailSizeZ > 0.0f)
        {
            if (!bIsBeam)
            {
                RedColor color = _color;
                color.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
                _Flare._color = color;
                _Flare._fSize = _fHeadSizeZ * 0.5f + _fHeadSizeX * 0.5f;
                RemoveFromRenderList();
                FLEffectObject::DeactivateEffect();
            }
        }
        else if (bIsBeam)
        {
            AddToRenderList();
            FLEffectObject::ActivateEffect();
        }
    }
    float fSize = bIsBeam ? fWidth * 0.5f : 0.0f;
    _fTailSizeZ = fSize;
    _fTailSizeX = fSize;
}

// FUNCTION: SWBF2 0x005706c0
void LightBeam::SetFinalWidth(float fWidthX, float fWidthZ)
{
    bool bIsBeam = fWidthX > 0.0f && fWidthZ > 0.0f;
    if (IsEffectActive() && !(_fLength > 0.0f))
    {
        if (_fTailSizeX > 0.0f && _fTailSizeZ > 0.0f)
        {
            if (!bIsBeam)
            {
                RedColor color = _color;
                color.a = (unsigned char)(int)(_color.a * _fFlareIntensity);
                _Flare._color = color;
                _Flare._fSize = _fHeadSizeZ * 0.5f + _fHeadSizeX * 0.5f;
                RemoveFromRenderList();
                FLEffectObject::DeactivateEffect();
            }
        }
        else if (bIsBeam)
        {
            AddToRenderList();
            FLEffectObject::ActivateEffect();
        }
    }
    _fTailSizeX = bIsBeam ? fWidthX * 0.5f : 0.0f;
    _fTailSizeZ = bIsBeam ? fWidthZ * 0.5f : 0.0f;
}

// FUNCTION: SWBF2 0x00570820
void LightBeam::UpdatedWorldMatrix()
{
    PblVector3 vPos;
    D3DXVec3TransformCoord(&vPos, &_vHeadPosition, &m_mWorldMatrix);
    _Flare.SetPosition(vPos);
}

// FUNCTION: SWBF2 0x00570860
void LightBeam::GetLocalBoundingSphere(PblVector3& vCenter, float& fRadius) const
{
    if (_fLength <= 0.0f)
    {
        vCenter.x = 0.0f;
        vCenter.y = 0.0f;
        vCenter.z = 0.0f;
        fRadius = _fFadeLength + _fTailSizeX;
        return;
    }

    float fHead = _fHeadSizeX;
    if (_fHeadSizeZ > fHead)
    {
        fHead = _fHeadSizeZ;
    }
    float fTail = _fTailSizeX;
    if (_fTailSizeZ > fTail)
    {
        fTail = _fTailSizeZ;
    }
    const float fTotal = _fFadeLength + _fLength;
    const float fHead2 = fHead * fHead;

    const float fFar = fTotal * 0.5f - fHead2 / (fTotal * 8.0f);
    const float fNear = (fTail * fTail - fHead2) / (_fLength * 8.0f) + _fLength * 0.5f;
    const float fFarRadius = fTotal - fFar;
    const float fNearRadius2 = fNear * fNear + fHead2 * 0.25f;
    if (fFarRadius * fFarRadius > fNearRadius2)
    {
        fRadius = fFarRadius;
        vCenter.x = _vDirection.x * fFar;
        vCenter.y = _vDirection.y * fFar;
        vCenter.z = _vDirection.z * fFar;
    }
    else
    {
        fRadius = sqrtf(fNearRadius2);
        vCenter.x = _vDirection.x * fNear;
        vCenter.y = _vDirection.y * fNear;
        vCenter.z = _vDirection.z * fNear;
    }
}
