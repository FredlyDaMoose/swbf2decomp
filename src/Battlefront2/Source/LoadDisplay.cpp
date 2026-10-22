#include "LoadDisplay.h"

#include <float.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "CameraManager.h"
#include "ColorControl.h"
#include "GameLoop.h"
#include "GameState.h"
#include "HDREffect.h"
#include "LoadUtil.h"
#include "MissionPlayList.h"
#include "NetSetup.h"
#include "ScreenTransition.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblJournal.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblTime.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/RedDirectionalLight.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedMaterial.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"

namespace Lua_Callbacks { bool IsTrainingMission(); }
namespace Lua_Callbacks { bool IsMetaGameStateSaved(); }
namespace Lua_Callbacks { void __fastcall GetMissionWorld(char* pcWorld, int); }

namespace PblProfile { bool IsPaused(); }
namespace PblProfile { void SetPaused(bool bValue); }

namespace GameLoop { extern bool mbHistoricalRules; }

extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

const RedColor Interpolate(const RedColor& a, const RedColor& b, float t);

namespace
{

RedColorValue& AmbientTopColor()
{
    return RedInterfaceScreen::s_lightingState.ambientLight.m_topColor;
}
RedColorValue& AmbientBottomColor()
{
    return RedInterfaceScreen::s_lightingState.ambientLight.m_bottomColor;
}
void SetAmbientColors(const RedColorValue& top, const RedColorValue& bottom)
{
    AmbientTopColor() = top;
    AmbientBottomColor() = bottom;
    RedColorValue& color = RedInterfaceScreen::s_lightingState.ambientLight.m_color;
    color.r = (top.r + bottom.r) * 0.5f;
    color.g = (top.g + bottom.g) * 0.5f;
    color.b = (top.b + bottom.b) * 0.5f;
    color.a = (top.a + bottom.a) * 0.5f;
}

RedColorValue ToColorValue(const RedColor& c)
{
    RedColorValue v;
    v.r = c.r * (1.0f / 255.0f);
    v.g = c.g * (1.0f / 255.0f);
    v.b = c.b * (1.0f / 255.0f);
    v.a = c.a * (1.0f / 255.0f);
    return v;
}
}

static const unsigned int MISSION_SHELL = 0x11e1fc01;
static const unsigned int MISSION_GAL_CAM = 0x906d4f8b;
static const unsigned int MISSION_GAL_CAM_SHELL = 0x20550dee;
static const unsigned int MISSION_GAL_CON = 0x91554c38;
static const unsigned int MISSION_GAL_CON_SHELL = 0xdff3de01;

static const float DEFAULT_TEAM_MODEL_OMEGA = 0.6f;
static const float DEFAULT_LOADING_TEXT_BLINK_RATE = 1.0f;
static const float DEFAULT_PROGRESS_TIME = 10.0f;
static const float DEFAULT_LED_HEIGHT = 10.0f;
static const float DEFAULT_LED_WIDTH = 5.0f;
static const float DEFAULT_LED_SPACING = 5.0f;
static const float LOADING_TIPS_POSITION_X = 0.35f;
static const float LOADING_TIPS_POSITION_Y = 0.18f;

// GLOBAL: SWBF2 0x0079193c
static const RedColor DEFAULT_LED_COLOR = { 0xc8, 0xc8, 0xc8, 0xff };

// GLOBAL: SWBF2 0x007ebb38
static RedColor MISSION_NAME_COLOR = { 0xc8, 0xc8, 0xc8, 0xff };
// GLOBAL: SWBF2 0x007ebb1c
static RedColor MODE_NAME_COLOR = { 0xc8, 0xc8, 0xc8, 0xff };

// GLOBAL: SWBF2 0x007ebb18
static RedColor DEFAULT_LEVELTIPS_TEXT_COLOR = { 0xc8, 0xc8, 0xff, 0xff };

// GLOBAL: SWBF2 0x01ec6108
// SYNTHETIC: SWBF2 0x00404120
// `dynamic initializer for 'DEFAULT_LED_TEXTURE_HASH''
static unsigned int DEFAULT_LED_TEXTURE_HASH = PblHash("white");

// GLOBAL: SWBF2 0x007e69d8
float LED_OFF_INTENSITY = 0.5f;

// GLOBAL: SWBF2 0x01f9c2e4
static int s_loadHeap;

// GLOBAL: SWBF2 0x01efcf3f
static bool s_profilePaused;

// GLOBAL: SWBF2 0x01e573f0
int LoadDisplay::m_ShowCount;

// GLOBAL: SWBF2 0x01faaa70
static unsigned __int64 load_update_qpc_stamp;

LoadDisplay* LoadDisplay::sInstance;

// GLOBAL: SWBF2 0x01ec6410
// SYNTHETIC: SWBF2 0x00404140
// `dynamic initializer for 'LoadDisplay::sTipsDatabase''
// SYNTHETIC: SWBF2 0x00768c60
// `dynamic atexit destructor for 'LoadDisplay::sTipsDatabase''
LoadDisplay::IDDatabase LoadDisplay::sTipsDatabase;
// GLOBAL: SWBF2 0x01ec70a0
// SYNTHETIC: SWBF2 0x00404160
// `dynamic initializer for 'LoadDisplay::sLVLDatabase''
// SYNTHETIC: SWBF2 0x00768c70
// `dynamic atexit destructor for 'LoadDisplay::sLVLDatabase''
LoadDisplay::IDDatabase LoadDisplay::sLVLDatabase;

bool IsJournaling()
{
    return PblJournal::Get(PJT_FULL)->mMode == PJM_READING ||
           PblJournal::Get(PJT_FULL)->mMode == PJM_WRITING;
}

static int GetLocalizedTeamNum(unsigned int uiTeamHash)
{
    switch (uiTeamHash)
    {
    case 0xe40c292c:
        return 0;
    case 0xf70c4715:
        return 1;
    case 0xe60c2c52:
        return 2;
    case 0xec0c35c4:
        return 3;
    default:
        return -1;
    }
}

// FUNCTION: SWBF2 0x00575550
int GetTeamNum(unsigned int uiTeamHash)
{
    switch (uiTeamHash)
    {
    case 0x13254bc4:
        return 0;
    case 0x2cf46160:
        return 1;
    case 0xf387c5d6:
        return 2;
    case 0x93e73ca1:
        return 3;
    default:
        return -1;
    }
}

