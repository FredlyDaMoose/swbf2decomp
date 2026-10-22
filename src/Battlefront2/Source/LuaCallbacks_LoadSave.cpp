#include "LuaCallbacks_LoadSave.h"
#include "LuaHelper.h"
#include "NetSetup.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LuaFL/include/lauxlib.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "Battlefront2/Platform/Pc/pcSaveFile.h"
#include "PlayerProfile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "ZlibFL/zlib.h"
#include "FLGuiManager.h"
#include "GameState.h"
#include "LuaCallbacks_IFace.h"
#include "LuaCallbacks_Net.h"
#include "NetComm.h"
#include "NetGame.h"
#include "PauseMenu.h"
#include "Team.h"

namespace SaveGameManager
{
void StartInitialMemcardCheck(void (*pfnDone)());
void StartPreOp(void (*pfnDone)(), bool bAllowFormat, unsigned int uiCheckFreeBytes,
                unsigned int uiCheckFreeFiles, unsigned short const* pusFailOpStr, bool bSilentFail,
                bool bNoFileList, bool bUseIMCMsg, bool bIsPreLoad);
bool GetSuccess();
bool InMemoryMode();
void SetMemoryMode(bool bMemoryMode);
bool CheckIfOkToExitMemoryMode();
void StartQueue();
void QueueLoad(unsigned short const* pusFilename, unsigned int uiType, void* pDest, unsigned int uiSize);
void QueueSave(unsigned short const* pusFilename, unsigned int uiType, void* pSrc, unsigned int uiSize);
void QueueDelete(unsigned short const* pusFilename, unsigned int uiType, bool bSilent);
void FinishQueue(void (*pfnDone)());
void MarkMemoryCard();
bool CheckMemoryCardMark();
bool IsMemoryCardUnformatted();
bool IsMemoryCardInsertted();
bool IsUniqueFilename(unsigned short const* pusFilename, int iType);

int GetUniqueFilename(unsigned short* pusFilename, int iStart, int iArg3, unsigned int uiArg4);
void ResetFileList();
unsigned short const* GetNextFileName(int iType, int* piArg);
bool CanCreateNewFile(int iType);
bool Input_Accept(int iAnswer);
}

namespace PblUnicode
{

int __cdecl ustrcmp(unsigned short const* pusA, unsigned short const* pusB);
unsigned int __cdecl ustrlcpy(unsigned short* pusDest, unsigned short const* pusSource,
                              unsigned int uiSize);
}

namespace LuaHelper
{

bool CallGlobalProc(char const* pcFunc, bool bArg);
}

namespace PblBuildType { extern unsigned char gCreateMemoryProfile; }
extern int gActiveViewport;
extern bool gInTrainingMission;

namespace RedLocalizeDB { const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing); }

namespace Lua_Callbacks { extern unsigned short sTempReturnString16[256]; }

// GLOBAL: SWBF2 0x007e7370
int s_doInitialMemcardCheck = 1;
// GLOBAL: SWBF2 0x01e57721
unsigned char gProfilesLoaded;
// GLOBAL: SWBF2 0x01ec80d0
// SYNTHETIC: SWBF2 0x004041e0
// `dynamic initializer for 'sSavedPlayerOne''
PlayerProfile::tProfile sSavedPlayerOne;
// GLOBAL: SWBF2 0x01f9c7c0
int savedWhich;
// GLOBAL: SWBF2 0x01f9c7c4
unsigned int sCampaignBufferSize;
// GLOBAL: SWBF2 0x01f9c7c8
unsigned int sMetagameBufferSize;

// GLOBAL: SWBF2 0x01f9c7cc
unsigned int sMissionBufferSize;
// GLOBAL: SWBF2 0x01f9c7d0
unsigned short sMetagameFilename[128];
// GLOBAL: SWBF2 0x01f9c8d0
unsigned char sMetagameBuffer[512];
// GLOBAL: SWBF2 0x01f9cad0
unsigned short sCampaignFilename[128];
// GLOBAL: SWBF2 0x01f9cbd0
unsigned char sCampaignBuffer[256];

// GLOBAL: SWBF2 0x01f9ccd0
unsigned char sMissionBuffer[256];

namespace Lua_Callbacks { static unsigned int SaveLuaItems(lua_State* L, unsigned char* pBuffer, unsigned int iCapacity); }
namespace Lua_Callbacks { static int LoadLuaItems(lua_State* L, unsigned char* pBuffer, unsigned int iSize); }

struct OneSavegame
{
    unsigned short mFilename[128];
    unsigned short mGamename[128];
    unsigned short mGamedate[128];
    bool mIsCurrent;
};

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00588350
int ScriptCB_GetLoginList(lua_State* L)
{
    int iNumArgs = lua_gettop(LuaHelper::mState);
    if (!lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    const char* pcTableName = luaL_checklstring(LuaHelper::mState, 1, 0);
    bool bShowMemory = false;
    if (iNumArgs == 2)
    {
        bShowMemory = lua_toboolean(LuaHelper::mState, 2) != 0;
    }

    lua_newtable(LuaHelper::mState);
    int iCount = 0;

    if (PblBuildType::gCreateMemoryProfile)
    {
        PlayerProfile::SetNumMemoryProfiles(1);
        unsigned short ausName[64];
        PblUnicode::usnprintf(ausName, 64, RedLocalizeDB::Find(0x3802F15B, false), 1);
        PlayerProfile::GetProfile(0)->InitProfile(ausName);
    }

    if (bShowMemory || PblBuildType::gCreateMemoryProfile)
    {
        int iNumFiles = 0;
        SaveGameManager::ResetFileList();
        while (SaveGameManager::GetNextFileName(1, 0))
        {
            ++iNumFiles;
        }

        PlayerProfile::tProfile* pCur = PlayerProfile::GetCurProfile();
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
        int iInList = 0;
        if (pProfile && !pProfile->mbFakeProfile &&
            SaveGameManager::IsUniqueFilename(pProfile->mName, 1))
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("showstr", pProfile->mName);
            LuaHelper::TableWriteVal("memslot", 1);
            lua_rawseti(LuaHelper::mState, -2, 1);
            iInList = 1;
            iCount = 1;
        }

        if (iInList + iNumFiles < 1 && gActiveViewport)
        {
            if (pCur)
            {
                memcpy(pProfile, pCur, sizeof(PlayerProfile::tProfile));
            }

            unsigned short* pusName = pProfile->mName;
            int iUnique = 0;
            bool bClash;
            do
            {
                pProfile->SetTrueProfileName(RedLocalizeDB::Find(0x3802F15B, false));
                iUnique = SaveGameManager::GetUniqueFilename(pusName, iUnique + 1, 0, 0);
                bClash = false;
                for (int i = 0; i < 1; ++i)
                {
                    if (i && !PblUnicode::ustrcmp(pusName, PlayerProfile::GetProfile(i)->mName))
                    {
                        bClash = true;
                        break;
                    }
                }
            } while (bClash);

            pProfile->mbFakeProfile = true;
            pProfile->UpdateProfCRC();
            pProfile->mIsDirty = false;

            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("showstr", pusName);
            LuaHelper::TableWriteVal("memslot", 2);
            lua_rawseti(LuaHelper::mState, -2, ++iCount);
        }
    }

    SaveGameManager::ResetFileList();
    int iArg;
    const unsigned short* pusFile;
    while ((pusFile = SaveGameManager::GetNextFileName(1, &iArg)) != 0)
    {
        lua_newtable(LuaHelper::mState);
        LuaHelper::TableWriteVal("showstr", pusFile);
        LuaHelper::TableWriteVal("memslot", 0);
        lua_rawseti(LuaHelper::mState, -2, ++iCount);
    }

    lua_setglobal(LuaHelper::mState, pcTableName);
    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, (float)(SaveGameManager::InMemoryMode() == 1 ? 1 : 1024));
    return 1;
}

