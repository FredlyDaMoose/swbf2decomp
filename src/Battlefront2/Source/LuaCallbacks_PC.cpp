#include <stdio.h>
#include <string.h>

#include "LuaCallbacks_PC.h"
#include "LuaHelper.h"
#include "FLGuiManager.h"
#include "FLInputManager.h"
#include "PlayerProfile.h"
#include "ColorControl.h"
#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

#include "RedEngineFL/input/RedKeyboard.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"

// GLOBAL: SWBF2 0x01e5794c
int markMouseX;
// GLOBAL: SWBF2 0x01e57964
int markMouseY;

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00599580
int ScriptCB_SetReticuleTransparency(lua_State*)
{
    luaL_checknumber(LuaHelper::mState, 1);
    return 0;
}

// FUNCTION: SWBF2 0x005995a0
int ScriptCB_GetReticuleTransparency(lua_State*)
{
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x0059b1b0
int ScriptCB_SetMouseMark(lua_State*)
{
    const MouseCoords& coords = FLInputManager::s_instance.GetPlayerGuiInputs(0)->mMouseCoords;
    markMouseX = coords.mPosX;
    markMouseY = coords.mPosY;
    return 0;
}

// FUNCTION: SWBF2 0x0059b1e0
int ScriptCB_ResetMouseMark(lua_State*)
{
    markMouseX = -1;
    markMouseY = -1;
    return 0;
}

// FUNCTION: SWBF2 0x0059b200
int ScriptCB_CheckMouseMark(lua_State*)
{
    const MouseCoords& coords = FLInputManager::s_instance.GetPlayerGuiInputs(0)->mMouseCoords;
    lua_pushboolean(LuaHelper::mState,
                    markMouseX != coords.mPosX || markMouseY != coords.mPosY);
    return 1;
}

// FUNCTION: SWBF2 0x00599330
int ScriptCB_SetMouseSensitivity(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(
        GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport));
    float fSensitivity = luaL_checknumber(LuaHelper::mState, 1);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mGameOptions.mMouseSensitivity[pJoystick->GetInputMode()] != fSensitivity)
    {
        pProfile->mGameOptions.mMouseSensitivity[pJoystick->GetInputMode()] = fSensitivity;
        pProfile->UpdateProfCRC();
        pProfile->ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005993c0
int ScriptCB_SetJoySensitivity(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    int iSensitivity = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iAxis = (int)luaL_checknumber(LuaHelper::mState, 2);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        RawControllerInputs* pJoystick = FLInputManager::s_instance.GetPlayerJoystick(
            GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport));
        unsigned char* pValue = 0;
        if (iAxis == 0)
        {
            pValue = &pProfile->mGameOptions.mJoystickSensitivityX[pJoystick->GetInputMode()];
        }
        if (iAxis == 1)
        {
            pValue = &pProfile->mGameOptions.mJoystickSensitivityY[pJoystick->GetInputMode()];
        }
        if (pValue && *pValue != (iAxis == 1 ? iSensitivity : (unsigned char)iSensitivity))
        {
            *pValue = (unsigned char)iSensitivity;
            pProfile->UpdateProfCRC();
            pProfile->ApplySettings();
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00599470
int ScriptCB_SetDeadZone(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    unsigned char iDeadZone = (unsigned char)(int)luaL_checknumber(LuaHelper::mState, 1);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mGameOptions.mDeadZone != iDeadZone)
    {
        pProfile->mGameOptions.mDeadZone = iDeadZone;
        pProfile->UpdateProfCRC();
        pProfile->ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x005994d0
int ScriptCB_IsJoystickEnabled(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    lua_pushboolean(LuaHelper::mState, pProfile ? pProfile->mPCOptions.mJoyEnabled : true);
    return 1;
}

// FUNCTION: SWBF2 0x00599510
int ScriptCB_SetJoystickEnabled(lua_State*)
{
    lua_gettop(LuaHelper::mState);
    bool bEnabled = lua_isnumber(LuaHelper::mState, 1) || lua_isstring(LuaHelper::mState, 1);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mPCOptions.mJoyEnabled != bEnabled)
    {
        pProfile->mPCOptions.mJoyEnabled = bEnabled;
        pProfile->UpdateProfCRC();
        pProfile->ApplySettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059abb0
int ScriptCB_GetProfileJoinIP(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    lua_pushstring(LuaHelper::mState, pProfile ? pProfile->mPCOptions.mJoinIp : "");
    return 1;
}

// FUNCTION: SWBF2 0x0059abe0
int ScriptCB_SetProfileJoinIP(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        const char* pcIp = luaL_checklstring(LuaHelper::mState, 1, 0);
        memset(pProfile->mPCOptions.mJoinIp, 0, sizeof(pProfile->mPCOptions.mJoinIp));
        strlcpy(pProfile->mPCOptions.mJoinIp, pcIp, sizeof(pProfile->mPCOptions.mJoinIp));
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059ac50
int ScriptCB_ResetPCVideoOptionsToDefault(lua_State*)
{
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        pProfile->mPCOptions.ResetToDefault();
        pProfile->ApplySettings();
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00599bd0
int ScriptCB_GetIdealResolution(lua_State*)
{
    int iWidth, iHeight;
    RedAutoSettings::GetIdealResolution(iWidth, iHeight);
    lua_pushnumber(LuaHelper::mState, (float)iWidth);
    lua_pushnumber(LuaHelper::mState, (float)iHeight);
    return 2;
}

// FUNCTION: SWBF2 0x00599c30
int ScriptCB_GetCustomResolution(lua_State*)
{
    int iPreset = 0;
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        iPreset = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    }
    int iWidth, iHeight;
    RedAutoSettings::GetCustomPresetResolution(iPreset, iWidth, iHeight);
    lua_pushnumber(LuaHelper::mState, (float)iWidth);
    lua_pushnumber(LuaHelper::mState, (float)iHeight);
    return 2;
}

// FUNCTION: SWBF2 0x0059ac80
int ScriptCB_IsVideoModeSupported(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((unsigned)(iArgs - 2) > 2 || !lua_isnumber(LuaHelper::mState, 1) ||
        !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    int iWidth = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iHeight = (int)luaL_checknumber(LuaHelper::mState, 2);
    int iType = 0, iQuality = 0;
    if (iArgs > 2)
    {
        if (lua_isnumber(LuaHelper::mState, 3))
        {
            iType = (int)luaL_checknumber(LuaHelper::mState, 3);
        }
        if (iArgs > 3 && lua_isnumber(LuaHelper::mState, 4))
        {
            iQuality = (int)luaL_checknumber(LuaHelper::mState, 4);
        }
    }
    lua_pushboolean(LuaHelper::mState, RedVideoConfig::VidMode_Usable(
        iWidth, iHeight, (D3DMULTISAMPLE_TYPE)iType, iQuality));
    return 1;
}

// FUNCTION: SWBF2 0x0059ad70
int ScriptCB_IsMultisamplingModeForcedFromDriver(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) > 0)
    {
        return 0;
    }
    lua_pushboolean(LuaHelper::mState, RedVideoConfig::VidMode_IsDriverMultisampling());
    return 1;
}

// FUNCTION: SWBF2 0x0059adb0
int ScriptCB_IsMultisamplingModeSupported(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((unsigned)(iArgs - 1) > 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iType = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iQuality = iArgs == 2 && lua_isnumber(LuaHelper::mState, 2)
        ? (int)luaL_checknumber(LuaHelper::mState, 2) : 0;
    lua_pushboolean(LuaHelper::mState, RedVideoConfig::VidMode_MultisamplingModeUseable(
        (D3DMULTISAMPLE_TYPE)iType, iQuality));
    return 1;
}

// FUNCTION: SWBF2 0x0059ae50
int ScriptCB_GetMaxScreenSizeForMultisamplingMode(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((unsigned)(iArgs - 1) > 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iType = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iQuality = iArgs == 2 && lua_isnumber(LuaHelper::mState, 2)
        ? (int)luaL_checknumber(LuaHelper::mState, 2) : 0;
    int iWidth = 0, iHeight = 0;
    if (RedVideoConfig::VidMode_GetMaxScreenSizeForMultisamplingMode(
        (D3DMULTISAMPLE_TYPE)iType, iQuality, iWidth, iHeight))
    {
        lua_pushnumber(LuaHelper::mState, (float)iWidth);
        lua_pushnumber(LuaHelper::mState, (float)iHeight);
    }
    else { lua_pushnil(LuaHelper::mState); lua_pushnil(LuaHelper::mState); }
    return 2;
}

// FUNCTION: SWBF2 0x0059af60
int ScriptCB_GetMaxMultisamplingModeForScreenSize(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) != 2 || !lua_isnumber(LuaHelper::mState, 1) ||
        !lua_isnumber(LuaHelper::mState, 2))
    {
        return 0;
    }
    int iWidth = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iHeight = (int)luaL_checknumber(LuaHelper::mState, 2);
    D3DMULTISAMPLE_TYPE eType;
    unsigned long ulQuality;
    if (RedVideoConfig::VidMode_GetMaxMultisamplingModeForScreenSize(iWidth, iHeight, eType, ulQuality))
    {
        lua_pushnumber(LuaHelper::mState, (float)eType);
        lua_pushnumber(LuaHelper::mState, (float)ulQuality);
    }
    else { lua_pushnil(LuaHelper::mState); lua_pushnil(LuaHelper::mState); }
    return 2;
}

// FUNCTION: SWBF2 0x0059b060
int ScriptCB_GetNumMultisamplingQualityModes(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) > 0)
    {
        return 0;
    }
    lua_pushnumber(LuaHelper::mState, (float)RedVideoConfig::VidMode_GetNumMultiSampleQualityModes());
    return 1;
}

// FUNCTION: SWBF2 0x0059b0a0
int ScriptCB_GetMultisamplingModeDescription(lua_State*)
{
    int iArgs = lua_gettop(LuaHelper::mState);
    if ((unsigned)(iArgs - 1) > 1 || !lua_isnumber(LuaHelper::mState, 1))
    {
        return 0;
    }
    int iType = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iQuality = iArgs == 2 && lua_isnumber(LuaHelper::mState, 2)
        ? (int)luaL_checknumber(LuaHelper::mState, 2) : 0;
    const char* pcDesc = RedVideoConfig::VidMode_GetMultiSampleDescription((D3DMULTISAMPLE_TYPE)iType, iQuality);
    if (pcDesc && *pcDesc && *pcDesc != 'U')
    {
        lua_pushstring(LuaHelper::mState, pcDesc);
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0059a070
int ScriptCB_GetCustomVideoOptions(lua_State*)
{
    int iPreset = 0;
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        iPreset = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    }
    const RenderOptionsT opts = *RedAutoSettings::GetCustomPreset(iPreset);
    lua_pushnumber(LuaHelper::mState, (float)(opts.mGraphicsTier + 1));
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, (float)(opts.textureDetail + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.textureQuality + 1));
    lua_pushnumber(LuaHelper::mState, opts.mFarSceneRangeFactor);
    lua_pushnumber(LuaHelper::mState, opts.mNearSceneFadeFactor);
    lua_pushnumber(LuaHelper::mState, (float)(opts.waterQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.terrainQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.particleQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.shadowQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.specularQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.mEnableHDR + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.bumpMapping + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.motionBlur + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.distortion + 1));
    return 16;
}

// FUNCTION: SWBF2 0x0059a320
int ScriptCB_GetAutodetectPCVideoOptions(lua_State*)
{
    const RenderOptionsT opts = RedAutoSettings::GetRenderOptions();
    if (ColorControl::s_pInstance)
    {
        ColorControl::s_pInstance->SetLevel((ControlT)0, opts.mGammaContrast);
        ColorControl::s_pInstance->SetLevel((ControlT)1, opts.mGammaBrightness);
        ColorControl::s_pInstance->Update(0.0f);
    }
    lua_pushnumber(LuaHelper::mState, (float)(opts.masterQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.mGraphicsTier + 1));
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, (float)(opts.textureDetail + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.textureQuality + 1));
    lua_pushnumber(LuaHelper::mState, opts.mGammaBrightness);
    lua_pushnumber(LuaHelper::mState, opts.mGammaContrast);
    lua_pushnumber(LuaHelper::mState, opts.mFarSceneRangeFactor);
    lua_pushnumber(LuaHelper::mState, opts.mNearSceneFadeFactor);
    lua_pushnumber(LuaHelper::mState, (float)(opts.waterQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.terrainQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.particleQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.shadowQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.specularQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.mEnableHDR + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.bumpMapping + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.motionBlur + 1));
    lua_pushnumber(LuaHelper::mState, (float)(opts.distortion + 1));
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 21;
}

static int ScriptCB_EnableCursor(lua_State* L);

// GLOBAL: SWBF2 0x007e8270
static const CallbackEntryT sPCCallbacks[36] =
{
    { "ScriptCB_GetMouseSensitivity", ScriptCB_GetMouseSensitivity },
    { "ScriptCB_GetJoySensitivity", ScriptCB_GetJoySensitivity },
    { "ScriptCB_GetDeadZone", ScriptCB_GetDeadZone },
    { "ScriptCB_SetMouseSensitivity", ScriptCB_SetMouseSensitivity },
    { "ScriptCB_SetJoySensitivity", ScriptCB_SetJoySensitivity },
    { "ScriptCB_SetDeadZone", ScriptCB_SetDeadZone },
    { "ScriptCB_IsJoystickEnabled", ScriptCB_IsJoystickEnabled },
    { "ScriptCB_SetJoystickEnabled", ScriptCB_SetJoystickEnabled },
    { "ScriptCB_SetReticuleTransparency", ScriptCB_SetReticuleTransparency },
    { "ScriptCB_GetReticuleTransparency", ScriptCB_GetReticuleTransparency },
    { "ScriptCB_FillResolutionTable", ScriptCB_FillResolutionTable },
    { "ScriptCB_FillMultisampleTable", ScriptCB_FillMultisampleTable },
    { "ScriptCB_SetResolution", ScriptCB_SetResolution },
    { "ScriptCB_GetIdealResolution", ScriptCB_GetIdealResolution },
    { "ScriptCB_GetCustomResolution", ScriptCB_GetCustomResolution },
    { "ScriptCB_GetPCVideoOptions", ScriptCB_GetPCVideoOptions },
    { "ScriptCB_GetAutodetectPCVideoOptions", ScriptCB_GetAutodetectPCVideoOptions },
    { "ScriptCB_GetCustomVideoOptions", ScriptCB_GetCustomVideoOptions },
    { "ScriptCB_SetPCVideoOptions", ScriptCB_SetPCVideoOptions },
    { "ScriptCB_SetPCBrightnessContrast", ScriptCB_SetPCBrightnessContrast },
    { "ScriptCB_IsPCVideoFixedFunction", ScriptCB_IsPCVideoFixedFunction },
    { "ScriptCB_GetProfileJoinIP", ScriptCB_GetProfileJoinIP },
    { "ScriptCB_SetProfileJoinIP", ScriptCB_SetProfileJoinIP },
    { "ScriptCB_ResetPCVideoOptionsToDefault", ScriptCB_ResetPCVideoOptionsToDefault },
    { "IsVideoModeSupported", ScriptCB_IsVideoModeSupported },
    { "IsMultisamplingModeForcedFromDriver", ScriptCB_IsMultisamplingModeForcedFromDriver },
    { "IsMultisamplingModeSupported", ScriptCB_IsMultisamplingModeSupported },
    { "GetMaxScreenSizeForMultisamplingMode", ScriptCB_GetMaxScreenSizeForMultisamplingMode },
    { "GetMaxMultisamplingModeForScreenSize", ScriptCB_GetMaxMultisamplingModeForScreenSize },
    { "GetNumMultisamplingQualityModes", ScriptCB_GetNumMultisamplingQualityModes },
    { "GetMultisamplingModeDescription", ScriptCB_GetMultisamplingModeDescription },
    { "ScriptCB_SetMouseMark", ScriptCB_SetMouseMark },
    { "ScriptCB_ResetMouseMark", ScriptCB_ResetMouseMark },
    { "ScriptCB_CheckMouseMark", ScriptCB_CheckMouseMark },
    { "ScriptCB_GetKeyboardPCFlags", ScriptCB_GetKeyboardPCFlags },
    { "ScriptCB_EnableCursor", ScriptCB_EnableCursor },
};

// FUNCTION: SWBF2 0x00599170
int ScriptCB_GetMouseSensitivity(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    float fSensitivity = 0.0f;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        int iMode = FLInputManager::s_instance.GetPlayerJoystick(
            GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport))->GetInputMode();
        fSensitivity = pProfile->mGameOptions.mMouseSensitivity[iMode];
    }
    lua_pushnumber(LuaHelper::mState, fSensitivity);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 100.0f);
    return 3;
}

