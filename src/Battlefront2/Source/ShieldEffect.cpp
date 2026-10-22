#include "ShieldEffect.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "Battlefront2Lod.h"

// GLOBAL: SWBF2 0x01f6dda8
// SYNTHETIC: SWBF2 0x00406370
// `dynamic initializer for 'ShieldEffect::sMemoryPool''
// SYNTHETIC: SWBF2 0x007694a0
// `dynamic atexit destructor for 'ShieldEffect::sMemoryPool''
MemoryPool ShieldEffect::sMemoryPool("ShieldEffect", 0xF0);

// GLOBAL: SWBF2 0x01eaf060
ShieldEffectManager* ShieldEffectManager::s_pInstance;

// FUNCTION: SWBF2 0x00636580
void ShieldEffectManager::Create()
{
    s_pInstance = new ShieldEffectManager();
}

// FUNCTION: SWBF2 0x00636610
ShieldEffectManager::ShieldEffectManager()
    : FLEffect(0x337519B0u, 3)
{
}

// FUNCTION: SWBF2 0x00636730
void ShieldEffectManager::Init()
{
    ShieldEffect::sMemoryPool.Setup(64, ShieldEffect::sMemoryPool.GetSize(),
                                    ShieldEffect::sMemoryPool.GetGrow());
}

// FUNCTION: SWBF2 0x006365f0
void ShieldEffectManager::Destroy()
{
    if (s_pInstance)
    {
        delete s_pInstance;
    }
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x00636530
void ShieldEffectManager::Render(int)
{
}

// FUNCTION: SWBF2 0x006366c0
ShieldEffectManager::~ShieldEffectManager()
{
    Cleanup();
}

// FUNCTION: SWBF2 0x00636750
void ShieldEffectManager::Cleanup()
{
    while (m_listClasses.GetCount())
    {
        ShieldEffectClass* pClass = m_listClasses.GetFirst()->_pObject;
        m_listClasses.Remove(&pClass->m_Node);
        delete pClass;
    }
}

// FUNCTION: SWBF2 0x006367a0
void ShieldEffectManager::ReadConfig(PblConfig&)
{
}

// FUNCTION: SWBF2 0x006367b0
FLEffectClass* ShieldEffectManager::BuildClass(PblConfig& config) const
{
    ShieldEffectClass* pClass = new ShieldEffectClass(config);
    const_cast<PblList<ShieldEffectClass>&>(m_listClasses).Add(&pClass->m_Node, pClass);
    return pClass;
}

namespace {

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

inline void ReadColor(const PblConfig::Data& data, RedColor& color)
{
    if (data.GetNumArgs() >= 3)
    {
        color.r = ClampColorComponent(data.GetFloatArg(0));
        color.g = ClampColorComponent(data.GetFloatArg(1));
        color.b = ClampColorComponent(data.GetFloatArg(2));
    }
    if (data.GetNumArgs() >= 4)
    {
        color.a = ClampColorComponent(data.GetFloatArg(3));
    }
}
}

// FUNCTION: SWBF2 0x00636830
ShieldEffectClass::ShieldEffectClass(PblConfig& config)
{
    m_type = 0;
    m_uiTextureHash = 0;
    m_color = RedColor::WHITE;
    m_colorSpecular = RedColor::WHITE;
    m_fRadius = 1.0f;
    m_fHeight = 1.0f;
    m_fViewAngleFactor = 0.0f;
    m_fTurnOnTime = 1.0f;
    m_fTurnOffTime = 1.0f;
    m_fScrollRate = 1.0f;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x5127F14Du:
            if (data.GetStringArg(0)[0] == 'f')
            {
                m_type = 1;
            }
            break;
        case 0x3C6468F4u:
            m_uiTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x3D7E6258u:
            ReadColor(data, m_color);
            break;
        case 0x01E4AAF7u:
            ReadColor(data, m_colorSpecular);
            break;
        case 0x0DBA4CB3u:
            m_fRadius = data.GetFloatArg(0);
            break;
        case 0xD5BDBB42u:
            m_fHeight = data.GetFloatArg(0);
            break;
        case 0xB28AF6F4u:
            m_fViewAngleFactor = data.GetFloatArg(0);
            break;
        case 0x08AAB3AAu:
            m_fTurnOnTime = data.GetFloatArg(0);
            break;
        case 0xC8A9E3C6u:
            m_fTurnOffTime = data.GetFloatArg(0);
            break;
        case 0xD8BB237Eu:
            m_fScrollRate = data.GetFloatArg(0);
            break;
        }
    }

    PlatformInit();
}

// FUNCTION: SWBF2 0x00636bd0
ShieldEffectClass::~ShieldEffectClass()
{
    PlatformCleanup();
}

class FambaaShieldEffectLodData : public RedLodData
{
public:
    FambaaShieldEffectLodData()
    {
        _LodCost[0] = 1;
        _LodCost[1] = 1;
        _LodCost[2] = 1;
        _LodCost[3] = 0;
        _LodBitmask = 1;
        _LodClass = 3;
        mLowRezFlags = 3;
        mMaxDist = 500.0f;
    }
};

