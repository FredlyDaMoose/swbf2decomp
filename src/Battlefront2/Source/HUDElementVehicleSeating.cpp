#include <stdio.h>

#include "HUDElementVehicleSeating.h"
#include "CameraManager.h"
#include "Character.h"
#include "Controllable.h"
#include "GameObject.h"
#include "NetGame.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

static const unsigned int kModel3DHash = 0x1eccdefd;
static const unsigned int kTextHash = 0xbde64e3e;

// GLOBAL: SWBF2 0x00789bd0
const char* const HUD::ElementVehicleSeating::sMarkerNames[4] = { "Empty", "Self", "Player", "AI" };

// GLOBAL: SWBF2 0x00789b80
const char* const HUD::ElementVehicleSeating::sMarkerTextNames[4] = { "EmptyText", "SelfText", "PlayerText",
                                                                      "AIText" };

// GLOBAL: SWBF2 0x00789b90
const unsigned int HUD::ElementVehicleSeating::sHPNames[16] = {
    0x963caf2a, 0x9b7f89f3, 0x9fbe9444, 0x81f9c441, 0x8538d9f6, 0x887bff2f, 0x8cbae298, 0xb4f55f25,
    0xb0344292, 0x66f171f7, 0x62306c40, 0x6f734a99, 0x6bb2572e, 0x75f5072b, 0x71341a9c, 0x7c773c45 };

// FUNCTION: SWBF2 0x0055ca40
HUD::ElementVehicleSeating::Factory::Factory()
    : HUD::ElementGroupPlayer::Factory(0x6A89B43Au)
{
    mName = "VehicleSeating";
}

// SYNTHETIC: SWBF2 0x0055d8e6
// [thunk]:HUD::ElementVehicleSeating::SeatingMarker::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x0055c920
HUD::ElementVehicleSeating::SeatingMarker::~SeatingMarker()
{
}