// FUNCTION: SWBF2 0x00575590
void AddTipsToDatabase(LoadDisplay::IDDatabase& database, const char* pcName)
{
    char acBuffer[256];
    _snprintf(acBuffer, sizeof(acBuffer) - 1, "%s.%d", pcName, 1);
    acBuffer[sizeof(acBuffer) - 1] = 0;
    unsigned int uiHash = PblHash(acBuffer);
    for (int i = 2; RedLocalizeDB::Find(uiHash, true); ++i)
    {
        database.AddID(0, uiHash);
        _snprintf(acBuffer, sizeof(acBuffer) - 1, "%s.%d", pcName, i);
        acBuffer[sizeof(acBuffer) - 1] = 0;
        uiHash = PblHash(acBuffer);
    }
}

// FUNCTION: SWBF2 0x00575640
void LoadDisplay::Create()
{
    s_loadHeap = __RedCurrHeap;
    sInstance = new LoadDisplay;
}

// FUNCTION: SWBF2 0x005756f0
LoadDisplay::LoadDisplay()
{
    if (!g_bNoRender)
    {
        float fWidth, fHeight, fOffsetX, fOffsetY;
        RedInterfaceScreen::s_screenFull.GetSafeZone(fWidth, fHeight, fOffsetX, fOffsetY);
        m_loadingScreen.SetSafeZone(fWidth, fHeight, fOffsetX, fOffsetY);
        m_loadingScreen.SetDimensions(RedInterfaceScreen::GetScreenWidth(),
                                      RedInterfaceScreen::GetScreenHeight());
        m_bDisplay = false;
        m_campaignLayout = false;
        m_redCamera = 0;
        m_missionFileName[0] = 0;
        m_entireMissionFileName[0] = 0;
        BuildSimpleLoadingBar();
        BuildLoadingTips();
        BuildCampaignLayout();
    }
}

// FUNCTION: SWBF2 0x005756b0
void LoadDisplay::Destroy()
{
    delete sInstance;
    s_loadHeap = 0;
    sInstance = 0;
}

// FUNCTION: SWBF2 0x00575900
LoadDisplay::~LoadDisplay()
{
    ++LoadDisplay::m_ShowCount;
    sInstance = 0;
    if (!g_bNoRender)
    {
        delete m_redCamera;
    }
}

// FUNCTION: SWBF2 0x00575a90
void LoadDisplay::SetTextElementDefaults(RedTextElement& element)
{
    element.Enable(true);
    element.SetFont(RedFont::GAMEFONT_SMALL);
    element.SetTextScale(1.0f, 1.0f);
    element.SetIntercharacterSpacing(0);
    element.SetColor(DEFAULT_LEVELTIPS_TEXT_COLOR);
    element.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_TOP);
    element.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
    element.SetPosition(0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x00575b30
void LoadDisplay::BuildCampaignLayout()
{
    const float fWidth = (float)RedInterfaceScreen::GetScreenWidth();
    const float fHeight = (float)RedInterfaceScreen::GetScreenHeight();
    const RedColor color = { 0xc8, 0xc8, 0xc8, 0xff };

    SetTextElementDefaults(m_textCampaignTitle);
    m_textCampaignTitle.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER,
                                     RedTextElement::TEXT_VALIGN_CENTER);
    m_textCampaignTitle.SetFont(RedFont::GAMEFONT_LARGE);
    m_textCampaignTitle.SetPosition(fWidth * 0.637f, fHeight * 0.245f);
    m_textCampaignTitle.SetColor(color);

    SetTextElementDefaults(m_textCampaignBriefing);
    m_textCampaignBriefing.SetFont(RedFont::GAMEFONT_MEDIUM);
    m_textCampaignBriefing.SetPosition(fWidth * 0.41f, fHeight * 0.34f);
    m_textCampaignBriefing.SetColor(color);
    m_textCampaignBriefing.SetTextBox((unsigned int)(fWidth * 0.515f),
                                      (unsigned int)(fHeight * 0.6f));

    m_groupCampaign.AddElement(&m_textCampaignTitle);
    m_groupCampaign.AddElement(&m_textCampaignBriefing);
}

// FUNCTION: SWBF2 0x00575c90
void LoadDisplay::BuildLoadingTips()
{
    const unsigned int uiWidth = RedInterfaceScreen::GetScreenWidth();
    const unsigned int uiHeight = RedInterfaceScreen::GetScreenHeight();

    SetTextElementDefaults(m_textLevelTips);
    m_textLevelTips.SetTextBox((unsigned int)((float)uiWidth * 0.525f),
                               (unsigned int)((float)uiHeight * 0.64f));
    m_groupLoadingTips.AddElement(&m_tipsBox);
    m_groupLoadingTips.AddElement(&m_textLevelTips);
}

// FUNCTION: SWBF2 0x00575d30
void LoadDisplay::BuildSimpleLoadingBar()
{
    m_textLoading.Enable(true);
    m_textLoading.SetFont(RedFont::GAMEFONT_LARGE);
    m_textLoading.SetTextScale(1.0f, 1.0f);
    m_textLoading.SetIntercharacterSpacing(0);
    m_textLoading.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_BOTTOM);
    m_textLoading.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
    m_textLoading.SetPosition(0.0f, 0.0f);
    m_textLoading.SetText(RedLocalizeDB::Find(PblHash("common.loading"), false), true);
    m_groupBottomLeft.AddElement(&m_textLoading);

    m_textMissionName.Enable(true);
    m_textMissionName.SetColor(MISSION_NAME_COLOR);
    m_textMissionName.SetFont(RedFont::GAMEFONT_LARGE);
    m_textMissionName.SetTextScale(1.0f, 1.0f);
    m_textMissionName.SetIntercharacterSpacing(0);
    m_textMissionName.SetTextAlign(RedTextElement::TEXT_HALIGN_LEFT, RedTextElement::TEXT_VALIGN_TOP);
    m_textMissionName.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
    m_textMissionName.SetPosition(0.0f, 0.0f);
    m_textMissionName.SetText(RedLocalizeDB::Find(m_missionNameHash, true), true);
    m_groupTopLeft.AddElement(&m_textMissionName);

    m_textModeName.Enable(true);
    m_textModeName.SetColor(MODE_NAME_COLOR);
    m_textModeName.SetFont(RedFont::GAMEFONT_LARGE);
    m_textModeName.SetTextScale(1.0f, 1.0f);
    m_textModeName.SetIntercharacterSpacing(0);
    m_textModeName.SetTextAlign(RedTextElement::TEXT_HALIGN_RIGHT, RedTextElement::TEXT_VALIGN_TOP);
    m_textModeName.SetTextStyle(RedGlyphCacheElement::TEXT_STYLE_NORMAL);
    m_textModeName.SetPosition(0.0f, 0.0f);
    m_textModeName.SetText(RedLocalizeDB::Find(m_modeNameHash, true), true);
    m_groupTopRight.AddElement(&m_textModeName);

    for (int i = 0; i < 4; ++i)
    {
        m_modelTeamIcon[i].Enable(false);
        RedColor color = RedColor::WHITE;
        color.a = 255;
        m_modelTeamIcon[i].SetColor(color);
        m_modelTeamIcon[i].SetDepth(0.2f);
        m_modelTeamIcon[i].SetModelScale(1.0f);
        m_modelTeamIcon[i].SetPosition(0.0f, 0.0f);
        m_modelTeamIcon[i].EnableLighting(true);
    }

    m_groupBottomLeft.AddElement(&m_progressBar);
}