// FUNCTION: SWBF2 0x00588660
int ScriptCB_IsCurProfileDirty(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    bool bArg = lua_toboolean(LuaHelper::mState, 1) != 0;
    if (PlayerProfile::GetDefaultProfileIdx() == 0)
    {
        PlayerProfile::BackupDefaultProfile();
    }
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    lua_pushboolean(LuaHelper::mState, pProfile && pProfile->IsDirty(bArg));
    return 1;
}

// FUNCTION: SWBF2 0x005886d0
int ScriptCB_IsProfileDirty(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    bool bArg = lua_toboolean(LuaHelper::mState, 2) != 0;
    if (PlayerProfile::GetDefaultProfileIdx() == 0)
    {
        PlayerProfile::BackupDefaultProfile();
    }
    unsigned int uiIndex = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    if (uiIndex >= 1)
    {
        return 0;
    }
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(uiIndex);
    lua_pushboolean(LuaHelper::mState,
                    pProfile && pProfile->IsDirty(bArg) && !pProfile->mbFakeProfile);
    return 1;
}

// FUNCTION: SWBF2 0x00588770
int ScriptCB_SetProfileNotDirty(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    unsigned int uiIndex = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    if (uiIndex < 1)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(uiIndex - 1);
        if (pProfile)
        {
            pProfile->UpdateProfCRC();
            pProfile->mIsDirty = false;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005887c0
int ScriptCB_LoadSavePopupResult(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isnumber(LuaHelper::mState, 1))
    {
        int iAnswer = (int)luaL_checknumber(LuaHelper::mState, 1);
        if (iAnswer >= 1 && iAnswer <= 3)
        {
            SaveGameManager::Input_Accept(iAnswer);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x00588820
int ScriptCB_DoInitialMemcardCheck(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, s_doInitialMemcardCheck != 0);
    s_doInitialMemcardCheck = 0;
    return 1;
}

}

// FUNCTION: SWBF2 0x00588850
static void InitialMemcardCheckDone()
{
    LuaHelper::CallGlobalProc("ifs_saveop_InitialMemcardCheckDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00588870
int ScriptCB_StartInitialMemcardCheck(lua_State* L)
{
    SaveGameManager::StartInitialMemcardCheck(InitialMemcardCheckDone);
    return 0;
}

}

// FUNCTION: SWBF2 0x00588880
static void PreOpDone()
{
    LuaHelper::CallGlobalProc("ifs_saveop_PreOpDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x005888a0
int ScriptCB_StartPreOp(lua_State* L)
{
    bool bIsLoad = lua_toboolean(LuaHelper::mState, 1) != 0;
    const char* pcFailOp = luaL_checklstring(LuaHelper::mState, 2, 0);
    bool bSilentFail = lua_toboolean(LuaHelper::mState, 3) != 0;
    bool bNoFileList = lua_toboolean(LuaHelper::mState, 4) != 0;
    bool bNoFreeCheck = lua_toboolean(LuaHelper::mState, 5) != 0;
    bool bUseIMCMsg = lua_toboolean(LuaHelper::mState, 6) != 0;
    bool bIsPreLoad = lua_toboolean(LuaHelper::mState, 7) != 0;

    bool bAllowFormat = false;
    unsigned int uiCheckFreeBytes = 0;
    unsigned int uiCheckFreeFiles = 0;
    if (!bIsLoad)
    {
        bAllowFormat = true;
        uiCheckFreeBytes = 2048;
        uiCheckFreeFiles = 1;
    }
    if (bNoFreeCheck)
    {
        uiCheckFreeBytes = 0;
    }

    SaveGameManager::StartPreOp(PreOpDone, bAllowFormat, uiCheckFreeBytes, uiCheckFreeFiles,
                                (unsigned short const*)pcFailOp, bSilentFail, bNoFileList,
                                bUseIMCMsg, bIsPreLoad);
    return 0;
}

// FUNCTION: SWBF2 0x00588970
int ScriptCB_IsPlayerLoggedIn(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mName[0])
    {
        lua_pushnumber(LuaHelper::mState, (float)PlayerProfile::GetCurProfileIndex() + 1.0f);
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x005889f0
int ScriptCB_GetCurrentProfileName(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mName[0])
    {
        LuaHelper::PushString(pProfile->mName);
        return 1;
    }
    lua_pushstring(LuaHelper::mState, "");
    return 1;
}

// FUNCTION: SWBF2 0x00588a40
int ScriptCB_GetProfileName(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    unsigned int uiIndex = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    if (uiIndex >= 1)
    {
        return 0;
    }
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(uiIndex);
    if (pProfile && pProfile->mName[0])
    {
        LuaHelper::PushString(pProfile->mName);
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00588ab0
int ScriptCB_GetCurrentProfileNetName(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    if (pProfile && pProfile->mName[0])
    {
        LuaHelper::PushString(pProfile->GetName());
        return 1;
    }
    lua_pushnil(LuaHelper::mState);
    return 1;
}

// FUNCTION: SWBF2 0x00588b00
int ScriptCB_GetCurrentPCTitleVersion(lua_State* L)
{
    char acBuffer[256];
    sprintf(acBuffer, "Star Wars Battlefront II v. %s", NetSetup::GetGameVersion());
    lua_pushstring(LuaHelper::mState, acBuffer);
    return 1;
}

// FUNCTION: SWBF2 0x00588b40
int ScriptCB_AddProfile(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    if (!lua_isstring(LuaHelper::mState, 1) || !SaveGameManager::CanCreateNewFile(1))
    {
        lua_pushnil(LuaHelper::mState);
        return 1;
    }

    const char* pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
    int iNum = PlayerProfile::GetNumMemoryProfiles();
    if (SaveGameManager::InMemoryMode() == 1)
    {
        if (iNum > 0)
        {
            memcpy(PlayerProfile::GetProfile(iNum), PlayerProfile::GetProfile(0),
                   sizeof(PlayerProfile::tProfile));
        }
        PlayerProfile::SetNumMemoryProfiles(iNum + 1);
    }

    int iViewport = gActiveViewport;
    PlayerProfile::SetCurProfile(iViewport);
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetCurProfile();
    pProfile->ResetProfile();
    pProfile->InitProfile((unsigned short const*)pcName);
    pProfile->mPCOptions.renderOptions = RedAutoSettings::GetRenderOptions();
    pProfile->UpdateProfCRC();

    PlayerProfile::SetCurProfile(iViewport);
    PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
    PlayerProfile::SetCurProfile(iViewport);

    char acName[130];
    PblUnicode::ustrncnv(acName, (unsigned short const*)pcName, sizeof(acName));
    NetSetup::SetLoginName(iViewport, acName);
    lua_pushnumber(LuaHelper::mState, 1.0f);
    return 1;
}

// FUNCTION: SWBF2 0x00588cb0
int ScriptCB_SwapActiveProfileSlots(lua_State* L)
{
    PlayerProfile::tProfile tmp;
    PlayerProfile::tProfile* pSecond = PlayerProfile::GetProfile(1);
    memcpy(&tmp, pSecond, sizeof(PlayerProfile::tProfile));
    PlayerProfile::tProfile* pFirst = PlayerProfile::GetProfile(0);
    memcpy(pSecond, pFirst, sizeof(PlayerProfile::tProfile));
    memcpy(pFirst, &tmp, sizeof(PlayerProfile::tProfile));
    return 0;
}

}

// FUNCTION: SWBF2 0x00588d20
static void LoadProfileDoneCallback()
{
    bool bSuccess = SaveGameManager::GetSuccess();
    if (bSuccess)
    {
        if (gProfilesLoaded & 1)
        {
            PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
            pProfile->mIsDirty = false;
            pProfile->mbFakeProfile = false;
            pProfile->SanityCheck();
        }
    }
    else
    {
        unsigned int uiCur = PlayerProfile::GetCurProfileIndex();
        int iViewport = gActiveViewport;
        PlayerProfile::ResetAllProfiles();
        PlayerProfile::DuplicateDefaultProfile(iViewport, PlayerProfile::GetDefaultProfileName());
        PlayerProfile::SetCurProfile(uiCur);
        gProfilesLoaded = 0;
    }

    int iViewport = gActiveViewport;
    if (gProfilesLoaded & 1)
    {
        PlayerProfile::SetCurProfile(0);
        PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
        PlayerProfile::SetCurProfile(0);

        if (NetComm::IsLAN())
        {
            PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
            if (pProfile)
            {
                unsigned short* pusName = pProfile->GetTrueProfileName();
                if (pusName && pusName[0])
                {
                    char acName[66];
                    PblUnicode::ustrncnv(acName, pusName, sizeof(acName));
                    NetSetup::SetLoginName(0, acName);
                }
            }
        }
    }
    PlayerProfile::SetCurProfile(iViewport);
    LuaHelper::CallGlobalProc("ifs_saveop_LoadProfileDone", bSuccess);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00588e10
int ScriptCB_StartLoadProfile(lua_State* L)
{
    gProfilesLoaded = 0;
    const char* pcName = 0;
    if (lua_isstring(LuaHelper::mState, 1))
    {
        pcName = luaL_checklstring(LuaHelper::mState, 1, 0);
        gProfilesLoaded |= 1;
    }
    else
    {
        lua_type(LuaHelper::mState, 1);
    }

    if (!SaveGameManager::InMemoryMode())
    {
        SaveGameManager::StartQueue();
        if (pcName)
        {
            SaveGameManager::QueueLoad((unsigned short const*)pcName, 1, PlayerProfile::GetProfile(0),
                                       sizeof(PlayerProfile::tProfile));
        }
        SaveGameManager::FinishQueue(LoadProfileDoneCallback);
    }
    else
    {
        if (pcName)
        {
            PblUnicode::ustrcmp((unsigned short const*)pcName, PlayerProfile::GetProfile(0)->mName);
        }
        gProfilesLoaded = 1;
        PlayerProfile::SetCurProfile(0);
        PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
        PlayerProfile::SetCurProfile(0);
        LoadProfileDoneCallback();
    }

    if (pcName)
    {
        char acName[130];
        PblUnicode::ustrncnv(acName, (unsigned short const*)pcName, sizeof(acName));
        NetSetup::SetLoginName(gActiveViewport, acName);
    }
    return 0;
}

}

// FUNCTION: SWBF2 0x00588f20
static void Make1FakeProfile(int iIndex, unsigned short const* pusName)
{
    if (iIndex >= 1)
    {
        return;
    }

    unsigned int uiCur = PlayerProfile::GetCurProfileIndex();
    PlayerProfile::DuplicateDefaultProfile(iIndex, pusName);
    PlayerProfile::SetCurProfile(iIndex);

    PlayerPrefs::mAllUserPrefs[iIndex].pProfile = PlayerProfile::GetCurProfile();
    PlayerProfile::SetCurProfile(iIndex);

    if (iIndex != gActiveViewport)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(iIndex);
        pProfile->mAccumPoints.ResetToDefault();
        memset(pProfile->mGameOptions.mMedalsGathered, 0, sizeof(pProfile->mGameOptions.mMedalsGathered));
        pProfile->mbFakeProfile = true;
        pProfile->UpdateProfCRC();
        pProfile->mIsDirty = false;
    }

    char acName[66];
    PblUnicode::ustrncnv(acName, pusName, sizeof(acName));
    NetSetup::SetLoginName(iIndex, acName);
    PlayerProfile::SetCurProfile(uiCur);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00588fd0
int ScriptCB_MakeFakeProfiles(lua_State* L)
{
    int iViewport = gActiveViewport;
    if (lua_isstring(LuaHelper::mState, 1))
    {
        Make1FakeProfile(0, (unsigned short const*)luaL_checklstring(LuaHelper::mState, 1, 0));
        return 0;
    }
    if (lua_isnoneornil(LuaHelper::mState, 1) && iViewport)
    {
        PlayerProfile::GetProfile(0)->InitProfile(0);
    }
    return 0;
}

// FUNCTION: SWBF2 0x00589040
int ScriptCB_SetProfileAsDefault(lua_State* L)
{
    PlayerProfile::BackupDefaultProfile();
    return 0;
}

// FUNCTION: SWBF2 0x00589050
int ScriptCB_RestoreDefaultProfile(lua_State* L)
{
    int iViewport = gActiveViewport;
    PlayerProfile::DuplicateDefaultProfile(iViewport, 0);
    PlayerProfile::SetCurProfile(iViewport);
    PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
    PlayerProfile::SetCurProfile(iViewport);
    return 0;
}

// FUNCTION: SWBF2 0x00589080
int ScriptCB_MarkCurrentProfile(lua_State* L)
{
    memcpy(&sSavedPlayerOne, PlayerProfile::GetProfile(0), sizeof(PlayerProfile::tProfile));
    return 0;
}

// FUNCTION: SWBF2 0x005890a0
int ScriptCB_ReloadMarkedProfile(lua_State* L)
{
    int iViewport = gActiveViewport;
    memcpy(PlayerProfile::GetProfile(iViewport), &sSavedPlayerOne, sizeof(PlayerProfile::tProfile));
    PlayerProfile::SetCurProfile(iViewport);
    PlayerPrefs::mAllUserPrefs[0].pProfile = PlayerProfile::GetCurProfile();
    PlayerProfile::SetCurProfile(iViewport);
    return 0;
}

// FUNCTION: SWBF2 0x005890e0
int ScriptCB_HasProfileChanged(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState,
                    memcmp(PlayerProfile::GetProfile(0), &sSavedPlayerOne, sizeof(PlayerProfile::tProfile)) != 0);
    return 1;
}

}

// FUNCTION: SWBF2 0x00589130
static void DeleteProfileDoneCallback()
{
    LuaHelper::CallGlobalProc("ifs_saveop_DeleteProfileDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589150
int ScriptCB_StartDeleteProfile(lua_State* L)
{
    if (!lua_isstring(LuaHelper::mState, 1))
    {
        return 0;
    }

    const unsigned short* pusName = (unsigned short const*)luaL_checklstring(LuaHelper::mState, 1, 0);
    if (!SaveGameManager::InMemoryMode())
    {
        SaveGameManager::StartQueue();
        SaveGameManager::QueueDelete(pusName, 1, false);
        SaveGameManager::FinishQueue(DeleteProfileDoneCallback);
        return 0;
    }

    int iFound = PlayerProfile::FindByName(pusName);
    if (iFound >= 0)
    {
        int iNum = PlayerProfile::GetNumMemoryProfiles();
        if (iFound < iNum - 1)
        {
            memmove(PlayerProfile::GetProfile(iFound), PlayerProfile::GetProfile(iFound + 1),
                    (iNum - iFound - 1) * sizeof(PlayerProfile::tProfile));
        }
        PlayerProfile::SetNumMemoryProfiles(iNum - 1);
    }
    LuaHelper::CallGlobalProc("ifs_saveop_DeleteProfileDone", true);
    return 0;
}

}

// FUNCTION: SWBF2 0x00589200
static void SaveProfileDoneCallback()
{
    bool bSuccess = SaveGameManager::GetSuccess();
    if (bSuccess == true)
    {
        PlayerProfile::SetNumMemoryProfiles(0);
        SaveGameManager::SetMemoryMode(false);
        PlayerProfile::GetProfile(savedWhich)->mIsDirty = false;
    }
    LuaHelper::CallGlobalProc("ifs_saveop_SaveProfileDone", bSuccess);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589240
int ScriptCB_StartSaveProfile(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1)
    {
        lua_pushnil(LuaHelper::mState);
        return 1;
    }
    unsigned int uiIndex = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    if (uiIndex < 1)
    {
        savedWhich = uiIndex;
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(uiIndex);
        if (pProfile->mIsDirty)
        {
            SaveGameManager::StartQueue();
            SaveGameManager::QueueSave(pProfile->mName, 1, pProfile, sizeof(PlayerProfile::tProfile));
            SaveGameManager::FinishQueue(SaveProfileDoneCallback);
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005892d0
void LoadMetagameDoneCallback()
{
    bool bSuccess = SaveGameManager::GetSuccess();
    sMetagameBufferSize = bSuccess ? sizeof(sMetagameBuffer) : 0;
    LuaHelper::CallGlobalProc("ifs_saveop_LoadMetagameDone", bSuccess);
}

// FUNCTION: SWBF2 0x00589300
int ScriptCB_StartLoadMetagame(lua_State* L)
{
    const unsigned short* pusName = (unsigned short const*)luaL_checklstring(LuaHelper::mState, 1, 0);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueLoad(pusName, 2, sMetagameBuffer, sizeof(sMetagameBuffer));
    SaveGameManager::FinishQueue(LoadMetagameDoneCallback);
    PblUnicode::ustrlcpy(sMetagameFilename, pusName, sizeof(sMetagameFilename));
    return 0;
}

}

// FUNCTION: SWBF2 0x00589350
static void SaveMetagameDoneCallback()
{
    LuaHelper::CallGlobalProc("ifs_saveop_SaveMetagameDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589370
int ScriptCB_StartSaveMetagame(lua_State* L)
{
    const unsigned short* pusOld = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 1, 0, 0);
    const unsigned short* pusName = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 2, 0, 0);
    unsigned short ausFilename[128];
    PblUnicode::ustrlcpy(ausFilename, pusName ? pusName : PlayerProfile::GetProfile(gActiveViewport)->mName, 128);
    SaveFile::MakeMetagameFilename(ausFilename, ausFilename);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueSave(ausFilename, 2, sMetagameBuffer, sizeof(sMetagameBuffer));
    if (pusOld)
    {
        SaveGameManager::QueueDelete(pusOld, 2, true);
    }
    SaveGameManager::FinishQueue(SaveMetagameDoneCallback);
    return 0;
}

}

// FUNCTION: SWBF2 0x00589420
static void DeleteMetagameDoneCallback()
{
    LuaHelper::CallGlobalProc("ifs_saveop_DeleteMetagameDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589440
int ScriptCB_StartDeleteMetagame(lua_State* L)
{
    const unsigned short* pusName = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 1, 0, 0);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueDelete(pusName, 2, false);
    SaveGameManager::FinishQueue(DeleteMetagameDoneCallback);
    return 0;
}

// FUNCTION: SWBF2 0x00589480
int ScriptCB_IsCurrentMetagame(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, sMetagameFilename[0] != 0);
    return 1;
}

// FUNCTION: SWBF2 0x005894b0
void LoadCampaignDoneCallback()
{
    bool bSuccess = SaveGameManager::GetSuccess();
    sCampaignBufferSize = bSuccess ? sizeof(sCampaignBuffer) : 0;
    LuaHelper::CallGlobalProc("ifs_saveop_LoadCampaignDone", bSuccess);
}

// FUNCTION: SWBF2 0x005894e0
int ScriptCB_StartLoadCampaign(lua_State* L)
{
    const unsigned short* pusName = (unsigned short const*)luaL_checklstring(LuaHelper::mState, 1, 0);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueLoad(pusName, 3, sCampaignBuffer, sizeof(sCampaignBuffer));
    SaveGameManager::FinishQueue(LoadCampaignDoneCallback);
    PblUnicode::ustrlcpy(sCampaignFilename, pusName, sizeof(sCampaignFilename));
    return 0;
}

}

// FUNCTION: SWBF2 0x00589530
static void SaveCampaignDoneCallback()
{
    LuaHelper::CallGlobalProc("ifs_saveop_SaveCampaignDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589550
int ScriptCB_StartSaveCampaign(lua_State* L)
{
    const unsigned short* pusOld = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 1, 0, 0);
    const unsigned short* pusName = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 2, 0, 0);
    unsigned short ausFilename[128];
    PblUnicode::ustrlcpy(ausFilename, pusName ? pusName : PlayerProfile::GetProfile(gActiveViewport)->mName, 128);
    SaveFile::MakeMetagameFilename(ausFilename, ausFilename);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueSave(ausFilename, 3, sCampaignBuffer, sizeof(sCampaignBuffer));
    if (pusOld)
    {
        SaveGameManager::QueueDelete(pusOld, 3, true);
    }
    SaveGameManager::FinishQueue(SaveCampaignDoneCallback);
    return 0;
}

}

// FUNCTION: SWBF2 0x00589600
static void DeleteCampaignDoneCallback()
{
    LuaHelper::CallGlobalProc("ifs_saveop_DeleteCampaignDone", SaveGameManager::GetSuccess());
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589620
int ScriptCB_StartDeleteCampaign(lua_State* L)
{
    const unsigned short* pusName = (unsigned short const*)luaL_optlstring(LuaHelper::mState, 1, 0, 0);
    SaveGameManager::StartQueue();
    SaveGameManager::QueueDelete(pusName, 3, false);
    SaveGameManager::FinishQueue(DeleteCampaignDoneCallback);
    return 0;
}

// FUNCTION: SWBF2 0x00589660
int ScriptCB_IsCurrentCampaign(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, sCampaignFilename[0] != 0);
    return 1;
}

// FUNCTION: SWBF2 0x00589690
int ScriptCB_TrimLoginName(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) == 1 && lua_isstring(LuaHelper::mState, 1))
    {
        char acName[100];
        strlcpy(acName, luaL_checklstring(LuaHelper::mState, 1, 0), sizeof(acName));

        char* pcName = acName;
        while (*pcName && isspace(*pcName))
        {
            ++pcName;
        }
        for (int i = (int)strlen(pcName) - 1; i >= 0 && isspace(pcName[i]); --i)
        {
            pcName[i] = 0;
        }

        if (strlen(pcName))
        {
            lua_pushstring(LuaHelper::mState, pcName);
            return 1;
        }
    }
    lua_pushboolean(LuaHelper::mState, false);
    return 1;
}

// FUNCTION: SWBF2 0x00589780
int ScriptCB_GetUniqueLoginName(lua_State* L)
{
    lua_gettop(LuaHelper::mState);
    const unsigned short* pusBase = RedLocalizeDB::Find(0x3802F15B, false);
    if (!pusBase)
    {
        lua_pushstring(LuaHelper::mState, "");
        return 1;
    }

    PblUnicode::ustrncpy(Lua_Callbacks::sTempReturnString16, pusBase, 256);
    if (!SaveGameManager::InMemoryMode())
    {
        SaveGameManager::GetUniqueFilename(Lua_Callbacks::sTempReturnString16, 1, 0, 0);
        LuaHelper::PushString(Lua_Callbacks::sTempReturnString16);
        return 1;
    }

    for (unsigned int uiTry = 1; uiTry <= 2; ++uiTry)
    {
        PblUnicode::usnprintf(Lua_Callbacks::sTempReturnString16, 512, pusBase, uiTry);
        bool bUnique = true;
        for (int i = 0; i < PlayerProfile::GetNumMemoryProfiles(); ++i)
        {
            if (!PblUnicode::ustrcmp(PlayerProfile::GetProfile(i)->mName, Lua_Callbacks::sTempReturnString16))
            {
                bUnique = false;
                break;
            }
        }
        if (bUnique)
        {
            break;
        }
    }
    LuaHelper::PushString(Lua_Callbacks::sTempReturnString16);
    return 1;
}

// FUNCTION: SWBF2 0x00589880
int ScriptCB_IsUniqueLoginName(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) != 1 || !lua_isstring(LuaHelper::mState, 1))
    {
        lua_pushnil(LuaHelper::mState);
        return 1;
    }

    const unsigned short* pusName = (unsigned short const*)luaL_checklstring(LuaHelper::mState, 1, 0);
    bool bUnique;
    if (!SaveGameManager::InMemoryMode())
    {
        bUnique = SaveGameManager::IsUniqueFilename(pusName, 1);
    }
    else
    {
        bUnique = true;
        for (int i = 0; i < PlayerProfile::GetNumMemoryProfiles(); ++i)
        {
            if (!PblUnicode::ustrcmp(PlayerProfile::GetProfile(i)->mName, pusName))
            {
                bUnique = false;
                break;
            }
        }
    }
    lua_pushboolean(LuaHelper::mState, bUnique);
    return 1;
}

