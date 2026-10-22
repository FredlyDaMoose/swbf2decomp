#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GameMovie.h"

#include "CommandLine.h"
#include "GameLoop.h"
#include "GameSoundEngine.h"
#include "GameState.h"
#include "HUDManager.h"
#include "LuaCallbacks_Options.h"
#include "NetSetup.h"
#include "NetGame.h"
#include "ShellLoop.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"

#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

extern bool mSafeMode;
extern eGAMELANGUAGE gForceLang;

namespace PlayerProfile {

// FUNCTION: SWBF2 0x0061d8f0
void SetSafeMode()
{
    mSafeMode = true;
}
}

namespace RedLocalizeDB {

// FUNCTION: SWBF2 0x006c5b80
void ForceLanguageID(int iLanguage)
{
    if (iLanguage == 5)
    {
        RedFont::sJapaneseOnly = true;
        gForceLang = static_cast<eGAMELANGUAGE>(iLanguage);
    }
    else if (iLanguage >= -1 && iLanguage < 9)
    {
        gForceLang = static_cast<eGAMELANGUAGE>(iLanguage);
    }
}
}

char gGamespyAutoName[32];
char gGamespyAutoConnectAddr[64];
char gGamespyAutoPassStr[128];
char gGamespyAutoEmail[64];
char gGamespyAutoPPass[32];
unsigned int gGamespyAutoPass;
bool gBootLagger;
bool gShowLoadSpam;
bool gLogFinalScore;
int gBootLaggerPing = 300;

// GLOBAL: SWBF2 0x007e69c9

bool sLoadNestedLevels = true;

// GLOBAL: SWBF2 0x007e666d
bool GameMovie::sUseMovies = true;
// GLOBAL: SWBF2 0x01ec0ace

bool gMissionSpecified;
char Destination[16];

// GLOBAL: SWBF2 0x007e9030
namespace PlayerPrefs { int gDefaultDifficulty = 2; }
// GLOBAL: SWBF2 0x007e9034
namespace PlayerPrefs { int gDefaultTeamNum = 1; }

extern bool pcWindowed;
extern bool pcPostInstall;
extern bool pcForceFixedFunction;
extern int pcWidth;
extern int pcHeight;

extern int journalMode;

extern bool dragEnabled;
extern bool netAutoAim;
extern bool netHiRes;
extern bool gNoPregame;
extern bool netWaitLate;
extern bool netHeroesEnabled;
extern bool gShowAllSessions;
extern bool netFrameLock;
extern bool gAIOptionEnabled;
extern bool gNoAIRespawn;
extern bool gSoundWarningEnabled;
extern bool gProfileOptionEnabled;
extern bool netAutoAssignTeamsOriginalSetting;
namespace AIDifficulty { extern bool gAutoBalanceEnabled; }
extern bool gCentreSpeakerEnhancement;

extern bool netIsOneWorld;

extern bool netAutoAssignTeams;
extern bool gUseValues;

extern bool netTeamDamage;
extern bool netShowNames;
extern int dragLoss;
extern int dragLatency;
extern int dragJitter;
extern int netHeroRuleRespawnVal;
extern int netVoiceMode;
extern int netThrottle;
extern int gAutoNetPlayers;
extern int gSpawnInvicibility;
namespace GameSoundEngine { extern int gSamplerate; }
namespace GameSoundEngine { extern int gMaxVoices; }
namespace GameSoundEngine { extern float gSoftMixerBufferSizeMs; }

extern int dragBandwidth;
extern int netHeroRuleTeam;
extern int netHeroRuleUnlock;
extern int netHeroRuleUnlockVal;
extern int netHeroRulePlayer;
extern int netHeroRuleRespawn;
extern int netUpdateSize;
extern int netPlayerLimit;
extern int netHostedPlayerLimit;
extern int netNumBots;
extern int netDedicatedPlayerLimit;
extern int autoNetPlayers;
extern int netFPS;
extern float gGameSlice;

void AddAutoNetMission(char const* pcArg, int iA, int iB);
namespace PlayerProfile { void SetSafeMode(); }
void SetNetAwardsEnabled(int);
namespace RedLocalizeDB { void ForceLanguageID(int iLanguage); }

// FUNCTION: SWBF2 0x00414900
int const Clamp(int iValue, int iMin, int iMax)
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