// FUNCTION: SWBF2 0x0055c940
bool HUD::ElementVehicleSeating::SeatingMarker::SetProperty(const unsigned int uHashID, const void* pValue)
{
    bool bResult = ElementModel3D::SetProperty(uHashID, pValue);
    if (bResult && mSeatingDisplay)
    {
        mSeatingDisplay->Clear();
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0055c970
bool HUD::ElementVehicleSeating::SeatingText::SetProperty(const unsigned int uHashID, const void* pValue)
{
    bool bResult = ElementText::SetProperty(uHashID, pValue);
    if (bResult && mSeatingDisplay)
    {
        mSeatingDisplay->Clear();
    }
    return bResult;
}

// FUNCTION: SWBF2 0x0055c9a0
void* HUD::ElementVehicleSeating::GetParent()
{
    return this;
}

// SYNTHETIC: SWBF2 0x0055c9b0
// HUD::ElementVehicleSeating::SeatingMarker::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055c9f0
// HUD::ElementVehicleSeating::SeatingText::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055d8f1
// [thunk]:HUD::ElementVehicleSeating::SeatingText::`scalar deleting destructor' adjustor{256}
// FUNCTION: SWBF2 0x0055ca20
HUD::ElementVehicleSeating::SeatingText::~SeatingText()
{
}

// SYNTHETIC: SWBF2 0x0055cdb0
// HUD::ElementVehicleSeating::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x0055d8db
// [thunk]:HUD::ElementVehicleSeating::`scalar deleting destructor' adjustor{384}
// FUNCTION: SWBF2 0x0055ce20
HUD::ElementVehicleSeating::~ElementVehicleSeating()
{
    if (mNode.mNext)
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x0055c8a0
HUD::Item* HUD::ElementVehicleSeating::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                           void* const pUserData)
{
    HUD::Item* pItem = new ElementVehicleSeating(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0055ca90
HUD::ElementVehicleSeating::ElementVehicleSeating(const char* const pName, Factory::ItemCallback pCallback,
                                                  void* const pUserData)
    : HUD::ElementGroupPlayer(*static_cast<RedGroupElement*>(this), pName, pCallback, pUserData)
{
    mChanged = false;

    const HUD::Item::Factory* pMarkerFactory = HUD::Item::Factory::FindByHashID(kModel3DHash);
    const HUD::Item::Factory* pTextFactory = HUD::Item::Factory::FindByHashID(kTextHash);

    unsigned int i;
    for (i = 0; i < 4; ++i)
    {
        mMarkerTemplates[i].HUD::Item::SetName(sMarkerNames[i]);
        mMarkerTemplates[i].SetDynamicallyAllocated(false);
        mMarkerTemplates[i].SetWriteEnabled(false);
        mMarkerTemplates[i].mSeatingDisplay = this;
        mMarkerTemplates[i].SetFactory(pMarkerFactory);

        mMarkerTextTemplate[i].HUD::Item::SetName(sMarkerTextNames[i]);
        mMarkerTextTemplate[i].SetDynamicallyAllocated(false);
        mMarkerTextTemplate[i].SetWriteEnabled(false);
        mMarkerTextTemplate[i].mSeatingDisplay = this;
        mMarkerTextTemplate[i].SetFactory(pTextFactory);
    }

    for (i = 0; i < 10; ++i)
    {
        mSeatingMarkers[i].SetDynamicallyAllocated(false);
        mSeatingMarkers[i].SetWriteEnabled(false);
        mSeatingMarkers[i].SetFactory(pMarkerFactory);
        RedGroupElement::AddElement(&mSeatingMarkers[i]);

        mSeatingText[i].SetDynamicallyAllocated(false);
        mSeatingText[i].SetWriteEnabled(false);
        mSeatingText[i].SetFactory(pTextFactory);
        RedGroupElement::AddElement(&mSeatingText[i]);

        mSeatingState[i] = seatingState_None;
    }

    mBackdrop.HUD::Item::SetName("Backdrop");
    mBackdrop.SetDynamicallyAllocated(false);
    mBackdrop.SetWriteEnabled(false);
    mBackdrop.SetFactory(pMarkerFactory);
    RedGroupElement::AddElement(&mBackdrop);

    sSeatingDisplays.AddLast(&mNode);
}

// FUNCTION: SWBF2 0x0055d4b0
void HUD::ElementVehicleSeating::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                      void* const pUserData)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (config.m_pChunk->PeekNextChildId() == 0x504f4353)
        {
            unsigned int i;
            for (i = 0; i < 4; ++i)
            {
                if (mMarkerTemplates[i].GetNameHash() == data.GetId())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mMarkerTemplates[i].Read(scope, 0, 0);
                    break;
                }
                if (mMarkerTextTemplate[i].GetNameHash() == data.GetId())
                {
                    PblConfig scope(config.ReadNextScope(), true);
                    mMarkerTextTemplate[i].Read(scope, 0, 0);
                    break;
                }
            }
            if (i < 4)
            {
                continue;
            }

            if (mBackdrop.GetNameHash() == data.GetId())
            {
                PblConfig scope(config.ReadNextScope(), true);
                mBackdrop.Read(scope, 0, 0);
            }
            else
            {
                ReadChild(config, data, pCallback, pUserData);
            }
        }
        else
        {
            ReadData(config, data);
        }
    }

    const PblVector3& vPos = RedGroupElement::GetPosition();
    float fX = vPos.x;
    if (CameraManager::Instance()->GetNumCameras() <= 1)
    {
        fX = ConvertWideScreenX(fX, sViewportWidth, sViewportHeight);
    }
    RedGroupElement::SetPosition(fX, vPos.y, vPos.z);

    PostReadSetup();
}

// FUNCTION: SWBF2 0x0055d6b0
void HUD::ElementVehicleSeating::PostReadSetup()
{
    char buffer[256];
    for (unsigned int i = 0; i < 4; ++i)
    {
        _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mMarkerTemplates[i].GetName());
        mMarkerTemplates[i].SetDisplayName(buffer);
        _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mMarkerTextTemplate[i].GetName());
        mMarkerTextTemplate[i].SetDisplayName(buffer);
    }
    _snprintf(buffer, sizeof(buffer), "%s.%s", GetName(), mBackdrop.GetName());
    mBackdrop.SetDisplayName(buffer);
}

// FUNCTION: SWBF2 0x0055d7c0
bool HUD::ElementVehicleSeating::ReadData(PblConfig& config, PblConfig::Data& data)
{
    return HUD::ElementGroupPlayer::ReadData(config, data);
}

