#include <float.h>
#include "PebbleFL/Common/PblVector.h"
#include <math.h>
#include <d3dx9math.h>

#include "HUDElement.h"
#include "HUDElementGroup.h"
#include "CameraManager.h"
#include "HUDManager.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

// FUNCTION: SWBF2 0x00549110
PblMatrix HUD::Element::CalculateGroupMatrix(HUD::ElementGroupBase* const pGroup) const
{
    const HUD::Element* pElement = pGroup;
    PblMatrix result;
    const PblMatrix local = pElement->mElement->GetMatrix();
    if (pElement->mGroup)
    {
        const PblMatrix parent = CalculateGroupMatrix(pElement->mGroup);
        D3DXMatrixMultiply((D3DXMATRIX*)&result, (const D3DXMATRIX*)&local,
                           (const D3DXMATRIX*)&parent);
    }
    else
    {
        result = local;
    }
    return result;
}

// GLOBAL: SWBF2 0x007e6910

char* HUD::gRelativeModeStrings[4] =
{
    "Pixels", "Screen", "Viewport", "Frame"
};

// GLOBAL: SWBF2 0x007e6908

char* HUD::Element::Factory::sBlendModeStrings[2] =
{
    "Alpha", "Additive"
};

// GLOBAL: SWBF2 0x01e56c34

float HUD::Element::sViewportWidth;

// GLOBAL: SWBF2 0x01e56c30

float HUD::Element::sViewportHeight;

// GLOBAL: SWBF2 0x007eba18
// SYNTHETIC: SWBF2 0x00403fb0
// `dynamic initializer for 'HUD::Element::sList''
// SYNTHETIC: SWBF2 0x00768b20
// `dynamic atexit destructor for 'HUD::Element::sList''
PblListDouble HUD::Element::sList;

// GLOBAL: SWBF2 0x007e6904
float HUD::Element::sRuntimeScale = 1.0f;

static const unsigned int kPosition = 0x934f4e0a;
static const unsigned int kPositionRelative = 0xf9f41ffa;
static const unsigned int kRotation = 0x21ac415f;
static const unsigned int kScale = 0x82971c71;
static const unsigned int kScaleUniform = 0x745450bf;
static const unsigned int kZOrder = 0xec9f263f;
static const unsigned int kAlpha = 0x5d8b6dab;
static const unsigned int kColor = 0x3d7e6258;
static const unsigned int kColorChange = 0x3c656d30;
static const unsigned int kColorChangeRate = 0xb4660872;
static const unsigned int kColorPulseRate = 0xe459101b;
static const unsigned int kUseChangeColor = 0x849604eb;
static const unsigned int kFadeInTime = 0x5083ecd9;
static const unsigned int kFadeHoldTime = 0x8e3e2923;
static const unsigned int kFadeSustainTime = 0xff48a8f9;
static const unsigned int kFadeOutTime = 0x798dc484;
static const unsigned int kBlendMode = 0xfa784eab;
static const unsigned int kViewport = 0xe4abbac3;
static const unsigned int kEventEnable = 0xd23096a8;
static const unsigned int kEventDisable = 0x7b5f76b1;
static const unsigned int kEventChanged = 0x0d1d42cd;
static const unsigned int kEventColor = 0x49f429ae;
static const unsigned int kEventPulseRate = 0xccd945ca;
static const unsigned int kScalable = 0x0dd385e2;
static const unsigned int kScaleAll = 0x6db146a8;

static const unsigned int kEventFadeOut = 0xb8e23cbb;
static const unsigned int kEditOnly = 0xca0b9ccb;
static const unsigned int kAnimEnable = 0xbdbfd84f;
static const unsigned int kAnimDisable = 0x6242102c;
static const unsigned int kAnimChange = 0x3455b0a8;

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00547010
float HUD::ConvertRelativeToPixels(RelativeMode eMode, float fValue, float fFrameSize,
                                   float fViewSize, float fScreenSize)
{
    switch (eMode)
    {
    case relativeMode_Screen:
        return fValue * fScreenSize;
    case relativeMode_Viewport:
        return fValue * fViewSize;
    case relativeMode_Frame:
        return fValue * fFrameSize;
    }
    return fValue;
}

// FUNCTION: SWBF2 0x00547050
float HUD::ConvertPixelsToRelative(RelativeMode eMode, float fValue, float fFrameSize,
                                   float fViewSize, float fScreenSize)
{
    switch (eMode)
    {
    case relativeMode_Screen:
        return fScreenSize != 0.0f ? fValue / fScreenSize : 0.0f;
    case relativeMode_Viewport:
        return fViewSize != 0.0f ? fValue / fViewSize : 0.0f;
    case relativeMode_Frame:
        return fFrameSize != 0.0f ? fValue / fFrameSize : 0.0f;
    }
    return fValue;
}

// FUNCTION: SWBF2 0x005470c0
float HUD::ConvertXForWideScreen(float fX, float fWidth, float fHeight)
{
    float fInvAspect = 1.0f / RedRenderer::GetScreenAspectRatio();
    float fCenter = fWidth * 0.5f;
    float fScale = (float)RedInterfaceScreen::GetScreenHeight() * fInvAspect
                 / (float)RedInterfaceScreen::GetScreenWidth();
    return fScale * (fX - fCenter) + fCenter;
}

// FUNCTION: SWBF2 0x00547150
float HUD::ConvertWideScreenX(float fX, float fWidth, float fHeight)
{
    float fInvAspect = 1.0f / RedRenderer::GetScreenAspectRatio();
    float fCenter = Element::sViewportWidth * 0.5f;
    float fScale = (float)RedInterfaceScreen::GetScreenHeight() * fInvAspect
                 / (float)RedInterfaceScreen::GetScreenWidth();
    return (fX - fCenter) / fScale + fCenter;
}

