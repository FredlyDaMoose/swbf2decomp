#include "Battlefront2/Source/SunFlare.h"
#include "pcSunFlare.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/PC/pcRedVertexBuffer.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcClearShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcFlareShader.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcSampleShader.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/RedWater.h"
#include "RedEngineFL/Graphics/RedCamera.h"

namespace FLRenderer
{
float GetNearSceneFadeStart();
float GetFarSceneRange();
}

RedColor ReadColor(const PblConfig::Data& data);

// GLOBAL: SWBF2 0x01e2fd4c
SunFlareEffect* SunFlareEffect::s_pInstance;

// SYNTHETIC: SWBF2 0x0042d0d0
// SunFlareEffect::`scalar deleting destructor'
SunFlareEffect::~SunFlareEffect()
{
}

// FUNCTION: SWBF2 0x0042d140
void SunFlareEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new SunFlareEffect();
    }
}

// GLOBAL: SWBF2 0x01eaffc8
// SYNTHETIC: SWBF2 0x00402120
// `dynamic initializer for 's_SunSkyRenderable''
// SYNTHETIC: SWBF2 0x00766d00
// `dynamic atexit destructor for 's_SunSkyRenderable''
SunSkyRenderable s_SunSkyRenderable;

// GLOBAL: SWBF2 0x01e2fd54
static RedTexture* s_sampleTex;
// GLOBAL: SWBF2 0x01e2fd50
static RedTexture* s_intensityTex;
// GLOBAL: SWBF2 0x01f9af38
static pcClearShader* s_pClearShader;
// GLOBAL: SWBF2 0x01f9af34
static pcSampleShader* s_pSampleShader;
// GLOBAL: SWBF2 0x01f9af74
static pcRedPrimitive* s_samplePoints;

// GLOBAL: SWBF2 0x01f9aeac
static pcFlareShader* s_pFlareOutShader;
// GLOBAL: SWBF2 0x01f9af78
static pcFlareShader* s_pRingShader;
// GLOBAL: SWBF2 0x01f9af3c
static pcFlareShader* s_pSunAndHaloShader;

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);

// GLOBAL: SWBF2 0x007eb748
static PblBox s_boundingBox(-3000.0f, -3000.0f, -3000.0f, 3000.0f, 3000.0f, 3000.0f);

struct PosXYZ
{
    float x;
    float y;
    float z;
};

struct TexCoord
{
    float u;
    float v;
};

struct FlareVertex
{
    PosXYZ pos;
    RedColor color;
    TexCoord uv;
};
 

struct CompressedTexCoord
{
    short u;
    short v;
};
struct FlareCompressedVertex
{
    CompressedPos pos;
    RedColor color;
    CompressedTexCoord uv;
};
 

template <class VERTEX, int RVF>
bool CreateTextures(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                    SunFlare::Rect& rect, bool bSunAndHalo);
template <class VERTEX, int RVF>
void RenderFlareOuts(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                     const SunFlare::Rect& rect, float fFlareOutSize, int iNumFlareOuts,
                     unsigned char initialAlpha);
template <class VERTEX>
void EmitRing(VERTEX*& pVertices, const PblVector3& vPosition, float, float, float, float,
              const RedColor&, const RedColor&, const RedColor&, int, float, float);

static void EmitSunRings(const SunFlare& flare, FlareVertex*& pVertices,
                         const PblVector3& vPosition, float fTexU, float fTexV);
static void EmitSunRings(const SunFlare& flare, FlareCompressedVertex*& pVertices,
                         const PblVector3& vPosition, float fTexU, float fTexV);

// GLOBAL: SWBF2 0x01faa7bc
float sFirstRingRadiusCompressed;
// GLOBAL: SWBF2 0x01faa7c0
unsigned int sFirstRingFlagsCompressed;
// GLOBAL: SWBF2 0x01faa7c4
float sFirstRingRadiusUncompressed;
// GLOBAL: SWBF2 0x01faa7c8
unsigned int sFirstRingFlagsUncompressed;
// GLOBAL: SWBF2 0x01faa450
unsigned char sFirstRingAlphaUncompressed;
// GLOBAL: SWBF2 0x01faa451
unsigned char sFirstRingAlphaCompressed;

static void CompressedPos_SetBoundingBox(const PblBox& box)
{
    PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
    PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                box._vMax.z - box._vMin.z };
    CompressedPos::s_vMin = vMin;
    CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
    CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
    CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
    CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;
}

static short CompressedPos_Encode(float f, float fMin, float fScaleInv)
{
    return (short)((f - fMin) * fScaleInv - 32767.5f);
}

// FUNCTION: SWBF2 0x0040cc80
void CompressedPos::SetPos(float fX, float fY, float fZ)
{
    x = CompressedPos_Encode(fX, CompressedPos::s_vMin.x,
                            CompressedPos::s_vScaleInv.x);
    y = CompressedPos_Encode(fY, CompressedPos::s_vMin.y,
                            CompressedPos::s_vScaleInv.y);
    z = CompressedPos_Encode(fZ, CompressedPos::s_vMin.z,
                            CompressedPos::s_vScaleInv.z);
}

static void SetSunFlareVertex(FlareVertex& v, float x, float y, float z,
                              const RedColor& color, float u, float texV)
{
    v.pos.x = x; v.pos.y = y; v.pos.z = z;
    v.color = color;
    v.uv.u = u; v.uv.v = texV;
}

static void SetSunFlareVertex(FlareCompressedVertex& v, float x, float y, float z,
                              const RedColor& color, float u, float texV)
{
    v.pos.SetPos(x, y, z);
    v.color = color;
    v.uv.u = (short)(u * 2048.0f);
    v.uv.v = (short)(texV * 2048.0f);
}

static void SetSunFlareDiscVertex(FlareVertex& v, float x, float y, float z)
{
    v.pos.x = x; v.pos.y = y; v.pos.z = z;
    v.color = RedColor::WHITE;
}

static void SetSunFlareDiscVertex(FlareCompressedVertex& v, float x, float y, float z)
{
    v.pos.SetPos(x, y, z);
    v.color = RedColor::WHITE;
}