// FUNCTION: SWBF2 0x00599200
int ScriptCB_GetJoySensitivity(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    int iAxis = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iSensitivity = 0;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        int iMode = FLInputManager::s_instance.GetPlayerJoystick(
            GuiManager::GetJoystickFromViewport(GuiManager::mCurViewport))->GetInputMode();
        if (iAxis == 0)
        {
            iSensitivity = pProfile->mGameOptions.mJoystickSensitivityX[iMode];
        }
        else
        {
            iSensitivity = pProfile->mGameOptions.mJoystickSensitivityY[iMode];
        }
    }
    lua_pushnumber(LuaHelper::mState, (float)iSensitivity);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 10.0f);
    return 3;
}

// FUNCTION: SWBF2 0x005995c0
static void AddResolution(int iIndex, unsigned short const* pusLabel, int iResX, int iResY)
{
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("label", pusLabel);
    lua_pushstring(LuaHelper::mState, "data");
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("resX", iResX);
    LuaHelper::TableWriteVal("resY", iResY);
    lua_rawset(LuaHelper::mState, -3);
    lua_rawseti(LuaHelper::mState, -2, iIndex);
}

// FUNCTION: SWBF2 0x00599640
static void AddMultisampleMode(int iIndex, unsigned short const* pusLabel, int iType, int iQuality)
{
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("label", pusLabel);
    lua_pushstring(LuaHelper::mState, "data");
    lua_newtable(LuaHelper::mState);
    LuaHelper::TableWriteVal("multisampleType", iType);
    LuaHelper::TableWriteVal("multisampleQuality", iQuality);
    lua_rawset(LuaHelper::mState, -3);
    lua_rawseti(LuaHelper::mState, -2, iIndex);
}