// FUNCTION: SWBF2 0x0055cf50
void HUD::ElementVehicleSeating::UpdateSeatingState(unsigned int uSeat, SeatingState eState,
                                                    const unsigned short* const pText)
{
    if (mSeatingState[uSeat] == eState)
    {
        return;
    }

    const bool bEnable = eState != seatingState_None;
    PblVector3 vOffset(0.0f, 0.0f, 0.0f);
    float fTextOffset = 0.0f;
    SeatingMarker& rMarker = mSeatingMarkers[uSeat];
    SeatingText& rText = mSeatingText[uSeat];

    if (bEnable)
    {
        rMarker.HUD::Element::Enable(false);
        rMarker = mMarkerTemplates[eState - 1];
        rMarker.RedInterfaceElement::SetMatrix(mMarkerTemplates[eState - 1].m_mLocal);
        rText.HUD::Element::Enable(false);
        rText.HUD::ElementText::operator=(mMarkerTextTemplate[eState - 1]);
        rText.RedInterfaceElement::SetMatrix(mMarkerTextTemplate[eState - 1].m_mLocal);

        const RedModel* pModel = mBackdrop.GetModel();
        if (pModel)
        {
            PblMatrix mHardpoint;
            if (pModel->GetBoneMatrix(sHPNames[uSeat], mHardpoint))
            {
                const float fInvDepth =
                    1.0f / (mBackdrop.GetDepth() * Red3DModelElementLite::GetDepthMultiplier(true));
                const PblVector3 vPos = mHardpoint.GetTranslation();
                vOffset.x = vPos.x * fInvDepth;
                vOffset.y = -(vPos.y * fInvDepth);
                vOffset.z = vPos.z;
            }
        }
        else
        {
            for (unsigned int i = uSeat; i > 0; --i)
            {
                const RedFont* pFont = mSeatingText[i - 1].GetFont();
                vOffset.y -= pFont ? (float)pFont->GetHeight() : 0.0f;
            }
        }

        const RedFont* pFont = rText.GetFont();
        fTextOffset = vOffset.y - (pFont ? (float)pFont->GetHeight() * 0.5f : 0.0f);
    }

    const PblVector3& vMarkerPos = rMarker.RedInterfaceElement::GetPosition();
    rMarker.RedInterfaceElement::SetPosition(
        PblVector3(vMarkerPos.x + vOffset.x, vMarkerPos.y + vOffset.y, vMarkerPos.z + vOffset.z));
    rMarker.RedInterfaceElement::Scale(0.4f, 0.4f);
    rMarker.HUD::Element::Enable(bEnable);

    const PblVector3& vTextPos = rText.RedInterfaceElement::GetPosition();
    rText.RedInterfaceElement::SetPosition(
        PblVector3(vTextPos.x + 0.0f, vTextPos.y + fTextOffset, vTextPos.z + 0.0f));
    rText.HUD::Element::Enable(pText && bEnable);
    rText.RedTextElement::SetText(pText, true);

    mSeatingState[uSeat] = eState;
    mChanged = true;
}

// FUNCTION: SWBF2 0x0055d290
unsigned int HUD::ElementVehicleSeating::UpdateSeatingMarkers(const Character* const pCharacter,
                                                              const GameObject* const pObject)
{
    unsigned int uNumSeats = 0;
    unsigned int uSeat = 0;
    if (pObject)
    {
        for (PblList<Controllable>::Itor itor(pObject->GetControllableList()); (Controllable*)itor; ++itor)
        {
            if (uSeat >= 10)
            {
                return uNumSeats;
            }

            Controllable* pSeat = itor;
            const Controllable* pPilot = pSeat->GetPilot();
            const ControllableClass* pClass = pSeat->GetControllableClass();

            SeatingState eState;
            const Character* pPilotCharacter;
            if (pPilot)
            {
                pPilotCharacter = pPilot->mCharacter;
                if (pPilot->GetPlayerId() >= 0)
                {
                    eState = pPilotCharacter != pCharacter ? seatingState_Player : seatingState_Self;
                }
                else
                {
                    eState = seatingState_AI;
                }
            }
            else
            {
                eState = seatingState_Empty;
                pPilotCharacter = 0;
            }

            const unsigned short* pText;
            if (!pClass)
            {
                pText = 0;
            }
            else if (eState != seatingState_Empty && pPilotCharacter)
            {
                pText = pPilotCharacter->GetName();
            }
            else
            {
                pText = RedLocalizeDB::Find(pClass->mLocalNameID, true);
            }

            UpdateSeatingState(uSeat, eState, pText);
            ++uSeat;
            ++uNumSeats;
        }
        if (uSeat >= 10)
        {
            return uNumSeats;
        }
    }

    for (; uSeat < 10; ++uSeat)
    {
        UpdateSeatingState(uSeat, seatingState_None, 0);
    }
    return uNumSeats;
}

