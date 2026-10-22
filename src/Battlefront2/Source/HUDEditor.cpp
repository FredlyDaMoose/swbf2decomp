#include <stdio.h>
#include <string.h>

#include "HUDEditor.h"

#include "FLInputManager.h"
#include "GameLoop.h"
#include "HUDElement.h"
#include "HUDEvent.h"
#include "HUDManager.h"

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "PlayerController.h"

// GLOBAL: SWBF2 0x007e6900
HUD::Editor::Mode HUD::gEditMode = HUD::Editor::mode_Disabled;

static const float EPSILON = 0.0001f;

static inline int const Clamp(int iValue, int iMin, int iMax)
{
    if (iValue < iMin)
    {
        return iMin;
    }
    if (iValue > iMax)
    {
        return iMax;
    }
    return iValue;
}

static inline float const Clamp(float fValue, float fMin, float fMax)
{
    if (fValue < fMin)
    {
        return fMin;
    }
    if (fValue > fMax)
    {
        return fMax;
    }
    return fValue;
}

namespace HUD {

class EditHeap
{
public:
    EditHeap()
    {
        if (gEditHeap != -1)
        {
            mHeap = RedSetCurrentHeap(gEditHeap);
        }
    }
    ~EditHeap();

    int mHeap;
};

// FUNCTION: SWBF2 0x00544c70
EditHeap::~EditHeap()
{
    if (gEditHeap != -1)
    {
        RedSetCurrentHeap(mHeap);
    }
}
}

static RedKeyboard* GetInputsKeyboard(RawControllerInputs* const pInputs)
{
    return *(RedKeyboard**)&pInputs->mRawInputStorage[0x2608 - 4];
}

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
void RemoveInterfaceElement(RedScreenGroupElement* pElement);
}

// FUNCTION: SWBF2 0x00544c90
HUD::Editor::Editor()
{
    mElement = 0;
    mProperty = 0;
    mMode = mode_Disabled;

    mEnabled = false;
    mElementList = true;
    mFirstUpdate = true;

    mSafeZone.Enable(false);

    mScreenElement.m_bUseSafeZone = false;
    mScreenElement.SetZOrder(0);
    mScreenElement.Enable(true);
    mScreenElement.AddElement(&mInfo);

    mBorderGroup.AddElement(&mBorder);
    mBorderGroup.Enable(true);

    mBackdrop.Init();

    const RedColor cBackdrop = { 0x00, 0x44, 0x00, 0x80 };
    mBackdrop.m_color = cBackdrop;

    mBackdrop.Enable(true);

    mScreenElement.AddElement(&mBackdrop);
    mScreenElement.AddElement(&mBorderGroup);

    RedInputEventHandler::AddEventHandler(this);

    float fY = 0.0f;
    for (int i = 0; i < 26; i++)
    {
        mMenu[i].SetPosition(0.0f, fY);
        mMenu[i].m_eStyle = RedGlyphCacheElement::TEXT_STYLE_SHADOW;
        mScreenElement.AddElement(&mMenu[i]);
        fY += 15.0f;
    }

    memset(mPressedTime, 0, sizeof(mPressedTime));
    memset(mKeyDown, 0, sizeof(mKeyDown));
}

// FUNCTION: SWBF2 0x00544ee0
HUD::Editor::~Editor()
{
    Enable(false);
    RedInputEventHandler::RemoveEventHandler(this);
}

// FUNCTION: SWBF2 0x00544fb0
void HUD::Editor::Enable(bool bEnable)
{
    if (mEnabled == bEnable)
    {
        return;
    }

    if (bEnable)
    {
        mScreenElement.m_bUseSafeZone = false;
        FLRenderer::AddInterfaceElement(&mScreenElement,
                                        RedInterfaceScreen::LEFT,
                                        RedInterfaceScreen::TOP, true);
        mEnabled = true;
    }
    else
    {
        FLRenderer::RemoveInterfaceElement(&mScreenElement);
        mEnabled = false;
    }
}

const RedModel* RedModel::GetFirst()
{
    return PblHashTable<RedModel, 1024>::Itor(&_HashTable);
}

// FUNCTION: SWBF2 0x00544950
const RedModel* RedModel::GetLast()
{
    const RedModel* pLast = 0;
    for (PblHashTable<RedModel, 1024>::Itor it(&_HashTable); (RedModel*)it; ++it)
    {
        pLast = it;
    }
    return pLast;
}