// FUNCTION: SWBF2 0x0042d8b0
void SunFlareEffect::Init()
{
    s_sampleTex = new RedTexture(16, 16, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                 D3DPOOL_DEFAULT, 0);
    s_intensityTex = new RedTexture(2, 2, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                                    D3DPOOL_DEFAULT, 0);
    s_pClearShader = (pcClearShader*)pcRedShader::Create(
        PblHash("Clear"), 0x203, 0, 0, 0x80000, RedColor::WHITE, RedColor::BLACK, 32.0f);

    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 2, 12, 256, 0, true);
        float* v = (float*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = (float)i;
                v[1] = (float)j;
                v[2] = 0.0f;
                v += 3;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareOutShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x282, 0, 0, 0x30000, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            PblHash("sample"), 2, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader->SetInputTexture(s_sampleTex);
        s_pSampleShader->SetScale(1.0f / 120.0f);
        s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
        s_pRingShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pRingShader->SetTexture(s_intensityTex);
        s_pSunAndHaloShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x82, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }
    else
    {
        s_samplePoints = pcRedPrimitive::Create(D3DPT_POINTLIST, 0x9282, 8, 256, 0, true);
        const PblBox box(0.0f, 0.0f, 0.0f, 15.0f, 15.0f, 0.0f);
        s_samplePoints->m_bbox = box;
        CompressedPos_SetBoundingBox(box);
        short* v = (short*)s_samplePoints->BeginVertexData();
        if (!v)
        {
            return;
        }
        for (int i = 0; i < 16; i++)
        {
            for (int j = 0; j < 16; j++)
            {
                v[0] = CompressedPos_Encode((float)i, CompressedPos::s_vMin.x,
                                            CompressedPos::s_vScaleInv.x);
                v[1] = CompressedPos_Encode((float)j, CompressedPos::s_vMin.y,
                                            CompressedPos::s_vScaleInv.y);
                v[2] = CompressedPos_Encode(0.0f, CompressedPos::s_vMin.z,
                                            CompressedPos::s_vScaleInv.z);
                v += 4;
            }
        }
        s_samplePoints->EndVertexData();

        s_pFlareOutShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x9282, 0, 0, 0x30000, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader = (pcSampleShader*)pcRedShader::Create(
            PblHash("sample"), 0x1002, 0, 0, 0, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pSampleShader->SetInputTexture(s_sampleTex);
        s_pSampleShader->SetScale(1.0f / 120.0f);
        s_pSampleShader->SetOutputPosition(0.0f, 0.0f);
        s_pRingShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x9282, 0, 0, 0x130040, RedColor::WHITE, RedColor::BLACK, 32.0f);
        s_pRingShader->SetTexture(s_intensityTex);
        s_pSunAndHaloShader = (pcFlareShader*)pcRedShader::Create(
            PblHash("flare"), 0x1082, 0, 0, 0x10040, RedColor::WHITE, RedColor::BLACK, 32.0f);
    }

    s_SunSkyRenderable.Activate(0);
}

// FUNCTION: SWBF2 0x0042d1e0
void SunFlareEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0042d200
void SunSkyRenderable::Render(int value)
{
    if (SunFlareEffect::s_pInstance)
    {
        SunFlareEffect::s_pInstance->RenderSky(value);
    }
}

// FUNCTION: SWBF2 0x0042d2f0
FLEffectClass* SunFlareEffect::BuildClass(PblConfig& config) const
{
    SunFlare* pFlare = new SunFlare();
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3D7E6258u:
            pFlare->SetColor(ReadColor(data));
            break;
        case 0x23A0D95Cu:
            pFlare->SetSize(data.GetFloatArg(0));
            break;
        case 0x4D609778u:
            pFlare->SetFlareOutSize(data.GetFloatArg(0));
            break;
        case 0x9EBE759Fu:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloInnerRing(data.GetFloatArg(0), color);
            break;
        }
        case 0x5671AC28u:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloMiddleRing(data.GetFloatArg(0), color);
            break;
        }
        case 0x59B407CEu:
        {
            RedColor color;
            color.r = data.GetUint8Arg(1);
            color.g = data.GetUint8Arg(2);
            color.b = data.GetUint8Arg(3);
            color.a = data.GetNumArgs() > 4 ? data.GetUint8Arg(4) : 0xff;
            pFlare->SetHaloOutterRing(data.GetFloatArg(0), color);
            break;
        }
        case 0xB9BAA468u:
        {
            RedColor color;
            color.r = data.GetUint8Arg(0);
            color.g = data.GetUint8Arg(1);
            color.b = data.GetUint8Arg(2);
            color.a = data.GetNumArgs() > 3 ? data.GetUint8Arg(3) : 0xff;
            pFlare->SetSpikeColor(color);
            break;
        }
        case 0xACAAECACu:
            pFlare->SetSpikeSize(data.GetFloatArg(0));
            break;
        case 0xAD544418u:
        {
            float fPitch = data.GetFloatArg(0) * 0.017453292f;
            float fYaw = data.GetNumArgs() > 1 ? data.GetFloatArg(1) * 0.017453292f : 0.0f;
            float fSinPitch, fCosPitch, fSinYaw, fCosYaw;
            PblMath::SinCos(&fSinPitch, &fCosPitch, fPitch);
            PblVector3 v(fSinPitch, -fCosPitch, 0.0f);
            PblMath::SinCos(&fSinYaw, &fCosYaw, fYaw);
            PblVector3 vDirection(fCosYaw * v.x - fSinYaw * v.z, v.y,
                                  fSinYaw * v.x + fCosYaw * v.z);
            pFlare->SetDirection(vDirection);
            RedWater::SetSpecularDirection(
                PblVector3(-vDirection.x, -vDirection.y, -vDirection.z));
            break;
        }
        case 0xE3C8D91Eu:
            pFlare->SetFlareEnable(data.GetInt32Arg(0) != 0);
            break;
        case 0xE453BE48u:
            pFlare->SetNumFlareOuts(data.GetInt32Arg(0));
            break;
        }
    }
    pFlare->ActivateEffect();
    return 0;
}

