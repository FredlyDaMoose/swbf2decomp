#include <string.h>
#include "FLJoystick.h"
#include "FLInputManager.h"
#include "GameSoundEngine.h"
#include "PauseMenu.h"
#include "LuaHelper.h"
#include "Trigger.h"
#include "GameLoop.h"
#include "HUDElementMap.h"
#include "HUDGameEvents.h"
#include "HUDEvent.h"
#include "NetGame.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "SoundFL/source/sndbus.h"

namespace PopupText { extern void DisablePopup(); }
namespace PopupText { void __fastcall ShowPopup(int iType, unsigned int uTextId); }
namespace HUD { namespace GameEvents { extern void DisableAllForObjectiveDisplay(int iViewport, bool bInit); } }

// FUNCTION: SWBF2 0x00527ea0
bool GUIInputs::IsLeftPressed() const
{
    return mProcessedGUIInputs[eGUIINPUT_Left] >= 0.125f ||
           mProcessedGUIInputs[eGUIINPUT_xAxis] <= -0.5f;
}

// FUNCTION: SWBF2 0x00527ed0
bool GUIInputs::IsRightPressed() const
{
    return mProcessedGUIInputs[eGUIINPUT_Right] >= 0.125f ||
           mProcessedGUIInputs[eGUIINPUT_xAxis] >= 0.5f;
}

namespace PopupText
{

enum PopupType { DISABLED = 0, HINT = 1, OBJECTIVE = 2, SELECTION = 3, MAX_POPUP_TYPES = 4 };

// GLOBAL: SWBF2 0x01eaedfb
bool gMapEnabled[1];
// GLOBAL: SWBF2 0x01eaedfa
bool gSpawnDisplayEnabled[1];
// GLOBAL: SWBF2 0x01eaee20
int gNumHints;
// GLOBAL: SWBF2 0x01eaee24
int gCurHint;
// GLOBAL: SWBF2 0x01eaee29
bool gDirectionPressedLastUpdate;

// GLOBAL: SWBF2 0x01eaee1f

bool gAcceptReleased;

// GLOBAL: SWBF2 0x01eaee28

bool gPopupShownLatch;
// GLOBAL: SWBF2 0x01eaee2c
unsigned int gLastObjectiveTextID;
// GLOBAL: SWBF2 0x01eaee30
PopupType gType;
// GLOBAL: SWBF2 0x01eaee38
unsigned int gHints[64];

// GLOBAL: SWBF2 0x01eaee2a
bool gPreventJump;
// GLOBAL: SWBF2 0x01ef7fcc
Trigger gButtonTrigger;

void Init();
bool IsEnabled();
bool GetPreventJump();
void SetPreventJump(bool bPrevent);
void Update(float fDeltaTime);
void ShowPopup(PopupType eType, unsigned int uTextId);
}

// FUNCTION: SWBF2 0x00620af0
void __fastcall PopupText::ShowPopup(int iType, unsigned int uTextId)
{
    using namespace PopupText;
    if (GameLoop::mNumCameras != 1 || (iType == HINT && gHints[gCurHint] == 0))
    {
        return;
    }

    if (IsEnabled())
    {
        PopupText::DisablePopup();
    }
    else
    {
        Snd::Bus* pBus = Snd::Bus::FindByHashID(0xB38617C5);
        if (pBus)
        {
            pBus->Fade(0.5f, 0.0f, -1.0f);
        }
        HUD::ElementMap* pMap = HUD::ElementMap::FindByPlayerIndex(0);
        gMapEnabled[0] = pMap && static_cast<RedInterfaceElement*>(static_cast<RedGroupElement*>(pMap))->m_bEnabled;
        HUD::GameEvents::DisableAllForObjectiveDisplay(0, true);
        SpawnDisplay* pSpawn = SpawnDisplay::Instance(0);
        if (pSpawn->mVisible && pSpawn->mMode == SpawnDisplay::eCHANGECLASS)
        {
            gSpawnDisplayEnabled[0] = true;
            pSpawn->ChangeClass(NetGame::GetLocalPlayer(0));
        }
        else
        {
            gSpawnDisplayEnabled[0] = false;
        }
    }

    if (!NetGame::IsEnabled())
    {
        GameLoop::Pause();
    }
    gType = (PopupType)iType;
    gButtonTrigger.Clear();

    if (iType == HINT)
    {
        HUD::Event(HUD::GameEvents::gObjectiveEvents.hintPopup, (int)gHints[gCurHint]).Send();
        unsigned short page[16];
        PblUnicode::usnprintf(page, 16, "%d/%d", gCurHint + 1, gNumHints);
        HUD::Event(HUD::GameEvents::gObjectiveEvents.hintPopupPageNumber,
                   (const wchar_t*)page).Send();
    }
    else if (iType == OBJECTIVE && uTextId)
    {
        gLastObjectiveTextID = uTextId;
        HUD::Event(HUD::GameEvents::gObjectiveEvents.objectivePopup, (int)uTextId).Send();
    }
    else if (iType == SELECTION && uTextId)
    {
        gLastObjectiveTextID = 0;
        HUD::Event(HUD::GameEvents::gObjectiveEvents.selectionPopup, (int)uTextId).Send();
    }

    HUD::Event(HUD::GameEvents::gPlayerEvents[0].pressSelectToReturnDisable, true).Send();
    HUD::Event(HUD::GameEvents::gPlayerEvents[0].objectiveDetailsDisable, true).Send();
    HUD::Event(HUD::GameEvents::gPlayerEvents[0].hintsAvailableDisable, true).Send();
    gPreventJump = true;
    gPopupShownLatch = true;
    gAcceptReleased = false;
}