// FUNCTION: SWBF2 0x005449c0
const RedModel* RedModel::GetNext(const RedModel* const pModel)
{
    bool bFound = false;
    PblHashTable<RedModel, 1024>::Itor it(&_HashTable);
    while ((RedModel*)it && !bFound)
    {
        bFound = (pModel == it);
        ++it;
    }
    const RedModel* pNext = it;
    if (!pNext)
    {
        pNext = GetFirst();
    }
    return pNext;
}

// FUNCTION: SWBF2 0x00544a60
const RedModel* RedModel::GetPrev(const RedModel* const pModel)
{
    const RedModel* pPrev = pModel;
    for (PblHashTable<RedModel, 1024>::Itor it(&_HashTable); (RedModel*)it; ++it)
    {
        if (pModel == it)
        {
            break;
        }
        pPrev = it;
    }
    return pPrev;
}

const RedTexture* RedTexture::GetFirst()
{
    return PblHashTable<RedTexture, 4096>::Itor(&s_textureHashTable);
}

// FUNCTION: SWBF2 0x00544ae0
const RedTexture* RedTexture::GetLast()
{
    const RedTexture* pLast = 0;
    for (PblHashTable<RedTexture, 4096>::Itor it(&s_textureHashTable); (RedTexture*)it; ++it)
    {
        pLast = it;
    }
    return pLast;
}

// FUNCTION: SWBF2 0x00544b50
const RedTexture* RedTexture::GetNext(const RedTexture* const pTexture)
{
    bool bFound = false;
    PblHashTable<RedTexture, 4096>::Itor it(&s_textureHashTable);
    while ((RedTexture*)it && !bFound)
    {
        bFound = (pTexture == it);
        ++it;
    }
    const RedTexture* pNext = it;
    if (!pNext)
    {
        pNext = GetFirst();
    }
    return pNext;
}

// FUNCTION: SWBF2 0x00544bf0
const RedTexture* RedTexture::GetPrev(const RedTexture* const pTexture)
{
    const RedTexture* pPrev = pTexture;
    for (PblHashTable<RedTexture, 4096>::Itor it(&s_textureHashTable); (RedTexture*)it; ++it)
    {
        if (pTexture == it)
        {
            break;
        }
        pPrev = it;
    }
    return pPrev;
}