// FUNCTION: SWBF2 0x00589940
int ScriptCB_MarkMemoryCard(lua_State* L)
{
    SaveGameManager::MarkMemoryCard();
    return 0;
}

// FUNCTION: SWBF2 0x00589950
int ScriptCB_CheckMemoryCardMark(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, SaveGameManager::CheckMemoryCardMark());
    return 1;
}

// FUNCTION: SWBF2 0x00589970
int ScriptCB_IsMemoryCardUnformatted(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, SaveGameManager::IsMemoryCardUnformatted());
    return 1;
}

// FUNCTION: SWBF2 0x00589990
int ScriptCB_IsMemoryCardInsertted(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, SaveGameManager::IsMemoryCardInsertted());
    return 1;
}

// FUNCTION: SWBF2 0x005899b0
int ScriptCB_DoesProfileExistOnCard(lua_State* L)
{
    unsigned int uiIndex = (int)luaL_checknumber(LuaHelper::mState, 1) - 1;
    if (uiIndex >= 1)
    {
        return 0;
    }
    bool bExists = false;
    PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(uiIndex);
    if (pProfile)
    {
        bExists = !SaveGameManager::IsUniqueFilename(pProfile->mName, 1);
    }
    lua_pushboolean(LuaHelper::mState, bExists);
    return 1;
}