// FUNCTION: SWBF2 0x005996c0
int ScriptCB_FillResolutionTable(lua_State* L)
{
    lua_pushnil(LuaHelper::mState);
    lua_setglobal(LuaHelper::mState, "ifs_opt_pcvideo_reslistbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);
    lua_gettop(LuaHelper::mState);
    lua_gettop(LuaHelper::mState);

    int iMultiSample = lua_isnumber(LuaHelper::mState, 1) ? LuaHelper::GetInt(1) : 0;
    int iMultiSampleQuality = lua_isnumber(LuaHelper::mState, 2) ? LuaHelper::GetInt(2) : 0;

    D3DDISPLAYMODE modes[50];
    unsigned int uiNumModes = RedRenderer::pcGetValidDisplayModes(modes, 50, 800, 600);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();

    int iCurSel = 1;
    int iIndex = 1;
    for (unsigned int i = 0; i < uiNumModes; ++i)
    {
        unsigned short usLabel[256];
        PblUnicode::usnprintf(usLabel, 255, "%d x %d", modes[i].Width, modes[i].Height);
        usLabel[255] = 0;

        if (!RedVideoConfig::VidMode_Usable(modes[i].Width, modes[i].Height,
                                            (D3DMULTISAMPLE_TYPE)iMultiSample,
                                            iMultiSampleQuality))
        {
            continue;
        }

        AddResolution(iIndex, usLabel, modes[i].Width, modes[i].Height);
        if (pProfile
            && modes[i].Width == pProfile->mPCOptions.mResolutionX
            && modes[i].Height == pProfile->mPCOptions.mResolutionY)
        {
            iCurSel = iIndex;
        }
        ++iIndex;
    }

    lua_setglobal(LuaHelper::mState, "ifs_opt_pcvideo_reslistbox_contents");
    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, (float)iCurSel);
    return 1;
}

