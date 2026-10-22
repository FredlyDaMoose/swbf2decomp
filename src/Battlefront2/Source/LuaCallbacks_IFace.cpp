#include "LuaCallbacks_IFace.h"
#include "LuaHelper.h"

#include <string.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Interface/RedHotSpot.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Interface/RedMaskedBitmapElement.h"
#include "RedEngineFL/movie/RedMovie.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "FLInputManager.h"
#include "FLJoystick.h"
#include "FlashyTextElement.h"
#include "FLGuiManager.h"
#include "GameMemory.h"
#include "GameMovie.h"
#include "LoadDisplay.h"
#include "LoadUtil.h"
#include "NetSetup.h"
#include "NetMatch.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblFile.h"
#include "GameState.h"
#include "GameLoop.h"
#include "IFaceSoldierAnimator.h"
#include "PlayerProfile.h"
#include "Battlefront2/Platform/Pc/pcFLJoystick.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/Interface/Red3DModelAnimator.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedBorderElement.h"
#include "RedEngineFL/Interface/RedGroupElement.h"
#include "RedEngineFL/Interface/RedIFScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "RedEngineFL/Util/RedCommandConsole.h"
#include "RedEngineFL/Util/RedConsole.h"
#include "RedEngineFL/Graphics/RedAmbientLight.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "SoundFL/source/sndqueue.h"
#include "Entity.h"
#include "EntityGeometry.h"
#include "GameModel.h"
#include "GameObject.h"
#include "GameSound.h"
#include "GameSoundEngine.h"
#include "PauseMenu.h"
#include "Weapon.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#include "Steam/gfGalaxy.h"

extern int g_InGameMoviePlayerState;
extern Snd::Queue<unsigned int, 12> g_InGameMovieQueue;

// FUNCTION: SWBF2 0x005807b0
void Red3DModelElement::QueueAnimation(unsigned int uAnimHash)
{
    EnsureAnimatorExists();
    Red3DModelAnimator* pAnimator = mAnimator;
    if (pAnimator->mAnimHash)
    {
        pAnimator->mQueuedAnim = uAnimHash;
    }
    else
    {
        pAnimator->Close();
        pAnimator->mAnimHash = uAnimHash;
        pAnimator->mQueuedAnim = 0;
        pAnimator->mAnimTransitionTimeTotal = 0.0f;
        pAnimator->mAnimTransitionTime = 0.0f;
        pAnimator->Open();
    }
}