// FUNCTION: SWBF2 0x00575ff0
void LoadDisplay::PostLoad()
{
    m_loadingScreen.AddElement(&m_groupCampaign);
    m_groupCampaign.SetZOrder(0);
    m_groupCampaign.SetUseSafezone(false);
    m_groupCampaign.SetScreenRelativePos(0.0f, 0.0f);
    if (RedRenderer::GetScreenAspectRatio() < 0.75f)
    {
        m_groupCampaign.SetUsePerspectiveHack(true);
    }

    m_groupLoadingTips.SetZOrder(0);
    m_loadingScreen.AddElement(&m_groupLoadingTips);
    m_groupLoadingTips.SetScreenRelativePos(LOADING_TIPS_POSITION_X, LOADING_TIPS_POSITION_Y);
    m_textLoading.SetZOrder(0);

    m_loadingScreen.AddElement(&m_groupTitleBar);
    m_groupTitleBar.SetZOrder(0);
    m_groupTitleBar.SetUseSafezone(true);
    m_groupTitleBar.SetScreenRelativePos(0.5f, 0.0f);

    m_loadingScreen.AddElement(&m_groupTopLeft);
    m_groupTopLeft.SetZOrder(0);
    m_groupTopLeft.SetUseSafezone(true);
    m_groupTopLeft.SetScreenRelativePos(0.0f, 0.0f);

    m_loadingScreen.AddElement(&m_groupTopRight);
    m_groupTopRight.SetZOrder(0);
    m_groupTopRight.SetUseSafezone(true);
    m_groupTopRight.SetScreenRelativePos(1.0f, 0.0f);

    m_loadingScreen.AddElement(&m_groupBottomLeft);
    m_groupBottomLeft.SetZOrder(0);
    m_groupBottomLeft.SetUseSafezone(true);
    m_groupBottomLeft.SetScreenRelativePos(0.0f, 1.0f);

    m_loadingScreen.AddElement(&m_groupBottomRight);
    m_groupBottomRight.SetZOrder(0);
    m_groupBottomRight.SetUseSafezone(true);
    m_groupBottomRight.SetScreenRelativePos(1.0f, 1.0f);

    if (m_team1Num >= 0)
    {
        m_groupBottomRight.AddElement(&m_modelTeamIcon[m_team1Num]);
        m_modelTeamIcon[m_team1Num].Enable(false);
        m_modelTeamIcon[m_team1Num].m_OmegaY = m_teamModelOmega;
        m_modelTeamIcon[m_team1Num].SetPosition(-100.0f, 0.0f, 0.0f);
    }
    if (m_team2Num >= 0)
    {
        m_groupBottomRight.AddElement(&m_modelTeamIcon[m_team2Num]);
        m_modelTeamIcon[m_team2Num].Enable(false);
        m_modelTeamIcon[m_team2Num].m_OmegaY = -m_teamModelOmega;
        m_modelTeamIcon[m_team2Num].SetPosition(100.0f, 0.0f, 0.0f);
    }

    if (RedLocalizeDB::Find(m_missionNameHash, true))
    {
        m_textMissionName.SetText(RedLocalizeDB::Find(m_missionNameHash, true), true);
    }
    else
    {
        m_textMissionName.SetText("", true);
    }
    if (RedLocalizeDB::Find(m_modeNameHash, true))
    {
        m_textModeName.SetText(RedLocalizeDB::Find(m_modeNameHash, true), true);
    }
    else
    {
        m_textModeName.SetText("", true);
    }

    m_progressBar.SetLEDSize(m_LEDWidth, m_LEDHeight);
    m_progressBar.Init(m_numLEDs, m_LEDSpacing, &m_LEDTextureHash, 1, &m_LEDColor, 1,
                       (float)m_numLEDs / m_totalProgressTime,
                       ProgressIndicator::LED_MODE_TIMER);
    m_progressBar.Enable(true);
    m_progressBar.SetPosition((float)m_textLoading.m_iTextExtent + 10.0f, -10.0f);

    float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
    m_tipsBox.GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
    m_textLevelTips.SetZOrder(0);
}

// FUNCTION: SWBF2 0x005763f0
void LoadDisplay::SetLoadState()
{
    if (GameState::GetState() == MISSION_SHELL)
    {
        if (Lua_Callbacks::IsTrainingMission())
        {
            m_missionHash = MISSION_SHELL;
        }
        else if (GameLoop::AreMetagameRulesOn())
        {
            m_missionHash = Lua_Callbacks::IsMetaGameStateSaved() ? MISSION_GAL_CON : MISSION_GAL_CON_SHELL;
        }
        else if (GameLoop::mbHistoricalRules)
        {
            m_missionHash = Lua_Callbacks::IsMetaGameStateSaved() ? MISSION_GAL_CAM : MISSION_GAL_CAM_SHELL;
        }
        else
        {
            m_missionHash = MISSION_SHELL;
        }

        m_entireMissionHash = m_missionHash;
        m_team1Num = -1;
        m_team2Num = -1;
        m_missionFileName[0] = 0;
        m_entireMissionFileName[0] = 0;
        m_campaignNameHash = 0;
        return;
    }

    strlcpy(m_missionFileName, GameLoop::mMissionScript, sizeof(m_missionFileName));
    strlcpy(m_entireMissionFileName, GameLoop::mMissionScript, sizeof(m_entireMissionFileName));
    m_entireMissionHash = PblHash(m_missionFileName);

    char* pcMode = "";
    char* pcUnderscore = strchr(m_missionFileName, '_');
    if (pcUnderscore)
    {
        *pcUnderscore = 0;
        pcMode = pcUnderscore + 1;
    }
    unsigned int uiLen = strlen(m_missionFileName);
    if (uiLen > 1)
    {
        m_missionFileName[uiLen - 1] = 0;
    }
    m_missionHash = PblHash(m_missionFileName);

    char acBuffer[256];
    if (GameLoop::AreMetagameRulesOn() && !_strnicmp(m_missionFileName, "spa", 3))
    {
        char acWorld[4];
        Lua_Callbacks::GetMissionWorld(acWorld, 0);
        sprintf(acBuffer, "mapname.name.spa_%s", acWorld);
    }
    else
    {
        sprintf(acBuffer, "mapname.name.%s", m_missionFileName);
    }
    m_missionNameHash = PblHash(acBuffer);

    if (!_stricmp(pcMode, "eli"))
    {
        strcpy(pcMode, "ass");
    }
    sprintf(acBuffer, "modename.name.%s", pcMode);
    m_modeNameHash = PblHash(acBuffer);

    unsigned int uiTeam1Hash, uiTeam2Hash;
    MissionPlayList::GetCurrentTeamNames(uiTeam1Hash, uiTeam2Hash);
    m_team1Num = GetLocalizedTeamNum(uiTeam1Hash);
    m_team2Num = GetLocalizedTeamNum(uiTeam2Hash);
}