// FUNCTION: SWBF2 0x004795a0
void ParseCommandLine(char const* pcAppName, char const* pcCmdLine)
{

    char* apcArgv[256] = { 0 };
    char szCmdLine[0x800] = "";
    char szArgv0[0x100] = "";

    (void)pcAppName;
    strlcat(szArgv0, "", sizeof(szArgv0));
    strlcat(szCmdLine, pcCmdLine, sizeof(szCmdLine));

    int iArgc = 1;
    int i = 0;

    while (szCmdLine[i] == ' ' || szCmdLine[i] == ',')
    {
        ++i;
    }

    while (szCmdLine[i] != '\0')
    {
        if (szCmdLine[i] == '"')
        {
            ++i;
            apcArgv[iArgc++] = &szCmdLine[i];

            while (szCmdLine[i] != '\0')
            {
                if (szCmdLine[i] == '"')
                {
                    if (szCmdLine[i - 1] != '\\')
                    {
                        break;
                    }

                    szCmdLine[i - 1] = '_';
                    szCmdLine[i] = '_';
                }
                ++i;
            }

            if (szCmdLine[i] == '"')
            {
                szCmdLine[i] = '\0';
                ++i;
            }
        }
        else
        {
            apcArgv[iArgc++] = &szCmdLine[i];

            while (szCmdLine[i] != '\0' && szCmdLine[i] != ' ' && szCmdLine[i] != ',')
            {
                ++i;
            }
        }

        if (szCmdLine[i] != '\0')
        {
            szCmdLine[i] = '\0';
            ++i;
        }

        while (szCmdLine[i] == ' ' || szCmdLine[i] == ',')
        {
            ++i;
        }
    }

    apcArgv[0] = szArgv0;

    ParseCommandLine(iArgc, apcArgv);
}