// FUNCTION: SWBF2 0x00580800
void IFaceSoldierAnimator::QueueUpperLowerAnimations(unsigned int uUpperAnimHash, unsigned int uLowerAnimHash)
{
    if (mAnimHash)
    {
        mQueuedAnim = uUpperAnimHash;
        mQueuedLowerAnim = uLowerAnimHash;
    }
    else
    {
        Close();
        mAnimHash = uUpperAnimHash;
        mLowerAnimHash = uLowerAnimHash;
        mQueuedAnim = 0;
        mQueuedLowerAnim = 0;
        mAnimTransitionTimeTotal = 0.0f;
        mAnimTransitionTime = 0.0f;
        Open();
    }
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationX(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationY(PblMatrix* pOut, float fAngle);
extern "C" PblMatrix* __stdcall D3DXMatrixRotationZ(PblMatrix* pOut, float fAngle);

namespace PblUnicode { unsigned int ustrncnv(unsigned short* pDest, const unsigned char* pSrc, unsigned int uiCount); }
namespace StatsManager { void Stop(); }
extern int netCurMaxPlayers;
extern bool netIsDedicated;

static const float DEG_TO_RAD = 0.017453292f;

namespace StatsManager { extern int m_state; }

namespace StatsManager {

// FUNCTION: SWBF2 0x005307b0
int GetState()
{
    return StatsManager::m_state;
}
}

// FUNCTION: SWBF2 0x005307c0
void StatsManagerSetStoppedState()
{
    StatsManager::m_state = 2;
}

namespace StatsManager {

// FUNCTION: SWBF2 0x005308c0
bool __fastcall ReadStatsGetResults(int iUnused, void* pData)
{
    memset(pData, 0, 0x460);
    int* pWords = static_cast<int*>(pData);
    pWords[131] = 10;
    pWords[132] = 10;
    return true;
}
}

namespace StatsManager {

// FUNCTION: SWBF2 0x005308f0
bool FindServicePlayerID()
{
    return false;
}
}

namespace StatsManager {

// FUNCTION: SWBF2 0x00530900
int GetLeaderBoardId()
{
    return 1;
}
}

namespace StatsManager {

// FUNCTION: SWBF2 0x00530910
bool ResetPlayer()
{
    return false;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00587ea0
void __fastcall TableReadValWithDefault(const char* pcKey, float* pfValue)
{
    lua_pushstring(LuaHelper::mState, pcKey);
    lua_rawget(LuaHelper::mState, -2);
    if (lua_type(LuaHelper::mState, -1) != LUA_TNIL)
    {
        *pfValue = (float)luaL_checknumber(LuaHelper::mState, -1);
    }
    lua_settop(LuaHelper::mState, -2);
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00587f00
void __fastcall TableReadColor(const char* pcKey, float* pfColor)
{
    lua_pushstring(LuaHelper::mState, pcKey);
    lua_rawget(LuaHelper::mState, -2);
    Lua_Callbacks::TableReadValWithDefault("red", pfColor);
    Lua_Callbacks::TableReadValWithDefault("green", pfColor + 1);
    Lua_Callbacks::TableReadValWithDefault("blue", pfColor + 2);
    lua_settop(LuaHelper::mState, -2);
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00587f60
void __fastcall TableReadDirection(int iUnused, float* pfDirection)
{
    lua_pushstring(LuaHelper::mState, "direction");
    lua_rawget(LuaHelper::mState, -2);
    Lua_Callbacks::TableReadValWithDefault("x", pfDirection);
    Lua_Callbacks::TableReadValWithDefault("y", pfDirection + 1);
    Lua_Callbacks::TableReadValWithDefault("z", pfDirection + 2);
    lua_settop(LuaHelper::mState, -2);
}
}

extern char netGameName[64];

// FUNCTION: SWBF2 0x005e7180
const char* NetSetup::GetHostName()
{
    return netGameName;
}

// GLOBAL: SWBF2 0x01fa88e0
unsigned __int64 netLocalGSID;

// FUNCTION: SWBF2 0x005d65b0
unsigned __int64 NetMatch::GetLocalGSID()
{
    return netLocalGSID;
}

// FUNCTION: SWBF2 0x00414990
int __fastcall RawControllerInputs::IndexToRedKey(int iIndex)
{
    return RedKeyTo_BindID[iIndex];
}

// FUNCTION: SWBF2 0x00585a10
static void fnErrorBoxOk(bool)
{
    return;
}

// FUNCTION: SWBF2 0x00617030
bool ToggleInvincibilityCheat()
{
    if (netInShell ? netEnabled : netEnabledNext)
    {
        return GameLoop::gInvincible;
    }
    GameLoop::gInvincible = !GameLoop::gInvincible;
    return GameLoop::gInvincible;
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x00586a00
bool GetServicePlayerID(const char* pcName, unsigned __int64* puId)
{
    if (!NetSetup::IsLoginSuccessful(0, false))
    {
        return false;
    }
    if (!pcName)
    {
        *puId = NetMatch::GetLocalGSID();
        return true;
    }
    for (int i = 0; i < Lua_Callbacks::GetLobbyPlayerCount(); ++i)
    {
        if (!strcmp(pcName, Lua_Callbacks::GetLobbyPlayer(i)))
        {
            *puId = NetGame::GetPlayerGSID(i);
            return true;
        }
    }
    for (unsigned int i = 0; i < gf::friends::friendCount(); ++i)
    {
        if (!strcmp(pcName, gf::friends::friendNameByIndex(i)))
        {
            *puId = gf::friends::friendByIndex(i).ToUint64();
            return true;
        }
    }
    return false;
}
}

namespace GameLoop { extern bool gUnlimitedAmmo; }
extern bool gLowResModelsEnabled;

// FUNCTION: SWBF2 0x00617070
bool ToggleUnlimitedAmmoCheat()
{
    if (netInShell ? netEnabled : netEnabledNext)
    {
        return GameLoop::gUnlimitedAmmo;
    }
    GameLoop::gUnlimitedAmmo = !GameLoop::gUnlimitedAmmo;
    return GameLoop::gUnlimitedAmmo;
}

// FUNCTION: SWBF2 0x006170b0
bool ToggleLowResModelsCheat()
{
    if (netInShell ? netEnabled : netEnabledNext)
    {
        return gLowResModelsEnabled;
    }
    gLowResModelsEnabled = !gLowResModelsEnabled;
    return gLowResModelsEnabled;
}

bool ToggleAltMeleeHitEffectsCheat();

bool ToggleSoundEnabled();
bool ToggleSoundPitch();

// GLOBAL: SWBF2 0x007e9000

CheatToggleT gCheatToggles[6] =
{
    { ToggleInvincibilityCheat, 0xb9982cffu },
    { ToggleUnlimitedAmmoCheat, 0x18d38f73u },
    { ToggleLowResModelsCheat, 0x2ab3aa8cu },
    { ToggleAltMeleeHitEffectsCheat, 0x6afb1548u },
    { ToggleSoundEnabled, 0xb7584045u },
    { ToggleSoundPitch, 0x84bffe04u },
};

// FUNCTION: SWBF2 0x00616f20
bool EvaluateCheatCode(const char* pcName, char* pcResult)
{
    unsigned int uHash = PblHash(pcName);
    for (unsigned int i = 0; i < 6; ++i)
    {
        if (gCheatToggles[i].uNameHash != uHash || !gCheatToggles[i].pfnToggle)
        {
            continue;
        }

        GameSound sound;
        sound.SetID(0x6a942216u, false, false);
        sound.Play(0, 0, false, true);
        bool bOn = gCheatToggles[i].pfnToggle();
        if (!pcResult)
        {
            continue;
        }

        switch (i)
        {
        case 0: _snprintf(pcResult, 49, "cheats.invincible"); break;
        case 1: _snprintf(pcResult, 49, "cheats.ammo"); break;
        case 2: _snprintf(pcResult, 49, "cheats.lowres"); break;
        case 3: _snprintf(pcResult, 49, "cheats.pow"); break;
        case 4: _snprintf(pcResult, 49, "cheats.nohud"); break;
        case 5: _snprintf(pcResult, 49, "cheats.slowmo"); break;
        default: _snprintf(pcResult, 49, ""); break;
        }
        strncat(pcResult, bOn ? "_on" : "_off", 49);
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x006f7610
void RedBorderElement::GetBorderTexCoords(float& fU0, float& fV0, float& fU1, float& fV1, float& fBorderU,
                                  float& fBorderV)
{
    fU0 = m_vertices[0].u;
    fV0 = m_vertices[0].v;
    fU1 = m_vertices[15].u;
    fV1 = m_vertices[15].v;
    fBorderU = m_vertices[5].u - fU0;
    fBorderV = m_vertices[5].v - fV0;
}

// FUNCTION: SWBF2 0x006f7db0
RedMaskedBitmapElement::RedMaskedBitmapElement(const char* pcName)
    : RedBitmapElement(pcName)
{
    m_pInterfaceMaskedBitmapShader = 0;
    SetMaskTexCoords(0.0f, 1.0f, 0.0f, 1.0f);
}

void Red3DModelElement::SetAnimationBanks(const unsigned int* puBanks)
{
    EnsureAnimatorExists();
    mAnimator->SetAnimationBanks(puBanks);
}

void Red3DModelElement::SetAnimationTime(float fTime)
{
    EnsureAnimatorExists();
    mAnimator->SetAnimationTime(fTime);
}

void RedInterfaceScreen::GetDirectionalLight(int iIndex, PblVector3& vDirection, RedColorValue& color)
{
    vDirection = s_directionalLight[iIndex].m_vDirection;
    color = s_directionalLight[iIndex].m_color;
}

// FUNCTION: SWBF2 0x006eca70
RedCommandConsole::Command* RedCommandConsole::Group::FindCommand(unsigned int uiID)
{
    for (PblList<Command>::Node* pNode = mCommandList._head._pNext; pNode->_pObject; pNode = pNode->_pNext)
    {
        if (pNode->_pObject->mID == uiID)
        {
            return pNode->_pObject;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006ecaa0
RedCommandConsole::Command* RedCommandConsole::Group::FindCommand(char const* pcName)
{
    return FindCommand(PblHash(pcName));
}

// FUNCTION: SWBF2 0x006ecb30
void RedCommandConsole::Command::Run(RedConsole* pConsole, char const* pcArgs)
{
    mVar.Run(pConsole, mName, mID, pcArgs);
}

// FUNCTION: SWBF2 0x006ecac0
int RedCommandConsole::Process(RedConsole* pConsole, char const* pcLine)
{
    Command* pCommand = _FindCommand(pcLine);
    if (!pCommand)
    {
        return 0;
    }

    const char* pcArgs = pcLine;
    while (*pcArgs && !isspace(*pcArgs))
    {
        ++pcArgs;
    }
    while (isspace(*pcArgs))
    {
        ++pcArgs;
    }
    pCommand->Run(pConsole, pcArgs);
    return 1;
}

// FUNCTION: SWBF2 0x006ed150
RedCommandConsole::Command* RedCommandConsole::_FindCommand(char const* pcPath)
{
    if (!pcPath || !*pcPath)
    {
        return 0;
    }
    Group* pGroup = _FindGroup(pcPath, true);
    if (!pGroup)
    {
        return 0;
    }

    const char* pcLastDot = 0;
    const char* pcEnd = pcPath;
    while (*pcEnd && !isspace(*pcEnd))
    {
        if (*pcEnd == '.')
        {
            pcLastDot = pcEnd;
        }
        ++pcEnd;
    }
    const char* pcName = pcLastDot ? pcLastDot + 1 : pcPath;
    if (!*pcName || isspace(*pcName))
    {
        return 0;
    }
    int iLen = pcEnd - pcName;
    if (iLen > 31)
    {
        return 0;
    }

    char acName[32];
    if (*pcEnd)
    {
        strlcpy(acName, pcName, iLen + 1 < 32 ? iLen + 1 : 32);
        pcName = acName;
    }
    return pGroup->FindCommand(pcName);
}

// FUNCTION: SWBF2 0x006ed240
RedCommandConsole::Group* RedCommandConsole::_FindGroup(char const* pcPath, bool bLeafIsCommand)
{
    if (!sRootNode)
    {
        return 0;
    }
    if (!*pcPath || isspace(*pcPath))
    {
        return bLeafIsCommand ? 0 : sRootNode;
    }

    Group* pGroup = sRootNode;
    char acName[32];
    char* pcOut = acName;
    while (*pcPath == '.')
    {
        ++pcPath;
    }
    while (*pcPath && !isspace(*pcPath))
    {
        if (*pcPath == '.')
        {
            *pcOut = 0;
            pGroup = pGroup->mChildList.Find(acName);
            if (!pGroup)
            {
                return 0;
            }
            pcOut = acName;
            acName[0] = 0;
            do
                ++pcPath;
            while (*pcPath == '.');
            continue;
        }
        if (pcOut == &acName[31])
        {
            return 0;
        }
        *pcOut++ = *pcPath++;
    }
    if (!bLeafIsCommand && acName[0])
    {
        *pcOut = 0;
        pGroup = pGroup->mChildList.Find(acName);
        if (!pGroup)
        {
            return 0;
        }
    }
    return pGroup;
}

// FUNCTION: SWBF2 0x00583b60
static Red3DModelAnimator* GetNormalAnimator(Red3DModelElement* pModel)
{
    Red3DModelAnimator* pAnimator = pModel->GetAnimator();
    if (pAnimator && !pAnimator->IsRtti(IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator))
    {
        return pAnimator;
    }

    Red3DModelAnimator* pNormal = new Red3DModelAnimator;
    pNormal->SetAnimationBanks(pAnimator->mAnimBankHash);
    pModel->SetAnimator(pNormal);
    delete pAnimator;
    return pNormal;
}

// FUNCTION: SWBF2 0x00583c40
static IFaceSoldierAnimator* GetIFaceSoldierAnimator(Red3DModelElement* pModel)
{
    Red3DModelAnimator* pAnimator = pModel->GetAnimator();
    if (pAnimator && pAnimator->IsRtti(IFaceSoldierAnimator::rttiHashIFaceSoldierAnimator))
    {
        return static_cast<IFaceSoldierAnimator*>(pAnimator);
    }

    IFaceSoldierAnimator* pSoldier = new IFaceSoldierAnimator(pAnimator);
    pModel->SetAnimator(pSoldier);
    delete pAnimator;
    return pSoldier;
}

// GLOBAL: SWBF2 0x01faaab0

static unsigned int sAnimationBankHashes[5];

// FUNCTION: SWBF2 0x00527fa0
void LuaHelper::SetGlobal(char const* pcName)
{
    lua_setglobal(mState, pcName);
}

// FUNCTION: SWBF2 0x0048c9e0
void RedInterfaceElement::CreateHotSpot(float fLeft, float fTop, float fRight, float fBottom)
{
    if (!m_pHotSpot)
    {
        m_pHotSpot = new RedHotSpot;
    }
    m_pHotSpot->SetRect(fLeft, fTop, fRight, fBottom);
}

namespace Lua_Callbacks
{

// GLOBAL: SWBF2 0x007e6cc0
static const CallbackEntryT sIFaceCallbacks[150] =
{
    { "ScriptCB_AddIFText", ScriptCB_AddIFText },
    { "ScriptCB_AddIFFlashyText", ScriptCB_AddIFFlashyText },
    { "ScriptCB_AddIFImage", ScriptCB_AddIFImage },
    { "ScriptCB_AddIFMaskImage", ScriptCB_AddIFMaskImage },
    { "ScriptCB_AddIFBorder", ScriptCB_AddIFBorder },
    { "ScriptCB_AddIFModel", ScriptCB_AddIFModel },
    { "ScriptCB_AddIFContainer", ScriptCB_AddIFContainer },
    { "ScriptCB_EndIFObj", ScriptCB_EndIFObj },
    { "ScriptCB_AddIFScreen", ScriptCB_AddIFScreen },
    { "ScriptCB_SetIFScreenViewport", ScriptCB_SetIFScreenViewport },
    { "ScriptCB_EndIFScreen", ScriptCB_EndIFScreen },
    { "ScriptCB_IsPAL", ScriptCB_IsPAL },
    { "ScriptCB_IsFileExist", ScriptCB_IsFileExist },
    { "ScriptCB_IFObj_GetCPointer", ScriptCB_IFObj_GetCPointer },
    { "ScriptCB_IFObj_SetPos", ScriptCB_IFObj_SetPos },
    { "ScriptCB_IFObj_SetAlpha", ScriptCB_IFObj_SetAlpha },
    { "ScriptCB_IFObj_SetColor", ScriptCB_IFObj_SetColor },
    { "ScriptCB_IFObj_SetZOrder", ScriptCB_IFObj_SetZOrder },
    { "ScriptCB_IFObj_SetVis", ScriptCB_IFObj_SetVis },
    { "ScriptCB_IFObj_SetEnabled", ScriptCB_IFObj_SetEnabled },
    { "ScriptCB_IFObj_SetWidthHeight", ScriptCB_IFObj_SetWidthHeight },
    { "ScriptCB_IFObj_SetPosition", ScriptCB_IFObj_SetPosition },
    { "ScriptCB_IFObj_SetBasicRotation", ScriptCB_IFObj_SetBasicRotation },
    { "ScriptCB_IFObj_SetRelativeRotation", ScriptCB_IFObj_SetRelativeRotation },
    { "ScriptCB_IFObj_SetRotation", ScriptCB_IFObj_SetRotation },
    { "ScriptCB_IFObj_GetPos", ScriptCB_IFObj_GetPos },
    { "ScriptCB_IFObj_GetAlpha", ScriptCB_IFObj_GetAlpha },
    { "ScriptCB_IFObj_GetColor", ScriptCB_IFObj_GetColor },
    { "ScriptCB_IFObj_GetZOrder", ScriptCB_IFObj_GetZOrder },
    { "ScriptCB_IFObj_GetVis", ScriptCB_IFObj_GetVis },
    { "ScriptCB_IFObj_GetWidthHeight", ScriptCB_IFObj_GetWidthHeight },
    { "ScriptCB_IFObj_GetPosition", ScriptCB_IFObj_GetPosition },
    { "ScriptCB_IFObj_BringToFront", ScriptCB_IFObj_BringToFront },
    { "ScriptCB_IFObj_SendToBack", ScriptCB_IFObj_SendToBack },
    { "ScriptCB_IFObj_Enable", ScriptCB_IFObj_Enable },
    { "ScriptCB_IFObj_IsEnabled", ScriptCB_IFObj_IsEnabled },
    { "ScriptCB_IFObj_IsGroup", ScriptCB_IFObj_IsGroup },
    { "ScriptCB_IFObj_MouseOver", ScriptCB_IFObj_MouseOver },
    { "ScriptCB_IFObj_TestHotSpot", ScriptCB_IFObj_TestHotSpot },
    { "ScriptCB_IFObj_CreateHotSpot", ScriptCB_IFObj_CreateHotSpot },
    { "ScriptCB_IFObj_RemoveHotSpot", ScriptCB_IFObj_RemoveHotSpot },
    { "ScriptCB_GetFontHeight", ScriptCB_GetFontHeight },
    { "ScriptCB_IFText_SetString", ScriptCB_IFText_SetString },
    { "ScriptCB_IFText_SetUString", ScriptCB_IFText_SetUString },
    { "ScriptCB_IFText_SetTextPosition", ScriptCB_IFText_SetTextPosition },
    { "ScriptCB_IFText_SetFont", ScriptCB_IFText_SetFont },
    { "ScriptCB_IFText_SetIntercharacterSpacing", ScriptCB_IFText_SetIntercharacterSpacing },
    { "ScriptCB_IFText_GetIntercharacterSpacing", ScriptCB_IFText_GetIntercharacterSpacing },
    { "ScriptCB_IFText_SetTextBox", ScriptCB_IFText_SetTextBox },
    { "ScriptCB_IFText_GetTextBox", ScriptCB_IFText_GetTextBox },
    { "ScriptCB_IFText_SetJustify", ScriptCB_IFText_SetJustify },
    { "ScriptCB_IFText_SetTextBreak", ScriptCB_IFText_SetTextBreak },
    { "ScriptCB_IFText_SetTextScale", ScriptCB_IFText_SetTextScale },
    { "ScriptCB_IFText_GetTextScaleX", ScriptCB_IFText_GetTextScaleX },
    { "ScriptCB_IFText_GetTextScaleY", ScriptCB_IFText_GetTextScaleY },
    { "ScriptCB_IFText_GetTextExtent", ScriptCB_IFText_GetTextExtent },
    { "ScriptCB_IFText_GetDisplayRect", ScriptCB_IFText_GetDisplayRect },
    { "ScriptCB_IFText_SetTextStyle", ScriptCB_IFText_SetTextStyle },
    { "ScriptCB_IFText_SetLeading", ScriptCB_IFText_SetLeading },
    { "ScriptCB_IFText_GetLeading", ScriptCB_IFText_GetLeading },
    { "ScriptCB_IFFlashyText_Setup", ScriptCB_IFFlashyText_Setup },
    { "ScriptCB_IFFlashyText_SetTextColor", ScriptCB_IFFlashyText_SetTextColor },
    { "ScriptCB_IFFlashyText_SetBackground", ScriptCB_IFFlashyText_SetBackground },
    { "ScriptCB_IFFlashyText_SetBackgroundSize", ScriptCB_IFFlashyText_SetBackgroundSize },
    { "ScriptCB_IFFlashyText_SetBackgroundRightJustify", ScriptCB_IFFlashyText_SetBackgroundRightJustify },
    { "ScriptCB_IFFlashyText_StartFlashiness", ScriptCB_IFFlashyText_StartFlashiness },
    { "ScriptCB_IFImage_SetTexture", ScriptCB_IFImage_SetTexture },
    { "ScriptCB_IFImage_GetRect", ScriptCB_IFImage_GetRect },
    { "ScriptCB_IFImage_SetRect", ScriptCB_IFImage_SetRect },
    { "ScriptCB_IFImage_GetTexCoords", ScriptCB_IFImage_GetTexCoords },
    { "ScriptCB_IFImage_SetTexCoords", ScriptCB_IFImage_SetTexCoords },
    { "ScriptCB_IFMaskImage_SetMaskTexture", ScriptCB_IFMaskImage_SetMaskTexture },
    { "ScriptCB_IFMaskImage_SetMaskTexCoords", ScriptCB_IFMaskImage_SetMaskTexCoords },
    { "ScriptCB_IFMaskImage_GetMaskTexCoords", ScriptCB_IFMaskImage_GetMaskTexCoords },
    { "ScriptCB_IFBorder_SetTexture", ScriptCB_IFBorder_SetTexture },
    { "ScriptCB_IFBorder_GetRect", ScriptCB_IFBorder_GetRect },
    { "ScriptCB_IFBorder_SetRect", ScriptCB_IFBorder_SetRect },
    { "ScriptCB_IFBorder_GetTexCoords", ScriptCB_IFBorder_GetTexCoords },
    { "ScriptCB_IFBorder_SetTexCoords", ScriptCB_IFBorder_SetTexCoords },
    { "ScriptCB_IFModel_SetModel", ScriptCB_IFModel_SetModel },
    { "ScriptCB_IFModel_SetAnimation", ScriptCB_IFModel_SetAnimation },
    { "ScriptCB_IFModel_QueueAnimation", ScriptCB_IFModel_QueueAnimation },
    { "ScriptCB_IFModel_SetAnimationBanks", ScriptCB_IFModel_SetAnimationBanks },
    { "ScriptCB_IFModel_SetAnimationTime", ScriptCB_IFModel_SetAnimationTime },
    { "ScriptCB_IFModel_AttachModel", ScriptCB_IFModel_AttachModel },
    { "ScriptCB_IFModel_SetScale", ScriptCB_IFModel_SetScale },
    { "ScriptCB_IFModel_GetScale", ScriptCB_IFModel_GetScale },
    { "ScriptCB_IFModel_SetRotation", ScriptCB_IFModel_SetRotation },
    { "ScriptCB_IFModel_GetRotation", ScriptCB_IFModel_GetRotation },
    { "ScriptCB_IFModel_SetTranslation", ScriptCB_IFModel_SetTranslation },
    { "ScriptCB_IFModel_GetTranslation", ScriptCB_IFModel_GetTranslation },
    { "ScriptCB_IFModel_SetDepth", ScriptCB_IFModel_SetDepth },
    { "ScriptCB_IFModel_GetDepth", ScriptCB_IFModel_GetDepth },
    { "ScriptCB_IFModel_SetOmegaY", ScriptCB_IFModel_SetOmegaY },
    { "ScriptCB_IFModel_UpdateAnimation", ScriptCB_IFModel_UpdateAnimation },
    { "ScriptCB_IFModel_SetAnimationLooping", ScriptCB_IFModel_SetAnimationLooping },
    { "ScriptCB_IFObj_SetScreenPosition", ScriptCB_IFObj_SetScreenPosition },
    { "ScriptCB_IFObj_SetUseSafezone", ScriptCB_IFObj_SetUseSafezone },
    { "ScriptCB_SetIFScreen", ScriptCB_SetIFScreen },
    { "ScriptCB_PushScreen", ScriptCB_PushScreen },
    { "ScriptCB_PopScreen", ScriptCB_PopScreen },
    { "ScriptCB_IsScreenInStack", ScriptCB_IsScreenInStack },
    { "ScriptCB_OpenPopup", ScriptCB_OpenPopup },
    { "ScriptCB_ClosePopup", ScriptCB_ClosePopup },
    { "ScriptCB_IsErrorBoxOpen", ScriptCB_IsErrorBoxOpen },
    { "ScriptCB_IsPopupOpen", ScriptCB_IsPopupOpen },
    { "ScriptCB_SetIgnoreInputs", ScriptCB_SetIgnoreInputs },
    { "ScriptCB_OpenMovie", ScriptCB_OpenMovie },
    { "ScriptCB_CloseMovie", ScriptCB_CloseMovie },
    { "ScriptCB_PlayMovie", ScriptCB_PlayMovie },
    { "ScriptCB_IsMoviePlaying", ScriptCB_IsMoviePlaying },
    { "ScriptCB_AreMoviePropertiesPlaying", ScriptCB_AreMoviePropertiesPlaying },
    { "ScriptCB_StopMovie", ScriptCB_StopMovie },
    { "ScriptCB_SetMovieAudioBus", ScriptCB_SetMovieAudioBus },
    { "ScriptCB_IsPropertyPlayingNow", ScriptCB_IsPropertyPlayingNow },
    { "ScriptCB_PlayInGameMovie", ScriptCB_PlayInGameMovie },
    { "ScriptCB_StopInGameMovie", ScriptCB_StopInGameMovie },
    { "ScriptCB_OpenErrorBox", ScriptCB_OpenErrorBox },
    { "ScriptCB_CloseErrorBox", ScriptCB_CloseErrorBox },
    { "ScriptCB_GetErrorLevel", ScriptCB_GetErrorLevel },
    { "ScriptCB_GetScreenInfo", ScriptCB_GetScreenInfo },
    { "ScriptCB_GetSafeScreenInfo", ScriptCB_GetSafeScreenInfo },
    { "ScriptCB_GetLeaderBoardPlayerData", ScriptCB_GetLeaderBoardPlayerData },
    { "ScriptCB_LeaderBoardDownloadDone", ScriptCB_LeaderBoardDownloadDone },
    { "ScriptCB_DownloadLeaderBoardData", ScriptCB_DownloadLeaderBoardData },
    { "ScriptCB_CancelLeaderBoardPlayerData", ScriptCB_CancelLeaderBoardPlayerData },
    { "ScriptCB_ResetLeaderBoardPlayerData", ScriptCB_ResetLeaderBoardPlayerData },
    { "ScriptCB_SetLeaderBoardID", ScriptCB_SetLeaderBoardID },
    { "ScriptCB_EnumerateLeaderBoard", ScriptCB_EnumerateLeaderBoard },
    { "ScriptCB_EnumerateLeaderBoardDone", ScriptCB_EnumerateLeaderBoardDone },
    { "ScriptCB_GetEnumerateLeaderBoardData", ScriptCB_GetEnumerateLeaderBoardData },
    { "ScriptCB_IsEnableLeaderBoard", ScriptCB_IsEnableLeaderBoard },
    { "ScriptCB_SetSunlight", ScriptCB_SetSunlight },
    { "ScriptCB_IFModel_SetLighting", ScriptCB_IFModel_SetLighting },
    { "ScriptCB_GetConsoleCmds", ScriptCB_GetConsoleCmds },
    { "ScriptCB_DoConsoleCmd", ScriptCB_DoConsoleCmd },
    { "ScriptCB_GetKeyBoardCmds", ScriptCB_GetKeyBoardCmds },
    { "ScriptCB_SetBinding", ScriptCB_SetBinding },
    { "ScriptCB_GetActionFromIdx", ScriptCB_GetActionFromIdx },
    { "ScriptCB_ResetControl", ScriptCB_ResetControl },
    { "ScriptCB_EnableCursor", ScriptCB_EnableCursor },
    { "ScriptCB_MrMrsEval", ScriptCB_MrMrsEval },
    { "ScriptCB_GetEntityMesh", ScriptCB_GetEntityMesh },
    { "ScriptCB_GetEntityWeaponMesh", ScriptCB_GetEntityWeaponMesh },
    { "ScriptCB_GetEntityLabel", ScriptCB_GetEntityLabel },
    { "ScriptCB_GetIFaceLighting", ScriptCB_GetIFaceLighting },
    { "ScriptCB_SetIFaceLighting", ScriptCB_SetIFaceLighting },
    { "ScriptCB_RemoveTexture", ScriptCB_RemoveTexture },
    { "ScriptCB_ShowLoadDisplay", ScriptCB_ShowLoadDisplay },
    { "ScriptCB_SetInputRepeat", ScriptCB_SetInputRepeat },
};

// GLOBAL: SWBF2 0x01e57714
RedInterfaceElement* pCurCreateObject;
// GLOBAL: SWBF2 0x01e57718
RedScreenGroupElement* pCurCreateScreenGroup;

extern unsigned short sTempReturnString16[256];

// FUNCTION: SWBF2 0x005808b0
bool Util_AddToHierarchy(RedInterfaceElement* pElement, const char* pcName)
{
    pCurCreateObject = pElement;
    pCurCreateScreenGroup = 0;
    const char* pcDot = strrchr(pcName, '.');
    if (!pcDot)
    {
        return false;
    }

    char acParent[260];
    unsigned int uiLen = pcDot - pcName;
    if (uiLen > 256)
    {
        uiLen = 256;
    }
    memcpy(acParent, pcName, uiLen);
    acParent[uiLen] = 0;

    IFScreen* pScreen = IFScreen::Find(acParent);
    if (pScreen)
    {
        RedScreenGroupElement* pGroup = new RedScreenGroupElement(pcName);
        pGroup->SetUseSafezone(true);
        pCurCreateScreenGroup = pGroup;
        pGroup->AddElement(pCurCreateObject);
        pScreen->AddToScreen(pCurCreateScreenGroup);
    }
    else
    {
        RedInterfaceElement* pParent = RedInterfaceElement::Find(acParent);
        if (!pParent || !pParent->IsGroup())
        {
            return false;
        }
        ((RedGroupElement*)pParent)->AddElement(pCurCreateObject);
    }
    pCurCreateObject->AddToSearchList(pcName);
    return true;
}

// FUNCTION: SWBF2 0x00580a00
bool Util_CreateGroupInHierarchy(const char* pcName)
{
    pCurCreateObject = 0;
    pCurCreateScreenGroup = 0;
    const char* pcDot = strrchr(pcName, '.');
    if (!pcDot)
    {
        RedScreenGroupElement* pGroup = new RedScreenGroupElement(pcName);
        pGroup->SetUseSafezone(true);
        pCurCreateScreenGroup = pGroup;
        pCurCreateObject = pGroup;
    }
    else
    {
        char acParent[260];
        unsigned int uiLen = pcDot - pcName;
        if (uiLen > 256)
        {
            uiLen = 256;
        }
        memcpy(acParent, pcName, uiLen);
        acParent[uiLen] = 0;

        IFScreen* pScreen = IFScreen::Find(acParent);
        if (pScreen)
        {
            RedScreenGroupElement* pGroup = new RedScreenGroupElement(pcName);
            pGroup->SetUseSafezone(true);
            pCurCreateScreenGroup = pGroup;
            pCurCreateObject = pGroup;
            pScreen->AddToScreen(pGroup);
        }
        else
        {
            RedInterfaceElement* pParent = RedInterfaceElement::Find(acParent);
            if (!pParent || !pParent->IsGroup())
            {
                return false;
            }
            pCurCreateScreenGroup = 0;
            RedGroupElement* pGroup = new RedGroupElement(pcName);
            pCurCreateObject = pGroup;
            ((RedGroupElement*)pParent)->AddElement(pGroup);
        }
    }
    pCurCreateObject->AddToSearchList(pcName);
    return true;
}

// FUNCTION: SWBF2 0x00580bc0
bool Util_GetRedInterfaceElement(int iNumArgs, RedInterfaceElement*& pElement,
                                 int& iArgStart, bool bExactArgs)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop > iNumArgs && lua_type(LuaHelper::mState, 1) == LUA_TLIGHTUSERDATA)
    {
        iArgStart = 2;
        if (bExactArgs && iTop > iNumArgs + 1)
        {
            return false;
        }
        pElement = (RedInterfaceElement*)lua_topointer(LuaHelper::mState, 1);
        return pElement != 0;
    }

    iArgStart = 1;
    if (iTop < iNumArgs || (bExactArgs && iTop > iNumArgs + 1))
    {
        return false;
    }
    pElement = pCurCreateObject;
    return pElement != 0;
}

// FUNCTION: SWBF2 0x00580c60
bool Util_GetRedScreenGroupElement(int iNumArgs, RedScreenGroupElement*& pElement,
                                   int& iArgStart, bool bExactArgs)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop > iNumArgs && lua_type(LuaHelper::mState, 1) == LUA_TLIGHTUSERDATA)
    {
        iArgStart = 2;
        if (bExactArgs && iTop > iNumArgs + 1)
        {
            return false;
        }
        RedScreenGroupElement* pGroup = (RedScreenGroupElement*)lua_topointer(LuaHelper::mState, 1);
        if (!pGroup->IsScreenGroup())
        {
            return false;
        }
        pElement = pGroup;
        return pElement != 0;
    }

    iArgStart = 1;
    if (iTop < iNumArgs || (bExactArgs && iTop > iNumArgs + 1))
    {
        return false;
    }
    pElement = pCurCreateScreenGroup;
    return pElement != 0;
}

// FUNCTION: SWBF2 0x00580d00
int ScriptCB_IFObj_SetPos(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(2, pElement, iArg, false))
    {
        return 0;
    }

    int iNumArgs = lua_gettop(LuaHelper::mState) - iArg + 1;
    float fX = 0.0f;
    float fY = 0.0f;
    if (lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        fX = (float)luaL_checknumber(LuaHelper::mState, iArg);
        fY = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
    }
    if (iNumArgs != 2 && lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        pElement->SetPosition(fX, fY, (float)luaL_checknumber(LuaHelper::mState, iArg + 2));
        return 0;
    }
    pElement->SetPosition(fX, fY);
    return 0;
}

// FUNCTION: SWBF2 0x00580e30
int ScriptCB_IFObj_GetPos(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, pElement->m_mLocal.m[3][0]);
    lua_pushnumber(LuaHelper::mState, pElement->m_mLocal.m[3][1]);
    return 2;
}

// FUNCTION: SWBF2 0x00580ea0
int ScriptCB_IFObj_SetBasicRotation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg))
    {
        pElement->Rotate((float)luaL_checknumber(LuaHelper::mState, iArg));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00580f10
int ScriptCB_IFObj_SetRelativeRotation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(3, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1) && lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        float fX = (float)luaL_checknumber(LuaHelper::mState, iArg);
        float fY = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
        float fZ = (float)luaL_checknumber(LuaHelper::mState, iArg + 2);
        pElement->Rotate(fX, fY, fZ);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00580fe0
int ScriptCB_IFObj_SetRotation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(3, pElement, iArg, true))
    {
        return 0;
    }

    float fX = 0.0f;
    float fY = 0.0f;
    float fZ = 0.0f;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fX = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    if (lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        fY = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
    }
    if (lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        fZ = (float)luaL_checknumber(LuaHelper::mState, iArg + 2);
    }

    PblMatrix mRot;
    PblMatrix mTemp;
    D3DXMatrixRotationY(&mRot, fY * DEG_TO_RAD);
    D3DXMatrixMultiply(&mRot, &mRot, D3DXMatrixRotationX(&mTemp, fX * DEG_TO_RAD));
    D3DXMatrixMultiply(&mRot, &mRot, D3DXMatrixRotationZ(&mTemp, fZ * DEG_TO_RAD));
    D3DXMatrixMultiply(&mTemp, &mRot, &pElement->m_mLocal);
    pElement->SetMatrix(mTemp);
    return 0;
}

// FUNCTION: SWBF2 0x00581160
int ScriptCB_IFObj_SetAlpha(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg))
    {
        float fAlpha = (float)luaL_checknumber(LuaHelper::mState, iArg);
        if (fAlpha > 0.99f)
        {
            pElement->m_color.a = 0xff;
        }
        else
        {
            pElement->m_color.a = (unsigned char)(int)(fAlpha * 255.0f);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005811e0
int ScriptCB_IFObj_GetAlpha(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (lua_Number)pElement->m_color.a);
    return 1;
}

// FUNCTION: SWBF2 0x00581240
int ScriptCB_IFObj_SetColor(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(3, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1) && lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        float fR = (float)luaL_checknumber(LuaHelper::mState, iArg);
        float fG = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
        float fB = (float)luaL_checknumber(LuaHelper::mState, iArg + 2);
        pElement->m_color.r = (unsigned char)(int)fR;
        pElement->m_color.g = (unsigned char)(int)fG;
        pElement->m_color.b = (unsigned char)(int)fB;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581320
int ScriptCB_IFObj_GetColor(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    RedColor color = pElement->m_color;
    lua_pushnumber(LuaHelper::mState, (lua_Number)color.r);
    lua_pushnumber(LuaHelper::mState, (lua_Number)color.g);
    lua_pushnumber(LuaHelper::mState, (lua_Number)color.b);
    return 3;
}

// FUNCTION: SWBF2 0x005813c0
int ScriptCB_IFObj_SetZOrder(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg))
    {
        int iZOrder = (int)(float)luaL_checknumber(LuaHelper::mState, iArg);
        pElement->SetZOrder((unsigned char)iZOrder);
        if (pElement == pCurCreateObject && pCurCreateScreenGroup && pCurCreateScreenGroup != pCurCreateObject)
        {
            pCurCreateScreenGroup->SetZOrder((unsigned char)iZOrder);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581440
int ScriptCB_IFObj_GetZOrder(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (lua_Number)pElement->m_zorder);
    return 1;
}

// FUNCTION: SWBF2 0x005814a0
int ScriptCB_IFObj_BringToFront(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(0, pElement, iArg, true) && pElement)
    {
        pElement->BringToFront();
        if (pElement == pCurCreateObject && pCurCreateScreenGroup && pCurCreateScreenGroup != pCurCreateObject)
        {
            pCurCreateScreenGroup->BringToFront();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005814f0
int ScriptCB_IFObj_SendToBack(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(0, pElement, iArg, true) && pElement)
    {
        pElement->SendToBack();
        if (pElement == pCurCreateObject && pCurCreateScreenGroup && pCurCreateScreenGroup != pCurCreateObject)
        {
            pCurCreateScreenGroup->SendToBack();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581540
int ScriptCB_IFObj_Enable(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(0, pElement, iArg, true) && pElement
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        pElement->Enable((int)(float)luaL_checknumber(LuaHelper::mState, iArg) != 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005815b0
int ScriptCB_IFObj_IsEnabled(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (lua_Number)pElement->IsEnabled());
    return 1;
}

// FUNCTION: SWBF2 0x00581610
int ScriptCB_IFObj_IsGroup(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (lua_Number)(pElement->IsGroup() == true));
    return 1;
}

// FUNCTION: SWBF2 0x00581670
int ScriptCB_IFObj_SetVis(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true))
    {
        pElement->Enable(lua_toboolean(LuaHelper::mState, iArg) != 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005816c0
int ScriptCB_IFObj_SetEnabled(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true))
    {
        pElement->Visible(lua_toboolean(LuaHelper::mState, iArg) != 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581710
int ScriptCB_IFObj_GetVis(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement)
    {
        return 0;
    }
    lua_pushboolean(LuaHelper::mState, pElement->IsEnabled());
    return 1;
}

// FUNCTION: SWBF2 0x00581760
int ScriptCB_IFObj_SetWidthHeight(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(2, pElement, iArg, true) && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        float fWidth = (float)luaL_checknumber(LuaHelper::mState, iArg);
        float fHeight = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
        pElement->Scale(fWidth, fHeight);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581800
int ScriptCB_IFObj_GetWidthHeight(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    Util_GetRedInterfaceElement(0, pElement, iArg, true);
    return 0;
}

// FUNCTION: SWBF2 0x00581820
int ScriptCB_IFObj_SetPosition(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, false))
    {
        lua_gettop(LuaHelper::mState);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581860
int ScriptCB_IFObj_GetPosition(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    Util_GetRedInterfaceElement(0, pElement, iArg, true);
    return 0;
}

// FUNCTION: SWBF2 0x00581880
int ScriptCB_IFObj_MouseOver(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 3 || lua_type(LuaHelper::mState, 1) != LUA_TLIGHTUSERDATA
        || !lua_isnumber(LuaHelper::mState, 2) || !lua_isnumber(LuaHelper::mState, 3))
    {
        return 0;
    }

    RedInterfaceElement* pElement = (RedInterfaceElement*)lua_topointer(LuaHelper::mState, 1);
    float fX = (float)luaL_checknumber(LuaHelper::mState, 2);
    float fY = (float)luaL_checknumber(LuaHelper::mState, 3);
    float fLocalX = 0.0f;
    float fLocalY = 0.0f;
    bool bOver = pElement->m_pHotSpot && pElement->m_pHotSpot->GetLocalScaledCoordinates(fX, fY, fLocalX, fLocalY);
    lua_pushboolean(LuaHelper::mState, bOver);
    lua_pushnumber(LuaHelper::mState, fLocalX);
    lua_pushnumber(LuaHelper::mState, fLocalY);
    return 3;
}

// FUNCTION: SWBF2 0x005819b0
int ScriptCB_IFObj_TestHotSpot(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1)
    {
        return 0;
    }

    bool bHit = false;
    RedInterfaceElement* pElement = (RedInterfaceElement*)lua_topointer(LuaHelper::mState, 1);
    if (pElement)
    {
        GUIInputs* pInputs = FLInputManager::s_instance.GetPlayerGuiInputs(0);
        bHit = pElement->TestHotSpot((float)pInputs->mMouseCoords.mPosX, (float)pInputs->mMouseCoords.mPosY);
    }
    lua_pushboolean(LuaHelper::mState, bHit);
    return 1;
}

// FUNCTION: SWBF2 0x00581a40
int ScriptCB_IFObj_CreateHotSpot(lua_State* L)
{
    int iTop = lua_gettop(LuaHelper::mState);
    if (iTop < 1 || (iTop > 3 && iTop != 5) || lua_type(LuaHelper::mState, 1) != LUA_TLIGHTUSERDATA)
    {
        return 0;
    }
    for (int i = 2; i <= iTop; ++i)
    {
        if (!lua_isnumber(LuaHelper::mState, i))
        {
            return 0;
        }
    }

    RedInterfaceElement* pElement = (RedInterfaceElement*)lua_topointer(LuaHelper::mState, 1);
    if (iTop == 1)
    {
        pElement->CreateHotSpot(0.0f);
    }
    else if (iTop == 2)
    {
        pElement->CreateHotSpot((float)luaL_checknumber(LuaHelper::mState, 2));
    }
    else if (iTop == 3)
    {
        float fRight = (float)luaL_checknumber(LuaHelper::mState, 2);
        float fBottom = (float)luaL_checknumber(LuaHelper::mState, 3);
        pElement->CreateHotSpot(0.0f, 0.0f, fRight, fBottom);
    }
    else if (iTop == 5)
    {
        float fLeft = (float)luaL_checknumber(LuaHelper::mState, 2);
        float fTop = (float)luaL_checknumber(LuaHelper::mState, 3);
        float fRight = (float)luaL_checknumber(LuaHelper::mState, 4);
        float fBottom = (float)luaL_checknumber(LuaHelper::mState, 5);
        pElement->CreateHotSpot(fLeft, fTop, fRight, fBottom);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581bf0
int ScriptCB_IFObj_RemoveHotSpot(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_type(LuaHelper::mState, 1) == LUA_TLIGHTUSERDATA)
    {
        ((RedInterfaceElement*)lua_topointer(LuaHelper::mState, 1))->RemoveHotSpot();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581c50
int ScriptCB_GetFontHeight(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    RedFont* pFont = RedFont::Find(luaL_checklstring(LuaHelper::mState, 1, 0));
    if (pFont)
    {
        lua_pushnumber(LuaHelper::mState, (lua_Number)pFont->m_uiHeight);
    }
    else
    {
        lua_pushnumber(LuaHelper::mState, 16.0f);
    }
    return 1;
}

// FUNCTION: SWBF2 0x00581cc0
int ScriptCB_IFText_SetString(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, false))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    if (!lua_isstring(LuaHelper::mState, iArg))
    {
        return 0;
    }

    RedTextElement* pText = (RedTextElement*)pElement;
    const char* pcKey = luaL_checklstring(LuaHelper::mState, iArg, 0);
    const unsigned short* pusLocalized = RedLocalizeDB::Find(pcKey ? (unsigned int)PblHash(pcKey) : 0, true);
    if (pusLocalized)
    {
        int iCase = (int)(float)luaL_optnumber(LuaHelper::mState, iArg + 1, 0.0f);
        if (iCase)
        {
            unsigned short ausText[1024];
            PblUnicode::ustrlcpy(ausText, pusLocalized, 1024);
            if (iCase > 0)
            {
                PblUnicode::ustrupr(ausText);
            }
            else
            {
                PblUnicode::ustrlwr(ausText);
            }
            pText->SetText(ausText, true);
        }
        else
        {
            pText->SetText(pusLocalized, true);
        }
    }
    else
    {
        memset(Lua_Callbacks::sTempReturnString16, 0, sizeof(Lua_Callbacks::sTempReturnString16));
        PblUnicode::ustrncnv(Lua_Callbacks::sTempReturnString16, (const unsigned char*)pcKey, 255);
        int iCase = (int)(float)luaL_optnumber(LuaHelper::mState, iArg + 1, 0.0f);
        if (iCase > 0)
        {
            PblUnicode::ustrupr(Lua_Callbacks::sTempReturnString16);
        }
        else if (iCase < 0)
        {
            PblUnicode::ustrlwr(Lua_Callbacks::sTempReturnString16);
        }
        pText->SetText(Lua_Callbacks::sTempReturnString16, true);
    }
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x00581e60
int ScriptCB_IFText_SetUString(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText()) && lua_isstring(LuaHelper::mState, iArg))
    {
        const unsigned short* pusText = (const unsigned short*)luaL_checklstring(LuaHelper::mState, iArg, 0);
        ((RedTextElement*)pElement)->SetText(pusText, true);
        DoPeriodicLuaGC(false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581ee0
int ScriptCB_IFText_SetTextPosition(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, false))
    {
        pElement->IsText();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581f10
int ScriptCB_IFText_SetFont(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText()) && lua_isstring(LuaHelper::mState, iArg))
    {
        ((RedTextElement*)pElement)->SetFont(luaL_checklstring(LuaHelper::mState, iArg, 0));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00581f90
int ScriptCB_IFText_SetJustify(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, false))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }

    RedTextElement* pText = (RedTextElement*)pElement;
    RedTextElement::TextHAlignT eHAlign;
    RedTextElement::TextVAlignT eVAlign;
    pText->GetTextAlign(eHAlign, eVAlign);
    int iTop = lua_gettop(LuaHelper::mState);
    for (int i = iArg; i <= iTop; ++i)
    {
        if (!lua_isstring(LuaHelper::mState, i))
        {
            continue;
        }
        switch ((unsigned int)PblHash(luaL_checklstring(LuaHelper::mState, i, 0)))
        {
        case 0x124aec70: eHAlign = RedTextElement::TEXT_HALIGN_LEFT; break;
        case 0x78e32de5: eHAlign = RedTextElement::TEXT_HALIGN_RIGHT; break;
        case 0x931be59a: eHAlign = RedTextElement::TEXT_HALIGN_CENTER; break;
        case 0xa710dc3c: eVAlign = RedTextElement::TEXT_VALIGN_TOP; break;
        case 0x4ea76b2a: eVAlign = RedTextElement::TEXT_VALIGN_BOTTOM; break;
        case 0x36928608: eVAlign = RedTextElement::TEXT_VALIGN_CENTER; break;
        case 0xabf8de42: eVAlign = RedTextElement::TEXT_VALIGN_BASELINE; break;
        }
    }
    pText->SetTextAlign(eHAlign, eVAlign);
    return 0;
}

// FUNCTION: SWBF2 0x005820d0
int ScriptCB_IFText_SetTextBreak(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    if (!lua_isstring(LuaHelper::mState, iArg))
    {
        return 0;
    }

    RedTextElement* pText = (RedTextElement*)pElement;
    switch ((unsigned int)PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0)))
    {
    case 0xada7afdb: pText->SetTextBreak(RedTextElement::TEXT_BREAK_NONE); break;
    case 0x6a98e9fd: pText->SetTextBreak(RedTextElement::TEXT_BREAK_WORD); break;
    case 0x6317ba9a: pText->SetTextBreak(RedTextElement::TEXT_BREAK_NONE_REFORMAT); break;
    case 0x30df2cc4: pText->SetTextBreak(RedTextElement::TEXT_BREAK_WORD_REFORMAT); break;
    }
    return 0;
}

// FUNCTION: SWBF2 0x005821b0
int ScriptCB_IFText_SetTextScale(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(2, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText()) && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        float fScaleX = (float)luaL_checknumber(LuaHelper::mState, iArg);
        float fScaleY = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
        ((RedTextElement*)pElement)->SetTextScale(fScaleX, fScaleY);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582270
int ScriptCB_IFText_GetTextScaleX(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, ((RedTextElement*)pElement)->m_fScaleX);
    return 1;
}

// FUNCTION: SWBF2 0x005822e0
int ScriptCB_IFText_GetTextScaleY(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, ((RedTextElement*)pElement)->m_fScaleY);
    return 1;
}

// FUNCTION: SWBF2 0x00582350
int ScriptCB_IFText_GetTextExtent(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (lua_Number)((RedTextElement*)pElement)->m_iTextExtent);
    return 1;
}

// FUNCTION: SWBF2 0x005823d0
int ScriptCB_IFText_GetDisplayRect(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    float fLeft, fTop, fRight, fBottom;
    ((RedTextElement*)pElement)->GetRect(fLeft, fTop, fRight, fBottom);
    lua_pushnumber(LuaHelper::mState, fLeft);
    lua_pushnumber(LuaHelper::mState, fTop);
    lua_pushnumber(LuaHelper::mState, fRight);
    lua_pushnumber(LuaHelper::mState, fBottom);
    return 4;
}

// FUNCTION: SWBF2 0x005825a0
int ScriptCB_IFText_SetTextBox(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(2, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText()) && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        int iWidth = (int)(float)luaL_checknumber(LuaHelper::mState, iArg);
        int iHeight = (int)(float)luaL_checknumber(LuaHelper::mState, iArg + 1);
        ((RedTextElement*)pElement)->SetTextBox(iWidth, iHeight);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005826f0
int ScriptCB_IFText_SetTextStyle(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, true))
    {
        return 0;
    }
    if (!pElement->IsText() && !pElement->IsFlashyText())
    {
        return 0;
    }
    if (!lua_isstring(LuaHelper::mState, iArg))
    {
        return 0;
    }

    RedTextElement* pText = (RedTextElement*)pElement;
    switch ((unsigned int)PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0)))
    {
    case 0x3a43c561:
    case 0x5b02d9ef:
    case 0x6a3827bc:
        pText->SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SHADOW);
        break;
    case 0x4e0a1774:
        pText->SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_SELECTED);
        break;
    case 0xe68b9c52:
        pText->SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
        break;
    }
    return 0;
}