// FUNCTION: SWBF2 0x00589a20
int ScriptCB_InMemoryProfileMode(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, SaveGameManager::InMemoryMode());
    return 1;
}

// FUNCTION: SWBF2 0x00589a40
int ScriptCB_SetMemoryProfileMode(lua_State* L)
{
    SaveGameManager::SetMemoryMode(lua_toboolean(LuaHelper::mState, 1) != 0);
    return 0;
}

// FUNCTION: SWBF2 0x00589a60
int ScriptCB_CheckIfOkToExitMemoryMode(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, SaveGameManager::CheckIfOkToExitMemoryMode());
    return 1;
}

}

// FUNCTION: SWBF2 0x00589a80
static int SavegameSortCallback(const void* pA, const void* pB)
{
    const OneSavegame* pSaveA = (const OneSavegame*)pA;
    const OneSavegame* pSaveB = (const OneSavegame*)pB;
    if (pSaveA->mIsCurrent)
    {
        return -1;
    }
    if (pSaveB->mIsCurrent)
    {
        return 1;
    }
    return -PblUnicode::ustrcmp(pSaveA->mGamedate, pSaveB->mGamedate);
}

// FUNCTION: SWBF2 0x00589ac0
static void SortSavegames(OneSavegame* pSavegames, int iCount)
{
    qsort(pSavegames, iCount, sizeof(OneSavegame), SavegameSortCallback);
}

