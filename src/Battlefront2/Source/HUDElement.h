#pragma once

#include <stddef.h>
#include "HUDItem.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "HUDEnvelope.h"

class PblMatrix;

namespace HUD { class ElementGroupBase; }
namespace HUD { class EventClass; }
class RedInterfaceElement;

namespace HUD {

extern char* gRelativeModeStrings[4];

enum RelativeMode
{
    relativeMode_Pixels = 0,
    relativeMode_Screen = 1,
    relativeMode_Viewport = 2,
    relativeMode_Frame = 3,
    relativeMode_MAX = 3
};

float ConvertRelativeToPixels(RelativeMode eMode, float fValue, float fFrameSize,
                              float fViewSize, float fScreenSize);
float ConvertPixelsToRelative(RelativeMode eMode, float fValue, float fFrameSize,
                              float fViewSize, float fScreenSize);

float ConvertXForWideScreen(float fX, float fWidth, float fHeight);
float ConvertWideScreenX(float fX, float fWidth, float fHeight);

class Matrix
{
public:
    Matrix();
    void ApplyRelativeMode(float, float, float, float, bool);
    bool ReadPosition(PblConfig::Data& data, unsigned int);
    void WritePosition(HUD::File& file, unsigned int uTabs, float, float, float, float,
                       const char* const pName, bool);
    bool ReadRotation(PblConfig::Data& data, unsigned int);
    void WriteRotation(HUD::File& file, unsigned int uTabs, const char* const pName);
    bool ReadScale(PblConfig::Data& data, unsigned int);
    void WriteScale(HUD::File& file, unsigned int uTabs, const char* const pName);
    void SetMatrix(PblMatrix* const pMatrix);
    const PblMatrix* GetMatrix() const;
    void SetRotation(float fX, float fY, float fZ);
    void ApplyRotation();
    void SetScale(float fX, float fY, float fZ);
    const PblVector3& GetScale() const;
    void ApplyScale();
    Matrix& operator=(const Matrix& other);
    const PblVector3& GetRotation() const;
    void SetRelativeMode(RelativeMode eMode);
    RelativeMode GetRelativeMode() const;

    PblVector3 mScale;
    PblMatrix* mMatrix;
    RelativeMode mRelativeMode;
    PblVector3 mRotation;
};

class Element : public HUD::Item
{
public:
    static void UpdateAll(float fDeltaTime);

    Element& operator=(const Element& other);

    bool InScreenGroup() const;

    // VTABLE: SWBF2 0x007a0774

    class Factory : public HUD::Item::Factory
    {
    public:
        // FUNCTION: SWBF2 0x00547990
        Factory(unsigned int uHashID);

        // SYNTHETIC: SWBF2 0x00547e30
        // HUD::Element::Factory::`scalar deleting destructor'

        static char* sBlendModeStrings[2];

        Property mPosition;
        Property mRelative;
        Property mRotation;
        Property mScale;
        Property mScaleUniform;
        Property mZOrder;
        Property mAlpha;
        Property mColor;
        Property mColorChange;
        Property mColorChangeRate;
        Property mColorPulseRate;
        Property mUseChangeColor;
        Property mFadeInTime;
        Property mFadeHoldTime;
        Property mFadeSustainTime;
        Property mFadeOutTime;
        Property mBlendMode;
        Property mViewport;
        Property mEventEnable;
        Property mEventDisable;
        Property mEventChanged;
        Property mEventColor;
        Property mEventPulseRate;
        Property mScalable;
        Property mScaleAll;
    };

    virtual ~Element();
    // SYNTHETIC: SWBF2 0x00548080
    // HUD::Element::`scalar deleting destructor'
    virtual Factory::Type GetType() const;
    virtual bool WriteEnabled() const;
    virtual void Update(const float);
    virtual void GetSize(float &, float &) = 0;

    enum EnableState
    {
        enableState_Invalid = 0,
        enableState_Enabling = 1,
        enableState_Enabled = 2,
        enableState_Disabling = 3,
        enableState_Disabled = 4
    };

    virtual void UpdateEnableState(EnableState eState);

    virtual bool SetProperty(const unsigned int, const void*);
    virtual bool GetProperty(const unsigned int, void*);
    virtual const char* PropertyTranslateEnum(const unsigned int, const unsigned int);
    virtual bool ReadData(PblConfig& config, PblConfig::Data& data);
    virtual void PostReadSetup();
    virtual void WriteData(HUD::File&, unsigned int);

    Element(RedInterfaceElement& element, const char* const pName,
            Factory::ItemCallback pCallback, void* const pUserData);

    unsigned char GetViewport() const;
    void SetViewport(unsigned char uViewport);

    float GetContainerFrameWidth() const;
    float GetContainerFrameHeight() const;
    float GetContainerViewWidth() const;
    float GetContainerViewHeight() const;

    void Enable(bool bEnable);

    static Element* FindByHashID(unsigned int uHashID);

    static Element* GetFirst();
    static Element* GetLast();
    static Element* GetNext(Element* const pElement);
    static Element* GetPrevious(Element* const pElement);

    PblMatrix CalculateScreenMatrix() const;
protected:

    PblMatrix CalculateGroupMatrix(HUD::ElementGroupBase* const pGroup) const;
public:

    static float sViewportWidth;
    static float sViewportHeight;

    static void SetupViewportDimensions(unsigned int uNumViewports, bool bVerticalSplit);

    static float GetViewportWidth() { return sViewportWidth; }
    static float GetViewportHeight() { return sViewportHeight; }

    static void SetRuntimeScaleAll(float fScale);
    static float GetRuntimeScaleAll();

    static float sRuntimeScale;

    RedInterfaceElement& GetRedInterfaceElement() { return *mElement; }

    void Changed();

    void SetRuntimeScale(float fScale);

    static PblListDouble sList;

    static void DestroyAll();

protected:

    static void EventEnable(const HUD::Event& event, void* const pData);
    static void EventDisable(const HUD::Event& event, void* const pData);
    static void EventColor(const HUD::Event& event, void* const pData);
    static void EventPulseRate(const HUD::Event& event, void* const pData);

    static void EventChanged(const HUD::Event& event, void* const pData);
public:

    float GetFadeInTime() const;

    float GetFadeHoldTime() const;
    float GetFadeSustainTime() const;
    float GetFadeOutTime() const;

    void ResetFlags();

    void SetDynamicallyAllocated(bool bDynamic) { mDynamicAlloc = bDynamic; }

    float GetFadeHoldElapsed() const;

    PblListDouble::Node mGroupNode;

    HUD::EventHandler mEventEnable;
    HUD::EventHandler mEventDisable;
    HUD::EventHandler mEventChanged;
    HUD::EventHandler mEventColor;
    HUD::EventHandler mEventPulseRate;
    HUD::EventClass* mEventFadeOut;
    HUD::Envelope mFader;
    EnableState mEnableState;
    float mAlpha;
    RedInterfaceElement* mElement;
    PblListDouble::Node mNode;
    HUD::Matrix mMatrix;
    RedColor mColorDefault;
    RedColor mColorChanged;
    float mColorIntRate;
    float mPulseColorRate;
    float mPulseTime;
    unsigned char mViewport;
    HUD::ElementGroupBase* mGroup;
    float mRuntimeScale;
    bool mUseChangeColor : 1;
    bool mFaderChanged : 1;
    bool mAnimChanged : 1;
    bool mHUDEditOnly : 1;
    bool mDynamicAlloc : 1;
    bool mScalable : 1;
};
}

 
 

 
 
 
 

 
 
 

 
 
 
 
 
 
 
 
 
 