// FUNCTION: SWBF2 0x005998c0
int ScriptCB_FillMultisampleTable(lua_State* L)
{
    lua_pushnil(LuaHelper::mState);
    lua_setglobal(LuaHelper::mState, "ifs_opt_pcvideo_fsaalistbox_contents");
    DoPeriodicLuaGC(false);
    lua_newtable(LuaHelper::mState);

    int aiType[17];
    int aiQuality[17];
    aiType[0] = D3DMULTISAMPLE_NONE;
    aiQuality[0] = 0;
    int iNumModes = 1;

    lua_gettop(LuaHelper::mState);
    int iWidth = lua_isnumber(LuaHelper::mState, 1) ? LuaHelper::GetInt(1) : 800;
    int iHeight = lua_isnumber(LuaHelper::mState, 2) ? LuaHelper::GetInt(2) : 600;

    int iNumQualities = RedVideoConfig::VidMode_GetNumMultiSampleQualityModes();
    if (iNumQualities > 0)
    {
        for (int q = 0; q < iNumQualities; ++q)
        {
            if (RedVideoConfig::VidMode_Usable(iWidth, iHeight, D3DMULTISAMPLE_NONMASKABLE, q))
            {
                aiType[iNumModes] = D3DMULTISAMPLE_NONMASKABLE;
                aiQuality[iNumModes] = q;
                ++iNumModes;
            }
        }
    }
    else
    {
        for (int t = D3DMULTISAMPLE_2_SAMPLES; t < D3DMULTISAMPLE_16_SAMPLES; ++t)
        {
            if (RedVideoConfig::VidMode_Usable(iWidth, iHeight, (D3DMULTISAMPLE_TYPE)t, 0))
            {
                aiType[iNumModes] = t;
                aiQuality[iNumModes] = 0;
                ++iNumModes;
            }
        }
    }

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    int iCurSel = 1;
    for (int i = 0; i < iNumModes; ++i)
    {
        const int iType = aiType[i];
        unsigned short const* pusLabel;
        unsigned short usDesc[32];
        if (iType == D3DMULTISAMPLE_NONE)
        {
            char acKey[256];
            _snprintf(acKey, 255, "common.off");
            acKey[255] = 0;
            pusLabel = RedLocalizeDB::Find(PblHash(acKey), false);
        }
        else
        {
            PblUnicode::ustrncnv(usDesc,
                                 RedVideoConfig::VidMode_GetMultiSampleDescription(
                                     (D3DMULTISAMPLE_TYPE)iType, aiQuality[i]),
                                 32);
            pusLabel = usDesc;
        }

        if (pProfile && iType == pProfile->mPCOptions.mMultiSample
            && (unsigned long)aiQuality[i] == pProfile->mPCOptions.mMultiSampleQuality)
        {
            iCurSel = i + 1;
        }

        AddMultisampleMode(i + 1, pusLabel, iType, aiQuality[i]);
    }

    lua_setglobal(LuaHelper::mState, "ifs_opt_pcvideo_fsaalistbox_contents");
    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, (float)iCurSel);
    return 1;
}