// FUNCTION: SWBF2 0x005471e0
HUD::Matrix::Matrix()
    : mMatrix(0),
      mRelativeMode(relativeMode_Pixels)
{
    mScale.SetAll(1.0f);
    mRotation.SetAll(0.0f);
}

// FUNCTION: SWBF2 0x00547220
void HUD::Matrix::ApplyRelativeMode(float fFrameWidth, float fFrameHeight, float fViewWidth,
                                    float fViewHeight, bool bWideScreen)
{
    float fX = mMatrix->m[3][0];
    float fY = mMatrix->m[3][1];
    float fZ = mMatrix->m[3][2];
    fX = ConvertRelativeToPixels(mRelativeMode, fX, fFrameWidth, fViewWidth,
                                 (float)(int)RedInterfaceScreen::GetScreenWidth());
    fY = ConvertRelativeToPixels(mRelativeMode, fY, fFrameHeight, fViewHeight,
                                 (float)(int)RedInterfaceScreen::GetScreenHeight());
    if (bWideScreen && CameraManager::sInstance->m_iNumCam == 1)
    {
        fX = ConvertXForWideScreen(fX, fViewWidth, fViewHeight);
    }
    mMatrix->m[3][0] = fX;
    mMatrix->m[3][1] = fY;
    mMatrix->m[3][2] = fZ;
    mMatrix->m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x005472e0
bool HUD::Matrix::ReadPosition(PblConfig::Data& data, unsigned int uHashID)
{
    if (uHashID == 0)
    {
        uHashID = data.GetId();
    }
    if (uHashID != kPosition)
    {
        return false;
    }

    int iMode = relativeMode_Pixels;
    if (data.GetNumArgs() == 4)
    {
        if (data.IsStringArg(3))
        {
            iMode = (int)TranslateString((const char* const*)gRelativeModeStrings, 4,
                                    data.GetStringArg(3));
        }
        else
        {
            iMode = data.GetBoolArg(3) ? relativeMode_Viewport : relativeMode_Pixels;
        }
        if (iMode < 0)
        {
            iMode = relativeMode_Pixels;
        }
    }
    mRelativeMode = (RelativeMode)iMode;

    mMatrix->m[3][0] = data.GetFloatArg(0);
    mMatrix->m[3][1] = data.GetFloatArg(1);
    mMatrix->m[3][2] = data.GetFloatArg(2);
    mMatrix->m[3][3] = 1.0f;
    return true;
}

// FUNCTION: SWBF2 0x005473b0
void HUD::Matrix::WritePosition(HUD::File& file, unsigned int uTabs, float fFrameWidth,
                                float fFrameHeight, float fViewWidth, float fViewHeight,
                                const char* const pName, bool bWideScreen)
{
    float fX = mMatrix->m[3][0];
    float fY = mMatrix->m[3][1];
    float fZ = mMatrix->m[3][2];
    if (fX == 0.0f && fY == 0.0f && fZ == 0.0f)
    {
        return;
    }

    if (bWideScreen)
    {
        fX = ConvertWideScreenX(fX, fViewWidth, fViewHeight);
    }
    file.WriteTabs(uTabs);
    file.WriteStringf("%s(%f, %f, %f, \"%s\")\n", pName ? pName : "Position",
                      ConvertPixelsToRelative(mRelativeMode, fX, fFrameWidth, fViewWidth,
                                              (float)(int)RedInterfaceScreen::GetScreenWidth()),
                      ConvertPixelsToRelative(mRelativeMode, fY, fFrameHeight, fViewHeight,
                                              (float)(int)RedInterfaceScreen::GetScreenHeight()),
                      fZ, gRelativeModeStrings[mRelativeMode]);
}

// FUNCTION: SWBF2 0x005474d0
bool HUD::Matrix::ReadRotation(PblConfig::Data& data, unsigned int)
{
    if (data.GetId() != kRotation)
    {
        return false;
    }
    SetRotation(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
    return true;
}

// FUNCTION: SWBF2 0x00547500
void HUD::Matrix::WriteRotation(HUD::File& file, unsigned int uTabs, const char* const pName)
{
    file.WriteProperty("Rotation", uTabs, mRotation.x, mRotation.y, mRotation.z,
                       0.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x00547550
bool HUD::Matrix::ReadScale(PblConfig::Data& data, unsigned int)
{
    if (data.GetId() != kScale)
    {
        return false;
    }
    SetScale(data.GetFloatArg(0), data.GetFloatArg(1), data.GetFloatArg(2));
    return true;
}

// FUNCTION: SWBF2 0x00547580
void HUD::Matrix::WriteScale(HUD::File& file, unsigned int uTabs, const char* const pName)
{
    file.WriteProperty("Scale", uTabs, mScale.x, mScale.y, mScale.z, 1.0f, 1.0f, 1.0f);
}

// FUNCTION: SWBF2 0x005475d0
void HUD::Matrix::SetMatrix(PblMatrix* const pMatrix)
{
    mMatrix = pMatrix;
}

// FUNCTION: SWBF2 0x005475e0
const PblMatrix* HUD::Matrix::GetMatrix() const
{
    return mMatrix;
}

// FUNCTION: SWBF2 0x005475f0
void HUD::Matrix::SetRotation(float fX, float fY, float fZ)
{
    while (fX >= 360.0f)
    {
        fX -= 360.0f;
    }
    while (fX < 0.0f)
    {
        fX += 360.0f;
    }
    while (fY >= 360.0f)
    {
        fY -= 360.0f;
    }
    while (fY < 0.0f)
    {
        fY += 360.0f;
    }
    while (fZ >= 360.0f)
    {
        fZ -= 360.0f;
    }
    while (fZ < 0.0f)
    {
        fZ += 360.0f;
    }

    PblMatrix matrix = _IdentityMatrix;
    PblMatrix rotation;
    D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (D3DXMATRIX*)&matrix,
                       D3DXMatrixRotationX((D3DXMATRIX*)&rotation, fX * 0.017453292f));
    D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (D3DXMATRIX*)&matrix,
                       D3DXMatrixRotationY((D3DXMATRIX*)&rotation, fY * 0.017453292f));
    D3DXMatrixMultiply((D3DXMATRIX*)&matrix, (D3DXMATRIX*)&matrix,
                       D3DXMatrixRotationZ((D3DXMATRIX*)&rotation, fZ * 0.017453292f));

    float fPosX = mMatrix->m[3][0];
    float fPosY = mMatrix->m[3][1];
    float fPosZ = mMatrix->m[3][2];
    *mMatrix = matrix;
    mMatrix->m[3][0] = fPosX;
    mMatrix->m[3][1] = fPosY;
    mMatrix->m[3][2] = fPosZ;
    mMatrix->m[3][3] = 1.0f;

    mRotation.x = fX;
    mRotation.y = fY;
    mRotation.z = fZ;
}

// FUNCTION: SWBF2 0x005477e0
void HUD::Matrix::ApplyRotation()
{
    SetRotation(mRotation.x, mRotation.y, mRotation.z);
}

// FUNCTION: SWBF2 0x00547800
void HUD::Matrix::SetScale(float fX, float fY, float fZ)
{
    mScale.x = fX;
    mScale.y = fY;
    mScale.z = fZ;
}

// FUNCTION: SWBF2 0x00547810
const PblVector3& HUD::Matrix::GetScale() const
{
    return mScale;
}

// FUNCTION: SWBF2 0x00547820
void HUD::Matrix::ApplyScale()
{
    float fPosX = mMatrix->m[3][0];
    float fPosY = mMatrix->m[3][1];
    float fPosZ = mMatrix->m[3][2];

    for (int i = 0; i < 3; ++i)
    {
        PblVector3 vAxis;
        D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)mMatrix->m[i]);
        mMatrix->m[i][0] = vAxis.x;
        mMatrix->m[i][1] = vAxis.y;
        mMatrix->m[i][2] = vAxis.z;
        mMatrix->m[i][3] = 0.0f;
    }

    PblMatrix scaling;
    D3DXMatrixMultiply((D3DXMATRIX*)mMatrix, (D3DXMATRIX*)mMatrix,
                       D3DXMatrixScaling((D3DXMATRIX*)&scaling, mScale.x, mScale.y, mScale.z));

    mMatrix->m[3][0] = fPosX;
    mMatrix->m[3][1] = fPosY;
    mMatrix->m[3][2] = fPosZ;
    mMatrix->m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x00547930
HUD::Matrix& HUD::Matrix::operator=(const Matrix& other)
{
    mScale = other.mScale;
    mRelativeMode = other.mRelativeMode;
    mRotation = other.mRotation;
    return *this;
}

// FUNCTION: SWBF2 0x00547960
const PblVector3& HUD::Matrix::GetRotation() const
{
    return mRotation;
}

// FUNCTION: SWBF2 0x00547970
void HUD::Matrix::SetRelativeMode(RelativeMode eMode)
{
    mRelativeMode = eMode;
}

// FUNCTION: SWBF2 0x00547980
HUD::RelativeMode HUD::Matrix::GetRelativeMode() const
{
    return mRelativeMode;
}

// FUNCTION: SWBF2 0x00547990
HUD::Element::Factory::Factory(unsigned int uHashID)
    : HUD::Item::Factory(uHashID)
{
    mType = type_Element;
    mName = "Element";

    mPosition.Init("Position", mProperties, Property::type_Vector3);
    mRelative.Init("PositionRelative", mProperties, HUD::gRelativeModeStrings, 0, 3);
    mRotation.Init("Rotation", mProperties, Property::type_Vector3);
    mScale.Init("Scale", mProperties, Property::type_Vector3);
    mScaleUniform.Init("ScaleUniform", mProperties, 0.0f, FLT_MAX, 0.01f);
    mScale.InitVectorRange(Property::axis_X, 0.0f, FLT_MAX, 0.1f);
    mScale.InitVectorRange(Property::axis_Y, 0.0f, FLT_MAX, 0.1f);
    mScale.InitVectorRange(Property::axis_Z, 0.0f, FLT_MAX, 0.1f);
    mZOrder.Init("ZOrder", mProperties, 0, 255);
    mAlpha.Init("Alpha", mProperties, 0.0f, 1.0f, 0.01f);
    mColor.Init("Color", mProperties, Property::type_Color);
    mColorChange.Init("ColorChange", mProperties, Property::type_Color);
    mColorChangeRate.Init("ColorChangeRate", mProperties, 0.0f, FLT_MAX, 0.001f);
    mColorPulseRate.Init("ColorPulseRate", mProperties, 0.0f, FLT_MAX, 0.001f);
    mUseChangeColor.Init("UseChangeColor", mProperties, Property::type_Bool);
    mFadeInTime.Init("FadeInTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeHoldTime.Init("FadeHoldTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeSustainTime.Init("FadeSustainTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mFadeOutTime.Init("FadeOutTime", mProperties, 0.0f, FLT_MAX, 0.01f);
    mBlendMode.Init("BlendMode", mProperties, sBlendModeStrings, 0, 1);
    mViewport.Init("Viewport", mProperties, 0, 4);
    mEventEnable.Init("EventEnable", mProperties, Property::type_Event);
    mEventDisable.Init("EventDisable", mProperties, Property::type_Event);
    mEventChanged.Init("EventChanged", mProperties, Property::type_Event);
    mEventColor.Init("EventColor", mProperties, Property::type_Event);
    mEventPulseRate.Init("EventPulseRate", mProperties, Property::type_Event);
    mScalable.Init("Scalable", mProperties, Property::type_Bool);
    mScaleAll.Init("ScaleAll", mProperties, 0.001f, FLT_MAX, 0.01f);
}

// FUNCTION: SWBF2 0x00547e60
HUD::Element::Element(RedInterfaceElement& element, const char* const pName,
                      Factory::ItemCallback pCallback, void* const pUserData)
    : HUD::Item(pName, 0, 0),
      mEventFadeOut(0),
      mEnableState(enableState_Disabled),
      mAlpha(1.0f),
      mElement(&element)
{
    mColorDefault = RedColor::WHITE;
    mColorChanged = RedColor::WHITE;
    mColorIntRate = 0.3f;
    mPulseColorRate = FLT_MAX;
    mPulseTime = FLT_MAX;
    mViewport = 0;
    mGroup = 0;
    mRuntimeScale = 1.0f;
    mUseChangeColor = false;
    mFaderChanged = false;
    mAnimChanged = false;
    mHUDEditOnly = false;
    mDynamicAlloc = true;
    mScalable = false;

    mEventEnable.Init(EventEnable, this);
    mEventDisable.Init(EventDisable, this);
    mEventChanged.Init(EventChanged, this);
    mEventColor.Init(EventColor, this);
    mEventPulseRate.Init(EventPulseRate, this);

    sList.AddLast(&mNode);

    if (pCallback)
    {
        pCallback(this, pUserData);
    }
}

// FUNCTION: SWBF2 0x005480b0
HUD::Element::~Element()
{
    if (mElement)
    {
        if (mGroup)
        {
            mGroup->RemoveElement(this);
        }
        else
        {
            RedGroupElement* pGroup = mElement->GetGroup();
            if (pGroup)
            {
                pGroup->RemoveElement(mElement);
            }
        }
    }
    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
    if (mGroupNode.IsAttached())
    {
        mGroupNode.Remove();
    }
}

// FUNCTION: SWBF2 0x005481c0
void HUD::Element::SetViewport(unsigned char uViewport)
{
    if (mGroup)
    {
        mViewport = mGroup->GetViewport();
    }
    else
    {
        mViewport = uViewport;
    }
}

// FUNCTION: SWBF2 0x005481f0
unsigned char HUD::Element::GetViewport() const
{
    if (mGroup)
    {
        return mGroup->GetViewport();
    }
    return mViewport;
}

// FUNCTION: SWBF2 0x00548210
void HUD::Element::Update(const float fTime)
{
    Envelope::Phase ePrevious = mFader.GetPhase();
    Envelope::Phase ePhase;
    if (mFaderChanged)
    {
        ePhase = ePrevious;
    }
    else
    {
        ePhase = mFader.Update(fTime);
    }
    mAnimChanged = mFaderChanged = mFaderChanged || ePrevious != ePhase;

    if (mFaderChanged && mAnimChanged)
    {
        EnableState eState = enableState_Invalid;
        switch (ePhase)
        {
        case Envelope::phase_Inactive:
            eState = enableState_Disabled;
            break;
        case Envelope::phase_Attack:
            eState = enableState_Enabling;
            break;
        case Envelope::phase_Hold:
            eState = enableState_Enabled;
            break;
        case Envelope::phase_Sustain:
        case Envelope::phase_Release:
            eState = enableState_Disabling;
            break;
        }
        if (mEventFadeOut && (eState == enableState_Disabling || eState == enableState_Disabled))
        {

            HUD::Event event;
            event.mClass = mEventFadeOut;
            event.mData.boolValue = true;
            event.Send();
        }
        UpdateEnableState(eState);
    }

    if (mElement->m_bEnabled)
    {
        float fBlend = 1.0f;
        if (mColorIntRate != 0.0f)
        {
            fBlend = 1.0f / mColorIntRate * fTime;
            if (fBlend < 0.0f)
            {
                fBlend = 0.0f;
            }
            else if (fBlend > 1.0f)
            {
                fBlend = 1.0f;
            }
        }
        RedColor current = mElement->m_color;
        RedColor color;
        color.b = (unsigned char)(int)(mColorDefault.b * fBlend + current.b * (1.0f - fBlend) + 0.5f);
        color.g = (unsigned char)(int)(mColorDefault.g * fBlend + current.g * (1.0f - fBlend) + 0.5f);
        color.r = (unsigned char)(int)(mColorDefault.r * fBlend + current.r * (1.0f - fBlend) + 0.5f);
        color.a = (unsigned char)(int)(mColorDefault.a * fBlend + current.a * (1.0f - fBlend) + 0.5f);
        mElement->m_color = color;

        mPulseTime -= fTime;
        if (mPulseTime <= 0.0f)
        {
            mElement->m_color = mColorChanged;
            mPulseTime = mPulseColorRate;
        }

        mElement->m_color.a = (unsigned char)(int)(mFader.CalculateValue() * 255.0f * mAlpha);
    }
}

// FUNCTION: SWBF2 0x005484a0
void HUD::Element::Enable(bool bEnable)
{
    UpdateEnableState(bEnable ? enableState_Enabling : enableState_Disabling);
    Update(0.001f);
}

// FUNCTION: SWBF2 0x005484d0
HUD::Element& HUD::Element::operator=(const Element& other)
{
    mAlpha = other.mAlpha;
    mColorDefault = other.mColorDefault;
    mColorChanged = other.mColorChanged;
    mColorIntRate = other.mColorIntRate;
    mPulseColorRate = other.mPulseColorRate;
    SetViewport(other.GetViewport());
    mUseChangeColor = other.mUseChangeColor;
    mFader = other.mFader;
    mElement->SetZOrder((unsigned char)other.mElement->m_zorder);
    mElement->SetBlendMode(other.mElement->GetBlendMode());
    mElement->m_color = other.mElement->m_color;
    mElement->SetMatrix(other.mElement->m_mLocal);
    mHUDEditOnly = other.mHUDEditOnly;
    return *this;
}

// FUNCTION: SWBF2 0x005485e0
void HUD::Element::SetRuntimeScale(float fScale)
{
    if (!mScalable)
    {
        return;
    }

    PblMatrix matrix = mElement->m_mLocal;
    float afScale[3];
    for (int i = 0; i < 3; ++i)
    {
        const float* pRow = matrix.m[i];
        afScale[i] = sqrtf(pRow[0] * pRow[0] + pRow[1] * pRow[1] + pRow[2] * pRow[2])
                   / mRuntimeScale * fScale;
    }
    mRuntimeScale = fScale;

    for (int i = 0; i < 3; ++i)
    {
        PblVector3 vAxis;
        D3DXVec3Normalize((D3DXVECTOR3*)&vAxis, (const D3DXVECTOR3*)matrix.m[i]);
        matrix.m[i][0] = vAxis.x * afScale[i];
        matrix.m[i][1] = vAxis.y * afScale[i];
        matrix.m[i][2] = vAxis.z * afScale[i];
        matrix.m[i][3] = 0.0f;
    }
    mElement->SetMatrix(matrix);
}

// FUNCTION: SWBF2 0x00548800
bool HUD::Element::SetProperty(const unsigned int uHashID, const void* pValue)
{
    const float* pfValue = (const float*)pValue;
    PblMatrix matrix;

    switch (uHashID)
    {
    case kScalable:
        mScalable = *(const bool*)pValue;
        return true;
    case kEventChanged:
        Item::SetEvent(*(const char* const*)pValue, mEventChanged);
        return true;
    case kRotation:
        matrix = mElement->m_mLocal;
        mMatrix.SetMatrix(&matrix);
        mMatrix.SetRotation(pfValue[0], pfValue[1], pfValue[2]);
        break;
    case kColorChange:
        mColorChanged = *(const RedColor*)pValue;
        return true;
    case kColor:
    {
        const RedColor* pColor = (const RedColor*)pValue;
        RedColor color;
        color.a = 0xff;
        color.b = pColor->b;
        color.g = pColor->g;
        color.r = pColor->r;
        mElement->SetColor(color);
        mColorDefault = mElement->m_color;
        return true;
    }
    case kEventColor:
        Item::SetEvent(*(const char* const*)pValue, mEventColor);
        return true;
    case kAlpha:
        mAlpha = *pfValue;
        return true;
    case kFadeInTime:
        mFader.SetPhaseTime(Envelope::phase_Attack, *pfValue);
        return true;
    case kScaleAll:
        SetRuntimeScaleAll(*pfValue);
        return true;
    case kScaleUniform:
        matrix = mElement->m_mLocal;
        mMatrix.SetMatrix(&matrix);
        mMatrix.ApplyRotation();
        mMatrix.SetScale(*pfValue, *pfValue, *pfValue);
        break;
    case kFadeOutTime:
        mFader.SetPhaseTime(Envelope::phase_Release, *pfValue);
        return true;
    case kUseChangeColor:
        mUseChangeColor = *(const bool*)pValue;
        return true;
    case kScale:
        matrix = mElement->m_mLocal;
        mMatrix.SetMatrix(&matrix);
        mMatrix.ApplyRotation();
        mMatrix.SetScale(pfValue[0], pfValue[1], pfValue[2]);
        break;
    case kEventDisable:
        Item::SetEvent(*(const char* const*)pValue, mEventDisable);
        return true;
    case kFadeHoldTime:
        mFader.SetPhaseTime(Envelope::phase_Hold, *pfValue);
        return true;
    case kPosition:
        mElement->SetPosition(*(const PblVector3*)pValue);
        return true;
    case kColorChangeRate:
        mColorIntRate = *pfValue;
        return true;
    case kEventPulseRate:
        Item::SetEvent(*(const char* const*)pValue, mEventPulseRate);
        return true;
    case kEventEnable:
        Item::SetEvent(*(const char* const*)pValue, mEventEnable);
        return true;
    case kZOrder:
        mElement->SetZOrder(*(const unsigned char*)pValue);
        return true;
    case kColorPulseRate:
        mPulseColorRate = *pfValue;
        mPulseTime = *pfValue;
        return true;
    case kPositionRelative:
        mMatrix.SetRelativeMode(*(const RelativeMode*)pValue);
        return true;
    case kViewport:
        return true;
    case kBlendMode:
        mElement->SetBlendMode(*(const RedInterfaceElement::BlendModeT*)pValue);
        return true;
    case kFadeSustainTime:
        mFader.SetPhaseTime(Envelope::phase_Sustain, *pfValue);
        return true;
    default:
        return false;
    }

    mMatrix.ApplyScale();
    mElement->SetMatrix(matrix);
    mMatrix.SetMatrix(0);
    return true;
}

// FUNCTION: SWBF2 0x00548c50
bool HUD::Element::GetProperty(const unsigned int uHashID, void* pValue)
{
    switch (uHashID)
    {
    case kScalable:
        *(bool*)pValue = mScalable;
        return true;
    case kEventChanged:
        *(const char**)pValue = mEventChanged.GetRegisteredClassName(false);
        return true;
    case kRotation:
        *(PblVector3*)pValue = mMatrix.GetRotation();
        return true;
    case kColorChange:
        *(RedColor*)pValue = mColorChanged;
        return true;
    case kColor:
        *(RedColor*)pValue = mColorDefault;
        return true;
    case kEventColor:
        *(const char**)pValue = mEventColor.GetRegisteredClassName(false);
        return true;
    case kAlpha:
        *(float*)pValue = mAlpha;
        return true;
    case kFadeInTime:
        *(float*)pValue = GetFadeInTime();
        return true;
    case kScaleAll:
        *(float*)pValue = GetRuntimeScaleAll();
        return true;
    case kScaleUniform:
    {
        PblVector3 vScale = mMatrix.GetScale();
        float fMax = vScale.x > vScale.y ? vScale.x : vScale.y;
        *(float*)pValue = fMax > vScale.z ? fMax : vScale.z;
        return true;
    }
    case kFadeOutTime:
        *(float*)pValue = GetFadeOutTime();
        return true;
    case kEventDisable:
        *(const char**)pValue = mEventDisable.GetRegisteredClassName(false);
        return true;
    case kScale:
        *(PblVector3*)pValue = mMatrix.GetScale();
        return true;
    case kUseChangeColor:
        *(bool*)pValue = mUseChangeColor;
        return true;
    case kFadeHoldTime:
        *(float*)pValue = GetFadeHoldTime();
        return true;
    case kPosition:
        *(PblVector3*)pValue = mElement->GetPosition();
        return true;
    case kColorChangeRate:
        *(float*)pValue = mColorIntRate;
        return true;
    case kEventPulseRate:
        *(const char**)pValue = mEventPulseRate.GetRegisteredClassName(false);
        return true;
    case kEventEnable:
        *(const char**)pValue = mEventEnable.GetRegisteredClassName(false);
        return true;
    case kPositionRelative:
        *(RelativeMode*)pValue = mMatrix.GetRelativeMode();
        return true;
    case kColorPulseRate:
        *(float*)pValue = mPulseColorRate;
        return true;
    case kViewport:
        *(unsigned int*)pValue = mViewport;
        return true;
    case kZOrder:
        *(unsigned int*)pValue = mElement->m_zorder;
        return true;
    case kBlendMode:
        *(RedInterfaceElement::BlendModeT*)pValue = mElement->GetBlendMode();
        return true;
    case kFadeSustainTime:
        *(float*)pValue = GetFadeSustainTime();
        return true;
    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x00548f90
const char* HUD::Element::PropertyTranslateEnum(const unsigned int uHashID, const unsigned int uValue)
{
    switch (uHashID)
    {
    case kPositionRelative:
        return gRelativeModeStrings[uValue];
    case kBlendMode:
        return Factory::sBlendModeStrings[uValue];
    }
    return 0;
}

// FUNCTION: SWBF2 0x00549070
float HUD::Element::GetContainerFrameWidth() const
{
    if (mGroup)
    {
        return mGroup->GetWidthPixelsFloat();
    }
    return GetContainerViewWidth();
}

// FUNCTION: SWBF2 0x00549090
float HUD::Element::GetContainerFrameHeight() const
{
    if (mGroup)
    {
        return mGroup->GetHeightPixelsFloat();
    }
    return GetContainerViewHeight();
}

// FUNCTION: SWBF2 0x005490b0
float HUD::Element::GetContainerViewWidth() const
{
    if (GetViewport())
    {
        return sViewportWidth;
    }
    return (float)RedInterfaceScreen::GetScreenWidth();
}

// FUNCTION: SWBF2 0x005490e0
float HUD::Element::GetContainerViewHeight() const
{
    if (GetViewport())
    {
        return sViewportHeight;
    }
    return (float)RedInterfaceScreen::GetScreenHeight();
}

// FUNCTION: SWBF2 0x00549250
bool HUD::Element::ReadData(PblConfig&, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kScalable:
        mScalable = data.GetBoolArg(0);
        return true;
    case kEventChanged:
        return Item::ReadEvent(data, mEventChanged);
    case kColorChange:
    {
        RedColor color;
        color.b = data.GetUint8Arg(2);
        color.g = data.GetUint8Arg(1);
        color.r = data.GetUint8Arg(0);
        color.a = 0xff;
        mColorChanged = color;
        return true;
    }
    case kColor:
    {
        RedColor color;
        color.b = data.GetUint8Arg(2);
        color.g = data.GetUint8Arg(1);
        color.r = data.GetUint8Arg(0);
        color.a = 0xff;
        mElement->SetColor(color);
        mColorDefault = mElement->m_color;
        return true;
    }
    case kFadeInTime:
        mFader.SetPhaseTime(Envelope::phase_Attack, data.GetFloatArg(0));
        return true;
    case kEventColor:
        return Item::ReadEvent(data, mEventColor);
    case kAlpha:
        mAlpha = data.GetFloatArg(0);
        return true;
    case kAnimEnable:
    case kAnimDisable:
    case kAnimChange:
        return true;
    case kFadeOutTime:
        mFader.SetPhaseTime(Envelope::phase_Release, data.GetFloatArg(0));
        return true;
    case kFadeHoldTime:
        mFader.SetPhaseTime(Envelope::phase_Hold, data.GetFloatArg(0));
        return true;
    case kFadeSustainTime:
        mFader.SetPhaseTime(Envelope::phase_Sustain, data.GetFloatArg(0));
        return true;
    case kEventDisable:
        return Item::ReadEvent(data, mEventDisable);
    case kUseChangeColor:
        mUseChangeColor = data.GetBoolArg(0);
        return true;
    case kColorChangeRate:
        mColorIntRate = data.GetFloatArg(0);
        return true;
    case kEventFadeOut:
        mEventFadeOut = Item::CreateEventA(data.GetStringArg(0), EventClass::type_Bool);
        return true;
    case kEditOnly:
        mHUDEditOnly = data.GetBoolArg(0);
        return true;
    case kEventPulseRate:
        return Item::ReadEvent(data, mEventPulseRate);
    case kZOrder:
        mElement->SetZOrder(data.GetUint8Arg(0));
        return true;
    case kEventEnable:
        return Item::ReadEvent(data, mEventEnable);
    case kColorPulseRate:
        mPulseColorRate = data.GetFloatArg(0);
        mPulseTime = data.GetFloatArg(0);
        return true;
    case kViewport:
    {
        int iViewport = data.GetUint8Arg(0);
        if (iViewport < 0)
        {
            iViewport = 0;
        }
        else if (iViewport > 4)
        {
            iViewport = 4;
        }
        SetViewport((unsigned char)iViewport);
        return true;
    }
    case kBlendMode:
    {
        unsigned int uMode = TranslateString((const char* const*)Factory::sBlendModeStrings, 2,
                                        data.GetStringArg(0));
        if (uMode < 2)
        {
            mElement->SetBlendMode((RedInterfaceElement::BlendModeT)uMode);
        }
        return true;
    }
    default:
    {
        PblMatrix matrix = mElement->m_mLocal;
        mMatrix.SetMatrix(&matrix);
        bool bRead = mMatrix.ReadScale(data, 0);
        bRead |= mMatrix.ReadRotation(data, 0);
        bRead |= mMatrix.ReadPosition(data, 0);
        if (bRead)
        {
            mElement->SetMatrix(matrix);
        }
        mMatrix.SetMatrix(0);
        return bRead;
    }
    }
}

// FUNCTION: SWBF2 0x00549700
void HUD::Element::PostReadSetup()
{
    PblMatrix matrix = mElement->m_mLocal;
    mMatrix.SetMatrix(&matrix);
    mMatrix.ApplyScale();
    mMatrix.ApplyRelativeMode(GetContainerFrameWidth(), GetContainerFrameHeight(),
                              GetContainerViewWidth(), GetContainerViewHeight(), InScreenGroup());
    mElement->SetMatrix(*mMatrix.GetMatrix());
}

// FUNCTION: SWBF2 0x00549800
void HUD::Element::WriteData(HUD::File& file, unsigned int uTabs)
{
    PblMatrix matrix = mElement->m_mLocal;
    mMatrix.SetMatrix(&matrix);
    file.WriteProperty("EditOnly", uTabs, (int)mHUDEditOnly, 0);
    file.WriteProperty("Viewport", uTabs, (int)GetViewport(), 0);
    mMatrix.WritePosition(file, uTabs, GetContainerFrameWidth(), GetContainerFrameHeight(),
                          GetContainerViewWidth(), GetContainerViewHeight(), 0, InScreenGroup());
    mMatrix.WriteRotation(file, uTabs, 0);
    mMatrix.WriteScale(file, uTabs, 0);
    file.WriteProperty("ZOrder", uTabs, (int)mElement->m_zorder, 128);
    file.WriteProperty("Alpha", uTabs, mAlpha, 1.0f);
    file.WriteProperty("Color", uTabs, mColorDefault.r, mColorDefault.g, mColorDefault.b,
                       255, 255, 255);
    file.WriteProperty("ColorChange", uTabs, mColorChanged.r, mColorChanged.g, mColorChanged.b,
                       255, 255, 255);
    file.WriteProperty("ColorChangeRate", uTabs, mColorIntRate, 0.3f);
    file.WriteProperty("ColorPulseRate", uTabs, mPulseColorRate, FLT_MAX);
    file.WriteProperty("UseChangeColor", uTabs, (int)mUseChangeColor, 0);
    file.WriteProperty("FadeInTime", uTabs, GetFadeInTime(), 0.0f);
    file.WriteProperty("FadeHoldTime", uTabs, GetFadeHoldTime(), FLT_MAX);
    file.WriteProperty("FadeSustainTime", uTabs, GetFadeSustainTime(), 0.0f);
    file.WriteProperty("FadeOutTime", uTabs, GetFadeOutTime(), 0.0f);
    file.WriteProperty("BlendMode", uTabs, Factory::sBlendModeStrings[mElement->GetBlendMode()],
                       Factory::sBlendModeStrings[0]);
    file.WriteProperty("EventEnable", uTabs, mEventEnable.GetRegisteredClassName(true), "");
    file.WriteProperty("EventDisable", uTabs, mEventDisable.GetRegisteredClassName(true), "");
    file.WriteProperty("EventChanged", uTabs, mEventChanged.GetRegisteredClassName(true), "");
    file.WriteProperty("EventColor", uTabs, mEventColor.GetRegisteredClassName(true), "");
    file.WriteProperty("EventPulseRate", uTabs, mEventPulseRate.GetRegisteredClassName(true), "");
    if (mEventFadeOut)
    {
        file.WriteProperty("EventFadeOut", uTabs, mEventFadeOut->GetName(), "");
    }
    file.WriteProperty("Scalable", uTabs, (int)mScalable, 0);
    mMatrix.SetMatrix(0);
}

// FUNCTION: SWBF2 0x00549be0
void HUD::Element::ResetFlags()
{
    mFaderChanged = false;
    mAnimChanged = false;
}

// FUNCTION: SWBF2 0x00549bf0
void HUD::Element::UpdateEnableState(EnableState eState)
{
    RedScreenGroupElement* pScreen;
    if (mHUDEditOnly)
    {
        pScreen = &gScreenGroupEdit[GetViewport()];
    }
    else
    {
        pScreen = &gScreenGroup[GetViewport()];
    }

    bool bSetState = true;
    switch (eState)
    {
    case enableState_Enabling:
        if (mEnableState != enableState_Enabling && mEnableState != enableState_Enabled)
        {
            if (mElement->GetGroup() || pScreen->AddElement(mElement))
            {
                mElement->Enable(true);
            }
        }
        mFader.Start();
        break;
    case enableState_Disabling:
        if (mEnableState != enableState_Disabling && mEnableState != enableState_Disabled)
        {
            mFader.Stop();
        }
        else
        {
            bSetState = false;
        }
        break;
    case enableState_Disabled:
        if (mElement->m_bEnabled)
        {
            mElement->SetColorOnly(mColorDefault);
            mElement->Enable(false);
            if (mElement->GetGroup() == pScreen)
            {
                pScreen->RemoveElement(mElement);
            }
        }
        break;
    }
    ResetFlags();
    if (bSetState)
    {
        mEnableState = eState;
    }
}

// FUNCTION: SWBF2 0x00549e50
void HUD::Element::SetRuntimeScaleAll(float fScale)
{
    for (PblListDouble::Node* pNode = sList.mTerminator.mNext; pNode != &sList.mTerminator;
         pNode = pNode->mNext)
    {
        Element* pElement = (Element*)((char*)pNode - offsetof(Element, mNode));
        pElement->SetRuntimeScale(fScale);
    }
    sRuntimeScale = fScale;
}

// FUNCTION: SWBF2 0x00549ea0
float HUD::Element::GetRuntimeScaleAll()
{
    return sRuntimeScale;
}

float HUD::Element::GetFadeInTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Attack);
}

float HUD::Element::GetFadeHoldTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Hold);
}