// FUNCTION: SWBF2 0x005828c0
int ScriptCB_IFFlashyText_Setup(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(3, pElement, iArg, false))
    {
        return 0;
    }
    if (!pElement->IsFlashyText())
    {
        return 0;
    }

    FlashyTextElement* pFlashy = (FlashyTextElement*)pElement;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        pFlashy->mTimeDelay = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ++iArg;
    pFlashy->mFlipped = lua_toboolean(LuaHelper::mState, iArg) != 0;
    ++iArg;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        pFlashy->mWidth = (unsigned short)(int)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ++iArg;
    if (iArg < lua_gettop(LuaHelper::mState))
    {
        pFlashy->mXFlipped = lua_toboolean(LuaHelper::mState, iArg) != 0;
        ++iArg;
        if (iArg < lua_gettop(LuaHelper::mState) && lua_isnumber(LuaHelper::mState, iArg))
        {
            pFlashy->mTail = (unsigned char)(int)LuaHelper::GetFloat(iArg);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582a10
int ScriptCB_IFFlashyText_SetTextColor(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(3, pElement, iArg, true))
    {
        return 0;
    }

    float fR = 255.0f;
    float fG = 255.0f;
    float fB = 255.0f;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fR = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    if (lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        fG = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
    }
    if (lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        fB = (float)luaL_checknumber(LuaHelper::mState, iArg + 2);
    }

    if (pElement->IsFlashyText())
    {
        RedColor color;
        color.a = 0xff;
        color.b = (unsigned char)(int)fB;
        color.g = (unsigned char)(int)fG;
        color.r = (unsigned char)(int)fR;
        ((FlashyTextElement*)pElement)->mTextColor = color;
    }
    else if (pElement->IsText())
    {
        pElement->m_color.b = (unsigned char)(int)fB;
        pElement->m_color.g = (unsigned char)(int)fG;
        pElement->m_color.r = (unsigned char)(int)fR;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582b50
int ScriptCB_IFFlashyText_SetBackground(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(3, pElement, iArg, true) || !pElement->IsFlashyText())
    {
        return 0;
    }

    unsigned int uHashLeft = 0;
    unsigned int uHashMid = 0;
    unsigned int uHashRight = 0;
    if (lua_isstring(LuaHelper::mState, iArg))
    {
        uHashLeft = PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0));
    }
    ++iArg;
    if (lua_isstring(LuaHelper::mState, iArg))
    {
        uHashMid = PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0));
    }
    ++iArg;
    if (lua_isstring(LuaHelper::mState, iArg))
    {
        uHashRight = PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0));
    }
    ((FlashyTextElement*)pElement)->SetBackground(uHashLeft, uHashMid, uHashRight);
    return 0;
}