// FUNCTION: SWBF2 0x0055d3c0
void HUD::ElementVehicleSeating::Update(const float fElapsed)
{
    if (mPlayerIndex >= 0)
    {
        unsigned int uNumSeats = 0;
        Character* pCharacter = NetGame::GetLocalPlayer(mPlayerIndex);
        if (pCharacter)
        {
            Controllable* pControllable = pCharacter->GetControllable();
            const Controllable* pPilot = pControllable ? pControllable->GetPilot() : 0;
            const GameObject* pPilotObject = pPilot ? pPilot->GetGameObject() : 0;
            const Controllable* pConstControllable = pControllable;
            const GameObject* pVehicle = pConstControllable ? pConstControllable->GetGameObject() : 0;
            uNumSeats = UpdateSeatingMarkers(pCharacter, pPilotObject != pVehicle ? pVehicle : 0);
            if (!uNumSeats)
            {
                mBackdrop.SetModel(0u);
            }
        }
        mBackdrop.HUD::Element::Enable(uNumSeats != 0);
    }
    HUD::Element::Update(fElapsed);
}

// FUNCTION: SWBF2 0x0055d8b0
HUD::ElementVehicleSeating* HUD::ElementVehicleSeating::FindByPlayerIndex(int iPlayerIndex)
{
    PblListSingle::Node* pHead = &sSeatingDisplays.mHead;
    for (PblListSingle::Node* pNode = pHead->mNext; pNode != pHead; pNode = pNode->mNext)
    {
        ElementVehicleSeating* pDisplay = reinterpret_cast<ElementVehicleSeating*>(
            reinterpret_cast<char*>(pNode) - offsetof(ElementVehicleSeating, mNode));
        if (pDisplay->mPlayerIndex == iPlayerIndex)
        {
            return pDisplay;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0055d260
void HUD::ElementVehicleSeating::Clear()
{
    for (unsigned int i = 0; i < 10; ++i)
    {
        UpdateSeatingState(i, seatingState_None, 0);
    }
}

// FUNCTION: SWBF2 0x0055d7a0
bool HUD::ElementVehicleSeating::SetProperty(const unsigned int uHashID, const void* pValue)
{
    return HUD::ElementGroupPlayer::SetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0055d7b0
bool HUD::ElementVehicleSeating::GetProperty(const unsigned int uHashID, void* pValue)
{
    return HUD::ElementGroupPlayer::GetProperty(uHashID, pValue);
}

// FUNCTION: SWBF2 0x0055d7d0
void HUD::ElementVehicleSeating::WriteData(HUD::File& file, unsigned int uTabs)
{
    float fX = RedGroupElement::GetPosition().x;
    if (CameraManager::sInstance->GetNumCameras() <= 1)
    {
        fX = HUD::ConvertXForWideScreen(fX, GetContainerViewWidth(), GetContainerViewHeight());
    }
    const PblVector3& vPosition = RedGroupElement::GetPosition();
    RedGroupElement::SetPosition(fX, vPosition.y, vPosition.z);
    HUD::ElementGroupPlayer::WriteData(file, uTabs);
    mBackdrop.Write(file, uTabs, mBackdrop.GetName());
    for (unsigned int i = 0; i < 4; ++i)
    {
        mMarkerTemplates[i].Write(file, uTabs, mMarkerTemplates[i].GetName());
        mMarkerTextTemplate[i].Write(file, uTabs, mMarkerTextTemplate[i].GetName());
    }
}

// GLOBAL: SWBF2 0x007eba58
// SYNTHETIC: SWBF2 0x00403fe0
// `dynamic initializer for 'HUD::ElementVehicleSeating::sSeatingDisplays''
// SYNTHETIC: SWBF2 0x00768b70
// `dynamic atexit destructor for 'HUD::ElementVehicleSeating::sSeatingDisplays''
PblListSingle HUD::ElementVehicleSeating::sSeatingDisplays;