// FUNCTION: SWBF2 0x0042dd30
void SunFlareEffect::Cleanup()
{
    if (s_samplePoints)
    {
        s_samplePoints->Release();
        s_samplePoints = 0;
    }
    delete s_sampleTex;
    delete s_intensityTex;
    if (s_pClearShader)
    {
        s_pClearShader->Release();
        s_pClearShader = 0;
    }
    if (s_pFlareOutShader)
    {
        s_pFlareOutShader->Release();
        s_pFlareOutShader = 0;
    }
    if (s_pSampleShader)
    {
        s_pSampleShader->Release();
        s_pSampleShader = 0;
    }
    if (s_pRingShader)
    {
        s_pRingShader->Release();
        s_pRingShader = 0;
    }
    if (s_pSunAndHaloShader)
    {
        s_pSunAndHaloShader->Release();
        s_pSunAndHaloShader = 0;
    }
    _List.Cleanup();
    s_SunSkyRenderable.Deactivate();
}

// FUNCTION: SWBF2 0x0042def0
void SunFlareEffect::Render(int value)
{
    if (value & 0x200000)
    {
        return;
    }
    for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
    {
        it->_Render();
    }
}

// FUNCTION: SWBF2 0x0042df30
void SunFlareEffect::RenderSky(unsigned int uiPass) const
{
    if (RedRenderer::pcGetGeometryMode() != GEOMETRYMODE_UNCOMPRESSED)
    {
        for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
        {
            it->RenderSunAndHalo<FlareCompressedVertex, 37506>(uiPass);
        }
    }
    else
    {
        for (PblList<SunFlare>::Itor it = _List.Begin(); it != _List.End(); ++it)
        {
            it->RenderSunAndHalo<FlareVertex, 642>(uiPass);
        }
    }
}

// FUNCTION: SWBF2 0x0042dfa0
void SunFlareEffect::SunFlareList::Cleanup()
{
    for (Itor it = Begin(); it != End(); ++it)
    {
        delete (SunFlare*)it;
    }
    RemoveAll();
}

// FUNCTION: SWBF2 0x0042e000
void SunFlareEffect::SunFlareList::Add(SunFlare* pFlare)
{
    PblList<SunFlare>::Add(&pFlare->_Node, pFlare);
}

// FUNCTION: SWBF2 0x0042e030
void SunFlareEffect::SunFlareList::Remove(SunFlare* pFlare)
{
    PblList<SunFlare>::Remove(&pFlare->_Node);
}

// FUNCTION: SWBF2 0x0042e070
FLEffectObject* SunFlare::BuildEffect()
{
    SunFlare* pFlare = new SunFlare(*this);
    pFlare->ActivateEffect();
    return pFlare;
}

// FUNCTION: SWBF2 0x0042e0e0
bool SunFlare::IsEffectActive() const
{
    return _Node.IsInList();
}