// FUNCTION: SWBF2 0x00576660
void LoadDisplay::Begin()
{
    if (IsJournaling() || g_bNoRender)
    {
        return;
    }

    s_profilePaused = PblProfile::IsPaused();
    PblProfile::SetPaused(true);

    m_totalTime = 0.0f;
    m_loadState = LOAD_STATE_LOADING;
    m_teamModelOmega = DEFAULT_TEAM_MODEL_OMEGA;
    m_loadingTextBlinkRate = DEFAULT_LOADING_TEXT_BLINK_RATE;
    m_loadingColorPalleteSize = 0;
    for (int i = 0; i < 16; ++i)
    {
        m_loadingColorPallete[i] = RedColor::WHITE;
    }
    m_timePerTip = FLT_MAX;
    sTipsDatabase.Initialize();
    sLVLDatabase.Initialize();
    m_LEDColor = DEFAULT_LED_COLOR;
    m_LEDTextureHash = DEFAULT_LED_TEXTURE_HASH;
    m_totalProgressTime = DEFAULT_PROGRESS_TIME;
    m_LEDHeight = DEFAULT_LED_HEIGHT;
    m_LEDWidth = DEFAULT_LED_WIDTH;
    m_LEDSpacing = DEFAULT_LED_SPACING;
    m_numLEDs = 20;
    LED_OFF_INTENSITY = 0.5f;
    m_randomBackDropFileName[0] = 0;

    SetLoadState();
    LoadData();

    m_groupBottomRight.Enable(true);
    m_groupBottomLeft.Enable(true);
    m_groupTopRight.Enable(true);
    m_groupTopLeft.Enable(true);
    m_groupTitleBar.Enable(true);
    m_groupLoadingTips.Enable(false);
    m_groupCampaign.Enable(false);

    if (m_missionHash != MISSION_SHELL && m_missionHash != MISSION_GAL_CAM_SHELL &&
        m_missionHash != MISSION_GAL_CAM)
    {
        if (m_campaignLayout)
        {
            char acBuffer[256];
            _snprintf(acBuffer, sizeof(acBuffer) - 1, "ifs.briefing.%s", m_entireMissionFileName);
            acBuffer[sizeof(acBuffer) - 1] = 0;
            m_groupCampaign.Enable(true);
            m_textCampaignTitle.SetText(RedLocalizeDB::Find(m_campaignNameHash, false), true);
            m_textCampaignBriefing.SetText(RedLocalizeDB::Find(PblHash(acBuffer), false), true);
        }
        else
        {
            m_groupLoadingTips.Enable(true);
            m_textLevelTips.SetText(RedLocalizeDB::Find(sTipsDatabase.GetRandomID(), false), true);
            float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
            m_tipsBox.GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
            m_textLevelTips.GetRect(fLeft, fTop, fRight, fBottom);
            m_tipsBox.SetBorderRect(fLeft - fBorderX, fTop - fBorderY, fRight + fBorderX,
                                      fBottom + fBorderY, fBorderX, fBorderY);
        }
    }

    m_tipTimer = m_timePerTip;

    delete m_redCamera;
    m_redCamera = new RedCamera(*RedRenderer::GetCamera());
    m_cameraMat = gMatrixIdentity;

    PblMatrix mCamera;
    mCamera.m[0][0] = m_cameraMat.m[0][0];
    mCamera.m[0][1] = m_cameraMat.m[0][1];
    mCamera.m[0][2] = m_cameraMat.m[0][2];
    mCamera.m[0][3] = 0.0f;
    mCamera.m[1][0] = m_cameraMat.m[1][0];
    mCamera.m[1][1] = m_cameraMat.m[1][1];
    mCamera.m[1][2] = m_cameraMat.m[1][2];
    mCamera.m[1][3] = 0.0f;
    mCamera.m[2][0] = -m_cameraMat.m[2][0];
    mCamera.m[2][1] = -m_cameraMat.m[2][1];
    mCamera.m[2][2] = -m_cameraMat.m[2][2];
    mCamera.m[2][3] = 0.0f;
    mCamera.m[3][0] = m_cameraMat.m[3][0];
    mCamera.m[3][1] = m_cameraMat.m[3][1];
    mCamera.m[3][2] = m_cameraMat.m[3][2];
    mCamera.m[3][3] = 1.0f;
    m_redCamera->SetMatrix(mCamera);
    m_redCamera->SetNearFarPlane(0.1f, 3000.0f);

    PostLoad();

    m_bDisplay = true;
    m_firstUpdate = true;
    m_titleBarTimer = 0.0f;
    Update();
    Update();
}

// FUNCTION: SWBF2 0x00576b90
void LoadDisplay::End()
{
    if (IsJournaling() || g_bNoRender)
    {
        return;
    }

    m_progressBar.SetAllOn();
    Render();
    m_loadState = LOAD_STATE_FINISHED;
    DeleteData();
    m_bDisplay = false;
    PblProfile::SetPaused(s_profilePaused);
}