// FUNCTION: SWBF2 0x00599b50
int ScriptCB_SetResolution(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    int iResX = (int)luaL_checknumber(LuaHelper::mState, 1);
    int iResY = (int)luaL_checknumber(LuaHelper::mState, 2);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile
        && (pProfile->mPCOptions.mResolutionX != iResX
            || pProfile->mPCOptions.mResolutionY != iResY))
    {
        pProfile->mPCOptions.mResolutionX = (unsigned short)iResX;
        pProfile->mPCOptions.mResolutionY = (unsigned short)iResY;
        pProfile->UpdateProfCRC();
    }
    return 0;
}

// FUNCTION: SWBF2 0x00599cc0
int ScriptCB_GetPCVideoOptions(lua_State* L)
{
    int iMasterQuality = 0;
    int iUnknown0038 = 0;
    int iTextureDetail = 0;
    int iTextureQuality = 0;
    float fUnknown0018 = 0.5f;
    float fUnknown001C = 0.5f;
    float fUnknown0024 = 0.5f;
    float fUnknown0020 = 0.5f;
    int iWaterQuality = 0;
    int iTerrainQuality = 0;
    int iParticleQuality = 0;
    int iShadowQuality = 0;
    int iSpecularQuality = 0;
    int iUnknown0028 = 0;
    int iBumpMapping = 0;
    int iMotionBlur = 0;
    int iDistortion = 0;
    int iWindowed = 0;
    int iVSync = 0;

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        const PlayerProfile::tPCOpt& rPC = pProfile->mPCOptions;
        const RenderOptionsT& rOpts = rPC.renderOptions;
        iMasterQuality = rOpts.masterQuality;
        iUnknown0038 = rOpts.mGraphicsTier;
        iTextureDetail = rOpts.textureDetail;
        iTextureQuality = rOpts.textureQuality;
        fUnknown0018 = rOpts.mGammaBrightness;
        fUnknown001C = rOpts.mGammaContrast;
        fUnknown0024 = rOpts.mFarSceneRangeFactor;
        fUnknown0020 = rOpts.mNearSceneFadeFactor;
        iWaterQuality = rOpts.waterQuality;
        iTerrainQuality = rOpts.terrainQuality;
        iParticleQuality = rOpts.particleQuality;
        iShadowQuality = rOpts.shadowQuality;
        iSpecularQuality = rOpts.specularQuality;
        iUnknown0028 = rOpts.mEnableHDR;
        iBumpMapping = rOpts.bumpMapping;
        iMotionBlur = rOpts.motionBlur;
        iDistortion = rOpts.distortion;
        iWindowed = rPC.mWindowed;
        iVSync = rPC.mVSync;

        if (fUnknown0018 < 0.2f)
        {
            fUnknown0018 = 0.2f;
        }
        else if (fUnknown0018 > 0.8f)
        {
            fUnknown0018 = 0.8f;
        }
        if (fUnknown001C < 0.2f)
        {
            fUnknown001C = 0.2f;
        }
        else if (fUnknown001C > 0.8f)
        {
            fUnknown001C = 0.8f;
        }
        if (fUnknown0024 < 0.0f)
        {
            fUnknown0024 = 0.0f;
        }
        else if (fUnknown0024 > 1.0f)
        {
            fUnknown0024 = 1.0f;
        }
        if (fUnknown0020 < 0.0f)
        {
            fUnknown0020 = 0.0f;
        }
        else if (fUnknown0020 > 1.0f)
        {
            fUnknown0020 = 1.0f;
        }
    }

    lua_pushnumber(LuaHelper::mState, (float)(iMasterQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iUnknown0038 + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iTextureDetail + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iTextureQuality + 1));
    lua_pushnumber(LuaHelper::mState, fUnknown0018);
    lua_pushnumber(LuaHelper::mState, fUnknown001C);
    lua_pushnumber(LuaHelper::mState, fUnknown0024);
    lua_pushnumber(LuaHelper::mState, fUnknown0020);
    lua_pushnumber(LuaHelper::mState, (float)(iWaterQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iTerrainQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iParticleQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iShadowQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iSpecularQuality + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iUnknown0028 + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iBumpMapping + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iMotionBlur + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iDistortion + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iWindowed + 1));
    lua_pushnumber(LuaHelper::mState, (float)(iVSync + 1));
    return 19;
}