// FUNCTION: SWBF2 0x0042e0f0
void SunFlare::ActivateEffect()
{
    if (!IsEffectActive())
    {
        SunFlareEffect::s_pInstance->_List.Add(this);
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x0042e120
void SunFlare::DeactivateEffect()
{
    if (IsEffectActive())
    {
        SunFlareEffect::s_pInstance->_List.Remove(this);
        FLEffectObject::DeactivateEffect();
    }
}

static RedColor MakeColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    RedColor color;
    color.r = r;
    color.g = g;
    color.b = b;
    color.a = a;
    return color;
}

// FUNCTION: SWBF2 0x0042e150
SunFlare::SunFlare()
    : _fSize(1.0f)
    , _color(RedColor::WHITE)
    , _initialFlareOutAlpha(70)
    , _spikeColor(MakeColor(255, 255, 0, 128))
    , _flareEnable(true)
{
    _sunHaloColor[0] = MakeColor(255, 255, 0, 255);
    _sunHaloColor[1] = MakeColor(255, 255, 0, 128);
    _sunHaloColor[2] = MakeColor(255, 255, 0, 0);
    _sunHaloSize[0] = 0.0f;
    _sunHaloSize[1] = 5.0f;
    _sunHaloSize[2] = 40.0f;
    m_sunTex = new RedTexture(256, 256, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                              D3DPOOL_DEFAULT, 0);
}

// SYNTHETIC: SWBF2 0x0042e320
// SunFlare::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0042e350
SunFlare::SunFlare(const SunFlare& that)
    : FLEffectClass()
    , FLEffectObject()
    , _fSize(that._fSize)
    , _color(that._color)
    , _fFlareOutSize(that._fFlareOutSize)
    , _iNumFlareOuts(that._iNumFlareOuts)
    , _flareEnable(that._flareEnable)
{
    m_sunTex = new RedTexture(256, 256, D3DFMT_A8R8G8B8, 1, D3DUSAGE_RENDERTARGET,
                              D3DPOOL_DEFAULT, 0);
}

// SYNTHETIC: SWBF2 0x00433dd8
// [thunk]:SunFlare::`scalar deleting destructor' adjustor{40}
// SYNTHETIC: SWBF2 0x00433de0
// [thunk]:SunFlare::`scalar deleting destructor' adjustor{16}
// FUNCTION: SWBF2 0x0042e4d0
SunFlare::~SunFlare()
{
    delete m_sunTex;
    DeactivateEffect();
}

// FUNCTION: SWBF2 0x0042e560
const PblMatrix& SunFlare::GetMatrix() const
{
    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const_cast<SunFlare*>(this)->SetPosition(
        PblVector3(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z));
    return m_mWorldMatrix;
}

// FUNCTION: SWBF2 0x0042e5f0
const PblVector3& SunFlare::GetPosition() const
{
    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    const_cast<SunFlare*>(this)->SetPosition(
        PblVector3(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z));
    return m_mWorldMatrix.GetTranslation();
}

// FUNCTION: SWBF2 0x0042e680
float SunFlare::GetSize() const
{
    float fZoom = RedRenderer::GetCamera()->GetZoom();
    return GetDistance() * _fSize * 0.00833f / fZoom;
}

// FUNCTION: SWBF2 0x0042e6c0
float SunFlare::GetDistance()
{
    float fDistance;
    if (FLRenderer::GetFarSceneRange() > FLRenderer::GetNearSceneFadeStart())
    {
        fDistance = FLRenderer::GetFarSceneRange() * 0.9f;
    }
    else
    {
        float fNear, fFar, fWidth, fHeight;
        RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
        fDistance = fFar * 0.9f;
    }
    if (fDistance > 2000.0f)
    {
        fDistance = 2000.0f;
    }
    return fDistance;
}

// FUNCTION: SWBF2 0x0042e960
void sub_42E960()
{
}

// FUNCTION: SWBF2 0x0042e740
void SunFlare::_Render() const
{
    if (!_flareEnable)
    {
        return;
    }
    const PblVector3& vAxis = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    if (_vDirection.x * vAxis.x + _vDirection.y * vAxis.y + _vDirection.z * vAxis.z > 0.0f)
    {
        return;
    }

    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblVector3 vPosition(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z);
    PblVector3 vView;
    D3DXVec3TransformCoord(&vView, &vPosition, &RedRenderer::GetCamera()->GetMatrixInverse());
    vPosition = vView;

    float fSize = _fSize * fDistance * 0.00833f / RedRenderer::GetCamera()->GetZoom();
    float fFlareOutSize = _fFlareOutSize * fDistance * 0.00833f;
    Rect rect;
    if (RedRenderer::pcGetGeometryMode() == GEOMETRYMODE_UNCOMPRESSED)
    {
        if (!CreateTextures<FlareVertex, 642>(m_sunTex, vPosition, fSize, rect, false))
        {
            return;
        }
        RenderFlareOuts<FlareVertex, 642>(m_sunTex, vPosition, fSize, rect, fFlareOutSize,
                                          _iNumFlareOuts, _initialFlareOutAlpha);
        RenderSunRings<FlareVertex, 642>(vPosition, fSize, 0.0f, 0.0f);
    }
    else
    {
        if (!CreateTextures<FlareCompressedVertex, 37506>(m_sunTex, vPosition, fSize, rect, false))
        {
            return;
        }
        RenderFlareOuts<FlareCompressedVertex, 37506>(m_sunTex, vPosition, fSize, rect,
                                                      fFlareOutSize, _iNumFlareOuts,
                                                      _initialFlareOutAlpha);
        RenderSunRings<FlareCompressedVertex, 37506>(vPosition, fSize, 0.0f, 0.0f);
    }
}

// TEMPLATE: SWBF2 0x0042e970
// SunFlare::RenderSunAndHalo<FlareVertex,642>
// FUNCTION: SWBF2 0x0042ed30
template <class VERTEX, int RVF>
void SunFlare::RenderSunAndHalo(unsigned int uiPass) const
{
    const PblVector3& vAxis = RedRenderer::GetCamera()->GetMatrix().GetZAxis();
    if (_vDirection.x * vAxis.x + _vDirection.y * vAxis.y + _vDirection.z * vAxis.z > 0.0f)
    {
        return;
    }

    float fDistance = GetDistance();
    PblVector3 vOffset(_vDirection.x * fDistance, _vDirection.y * fDistance,
                       _vDirection.z * fDistance);
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblVector3 vPosition(vCamera.x + vOffset.x, vCamera.y + vOffset.y, vCamera.z + vOffset.z);
    PblVector3 vView;
    D3DXVec3TransformCoord(&vView, &vPosition, &RedRenderer::GetCamera()->GetMatrixInverse());
    vPosition = vView;

    const float fScale = fDistance * 0.00833f;
    const float fSize = _fSize * fScale / RedRenderer::GetCamera()->GetZoom();
    if (!(uiPass & 0x200000))
    {
        Rect rect;
        CreateTextures<VERTEX, RVF>(m_sunTex, vPosition, fSize, rect, true);
    }

    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(300, sizeof(VERTEX), false, true);
    CompressedPos_SetBoundingBox(s_boundingBox);
    unsigned int uiStart;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(300, uiStart);
    VERTEX* pVertices = pBase;
    EmitRing(pVertices, vPosition, 0.0f, _sunHaloSize[0] * fScale, _sunHaloSize[1] * fScale,
             _sunHaloSize[2] * fScale, _sunHaloColor[0], _sunHaloColor[1], _sunHaloColor[2], 30,
             0.0f, 0.0f);
    if (_flareEnable)
    {
        EmitRing(pVertices, vPosition, 0.0f, 0.0f, 0.0f, fSize, _color, _color, _color, 30, 0.0f,
                 0.0f);
    }
    const int iNumVertices = pVertices - pBase;
    pBuffer->Unlock();

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, iNumVertices - 2, RVF,
                                                        iNumVertices, uiStart, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pSunAndHaloShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x0042f0e0
template <class VERTEX, int RVF>
bool CreateTextures(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                    SunFlare::Rect& rect, bool bSunAndHalo)
{
    if (bSunAndHalo && FLRenderer::GetNearSceneFadeStart() >= FLRenderer::GetFarSceneRange())
    {
        return false;
    }

    const PblVector3 vMin = RedRenderer::TransformProjectionPointToScreenSpace(
        RedRenderer::TransformCameraPointToProjectionSpace(
            PblVector3(vPosition.x - fSize, vPosition.y - fSize, vPosition.z)));
    const PblVector3 vMax = RedRenderer::TransformProjectionPointToScreenSpace(
        RedRenderer::TransformCameraPointToProjectionSpace(
            PblVector3(vPosition.x + fSize, vPosition.y + fSize, vPosition.z)));
    float viewMinX, viewMinY, viewMaxX, viewMaxY;
    RedRenderer::pcGetViewportExtents(viewMinX, viewMinY, viewMaxX, viewMaxY);
    if (viewMinX >= vMax.x || vMin.x >= viewMaxX || viewMinY >= vMin.y ||
        vMax.y >= viewMaxY)
    {
        return false;
    }

    const float sourceWidth = vMax.x - vMin.x;
    const float sourceHeight = vMin.y - vMax.y;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    if (bSunAndHalo)
    {
        IDirect3DSurface9* pBack;
        IDirect3DSurface9* pSky;
        RedRenderer::pcGetDevice()->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBack);
        ((IDirect3DTexture9*)RedRenderer::pcGetBuffer(PC_BUFFER_SKY)->m_pTexture)->GetSurfaceLevel(0, &pSky);
        D3DSURFACE_DESC backDesc, skyDesc;
        pBack->GetDesc(&backDesc);
        pSky->GetDesc(&skyDesc);
        pBack->Release();
        pSky->Release();
        if (backDesc.Width != skyDesc.Width)
        {
            scaleX = (float)backDesc.Width / (float)skyDesc.Width;
        }
        if (backDesc.Height != skyDesc.Height)
        {
            scaleY = (float)backDesc.Height / (float)skyDesc.Height;
        }
    }
    if (sourceWidth > (float)pTexture->GetWidth() || sourceHeight > (float)pTexture->GetHeight())
    {
        if (sourceWidth > sourceHeight)
        {
            scaleX *= (float)pTexture->GetWidth() / sourceWidth;
        }
        else
        {
            scaleY *= (float)pTexture->GetHeight() / sourceHeight;
        }
    }
    float destWidth = sourceWidth * scaleX;
    float destHeight = sourceHeight * scaleY;
    float destMinX = 0.0f, destMinY = 0.0f;
    float minX = vMin.x, maxX = vMax.x, minY = vMax.y, maxY = vMin.y;
    if (viewMinX > minX) { destMinX = (viewMinX - minX) * scaleX; minX = viewMinX; }
    if (maxX > viewMaxX) { destWidth -= (maxX - viewMaxX) * scaleX; maxX = viewMaxX; }
    if (viewMinY > minY) { destMinY = (viewMinY - minY) * scaleY; minY = viewMinY; }
    if (maxY > viewMaxY) { destHeight -= (maxY - viewMaxY) * scaleY; maxY = viewMaxY; }
    RECT source = { (int)minX, (int)minY, (int)maxX, (int)maxY };
    RECT dest = { (int)destMinX, (int)destMinY, (int)destWidth, (int)destHeight };
    if (source.left >= source.right || source.top >= source.bottom ||
        dest.left >= dest.right || dest.top >= dest.bottom)
    {
        return false;
    }

    rect.minX = 0.0f; rect.minY = 0.0f;
    rect.maxX = sourceWidth * scaleX / (float)pTexture->GetWidth();
    rect.maxY = sourceHeight * scaleY / (float)pTexture->GetHeight();

    pcRedPrimitive* pClear = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 515, 24, 4, 0, false);
    float* pClearV = (float*)pClear->BeginVertexData();
    if (!pClearV)
    {
        return false;
    }
    const float x0 = minX - 2.0f, x1 = maxX + 2.0f;
    const float y0 = minY - 2.0f, y1 = maxY + 2.0f;
    const float clearXY[8] = { x0, y0, x0, y1, x1, y0, x1, y1 };
    for (int i = 0; i < 4; ++i)
    {
        pClearV[i * 6] = clearXY[2 * i];
        pClearV[i * 6 + 1] = clearXY[2 * i + 1];
        pClearV[i * 6 + 2] = 0.0f;
        pClearV[i * 6 + 3] = 1.0f;
    }
    pClear->EndVertexData();
    RedColor transparent = { 0, 0, 0, 0 };
    RedRenderer::pcRenderPrimitive(pClear, s_pClearShader,
                                   gMatrixIdentity, transparent, 0x200, 0, 0, 0);
    pClear->Release();

    pcRedPrimitive* pDisc = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, RVF,
                                                   sizeof(VERTEX), 42, 0, false);
    pDisc->m_bbox = s_boundingBox;
    CompressedPos_SetBoundingBox(s_boundingBox);
    VERTEX* pDiscV = (VERTEX*)pDisc->BeginVertexData();
    if (!pDiscV)
    {
        return false;
    }
    for (int i = 0; i <= 20; ++i)
    {
        float s, c;
        PblMath::SinCos(&s, &c, (float)i * 6.2831855f / 20.0f);
        SetSunFlareDiscVertex(pDiscV[2 * i], vPosition.x, vPosition.y, vPosition.z);
        SetSunFlareDiscVertex(pDiscV[2 * i + 1], vPosition.x + c * fSize,
                             vPosition.y + s * fSize, vPosition.z);
    }
    pDisc->EndVertexData();
    RedRenderer::pcRenderPrimitive(pDisc, s_pSunAndHaloShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::BLACK, 0x200, 0, 0, 0);
    pDisc->Release();

    if (!bSunAndHalo && FLRenderer::GetFarSceneRange() > FLRenderer::GetNearSceneFadeStart())
    {
        pcRedPrimitive* pFade = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP, 515, 24, 4, 0, false);
        float* pFadeV = (float*)pFade->BeginVertexData();
        if (!pFadeV)
        {
            return false;
        }
        const float xy[8] = { vMin.x, vMax.y, vMin.x, vMin.y,
                              vMax.x, vMax.y, vMax.x, vMin.y };
        const float uv[8] = { rect.minX, rect.minY, rect.minX, rect.maxY,
                              rect.maxX, rect.minY, rect.maxX, rect.maxY };
        for (int i = 0; i < 4; ++i)
        {
            pFadeV[i * 6] = xy[2 * i]; pFadeV[i * 6 + 1] = xy[2 * i + 1];
            pFadeV[i * 6 + 2] = 0.0f; pFadeV[i * 6 + 3] = 1.0f;
            pFadeV[i * 6 + 4] = uv[2 * i]; pFadeV[i * 6 + 5] = uv[2 * i + 1];
        }
        pFade->EndVertexData();
        s_pClearShader->SetBlendMode(pcClearShader::BLENDMODE_MODULATE);
        s_pClearShader->m_pTex = pTexture;
        RedRenderer::pcRenderPrimitive(pFade, s_pClearShader,
                                       gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
        pFade->Release();
        s_pClearShader->SetBlendMode(pcClearShader::BLENDMODE_NONE);
        s_pClearShader->m_pTex = 0;
    }

    if (bSunAndHalo || FLRenderer::GetNearSceneFadeStart() >= FLRenderer::GetFarSceneRange())
    {
        RedTexture* pOld = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepth = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(pTexture, pDepth);
        RedColor clearColor = RedColor::BLACK;
        clearColor.a = 0;
        RedRenderer::pcClear(1, clearColor, 1.0f, 0);
        RedRenderer::pcSetRenderTarget(pOld, pDepth);
        RedRenderer::pcSetViewport(viewport);
    }

    IDirect3DSurface9* pSource;
    IDirect3DSurface9* pSun;
    IDirect3DSurface9* pSample;
    RedRenderer::pcGetDevice()->GetRenderTarget(0, &pSource);
    ((IDirect3DTexture9*)pTexture->m_pTexture)->GetSurfaceLevel(0, &pSun);
    ((IDirect3DTexture9*)s_sampleTex->m_pTexture)->GetSurfaceLevel(0, &pSample);
    RedRenderer::pcGetDevice()->StretchRect(pSource, &source, pSun, &dest, D3DTEXF_NONE);
    if (!bSunAndHalo)
    {
        RECT sampleDest = { 0, 0, 16, 16 };
        RedRenderer::pcGetDevice()->StretchRect(pSource, &source, pSample, &sampleDest,
                                                D3DTEXF_NONE);
    }
    pSource->Release(); pSun->Release(); pSample->Release();
    if (!bSunAndHalo)
    {
        RedTexture* pOld = RedRenderer::pcGetRenderTarget();
        IDirect3DSurface9* pDepth = RedRenderer::pcGetDepthTarget();
        RedViewport viewport;
        RedRenderer::pcGetViewport(viewport);
        RedRenderer::pcSetRenderTarget(s_intensityTex, 0);
        RedColor clearColor = RedColor::BLACK;
        clearColor.a = 0;
        RedRenderer::pcClear(1, clearColor, 1.0f, 0);
        RedRenderer::pcRenderPrimitive(s_samplePoints, s_pSampleShader,
                                       gMatrixIdentity, RedColor::WHITE, 0x200, 0, 0, 0);
        RedRenderer::pcSetRenderTarget(pOld, pDepth);
        RedRenderer::pcSetViewport(viewport);
    }
    return true;
}

