#include "HUDObjectiveList.h"

#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "Character.h"
#include "NetGame.h"

namespace ObjectiveManager { void AddObjectiveList(HUD::ObjectiveList* pList); }

// FUNCTION: SWBF2 0x00565e90
HUD::ObjectiveList::Factory::Factory()
    : HUD::ElementGroupBase::Factory(0x6FA0D380u)
{
    mName = "ObjectiveList";
}

// FUNCTION: SWBF2 0x00565d40
HUD::Item* HUD::ObjectiveList::Factory::CreateItem(const char* const pName, ItemCallback pCallback,
                                                    void* const pUserData)
{
    HUD::Item* pItem = new ObjectiveList(pName, pCallback, pUserData);
    if (pItem)
    {
        return pItem->SetFactory(this);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00565dc0
HUD::Objective::Objective()
{
    _team = 0;
    _state = INACTIVE;
    _textHashID = 0;
    _popupTextID = 0;
    _textElement.SetFont(RedFont::GAMEFONT_SMALL);
    _textElement.Enable(false);
}

// FUNCTION: SWBF2 0x00565e40
HUD::Objective::~Objective()
{
}

// FUNCTION: SWBF2 0x00565e50
void HUD::Objective::SetState(ObjectiveState state)
{
    _state = state;
}

// FUNCTION: SWBF2 0x00565e60
void HUD::Objective::SetText(unsigned int uiTextHashID)
{
    _textElement.SetText(RedLocalizeDB::Find(uiTextHashID, false), true);
    _textHashID = uiTextHashID;
}

// FUNCTION: SWBF2 0x00565ee0
HUD::ObjectiveList::ObjectiveList(const char* const pName, Factory::ItemCallback pCallback,
                                  void* const pUserData)
    : HUD::ElementGroup(pName, pCallback, pUserData)
{
    _viewportId = 0;
    _team = -1;
    _bLastButtonState = false;
    _lastWidth = 0.0f;
    _lastHeight = 0.0f;
    switch ((unsigned int)PblHash(pName))
    {
    case 0xebe40f77u:
        _viewportId = 0;
        break;
    case 0xa19881deu:
        _viewportId = 1;
        break;
    case 0x127bd03du:
        _viewportId = 2;
        break;
    case 0xf03a3654u:
        _viewportId = 3;
        break;
    }
    ObjectiveManager::AddObjectiveList(this);
    HUD::Element::Enable(false);
}

// SYNTHETIC: SWBF2 0x00566040
// HUD::ObjectiveList::`scalar deleting destructor'

// SYNTHETIC: SWBF2 0x00566740
// [thunk]:HUD::ObjectiveList::`scalar deleting destructor' adjustor{368}
// FUNCTION: SWBF2 0x00566070
HUD::ObjectiveList::~ObjectiveList()
{
    RemoveAllObjectives();
}

// FUNCTION: SWBF2 0x005661b0
void HUD::ObjectiveList::RemoveAllObjectives()
{
    while (_objectives.GetCount() > 0)
    {
        PblList<Objective>::Node* pNode = _objectives.GetFirst();
        delete pNode->_pObject;
        _objectives.Remove(pNode);
    }
}

// FUNCTION: SWBF2 0x00566100
void HUD::ObjectiveList::Update(float fDeltaTime)
{
    if (!GetOwner())
    {
        return;
    }
    bool bChanged = false;
    if (_team != GetOwner()->mTeamNumber)
    {
        bChanged = true;
        _team = GetOwner()->mTeamNumber;
    }
    if (_lastHeight != mHeight || _lastWidth != mWidth)
    {
        _lastHeight = mHeight;
        _lastWidth = mWidth;
        bChanged = true;
    }
    if (bChanged)
    {
        SetupText();
    }
    HUD::Element::Update(fDeltaTime);
}

// FUNCTION: SWBF2 0x00566210
void HUD::ObjectiveList::AddObjective(unsigned int uiTextHashID, unsigned int uiPopupTextID, int iTeam)
{
    if (FindObjective(uiTextHashID))
    {
        return;
    }

    Objective* pObjective = new Objective;
    PblList<Objective>::Node* pNode = new PblList<Objective>::Node;
    pObjective->SetState(Objective::NORMAL);
    pObjective->SetText(uiTextHashID);
    pObjective->_popupTextID = uiPopupTextID;
    pObjective->_team = iTeam;
    if (_objectives.GetCount() == 0)
    {
        _objectives.Add(pNode, pObjective);
    }
    else
    {
        _objectives.AddAfter(_objectives.GetLast(), pNode, pObjective);
    }
    RedGroupElement::AddElement(&pObjective->_textElement);
    SetupText();
    if (NetGame::IsEnabled() && !NetGame::IsClient())
    {
        NetGame::CreateEvent_AddObjective(pObjective);
    }
}

// FUNCTION: SWBF2 0x00566480
void HUD::ObjectiveList::SetupText()
{
    float fY = 8.0f;
    float fUsedHeight = 0.0f;
    float fWidth = GetWidthPixelsFloat() - 16.0f;
    if (fWidth < 0.0f)
    {
        fWidth = 0.0f;
    }
    float fHeight = GetHeightPixelsFloat() - 16.0f;
    if (fHeight < 0.0f)
    {
        fHeight = 0.0f;
    }
    bool bFull = false;
    int iNumShown = 0;
    float fLeft, fTop, fRight, fBottom;

    for (PblList<Objective>::Itor it(_objectives.GetLast()); it != _objectives.End(); --it)
    {
        RedTextElement& text = it->_textElement;
        if (!GetOwner() || it->_team != GetOwner()->mTeamNumber)
        {
            text.Enable(false);
            continue;
        }
        switch (it->_state)
        {
        case Objective::ACTIVE:
            text.SetColor(RedColor::YELLOW);
            break;
        case Objective::COMPLETED:
            text.SetColor(RedColor::MDGREY);
            break;
        default:
            text.Enable(false);
            continue;
        }
        text.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_TOP);
        text.SetTextBox((unsigned int)fWidth, 1000);
        text.GetRect(fLeft, fTop, fRight, fBottom);
        float fNewHeight = fBottom - fTop + 2.0f + fUsedHeight;
        if (fHeight >= fNewHeight && !bFull)
        {
            fUsedHeight = fNewHeight;
            text.Enable(true);
            ++iNumShown;
        }
        else
        {
            bFull = true;
            if (iNumShown == 0)
            {
                text.Enable(true);
                iNumShown = 1;
            }
            else
            {
                text.Enable(false);
            }
        }
    }

    for (PblList<Objective>::Itor it(_objectives); it != _objectives.End(); ++it)
    {
        RedTextElement& text = it->_textElement;
        if (text.IsEnabled())
        {
            text.SetPosition(8.0f, fY);
            text.GetRect(fLeft, fTop, fRight, fBottom);
            fY += fBottom - fTop + 2.0f;
        }
    }
}

// FUNCTION: SWBF2 0x005666e0
Character* HUD::ObjectiveList::GetOwner()
{
    if (Character::sCharacters)
    {
        return NetGame::GetLocalPlayer(_viewportId);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00566700
void HUD::ObjectiveList::BeginIter()
{
    _itor = PblList<Objective>::Itor(_objectives);
}

// FUNCTION: SWBF2 0x00566710
HUD::Objective* HUD::ObjectiveList::GetCurrentIter()
{
    if ((Objective*)_itor != (Objective*)_objectives.End())
    {
        return _itor;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00566730
void HUD::ObjectiveList::IncrementIter()
{
    ++_itor;
}