// FUNCTION: SWBF2 0x00576c00
void LoadDisplay::Update()
{
    if (IsJournaling() || g_bNoRender || !m_bDisplay)
    {
        return;
    }

    if (m_firstUpdate)
    {
        load_update_qpc_stamp = PblTime::GetTickCount();
        m_firstUpdate = false;
    }
    unsigned __int64 uiNow = PblTime::GetTickCount();
    float fDelta = (float)((double)(uiNow - load_update_qpc_stamp) / (double)PblTime::ticksPerSecond);
    if (fDelta < 0.05f)
    {
        return;
    }

    int iHeap = RedSetCurrentHeap(s_loadHeap);
    load_update_qpc_stamp = uiNow;
    m_totalTime += fDelta;
    m_tipTimer -= fDelta;
    if (m_tipTimer <= 0.0f)
    {
        m_tipTimer = m_timePerTip;
        m_textLevelTips.SetText(RedLocalizeDB::Find(sTipsDatabase.GetRandomID(), false), true);
        float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
        m_tipsBox.GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
        m_textLevelTips.GetRect(fLeft, fTop, fRight, fBottom);
        m_tipsBox.SetBorderRect(fLeft - fBorderX, fTop - fBorderY, fRight + fBorderX,
                                  fBottom + fBorderY, fBorderX, fBorderY);
    }

    ScreenTransition::sInstance->Update(fDelta);

    float fBlink = PblMath::Sin(m_loadingTextBlinkRate * PblMath::fTwoPi * m_totalTime);
    float fIndex = (fBlink * 0.5f + 0.5f) * (float)(m_loadingColorPalleteSize - 1);
    int iIndex = (int)fIndex;
    m_textLoading.SetColor(Interpolate(m_loadingColorPallete[iIndex],
                                       m_loadingColorPallete[PblMath::Ceiling(fIndex)],
                                       fIndex - (float)iIndex));

    m_progressBar.Update(fDelta, 0, 0);
    Render();
    RedSetCurrentHeap(iHeap);
}

// FUNCTION: SWBF2 0x00576f10
void LoadDisplay::Render()
{
    const RedCamera* pCamera = RedRenderer::GetCamera();
    CameraManager::Instance()->SetNumCameras(1);

    RedDirectionalLight& light = RedInterfaceScreen::s_directionalLight[0];
    RedColorValue savedTop = AmbientTopColor();
    RedColorValue savedBottom = AmbientBottomColor();
    PblVector3 savedDirection = light.m_vDirection;
    RedColorValue savedColor = light.m_color;
    bool bColorControl = ColorControl::s_pInstance->IsEnabled();
    bool bHDR = HDREffect::Instance()->IsEnabled();

    RedRenderer::SetCamera(m_redCamera);

    RedColorValue ambient = ToColorValue(m_ambientColor);
    SetAmbientColors(ambient, ambient);
    light.SetDirection(m_sunDirection);
    light.SetColor(ToColorValue(m_sunColor));

    ColorControl::s_pInstance->Enable(false);
    HDREffect::Instance()->Enable(false);
    PlatformRender();
    ColorControl::s_pInstance->Enable(bColorControl);
    HDREffect::Instance()->Enable(bHDR);

    RedRenderer::SetCamera(pCamera);
    SetAmbientColors(savedTop, savedBottom);
    light.SetDirection(savedDirection);
    light.SetColor(savedColor);
    CameraManager::Instance()->SetNumCameras(GameLoop::mNumCameras);
}

// FUNCTION: SWBF2 0x00577280
void LoadDisplay::RenderScreen() const
{
    PlatformRenderTexture(m_backDropHash, 0.0f, 0.0f, 1.0f, 1.0f, RedColor::WHITE, false, 0.0f,
                          0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f);
}

// FUNCTION: SWBF2 0x005772f0
void LoadDisplay::DeleteData()
{
    int iHeap = RedSetCurrentHeap(__RedTempHeap);
    for (int i = 0; i < m_numTextures; ++i)
    {
        delete m_textures[i];
        m_textures[i] = 0;
    }
    m_numTextures = 0;
    for (int i = 0; i < m_numSkeletons; ++i)
    {
        m_skeletons[i] = 0;
    }
    m_numSkeletons = 0;
    RedSetCurrentHeap(iHeap);
}

// FUNCTION: SWBF2 0x005773a0
void LoadDisplay::LoadData()
{
    m_numModels = 0;
    m_numTextures = 0;
    m_numSkeletons = 0;
    int iHeap = RedSetCurrentHeap(__RedTempHeap);
    RedModel::pcSetShareBufferEnable(false);
    LoadDataFile("Load\\load");
    if (m_randomBackDropFileName[0])
    {
        m_backDropHash = LoadRandomLVLChunk(m_randomBackDropFileName, sLVLDatabase);
    }
    RedModel::pcSetShareBufferEnable(true);
    RedSetCurrentHeap(iHeap);
}

// FUNCTION: SWBF2 0x00577410
unsigned int LoadDisplay::LoadRandomLVLChunk(const char* pcFileName, IDDatabase& database)
{
    PblFile file;
    char acFileName[256];
    LoadUtil::MakeFullName(pcFileName, LoadUtil::FILE_TYPE_LEVEL, acFileName, sizeof(acFileName),
                           false);
    if (!PblFile::Exists(acFileName) || !file.Open(acFileName, true, false, true))
    {
        return 0;
    }

    PblFileChunk chunk(&file);
    const unsigned int uiParent = PblHash(pcFileName);
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x5f6c766c)
        {
            while (child.HasMoreChildren())
            {
                PblFileChunk lvl = child.ReadNextChild();
                database.AddID(uiParent, lvl._iId);
            }
        }
    }

    file.Seek(0, PblFile::SeekFromStart);
    PblFileChunk root(&file);
    unsigned int uiHash = 0;
    const unsigned int uiRandomID = database.GetRandomID();
    while (root.HasMoreChildren())
    {
        PblFileChunk child = root.ReadNextChild();
        if (child._iId == 0x5f6c766c)
        {
            while (child.HasMoreChildren())
            {
                PblFileChunk lvl = child.ReadNextChild();
                if (lvl._iId == uiRandomID)
                {
                    uiHash = lvl._iId;
                    LoadDataChunk(lvl);
                    break;
                }
            }
        }
    }
    file.Close();
    return uiHash;
}

// FUNCTION: SWBF2 0x00577620
void LoadDisplay::LoadDataFile(const char* pcFileName)
{
    PblFile file;
    char acFileName[256];
    LoadUtil::MakeFullName(pcFileName, LoadUtil::FILE_TYPE_LEVEL, acFileName, sizeof(acFileName),
                           false);
    if (PblFile::Exists(acFileName) && file.Open(acFileName, true, false, true))
    {
        PblFileChunk chunk(&file);
        LoadDataChunk(chunk);
        file.Close();
    }
}