// GLOBAL: SWBF2 0x007ec2a8
FambaaShieldEffectLodData fambaaShieldEffectLodData;

// FUNCTION: SWBF2 0x00636c20
FLEffectObject* ShieldEffectClass::BuildEffect()
{
    return new ShieldEffect(this);
}

// FUNCTION: SWBF2 0x00636c90
ShieldEffect::ShieldEffect(const ShieldEffectClass* pClass)
    : RedSceneObject(&fambaaShieldEffectLodData)
{
    m_uiAttachFlags |= UPDATE_REQUIRED;
    m_pClass = const_cast<ShieldEffectClass*>(pClass);
    m_fScrollTimer = 0.0f;
    m_fTurnOnTimer = 0.0f;
    m_fTurnOnFactor = 0.0f;
    m_bTurnOn = true;
    m_bStopAndFinish = false;
    PlatformInit();
}

// SYNTHETIC: SWBF2 0x006371ab
// [thunk]:ShieldEffect::`scalar deleting destructor' adjustor{24}
// SYNTHETIC: SWBF2 0x006371b3
// [thunk]:ShieldEffect::`scalar deleting destructor' adjustor{128}
// SYNTHETIC: SWBF2 0x00636d80
// ShieldEffect::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00636db0
ShieldEffect::~ShieldEffect()
{
    PlatformCleanup();
}

// FUNCTION: SWBF2 0x00636550
bool ShieldEffect::IsEffectActive() const
{
    return _bActive;
}

// FUNCTION: SWBF2 0x00636560
const PblMatrix& ShieldEffect::GetBBoxMatrix() const
{
    return m_mWorldMatrix;
}

// FUNCTION: SWBF2 0x00636570
bool ShieldEffect::IsReflected() const
{
    return true;
}

// FUNCTION: SWBF2 0x00636e20
void ShieldEffect::ActivateEffect()
{
    if (!_bActive)
    {
        SetRenderPos(*(const PblVector3*)&m_mWorldMatrix.m[3][0]);
        SetRenderRadius(m_pClass->m_fRadius);
        Activate();
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x00636e70
void ShieldEffect::DeactivateEffect()
{
    if (_bActive)
    {
        Deactivate();
        FLEffectObject::DeactivateEffect();
    }
}

// FUNCTION: SWBF2 0x00636ea0
void ShieldEffect::StopAndFinish()
{
    TurnOff();
    m_bStopAndFinish = true;
}

// FUNCTION: SWBF2 0x00637190
void ShieldEffect::TurnOff()
{
    if (m_bTurnOn)
    {
        m_bTurnOn = false;
        m_fTurnOnTimer = 0.0f;
    }
}

// FUNCTION: SWBF2 0x00636eb0
void ShieldEffect::Render(unsigned char ucViewport, float, unsigned int uiFlags)
{
    InterpolateWorldMatrix();

    const PblMatrix* pMatrix = &m_mWorldMatrix;
    PblMatrix mScaled;
    if (m_pClass->m_type == 0)
    {
        float fScale;
        if (m_bTurnOn)
        {
            static float s_fInvPeak = 1.0f / 0.94f;
            static float s_fInvSettle = 1.0f / (1.0f - 0.94f);
            if (m_fTurnOnFactor < 0.94f)
            {
                fScale = m_fTurnOnFactor * 1.06f * s_fInvPeak;
            }
            else
            {
                fScale = (1.0f - m_fTurnOnFactor) * (1.06f - 1.0f) * s_fInvSettle + 1.0f;
            }
        }
        else
        {
            fScale = m_fTurnOnFactor;
        }

        mScaled = m_mWorldMatrix;
        if (fScale != 1.0f)
        {
            for (int r = 0; r < 3; r++)
            {
                mScaled.m[r][0] *= fScale;
                mScaled.m[r][1] *= fScale;
                mScaled.m[r][2] *= fScale;
                mScaled.m[r][3] = 0.0f;
            }
        }
        pMatrix = &mScaled;
    }
    PlatformRender(*pMatrix, ucViewport, uiFlags);
}

// FUNCTION: SWBF2 0x00637090
bool ShieldEffect::Update(float fDeltaTime)
{
    m_fTurnOnTimer += fDeltaTime;
    m_fScrollTimer += fDeltaTime;
    if (m_bTurnOn)
    {
        if (m_fTurnOnTimer < m_pClass->m_fTurnOnTime)
        {
            m_fTurnOnFactor = m_fTurnOnTimer / m_pClass->m_fTurnOnTime;
        }
        else
        {
            m_fTurnOnFactor = 1.0f;
        }
    }
    else
    {
        if (m_fTurnOnTimer < m_pClass->m_fTurnOffTime)
        {
            m_fTurnOnFactor = 1.0f - m_fTurnOnTimer / m_pClass->m_fTurnOffTime;
        }
        else
        {
            if (m_bStopAndFinish)
            {
                return false;
            }
            m_fTurnOnFactor = 0.0f;
        }
    }
    return FLEffectObject::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00637170
void ShieldEffect::UpdatedWorldMatrix()
{
    SetRenderPos(*(const PblVector3*)&m_mWorldMatrix.m[3][0]);
}