// TEMPLATE: SWBF2 0x00430100
// CreateTextures<FlareCompressedVertex,37506>
template bool CreateTextures<FlareVertex, 642>(RedTexture*, const PblVector3&, float,
                                                SunFlare::Rect&, bool);
template bool CreateTextures<FlareCompressedVertex, 37506>(RedTexture*, const PblVector3&,
                                                             float, SunFlare::Rect&, bool);

// FUNCTION: SWBF2 0x0042fb20
template <class VERTEX, int RVF>
void RenderFlareOuts(RedTexture* pTexture, const PblVector3& vPosition, float fSize,
                     const SunFlare::Rect& rect, float fFlareOutSize, int iNumFlareOuts,
                     unsigned char initialAlpha)
{
    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(6 * iNumFlareOuts,
                                                            sizeof(VERTEX), false, true);
    CompressedPos_SetBoundingBox(s_boundingBox);
    unsigned int startVertex;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(6 * iNumFlareOuts, startVertex);
    VERTEX* pVertex = pBase;
    float size = fSize;
    float step = (fFlareOutSize - fSize) / (float)iNumFlareOuts;
    for (int i = 0; i < iNumFlareOuts; ++i)
    {
        size += step;
        float fade = 1.0f - (float)i / (float)iNumFlareOuts;
        RedColor color = RedColor::WHITE;
        color.a = (unsigned char)(int)((float)initialAlpha * fade * fade * fade);
        if (color.a < 3)
        {
            break;
        }
        const float x0 = vPosition.x - size, x1 = vPosition.x + size;
        const float y0 = vPosition.y - size, y1 = vPosition.y + size;
        SetSunFlareVertex(pVertex[0], x0, y0, vPosition.z, color, rect.minX, rect.maxY);
        SetSunFlareVertex(pVertex[1], x1, y0, vPosition.z, color, rect.maxX, rect.maxY);
        SetSunFlareVertex(pVertex[2], x1, y1, vPosition.z, color, rect.maxX, rect.minY);
        SetSunFlareVertex(pVertex[3], x0, y0, vPosition.z, color, rect.minX, rect.maxY);
        SetSunFlareVertex(pVertex[4], x1, y1, vPosition.z, color, rect.maxX, rect.minY);
        SetSunFlareVertex(pVertex[5], x0, y1, vPosition.z, color, rect.minX, rect.minY);
        pVertex += 6;
    }
    const int vertexCount = pVertex - pBase;
    pBuffer->Unlock();
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLELIST,
                                                         vertexCount / 3, RVF, vertexCount,
                                                         startVertex, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    s_pFlareOutShader->SetTexture(pTexture);
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pFlareOutShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// TEMPLATE: SWBF2 0x00430b70
// RenderFlareOuts<FlareCompressedVertex,37506>
template void RenderFlareOuts<FlareVertex, 642>(RedTexture*, const PblVector3&, float,
                                                const SunFlare::Rect&, float, int, unsigned char);
template void RenderFlareOuts<FlareCompressedVertex, 37506>(RedTexture*, const PblVector3&,
                                                             float, const SunFlare::Rect&, float,
                                                             int, unsigned char);

// FUNCTION: SWBF2 0x0042ff10
template <class VERTEX, int RVF>
void SunFlare::RenderSunRings(const PblVector3& vPosition, float fSize, float fTexU,
                             float fTexV) const
{
    pcRedVertexBuffer* pBuffer = pcRedVertexBuffer::Create(700, sizeof(VERTEX), false, true);
    CompressedPos_SetBoundingBox(s_boundingBox);
    unsigned int uiStart;
    VERTEX* pBase = (VERTEX*)pBuffer->Lock(700, uiStart);
    VERTEX* pVertices = pBase;
    ::EmitSunRings(*this, pVertices, vPosition, fTexU, fTexV);
    const int iNumVertices = pVertices - pBase;
    pBuffer->Unlock();
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(D3DPT_TRIANGLESTRIP,
        iNumVertices - 2, RVF, iNumVertices, uiStart, pBuffer, 0, 0);
    pPrimitive->m_bbox = s_boundingBox;
    RedRenderer::pcRenderPrimitive(pPrimitive, s_pRingShader,
                                   RedRenderer::GetCamera()->GetMatrix(), RedColor::WHITE, 0x200, 0, 0, 0);
    pBuffer->Release();
    pPrimitive->Release();
}

// TEMPLATE: SWBF2 0x00431140
// SunFlare::RenderSunRings<FlareCompressedVertex,37506>
template void SunFlare::RenderSunRings<FlareVertex, 642>(const PblVector3&, float, float, float) const;
template void SunFlare::RenderSunRings<FlareCompressedVertex, 37506>(const PblVector3&, float, float, float) const;

static void SetRingPosition(FlareVertex& v, const PblVector3& center, float cosine,
                            float sine, float radius)
{
    v.pos.x = center.x + cosine * radius;
    v.pos.y = center.y + sine * radius;
    v.pos.z = center.z;
}

static void SetRingPosition(FlareCompressedVertex& v, const PblVector3& center,
                            float cosine, float sine, float radius)
{
    v.pos.SetPos(center.x + cosine * radius,
                       center.y + sine * radius, center.z);
}

// FUNCTION: SWBF2 0x00431330
template <class VERTEX>
void EmitRing(VERTEX*& pVertices, const PblVector3& vPosition, float fAngle,
              float fInnerRadius, float fMiddleRadius, float fOuterRadius,
              const RedColor& innerColor, const RedColor& middleColor,
              const RedColor& outerColor, int iSegments, float fU, float fV)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    SetRingPosition(*pVertices++, vPosition, fCos, fSin, fInnerRadius);
    for (int i = 0; i <= iSegments; ++i)
    {
        PblMath::SinCos(&fSin, &fCos, (float)i * 6.2831855f / (float)iSegments + fAngle);
        VERTEX* pPair = pVertices;
        SetSunFlareVertex(pPair[0], vPosition.x + fCos * fInnerRadius,
                          vPosition.y + fSin * fInnerRadius, vPosition.z,
                          innerColor, fU, fV);
        SetSunFlareVertex(pPair[1], vPosition.x + fCos * fMiddleRadius,
                          vPosition.y + fSin * fMiddleRadius, vPosition.z,
                          middleColor, fU, fV);
        if (i == 0)
        {
            pPair[2 * iSegments + 2] = pPair[1];
        }
        pPair[2 * iSegments + 3] = pPair[1];
        SetSunFlareVertex(pPair[2 * iSegments + 4],
                          vPosition.x + fCos * fOuterRadius,
                          vPosition.y + fSin * fOuterRadius, vPosition.z,
                          outerColor, fU, fV);
        pVertices += 2;
    }
    pVertices += 2 * iSegments + 3;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    SetRingPosition(*pVertices++, vPosition, fCos, fSin, fOuterRadius);
}