float HUD::Element::GetFadeSustainTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Sustain);
}

float HUD::Element::GetFadeOutTime() const
{
    return mFader.GetPhaseTime(Envelope::phase_Release);
}

// FUNCTION: SWBF2 0x00549200
bool HUD::Element::InScreenGroup() const
{
    if (!mElement->GetGroup())
    {
        return true;
    }
    return mElement->GetGroup() == &gScreenGroup[GetViewport()];
}

// FUNCTION: SWBF2 0x00549fe0
void HUD::Element::EventEnable(const HUD::Event& event, void* const pData)
{
    Element* pElement = (Element*)pData;
    pElement->UpdateEnableState(enableState_Enabling);
    pElement->Update(0.001f);
}

// FUNCTION: SWBF2 0x0054a010
void HUD::Element::EventDisable(const HUD::Event& event, void* const pData)
{
    Element* pElement = (Element*)pData;
    pElement->UpdateEnableState(enableState_Disabling);
    pElement->Update(0.001f);
}

// FUNCTION: SWBF2 0x0054a040
void HUD::Element::Changed()
{
    if (mUseChangeColor)
    {
        mElement->SetColorOnly(mColorChanged);
    }
}

// FUNCTION: SWBF2 0x0054a070
void HUD::Element::EventChanged(const HUD::Event& event, void* const pData)
{
    ((Element*)pData)->Changed();
}

