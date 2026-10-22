#include "LensFlare.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblVector.h"

// GLOBAL: SWBF2 0x01e573d8
LensFlareEffect* LensFlareEffect::s_pInstance;

// SYNTHETIC: SWBF2 0x0056f430
// LensFlareEffect::`scalar deleting destructor'
LensFlareEffect::~LensFlareEffect()
{
}

// FUNCTION: SWBF2 0x0056f5d0
void LensFlareEffect::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new LensFlareEffect();
    }
}

// FUNCTION: SWBF2 0x0056f670
void LensFlareEffect::Destroy()
{
    delete s_pInstance;
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0056f690
void LensFlareEffect::Init()
{
    PlatformInit();
}

// FUNCTION: SWBF2 0x0056f6a0
void LensFlareEffect::Cleanup()
{
    PlatformCleanup();
    _List.Cleanup();
}

// FUNCTION: SWBF2 0x0056f6c0
FLEffectClass* LensFlareEffect::BuildClass(PblConfig& config) const
{
    LensFlare* pFlare = new LensFlare;
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x23a0d95cu:
            pFlare->SetSize(data.GetFloatArg(0));
            break;
        case 0x1612c34cu:
            pFlare->SetDrawDistance(data.GetFloatArg(0));
            break;
        case 0x3d7e6258u:
        {
            RedColor color;
            color.r = data.GetUint8Arg(0);
            color.g = data.GetUint8Arg(1);
            color.b = data.GetUint8Arg(2);
            color.a = data.GetNumArgs() > 3 ? data.GetUint8Arg(3) : 255;
            pFlare->SetColor(color);
            break;
        }
        case 0x934f4e0au:
        {
            const PblVector3 vPos(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
            pFlare->SetPosition(vPos);
            break;
        }
        }
    }
    pFlare->ActivateEffect();
    return pFlare;
}

// FUNCTION: SWBF2 0x0056f4a0
LensFlare::LensFlare(const LensFlare& that)
    : _fSize(that._fSize)
    , _color(that._color)
    , _fSquaredDrawDistance(that._fSquaredDrawDistance)
    , _uiHideInView(0)
{
}

// FUNCTION: SWBF2 0x0056f8d0
void LensFlareEffect::LensFlareList::Add(LensFlare* pFlare)
{
    PblList<LensFlare>::Add(&pFlare->_Node, pFlare);
}

// FUNCTION: SWBF2 0x0056f900
void LensFlareEffect::LensFlareList::Remove(LensFlare* pFlare)
{
    PblList<LensFlare>::Remove(&pFlare->_Node);
}

// FUNCTION: SWBF2 0x0056f940
FLEffectObject* LensFlare::BuildEffect()
{
    LensFlare* pFlare = new LensFlare(*this);
    pFlare->ActivateEffect();
    return pFlare;
}

// FUNCTION: SWBF2 0x0056f9b0
bool LensFlare::IsEffectActive() const
{
    return _Node.IsInList();
}

// FUNCTION: SWBF2 0x0056f9c0
void LensFlare::ActivateEffect()
{
    if (!IsEffectActive())
    {
        LensFlareEffect::s_pInstance->_List.Add(this);
        FLEffectObject::ActivateEffect();
    }
}

// FUNCTION: SWBF2 0x0056f9f0
void LensFlare::DeactivateEffect()
{
    if (IsEffectActive())
    {
        LensFlareEffect::s_pInstance->_List.Remove(this);
        FLEffectObject::DeactivateEffect();
    }
}

// FUNCTION: SWBF2 0x0056fa20
bool LensFlare::Update(float fDeltaTime)
{
    return FLEffectObject::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x0056fa40
void LensFlare::UpdatedWorldMatrix()
{
}