// TEMPLATE: SWBF2 0x004315f0
// EmitRing<FlareCompressedVertex>
template void EmitRing<FlareVertex>(FlareVertex*&, const PblVector3&, float, float, float,
                                    float, const RedColor&, const RedColor&, const RedColor&,
                                    int, float, float);
template void EmitRing<FlareCompressedVertex>(FlareCompressedVertex*&, const PblVector3&,
                                              float, float, float, float, const RedColor&,
                                              const RedColor&, const RedColor&, int, float, float);

static RedColor SunRingColor(unsigned int rgb, unsigned char alpha)
{
    RedColor color;
    color.b = (unsigned char)rgb;
    color.g = (unsigned char)(rgb >> 8);
    color.r = (unsigned char)(rgb >> 16);
    color.a = alpha;
    return color;
}

static float& SunRingRadius(FlareVertex*) { return sFirstRingRadiusUncompressed; }
static float& SunRingRadius(FlareCompressedVertex*) { return sFirstRingRadiusCompressed; }
static unsigned int& SunRingFlags(FlareVertex*) { return sFirstRingFlagsUncompressed; }
static unsigned int& SunRingFlags(FlareCompressedVertex*) { return sFirstRingFlagsCompressed; }
static unsigned char SunRingAlpha(FlareVertex*) { return sFirstRingAlphaUncompressed; }
static unsigned char SunRingAlpha(FlareCompressedVertex*) { return sFirstRingAlphaCompressed; }