// FUNCTION: SWBF2 0x0054a080
void HUD::Element::EventColor(const HUD::Event& event, void* const pData)
{
    if (event.GetClass()->GetType() == EventClass::type_Color)
    {
        ((Element*)pData)->mColorDefault = *event.GetData().color;
    }
}

// FUNCTION: SWBF2 0x0054a0c0
void HUD::Element::EventPulseRate(const HUD::Event& event, void* const pData)
{
    switch (event.GetClass()->GetType())
    {
    case EventClass::type_Int:
    case EventClass::type_Uint:
    case EventClass::type_Float:
    {
        Element* pElement = (Element*)pData;
        if (fabs(event.GetDataFloat() - pElement->mPulseColorRate) > 0.0001f)
        {
            pElement->mPulseTime = event.GetDataFloat();
        }
        pElement->mPulseColorRate = event.GetDataFloat();
        break;
    }
    default:
        break;
    }
}

// FUNCTION: SWBF2 0x00548fd0
PblMatrix HUD::Element::CalculateScreenMatrix() const
{
    PblMatrix result = mElement->GetMatrix();
    if (mGroup)
    {
        const PblMatrix parent = CalculateGroupMatrix(mGroup);
        PblMatrix combined;
        D3DXMatrixMultiply((D3DXMATRIX*)&combined, (const D3DXMATRIX*)&parent,
                           (const D3DXMATRIX*)&result);
        result = combined;
    }
    return result;
}