// FUNCTION: SWBF2 0x005776e0
void LoadDisplay::LoadDataChunk(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x6c646f6d:
            m_models[m_numModels++] = RedModel::Read(child);
            break;
        case 0x5f786574:
        {
            RedTexture* pTexture = RedTexture::Read(child);
            int i;
            for (i = 0; i < m_numTextures; ++i)
            {
                if (m_textures[i] == pTexture)
                {
                    break;
                }
            }
            if (i >= m_numTextures)
            {
                m_textures[m_numTextures++] = pTexture;
            }
            break;
        }
        case 0x64616f6c:
            LoadConfig(child);
            break;
        case 0x6c656b73:
            m_skeletons[m_numSkeletons++] = RedSkeleton::Read(child);
            break;
        }
    }
}

namespace
{
enum ConfigKeyT
{
    CFG_LOADDISPLAY = 0x8689c861,
    CFG_WORLD = 0x37a3e893,
    CFG_MAP = 0xdfa2efb1,
    CFG_BACKDROP = 0x93c0f4f7,
    CFG_RANDOMBACKDROP = 0x97d04fa8,
    CFG_SUNCOLOR = 0x664a1f44,
    CFG_SUNDIRECTION = 0xe1907cf6,
    CFG_AMBIENTCOLOR = 0x627ebb4a,
    CFG_TEAMMODEL = 0xd6c2b5f9,
    CFG_TEAMMODELROTATIONSPEED = 0x26455a06,
    CFG_TIPSTIME = 0x55c13372,
    CFG_TIPSBOXTEXTURE = 0x6c30ed87,
    CFG_TIPSCOLOR = 0xd1eb1b5a,
    CFG_TIPS = 0xc5ad880f,
    CFG_PROGRESSBARTOTALTIME = 0x31a6bc76,
    CFG_PROGRESSBARNUMLEDS = 0xee4ccfc3,
    CFG_PROGRESSBARLEDCOLOR = 0x533f1e37,
    CFG_PROGRESSBARLEDTEXTURE = 0x0925227b,
    CFG_PROGRESSBARLEDWIDTH = 0xc53b850c,
    CFG_PROGRESSBARLEDHEIGHT = 0x836ca5fb,
    CFG_PROGRESSBARLEDSPACING = 0x4d649461,
    CFG_LEDOFFINTENSITY = 0x904a07ed,
    CFG_LOADINGTEXTBLINKRATE = 0xdfe9346c,
    CFG_LOADINGTEXTCOLORPALLETE = 0xa6fb2870,
    CFG_COLOR = 0x3d7e6258,
    CFG_CAMPAIGNLAYOUT = 0x001de6b9,
    CFG_CAMPAIGNNAME = 0xd169eaf8
};

RedColor ConfigColor(const PblConfig::Data& data)
{
    RedColor color;
    color.r = data.GetUint8Arg(0);
    color.g = data.GetUint8Arg(1);
    color.b = data.GetUint8Arg(2);
    color.a = data.GetNumArgs() > 3 ? data.GetUint8Arg(3) : 0xff;
    return color;
}

void CopyBackDropName(char (&acDest)[64], const char* pcName)
{
    if (strlen(pcName) <= sizeof(acDest) - 1)
    {
        strcpy(acDest, pcName);
    }
}
}