struct SunRingSpec
{
    float offset;
    float innerRadius;
    float middleRadius;
    float outerRadius;
    unsigned int innerRGB;
    unsigned int middleRGB;
    unsigned int outerRGB;
    int segments;
    bool outerUsesAlpha;
};

template <class VERTEX>
static void EmitSunRingsBody(const SunFlare& flare, VERTEX*& pVertices,
                             const PblVector3& vPosition, float fDistance,
                             float fTexU, float fTexV)
{
    const PblMatrix& projection = RedRenderer::GetProjectionMatrix();
    const float fScale = fDistance * 0.0037f;
    const float fDepthInv = 1.0f / vPosition.z;
    const float fProjX = projection.m[0][0] * vPosition.x * fDepthInv;
    const float fProjY = projection.m[1][1] * vPosition.y * fDepthInv;
    float fForeshorten = 1.0f - (fProjX * fProjX + fProjY * fProjY) * 0.5f;
    if (fForeshorten < 0.0f)
    {
        fForeshorten = 0.0f;
    }

    unsigned char maxColor = flare._color.b;
    if (flare._color.g > maxColor)
    {
        maxColor = flare._color.g;
    }
    if (flare._color.r > maxColor)
    {
        maxColor = flare._color.r;
    }
    const unsigned char alpha = (unsigned char)(int)((float)maxColor * fForeshorten + 0.5f);
    const float fSpikeScale = flare._spikeSize * fScale;
    const float fAngle = (vPosition.y * 6.2831855f + vPosition.x * 3.1415927f)
                         / vPosition.z + fSpikeScale * 3.1415927f * 0.33333001f;
    float s1, c1, s2, c2;
    PblMath::SinCos(&s1, &c1, fAngle);
    PblMath::SinCos(&s2, &c2, 1.0471871f - fAngle);
    const float r = ((fForeshorten + 1.0f) * (float)maxColor * 0.5f * 0.05f * 0.1f)
                    * fSpikeScale;
    const float x = vPosition.x, y = vPosition.y, z = vPosition.z;
    const RedColor clear = SunRingColor(0, 0);
    const RedColor& spike = flare._spikeColor;
    const float px[13] = {
        x + c1 * r, x + (c1 * -0.5f - s1 * 0.866f) * r, x,
        x + (c1 * -0.5f + s1 * 0.866f) * r, x + c1 * r, x + c1 * r,
        x + c2 * (r * 2.5f), x + c2 * (r * 2.5f),
        x + (c2 * -0.5f - s2 * 0.866f) * (r * 2.5f), x,
        x + (c2 * -0.5f + s2 * 0.866f) * (r * 2.5f),
        x + c2 * (r * 2.5f), x + c2 * (r * 2.5f)
    };
    const float py[13] = {
        y + s1 * (r * 2.0f), y + (c1 * 0.866f - s1 * 0.5f) * (r * 2.0f), y,
        y + (s1 * -0.5f - c1 * 0.866f) * (r * 2.0f), y + s1 * (r * 2.0f),
        y + s1 * (r * 2.0f), y + s2 * r, y + s2 * r,
        y + (c2 * 0.866f - s2 * 0.5f) * r, y,
        y + (s2 * -0.5f - c2 * 0.866f) * r, y + s2 * r, y + s2 * r
    };
    for (int i = 0; i < 13; ++i)
    {
        SetSunFlareVertex(pVertices[i], px[i], py[i], z,
                          i == 2 || i == 9 ? spike : clear, fTexU, fTexV);
    }
    pVertices += 13;

    const RedCamera* pCamera = RedRenderer::GetCamera();
    const float fThreshold = PblMath::Cos(pCamera->GetFOV() * 0.5f);
    const PblVector3& cameraAxis = pCamera->GetMatrix().GetZAxis();
    float facing = -(cameraAxis.x * flare._vDirection.x +
                     cameraAxis.y * flare._vDirection.y +
                     cameraAxis.z * flare._vDirection.z);
    if (facing < 0.0f)
    {
        facing = fThreshold;
    }
    float fade = (facing - fThreshold) / (1.0f - fThreshold);
    if (fade < 0.0f)
    {
        fade = 0.0f;
    }

    float& firstRadius = SunRingRadius((VERTEX*)0);
    unsigned int& flags = SunRingFlags((VERTEX*)0);
    if (!(flags & 1))
    {
        firstRadius = fScale * 35.0f;
        flags |= 1;
    }
    const RedColor whiteInner = SunRingColor(0xffffff,
        (unsigned char)(int)((float)SunRingAlpha((VERTEX*)0) * fade));
    const RedColor whiteMiddle = SunRingColor(0xffffff,
        (unsigned char)(int)(fade * 72.0f));
    const RedColor whiteOuter = SunRingColor(0xffffff, 0);
    EmitRing(pVertices, vPosition, 0.0f, 0.0f, fade * firstRadius,
             fade * 3.0f * firstRadius, whiteInner, whiteMiddle, whiteOuter,
             30, fTexU, fTexV);

    const SunRingSpec rings[] = {
        { -0.30000001f, 0, 0.015f, 0.02f, 0x1b360d, 0x0d1b0d, 0x0d1b1b, 6, false },
        { -0.34f, 0, 0.0049999999f, 0.0099999998f, 0x6c3636, 0x363636, 0x36366c, 5, false },
        { -0.5f, fForeshorten * 0.059999999f, (fForeshorten + 1.0f) * 0.082500003f,
          (fForeshorten + 1.0f) * 0.1f, 0x1b0d36, 0x141b0a, 0x360d00, 20, false },
        { -0.77499998f, 0, 0.0099999998f, 0.02f, 0x0d140d, 0x0d140d, 0x0d140d, 6, false },
        { -0.82499999f, 0, 0.02f, 0.039999999f, 0x142214, 0x142214, 0x142214, 8, false },
        { -0.89499998f, 0, 0.01825f, 0.037500001f, 0x142214, 0x142214, 0x1b281b, 8, false },
        { -0.94999999f, 0, 0.015f, 0.029999999f, 0x0d140d, 0x0d140d, 0x0d140d, 7, false },
        { 0.30000001f, 0, 0.0040000002f, 0.0080000004f, 0x88441b, 0x6c440d, 0, 5, true },
        { 0.2f, 0, 0.025f, 0.039999999f, 0x440000, 0x360d00, 0, 6, true },
        { 0.125f, 0, 0.015f, 0.025f, 0x3d3600, 0x1b3600, 0, 6, true },
        { 1.7f, 0, 0.015f, 0.02f, 0x000d51, 0x000044, 0x000044, 6, false },
        { 1.53f, 0, 0.0145f, 0.0175f, 0x220051, 0x000044, 0x000044, 5, false },
        { 1.62f, 0, 0.02f, 0.022500001f, 0x141b44, 0x0d1436, 0, 7, false }
    };
    for (unsigned int i = 0; i < sizeof(rings) / sizeof(rings[0]); ++i)
    {
        const SunRingSpec& ring = rings[i];
        PblVector3 center(x * ring.offset, y * ring.offset, z);
        const RedColor inner = SunRingColor(ring.innerRGB, i == 2 ? 0 : alpha);
        const RedColor middle = SunRingColor(ring.middleRGB, alpha);
        const RedColor outer = SunRingColor(ring.outerRGB,
                                            ring.outerUsesAlpha ? alpha : 0);
        EmitRing(pVertices, center, 0.0f, z * ring.innerRadius,
                 z * ring.middleRadius, z * ring.outerRadius,
                 inner, middle, outer, ring.segments, fTexU, fTexV);
    }
}

// FUNCTION: SWBF2 0x00431ad0
template <>
void SunFlare::EmitSunRings<FlareVertex>(FlareVertex*& pVertices, const PblVector3& vPosition,
                                         float fTexU, float fTexV) const
{
    EmitSunRingsBody(*this, pVertices, vPosition, GetDistance(), fTexU, fTexV);
}

// FUNCTION: SWBF2 0x00432b00
template <>
void SunFlare::EmitSunRings<FlareCompressedVertex>(FlareCompressedVertex*& pVertices,
                                                   const PblVector3& vPosition, float fTexU,
                                                   float fTexV) const
{
    EmitSunRingsBody(*this, pVertices, vPosition, GetDistance(), fTexU, fTexV);
}

static void EmitSunRings(const SunFlare& flare, FlareVertex*& pVertices,
                         const PblVector3& vPosition, float fTexU, float fTexV)
{
    flare.EmitSunRings(pVertices, vPosition, fTexU, fTexV);
}

static void EmitSunRings(const SunFlare& flare, FlareCompressedVertex*& pVertices,
                         const PblVector3& vPosition, float fTexU, float fTexV)
{
    flare.EmitSunRings(pVertices, vPosition, fTexU, fTexV);
}