namespace Lua_Callbacks
{

// FUNCTION: SWBF2 0x00589ae0
int ScriptCB_GetSavedMetagameList(lua_State* L)
{
    OneSavegame aSavegames[0x800];
    unsigned short ausName[128];
    unsigned short ausDate[128];

    lua_gettop(LuaHelper::mState);
    lua_toboolean(LuaHelper::mState, 1);
    lua_newtable(LuaHelper::mState);

    SaveGameManager::ResetFileList();
    int iNumFiles = 0;
    while (SaveGameManager::GetNextFileName(2, 0))
    {
        ++iNumFiles;
    }

    int iArg;
    const unsigned short* pusFile;
    if (iNumFiles >= 0x800)
    {
        SaveGameManager::ResetFileList();
        int iIndex = 1;
        while ((pusFile = SaveGameManager::GetNextFileName(2, &iArg)) != 0)
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("filename", pusFile);
            SaveFile::SplitMetagameFilename(pusFile, ausName, ausDate, 0, 0, 0);
            LuaHelper::TableWriteVal("namestr", ausName);
            LuaHelper::TableWriteVal("datestr", ausDate);
            if (sMetagameFilename[0] && !PblUnicode::ustrcmp(sMetagameFilename, pusFile))
            {
                LuaHelper::TableWriteVal("isCurrent", true);
            }
            lua_rawseti(LuaHelper::mState, -2, iIndex++);
        }
    }
    else
    {
        int iCount = 0;
        SaveGameManager::ResetFileList();
        while ((pusFile = SaveGameManager::GetNextFileName(2, &iArg)) != 0)
        {
            OneSavegame* pSave = &aSavegames[iCount];
            SaveFile::SplitMetagameFilename(pusFile, ausName, ausDate, 0, 0, 0);
            memcpy(pSave->mFilename, pusFile, sizeof(pSave->mFilename));
            memcpy(pSave->mGamename, ausName, sizeof(pSave->mGamename));
            memcpy(pSave->mGamedate, ausDate, sizeof(pSave->mGamedate));
            pSave->mIsCurrent = sCampaignFilename[0] && !PblUnicode::ustrcmp(sCampaignFilename, pusFile);
            ++iCount;
        }

        SortSavegames(aSavegames, iCount);
        for (int i = 0; i < iNumFiles; ++i)
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("filename", aSavegames[i].mFilename);
            LuaHelper::TableWriteVal("namestr", aSavegames[i].mGamename);
            LuaHelper::TableWriteVal("datestr", aSavegames[i].mGamedate);
            if (aSavegames[i].mIsCurrent)
            {
                LuaHelper::TableWriteVal("isCurrent", true);
            }
            lua_rawseti(LuaHelper::mState, -2, i + 1);
        }
    }

    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, 1024.0f);
    return 2;
}