// FUNCTION: SWBF2 0x0059a5e0
int ScriptCB_SetPCVideoOptions(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 21)
    {
        return 0;
    }

    RenderOptionsT opts;
    opts.masterQuality = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    opts.mGraphicsTier = (int)luaL_checknumber(LuaHelper::mState, 2) - 1;
    opts.textureDetail = (QualityT)((int)luaL_checknumber(LuaHelper::mState, 3) - 1);
    opts.textureQuality = (TextureQualityT)((int)luaL_checknumber(LuaHelper::mState, 4) - 1);
    D3DMULTISAMPLE_TYPE eMultiSample = (D3DMULTISAMPLE_TYPE)(int)luaL_checknumber(LuaHelper::mState, 5);
    unsigned long ulMultiSampleQuality = (int)luaL_checknumber(LuaHelper::mState, 6);
    opts.waterQuality = (QualityT)((int)luaL_checknumber(LuaHelper::mState, 7) - 1);
    opts.terrainQuality = (QualityT)((int)luaL_checknumber(LuaHelper::mState, 8) - 1);
    opts.particleQuality = (QualityT)((int)luaL_checknumber(LuaHelper::mState, 9) - 1);
    opts.shadowQuality = (Quality2T)((int)luaL_checknumber(LuaHelper::mState, 10) - 1);
    opts.specularQuality = (Quality2T)((int)luaL_checknumber(LuaHelper::mState, 11) - 1);
    opts.mEnableHDR = (int)luaL_checknumber(LuaHelper::mState, 12) - 1 != 0;
    opts.bumpMapping = (int)luaL_checknumber(LuaHelper::mState, 13) - 1 != 0;
    opts.motionBlur = (int)luaL_checknumber(LuaHelper::mState, 14) - 1 != 0;
    opts.distortion = (int)luaL_checknumber(LuaHelper::mState, 15) - 1 != 0;
    bool bWindowed = (int)luaL_checknumber(LuaHelper::mState, 16) - 1 != 0;
    opts.mGammaBrightness = luaL_checknumber(LuaHelper::mState, 17);
    opts.mGammaContrast = luaL_checknumber(LuaHelper::mState, 18);
    opts.mFarSceneRangeFactor = luaL_checknumber(LuaHelper::mState, 19);
    opts.mNearSceneFadeFactor = luaL_checknumber(LuaHelper::mState, 20);
    bool bVSync = (int)luaL_checknumber(LuaHelper::mState, 21) - 1 != 0;

    if (opts.mGammaBrightness < 0.2)
    {
        opts.mGammaBrightness = 0.2;
    }
    else if (opts.mGammaBrightness > 0.80000001)
    {
        opts.mGammaBrightness = 0.80000001;
    }
    if (opts.mGammaContrast < 0.2)
    {
        opts.mGammaContrast = 0.2;
    }
    else if (opts.mGammaContrast > 0.80000001)
    {
        opts.mGammaContrast = 0.80000001;
    }
    if (opts.mFarSceneRangeFactor < 0.0f)
    {
        opts.mFarSceneRangeFactor = 0.0f;
    }
    else if (opts.mFarSceneRangeFactor > 1.0f)
    {
        opts.mFarSceneRangeFactor = 1.0f;
    }
    if (opts.mNearSceneFadeFactor < 0.0f)
    {
        opts.mNearSceneFadeFactor = 0.0f;
    }
    else if (opts.mNearSceneFadeFactor > 1.0f)
    {
        opts.mNearSceneFadeFactor = 1.0f;
    }

    RedAutoSettings::_SetGeneration(&opts);

    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (!pProfile)
    {
        return 0;
    }

    PlayerProfile::tPCOpt& rPC = pProfile->mPCOptions;
    RenderOptionsT& rOpts = rPC.renderOptions;
    if (!RedVideoConfig::VidMode_Usable(rPC.mResolutionX, rPC.mResolutionY,
                                        eMultiSample, ulMultiSampleQuality))
    {
        ulMultiSampleQuality = 0;
        eMultiSample = D3DMULTISAMPLE_NONE;
    }

    bool bChanged = false;
    if (opts.masterQuality != rOpts.masterQuality) { rOpts.masterQuality = opts.masterQuality; bChanged = true; }
    if (opts.mGraphicsTier != rOpts.mGraphicsTier) { rOpts.mGraphicsTier = opts.mGraphicsTier; bChanged = true; }
    if (opts.textureDetail != rOpts.textureDetail) { rOpts.textureDetail = opts.textureDetail; bChanged = true; }
    if (opts.textureQuality != rOpts.textureQuality) { rOpts.textureQuality = opts.textureQuality; bChanged = true; }
    if (eMultiSample != rPC.mMultiSample) { rPC.mMultiSample = eMultiSample; bChanged = true; }
    if (ulMultiSampleQuality != rPC.mMultiSampleQuality) { rPC.mMultiSampleQuality = ulMultiSampleQuality; bChanged = true; }
    if (opts.waterQuality != rOpts.waterQuality) { rOpts.waterQuality = opts.waterQuality; bChanged = true; }
    if (opts.terrainQuality != rOpts.terrainQuality) { rOpts.terrainQuality = opts.terrainQuality; bChanged = true; }
    if (opts.particleQuality != rOpts.particleQuality) { rOpts.particleQuality = opts.particleQuality; bChanged = true; }
    if (opts.shadowQuality != rOpts.shadowQuality) { rOpts.shadowQuality = opts.shadowQuality; bChanged = true; }
    if (opts.specularQuality != rOpts.specularQuality) { rOpts.specularQuality = opts.specularQuality; bChanged = true; }
    if (opts.mEnableHDR != rOpts.mEnableHDR) { rOpts.mEnableHDR = opts.mEnableHDR; bChanged = true; }
    if (opts.bumpMapping != rOpts.bumpMapping) { rOpts.bumpMapping = opts.bumpMapping; bChanged = true; }
    if (opts.motionBlur != rOpts.motionBlur) { rOpts.motionBlur = opts.motionBlur; bChanged = true; }
    if (opts.distortion != rOpts.distortion) { rOpts.distortion = opts.distortion; bChanged = true; }
    if (bWindowed != rPC.mWindowed) { rPC.mWindowed = bWindowed; bChanged = true; }
    if (bVSync != rPC.mVSync) { rPC.mVSync = bVSync; bChanged = true; }
    if (opts.mGammaBrightness != rOpts.mGammaBrightness) { rOpts.mGammaBrightness = opts.mGammaBrightness; bChanged = true; }
    if (opts.mGammaContrast != rOpts.mGammaContrast) { rOpts.mGammaContrast = opts.mGammaContrast; bChanged = true; }
    if (opts.mFarSceneRangeFactor != rOpts.mFarSceneRangeFactor) { rOpts.mFarSceneRangeFactor = opts.mFarSceneRangeFactor; bChanged = true; }
    if (opts.mNearSceneFadeFactor != rOpts.mNearSceneFadeFactor) { rOpts.mNearSceneFadeFactor = opts.mNearSceneFadeFactor; bChanged = true; }

    if (bChanged)
    {
        pProfile->UpdateProfCRC();
        rPC.ApplyPostLoadSettings();
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059ab00
int ScriptCB_SetPCBrightnessContrast(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 2)
    {
        float fBrightness = luaL_checknumber(LuaHelper::mState, 1);
        float fContrast = luaL_checknumber(LuaHelper::mState, 2);
        if (ColorControl::s_pInstance)
        {
            ColorControl::s_pInstance->SetLevel(COLORCONTROL_GAMMA_CONTRAST, fContrast);
            ColorControl::s_pInstance->SetLevel(COLORCONTROL_GAMMA_BRIGHTNESS, fBrightness);
            ColorControl::s_pInstance->Update(0.0f);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x0059ab80
int ScriptCB_IsPCVideoFixedFunction(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED);
    return 1;
}

// FUNCTION: SWBF2 0x0059b170
void InitPC()
{
    for (int i = 0; i < 36; ++i)
    {
        LuaHelper::Register(sPCCallbacks[i].pcName, sPCCallbacks[i].pfnCallback, 0);
    }
}

}

// FUNCTION: SWBF2 0x005992c0
int Lua_Callbacks::ScriptCB_GetDeadZone(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    int iDeadZone = 5;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile)
    {
        iDeadZone = pProfile->mGameOptions.mDeadZone;
    }
    lua_pushnumber(LuaHelper::mState, (float)iDeadZone);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    lua_pushnumber(LuaHelper::mState, 10.0f);
    return 3;
}

// FUNCTION: SWBF2 0x0059b250
int Lua_Callbacks::ScriptCB_GetKeyboardPCFlags(lua_State* L)
{
    bool bShift = false;
    bool bCtrl = false;
    RedKeyboard* pKeyboard = FLInputManager::s_instance.GetPlayerJoystick(0)->GetKeyboard();
    if (pKeyboard)
    {
        const unsigned short usFlags = pKeyboard->GetFlags();
        bShift = (usFlags & 2) != 0;
        bCtrl = (usFlags & 1) != 0;
    }
    lua_pushboolean(LuaHelper::mState, bShift);
    lua_pushboolean(LuaHelper::mState, bCtrl);
    return 2;
}

// FUNCTION: SWBF2 0x0059b2b0
int Lua_Callbacks::ScriptCB_EnableCursor(lua_State* L)
{
    bool bEnable = true;
    if (lua_gettop(LuaHelper::mState) > 0)
    {
        bEnable = lua_toboolean(LuaHelper::mState, 1) != 0;
    }
    FLRenderer::EnableCursor(bEnable);
    return 0;
}