void PopupText::ShowPopup(PopupType eType, unsigned int uTextId)
{
    PopupText::ShowPopup(eType, uTextId);
}

// FUNCTION: SWBF2 0x006210d0
void PopupText::SetPreventJump(bool bPrevent)
{
    gPreventJump = bPrevent;
}

namespace PopupText
{

// FUNCTION: SWBF2 0x00620f00
void Update(float fDeltaTime)
{
    GameSoundEngine::Update(fDeltaTime);
    if (PauseMenu::GetPaused(PauseMenu::GetPausingViewport()) || FLInputManager::s_instance.AnyMissing())
    {
        return;
    }

    ProcessedInputs* pInput = FLInputManager::s_instance.GetPlayerInputs(0);
    GUIInputs* pGui = FLInputManager::s_instance.GetPlayerGuiInputs(0);
    const float fAccept = pGui->mProcessedGUIInputs[eGUIINPUT_Accept];
    if (fAccept < 0.125f)
    {
        gAcceptReleased = true;
    }

    if (gType == SELECTION)
    {
        if (pInput)
        {
            gButtonTrigger.Update(fDeltaTime, pInput->IsAcceptHeroPressed());
            if (gButtonTrigger.IsPressed())
            {
                LuaHelper::CallGlobalProc("SetFlyerControllerConfig", 1.0f);
                DisablePopup();
                return;
            }
            gButtonTrigger.Update(fDeltaTime, pInput->IsDeclineHeroPressed());
            if (gButtonTrigger.IsPressed())
            {
                LuaHelper::CallGlobalProc("SetFlyerControllerConfig", 2.0f);
                DisablePopup();
            }
        }
        return;
    }
    if (gType == OBJECTIVE)
    {
        if (gAcceptReleased && fAccept >= 0.125f)
        {
            DisablePopup();
        }
        return;
    }
    if (gType != HINT)
    {
        return;
    }

    if (gAcceptReleased && fAccept >= 0.125f)
    {
        DisablePopup();
    }
    else if (pGui->IsLeftPressed() && !gDirectionPressedLastUpdate)
    {
        if (--gCurHint < 0)
        {
            gCurHint = gNumHints - 1;
        }
        ShowPopup(HINT, 0);
    }
    else if (pGui->IsRightPressed() && !gDirectionPressedLastUpdate)
    {
        if (++gCurHint >= gNumHints)
        {
            gCurHint = 0;
        }
        ShowPopup(HINT, 0);
    }
    gDirectionPressedLastUpdate = pGui->IsRightPressed() || pGui->IsLeftPressed();
}
}

// FUNCTION: SWBF2 0x00620a90
void PopupText::Init()
{
    gType = DISABLED;
    memset(gHints, 0, sizeof(gHints));
    gCurHint = 0;
    gNumHints = 0;
    gLastObjectiveTextID = 0;
    gDirectionPressedLastUpdate = false;
    gMapEnabled[0] = false;
    gSpawnDisplayEnabled[0] = false;
}

// FUNCTION: SWBF2 0x00620ef0
bool PopupText::IsEnabled()
{
    return gType != DISABLED;
}

// FUNCTION: SWBF2 0x006210e0
bool PopupText::GetPreventJump()
{
    return gPreventJump;
}

namespace PopupText { int GetNumHints(); }

// FUNCTION: SWBF2 0x00620ee0
int PopupText::GetNumHints()
{
    return PopupText::gNumHints;
}