// FUNCTION: SWBF2 0x00582c40
int ScriptCB_IFFlashyText_SetBackgroundSize(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(4, pElement, iArg, true) || !pElement->IsFlashyText())
    {
        return 0;
    }

    float fOffsetX = 0.0f;
    float fOffsetY = 0.0f;
    float fExpandX = 0.0f;
    float fExpandY = 0.0f;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fOffsetX = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ++iArg;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fOffsetY = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ++iArg;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fExpandX = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ++iArg;
    if (lua_isnumber(LuaHelper::mState, iArg))
    {
        fExpandY = (float)luaL_checknumber(LuaHelper::mState, iArg);
    }
    ((FlashyTextElement*)pElement)->SetBackgroundSize((char)(int)fOffsetX, (char)(int)fOffsetY,
                                                      (char)(int)fExpandX, (char)(int)fExpandY);
    return 0;
}

// FUNCTION: SWBF2 0x00582d90
int ScriptCB_IFFlashyText_SetBackgroundRightJustify(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsFlashyText())
    {
        ((FlashyTextElement*)pElement)->mRightJustifyBG = lua_toboolean(LuaHelper::mState, iArg) != 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582e00
int ScriptCB_IFFlashyText_StartFlashiness(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(0, pElement, iArg, true) && pElement->IsFlashyText())
    {
        ((FlashyTextElement*)pElement)->StartFlashiness();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582e40
int ScriptCB_IFImage_SetTexture(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsBitmap()
        && lua_isstring(LuaHelper::mState, iArg))
    {
        const char* pcTexture = luaL_checklstring(LuaHelper::mState, iArg, 0);
        pElement->IsMaskedBitmap();
        ((RedBitmapElement*)pElement)->SetTexture(PblHash(pcTexture));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582f90
int ScriptCB_IFImage_SetRect(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, true) && pElement->IsBitmap()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2) && LuaHelper::IsNumber(iArg + 3))
    {
        float fLeft = LuaHelper::GetFloat(iArg);
        float fTop = LuaHelper::GetFloat(iArg + 1);
        float fRight = LuaHelper::GetFloat(iArg + 2);
        float fBottom = LuaHelper::GetFloat(iArg + 3);
        ((RedBitmapElement*)pElement)->SetRect(fLeft, fTop, fRight, fBottom);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583160
int ScriptCB_IFImage_SetTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, false) && pElement->IsBitmap()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2) && LuaHelper::IsNumber(iArg + 3))
    {
        float fU0 = LuaHelper::GetFloat(iArg);
        float fV0 = LuaHelper::GetFloat(iArg + 1);
        float fU1 = LuaHelper::GetFloat(iArg + 2);
        float fV1 = LuaHelper::GetFloat(iArg + 3);
        bool bSwap = LuaHelper::GetBool(iArg + 4);
        ((RedBitmapElement*)pElement)->SetTexCoords(fU0, fV0, fU1, fV1, bSwap);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005834d0
int ScriptCB_IFBorder_SetTexture(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsBorderElement()
        && lua_isstring(LuaHelper::mState, iArg))
    {
        ((RedBorderElement*)pElement)->SetTextureHash(PblHash(luaL_checklstring(LuaHelper::mState, iArg, 0)));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583650
int ScriptCB_IFBorder_SetRect(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, false) && pElement->IsBorderElement()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2) && LuaHelper::IsNumber(iArg + 3)
        && (LuaHelper::IsNoneOrNil(iArg + 4) || LuaHelper::IsNumber(iArg + 4))
        && (LuaHelper::IsNoneOrNil(iArg + 5) || LuaHelper::IsNumber(iArg + 5)))
    {
        float fLeft = LuaHelper::GetFloat(iArg);
        float fTop = LuaHelper::GetFloat(iArg + 1);
        float fRight = LuaHelper::GetFloat(iArg + 2);
        float fBottom = LuaHelper::GetFloat(iArg + 3);
        float fBorderX = LuaHelper::GetFloat(iArg + 4, 0.0f);
        float fBorderY = LuaHelper::GetFloat(iArg + 5, 0.0f);
        ((RedBorderElement*)pElement)->SetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005838e0
int ScriptCB_IFBorder_SetTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, false) && pElement->IsBorderElement()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2) && LuaHelper::IsNumber(iArg + 3)
        && (LuaHelper::IsNoneOrNil(iArg + 4) || LuaHelper::IsNumber(iArg + 4))
        && (LuaHelper::IsNoneOrNil(iArg + 5) || LuaHelper::IsNumber(iArg + 5)))
    {
        float fLeft = LuaHelper::GetFloat(iArg);
        float fTop = LuaHelper::GetFloat(iArg + 1);
        float fRight = LuaHelper::GetFloat(iArg + 2);
        float fBottom = LuaHelper::GetFloat(iArg + 3);
        float fBorderX = LuaHelper::GetFloat(iArg + 4, 0.0f);
        float fBorderY = LuaHelper::GetFloat(iArg + 5, 0.0f);
        bool bFlag = LuaHelper::IsBool(iArg + 6) && LuaHelper::GetBool(iArg + 6);
        ((RedBorderElement*)pElement)->SetBorderTexCoords(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY, bFlag);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005841b0
int ScriptCB_IFModel_SetScale(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        float fScale = (float)luaL_checknumber(LuaHelper::mState, iArg);
        if (fScale > 0.0f)
        {
            ((Red3DModelElement*)pElement)->SetDepth(0.0045f / fScale);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005842a0
int ScriptCB_IFModel_SetRotation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2) && LuaHelper::IsNumber(iArg + 3))
    {
        PblQuaternion q;
        q._fS = LuaHelper::GetFloat(iArg);
        q._fX = LuaHelper::GetFloat(iArg + 1);
        q._fY = LuaHelper::GetFloat(iArg + 2);
        q._fZ = LuaHelper::GetFloat(iArg + 3);
        ((Red3DModelElement*)pElement)->m_rotation = q;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584440
int ScriptCB_IFModel_SetTranslation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(3, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2))
    {
        float fX = LuaHelper::GetFloat(iArg);
        float fY = LuaHelper::GetFloat(iArg + 1);
        float fZ = LuaHelper::GetFloat(iArg + 2);
        ((Red3DModelElement*)pElement)->m_translation = PblVector3(fX, fY, fZ);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005845c0
int ScriptCB_IFModel_SetDepth(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        ((Red3DModelElement*)pElement)->SetDepth((float)luaL_checknumber(LuaHelper::mState, iArg));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584690
int ScriptCB_IFModel_SetOmegaY(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        ((Red3DModelElement*)pElement)->SetOmegaY((float)luaL_checknumber(LuaHelper::mState, iArg));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584700
int ScriptCB_IFModel_SetLighting(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        float fEnable = (float)luaL_checknumber(LuaHelper::mState, iArg);
        ((Red3DModelElement*)pElement)->EnableLighting((int)fEnable != 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584880
int ScriptCB_AddIFScreen(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        IFScreen* pScreen = new IFScreen(pcName);
        IFScreen::SetCurScreen(GuiManager::mCurViewport, pScreen);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584920
int ScriptCB_SetIFScreenViewport(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 2 && lua_isstring(LuaHelper::mState, 1)
        && lua_isnumber(LuaHelper::mState, 2))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        IFScreen* pScreen = IFScreen::GetCurScreen(GuiManager::mCurViewport);
        unsigned int uiName = pcName ? (unsigned int)PblHash(pcName) : 0;
        (void)uiName;
        pScreen->SetViewport((unsigned int)luaL_checknumber(LuaHelper::mState, 2));
    }
    return 0;
}

// FUNCTION: SWBF2 0x005849c0
int ScriptCB_EndIFScreen(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        IFScreen* pScreen = IFScreen::GetCurScreen(GuiManager::mCurViewport);
        unsigned int uiName = pcName ? (unsigned int)PblHash(pcName) : 0;
        (void)uiName;
        pScreen->Visible(true);
        pScreen->Visible(false);
        IFScreen::SetCurScreen(GuiManager::mCurViewport, 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584b50
int ScriptCB_IFObj_GetCPointer(lua_State* L)
{
    if (!pCurCreateObject)
    {
        return 0;
    }
    lua_pushlightuserdata(LuaHelper::mState, pCurCreateObject);
    return 1;
}

// FUNCTION: SWBF2 0x00584b70
int ScriptCB_IFObj_SetScreenPosition(lua_State* L)
{
    RedScreenGroupElement* pGroup;
    int iArg;
    if (Util_GetRedScreenGroupElement(2, pGroup, iArg, true) && pGroup && pGroup->IsScreenGroup()
        && lua_isnumber(LuaHelper::mState, iArg) && lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        float fX = LuaHelper::GetFloat(iArg);
        float fY = LuaHelper::GetFloat(iArg + 1);
        pGroup->SetScreenRelativePos(fX, fY);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584c10
int ScriptCB_IFObj_SetUseSafezone(lua_State* L)
{
    RedScreenGroupElement* pGroup;
    int iArg;
    if (Util_GetRedScreenGroupElement(1, pGroup, iArg, true) && pGroup && pGroup->IsScreenGroup()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        pGroup->SetUseSafezone((float)luaL_checknumber(LuaHelper::mState, iArg) >= 0.1f);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584ca0
int ScriptCB_EndIFObj(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && pCurCreateObject)
    {
        if (lua_isstring(LuaHelper::mState, 1))
        {
            luaL_checklstring(LuaHelper::mState, 1, 0);
        }
        pCurCreateObject = 0;
        pCurCreateScreenGroup = 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584d00
int ScriptCB_AddIFText(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        RedTextElement* pText = new RedTextElement(pcName);
        pText->Enable(true);
        pText->SetFont(RedFont::GAMEFONT_SMALL);
        Util_AddToHierarchy(pText, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584dc0
int ScriptCB_AddIFFlashyText(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        FlashyTextElement* pFlashy = new FlashyTextElement(pcName);
        pFlashy->Enable(true);
        pFlashy->Init();
        Util_AddToHierarchy(pFlashy, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584e80
int ScriptCB_AddIFImage(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        RedBitmapElement* pBitmap = new RedBitmapElement(pcName);
        pBitmap->Enable(true);
        pBitmap->SetRect(0.0f, 0.0f, 100.0f, 100.0f);
        Util_AddToHierarchy(pBitmap, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585060
int ScriptCB_AddIFBorder(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        RedBorderElement* pBorder = new RedBorderElement(pcName);
        pBorder->Enable(true);
        pBorder->SetBorderRect(0.0f, 0.0f, 100.0f, 100.0f, 0.0f, 0.0f);
        Util_AddToHierarchy(pBorder, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585160
int ScriptCB_AddIFModel(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        Red3DModelElement* pModel = new Red3DModelElement(pcName);
        pModel->Enable(true);
        Util_AddToHierarchy(pModel, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585210
int ScriptCB_AddIFContainer(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        Util_CreateGroupInHierarchy(luaL_checklstring(LuaHelper::mState, 1, 0));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585260
int ScriptCB_SetIFScreen(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        GuiManager* pGui = GuiManager::Instance();
        if (pGui->IsPopupOpen())
        {
            ScriptCB_ClosePopup(0);
        }
        pGui->ChangeToScreen(pcName, false, false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x005852e0
int ScriptCB_PushScreen(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        GuiManager* pGui = GuiManager::Instance();
        if (pGui->IsPopupOpen())
        {
            ScriptCB_ClosePopup(0);
        }
        pGui->PushScreen(pcName, -1, 0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585360
int ScriptCB_PopScreen(lua_State* L)
{
    const char* pcName = 0;
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
    }
    GuiManager::Instance()->PopScreen(pcName);
    return 0;
}

// FUNCTION: SWBF2 0x00585450
int ScriptCB_OpenPopup(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        GuiManager::Instance()->OpenPopup(luaL_checklstring(LuaHelper::mState, 1, 0));
    }
    return 0;
}

// FUNCTION: SWBF2 0x005854c0
int ScriptCB_ClosePopup(lua_State* L)
{
    GuiManager::Instance()->ClosePopup(false);
    return 0;
}

// FUNCTION: SWBF2 0x005854f0
int ScriptCB_IsPopupOpen(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GuiManager::Instance()->IsPopupOpen());
    return 1;
}

// FUNCTION: SWBF2 0x00585560
int ScriptCB_OpenMovie(lua_State* L)
{
    GameMovie::Open(luaL_checklstring(LuaHelper::mState, 1, 0), luaL_checklstring(LuaHelper::mState, 2, 0),
                    GameMemory::RunTimeHeap, GameMemory::RunTimeHeap);
    return 0;
}

// FUNCTION: SWBF2 0x005855a0
int ScriptCB_CloseMovie(lua_State* L)
{
    GameMovie::Close();
    return 0;
}

// FUNCTION: SWBF2 0x005855b0
int ScriptCB_PlayMovie(lua_State* L)
{
    const char* pcAtEnd = strlen(luaL_checklstring(LuaHelper::mState, 3, 0))
                              ? luaL_checklstring(LuaHelper::mState, 3, 0)
                              : 0;
    GameMovie::Play(luaL_checklstring(LuaHelper::mState, 1, 0), pcAtEnd,
                    (unsigned int)luaL_checknumber(LuaHelper::mState, 4),
                    (unsigned int)luaL_checknumber(LuaHelper::mState, 5),
                    (unsigned int)luaL_checknumber(LuaHelper::mState, 6),
                    (unsigned int)luaL_checknumber(LuaHelper::mState, 7),
                    (int)(float)luaL_checknumber(LuaHelper::mState, 2) > 0);
    return 0;
}

// GLOBAL: SWBF2 0x01faa694

static bool sMoviePlayingLastFrame;

// FUNCTION: SWBF2 0x005856a0
int ScriptCB_IsMoviePlaying(lua_State* L)
{
    bool bPlaying = GameMovie::IsPlaying();
    if (GameState::GetState() != 0x11e1fc01)
    {
        if (bPlaying || sMoviePlayingLastFrame)
        {
            FLRenderer::EnableCursor(!bPlaying);
            sMoviePlayingLastFrame = bPlaying;
        }
    }
    lua_pushboolean(LuaHelper::mState, bPlaying);
    return 1;
}

// FUNCTION: SWBF2 0x005856f0
int ScriptCB_AreMoviePropertiesPlaying(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GameMovie::PropertiesPlaying(luaL_checklstring(LuaHelper::mState, 1, 0), false));
    return 1;
}

// FUNCTION: SWBF2 0x00585740
int ScriptCB_SetMovieAudioBus(lua_State* L)
{
    GameMovie::SetAudioBus(luaL_checklstring(LuaHelper::mState, 1, 0));
    return 0;
}

// FUNCTION: SWBF2 0x00585860
int ScriptCB_GetScreenInfo(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    lua_pushnumber(LuaHelper::mState, (float)RedInterfaceScreen::GetScreenWidth());
    lua_pushnumber(LuaHelper::mState, (float)RedInterfaceScreen::GetScreenHeight());
    lua_pushnumber(LuaHelper::mState, RedInterfaceScreen::GetScreenBottomV());
    lua_pushnumber(LuaHelper::mState, 0.75f / RedRenderer::GetScreenAspectRatio());
    return 4;
}

// FUNCTION: SWBF2 0x00585920
int ScriptCB_GetSafeScreenInfo(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    float fSafeZone = RedInterfaceScreen::GetScreenSafeZone();
    lua_pushnumber(LuaHelper::mState, (float)RedInterfaceScreen::GetScreenWidth() * fSafeZone);
    lua_pushnumber(LuaHelper::mState, (float)RedInterfaceScreen::GetScreenHeight() * fSafeZone);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 3;
}

// FUNCTION: SWBF2 0x005859d0
int ScriptCB_IsErrorBoxOpen(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, GuiManager::Instance()->IsErrorOpen());
    return 1;
}

// GLOBAL: SWBF2 0x00789de0
static ePROCESSEDINPUT_TYPE sKeyboardCmdsPerMode[5][43] =
{
    {
        ePROCESSEDINPUT_primaryFireButtonDown,
        ePROCESSEDINPUT_primaryNextButtonPressed,
        ePROCESSEDINPUT_secondaryFireButtonDown,
        ePROCESSEDINPUT_secondaryNextButtonPressed,
        ePROCESSEDINPUT_lockTargetButtonPressed,
        ePROCESSEDINPUT_sprintButtonDown,
        ePROCESSEDINPUT_jumpButtonDown,
        ePROCESSEDINPUT_crouchButtonDown,
        ePROCESSEDINPUT_TakeScreenshot,
        ePROCESSEDINPUT_zoomButtonDown,
        ePROCESSEDINPUT_viewButtonDown,
        ePROCESSEDINPUT_reloadButtonDown,
        ePROCESSEDINPUT_useButtonPressed,
        ePROCESSEDINPUT_STRAFE_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_MOVE_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_TURN_AXIS,
        ePROCESSEDINPUT_PITCH_AXIS,
        ePROCESSEDINPUT_STRAFE_POS,
        ePROCESSEDINPUT_STRAFE_NEG,
        ePROCESSEDINPUT_MOVE_POS,
        ePROCESSEDINPUT_MOVE_NEG,
        ePROCESSEDINPUT_TURN_POS,
        ePROCESSEDINPUT_TURN_NEG,
        ePROCESSEDINPUT_PITCH_POS,
        ePROCESSEDINPUT_PITCH_NEG,
        ePROCESSEDINPUT_acceptHeroPressed,
        ePROCESSEDINPUT_declineHeroPressed,
        ePROCESSEDINPUT_playerList,
        ePROCESSEDINPUT_chat,
        ePROCESSEDINPUT_friendChat,
        ePROCESSEDINPUT_squadCommandButtonPressed,
        ePROCESSEDINPUT_Comm_Chat_0,
        ePROCESSEDINPUT_Comm_Chat_1,
        ePROCESSEDINPUT_Comm_Chat_2,
        ePROCESSEDINPUT_Comm_Chat_3,
        ePROCESSEDINPUT_Comm_Chat_4,
        ePROCESSEDINPUT_Comm_Chat_5,
        ePROCESSEDINPUT_Comm_Chat_6,
        ePROCESSEDINPUT_Comm_Chat_7,
        ePROCESSEDINPUT_Map,
        ePROCESSEDINPUT_MAX,
    },
    {
        ePROCESSEDINPUT_primaryFireButtonDown,
        ePROCESSEDINPUT_secondaryFireButtonDown,
        ePROCESSEDINPUT_lockTargetButtonPressed,
        ePROCESSEDINPUT_sprintButtonDown,
        ePROCESSEDINPUT_jumpButtonDown,
        ePROCESSEDINPUT_crouchButtonDown,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_zoomButtonDown,
        ePROCESSEDINPUT_viewButtonDown,
        ePROCESSEDINPUT_reloadButtonDown,
        ePROCESSEDINPUT_useButtonPressed,
        ePROCESSEDINPUT_primaryNextButtonPressed,
        ePROCESSEDINPUT_primaryPrevButtonPressed,
        ePROCESSEDINPUT_STRAFE_AXIS,
        ePROCESSEDINPUT_MOVE_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_TURN_AXIS,
        ePROCESSEDINPUT_PITCH_AXIS,
        ePROCESSEDINPUT_STRAFE_POS,
        ePROCESSEDINPUT_STRAFE_NEG,
        ePROCESSEDINPUT_MOVE_POS,
        ePROCESSEDINPUT_MOVE_NEG,
        ePROCESSEDINPUT_TURN_POS,
        ePROCESSEDINPUT_TURN_NEG,
        ePROCESSEDINPUT_PITCH_POS,
        ePROCESSEDINPUT_PITCH_NEG,
        ePROCESSEDINPUT_acceptHeroPressed,
        ePROCESSEDINPUT_declineHeroPressed,
        ePROCESSEDINPUT_playerList,
        ePROCESSEDINPUT_chat,
        ePROCESSEDINPUT_friendChat,
        ePROCESSEDINPUT_squadCommandButtonPressed,
        ePROCESSEDINPUT_Comm_Chat_0,
        ePROCESSEDINPUT_Comm_Chat_1,
        ePROCESSEDINPUT_Comm_Chat_2,
        ePROCESSEDINPUT_Comm_Chat_3,
        ePROCESSEDINPUT_Comm_Chat_4,
        ePROCESSEDINPUT_Comm_Chat_5,
        ePROCESSEDINPUT_Comm_Chat_6,
        ePROCESSEDINPUT_Comm_Chat_7,
        ePROCESSEDINPUT_Map,
        ePROCESSEDINPUT_MAX,
    },
    {
        ePROCESSEDINPUT_primaryFireButtonDown,
        ePROCESSEDINPUT_secondaryFireButtonDown,
        ePROCESSEDINPUT_lockTargetButtonPressed,
        ePROCESSEDINPUT_sprintButtonDown,
        ePROCESSEDINPUT_jumpButtonDown,
        ePROCESSEDINPUT_crouchButtonDown,
        ePROCESSEDINPUT_TakeScreenshot,
        ePROCESSEDINPUT_zoomButtonDown,
        ePROCESSEDINPUT_viewButtonDown,
        ePROCESSEDINPUT_reloadButtonDown,
        ePROCESSEDINPUT_useButtonPressed,
        ePROCESSEDINPUT_primaryNextButtonPressed,
        ePROCESSEDINPUT_primaryPrevButtonPressed,
        ePROCESSEDINPUT_STRAFE_AXIS,
        ePROCESSEDINPUT_MOVE_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_TURN_AXIS,
        ePROCESSEDINPUT_PITCH_AXIS,
        ePROCESSEDINPUT_STRAFE_POS,
        ePROCESSEDINPUT_STRAFE_NEG,
        ePROCESSEDINPUT_MOVE_POS,
        ePROCESSEDINPUT_MOVE_NEG,
        ePROCESSEDINPUT_TURN_POS,
        ePROCESSEDINPUT_TURN_NEG,
        ePROCESSEDINPUT_PITCH_POS,
        ePROCESSEDINPUT_PITCH_NEG,
        ePROCESSEDINPUT_acceptHeroPressed,
        ePROCESSEDINPUT_declineHeroPressed,
        ePROCESSEDINPUT_playerList,
        ePROCESSEDINPUT_chat,
        ePROCESSEDINPUT_friendChat,
        ePROCESSEDINPUT_squadCommandButtonPressed,
        ePROCESSEDINPUT_Comm_Chat_0,
        ePROCESSEDINPUT_Comm_Chat_1,
        ePROCESSEDINPUT_Comm_Chat_2,
        ePROCESSEDINPUT_Comm_Chat_3,
        ePROCESSEDINPUT_Comm_Chat_4,
        ePROCESSEDINPUT_Comm_Chat_5,
        ePROCESSEDINPUT_Comm_Chat_6,
        ePROCESSEDINPUT_Comm_Chat_7,
        ePROCESSEDINPUT_Map,
        ePROCESSEDINPUT_MAX,
    },
    {
        ePROCESSEDINPUT_primaryFireButtonDown,
        ePROCESSEDINPUT_secondaryFireButtonDown,
        ePROCESSEDINPUT_reloadButtonDown,
        ePROCESSEDINPUT_secondaryNextButtonPressed,
        ePROCESSEDINPUT_lockTargetButtonPressed,
        ePROCESSEDINPUT_sprintButtonDown,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_jumpButtonDown,
        ePROCESSEDINPUT_crouchButtonDown,
        ePROCESSEDINPUT_TakeScreenshot,
        ePROCESSEDINPUT_useButtonPressed,
        ePROCESSEDINPUT_STRAFE_AXIS,
        ePROCESSEDINPUT_MOVE_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_TURN_AXIS,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_PITCH_AXIS,
        ePROCESSEDINPUT_STRAFE_POS,
        ePROCESSEDINPUT_STRAFE_NEG,
        ePROCESSEDINPUT_MOVE_POS,
        ePROCESSEDINPUT_MOVE_NEG,
        ePROCESSEDINPUT_TURN_POS,
        ePROCESSEDINPUT_TURN_NEG,
        ePROCESSEDINPUT_PITCH_POS,
        ePROCESSEDINPUT_PITCH_NEG,
        ePROCESSEDINPUT_acceptHeroPressed,
        ePROCESSEDINPUT_declineHeroPressed,
        ePROCESSEDINPUT_playerList,
        ePROCESSEDINPUT_chat,
        ePROCESSEDINPUT_friendChat,
        ePROCESSEDINPUT_squadCommandButtonPressed,
        ePROCESSEDINPUT_Comm_Chat_0,
        ePROCESSEDINPUT_Comm_Chat_1,
        ePROCESSEDINPUT_Comm_Chat_2,
        ePROCESSEDINPUT_Comm_Chat_3,
        ePROCESSEDINPUT_Comm_Chat_4,
        ePROCESSEDINPUT_Comm_Chat_5,
        ePROCESSEDINPUT_Comm_Chat_6,
        ePROCESSEDINPUT_Comm_Chat_7,
        ePROCESSEDINPUT_Map,
        ePROCESSEDINPUT_MAX,
    },
    {
        ePROCESSEDINPUT_primaryFireButtonDown,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_lockTargetButtonPressed,
        ePROCESSEDINPUT_zoomButtonDown,
        ePROCESSEDINPUT_MOVE_AXIS,
        ePROCESSEDINPUT_MOVE_POS,
        ePROCESSEDINPUT_MOVE_NEG,
        ePROCESSEDINPUT_useButtonPressed,
        ePROCESSEDINPUT_primaryNextButtonPressed,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_primaryPrevButtonPressed,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_TURN_AXIS,
        ePROCESSEDINPUT_PITCH_AXIS,
        ePROCESSEDINPUT_TURN_POS,
        ePROCESSEDINPUT_TURN_NEG,
        ePROCESSEDINPUT_PITCH_POS,
        ePROCESSEDINPUT_PITCH_NEG,
        ePROCESSEDINPUT_acceptHeroPressed,
        ePROCESSEDINPUT_declineHeroPressed,
        ePROCESSEDINPUT_playerList,
        ePROCESSEDINPUT_chat,
        ePROCESSEDINPUT_friendChat,
        ePROCESSEDINPUT_squadCommandButtonPressed,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_Comm_Chat_0,
        ePROCESSEDINPUT_Comm_Chat_1,
        ePROCESSEDINPUT_Comm_Chat_2,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_NONE,
        ePROCESSEDINPUT_Comm_Chat_3,
        ePROCESSEDINPUT_Comm_Chat_4,
        ePROCESSEDINPUT_Comm_Chat_5,
        ePROCESSEDINPUT_Comm_Chat_6,
        ePROCESSEDINPUT_Comm_Chat_7,
        ePROCESSEDINPUT_Map,
        ePROCESSEDINPUT_MAX,
    },
};

// GLOBAL: SWBF2 0x0078a148
static unsigned int sIndexToButtonName[76] =
{
    0xe7f720c1u, 0xe6f71f2eu, 0xe5f71d9bu, 0xe4f71c08u,
    0xebf7270du, 0xeaf7257au, 0xe9f723e7u, 0xe8f72254u,
    0xdff71429u, 0xdef71296u, 0xb505fc3au, 0xb605fdcdu,
    0xb305f914u, 0xb405faa7u, 0xb105f5eeu, 0xb205f781u,
    0xaf05f2c8u, 0xb005f45bu, 0xad05efa2u, 0xae05f135u,
    0xaf03b431u, 0xae03b29eu, 0xad03b10bu, 0xac03af78u,
    0xb303ba7du, 0xb203b8eau, 0xb103b757u, 0xb003b5c4u,
    0xa703a799u, 0xa603a606u, 0xa9016c28u, 0xaa016dbbu,
    0xb186059fu, 0xbe97c2e0u, 0xf454413au, 0xcd39a9efu,
    0x14053ad0u, 0x74c623f5u, 0x88e98c65u, 0xd20096c0u,
    0xe4921c01u, 0xac4516e6u, 0x82045768u, 0x8d72f1adu,
    0x5724f21au, 0x3aec917bu, 0xd70e04dbu, 0xe7af60c6u,
    0x9deab321u, 0x9feab647u, 0x7decbf58u, 0x83ecc8cau,
    0xa5ef3ce7u, 0xa3ef39c1u, 0x6412664bu, 0x6a126fbdu,
    0x8c14e3dau, 0x8614da68u, 0x0c0d5e95u, 0x060d5523u,
    0x66c23ee9u, 0x68c2420fu, 0xc6c514a0u, 0xccc51e12u,
    0xad968f70u, 0xae969103u, 0xe4b75d5cu, 0xe5b75eefu,
    0xe6b76082u, 0xe7b76215u, 0xe0b75710u, 0xe1b758a3u,
    0xe2b75a36u, 0xe3b75bc9u, 0x414699eeu, 0x2d715b79u,
};

// FUNCTION: SWBF2 0x00587330
int ScriptCB_GetKeyBoardCmds(lua_State* L)
{
    lua_newtable(LuaHelper::mState);

    unsigned short usColon[] = { ':', ' ', 0 };
    unsigned short usComma[] = { ',', ' ', 0 };
    unsigned short usHash[] = { '#', 0 };
    unsigned short usButton[256];
    unsigned short usAction[256];
    unsigned short usKeys[256];
    unsigned short usKey[256];
    unsigned short usNum[3];

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(0);
    int iMode = pJoystick->GetInputMode();
    int iIndex = 1;

    for (int i = 0; i < 43; ++i)
    {
        memset(usAction, 0, sizeof(usAction));
        memset(usKey, 0, sizeof(usKey));
        memset(usKeys, 0, sizeof(usKeys));
        char acKeys[256] = { 0 };

        int iAction = sKeyboardCmdsPerMode[iMode][i];
        if (iAction == ePROCESSEDINPUT_NONE || iAction == ePROCESSEDINPUT_MAX)
        {
            continue;
        }

        RedLocalizeDB::GetLocalizedString(usAction, sizeof(usAction), GetActionName(iAction, iMode));
        PblUnicode::ustrlcat(usAction, usColon, 256);

        ActionKey& rKey = pProfile->mGameOptions.mKeyboardBindings[iMode][iAction];

        const unsigned char* pucMods = (const unsigned char*)&rKey.mKey[2];
        for (int j = 0; j < 2; ++j)
        {
            char acKey[128] = { 0 };
            unsigned short usRaw = rKey.mKey[j];
            if (!usRaw)
            {
                continue;
            }

            if (usRaw <= 0xFF)
            {
                pJoystick->GetKeyboard()->pcGetKeyName((unsigned char)usRaw, usKey, 256);
                _wcsupr((wchar_t*)usKey);
            }
            else
            {
                RedLocalizeDB::GetLocalizedString(usButton, sizeof(usButton),
                                                  GetButtonName(ActionKeyToRawInput(usRaw)));
                unsigned char ucMod = pucMods[j];
                if (ucMod)
                {
                    PblUnicode::ustrlcpy(usKey, usHash, 512);
                    usNum[0] = (unsigned short)(ucMod + '1');
                    usNum[1] = ' ';
                    usNum[2] = 0;
                    PblUnicode::ustrlcat(usKey, usNum, 512);
                    PblUnicode::ustrlcat(usKey, usButton, 512);
                }
                else
                {
                    PblUnicode::ustrlcpy(usKey, usButton, 512);
                }
            }

            if (PblUnicode::ustrlen(usKeys))
            {
                PblUnicode::ustrlcat(usKeys, usComma, 256);
            }
            strlcat(acKeys, acKey, 256);
            PblUnicode::ustrlcat(usKeys, usKey, 256);
        }

        if (!usKeys[0])
        {

            RedLocalizeDB::GetLocalizedString(usKey, sizeof(usKey), 0x6054a42du);
            PblUnicode::ustrlcat(usKeys, usKey, 256);
        }

        lua_newtable(LuaHelper::mState);
        LuaHelper::TableWriteVal("actionStr", usAction);
        memset(usAction, 0, sizeof(usAction));
        _strupr(acKeys);
        LuaHelper::TableWriteVal("keyStr", usKeys);
        lua_rawseti(LuaHelper::mState, -2, iIndex);
        ++iIndex;
    }

    lua_pushstring(LuaHelper::mState, "ifs_opt_pckeyboard_listbox_contents");
    lua_insert(LuaHelper::mState, -2);
    lua_settable(LuaHelper::mState, LUA_GLOBALSINDEX);
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x005876e0
unsigned int GetButtonName(int iButton)
{
    if ((unsigned int)iButton >= 76)
    {
        return 0;
    }
    return sIndexToButtonName[iButton];
}

// FUNCTION: SWBF2 0x005876f0
unsigned int GetActionName(int iAction, int iMode)
{
    switch (iAction)
    {
    case ePROCESSEDINPUT_primaryFireButtonDown:
        if (iMode == 3)
        {
            return 0xce22c384u;
        }
        if (iMode == 4)
        {
            return 0x4813f4c9u;
        }
        return 0x21ecf289u;
    case ePROCESSEDINPUT_secondaryFireButtonDown:
        return iMode == 3 ? 0x47d420e9u : 0x95067ca9u;
    case ePROCESSEDINPUT_sprintButtonDown:
        if (iMode > 0)
        {
            if (iMode <= 2)
            {
                return 0x9468cb58u;
            }
            if (iMode == 3)
            {
                return 0xeb047957u;
            }
        }
        return 0x7efc5a41u;
    case ePROCESSEDINPUT_jumpButtonDown:
        if (iMode == 2)
        {
            return 0xd97027fdu;
        }
        if (iMode == 3)
        {
            return 0x5fb1e4fcu;
        }
        return 0xa9f6dc69u;
    case ePROCESSEDINPUT_crouchButtonDown:
        if (iMode == 1)
        {
            return 0xbdb6f1f3u;
        }
        if (iMode == 2)
        {
            return 0xc34823b0u;
        }
        return 0x4f860e97u;
    case ePROCESSEDINPUT_zoomButtonDown:
        return iMode == 2 ? 0xfce0ab1du : 0x01808c66u;
    case ePROCESSEDINPUT_viewButtonDown:
        return iMode == 1 ? 0x494c4741u : 0x04c5f38cu;
    case ePROCESSEDINPUT_reloadButtonDown:
        return iMode == 3 ? 0xd05bcadcu : 0x05f51058u;
    case ePROCESSEDINPUT_useButtonPressed:
        if (iMode == 1)
        {
            return 0x96520d93u;
        }
        if (iMode == 2)
        {
            return 0x36cda1c2u;
        }
        if (iMode == 4)
        {
            return 0xbb01e263u;
        }
        return 0x58f98b3fu;
    case ePROCESSEDINPUT_squadCommandButtonPressed: return 0x510446aeu;
    case ePROCESSEDINPUT_acceptHeroPressed:         return 0x1f1d54b5u;
    case ePROCESSEDINPUT_declineHeroPressed:        return 0xc22f1315u;
    case ePROCESSEDINPUT_lockTargetButtonPressed:   return 0x2968b3afu;
    case ePROCESSEDINPUT_primaryNextButtonPressed:
        return (iMode > 0 && (iMode <= 2 || iMode == 4)) ? 0x60f5b57fu : 0xcff83738u;
    case ePROCESSEDINPUT_primaryPrevButtonPressed:
        return (iMode > 0 && (iMode <= 2 || iMode == 4)) ? 0xadf4d65fu : 0x5de79e60u;
    case ePROCESSEDINPUT_secondaryNextButtonPressed:
        return iMode == 3 ? 0xda9424a5u : 0x4311c158u;
    case ePROCESSEDINPUT_secondaryPrevButtonPressed: return 0xba2edc80u;
    case ePROCESSEDINPUT_playerList:     return 0x5625eba4u;
    case ePROCESSEDINPUT_chat:           return 0xfd43d7cfu;
    case ePROCESSEDINPUT_friendChat:     return 0x4c1553e6u;
    case ePROCESSEDINPUT_Comm_Chat_0:    return 0x1b12d399u;
    case ePROCESSEDINPUT_Comm_Chat_1:    return 0x1a12d206u;
    case ePROCESSEDINPUT_Comm_Chat_2:    return 0x1912d073u;
    case ePROCESSEDINPUT_Comm_Chat_3:    return 0x1812cee0u;
    case ePROCESSEDINPUT_Comm_Chat_4:    return 0x1f12d9e5u;
    case ePROCESSEDINPUT_Comm_Chat_5:    return 0x1e12d852u;
    case ePROCESSEDINPUT_Comm_Chat_6:    return 0x1d12d6bfu;
    case ePROCESSEDINPUT_Comm_Chat_7:    return 0x1c12d52cu;
    case ePROCESSEDINPUT_Map:            return 0xf5b9358du;
    case ePROCESSEDINPUT_TakeScreenshot:
        return iMode == 2 ? 0x502ce55du : 0x98aef712u;
    case ePROCESSEDINPUT_STRAFE_AXIS:
        return iMode == 2 ? 0x10241b37u : 0x72deaafau;
    case ePROCESSEDINPUT_MOVE_AXIS:
        if (iMode == 2)
        {
            return 0x37c51c28u;
        }
        if (iMode == 4)
        {
            return 0x72189cdbu;
        }
        return 0x96139f99u;
    case ePROCESSEDINPUT_TURN_AXIS:
        return iMode == 2 ? 0x8928fdf3u : 0xa2d9a4d1u;
    case ePROCESSEDINPUT_PITCH_AXIS:
        return iMode == 2 ? 0x0378be5cu : 0xca189fa6u;
    case ePROCESSEDINPUT_STRAFE_POS:
        return iMode == 2 ? 0x6790dafcu : 0x1b620c12u;
    case ePROCESSEDINPUT_STRAFE_NEG:
        return iMode == 2 ? 0x83d7e5d5u : 0xd152a071u;
    case ePROCESSEDINPUT_MOVE_POS:
        if (iMode == 2)
        {
            return 0x335b2dcbu;
        }
        if (iMode == 4)
        {
            return 0x88971f89u;
        }
        return 0xdaadd13du;
    case ePROCESSEDINPUT_MOVE_NEG:
        if (iMode == 2)
        {
            return 0xd381684fu;
        }
        if (iMode == 4)
        {
            return 0x77270a8au;
        }
        return 0x92ebb24bu;
    case ePROCESSEDINPUT_TURN_POS:
        return iMode == 2 ? 0xce820494u : 0x50a56d68u;
    case ePROCESSEDINPUT_TURN_NEG:
        return iMode == 2 ? 0x7ac3d78bu : 0x05718b77u;
    case ePROCESSEDINPUT_PITCH_POS:
        return iMode == 2 ? 0xbd3f9428u : 0xaf1aa3f7u;
    case ePROCESSEDINPUT_PITCH_NEG:
        return iMode == 2 ? 0xf032a54eu : 0x7ded0392u;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00588220
int ScriptCB_RemoveTexture(lua_State* L)
{
    RedTexture::Remove(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    return 0;
}

// FUNCTION: SWBF2 0x00588280
void InitIFace()
{
    for (int i = 0; i < 150; ++i)
    {
        LuaHelper::Register(sIFaceCallbacks[i].pcName, sIFaceCallbacks[i].pfnCallback, 0);
    }
}

// FUNCTION: SWBF2 0x00583aa0
int ScriptCB_IFModel_SetModel(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel())
    {
        Red3DModelElementLite* pModel = static_cast<Red3DModelElementLite*>(pElement);
        if (lua_type(LuaHelper::mState, iArg) == LUA_TLIGHTUSERDATA)
        {
            pModel->SetModel(static_cast<const RedModel*>(lua_topointer(LuaHelper::mState, iArg)));
        }
        else if (lua_isstring(LuaHelper::mState, iArg))
        {
            pModel->SetModel(luaL_checkstring(LuaHelper::mState, iArg));
        }
        else if (lua_type(LuaHelper::mState, iArg) == LUA_TNIL)
        {
            pModel->SetModel(static_cast<const RedModel*>(0));
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005824a0
int ScriptCB_IFText_SetIntercharacterSpacing(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText())
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        float fSpacing = (float)luaL_checknumber(LuaHelper::mState, iArg);
        static_cast<RedTextElement*>(pElement)->SetIntercharacterSpacing((int)fSpacing);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582520
int ScriptCB_IFText_GetIntercharacterSpacing(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true)
        || (!pElement->IsText() && !pElement->IsFlashyText()))
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, static_cast<RedTextElement*>(pElement)->m_iSpacing);
    return 1;
}

// FUNCTION: SWBF2 0x00582650
int ScriptCB_IFText_GetTextBox(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true)
        || (!pElement->IsText() && !pElement->IsFlashyText()))
    {
        return 0;
    }
    unsigned int uiWidth, uiHeight;
    static_cast<RedTextElement*>(pElement)->GetTextBox(uiWidth, uiHeight);
    lua_pushnumber(LuaHelper::mState, (float)(int)uiWidth);
    lua_pushnumber(LuaHelper::mState, (float)(int)uiHeight);
    return 2;
}

// FUNCTION: SWBF2 0x005827c0
int ScriptCB_IFText_SetLeading(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && (pElement->IsText() || pElement->IsFlashyText())
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        static_cast<RedTextElement*>(pElement)->m_leading =
            (short)(unsigned int)luaL_checknumber(LuaHelper::mState, iArg);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00582840
int ScriptCB_IFText_GetLeading(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, true)
        || (!pElement->IsText() && !pElement->IsFlashyText()))
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, static_cast<RedTextElement*>(pElement)->m_leading);
    return 1;
}

// FUNCTION: SWBF2 0x00582ec0
int ScriptCB_IFImage_GetRect(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true)
        || !pElement || !pElement->IsBitmap())
    {
        return 0;
    }
    float fLeft, fTop, fRight, fBottom;
    static_cast<RedBitmapElement*>(pElement)->GetRect(fLeft, fTop, fRight, fBottom);
    lua_pushnumber(LuaHelper::mState, fLeft);
    lua_pushnumber(LuaHelper::mState, fTop);
    lua_pushnumber(LuaHelper::mState, fRight);
    lua_pushnumber(LuaHelper::mState, fBottom);
    return 4;
}

// FUNCTION: SWBF2 0x00583090
int ScriptCB_IFImage_GetTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true)
        || !pElement || !pElement->IsBitmap())
    {
        return 0;
    }
    float fU0, fV0, fU1, fV1;
    static_cast<RedBitmapElement*>(pElement)->GetTexCoords(fU0, fV0, fU1, fV1);
    lua_pushnumber(LuaHelper::mState, fU0);
    lua_pushnumber(LuaHelper::mState, fV0);
    lua_pushnumber(LuaHelper::mState, fU1);
    lua_pushnumber(LuaHelper::mState, fV1);
    return 4;
}

// FUNCTION: SWBF2 0x00583280
int ScriptCB_IFMaskImage_SetMaskTexture(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && pElement->IsMaskedBitmap() && lua_isstring(LuaHelper::mState, iArg))
    {
        static_cast<RedMaskedBitmapElement*>(pElement)->SetMaskTexture(
            PblHash(luaL_checkstring(LuaHelper::mState, iArg)));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583300
int ScriptCB_IFMaskImage_SetMaskTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(4, pElement, iArg, true)
        && pElement->IsMaskedBitmap()
        && lua_isnumber(LuaHelper::mState, iArg)
        && lua_isnumber(LuaHelper::mState, iArg + 1)
        && lua_isnumber(LuaHelper::mState, iArg + 2)
        && LuaHelper::IsNumber(iArg + 3))
    {
        float fU0 = LuaHelper::GetFloat(iArg);
        float fV0 = LuaHelper::GetFloat(iArg + 1);
        float fU1 = LuaHelper::GetFloat(iArg + 2);
        float fV1 = LuaHelper::GetFloat(iArg + 3);
        static_cast<RedBitmapElement*>(pElement)->SetTexCoords(fU0, fV0, fU1, fV1, false);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583400
int ScriptCB_IFMaskImage_GetMaskTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true)
        || !pElement || !pElement->IsMaskedBitmap())
    {
        return 0;
    }
    float fU0, fU1, fV0, fV1;
    static_cast<RedMaskedBitmapElement*>(pElement)->GetMaskTexCoords(fU0, fU1, fV0, fV1);
    lua_pushnumber(LuaHelper::mState, fU0);
    lua_pushnumber(LuaHelper::mState, fU1);
    lua_pushnumber(LuaHelper::mState, fV0);
    lua_pushnumber(LuaHelper::mState, fV1);
    return 4;
}

// FUNCTION: SWBF2 0x00584230
int ScriptCB_IFModel_GetScale(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, static_cast<Red3DModelElement*>(pElement)->m_scale);
    return 1;
}

// FUNCTION: SWBF2 0x00584380
int ScriptCB_IFModel_GetRotation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    const PblQuaternion& rRotation = static_cast<Red3DModelElement*>(pElement)->m_rotation;
    lua_pushnumber(LuaHelper::mState, rRotation._fS);
    lua_pushnumber(LuaHelper::mState, rRotation._fX);
    lua_pushnumber(LuaHelper::mState, rRotation._fY);
    lua_pushnumber(LuaHelper::mState, rRotation._fZ);
    return 4;
}

// FUNCTION: SWBF2 0x00584510
int ScriptCB_IFModel_GetTranslation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    const PblVector3& rTranslation = static_cast<Red3DModelElement*>(pElement)->m_translation;
    lua_pushnumber(LuaHelper::mState, rTranslation.x);
    lua_pushnumber(LuaHelper::mState, rTranslation.y);
    lua_pushnumber(LuaHelper::mState, rTranslation.z);
    return 3;
}

// FUNCTION: SWBF2 0x00584630
int ScriptCB_IFModel_GetDepth(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, static_cast<Red3DModelElementLite*>(pElement)->GetDepth());
    return 1;
}

// FUNCTION: SWBF2 0x00584800
int ScriptCB_IFModel_SetAnimationLooping(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && pElement->IsModel() && lua_isnumber(LuaHelper::mState, iArg))
    {
        float fLoop = (float)luaL_checknumber(LuaHelper::mState, iArg);
        static_cast<Red3DModelElement*>(pElement)->mAnimator->mLooping = (int)fLoop != 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584780
int ScriptCB_IFModel_UpdateAnimation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true)
        && pElement->IsModel() && lua_isnumber(LuaHelper::mState, iArg))
    {
        static_cast<Red3DModelElement*>(pElement)->mAnimator->Update(
            (float)luaL_checknumber(LuaHelper::mState, iArg));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584100
int ScriptCB_IFModel_AttachModel(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(2, pElement, iArg, true)
        && pElement->IsModel()
        && lua_type(LuaHelper::mState, iArg) == LUA_TLIGHTUSERDATA
        && lua_isstring(LuaHelper::mState, iArg + 1))
    {
        Red3DModelElement* pAttached = static_cast<Red3DModelElement*>(
            const_cast<void*>(lua_topointer(LuaHelper::mState, iArg)));
        Red3DModelElement* pModel = static_cast<Red3DModelElement*>(pElement);
        pModel->m_AttachedObj = pAttached;
        pModel->m_uiHardpointHash = PblTEMPHash(
            luaL_checkstring(LuaHelper::mState, iArg + 1));
        pAttached->m_bAmAttached = true;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584a50
int ScriptCB_IsPAL(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, 0.0);
    return 1;
}

// FUNCTION: SWBF2 0x00586900
int ScriptCB_IsEnableLeaderBoard(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, 0);
    return 1;
}

// FUNCTION: SWBF2 0x00586610
int ScriptCB_LeaderBoardDownloadDone(lua_State* L)
{
    int iState = StatsManager::GetState();
    int iResult = iState == 0 || iState == 6 ? -1 : iState == 2 ? 1 : 0;
    lua_pushnumber(LuaHelper::mState, (float)iResult);
    return 1;
}

// FUNCTION: SWBF2 0x00586670
int ScriptCB_SetLeaderBoardID(lua_State* L)
{
    luaL_checknumber(LuaHelper::mState, 1);
    luaL_checknumber(LuaHelper::mState, 2);
    return 0;
}

// FUNCTION: SWBF2 0x005867a0
int ScriptCB_EnumerateLeaderBoardDone(lua_State* L)
{
    int iState = StatsManager::GetState();
    int iResult = iState == 0 || iState == 6 ? -1 : iState == 2 ? 1 : 0;
    lua_pushnumber(LuaHelper::mState, (float)iResult);
    return 1;
}

// FUNCTION: SWBF2 0x005882c0
int ScriptCB_SetInputRepeat(lua_State* L)
{
    GuiManager::SetInputRepeat(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x00585530
int ScriptCB_SetIgnoreInputs(lua_State* L)
{
    float fDelay = (float)luaL_checknumber(LuaHelper::mState, 1);
    GuiManager::sStartupDelay = (int)fDelay;
    return 0;
}

// FUNCTION: SWBF2 0x00585b10
int ScriptCB_GetErrorLevel(lua_State* L)
{
    lua_pushnumber(LuaHelper::mState, (float)(int)GuiManager::Instance()->mErrorLevel);
    return 1;
}

// FUNCTION: SWBF2 0x00588250
int ScriptCB_ShowLoadDisplay(lua_State* L)
{
    if (lua_toboolean(LuaHelper::mState, 1))
    {
        LoadDisplay::sInstance->Begin();
    }
    else
    {
        LoadDisplay::sInstance->End();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584a70
int ScriptCB_IsFileExist(lua_State* L)
{
    PblFile file;
    if (lua_gettop(LuaHelper::mState) != 1)
    {
        return 0;
    }
    if (lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checkstring(LuaHelper::mState, 1);
        char acFileName[256];
        LoadUtil::MakeFullName(pcName, LoadUtil::FILE_TYPE_NONE, acFileName, sizeof(acFileName), false);
        lua_pushnumber(LuaHelper::mState, PblFile::Exists(acFileName) ? 1.0 : 0.0);
    }
    return 1;
}

// FUNCTION: SWBF2 0x005853d0
int ScriptCB_IsScreenInStack(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }
    const char* pcScreen = luaL_checkstring(LuaHelper::mState, 1);
    lua_pushboolean(LuaHelper::mState, GuiManager::Instance()->IsScreenInStack(pcScreen));
    return 1;
}

// FUNCTION: SWBF2 0x00585760
int ScriptCB_IsPropertyPlayingNow(lua_State* L)
{
    const char* pcProperty = luaL_checkstring(LuaHelper::mState, 1);
    lua_pushboolean(LuaHelper::mState, GameMovie::PropertiesPlaying(pcProperty, true));
    return 1;
}

// FUNCTION: SWBF2 0x00585720
int ScriptCB_StopMovie(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    GameMovie::Stop();
    return 0;
}

// FUNCTION: SWBF2 0x00585ac0
int ScriptCB_CloseErrorBox(lua_State* L)
{
    bool bOk = lua_toboolean(LuaHelper::mState, 1) != 0;
    GuiManager* pGui = GuiManager::Instance();
    pGui->CloseError(bOk, false);
    NetSetup::HandleError(bOk);
    return 0;
}

// FUNCTION: SWBF2 0x00585a20
int ScriptCB_OpenErrorBox(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 2
        && lua_isnumber(LuaHelper::mState, 1)
        && lua_isstring(LuaHelper::mState, 2))
    {
        float fSeverity = (float)luaL_checknumber(LuaHelper::mState, 1);
        const char* pcMessage = luaL_checkstring(LuaHelper::mState, 2);
        GuiManager::Instance()->OpenError((GuiManager::eErrorSeverity)(int)fSeverity,
            reinterpret_cast<const unsigned short*>(pcMessage), 0, fnErrorBoxOk);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00587cc0
void TableWriteColor(const char* pcName, const RedColorValue& color)
{
    lua_pushstring(LuaHelper::mState, pcName);
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("red", color.r);
    LuaHelper::TableWriteVal("green", color.g);
    LuaHelper::TableWriteVal("blue", color.b);
    lua_rawset(LuaHelper::mState, -3);
}

// FUNCTION: SWBF2 0x00587d20
void TableWriteDirection(const char* pcName, const PblVector3& direction)
{
    lua_pushstring(LuaHelper::mState, "direction");
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("x", direction.x);
    LuaHelper::TableWriteVal("y", direction.y);
    LuaHelper::TableWriteVal("z", direction.z);
    lua_rawset(LuaHelper::mState, -3);
}

// FUNCTION: SWBF2 0x00587a30
int ScriptCB_EnableCursor(lua_State* L)
{
    float fVisible = (float)luaL_checknumber(LuaHelper::mState, 1);
    FLRenderer::EnableCursor((int)fVisible != 0);
    return 0;
}

// FUNCTION: SWBF2 0x00586660
int ScriptCB_CancelLeaderBoardPlayerData(lua_State* L)
{
    StatsManager::Stop();
    return 0;
}

// FUNCTION: SWBF2 0x00583550
int ScriptCB_IFBorder_GetRect(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement || !pElement->IsBorderElement())
    {
        return 0;
    }
    float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
    static_cast<RedBorderElement*>(pElement)->GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
    lua_pushnumber(LuaHelper::mState, fLeft);
    lua_pushnumber(LuaHelper::mState, fTop);
    lua_pushnumber(LuaHelper::mState, fRight);
    lua_pushnumber(LuaHelper::mState, fBottom);
    lua_pushnumber(LuaHelper::mState, fBorderX);
    lua_pushnumber(LuaHelper::mState, fBorderY);
    return 4;
}

// FUNCTION: SWBF2 0x005837e0
int ScriptCB_IFBorder_GetTexCoords(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(0, pElement, iArg, true) || !pElement || !pElement->IsBorderElement())
    {
        return 0;
    }
    float fU0, fV0, fU1, fV1, fBorderU, fBorderV;
    static_cast<RedBorderElement*>(pElement)->GetBorderTexCoords(fU0, fV0, fU1, fV1, fBorderU, fBorderV);
    lua_pushnumber(LuaHelper::mState, fU0);
    lua_pushnumber(LuaHelper::mState, fV0);
    lua_pushnumber(LuaHelper::mState, fU1);
    lua_pushnumber(LuaHelper::mState, fV1);
    lua_pushnumber(LuaHelper::mState, fBorderU);
    lua_pushnumber(LuaHelper::mState, fBorderV);
    return 4;
}

// FUNCTION: SWBF2 0x00583cf0
int ScriptCB_IFModel_SetAnimation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(2, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    Red3DModelElement* pModel = static_cast<Red3DModelElement*>(pElement);

    float fTransitionTime = 0.0f;
    if (lua_isnumber(LuaHelper::mState, iArg + 1))
    {
        fTransitionTime = (float)luaL_checknumber(LuaHelper::mState, iArg + 1);
    }

    if (lua_isstring(LuaHelper::mState, iArg))
    {
        const char* pcAnim = luaL_checklstring(LuaHelper::mState, iArg, 0);
        GetNormalAnimator(pModel)->SetAnimation(PblTEMPHash(pcAnim), fTransitionTime);
    }
    else if (lua_type(LuaHelper::mState, iArg) == LUA_TTABLE)
    {
        char acUpper[80];
        char acLower[80];
        lua_pushvalue(LuaHelper::mState, iArg);
        LuaHelper::TableReadVal("upper", acUpper, sizeof(acUpper), false);
        LuaHelper::TableReadVal("lower", acLower, sizeof(acLower), false);
        lua_settop(LuaHelper::mState, -2);
        GetIFaceSoldierAnimator(pModel)->SetUpperLowerAnimations(PblTEMPHash(acUpper), PblTEMPHash(acLower),
                                                                 fTransitionTime);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583e70
int ScriptCB_IFModel_QueueAnimation(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, true) || !pElement->IsModel())
    {
        return 0;
    }
    Red3DModelElement* pModel = static_cast<Red3DModelElement*>(pElement);

    if (lua_isstring(LuaHelper::mState, iArg))
    {
        pModel->QueueAnimation(PblTEMPHash(luaL_checklstring(LuaHelper::mState, iArg, 0)));
    }
    else if (lua_type(LuaHelper::mState, iArg) == LUA_TTABLE)
    {
        char acUpper[80];
        char acLower[80];
        lua_pushvalue(LuaHelper::mState, iArg);
        LuaHelper::TableReadVal("upper", acUpper, sizeof(acUpper), false);
        LuaHelper::TableReadVal("lower", acLower, sizeof(acLower), false);
        lua_settop(LuaHelper::mState, -2);
        GetIFaceSoldierAnimator(pModel)->QueueUpperLowerAnimations(PblTEMPHash(acUpper), PblTEMPHash(acLower));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00583f90
int ScriptCB_IFModel_SetAnimationBanks(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (!Util_GetRedInterfaceElement(1, pElement, iArg, false) || !pElement->IsModel())
    {
        return 0;
    }

    memset(sAnimationBankHashes, 0, sizeof(sAnimationBankHashes));
    int iNumArgs = lua_gettop(LuaHelper::mState);
    for (int i = 0; iArg <= iNumArgs; ++iArg, ++i)
    {
        if (i >= 5)
        {
            return 0;
        }
        sAnimationBankHashes[i] = PblHash(luaL_optlstring(LuaHelper::mState, iArg, "", 0));
    }
    static_cast<Red3DModelElement*>(pElement)->SetAnimationBanks(sAnimationBankHashes);
    return 0;
}

// FUNCTION: SWBF2 0x00584080
int ScriptCB_IFModel_SetAnimationTime(lua_State* L)
{
    RedInterfaceElement* pElement;
    int iArg;
    if (Util_GetRedInterfaceElement(1, pElement, iArg, true) && pElement->IsModel()
        && lua_isnumber(LuaHelper::mState, iArg))
    {
        static_cast<Red3DModelElement*>(pElement)->SetAnimationTime(
            (float)luaL_checknumber(LuaHelper::mState, iArg));
    }
    return 0;
}

// FUNCTION: SWBF2 0x00584f70
int ScriptCB_AddIFMaskImage(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && !pCurCreateObject && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        RedMaskedBitmapElement* pBitmap = new RedMaskedBitmapElement(pcName);
        pBitmap->Enable(true);
        pBitmap->SetRect(0.0f, 0.0f, 100.0f, 100.0f);
        Util_AddToHierarchy(pBitmap, pcName);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585790
int ScriptCB_PlayInGameMovie(lua_State* L)
{
    GameSoundEngine::StopAll();
    GameSoundEngine::Update(0.0001f);
    FLInputManager::s_instance.StopAllRumble();
    if (g_InGameMoviePlayerState)
    {
        g_InGameMovieQueue.Add(PblHash(luaL_checklstring(LuaHelper::mState, 2, 0)));
        return 0;
    }

    const char* pcFile;
    switch (RedLocalizeDB::GetLanguageID())
    {
    case eLANG_FRENCH: pcFile = "ingamefr.mvs"; break;
    case eLANG_GERMAN: pcFile = "ingamegr.mvs"; break;
    default: pcFile = "ingame.mvs"; break;
    }
    GameMovie::StartInGameMoviePlay(pcFile, luaL_checklstring(LuaHelper::mState, 2, 0));
    return 0;
}

// FUNCTION: SWBF2 0x00585850
int ScriptCB_StopInGameMovie(lua_State* L)
{
    StopInGameMoviePlay();
    return 0;
}

// FUNCTION: SWBF2 0x00585b50
int ScriptCB_GetConsoleCmds(lua_State* L)
{
    lua_newtable(LuaHelper::mState);
    const RedCommandConsole::Group* pGroup = RedCommandConsole::FindGroup("debugmenu");
    if (pGroup)
    {
        int iIndex = 0;
        for (PblList<RedCommandConsole::Command>::Itor it(pGroup->GetCommandList()); it; ++it)
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("ShowStr", it->GetName());
            lua_rawseti(LuaHelper::mState, -2, ++iIndex);
        }
    }
    LuaHelper::SetGlobal("gConsoleCmdList");
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x00585c00
int ScriptCB_DoConsoleCmd(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1) && !NetSetup::InNetGame())
    {
        const char* pcCmd = luaL_checklstring(LuaHelper::mState, 1, 0);
        char acLine[64];
        strlcpy(acLine, "debugmenu", 62);
        strlcat(acLine, ".", 62);
        strlcat(acLine, pcCmd, 62);
        RedCommandConsole::Process(RedConsole::Instance(), acLine);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00585c90
void LeaderBoard_FilloutTableData(LeaderBoardStatsT& stats)
{
    char acBuffer[256];
    unsigned short usBuffer[258];

    const float fAccuracy = stats.shotsFired ? (float)stats.shotsHit / (float)stats.shotsFired : 0.0f;
    _snprintf(acBuffer, 256, "%d %%%%", (int)(fAccuracy * 100.0f));
    PblUnicode::ustrncnv(usBuffer, acBuffer, 256);
    LuaHelper::TableWriteVal("accuracy", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.criticalshots);
    LuaHelper::TableWriteVal("headShots", usBuffer);

    int iTime = stats.livingStreak;
    int iMinutes = iTime % 3600 / 60;
    _snprintf(acBuffer, 256, "%02d:%02d:%02d", iTime / 3600, iMinutes, iTime - iMinutes * 60);
    PblUnicode::ustrncnv(usBuffer, acBuffer, 256);
    LuaHelper::TableWriteVal("livingStreak", usBuffer);

    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.narrowEscapes);
    LuaHelper::TableWriteVal("narrowEscapes", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.kills);
    LuaHelper::TableWriteVal("kills", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.deaths);
    LuaHelper::TableWriteVal("deaths", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.captures);
    LuaHelper::TableWriteVal("captures", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.vehiclesDestroyed);
    LuaHelper::TableWriteVal("vehiclesDestroyed", usBuffer);

    __time64_t lastPlayed = stats.lastPlayed;
    const struct tm* pTime = _localtime64(&lastPlayed);
    if (pTime && lastPlayed > 0)
    {
        _snprintf(acBuffer, 256, "%04d-%02d-%02d %02d:%02d:%02d", pTime->tm_year + 1900, pTime->tm_mon + 1,
                  pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
        PblUnicode::ustrncnv(usBuffer, acBuffer, 256);
        LuaHelper::TableWriteVal("lastGamePlayed", usBuffer);
    }

    iTime = stats.totalPlayingTime;
    iMinutes = iTime % 3600 / 60;
    _snprintf(acBuffer, 256, "%02d:%02d:%02d", iTime / 3600, iMinutes, iTime - iMinutes * 60);
    PblUnicode::ustrncnv(usBuffer, acBuffer, 512);
    LuaHelper::TableWriteVal("totalTimePlayed", usBuffer);

    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.wins);
    LuaHelper::TableWriteVal("wins", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.losses);
    LuaHelper::TableWriteVal("losses", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.tankBuster);
    LuaHelper::TableWriteVal("tankBuster", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.killingSpree);
    LuaHelper::TableWriteVal("killingSpree", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.camper);
    LuaHelper::TableWriteVal("camper", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.survivalist);
    LuaHelper::TableWriteVal("survivalist", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.traitor);
    LuaHelper::TableWriteVal("traitor", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.publicEnemy);
    LuaHelper::TableWriteVal("publicEnemy", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.meat);
    LuaHelper::TableWriteVal("meat", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.deadEye);
    LuaHelper::TableWriteVal("deadEye", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.pointman);
    LuaHelper::TableWriteVal("pointMan", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.points);
    LuaHelper::TableWriteVal("points", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.hero_points);
    LuaHelper::TableWriteVal("hero_points", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.game_start);
    LuaHelper::TableWriteVal("game_start", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.game_finish);
    LuaHelper::TableWriteVal("game_finish", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.game_rank);
    LuaHelper::TableWriteVal("game_rank", usBuffer);

    _snprintf(acBuffer, 256, "%I64d", stats.rating);
    PblUnicode::ustrncnv(usBuffer, acBuffer, 256);
    LuaHelper::TableWriteVal("experience", usBuffer);
    double dSkill = 0.0;
    if (stats.totalPlayingTime > 0)
    {
        dSkill = (double)stats.rating / (double)stats.totalPlayingTime;
    }
    _snprintf(acBuffer, 256, "%0.2f", dSkill);
    PblUnicode::ustrncnv(usBuffer, acBuffer, 256);
    LuaHelper::TableWriteVal("skill", usBuffer);

    if (RedLocalizeDB::Find(stats.favoriteClassHash, false))
    {
        LuaHelper::TableWriteVal("favoriteClass", RedLocalizeDB::Find(stats.favoriteClassHash, false));
    }
    if (RedLocalizeDB::Find(stats.favoriteVehicleHash, false))
    {
        LuaHelper::TableWriteVal("favoriteVehicle", RedLocalizeDB::Find(stats.favoriteVehicleHash, false));
    }
    if (RedLocalizeDB::Find(stats.favoriteWeaponHash, false))
    {
        LuaHelper::TableWriteVal("favoriteWeapon", RedLocalizeDB::Find(stats.favoriteWeaponHash, false));
    }

    PblUnicode::ustrncnv(usBuffer, stats.nemesis, 256);
    LuaHelper::TableWriteVal("nemesis", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.nemesisDeaths);
    LuaHelper::TableWriteVal("nemesisDeaths", usBuffer);
    PblUnicode::ustrncnv(usBuffer, stats.bait, 256);
    LuaHelper::TableWriteVal("bait", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.baitKills);
    LuaHelper::TableWriteVal("baitKills", usBuffer);
}

// FUNCTION: SWBF2 0x00586410
int ScriptCB_GetLeaderBoardPlayerData(lua_State* L)
{
    DoPeriodicLuaGC(true);
    LeaderBoardPlayerStatsT stats;
    StatsManager::ReadStatsGetResults(0, &stats);
    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal("leaderboarddetails_data");
    DoPeriodicLuaGC(false);

    lua_newtable(LuaHelper::mState);
    unsigned short usBuffer[256];
    PblUnicode::ustrncnv(usBuffer, stats.username, 256);
    LuaHelper::TableWriteVal("name", usBuffer);
    PblUnicode::usnprintf(usBuffer, 256, "%d", stats.level, 256);
    LuaHelper::TableWriteVal("level", usBuffer);
    if (RedLocalizeDB::Find(stats.titleHash, false))
    {
        LuaHelper::TableWriteVal("title", RedLocalizeDB::Find(stats.titleHash, false));
    }

    lua_pushstring(LuaHelper::mState, "overall");
    lua_newtable(LuaHelper::mState);
    LeaderBoard_FilloutTableData(stats.overallStats);
    lua_rawset(LuaHelper::mState, -3);
    LuaHelper::SetGlobal("leaderboarddetails_data");
    DoPeriodicLuaGC(false);
    return 0;
}

// FUNCTION: SWBF2 0x00586590
int ScriptCB_DownloadLeaderBoardData(lua_State* L)
{
    const char* pcName = luaL_optlstring(LuaHelper::mState, 1, 0, 0);
    luaL_checknumber(LuaHelper::mState, 2);
    unsigned __int64 uId;
    if (Lua_Callbacks::GetServicePlayerID(pcName, &uId) || StatsManager::FindServicePlayerID())
    {
        StatsManager::Stop();
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, -1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x005866a0
int ScriptCB_ResetLeaderBoardPlayerData(lua_State* L)
{
    unsigned __int64 uId;
    if (Lua_Callbacks::GetServicePlayerID(luaL_optlstring(LuaHelper::mState, 1, 0, 0), &uId) && StatsManager::ResetPlayer())
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, -1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00586700
int ScriptCB_EnumerateLeaderBoard(lua_State* L)
{
    unsigned __int64 uId;
    if (!Lua_Callbacks::GetServicePlayerID(luaL_optlstring(LuaHelper::mState, 1, 0, 0), &uId))
    {
        lua_pushnumber(LuaHelper::mState, -1.0f);
        return 1;
    }
    luaL_checknumber(LuaHelper::mState, 2);
    luaL_checknumber(LuaHelper::mState, 3);
    luaL_checknumber(LuaHelper::mState, 4);
    luaL_checknumber(LuaHelper::mState, 5);
    luaL_checknumber(LuaHelper::mState, 6);
    StatsManager::Stop();
    return 0;
}

// FUNCTION: SWBF2 0x005867f0
int ScriptCB_GetEnumerateLeaderBoardData(lua_State* L)
{
    const char* pcScreen = GuiManager::Instance()->GetCurrentScreen();
    if (!*pcScreen)
    {
        return 0;
    }

    DoPeriodicLuaGC(true);
    lua_gettop(LuaHelper::mState);
    lua_pushnil(LuaHelper::mState);
    LuaHelper::SetGlobal("leaderboard_listbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);
    LuaHelper::SetGlobal("leaderboard_listbox_contents");
    DoPeriodicLuaGC(false);
    LuaHelper::CallTableProcWThis(pcScreen, "RepaintListbox");
    lua_pushnumber(LuaHelper::mState, 0.0f);
    lua_pushnumber(LuaHelper::mState, (float)StatsManager::GetLeaderBoardId());
    return 2;
}

// FUNCTION: SWBF2 0x00586920
int ScriptCB_SetSunlight(lua_State* L)
{
    RedAmbientLight& rAmbient = RedInterfaceScreen::s_lightingState.ambientLight;
    const RedColorValue& rTop = RedColorValue::BLACK;
    const RedColorValue& rBottom = RedColorValue::BLACK;
    rAmbient.SetTopColor(rTop);
    rAmbient.SetBottomColor(rBottom);
    RedColorValue mean;
    mean.r = (rTop.r + rBottom.r) * 0.5f;
    mean.g = (rTop.g + rBottom.g) * 0.5f;
    mean.b = (rTop.b + rBottom.b) * 0.5f;
    mean.a = (rTop.a + rBottom.a) * 0.5f;
    rAmbient.SetColor(mean);

    RedColorValue sunColor(255.0f, 255.0f, 255.0f, 1.0f);
    RedInterfaceScreen::s_directionalLight[0].SetDirection(PblVector3(1.0f, -1.0f, -1.0f));
    RedInterfaceScreen::s_directionalLight[0].SetColor(sunColor);
    return 1;
}

// FUNCTION: SWBF2 0x00586b20
int ScriptCB_ResetControl(lua_State* L)
{
    RawControllerInputs* pJoystick =
        FLInputManager::s_instance.GetPlayerJoystick(GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport));
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    pProfile->mGameOptions.ResetBindings(pJoystick->GetInputMode());
    pProfile->mGameOptions.mFlip[pJoystick->GetInputMode()] = pJoystick->GetInputMode() == 2;
    pProfile->ApplySettings();
    pProfile->UpdateProfCRC();
    return 0;
}

// GLOBAL: SWBF2 0x01e5771c

float gEscapeHoldTime;
// GLOBAL: SWBF2 0x01faaa7c

bool gBindingSlotToggle[43];

namespace
{

inline unsigned char* KeyDevices(ActionKey& key)
{
    return (unsigned char*)&key.mKey[2];
}

inline int PickBindingSlot(const ActionKey& key, int iAction)
{
    if (!key.mKey[0])
    {
        return 0;
    }
    if (!key.mKey[1])
    {
        return 1;
    }
    const int iSlot = gBindingSlotToggle[iAction] != 0;
    gBindingSlotToggle[iAction] = !gBindingSlotToggle[iAction];
    return iSlot;
}
}

// FUNCTION: SWBF2 0x00586b70
int ScriptCB_SetBinding(lua_State* L)
{
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(GuiManager::mCurViewport);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    const unsigned int uiMode = pJoystick->GetInputMode();
    int iIndex = (int)(float)luaL_checknumber(LuaHelper::mState, 1) - 1;
    const float fDeltaTime = (float)luaL_checknumber(LuaHelper::mState, 2);
    for (int i = 0; i <= iIndex; ++i)
    {
        const int iCmd = sKeyboardCmdsPerMode[uiMode][i];
        if (iCmd == ePROCESSEDINPUT_NONE || iCmd == ePROCESSEDINPUT_MAX)
        {
            ++iIndex;
        }
    }
    const int iAction = sKeyboardCmdsPerMode[uiMode][iIndex];
    ActionKey* pKeys = pProfile->mGameOptions.mKeyboardBindings[uiMode];
    ActionKey& key = pKeys[iAction];

    for (int i = 0; i < 144; ++i)
    {
        const unsigned char ucKey = (unsigned char)RawControllerInputs::IndexToRedKey(i);
        if (!ucKey)
        {
            continue;
        }
        if (pJoystick->GetKeyboard()->IsKeyDown(ucKey))
        {
            lua_pushnumber(LuaHelper::mState, 1.0f);
            for (int iSlot = 0; iSlot < 2; ++iSlot)
            {
                if (key.mKey[iSlot] == ucKey)
                {
                    gEscapeHoldTime = 0.0f;
                    return 1;
                }
            }
            if (ucKey == DIK_ESCAPE)
            {
                float fHeld = gEscapeHoldTime;
                if (fHeld == 0.0f)
                {
                    fHeld = 0.0001f;
                }
                gEscapeHoldTime = fHeld + fDeltaTime;
                if (gEscapeHoldTime < 1.0f)
                {
                    lua_settop(LuaHelper::mState, -2);
                    return 0;
                }
                for (int iSlot = 0; iSlot < 2; ++iSlot)
                {
                    key.mKey[iSlot] = 0;
                    KeyDevices(key)[iSlot] = 0;
                }
                gEscapeHoldTime = 0.0f;
                FLInputManager::s_instance.GetPlayerJoystick(GuiManager::mCurViewport)
                    ->SetKeyBindingsFromMode(pKeys, uiMode);
                return 1;
            }

            for (int iOther = 0; iOther < 43; ++iOther)
            {
                if (iOther == iAction)
                {
                    continue;
                }
                for (int iSlot = 0; iSlot < 2; ++iSlot)
                {
                    if (pKeys[iOther].mKey[iSlot] == ucKey)
                    {
                        pKeys[iOther].mKey[iSlot] = 0;
                    }
                }
            }
            key.mKey[PickBindingSlot(key, iAction)] = ucKey;
            FLInputManager::s_instance.GetPlayerJoystick(GuiManager::mCurViewport)
                ->SetKeyBindingsFromMode(pKeys, uiMode);
            pProfile->NoteBindingChange();
            pProfile->ApplySettings();
            pProfile->UpdateProfCRC();
            gEscapeHoldTime = 0.0f;
            return 1;
        }
        if (ucKey == DIK_ESCAPE && gEscapeHoldTime > 0.0f)
        {
            lua_pushnumber(LuaHelper::mState, 1.0f);
            gEscapeHoldTime = 0.0f;
            return 1;
        }
    }

    unsigned char* const pRaw = (unsigned char*)pJoystick;
    for (unsigned char ucDevice = 0; ucDevice < 10; ++ucDevice)
    {
        void* const* ppDevices = (void* const*)(pRaw + 0x25d4);
        if (ucDevice && !ppDevices[ucDevice])
        {
            continue;
        }
        const int iEnd = ucDevice ? 64 : 76;
        const float* pfValue = (const float*)(pRaw + 0x14) + ucDevice * 76;
        const float* pfLast = (const float*)(pRaw + 0xbf4) + ucDevice * 76;

        for (int iInput = ppDevices[ucDevice] ? 0 : 64; iInput < iEnd; ++iInput)
        {
            unsigned short usOpposite = 0;
            const float fValue = pfValue[iInput];
            switch (iInput)
            {
            case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55:
            case 56: case 57: case 58: case 59: case 60: case 61: case 62: case 63:

                if (fValue <= 0.4f || fValue - pfLast[iInput] <= 0.03f)
                {
                    continue;
                }
                usOpposite = RawInputToActionKey(
                    (unsigned char)(int)((float)iInput + ((iInput & 1) ? -1.0f : 1.0f)));
                break;
            case 64:
            case 65:
                if (fabsf(fValue) <= 0.5f)
                {
                    continue;
                }
                break;
            default:
                if (fValue <= 0.4f || fValue - pfLast[iInput] <= 0.03f)
                {
                    continue;
                }
                break;
            }

            lua_pushnumber(LuaHelper::mState, 1.0f);
            unsigned short usKey = RawInputToActionKey((unsigned char)iInput);
            unsigned short usOther = usOpposite;
            if (pJoystick->GetFlipForMode(uiMode) && usOpposite && iAction == ePROCESSEDINPUT_PITCH_AXIS)
            {
                usOther = usKey;
                usKey = usOpposite;
            }

            for (int iSlot = 0; iSlot < 2; ++iSlot)
            {
                if (key.mKey[iSlot] == usKey && KeyDevices(key)[iSlot] == ucDevice)
                {
                    gEscapeHoldTime = 0.0f;
                    return 1;
                }
            }
            for (int iOther = 0; iOther < 43; ++iOther)
            {
                if (iOther == iAction)
                {
                    continue;
                }
                for (int iSlot = 0; iSlot < 2; ++iSlot)
                {
                    const unsigned short usBound = pKeys[iOther].mKey[iSlot];
                    if ((usBound == usKey ||
                         (usOther && usBound == usOther &&
                          (iOther == ePROCESSEDINPUT_PITCH_AXIS || iOther == ePROCESSEDINPUT_TURN_AXIS ||
                           iOther == ePROCESSEDINPUT_MOVE_AXIS || iOther == ePROCESSEDINPUT_STRAFE_AXIS))) &&
                        KeyDevices(pKeys[iOther])[iSlot] == ucDevice)
                    {
                        pKeys[iOther].mKey[iSlot] = 0;
                        KeyDevices(pKeys[iOther])[iSlot] = 0;
                    }
                }
            }
            if (usOther)
            {
                if (iAction == ePROCESSEDINPUT_PITCH_AXIS || iAction == ePROCESSEDINPUT_TURN_AXIS ||
                    iAction == ePROCESSEDINPUT_MOVE_AXIS || iAction == ePROCESSEDINPUT_STRAFE_AXIS)
                {
                    for (int iOther = 0; iOther < 43; ++iOther)
                    {
                        if (iOther == iAction)
                        {
                            continue;
                        }
                        for (int iSlot = 0; iSlot < 2; ++iSlot)
                        {
                            if (pKeys[iOther].mKey[iSlot] == usOther && KeyDevices(pKeys[iOther])[iSlot] == ucDevice)
                            {
                                pKeys[iOther].mKey[iSlot] = 0;
                                KeyDevices(pKeys[iOther])[iSlot] = 0;
                            }
                        }
                    }
                }
                for (int iSlot = 0; iSlot < 2; ++iSlot)
                {
                    if (key.mKey[iSlot] == usOther && KeyDevices(key)[iSlot] == ucDevice)
                    {
                        key.mKey[iSlot] = 0;
                        KeyDevices(key)[iSlot] = 0;
                    }
                }
            }

            const int iSlot = PickBindingSlot(key, iAction);
            key.mKey[iSlot] = usKey;
            KeyDevices(key)[iSlot] = ucDevice;
            FLInputManager::s_instance.GetPlayerJoystick(GuiManager::mCurViewport)
                ->SetKeyBindingsFromMode(pKeys, uiMode);
            pProfile->NoteBindingChange();
            pProfile->ApplySettings();
            pProfile->UpdateProfCRC();
            gEscapeHoldTime = 0.0f;
            return 1;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00587290
int ScriptCB_GetActionFromIdx(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    int iIndex = (int)(float)luaL_checknumber(LuaHelper::mState, 1) - 1;
    int iMode = FLInputManager::s_instance.GetPlayerJoystick(PauseMenu::GetPausingViewport())->GetInputMode();
    for (int i = 0; i <= iIndex; ++i)
    {
        int iAction = sKeyboardCmdsPerMode[iMode][i];
        if (iAction == ePROCESSEDINPUT_NONE || iAction == ePROCESSEDINPUT_MAX)
        {
            ++iIndex;
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)sKeyboardCmdsPerMode[iMode][iIndex]);
    return 1;
}

// FUNCTION: SWBF2 0x00587a60
int ScriptCB_MrMrsEval(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    char acResult[52];
    if (EvaluateCheatCode(luaL_checklstring(LuaHelper::mState, 1, 0), acResult))
    {
        lua_pushstring(LuaHelper::mState, acResult);
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00587ad0
int ScriptCB_GetEntityMesh(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    EntityClass* pClass = EntityClass::Find(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    if (pClass && pClass->IsRtti(EntityGeometryClass::rttiHashEntityGeometryClass))
    {
        const GameModel* pModel = static_cast<EntityGeometryClass*>(pClass)->GetModel();
        if (pModel && pModel->GetRedModel(0))
        {
            lua_pushlightuserdata(LuaHelper::mState, (void*)pModel->GetRedModel(0));
            return 1;
        }
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00587b80
int ScriptCB_GetEntityWeaponMesh(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    EntityClass* pClass = EntityClass::Find(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    if (pClass && pClass->IsRtti(GameObjectClass::rttiHashGameObjectClass))
    {
        GameObjectClass* pObjectClass = static_cast<GameObjectClass*>(pClass);
        if (pObjectClass->GetWeaponCount() > 0)
        {
            const WeaponClass* pWeapon = pObjectClass->GetWeaponClass(0);
            if (pWeapon && pWeapon->mModel)
            {
                lua_pushlightuserdata(LuaHelper::mState, pWeapon->mModel);
                return 1;
            }
        }
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00587c40
int ScriptCB_GetEntityLabel(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    EntityClass* pClass = EntityClass::Find(PblHash(luaL_checklstring(LuaHelper::mState, 1, 0)));
    if (!pClass)
    {
        lua_pushnil(LuaHelper::mState);
        return 1;
    }
    LuaHelper::PushString(pClass->mLabel);
    return 1;
}

// FUNCTION: SWBF2 0x00587d80
int ScriptCB_GetIFaceLighting(lua_State* L)
{
    const RedAmbientLight& rAmbient =
        RedInterfaceScreen::s_lightingState.ambientLight;
    RedColorValue top = rAmbient.GetTopColor();
    RedColorValue bottom = rAmbient.GetBottomColor();
    lua_newtable(LuaHelper::mState);
    lua_pushstring(LuaHelper::mState, "ambient");
    lua_newtable(LuaHelper::mState);
    TableWriteColor("top", top);
    TableWriteColor("bottom", bottom);
    lua_rawset(LuaHelper::mState, -3);

    lua_pushstring(LuaHelper::mState, "directional");
    lua_newtable(LuaHelper::mState);
    for (int i = 0; i < 2; ++i)
    {
        PblVector3 direction;
        RedColorValue color;
        RedInterfaceScreen::GetDirectionalLight(i, direction, color);
        lua_newtable(LuaHelper::mState);
        TableWriteDirection("direction", direction);
        TableWriteColor("color", color);
        lua_rawseti(LuaHelper::mState, -2, i + 1);
    }
    lua_rawset(LuaHelper::mState, -3);
    return 1;
}

// FUNCTION: SWBF2 0x00588000
int ScriptCB_SetIFaceLighting(lua_State* L)
{
    if (lua_isnil(LuaHelper::mState, -1))
    {
        return 0;
    }

    lua_pushstring(LuaHelper::mState, "ambient");
    lua_rawget(LuaHelper::mState, -2);
    if (!lua_isnil(LuaHelper::mState, -1))
    {
        RedAmbientLight& rAmbient = RedInterfaceScreen::s_lightingState.ambientLight;
        RedColorValue top = rAmbient.GetTopColor();
        RedColorValue bottom = rAmbient.GetBottomColor();
        Lua_Callbacks::TableReadColor("top", &top.r);
        Lua_Callbacks::TableReadColor("bottom", &bottom.r);
        rAmbient.SetTopColor(top);
        rAmbient.SetBottomColor(bottom);
        RedColorValue mean;
        mean.r = (top.r + bottom.r) * 0.5f;
        mean.g = (top.g + bottom.g) * 0.5f;
        mean.b = (top.b + bottom.b) * 0.5f;
        mean.a = (top.a + bottom.a) * 0.5f;
        rAmbient.SetColor(mean);
    }
    lua_settop(LuaHelper::mState, -2);

    lua_pushstring(LuaHelper::mState, "directional");
    lua_rawget(LuaHelper::mState, -2);
    if (!lua_isnil(LuaHelper::mState, -1))
    {
        for (int i = 0; i < 2; ++i)
        {
            lua_rawgeti(LuaHelper::mState, -1, i + 1);
            if (!lua_isnil(LuaHelper::mState, -1))
            {
                PblVector3 direction;
                RedColorValue color;
                RedInterfaceScreen::GetDirectionalLight(i, direction, color);
                Lua_Callbacks::TableReadDirection(0, &direction.x);
                Lua_Callbacks::TableReadColor("color", &color.r);
                RedInterfaceScreen::s_directionalLight[i].SetDirection(direction);
                RedInterfaceScreen::s_directionalLight[i].SetColor(color);
            }
            lua_settop(LuaHelper::mState, -2);
        }
    }
    lua_settop(LuaHelper::mState, -2);
    return 0;
}

// TEMPLATE: SWBF2 0x005882e0
// Snd::Queue<unsigned int,12>::Add

}

// GLOBAL: SWBF2 0x01ec610c
// SYNTHETIC: SWBF2 0x004041d0
// `dynamic initializer for 'sDefaultFont''
unsigned int sDefaultFont = RedFont::GAMEFONT_SMALL;