// FUNCTION: SWBF2 0x00479740
void ParseCommandLine(int iArgc, char** const apcArgv)
{

    char acBuf[0x80];
    memset(acBuf, 0, sizeof(acBuf));
    _snprintf(acBuf, 0x7F, "%i Arguments Found\n", iArgc - 1);

    HUD::gScreenGroupEditEnabled = false;
    bool bNamed = false;

    for (int i = 1; i < iArgc; ++i)
    {
        _snprintf(acBuf, 0x7F, "Arg %i: %s\n", i, apcArgv[i]);

        char* pcArg = apcArgv[i];
        if (pcArg[0] != '/' && pcArg[0] != '-' && pcArg[0] != '+')
        {

            if (autoNetEnabled)
            {
                int iA = 0;
                int iB = 0;
                if (++i < iArgc)
                {
                    iA = atoi(apcArgv[i]);
                    if (iA)
                    {
                        iB = atoi(apcArgv[++i]);
                    }
                    else
                    {
                        --i;
                    }
                }
                AddAutoNetMission(pcArg, iA, iB);
            }
            else
            {
                GameLoop::SetNextMission(pcArg);
                gMissionSpecified = true;
            }
            continue;
        }

        if (_stricmp(pcArg + 1, "DecompCONMinutes") == 0)
        {
            if (i + 1 < iArgc)
            {
                const char* value = apcArgv[++i];
                const int slot = (int)Lua_Callbacks::gCurRules;
                if (strcmp(value, "5") == 0 && slot >= 0 && slot <= 4)
                {
                    Lua_Callbacks::gCONMaxTimeLimit[slot] = 5;
                }
            }
            continue;
        }

        switch (PblHash(pcArg + 1))
        {
        case 0x02049853:
            dragEnabled = true;
            dragJitter = atoi(apcArgv[++i]);
            break;
        case 0x03D17781:
            strncpy(Destination, apcArgv[++i], 16);
            Destination[15] = '\0';
            break;
        case 0x0451542B:
            PlayerProfile::SetSafeMode();
            GameMovie::sUseMovies = false;
            break;
        case 0x0481D555:
        case 0xA5CD594E:
            ShellLoop::SetLegalShowtype(ShellLoop::eLEGALSCREENS_SHOW);
            break;
        case 0x0652936B:
            sscanf(apcArgv[++i], "%f", &GameSoundEngine::gSoftMixerBufferSizeMs);
            if (GameSoundEngine::gSoftMixerBufferSizeMs < 20.0f)
            {
                GameSoundEngine::gSoftMixerBufferSizeMs = 20.0f;
            }
            break;
        case 0x09C3E0FF:
            gLogFinalScore = true;
            break;
        case 0x09CFDCF0:
            netDifficulty = atoi(apcArgv[++i]);
            break;
        case 0x0BF7CB59:
            pcWindowed = true;
            break;
        case 0x0D3D814F:
            netAutoAim = false;
            break;
        case 0x124E8FBB:
            netHiRes = true;
            break;
        case 0x1503F968:
            netUpdateSize = atoi(apcArgv[++i]);
            break;
        case 0x1D215C8F:
            pcWidth = atoi(apcArgv[++i]);
            pcHeight = atoi(apcArgv[++i]);
            break;
        case 0x1DEBAE35:
        case 0xA563005A:
            netHeroRuleRespawnVal = Lua_Callbacks::gHeroRuleRespawnVal[0] = Lua_Callbacks::gHeroRuleRespawnVal[1] = atoi(apcArgv[++i]);
            break;
        case 0x202DB217:
        case 0xCD0D5437:
            netHeroRuleUnlock = Lua_Callbacks::gHeroRuleUnlock[0] = Lua_Callbacks::gHeroRuleUnlock[1] = atoi(apcArgv[++i]);
            break;
        case 0x204F0D0D:
            break;
        case 0x2264B8ED:
            netIsOneWorld = true;
            break;
        case 0x26D94B4E:
            netIsOneWorld = false;
            break;
        case 0x2E325FB4:
            SetNetAwardsEnabled(0);
            break;
        case 0x31694A02:
            GameSoundEngine::DisableStartupMusic();
            break;
        case 0x331843BC:
            autoNetRandomize = true;
            break;
        case 0x351FC388:
            gMemoryPoolErrors = true;
            break;
        case 0x364B5F18:
            strlcpy(gGamespyAutoPassStr, apcArgv[++i], sizeof(gGamespyAutoPassStr));
            if (gGamespyAutoPassStr[0])
            {
                unsigned int uiHash = NetSetup::CalcPassword(gGamespyAutoPassStr);
                gGamespyAutoPass = uiHash;
                netPasswordHash = uiHash;
            }
            bNamed = true;
            break;
        case 0x371E360D:
        {
            int iSeconds = atoi(apcArgv[++i]);
            NetGame::SetMaxPreGameTime(iSeconds);
            if (iSeconds <= 0)
            {
                gNoPregame = true;
            }
            break;
        }
        case 0x378F32DD:
            netThrottle = atoi(apcArgv[++i]);
            break;
        case 0x3A00D703:
            netPlayerLimit = netHostedPlayerLimit = netDedicatedPlayerLimit = atoi(apcArgv[++i]);
            break;
        case 0x3A224D98:
            gSpawnInvicibility = atoi(apcArgv[++i]);
            break;
        case 0x3C3F5C38:
            gCentreSpeakerEnhancement = false;
            break;
        case 0x3EC8DA95:
        case 0xD0CFBC35:
            netHeroRuleRespawn = Lua_Callbacks::gHeroRuleRespawn[0] = Lua_Callbacks::gHeroRuleRespawn[1] = atoi(apcArgv[++i]);
            break;
        case 0x3F0ECDEB:
        case 0x77E5E744:
        case 0xBBB3B003:
            ShellLoop::SetLegalShowtype(ShellLoop::eLEGALSCREENS_SKIP);
            break;
        case 0x42EACCDD:
        case 0x4EAE0006:
            gShowAllSessions = true;
            break;
        case 0x4424F79F:
            gAIOptionEnabled = true;
            break;
        case 0x4674CAEE:
            gProfileOptionEnabled = true;
            break;
        case 0x4BE2EE33:
            netWaitLate = false;
            break;
        case 0x4F0C4904:
            strlcpy(autoNetSession, apcArgv[++i], sizeof(autoNetSession));
            break;
        case 0x56608010:
            autoNetLan = true;
            break;
        case 0x593058CC:
            journalMode = 1;
            GameMovie::sUseMovies = false;
            break;
        case 0x5D62BEB3:
        {
            gBootLagger = true;
            int iPing = atoi(apcArgv[++i]);
            if (iPing > gBootLaggerPing)
            {
                gBootLaggerPing = iPing;
            }
            break;
        }
        case 0x6363D774:
            GameLoop::EnableFirstPerson(true);
            break;
        case 0x68F63ADB:
            netAutoAim = true;
            break;
        case 0x69F6C31E:
        case 0xF546DAFE:
            netHeroRuleTeam = Lua_Callbacks::gHeroRuleTeam[0] = Lua_Callbacks::gHeroRuleTeam[1] = atoi(apcArgv[++i]);
            break;
        case 0x70EA3B55:
            RedLocalizeDB::ForceLanguageID(atoi(apcArgv[++i]));
            break;
        case 0x718E7AD4:
            dragEnabled = false;
            break;
        case 0x75E8450E:
            g_bNoRender = true;
            GameMovie::sUseMovies = false;
            break;
        case 0x7AC8BB2B:
            g_fFixedRate = (float)atof(apcArgv[++i]);
            break;
        case 0x81BA46B8:
            gNoAIRespawn = true;
            break;
        case 0x8463E1A0:
            pcPostInstall = true;
            break;
        case 0x8A8753C7:
            strlcpy(gGamespyAutoEmail, apcArgv[++i], sizeof(gGamespyAutoEmail));
            bNamed = true;
            break;
        case 0x8CEAC9BD:
            sscanf(apcArgv[++i], "%d", &GameSoundEngine::gMaxVoices);
            GameSoundEngine::gMaxVoices = Clamp(GameSoundEngine::gMaxVoices, 8, 32);
            break;
        case 0x8D39BDE6:
            strlcpy(gGamespyAutoName, apcArgv[++i], sizeof(gGamespyAutoName));
            bNamed = true;
            break;
        case 0x8DE05C06:
        case 0xE91814A3:
            ++i;
            break;
        case 0x90B89FFA:
            gAutoNetPlayers = atoi(apcArgv[++i]);
            break;
        case 0x91739772:
            dragEnabled = true;
            dragBandwidth = atoi(apcArgv[++i]);
            break;
        case 0x98BF5C17:
            gGameSlice = (float)atof(apcArgv[++i]);
            break;
        case 0x99317EA6:
            netAutoAssignTeams = false;
            netAutoAssignTeamsOriginalSetting = false;
            break;
        case 0x996359B2:
            AIDifficulty::gAutoBalanceEnabled = false;
            break;
        case 0x99B7123E:
        case 0xECD7701E:
            netHeroRulePlayer = Lua_Callbacks::gHeroRulePlayer[0] = Lua_Callbacks::gHeroRulePlayer[1] = atoi(apcArgv[++i]);
            break;
        case 0xA2FD7D0C:
            PlayerPrefs::gDefaultTeamNum = atoi(apcArgv[++i]);
            break;
        case 0xA8C4C105:
        {
            bool bHeroes = atoi(apcArgv[++i]) != 0;
            netHeroesEnabled = bHeroes;
            Lua_Callbacks::gHeroesEnabled[0] = bHeroes;
            Lua_Callbacks::gHeroesEnabled[1] = bHeroes;
            break;
        }
        case 0xAAE0CCF9:
            strlcpy(gGamespyAutoConnectAddr, apcArgv[++i], sizeof(gGamespyAutoConnectAddr));
            bNamed = true;
            break;
        case 0xAC16C90C:
        case 0xB418A0AC:
            netHeroRuleUnlockVal = Lua_Callbacks::gHeroRuleUnlockVal[0] = Lua_Callbacks::gHeroRuleUnlockVal[1] = atoi(apcArgv[++i]);
            break;
        case 0xB08CEC40:
            gUseValues = false;
            break;
        case 0xB22236AE:
            netFPS = atoi(apcArgv[++i]);
            if (!netFPS)
            {
                __debugbreak();
            }
            break;
        case 0xBA0ECD59:
            netCrashAddr = atoi(apcArgv[++i]);
            break;
        case 0xBC494AFF:
            g_bNoWait = true;
            break;
        case 0xC01E4E25:
            gDisableLCD = true;
            break;
        case 0xC11F80F6:
            netShowNames = false;
            break;
        case 0xC16650C8:
            gSoundWarningEnabled = true;
            break;
        case 0xC23937B9:
            autoNetEnabled = true;
            strlcpy(autoNetScript, apcArgv[++i], sizeof(autoNetScript));
            break;
        case 0xC53B7844:
            gShowLoadSpam = true;
            break;
        case 0xC707B50F:
            autoNetPlayers = atoi(apcArgv[++i]);
            break;
        case 0xCA15D036:
            PlayerPrefs::gDefaultDifficulty = Clamp(atoi(apcArgv[++i]), 1, 3);
            break;
        case 0xCD154944:
            netFrameLock = false;
            break;
        case 0xCF8A43EC:
            GameMovie::sUseMovies = false;
            journalMode = 2;
            break;
        case 0xD0BD1109:
        {

            static const int aiRates[9] = { 8000, 11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000 };
            sscanf(apcArgv[++i], "%d", &GameSoundEngine::gSamplerate);
            unsigned int uiBest = 0xFFFFFFFF;
            int iBest = 0;
            for (int j = 0; j < 9; ++j)
            {
                unsigned int uiDiff = abs(aiRates[j] - GameSoundEngine::gSamplerate);
                if (uiDiff < uiBest)
                {
                    uiBest = uiDiff;
                    iBest = j;
                }
            }
            GameSoundEngine::gSamplerate = aiRates[iBest];
            break;
        }
        case 0xD43981B7:
            gMemoryPoolErrors = false;
            break;
        case 0xD5C0B6F5:
            Lua_Callbacks::gSpaceAssaultScoreLimit[0] = Lua_Callbacks::gSpaceAssaultScoreLimit[1] = atoi(apcArgv[++i]);
            break;
        case 0xDABA84C3:
            GameMovie::sUseMovies = false;
            break;
        case 0xDADFA856:
            netVoiceMode = atoi(apcArgv[++i]);
            break;
        case 0xE0EBCA12:
            strlcpy(gGamespyAutoPPass, apcArgv[++i], sizeof(gGamespyAutoPPass));
            bNamed = true;
            break;
        case 0xE181E077:
            netNumBots =
                Lua_Callbacks::gASSNumBots[0] =
                Lua_Callbacks::gASSNumBots[1] =
                Lua_Callbacks::gASSNumBots[2] =
                Lua_Callbacks::gASSNumBots[3] =
                Lua_Callbacks::gELINumBots[0] =
                Lua_Callbacks::gELINumBots[1] =
                Lua_Callbacks::gELINumBots[2] =
                Lua_Callbacks::gELINumBots[3] =
                Lua_Callbacks::gCTFNumBots[0] =
                Lua_Callbacks::gCTFNumBots[1] =
                Lua_Callbacks::gCTFNumBots[2] =
                Lua_Callbacks::gCTFNumBots[3] =
                Lua_Callbacks::gCONNumBots[0] =
                Lua_Callbacks::gCONNumBots[1] =
                Lua_Callbacks::gCONNumBots[2] =
                Lua_Callbacks::gCONNumBots[3] = atoi(apcArgv[++i]);
            break;
        case 0xE3D8AB51:
            netFrameLock = true;
            break;
        case 0xE51023BA:
            netTeamDamage = false;
            break;
        case 0xE7347B1A:
            dragEnabled = true;
            dragLoss = atoi(apcArgv[++i]);
            break;
        case 0xF2FB8359:
            dragEnabled = true;
            dragLatency = atoi(apcArgv[++i]);
            break;
        case 0xF4553EF1:
            g_bNoSound = true;
            break;
        case 0xF5DD1FBD:
            pcForceFixedFunction = true;
            break;
        case 0xF6976B61:
            gNoPregame = true;
            break;
        }
    }

    if (bNamed)
    {
        ShellLoop::SetLegalShowtype(ShellLoop::GetLegalShowtype() == ShellLoop::eLEGALSCREENS_SKIP
                                        ? ShellLoop::eLEGALSCREENS_SKIP
                                        : ShellLoop::eLEGALSCREENS_SHOWFAST);
    }
}

// FUNCTION: SWBF2 0x0047a550
float __fastcall ReadSpawnCameraFirstValue(void* pValue)
{
    return *(float*)pValue;
}

// FUNCTION: SWBF2 0x0047a560
float __fastcall ReadSpawnCameraSecondValue(void* pValue)
{
    return static_cast<SpawnCameraFloatPair*>(pValue)->mSecondValue;
}

// GLOBAL: SWBF2 0x01e2fdbb

bool gAIOptionEnabled = false;

// GLOBAL: SWBF2 0x01e56281

bool gSoundWarningEnabled = false;

// GLOBAL: SWBF2 0x01e65033

bool gShowAllSessions = false;

// GLOBAL: SWBF2 0x01eaefc5

bool gProfileOptionEnabled = false;

// GLOBAL: SWBF2 0x007e802e

bool gUseValues = true;
