#pragma once

#include "LuaCallbacks.h"

namespace Lua_Callbacks
{

int ScriptCB_GetMouseSensitivity(lua_State* L);
int ScriptCB_GetJoySensitivity(lua_State* L);
int ScriptCB_SetMouseSensitivity(lua_State* L);
int ScriptCB_SetJoySensitivity(lua_State* L);
int ScriptCB_IsJoystickEnabled(lua_State* L);
int ScriptCB_SetJoystickEnabled(lua_State* L);
int ScriptCB_FillResolutionTable(lua_State* L);
int ScriptCB_FillMultisampleTable(lua_State* L);
int ScriptCB_SetResolution(lua_State* L);
int ScriptCB_GetIdealResolution(lua_State* L);
int ScriptCB_GetPCVideoOptions(lua_State* L);
int ScriptCB_GetAutodetectPCVideoOptions(lua_State* L);
int ScriptCB_SetPCVideoOptions(lua_State* L);
int ScriptCB_SetPCBrightnessContrast(lua_State* L);
int ScriptCB_IsPCVideoFixedFunction(lua_State* L);
int ScriptCB_GetProfileJoinIP(lua_State* L);
int ScriptCB_SetProfileJoinIP(lua_State* L);
int ScriptCB_SetMouseMark(lua_State* L);
int ScriptCB_ResetMouseMark(lua_State* L);
int ScriptCB_CheckMouseMark(lua_State* L);

int ScriptCB_GetDeadZone(lua_State* L);
int ScriptCB_SetDeadZone(lua_State* L);
int ScriptCB_SetReticuleTransparency(lua_State* L);
int ScriptCB_GetReticuleTransparency(lua_State* L);
int ScriptCB_GetCustomResolution(lua_State* L);
int ScriptCB_GetCustomVideoOptions(lua_State* L);
int ScriptCB_ResetPCVideoOptionsToDefault(lua_State* L);
int ScriptCB_IsVideoModeSupported(lua_State* L);
int ScriptCB_IsMultisamplingModeForcedFromDriver(lua_State* L);
int ScriptCB_IsMultisamplingModeSupported(lua_State* L);
int ScriptCB_GetMaxScreenSizeForMultisamplingMode(lua_State* L);
int ScriptCB_GetMaxMultisamplingModeForScreenSize(lua_State* L);
int ScriptCB_GetNumMultisamplingQualityModes(lua_State* L);
int ScriptCB_GetMultisamplingModeDescription(lua_State* L);
int ScriptCB_GetKeyboardPCFlags(lua_State* L);

void InitPC();

}