// FUNCTION: SWBF2 0x00549d20
HUD::Element* HUD::Element::GetFirst()
{
    PblListDouble::Node* node = sList.mTerminator.mNext;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d40
HUD::Element* HUD::Element::GetLast()
{
    PblListDouble::Node* node = sList.mTerminator.mPrev;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d70
HUD::Element* HUD::Element::GetNext(Element* const pElement)
{
    PblListDouble::Node* node = pElement->mNode.mNext;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549d90
HUD::Element* HUD::Element::GetPrevious(Element* const pElement)
{
    PblListDouble::Node* node = pElement->mNode.mPrev;
    return node == &sList.mTerminator ? 0 : reinterpret_cast<Element*>(reinterpret_cast<char*>(node) - offsetof(Element, mNode));
}

// FUNCTION: SWBF2 0x00549db0
HUD::Element* HUD::Element::FindByHashID(unsigned int uHashID)
{
    for (Element* pElement = GetFirst(); pElement; pElement = GetNext(pElement))
    {
        if (pElement->mNameHash == uHashID)
        {
            return pElement;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00549de0
void HUD::Element::SetupViewportDimensions(unsigned int uNumViewports, bool bVerticalSplit)
{
    sViewportWidth = (float)(int)RedInterfaceScreen::GetScreenWidth();
    sViewportHeight = (float)(int)RedInterfaceScreen::GetScreenHeight();
    switch (uNumViewports)
    {
    case 2:
        if (bVerticalSplit)
        {
            sViewportWidth *= 0.5f;
        }
        else
        {
            sViewportHeight *= 0.5f;
        }
        break;
    case 3:
    case 4:
        sViewportWidth *= 0.5f;
        sViewportHeight *= 0.5f;
        break;
    }
}

// FUNCTION: SWBF2 0x00549f00
void HUD::Element::DestroyAll()
{
    PblListDouble dynamicList;

    PblListDouble::Node* pNode = sList.mTerminator.mNext;
    while (pNode != &sList.mTerminator)
    {
        Element* pElement = (Element*)((char*)pNode - offsetof(Element, mNode));
        pNode = pNode->mNext;
        if (pElement->mDynamicAlloc)
        {
            pElement->mNode.Remove();
            dynamicList.AddLast(&pElement->mNode);
        }
    }

    while (dynamicList.mTerminator.mNext != &dynamicList.mTerminator)
    {
        PblListDouble::Node* pFirst = dynamicList.mTerminator.mNext;
        Element* pElement = pFirst == &dynamicList.mTerminator
                                ? 0
                                : (Element*)((char*)pFirst - offsetof(Element, mNode));
        delete pElement;
    }

    sList.Clear();
}