// FUNCTION: SWBF2 0x00589e20
int ScriptCB_GetSavedCampaignList(lua_State*)
{
    OneSavegame aSavegames[0x800];
    unsigned short ausName[128];
    unsigned short ausDate[128];

    lua_gettop(LuaHelper::mState);
    lua_toboolean(LuaHelper::mState, 1);
    lua_newtable(LuaHelper::mState);

    SaveGameManager::ResetFileList();
    int iNumFiles = 0;
    while (SaveGameManager::GetNextFileName(3, 0))
    {
        ++iNumFiles;
    }

    int iArg;
    const unsigned short* pusFile;
    if (iNumFiles >= 0x800)
    {
        SaveGameManager::ResetFileList();
        int iIndex = 1;
        while ((pusFile = SaveGameManager::GetNextFileName(3, &iArg)) != 0)
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("filename", pusFile);
            SaveFile::SplitMetagameFilename(pusFile, ausName, ausDate, 0, 0, 0);
            LuaHelper::TableWriteVal("namestr", ausName);
            LuaHelper::TableWriteVal("datestr", ausDate);
            if (sCampaignFilename[0] && !PblUnicode::ustrcmp(sCampaignFilename, pusFile))
            {
                LuaHelper::TableWriteVal("isCurrent", true);
            }
            lua_rawseti(LuaHelper::mState, -2, iIndex++);
        }
    }
    else
    {
        int iCount = 0;
        SaveGameManager::ResetFileList();
        while ((pusFile = SaveGameManager::GetNextFileName(3, &iArg)) != 0)
        {
            OneSavegame* pSave = &aSavegames[iCount];
            SaveFile::SplitMetagameFilename(pusFile, ausName, ausDate, 0, 0, 0);
            memcpy(pSave->mFilename, pusFile, sizeof(pSave->mFilename));
            memcpy(pSave->mGamename, ausName, sizeof(pSave->mGamename));
            memcpy(pSave->mGamedate, ausDate, sizeof(pSave->mGamedate));
            pSave->mIsCurrent = sCampaignFilename[0] && !PblUnicode::ustrcmp(sCampaignFilename, pusFile);
            ++iCount;
        }

        SortSavegames(aSavegames, iCount);
        for (int i = 0; i < iNumFiles; ++i)
        {
            lua_newtable(LuaHelper::mState);
            LuaHelper::TableWriteVal("filename", aSavegames[i].mFilename);
            LuaHelper::TableWriteVal("namestr", aSavegames[i].mGamename);
            LuaHelper::TableWriteVal("datestr", aSavegames[i].mGamedate);
            if (aSavegames[i].mIsCurrent)
            {
                LuaHelper::TableWriteVal("isCurrent", true);
            }
            lua_rawseti(LuaHelper::mState, -2, i + 1);
        }
    }

    DoPeriodicLuaGC(false);
    lua_pushnumber(LuaHelper::mState, 1024.0f);
    return 2;
}

// FUNCTION: SWBF2 0x0058a160
int ScriptCB_Logout(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState) <= 1)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(0);
        if (pProfile)
        {
            pProfile->UpdateProfCRC();
            pProfile->mIsDirty = false;
        }
        PlayerProfile::SetCurProfile((unsigned int)-1);
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058a1f0
int ScriptCB_IsMetagameStateSaved(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, sMetagameBufferSize != 0);
    return 1;
}

// FUNCTION: SWBF2 0x0058a270
int ScriptCB_IsCampaignStateSaved(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, sCampaignBufferSize != 0);
    return 1;
}

// FUNCTION: SWBF2 0x0058a2d0
int ScriptCB_GetInTrainingMission(lua_State* L)
{
    if (lua_gettop(LuaHelper::mState))
    {
        return 0;
    }
    lua_pushboolean(LuaHelper::mState, gInTrainingMission);
    return 1;
}

// FUNCTION: SWBF2 0x0058a360
int ScriptCB_IsMissionSetupSaved(lua_State* L)
{
    lua_pushboolean(LuaHelper::mState, sMissionBufferSize != 0);
    return 1;
}

// FUNCTION: SWBF2 0x00589da0
int ScriptCB_StartNewMetagame(lua_State*)
{
    memset(sMetagameFilename, 0, sizeof(sMetagameFilename));
    return 0;
}

