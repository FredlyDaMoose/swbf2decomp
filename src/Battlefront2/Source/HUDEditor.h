#pragma once

#include <stddef.h>

#include "HUDItem.h"

#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Interface/RedVectorElement.h"
#include "RedEngineFL/input/RedInputEventHandler.h"

class RedModel;
class RedTexture;
class RawControllerInputs;

namespace HUD { class Element; }

// VTABLE: SWBF2 0x007a0578

namespace HUD {
class Editor : public RedInputEventHandler
{
public:

    enum Mode
    {
        mode_Invalid = 0,
        mode_Disabled = 1,
        mode_SelectElement = 2,
        mode_SelectProperty = 3,
        mode_EditProperty = 4
    };

    enum Control
    {
        control_Invalid = 0,
        control_LLeft = 1,
        control_LRight = 2,
        control_LUp = 3,
        control_LDown = 4,
        control_RLeft = 5,
        control_RRight = 6,
        control_RUp = 7,
        control_RDown = 8,
        control_L1 = 9,
        control_L2 = 10,
        control_R1 = 11,
        control_R2 = 12,
        control_Start = 13,
        control_Select = 14,
        control_Max = 15
    };

    union Value
    {
        bool boolValue;
        unsigned int enumValue;
        int intValue;
        float floatValue;
        char* eventValue;
        unsigned char colorValue[4];
        unsigned char vector2Value[12];
        unsigned char vector3Value[12];
        unsigned char vector4Value[16];
        RedModel* modelValue;
        RedTexture* textureValue;
    };

    // FUNCTION: SWBF2 0x00544c90
    Editor();

    // SYNTHETIC: SWBF2 0x00544eb0
    // HUD::Editor::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x00544ee0
    virtual ~Editor();

    // FUNCTION: SWBF2 0x00545020
    void Update(RawControllerInputs* const pInputs, float fDelta);

protected:

    // FUNCTION: SWBF2 0x00546e20
    virtual void KeyboardEvent(RedInputEventType eType, RedKeyboard* pKeyboard,
                               unsigned char ucA, unsigned char ucB,
                               unsigned short usC);

    // FUNCTION: SWBF2 0x00544fb0
    void Enable(bool bEnable);

    // FUNCTION: SWBF2 0x00546090
    float GetFloatStep(Control eControl);
    // FUNCTION: SWBF2 0x005460c0
    void NextElement();
    // FUNCTION: SWBF2 0x00546100
    void PrevElement();
    // FUNCTION: SWBF2 0x00546140
    unsigned int ElementSelectedIndex();
    // FUNCTION: SWBF2 0x00546170
    void UpdateElementDisplay();
    // FUNCTION: SWBF2 0x00546200
    void HighlightElement(bool bHighlight);
    // FUNCTION: SWBF2 0x005464e0
    void NextProperty();
    // FUNCTION: SWBF2 0x00546510
    void PrevProperty();
    // FUNCTION: SWBF2 0x00546540
    unsigned int PropertySelectedIndex();
    // FUNCTION: SWBF2 0x00546590
    bool PropertyString(const HUD::Item::Factory::Property* const pProperty,
                        char* const pBuffer);
    // FUNCTION: SWBF2 0x00546860
    void DisplayProperties();
    // FUNCTION: SWBF2 0x00546ab0
    PblVector3 GetScreenElementPosition();
    // FUNCTION: SWBF2 0x00546bc0
    void DisplayProperty(bool bDisplay);
    // FUNCTION: SWBF2 0x00546d30
    bool GetControlValue(RawControllerInputs* const pInputs, Control eControl,
                         bool bPressed);
    // FUNCTION: SWBF2 0x00546e80
    void SetMode(Mode eMode);

public:

    HUD::Element* mElement;
    HUD::Item::Factory::Property* mProperty;
    Value mValue;
    Mode mMode;
    RedScreenGroupElement mScreenElement;
    RedTextElement mInfo;
    RedTextElement mMenu[26];
    RedVectorElement mSafeZone;
    RedGroupElement mBorderGroup;
    RedVectorElement mBorder;
    RedBitmapElement mBackdrop;

    float mPressedTime[15];
    bool mKeyDown[15];

    bool mEnabled : 1;
    bool mElementList : 1;
    bool mFirstUpdate : 1;
};
}

namespace HUD {

extern Editor::Mode gEditMode;
}

 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