// FUNCTION: SWBF2 0x005777e0
void LoadDisplay::LoadConfig(PblFileChunk& chunk)
{
    PblConfig config(chunk);
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == CFG_LOADDISPLAY)
        {
            PblConfig scope(config.ReadNextScope(), true);
            while (!scope.IsDone())
            {
                PblConfig::Data item = scope.ReadNextData();
                switch (item.GetId())
                {
                case CFG_PROGRESSBARTOTALTIME:
                    m_totalProgressTime = item.GetFloatArg(0);
                    break;
                case CFG_TEAMMODELROTATIONSPEED:
                    m_teamModelOmega = item.GetFloatArg(0);
                    break;
                case CFG_PROGRESSBARLEDTEXTURE:
                    m_LEDTextureHash = PblHash(item.GetStringArg(0));
                    break;
                case CFG_PROGRESSBARLEDSPACING:
                    m_LEDSpacing = item.GetFloatArg(0);
                    break;
                case CFG_PROGRESSBARLEDCOLOR:
                    m_LEDColor = ConfigColor(item);
                    break;
                case CFG_TIPSTIME:
                    m_timePerTip = item.GetFloatArg(0);
                    break;
                case CFG_AMBIENTCOLOR:
                    m_ambientColor.r = item.GetUint8Arg(0);
                    m_ambientColor.g = item.GetUint8Arg(1);
                    m_ambientColor.a = 0xff;
                    m_ambientColor.b = item.GetUint8Arg(2);
                    break;
                case CFG_SUNCOLOR:
                    m_sunColor.r = item.GetUint8Arg(0);
                    m_sunColor.g = item.GetUint8Arg(1);
                    m_sunColor.a = 0xff;
                    m_sunColor.b = item.GetUint8Arg(2);
                    break;
                case CFG_LEDOFFINTENSITY:
                    LED_OFF_INTENSITY = item.GetFloatArg(0);
                    break;
                case CFG_PROGRESSBARLEDHEIGHT:
                    m_LEDHeight = item.GetFloatArg(0);
                    break;
                case CFG_TIPSBOXTEXTURE:
                {
                    const int iHeap = RedSetCurrentHeap(s_loadHeap);
                    m_tipsBox.SetTextureHash(PblHash(item.GetStringArg(0)));
                    float fLeft, fTop, fRight, fBottom, fBorderX, fBorderY;
                    m_tipsBox.GetBorderRect(fLeft, fTop, fRight, fBottom, fBorderX, fBorderY);
                    const float fNewBorderX = item.GetFloatArg(1);
                    const float fNewBorderY = item.GetFloatArg(2);
                    m_tipsBox.SetBorderRect(fLeft + fBorderX - fNewBorderX,
                                              fTop + fBorderY - fNewBorderY,
                                              fRight - fBorderX + fNewBorderX,
                                              fBottom - fBorderY + fNewBorderY, fNewBorderX,
                                              fNewBorderY);
                    const RedTexture* pTexture = m_tipsBox.GetTexture();
                    if (pTexture)
                    {
                        m_tipsBox.SetBorderTexCoords(0.0f, 0.0f, 1.0f, 1.0f,
                                                  fNewBorderX / (float)pTexture->m_uiWidth,
                                                  fNewBorderY / (float)pTexture->m_uiHeight, 0);
                    }
                    RedSetCurrentHeap(iHeap);
                    break;
                }
                case CFG_BACKDROP:
                    m_backDropHash = PblHash(item.GetStringArg(0));
                    break;
                case CFG_RANDOMBACKDROP:
                    CopyBackDropName(m_randomBackDropFileName, item.GetStringArg(0));
                    break;
                case CFG_LOADINGTEXTCOLORPALLETE:
                {
                    PblConfig palette(scope.ReadNextScope(), true);
                    while (!palette.IsDone())
                    {
                        PblConfig::Data color = palette.ReadNextData();
                        if (color.GetId() == CFG_COLOR && m_loadingColorPalleteSize < 16)
                        {
                            m_loadingColorPallete[m_loadingColorPalleteSize++] = ConfigColor(color);
                        }
                    }
                    break;
                }
                case CFG_PROGRESSBARLEDWIDTH:
                    m_LEDWidth = item.GetFloatArg(0);
                    break;
                case CFG_TIPS:
                    AddTipsToDatabase(sTipsDatabase, item.GetStringArg(0));
                    break;
                case CFG_TIPSCOLOR:
                    m_textLevelTips.SetColor(ConfigColor(item));
                    break;
                case CFG_TEAMMODEL:
                {
                    const int iTeam = GetTeamNum(PblHash(item.GetStringArg(0)));
                    if (iTeam >= 0)
                    {
                        m_modelTeamIcon[iTeam].SetModel(item.GetStringArg(1));
                    }
                    break;
                }
                case CFG_LOADINGTEXTBLINKRATE:
                    m_loadingTextBlinkRate = item.GetFloatArg(0);
                    break;
                case CFG_SUNDIRECTION:
                {
                    m_sunDirection.x = item.GetFloatArg(0);
                    m_sunDirection.y = item.GetFloatArg(1);
                    m_sunDirection.z = item.GetFloatArg(2);
                    PblVector3 vNormal;
                    D3DXVec3Normalize(&vNormal, &m_sunDirection);
                    m_sunDirection = vNormal;
                    break;
                }
                case CFG_PROGRESSBARNUMLEDS:
                    m_numLEDs = item.GetUint32Arg(0);
                    break;
                }
            }
        }
        else if (data.GetId() == CFG_WORLD || data.GetId() == CFG_MAP)
        {
            const unsigned int uiMission = PblHash(data.GetStringArg(0));
            PblConfig scope(config.ReadNextScope(), true);
            if (m_missionHash != uiMission && m_entireMissionHash != uiMission)
            {
                continue;
            }
            while (!scope.IsDone())
            {
                PblConfig::Data item = scope.ReadNextData();
                switch (item.GetId())
                {
                case CFG_BACKDROP:
                    m_backDropHash = PblHash(item.GetStringArg(0));
                    break;
                case CFG_CAMPAIGNLAYOUT:
                    m_campaignLayout = item.GetUint8Arg(0) == 1;
                    break;
                case CFG_PROGRESSBARTOTALTIME:
                    m_totalProgressTime = item.GetFloatArg(0);
                    break;
                case CFG_TIPSTIME:
                    m_timePerTip = item.GetFloatArg(0);
                    break;
                case CFG_RANDOMBACKDROP:
                    CopyBackDropName(m_randomBackDropFileName, item.GetStringArg(0));
                    break;
                case CFG_TIPS:
                    sTipsDatabase.RemoveAll();
                    AddTipsToDatabase(sTipsDatabase, item.GetStringArg(0));
                    break;
                case CFG_CAMPAIGNNAME:
                    m_campaignNameHash = PblHash(item.GetStringArg(0));
                    break;
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x005782a0
LoadDisplay::ProgressIndicator::ProgressIndicator()
{
    m_pLED = 0;
    m_pIntensity = 0;
    m_pColor = 0;
    m_curOnLED = 0;
    m_progressBarDirection = 0;
    m_LEDTexHash = 0;
    m_width = 0.0f;
    m_numLEDs = 0;
    m_OnLEDFraction = 0.0f;
    m_LEDSpeed = 30.0f;
}

// SYNTHETIC: SWBF2 0x00578320
// LoadDisplay::ProgressIndicator::`scalar deleting destructor'
// FUNCTION: SWBF2 0x00578350
LoadDisplay::ProgressIndicator::~ProgressIndicator()
{
    const int iHeap = RedSetCurrentHeap(s_loadHeap);
    RemoveAllElements();
    delete[] m_pLED;
    delete[] m_pIntensity;
    delete[] m_pColor;
    RedSetCurrentHeap(iHeap);
}

// FUNCTION: SWBF2 0x00578400
void LoadDisplay::ProgressIndicator::Init(int iNumLEDs, float fSpacing,
                                          const unsigned int* puiTextureHashes, int iNumTextures,
                                          const RedColor* pColors, int iNumColors, float fSpeed,
                                          LEDModeT eMode)
{
    const int iHeap = RedSetCurrentHeap(s_loadHeap);
    delete[] m_pLED;
    delete[] m_pIntensity;
    delete[] m_pColor;

    m_curOnLED = 0;
    m_progressBarDirection = 1;
    float fX = 0.0f;
    m_LEDSpeed = fSpeed;
    m_numLEDs = iNumLEDs;
    m_width = 0.0f;
    m_mode = eMode;
    m_OnLEDFraction = 0.0f;

    m_pLED = new RedBitmapElement[iNumLEDs];
    m_pIntensity = new float[m_numLEDs];
    m_pColor = new RedColor[m_numLEDs];

    for (int i = 0; i < m_numLEDs; ++i)
    {
        m_pIntensity[i] = LED_OFF_INTENSITY;
        const unsigned int uiTexture =
            puiTextureHashes[(unsigned int)(PblRandom::GetGlobalFloat() * iNumTextures)];
        m_pColor[i] = pColors[(unsigned int)(PblRandom::GetGlobalFloat() * iNumColors)];

        m_pLED[i].Enable(true);
        m_pLED[i].SetColor(m_pColor[i]);
        m_pLED[i].SetTexture(uiTexture);
        m_pLED[i].SetTexCoords(0.0f, 0.0f, 1.0f, 1.0f, false);
        m_pLED[i].SetRect(m_LEDWidth, m_LEDHeight, RedBitmapElement::BITMAP_HALIGN_LEFT,
                          RedBitmapElement::BITMAP_VALIGN_CENTER);
        m_pLED[i].SetPosition(fX, 0.0f);

        const float fIntensity = m_pIntensity[m_curOnLED];
        const RedColor& onColor = m_pColor[m_curOnLED];
        RedColor color;
        color.b = (unsigned char)(int)(onColor.b * fIntensity);
        color.g = (unsigned char)(int)(onColor.g * fIntensity);
        color.r = (unsigned char)(int)(onColor.r * fIntensity);
        color.a = onColor.a;
        m_pLED[i].SetColor(color);

        AddElement(&m_pLED[i]);
        fX += m_LEDWidth + fSpacing;
    }
    m_width = fX - fSpacing;
    RedSetCurrentHeap(iHeap);
}

static void SetLEDColor(LoadDisplay::ProgressIndicator& bar, int i)
{
    const float fIntensity = bar.m_pIntensity[i];
    const RedColor& base = bar.m_pColor[i];
    RedColor color;
    color.b = (unsigned char)(int)(base.b * fIntensity);
    color.g = (unsigned char)(int)(base.g * fIntensity);
    color.r = (unsigned char)(int)(base.r * fIntensity);
    color.a = base.a;
    bar.m_pLED[i].SetColor(color);
}

// FUNCTION: SWBF2 0x00578c00
void LoadDisplay::ProgressIndicator::SetAllOn()
{
    for (int i = 0; i < m_numLEDs; ++i)
    {
        m_pIntensity[i] = 1.0f;
        SetLEDColor(*this, i);
    }
}

// FUNCTION: SWBF2 0x00578870
void LoadDisplay::ProgressIndicator::Update(float fDelta, GameSound* const pSound,
                                            unsigned int uiSoundHash)
{
    int iNumLEDs = m_numLEDs;
    if (iNumLEDs <= 0)
    {
        return;
    }

    m_OnLEDFraction += m_LEDSpeed * fDelta;
    if (m_OnLEDFraction >= 1.0f)
    {
        m_OnLEDFraction -= 1.0f;
        switch (m_mode)
        {
        case LED_MODE_RANDOM:
            m_curOnLED = (int)(PblRandom::GetGlobalFloat() * (float)iNumLEDs);
            break;
        case LED_MODE_WRAP:
            m_curOnLED = (m_curOnLED + 1) % iNumLEDs;
            break;
        case LED_MODE_TIMER:
            m_pIntensity[m_curOnLED] = 1.0f;
            SetLEDColor(*this, m_curOnLED);
            ++m_curOnLED;
            iNumLEDs = m_numLEDs;
            if (m_curOnLED >= iNumLEDs)
            {
                m_OnLEDFraction = 1.0f;
                m_curOnLED = iNumLEDs - 1;
            }
            break;
        default:
            m_curOnLED += m_progressBarDirection;
            if (m_curOnLED >= iNumLEDs)
            {
                m_progressBarDirection = -1;
                m_curOnLED = iNumLEDs > 1 ? iNumLEDs - 2 : 0;
            }
            else if (m_curOnLED < 0)
            {
                m_progressBarDirection = 1;
                m_curOnLED = iNumLEDs > 1 ? 1 : 0;
            }
            break;
        }
    }

    if (m_mode != LED_MODE_TIMER)
    {
        const float fOff = LED_OFF_INTENSITY;
        for (int i = 0; i < m_numLEDs; ++i)
        {
            m_pIntensity[i] = (m_pIntensity[i] - fOff) * 0.95f + fOff;
            SetLEDColor(*this, i);
        }
        m_pIntensity[m_curOnLED] = (1.0f - fOff) * m_OnLEDFraction + fOff;
    }
    else
    {
        if (m_pIntensity[m_curOnLED] >= 0.99f)
        {
            return;
        }
        m_pIntensity[m_curOnLED] = (1.0f - LED_OFF_INTENSITY) * m_OnLEDFraction + LED_OFF_INTENSITY;
    }
    SetLEDColor(*this, m_curOnLED);
}

// FUNCTION: SWBF2 0x00578d50
void LoadDisplay::IDDatabase::Initialize()
{
    m_numLoadedIDs = 0;
    m_usedLoadedIDsStartIndex = m_usedLoadedIDsEndIndex;
}

// LIBRARY: SWBF2 0x00575540
// time

// FUNCTION: SWBF2 0x00578cd0
LoadDisplay::IDDatabase::IDDatabase()
{
    m_numLoadedIDs = 0;
    m_usedLoadedIDsStartIndex = 0;
    m_usedLoadedIDsEndIndex = 0;
    rand.SetSeed((unsigned int)time(NULL));
}

// FUNCTION: SWBF2 0x00578d70
void LoadDisplay::IDDatabase::AddID(unsigned int uiParent, unsigned int uiID)
{
    const unsigned int uiNumUsed = m_usedLoadedIDsStartIndex;
    for (unsigned int i = 0; i < uiNumUsed; ++i)
    {
        if (m_usedIDs[i].parent == uiParent && m_usedIDs[i].id == uiID)
        {
            const IDType last = m_usedIDs[uiNumUsed - 1];
            m_usedIDs[uiNumUsed - 1] = m_usedIDs[i];
            m_usedIDs[i] = last;
            --m_usedLoadedIDsStartIndex;
            return;
        }
    }

    if (m_numLoadedIDs < 200)
    {
        m_loadedIDs[m_numLoadedIDs].parent = uiParent;
        m_loadedIDs[m_numLoadedIDs].id = uiID;
        ++m_numLoadedIDs;
    }
}

// FUNCTION: SWBF2 0x00578e10
void LoadDisplay::IDDatabase::RemoveAll()
{
    m_numLoadedIDs = 0;
    m_usedLoadedIDsEndIndex = 0;
    m_usedLoadedIDsStartIndex = 0;
}

// FUNCTION: SWBF2 0x00578e30
unsigned int LoadDisplay::IDDatabase::GetRandomID()
{
    if (m_numLoadedIDs == 0)
    {
        m_numLoadedIDs = m_usedLoadedIDsEndIndex - m_usedLoadedIDsStartIndex;
        if (m_numLoadedIDs == 0)
        {
            return 0;
        }
        memcpy(m_loadedIDs, &m_usedIDs[m_usedLoadedIDsStartIndex],
               m_numLoadedIDs * sizeof(IDType));
        m_usedLoadedIDsEndIndex = m_usedLoadedIDsStartIndex;
    }

    IDType& entry = m_loadedIDs[rand.GetInt() % m_numLoadedIDs];
    const unsigned int uiID = entry.id;
    m_usedIDs[m_usedLoadedIDsEndIndex] = entry;
    ++m_usedLoadedIDsEndIndex;
    --m_numLoadedIDs;
    entry = m_loadedIDs[m_numLoadedIDs];
    return uiID;
}

// FUNCTION: SWBF2 0x00578d40
LoadDisplay::IDDatabase::~IDDatabase()
{
}