// FUNCTION: SWBF2 0x00589dc0
int ScriptCB_DoesMetagameExistOnCard(lua_State*)
{
    if (lua_isstring(LuaHelper::mState, 1))
    {
        const unsigned short* pusName = reinterpret_cast<const unsigned short*>(luaL_checklstring(LuaHelper::mState, 1, 0));
        lua_pushboolean(LuaHelper::mState, !SaveGameManager::IsUniqueFilename(pusName, 2));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058a0e0
int ScriptCB_StartNewCampaign(lua_State*)
{
    memset(sCampaignFilename, 0, sizeof(sCampaignFilename));
    return 0;
}

// FUNCTION: SWBF2 0x0058a100
int ScriptCB_DoesCampaignExistOnCard(lua_State*)
{
    if (lua_isstring(LuaHelper::mState, 1))
    {
        const unsigned short* pusName = reinterpret_cast<const unsigned short*>(luaL_checklstring(LuaHelper::mState, 1, 0));
        lua_pushboolean(LuaHelper::mState, !SaveGameManager::IsUniqueFilename(pusName, 3));
    }
    else
    {
        lua_pushnil(LuaHelper::mState);
    }
    return 1;
}

// FUNCTION: SWBF2 0x0058a1a0
int ScriptCB_SaveMetagameState(lua_State* L)
{
    sMetagameBufferSize = Lua_Callbacks::SaveLuaItems(L, sMetagameBuffer, sizeof(sMetagameBuffer));
    return 0;
}

// FUNCTION: SWBF2 0x0058a1c0
int ScriptCB_LoadMetagameState(lua_State* L)
{
    return Lua_Callbacks::LoadLuaItems(L, sMetagameBuffer, sMetagameBufferSize);
}

// FUNCTION: SWBF2 0x0058a1e0
int ScriptCB_ClearMetagameState(lua_State*)
{
    sMetagameBufferSize = 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058a220
int ScriptCB_SaveCampaignState(lua_State* L)
{
    sCampaignBufferSize = Lua_Callbacks::SaveLuaItems(L, sCampaignBuffer, sizeof(sCampaignBuffer));
    return 0;
}

// FUNCTION: SWBF2 0x0058a240
int ScriptCB_LoadCampaignState(lua_State* L)
{
    return Lua_Callbacks::LoadLuaItems(L, sCampaignBuffer, sCampaignBufferSize);
}

// FUNCTION: SWBF2 0x0058a260
int ScriptCB_ClearCampaignState(lua_State*)
{
    sCampaignBufferSize = 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058a2a0
int ScriptCB_SetInTrainingMission(lua_State*)
{
    if (lua_gettop(LuaHelper::mState) == 1)
    {
        gInTrainingMission = lua_toboolean(LuaHelper::mState, 1) != 0;
    }
    return 0;
}

// FUNCTION: SWBF2 0x0058a310
int ScriptCB_SaveMissionSetup(lua_State* L)
{
    sMissionBufferSize = Lua_Callbacks::SaveLuaItems(L, sMissionBuffer, sizeof(sMissionBuffer));
    return 0;
}

// FUNCTION: SWBF2 0x0058a330
int ScriptCB_LoadMissionSetup(lua_State* L)
{
    return Lua_Callbacks::LoadLuaItems(L, sMissionBuffer, sMissionBufferSize);
}

// FUNCTION: SWBF2 0x0058a350
int ScriptCB_ClearMissionSetup(lua_State*)
{
    sMissionBufferSize = 0;
    return 0;
}

// FUNCTION: SWBF2 0x0058a390
static int FindLuaGlobal(void const* pValue)
{
    lua_pushnil(LuaHelper::mState);
    while (lua_next(LuaHelper::mState, LUA_GLOBALSINDEX))
    {
        if (lua_topointer(LuaHelper::mState, -1) == pValue)
        {
            lua_settop(LuaHelper::mState, -2);
            return 1;
        }
        lua_settop(LuaHelper::mState, -2);
    }
    return 0;
}

// GLOBAL: SWBF2 0x007e7170
static const CallbackEntryT sLoadSaveCallbacks[64] =
{
    { "ScriptCB_IsCurProfileDirty", ScriptCB_IsCurProfileDirty },
    { "ScriptCB_IsProfileDirty", ScriptCB_IsProfileDirty },
    { "ScriptCB_SetProfileNotDirty", ScriptCB_SetProfileNotDirty },
    { "ScriptCB_LoadSavePopupResult", ScriptCB_LoadSavePopupResult },
    { "ScriptCB_DoInitialMemcardCheck", ScriptCB_DoInitialMemcardCheck },
    { "ScriptCB_StartInitialMemcardCheck", ScriptCB_StartInitialMemcardCheck },
    { "ScriptCB_StartPreOp", ScriptCB_StartPreOp },
    { "ScriptCB_StartLoadProfile", ScriptCB_StartLoadProfile },
    { "ScriptCB_MakeFakeProfiles", ScriptCB_MakeFakeProfiles },
    { "ScriptCB_SetProfileAsDefault", ScriptCB_SetProfileAsDefault },
    { "ScriptCB_RestoreDefaultProfile", ScriptCB_RestoreDefaultProfile },
    { "ScriptCB_MarkCurrentProfile", ScriptCB_MarkCurrentProfile },
    { "ScriptCB_ReloadMarkedProfile", ScriptCB_ReloadMarkedProfile },
    { "ScriptCB_HasProfileChanged", ScriptCB_HasProfileChanged },
    { "ScriptCB_StartDeleteProfile", ScriptCB_StartDeleteProfile },
    { "ScriptCB_StartSaveProfile", ScriptCB_StartSaveProfile },
    { "ScriptCB_StartLoadMetagame", ScriptCB_StartLoadMetagame },
    { "ScriptCB_StartSaveMetagame", ScriptCB_StartSaveMetagame },
    { "ScriptCB_IsCurrentMetagame", ScriptCB_IsCurrentMetagame },
    { "ScriptCB_StartDeleteMetagame", ScriptCB_StartDeleteMetagame },
    { "ScriptCB_StartLoadCampaign", ScriptCB_StartLoadCampaign },
    { "ScriptCB_StartSaveCampaign", ScriptCB_StartSaveCampaign },
    { "ScriptCB_IsCurrentCampaign", ScriptCB_IsCurrentCampaign },
    { "ScriptCB_StartDeleteCampaign", ScriptCB_StartDeleteCampaign },
    { "ScriptCB_MarkMemoryCard", ScriptCB_MarkMemoryCard },
    { "ScriptCB_CheckMemoryCardMark", ScriptCB_CheckMemoryCardMark },
    { "ScriptCB_IsMemoryCardUnformatted", ScriptCB_IsMemoryCardUnformatted },
    { "ScriptCB_IsMemoryCardInsertted", ScriptCB_IsMemoryCardInsertted },
    { "ScriptCB_DoesProfileExistOnCard", ScriptCB_DoesProfileExistOnCard },
    { "ScriptCB_InMemoryProfileMode", ScriptCB_InMemoryProfileMode },
    { "ScriptCB_SetMemoryProfileMode", ScriptCB_SetMemoryProfileMode },
    { "ScriptCB_CheckIfOkToExitMemoryMode", ScriptCB_CheckIfOkToExitMemoryMode },
    { "ScriptCB_GetLoginList", ScriptCB_GetLoginList },
    { "ScriptCB_AddProfile", ScriptCB_AddProfile },
    { "ScriptCB_SwapActiveProfileSlots", ScriptCB_SwapActiveProfileSlots },
    { "ScriptCB_IsPlayerLoggedIn", ScriptCB_IsPlayerLoggedIn },
    { "ScriptCB_GetCurrentProfileNetName", ScriptCB_GetCurrentProfileNetName },
    { "ScriptCB_GetCurrentProfileName", ScriptCB_GetCurrentProfileName },
    { "ScriptCB_GetProfileName", ScriptCB_GetProfileName },
    { "ScriptCB_GetCurrentPCTitleVersion", ScriptCB_GetCurrentPCTitleVersion },
    { "ScriptCB_Logout", ScriptCB_Logout },
    { "ScriptCB_TrimLoginName", ScriptCB_TrimLoginName },
    { "ScriptCB_GetUniqueLoginName", ScriptCB_GetUniqueLoginName },
    { "ScriptCB_IsUniqueLoginName", ScriptCB_IsUniqueLoginName },
    { "ScriptCB_SaveMetagameState", ScriptCB_SaveMetagameState },
    { "ScriptCB_LoadMetagameState", ScriptCB_LoadMetagameState },
    { "ScriptCB_ClearMetagameState", ScriptCB_ClearMetagameState },
    { "ScriptCB_IsMetagameStateSaved", ScriptCB_IsMetagameStateSaved },
    { "ScriptCB_SaveCampaignState", ScriptCB_SaveCampaignState },
    { "ScriptCB_LoadCampaignState", ScriptCB_LoadCampaignState },
    { "ScriptCB_ClearCampaignState", ScriptCB_ClearCampaignState },
    { "ScriptCB_IsCampaignStateSaved", ScriptCB_IsCampaignStateSaved },
    { "ScriptCB_SetInTrainingMission", ScriptCB_SetInTrainingMission },
    { "ScriptCB_GetInTrainingMission", ScriptCB_GetInTrainingMission },
    { "ScriptCB_SaveMissionSetup", ScriptCB_SaveMissionSetup },
    { "ScriptCB_LoadMissionSetup", ScriptCB_LoadMissionSetup },
    { "ScriptCB_ClearMissionSetup", ScriptCB_ClearMissionSetup },
    { "ScriptCB_IsMissionSetupSaved", ScriptCB_IsMissionSetupSaved },
    { "ScriptCB_GetSavedMetagameList", ScriptCB_GetSavedMetagameList },
    { "ScriptCB_StartNewMetagame", ScriptCB_StartNewMetagame },
    { "ScriptCB_DoesMetagameExistOnCard", ScriptCB_DoesMetagameExistOnCard },
    { "ScriptCB_GetSavedCampaignList", ScriptCB_GetSavedCampaignList },
    { "ScriptCB_StartNewCampaign", ScriptCB_StartNewCampaign },
    { "ScriptCB_DoesCampaignExistOnCard", ScriptCB_DoesCampaignExistOnCard },
};

// FUNCTION: SWBF2 0x0058aa30
void InitLoadSave()
{
    for (int i = 0; i < 64; ++i)
    {
        LuaHelper::Register(sLoadSaveCallbacks[i].pcName, sLoadSaveCallbacks[i].pfnCallback, 0);
    }
}

}

// FUNCTION: SWBF2 0x00631380
int SaveFile::TimeToString(unsigned short* pusDest, int iYear, int iMonth, int iDay,
                       int iHour, int iMinute, int iSecond)
{
    const unsigned int uStamp = iSecond + 60 * (iMinute + 60 *
        (iHour + 24 * (iDay % 32 + 32 *
        (iMonth + 13 * (iYear + iMonth / -13 - 100 * (iYear / 100))) - iHour / 24)
        - iMinute / 60) - iSecond / 60);
    char acStamp[12];
    _snprintf(acStamp, 9, "%08x", uStamp);
    return PblUnicode::ustrncnv(pusDest, acStamp, 16);
}

// FUNCTION: SWBF2 0x004266b0
void SaveFile::MakeMetagameFilename(unsigned short* pusDest, unsigned short const* pusSource)
{
    unsigned short ausStamp[16];
    SYSTEMTIME localTime;
    PblUnicode::ustrncpy(pusDest, pusSource, 128);
    GetLocalTime(&localTime);
    SaveFile::TimeToString(ausStamp, localTime.wYear, localTime.wMonth, localTime.wDay,
               localTime.wHour, localTime.wMinute, localTime.wSecond);
    PblUnicode::ustrlcat(pusDest, ausStamp, 128);
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058a410
static int WriteLuaItem(unsigned char* pOut, int iIndex, int iDepth, bool bArg)
{
    (void)bArg;
    lua_State* L = LuaHelper::mState;
    lua_pushvalue(L, iIndex);
    const int iType = lua_type(L, -1);
    *pOut = static_cast<unsigned char>(iType);
    unsigned char* pWrite = pOut + 1;
    switch (iType)
    {
    case LUA_TBOOLEAN:
        *pWrite++ = lua_toboolean(L, -1) != 0;
        break;
    case LUA_TLIGHTUSERDATA:
    {
        void const* pValue = lua_topointer(L, -1);
        memcpy(pWrite, &pValue, sizeof(pValue));
        pWrite += sizeof(pValue);
        break;
    }
    case LUA_TNUMBER:
    {
        float fValue = static_cast<float>(luaL_checknumber(L, -1));
        memcpy(pWrite, &fValue, sizeof(fValue));
        pWrite += sizeof(fValue);
        break;
    }
    case LUA_TSTRING:
    {
        const char* pcValue = luaL_checklstring(L, -1, 0);
        const unsigned int uLength = strlen(pcValue) + 1;
        memcpy(pWrite, pcValue, uLength);
        pWrite += uLength;
        break;
    }
    case LUA_TTABLE:
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            pWrite += WriteLuaItem(pWrite, -2, iDepth + 1, false);
            pWrite += WriteLuaItem(pWrite, -1, iDepth + 1, false);
            lua_settop(L, -2);
        }
        *pWrite++ = 0;
        break;
    case LUA_TFUNCTION:
    {
        void const* pValue = lua_topointer(L, -1);
        if (Lua_Callbacks::FindLuaGlobal(pValue))
        {
            const char* pcName = luaL_checklstring(L, -1, 0);
            const unsigned int uLength = strlen(pcName) + 1;
            memcpy(pWrite, pcName, uLength);
            pWrite += uLength;
            lua_settop(L, -2);
        }
        else
        {
            *pWrite++ = 0;
        }
        break;
    }
    default:
        break;
    }
    lua_settop(L, -2);
    return static_cast<int>(pWrite - pOut);
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058a5c0
static int ReadLuaItem(unsigned char* pData, int iDepth, bool bArg)
{
    (void)bArg;
    lua_State* L = LuaHelper::mState;
    unsigned char* pRead = pData + 1;
    switch (*pData)
    {
    case 0xff:
        break;
    case LUA_TBOOLEAN:
        lua_pushboolean(L, *pRead++ != 0);
        break;
    case LUA_TLIGHTUSERDATA:
    {
        void* pValue;
        memcpy(&pValue, pRead, sizeof(pValue));
        pRead += sizeof(pValue);
        lua_pushlightuserdata(L, pValue);
        break;
    }
    case LUA_TNUMBER:
    {
        float fValue;
        memcpy(&fValue, pRead, sizeof(fValue));
        pRead += sizeof(fValue);
        lua_pushnumber(L, fValue);
        break;
    }
    case LUA_TSTRING:
    {
        const unsigned int uLength = strlen(reinterpret_cast<const char*>(pRead));
        lua_pushlstring(L, reinterpret_cast<const char*>(pRead), uLength);
        pRead += uLength + 1;
        break;
    }
    case LUA_TTABLE:
        lua_newtable(L);
        while (*pRead)
        {
            pRead += ReadLuaItem(pRead, iDepth + 1, false);
            pRead += ReadLuaItem(pRead, iDepth + 1, false);
            lua_rawset(L, -3);
        }
        ++pRead;
        break;
    case LUA_TFUNCTION:
        LuaHelper::GetGlobal(reinterpret_cast<const char*>(pRead));
        pRead += strlen(reinterpret_cast<const char*>(pRead)) + 1;
        break;
    default:
        lua_pushnil(L);
        break;
    }
    return static_cast<int>(pRead - pData);
}
}

// FUNCTION: SWBF2 0x0058a750
static void* __cdecl _zlibAlloc(void*, unsigned int uCount, unsigned int uSize)
{
    const unsigned int uBytes = uCount * uSize;
    void* pResult = RedAllocFromHeap(__RedCurrHeap, uBytes, 0);
    if (pResult)
    {
        memset(pResult, 0, uBytes);
    }
    return pResult;
}

// FUNCTION: SWBF2 0x0058a790
static void __cdecl _zlibFree(void*, void* pBlock)
{
    RedFreeToHeap(__RedCurrHeap, pBlock, 0);
}

// FUNCTION: SWBF2 0x0058a7b0
static unsigned int _zlibcompress(unsigned char* pDest, int iCapacity, unsigned char* pSource, int iSize)
{
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.zalloc = _zlibAlloc;
    stream.zfree = _zlibFree;
    if (deflateInit(&stream, 9))
    {
        return 0;
    }
    stream.next_in = pSource;
    stream.avail_in = iSize;
    stream.next_out = pDest;
    stream.avail_out = iCapacity;
    deflate(&stream, Z_FINISH);
    deflateEnd(&stream);
    return stream.total_out;
}

// FUNCTION: SWBF2 0x0058a850
static unsigned int _zlibuncompress(unsigned char* pDest, int iCapacity, unsigned char* pSource, int iSize)
{
    z_stream stream;
    memset(&stream, 0, sizeof(stream));
    stream.zalloc = _zlibAlloc;
    stream.zfree = _zlibFree;
    if (inflateInit_(&stream, ZLIB_VERSION, sizeof(stream)))
    {
        return 0;
    }
    stream.next_in = pSource;
    stream.avail_in = iSize;
    stream.next_out = pDest;
    stream.avail_out = iCapacity;
    inflate(&stream, Z_FINISH);
    inflateEnd(&stream);
    return stream.total_out;
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058a8e0
static unsigned int SaveLuaItems(lua_State* L, unsigned char* pBuffer, unsigned int iCapacity)
{
    (void)L;
    unsigned char aSerialized[0x10000];
    const int iNumArgs = lua_gettop(LuaHelper::mState);
    if (iNumArgs <= 0)
    {
        return 0;
    }
    aSerialized[0] = static_cast<unsigned char>(iNumArgs);
    int iWritten = 1;
    for (int i = 1; i <= iNumArgs; ++i)
    {
        iWritten += Lua_Callbacks::WriteLuaItem(aSerialized + iWritten, i, 0, false);
    }
    return _zlibcompress(pBuffer, iCapacity, aSerialized, iWritten);
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058a960
static int LoadLuaItems(lua_State* L, unsigned char* pBuffer, unsigned int iSize)
{
    (void)L;
    if (!iSize)
    {
        return 0;
    }
    unsigned char aSerialized[0x10004];
    _zlibuncompress(aSerialized, 0x10000, pBuffer, iSize);
    const unsigned int uCount = aSerialized[0];
    unsigned int uRead = 1;
    for (unsigned int i = 0; i < uCount; ++i)
    {
        uRead += Lua_Callbacks::ReadLuaItem(aSerialized + uRead, 0, false);
    }
    return uCount;
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058a9d0
void __fastcall GetMissionWorld(char* pcWorld, int)
{
    Lua_Callbacks::ScriptCB_LoadMissionSetup(LuaHelper::mState);
    *pcWorld = 0;
    LuaHelper::TableReadVal("world", pcWorld, 4, true);
    lua_settop(LuaHelper::mState, -2);
}
}

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058aa10
bool IsMetaGameStateSaved()
{
    return sMetagameBufferSize != 0;
}
}

// GLOBAL: SWBF2 0x01e57720
bool gInTrainingMission;

namespace Lua_Callbacks {

// FUNCTION: SWBF2 0x0058aa20
bool IsTrainingMission()
{
    return gInTrainingMission;
}
}