// FUNCTION: SWBF2 0x00545020
void HUD::Editor::Update(RawControllerInputs* const pInputs, float fDelta)
{
    typedef Item::Factory::Property Property;

    if (mFirstUpdate)
    {
        mInfo.SetFont("systemfont");
        mInfo.SetTextScale(0.8f, 0.8f);
        for (int i = 0; i < 26; i++)
        {
            mMenu[i].SetFont("systemfont");
            mMenu[i].SetTextScale(0.8f, 0.8f);
        }
        mFirstUpdate = false;
    }

    if (gEditMode != mMode)
    {
        SetMode(gEditMode);
    }

    switch (mMode)
    {
    case mode_Disabled:
        return;

    case mode_SelectElement:
    {
        bool bChanged = false;
        if (GetControlValue(pInputs, control_LUp, true))
        {
            HighlightElement(false);
            PrevElement();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LLeft, false))
        {
            HighlightElement(false);
            float fStep = GetFloatStep(control_LLeft);
            for (float f = 0.0f; f < fStep; f += 1.0f)
                ;
            PrevElement();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LDown, true))
        {
            HighlightElement(false);
            NextElement();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LRight, false))
        {
            HighlightElement(false);
            float fStep = GetFloatStep(control_LRight);
            for (float f = 0.0f; f < fStep; f += 1.0f)
                ;
            NextElement();
            bChanged = true;
        }

        if (GetControlValue(pInputs, control_L2, true))
        {
            mElementList = !mElementList;
            bChanged = true;
        }

        if (GetControlValue(pInputs, control_R1, true))
        {
            HighlightElement(false);
            SetMode(mode_SelectProperty);
        }
        else if (bChanged)
        {
            HighlightElement(true);
        }
        break;
    }

    case mode_SelectProperty:
    {
        bool bChanged = false;
        if (GetControlValue(pInputs, control_LUp, true))
        {
            PrevProperty();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LLeft, false))
        {
            float fStep = GetFloatStep(control_LLeft);
            for (float f = 0.0f; f < fStep; f += 1.0f)
                ;
            PrevProperty();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LDown, true))
        {
            NextProperty();
            bChanged = true;
        }
        else if (GetControlValue(pInputs, control_LRight, false))
        {
            float fStep = GetFloatStep(control_LRight);
            for (float f = 0.0f; f < fStep; f += 1.0f)
                ;
            NextProperty();
            bChanged = true;
        }
        if (bChanged)
        {
            DisplayProperties();
        }

        if (GetControlValue(pInputs, control_R1, true))
        {
            SetMode(mode_EditProperty);
            DisplayProperty(true);
        }
        if (GetControlValue(pInputs, control_L1, true))
        {
            DisplayProperty(false);
            SetMode(mode_SelectElement);
        }
        break;
    }

    case mode_EditProperty:
    {
        bool bChanged = false;
        switch (mProperty->GetType())
        {
        case Property::type_Bool:
            if (GetControlValue(pInputs, control_LLeft, true) ||
                GetControlValue(pInputs, control_LRight, true))
            {
                mValue.boolValue = !mValue.boolValue;
                bChanged = true;
            }
            break;

        case Property::type_Enum:
            if (GetControlValue(pInputs, control_LLeft, true))
            {
                if (mValue.enumValue > mProperty->GetMinEnumValue())
                {
                    mValue.enumValue--;
                }
                else
                {
                    mValue.enumValue = mProperty->GetMaxEnumValue();
                }
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, true))
            {
                if (mValue.enumValue < mProperty->GetMaxEnumValue())
                {
                    mValue.enumValue++;
                }
                else
                {
                    mValue.enumValue = mProperty->GetMinEnumValue();
                }
                bChanged = true;
            }
            break;

        case Property::type_Int:
            if (GetControlValue(pInputs, control_LLeft, false) &&
                mValue.intValue > mProperty->GetMinIntValue())
            {
                if (mPressedTime[control_LLeft] < EPSILON)
                {
                    mValue.intValue--;
                }
                else
                {
                    mValue.intValue -= (int)(mPressedTime[control_LLeft] * 2.0f);
                }
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, false) &&
                mValue.intValue < mProperty->GetMaxIntValue())
            {
                if (mPressedTime[control_LRight] < EPSILON)
                {
                    mValue.intValue++;
                }
                else
                {
                    mValue.intValue -= (int)(mPressedTime[control_LRight] * -2.0f);
                }
                bChanged = true;
            }
            if (bChanged)
            {
                mValue.intValue = Clamp(mValue.intValue, mProperty->GetMinIntValue(),
                                        mProperty->GetMaxIntValue());
            }
            break;

        case Property::type_Float:
            if (GetControlValue(pInputs, control_LLeft, false))
            {
                mValue.floatValue -= mProperty->GetStepFloatValue() *
                                     (float)(int)(mPressedTime[control_LLeft] + 1.0f);
                mValue.floatValue = Clamp(mValue.floatValue, mProperty->GetMinFloatValue(),
                                          mProperty->GetMaxFloatValue());
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, false))
            {
                mValue.floatValue += mProperty->GetStepFloatValue() *
                                     (float)(int)(mPressedTime[control_LRight] + 1.0f);
                mValue.floatValue = Clamp(mValue.floatValue, mProperty->GetMinFloatValue(),
                                          mProperty->GetMaxFloatValue());
                bChanged = true;
            }
            if (bChanged)
            {
                mValue.floatValue = Clamp(mValue.floatValue, mProperty->GetMinFloatValue(),
                                          mProperty->GetMaxFloatValue());
            }
            break;

        case Property::type_Event:
            if (GetControlValue(pInputs, control_LLeft, true))
            {
                EventClass* pEvent;
                if (mValue.eventValue)
                {
                    pEvent = EventClass::FindByHashID(PblHash(mValue.eventValue));
                    if (pEvent)
                    {
                        pEvent = EventClass::GetPrevious(pEvent);
                    }
                }
                else
                {
                    pEvent = EventClass::GetLast();
                }
                mValue.eventValue = pEvent ? (char*)pEvent->GetName() : 0;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, true))
            {
                EventClass* pEvent;
                if (mValue.eventValue)
                {
                    pEvent = EventClass::FindByHashID(PblHash(mValue.eventValue));
                    if (pEvent)
                    {
                        pEvent = EventClass::GetNext(pEvent);
                    }
                }
                else
                {
                    pEvent = EventClass::GetFirst();
                }
                mValue.eventValue = pEvent ? (char*)pEvent->GetName() : 0;
                bChanged = true;
            }
            break;

        case Property::type_Color:
            if (GetControlValue(pInputs, control_LLeft, false) && mValue.colorValue[2] > 0)
            {
                mValue.colorValue[2]--;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, false) && mValue.colorValue[2] < 255)
            {
                mValue.colorValue[2]++;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LDown, false) && mValue.colorValue[1] > 0)
            {
                mValue.colorValue[1]--;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LUp, false) && mValue.colorValue[1] < 255)
            {
                mValue.colorValue[1]++;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_RLeft, false) && mValue.colorValue[0] > 0)
            {
                mValue.colorValue[0]--;
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_RRight, false) && mValue.colorValue[0] < 255)
            {
                mValue.colorValue[0]++;
                bChanged = true;
            }
            break;

        case Property::type_Vector2:
        case Property::type_Vector3:
        case Property::type_Vector4:
        {
            float* pVector = (float*)mValue.vector4Value;
            if (GetControlValue(pInputs, control_LLeft, false))
            {
                pVector[0] = Clamp(pVector[0] - mProperty->GetStepVectorValue(Property::axis_X) *
                                                    GetFloatStep(control_LLeft),
                                   mProperty->GetMinVectorValue(Property::axis_X),
                                   mProperty->GetMaxVectorValue(Property::axis_X));
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LRight, false))
            {
                pVector[0] = Clamp(mProperty->GetStepVectorValue(Property::axis_X) *
                                       GetFloatStep(control_LRight) + pVector[0],
                                   mProperty->GetMinVectorValue(Property::axis_X),
                                   mProperty->GetMaxVectorValue(Property::axis_X));
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LDown, false))
            {
                pVector[1] = Clamp(mProperty->GetStepVectorValue(Property::axis_Y) *
                                       GetFloatStep(control_LDown) + pVector[1],
                                   mProperty->GetMinVectorValue(Property::axis_Y),
                                   mProperty->GetMaxVectorValue(Property::axis_Y));
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_LUp, false))
            {
                pVector[1] = Clamp(pVector[1] - mProperty->GetStepVectorValue(Property::axis_Y) *
                                                    GetFloatStep(control_LUp),
                                   mProperty->GetMinVectorValue(Property::axis_Y),
                                   mProperty->GetMaxVectorValue(Property::axis_Y));
                bChanged = true;
            }
            if (mProperty->GetType() == Property::type_Vector2)
            {
                break;
            }

            if (GetControlValue(pInputs, control_RLeft, false))
            {
                pVector[2] = Clamp(pVector[2] - mProperty->GetStepVectorValue(Property::axis_Z) *
                                                    GetFloatStep(control_RLeft),
                                   mProperty->GetMinVectorValue(Property::axis_Z),
                                   mProperty->GetMaxVectorValue(Property::axis_Z));
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_RRight, false))
            {
                pVector[2] = Clamp(mProperty->GetStepVectorValue(Property::axis_Z) *
                                       GetFloatStep(control_RRight) + pVector[2],
                                   mProperty->GetMinVectorValue(Property::axis_Z),
                                   mProperty->GetMaxVectorValue(Property::axis_Z));
                bChanged = true;
            }
            if (mProperty->GetType() == Property::type_Vector3)
            {
                break;
            }

            if (GetControlValue(pInputs, control_RDown, false))
            {
                pVector[3] = Clamp(mProperty->GetStepVectorValue(Property::axis_W) *
                                       GetFloatStep(control_RDown) + pVector[3],
                                   mProperty->GetMinVectorValue(Property::axis_W),
                                   mProperty->GetMaxVectorValue(Property::axis_W));
                bChanged = true;
            }
            if (GetControlValue(pInputs, control_RUp, false))
            {
                pVector[3] = Clamp(pVector[3] - mProperty->GetStepVectorValue(Property::axis_W) *
                                                    GetFloatStep(control_RUp),
                                   mProperty->GetMinVectorValue(Property::axis_W),
                                   mProperty->GetMaxVectorValue(Property::axis_W));
                bChanged = true;
            }
            break;
        }

        case Property::type_Model:
            if (GetControlValue(pInputs, control_LLeft, true))
            {
                mValue.modelValue = (RedModel*)(mValue.modelValue ? RedModel::GetPrev(mValue.modelValue)
                                                                  : RedModel::GetLast());
                bChanged = mValue.modelValue != 0;
            }
            if (GetControlValue(pInputs, control_LRight, true))
            {
                mValue.modelValue = (RedModel*)(mValue.modelValue ? RedModel::GetNext(mValue.modelValue)
                                                                  : RedModel::GetFirst());
                bChanged = mValue.modelValue != 0;
            }
            break;

        case Property::type_Texture:
            if (GetControlValue(pInputs, control_LLeft, true))
            {
                mValue.textureValue =
                    (RedTexture*)(mValue.textureValue ? RedTexture::GetPrev(mValue.textureValue)
                                                      : RedTexture::GetLast());
                bChanged = mValue.textureValue != 0;
            }
            if (GetControlValue(pInputs, control_LRight, true))
            {
                mValue.textureValue =
                    (RedTexture*)(mValue.textureValue ? RedTexture::GetNext(mValue.textureValue)
                                                      : RedTexture::GetFirst());
                bChanged = mValue.textureValue != 0;
            }
            break;
        }

        if (bChanged)
        {
            mElement->SetProperty(mProperty->GetHashID(), &mValue);
            mElement->GetProperty(mProperty->GetHashID(), &mValue);
            DisplayProperty(true);
            UpdateElementDisplay();
        }

        if (GetControlValue(pInputs, control_L1, true))
        {
            DisplayProperty(false);
            SetMode(mode_SelectProperty);
        }
        if (GetControlValue(pInputs, control_L2, true))
        {
            mElementList = !mElementList;
            DisplayProperty(true);
        }
        break;
    }
    }

    if (GetControlValue(pInputs, control_Select, true))
    {
        Manager::Write();
    }

    if (GetControlValue(pInputs, control_R2, true))
    {
        if (mSafeZone.IsEnabled())
        {
            mScreenElement.RemoveElement(&mSafeZone);
            mSafeZone.Enable(false);
        }
        else
        {
            mScreenElement.AddElement(&mSafeZone);
            mSafeZone.Clear();

            const RedColor cSafeZone = { 0x00, 0xff, 0x00, 0xbf };
            mSafeZone.m_color = cSafeZone;

            float fLeft, fTop, fRight, fBottom;
            RedInterfaceScreen::s_screenFull.GetSafeArea(fLeft, fTop, fRight, fBottom);
            fRight -= fLeft;
            fLeft = 0.0f;
            fBottom -= fTop;
            fTop = 0.0f;
            fRight *= HUD::Element::GetViewportHeight() / RedRenderer::GetScreenAspectRatio() /
                      HUD::Element::GetViewportWidth();

            mSafeZone.Begin();
            mSafeZone.MovePen(fLeft, fTop);
            mSafeZone.DrawLine(fRight, fTop);
            mSafeZone.DrawLine(fRight, fBottom);
            mSafeZone.DrawLine(fLeft, fBottom);
            mSafeZone.DrawLine(fLeft, fTop);
            mSafeZone.End();
            mSafeZone.Enable(true);
        }
    }

    for (unsigned int i = 1; i < control_Max; i++)
    {
        if (GetControlValue(pInputs, (Control)i, false))
        {
            mPressedTime[i] += fDelta;
        }
        else
        {
            mPressedTime[i] = 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x00546090
float HUD::Editor::GetFloatStep(Control eControl)
{
    return Clamp(mPressedTime[eControl], 0.0f, 100.0f);
}

// FUNCTION: SWBF2 0x005460c0
void HUD::Editor::NextElement()
{
    bool bWrapped = false;
    do
    {
        mElement = Element::GetNext(mElement);
        if (!mElement)
        {
            if (bWrapped)
            {
                mElement = 0;
                return;
            }
            bWrapped = true;
            mElement = Element::GetFirst();
        }
    } while (!mElement->GetDisplayName());
}

// FUNCTION: SWBF2 0x00546100
void HUD::Editor::PrevElement()
{
    bool bWrapped = false;
    do
    {
        mElement = Element::GetPrevious(mElement);
        if (!mElement)
        {
            if (bWrapped)
            {
                mElement = 0;
                return;
            }
            bWrapped = true;
            mElement = Element::GetLast();
        }
    } while (!mElement->GetDisplayName());
}

// FUNCTION: SWBF2 0x00546140
unsigned int HUD::Editor::ElementSelectedIndex()
{
    unsigned int uIndex = 0;
    for (Element* pElement = Element::GetFirst(); pElement && pElement != mElement;
         pElement = Element::GetNext(pElement))
    {
        if (pElement->GetDisplayName())
        {
            uIndex++;
        }
    }
    return uIndex;
}

// FUNCTION: SWBF2 0x00546170
void HUD::Editor::UpdateElementDisplay()
{
    EditHeap heap;
    mBorder.Clear();
    mBorder.Enable(false);
}

// FUNCTION: SWBF2 0x00546200
void HUD::Editor::HighlightElement(bool bHighlight)
{
    if (!mElement)
    {
        return;
    }

    EditHeap heap;
    RedTextElement* pMenu = mMenu;
    for (int i = 0; i < 26; i++)
    {
        mMenu[i].SetText("", true);
        mMenu[i].Enable(false);
    }
    mInfo.SetText("", true);
    mInfo.Enable(false);
    UpdateElementDisplay();

    float fWidth = 0.0f;
    mBackdrop.SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    if (!bHighlight)
    {
        return;
    }

    if (mElementList)
    {
        unsigned int uSelected = ElementSelectedIndex();
        unsigned int uIndex = 0;
        unsigned int uFirst = uSelected < 26 ? 0 : uSelected - 25;
        unsigned int uCount = 0;
        for (Element* pElement = Element::GetFirst(); pElement; pElement = Element::GetNext(pElement))
        {
            if (pElement->GetDisplayName())
            {
                if (uIndex >= uFirst)
                {
                    RedTextElement* pText = pMenu++;
                    uCount++;
                    pText->SetText(pElement->GetDisplayName(), true);
                    pText->m_color.a = (mElement == pElement) ? 0xff : 0x7f;
                    pText->Enable(true);
                    if ((float)pText->m_iTextExtent > fWidth)
                    {
                        fWidth = (float)pText->m_iTextExtent;
                    }
                }
                uIndex++;
            }
            if (uCount == 26)
            {
                break;
            }
        }
        mBackdrop.SetRect(0.0f, 0.0f, fWidth, (float)uCount * 15.0f);
        for (unsigned int i = uCount; i < 26; i++)
        {
            mMenu[i].Enable(false);
        }
    }
    else
    {
        mInfo.SetPosition(GetScreenElementPosition());
        mInfo.SetText(mElement->GetDisplayName(), true);
        mInfo.Enable(true);
        mBackdrop.SetRect(mInfo.GetPosition().x, mInfo.GetPosition().y,
                          (float)mInfo.m_iTextExtent, 15.0f);
    }
}

// FUNCTION: SWBF2 0x005464e0
void HUD::Editor::NextProperty()
{
    mProperty = const_cast<Item::Factory::Property*>(
        mElement->GetFactory()->PropertyGetNext(mProperty));
    if (!mProperty)
    {
        mProperty = const_cast<Item::Factory::Property*>(
            mElement->GetFactory()->PropertyGetFirst());
    }
}

// FUNCTION: SWBF2 0x00546510
void HUD::Editor::PrevProperty()
{
    mProperty = const_cast<Item::Factory::Property*>(
        mElement->GetFactory()->PropertyGetPrevious(mProperty));
    if (!mProperty)
    {
        mProperty = const_cast<Item::Factory::Property*>(
            mElement->GetFactory()->PropertyGetLast());
    }
}

// FUNCTION: SWBF2 0x00546540
unsigned int HUD::Editor::PropertySelectedIndex()
{
    unsigned int uIndex = 0;
    if (mElement)
    {
        for (const Item::Factory::Property* pProperty = mElement->GetFactory()->PropertyGetFirst();
             pProperty && pProperty != mProperty;
             pProperty = mElement->GetFactory()->PropertyGetNext(pProperty))
        {
            uIndex++;
        }
    }
    return uIndex;
}

// FUNCTION: SWBF2 0x00546590
bool HUD::Editor::PropertyString(const HUD::Item::Factory::Property* const pProperty,
                                 char* const pBuffer)
{
    typedef Item::Factory::Property Property;

    Value value;
    bool bResult = mElement->GetProperty(pProperty->GetHashID(), &value);
    if (!bResult)
    {
        return bResult;
    }

    switch (pProperty->GetType())
    {
    case Property::type_Bool:
        sprintf(pBuffer, "%s (%s)", pProperty->GetName(), value.boolValue ? "true" : "false");
        break;
    case Property::type_Enum:
        sprintf(pBuffer, "%s (%s)", pProperty->GetName(),
                mElement->PropertyTranslateEnum(pProperty->GetHashID(), value.enumValue));
        break;
    case Property::type_Int:
        sprintf(pBuffer, "%s (%d)", pProperty->GetName(), value.intValue);
        break;
    case Property::type_Float:
        sprintf(pBuffer, "%s (%f)", pProperty->GetName(), value.floatValue);
        break;
    case Property::type_Event:
        if (value.eventValue)
        {
            sprintf(pBuffer, "%s (%s)", pProperty->GetName(), value.eventValue);
        }
        else
        {
            sprintf(pBuffer, "%s (no event)", pProperty->GetName());
        }
        break;
    case Property::type_Color:
        sprintf(pBuffer, "%s (%d, %d, %d)", pProperty->GetName(), value.colorValue[2],
                value.colorValue[1], value.colorValue[0]);
        break;
    case Property::type_Vector2:
    {
        const float* pVector = (const float*)value.vector2Value;
        sprintf(pBuffer, "%s (%.2f, %.2f)", pProperty->GetName(), pVector[0], pVector[1]);
        break;
    }
    case Property::type_Vector3:
    {
        const float* pVector = (const float*)value.vector3Value;
        sprintf(pBuffer, "%s (%.2f, %.2f, %.2f)", pProperty->GetName(), pVector[0], pVector[1],
                pVector[2]);
        break;
    }
    case Property::type_Vector4:
    {
        const float* pVector = (const float*)value.vector4Value;
        sprintf(pBuffer, "%s (%.2f, %.2f, %.2f, %.2f)", pProperty->GetName(), pVector[0],
                pVector[1], pVector[2], pVector[3]);
        break;
    }
    case Property::type_Model:
        if (value.modelValue)
        {
            sprintf(pBuffer, "%s (0x%08x)", pProperty->GetName(), value.modelValue->GetName());
        }
        else
        {
            sprintf(pBuffer, "%s (no model)", pProperty->GetName());
        }
        break;
    case Property::type_Texture:
        if (value.textureValue)
        {
            sprintf(pBuffer, "%s (0x%08x)", pProperty->GetName(), value.textureValue->m_uiNameHash);
        }
        else
        {
            sprintf(pBuffer, "%s (no texture)", pProperty->GetName());
        }
        break;
    }
    return bResult;
}

// FUNCTION: SWBF2 0x00546860
void HUD::Editor::DisplayProperties()
{
    mBackdrop.SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    if (!mProperty)
    {
        return;
    }

    unsigned int uSelected = PropertySelectedIndex();
    unsigned int uIndex = 0;
    unsigned int uCount = 0;
    unsigned int uFirst = uSelected < 26 ? 0 : uSelected - 25;

    EditHeap heap;
    char acBuffer[1024];
    RedTextElement* pMenu = mMenu;
    for (const Item::Factory::Property* pProperty = mElement->GetFactory()->PropertyGetFirst();
         pProperty; pProperty = mElement->GetFactory()->PropertyGetNext(pProperty))
    {
        if (uIndex >= uFirst && PropertyString(pProperty, acBuffer))
        {
            RedTextElement* pText = pMenu++;
            uCount++;
            pText->SetText(acBuffer, true);
            pText->m_color.a = (mProperty == pProperty) ? 0xff : 0x7f;
            pText->Enable(true);
        }
        uIndex++;
        if (uCount == 26)
        {
            break;
        }
    }
    mBackdrop.SetRect(0.0f, 0.0f, (float)RedInterfaceScreen::GetScreenWidth(),
                      (float)uCount * 15.0f);
    for (unsigned int i = uCount; i < 26; i++)
    {
        mMenu[i].Enable(false);
    }

    mElement->GetProperty(mProperty->GetHashID(), &mValue);
}

// FUNCTION: SWBF2 0x00546ab0
PblVector3 HUD::Editor::GetScreenElementPosition()
{
    PblVector3 vPos;
    if (mElementList)
    {
        vPos = PblVector3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        float fWidth, fHeight, fOffsetX, fOffsetY;
        RedInterfaceScreen::s_screenFull.GetSafeZone(fWidth, fHeight, fOffsetX, fOffsetY);
        fOffsetX = (float)RedInterfaceScreen::GetScreenWidth() * fOffsetX;
        fOffsetY = (float)RedInterfaceScreen::GetScreenHeight() * fOffsetY;
        PblMatrix mScreen = mElement->CalculateScreenMatrix();
        mInfo.SetMatrix(mScreen);
        vPos = *(const PblVector3*)mScreen.m[3];
        vPos.x -= fOffsetX;
        vPos.y -= fOffsetY;
    }
    mInfo.SetPosition(vPos);
    return vPos;
}

// FUNCTION: SWBF2 0x00546bc0
void HUD::Editor::DisplayProperty(bool bDisplay)
{
    EditHeap heap;
    mBackdrop.SetRect(0.0f, 0.0f, 0.0f, 0.0f);
    if (bDisplay)
    {
        char acBuffer[1024];
        if (PropertyString(mProperty, acBuffer))
        {
            mInfo.SetPosition(GetScreenElementPosition());
            mInfo.m_color.a = 0xff;
            mInfo.SetText(acBuffer, true);
            mInfo.Enable(true);
            mBackdrop.SetRect(mInfo.GetPosition().x, mInfo.GetPosition().y,
                              (float)mInfo.m_iTextExtent, 15.0f);
        }
    }
    else
    {
        mInfo.SetText("", true);
        mInfo.Enable(false);
    }
}

// FUNCTION: SWBF2 0x00546d30
bool HUD::Editor::GetControlValue(RawControllerInputs* const pInputs, Control eControl,
                                  bool bPressed)
{
    unsigned char ucKey = 0;
    switch (eControl)
    {
    case control_LLeft:  ucKey = DIK_LEFT; break;
    case control_LRight: ucKey = DIK_RIGHT; break;
    case control_LUp:    ucKey = DIK_UP; break;
    case control_LDown:  ucKey = DIK_DOWN; break;
    case control_RLeft:  ucKey = DIK_NUMPAD4; break;
    case control_RRight: ucKey = DIK_NUMPAD6; break;
    case control_RUp:    ucKey = DIK_NUMPAD8; break;
    case control_RDown:  ucKey = DIK_NUMPAD2; break;
    case control_L1:     ucKey = DIK_HOME; break;
    case control_L2:     ucKey = DIK_END; break;
    case control_R1:     ucKey = DIK_PRIOR; break;
    case control_R2:     ucKey = DIK_NEXT; break;
    case control_Start:  ucKey = DIK_DELETE; break;
    case control_Select: ucKey = DIK_INSERT; break;
    }

    bool bDown = GetInputsKeyboard(pInputs)->IsKeyDown(ucKey);
    if (bPressed)
    {
        if (bDown && !mKeyDown[eControl])
        {
            mKeyDown[eControl] = true;
            return true;
        }
        bDown = false;
    }
    mKeyDown[eControl] = bDown;
    return bDown;
}

// FUNCTION: SWBF2 0x00546e20
void HUD::Editor::KeyboardEvent(RedInputEventType eType, RedKeyboard* pKeyboard,
                                unsigned char ucA, unsigned char ucB, unsigned short usC)
{
    if (eType != KEYCHAR || !(usC & 1))
    {
        return;
    }

    if (ucA == DIK_E)
    {
        if (mMode == mode_Disabled)
        {
            SetMode(mode_SelectElement);
        }
        else
        {
            SetMode(mode_Disabled);
        }
    }
    else if (ucA == DIK_B)
    {
        mBackdrop.Enable(!mBackdrop.IsEnabled());
    }
}

// FUNCTION: SWBF2 0x00546e80
void HUD::Editor::SetMode(Mode eMode)
{
    gEditMode = eMode;
    bool bEnable = false;
    Mode eOldMode = mMode;
    mMode = eMode;

    switch (eMode)
    {
    case mode_SelectElement:
        if (!mElement)
        {
            mElement = Element::GetFirst();
            if (!mElement || !mElement->GetDisplayName())
            {
                mMode = mode_Disabled;
            }
        }
        if (mElement)
        {
            bEnable = true;
            HighlightElement(true);
        }
        break;
    case mode_SelectProperty:
        if (eOldMode != mode_EditProperty)
        {
            mProperty = const_cast<Item::Factory::Property*>(
                mElement->GetFactory()->PropertyGetFirst());
        }
        DisplayProperties();
        bEnable = true;
        break;
    case mode_EditProperty:
        bEnable = true;
        break;
    }

    if (bEnable)
    {
        PlayerController::sEnabled = 0;
        GameLoop::Pause();
    }
    else
    {
        PlayerController::sEnabled = -1;
        GameLoop::Resume();
    }

    Enable(bEnable);

    if (mMode != eOldMode)
    {
        EditHeap heap;
        if (mMode != mode_SelectProperty && mMode != mode_SelectElement)
        {
            for (int i = 0; i < 26; i++)
            {
                mMenu[i].Enable(false);
                mMenu[i].SetText("", true);
            }
        }
        mInfo.Enable(mMode != mode_Disabled);
    }
}
