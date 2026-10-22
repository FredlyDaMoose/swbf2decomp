#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>

#include <d3d9.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Util/RedWarning.h"
#include "SoundFL/source/sndbus.h"
#include "SoundFL/source/win/sndengine.h"
#include "SoundFL/source/win/snddsbuffer.h"
#include "SoundFL/source/sndfile.h"
#include "SoundFL/source/sndglobals.h"
#include "SoundFL/source/sndsource3d.h"
#include "Steam/gfSteamAuth.h"
#include "Steam/galaxy_api.h"
#include "CommandLine.h"
#include "GameLoop.h"
#include "GameMemory.h"
#include "GameSoundEngine.h"
#include "GameState.h"
#include "LoadDisplay.h"
#include "LoadUtil.h"
#include "PauseMenu.h"
#include "RedEngineFL/input/pc/pcRedInputManager.h"
#include "NetSetup.h"
#include "PlayerProfile.h"

void __fastcall ParseAutoNetCommandLine(char* pCmdLine);
void InitializeMainCameraState();

void* DebugSystemAlloc(unsigned int uiSize);
void DebugSystemFree(void* pBlock);
long __stdcall LowLevelKeyboardProc(int nCode, unsigned int wParam, long lParam);

void CloseJournal();

extern bool sPauseMenuSoundEnabled;

char appName[260] = "Star Wars Battlefront II";

bool gAppActive = true;

void* s_memoryArena;
void* g_hMutex;

// GLOBAL: SWBF2 0x01eaedd4
bool gAltMeleeHitEffects;

bool pcForceFixedFunction;
bool pcWindowed;
HWND hGlobalWnd;
bool pcPostInstall;

// GLOBAL: SWBF2 0x01eaede5
bool gIsQuitting;
bool pcEnableLogging;
bool gbMutexValid;
HINSTANCE hGlobalInstance;
int PCDebugHeap;

int pcHeight;
int pcWidth;

D3DGAMMARAMP s_originalGammaRamp;

// FUNCTION: SWBF2 0x006170f0
bool ToggleAltMeleeHitEffectsCheat()
{
    gAltMeleeHitEffects = !gAltMeleeHitEffects;
    return gAltMeleeHitEffects;
}

// FUNCTION: SWBF2 0x00617100
bool ToggleSoundEnabled()
{
    sPauseMenuSoundEnabled = !sPauseMenuSoundEnabled;
    return !sPauseMenuSoundEnabled;
}

// FUNCTION: SWBF2 0x00617120
bool ToggleSoundPitch()
{

    if (Snd::Source3D::sPitchMultiplier == 1.0f)
    {
        Snd::Source3D::sPitchMultiplier = 0.3f;
        return true;
    }
    Snd::Source3D::sPitchMultiplier = 1.0f;
    return false;
}

// FUNCTION: SWBF2 0x00617150
static bool AcquireMutex()
{
    g_hMutex = CreateMutexA(0, FALSE, "PlayBF");
    if (g_hMutex != 0 && GetLastError() != ERROR_ALREADY_EXISTS)
    {
        gbMutexValid = true;
        return true;
    }

    CloseHandle(g_hMutex);
    return false;
}

// FUNCTION: SWBF2 0x00617190
static void ReleaseMutex()
{
    if (gbMutexValid)
    {
        CloseHandle(g_hMutex);
    }
    gbMutexValid = false;
}

// FUNCTION: SWBF2 0x006171b0
void Startup_InitApplicationMemory()
{
    s_memoryArena = malloc(0x4000000);

    GameMemory::InitApplicationMemory((char*)s_memoryArena,
                                      (char*)s_memoryArena + 0x4000000);
}

// FUNCTION: SWBF2 0x006171d0
void Startup_DebugHeap()
{
    void* p = malloc(0x300000);
    PCDebugHeap = RedCreateHeap(p, (char*)p + 0x300000, 0x10, "Debug");
    RedSetDebugHeap(PCDebugHeap);
}

// FUNCTION: SWBF2 0x00617200

extern "C" void Startup_RedInitHeap()
{
    Startup_InitApplicationMemory();
    Startup_DebugHeap();
}

// FUNCTION: SWBF2 0x00617280
static void SoundEnable(bool bEnable)
{

    if (!Snd::EngineBase::smOpened)
    {
        return;
    }

    if (bEnable)
    {
        PlayerProfile::tProfile* pProfile = PlayerProfile::GetProfile(PauseMenu::GetPausingJoystick());
        if (pProfile)
        {

            unsigned char ucGain = pProfile->mSoundOptions.mu8MasterVol;
            Snd::Bus::smRootBus.SetGain((float)ucGain / 10.0f, true);
        }
        else
        {
            Snd::Bus::smRootBus.SetGain(1.0f, true);
        }

        Snd::DSBuffer::ResetBuffers();
    }
    else
    {
        Snd::Bus::smRootBus.SetGain(0.0f, true);
    }
}

// FUNCTION: SWBF2 0x006172a0
long __stdcall WindowProc(HWND__* hWnd, unsigned int Msg, unsigned int wParam, long lParam)
{
    switch (Msg)
    {
    case WM_CLOSE:
        if (gLoading)
        {
            return 0;
        }
        gIsQuitting = true;

        if (GameSoundEngine::gDefaultSpeakerConfig)
        {
            Snd::Engine::SetSpeakerConfig(GameSoundEngine::gDefaultSpeakerConfig);
        }
        CloseJournal();
        GameLoop::Cleanup();

        GameState::PostStateCleanup(GameState::GetState() != 0x11E1FC01);
        GameState::OneTimeCleanup();
        {

            HMENU hMenu = GetMenu(hGlobalWnd);
            if (hMenu)
            {
                DestroyMenu(hMenu);
            }
        }
        DestroyWindow(hGlobalWnd);
        PostQuitMessage(0);
        hGlobalWnd = 0;
        _exit(0);

    case WM_ACTIVATE:
        return 0;

    case WM_ERASEBKGND:
        return 1;

    case WM_ACTIVATEAPP:
        gAppActive = wParam != 0;
        return 0;

    case WM_SETCURSOR:
        SetCursor(0);

        if (!g_bNoRender)
        {
            RedRenderer::SetCursorImage(
                RedTexture::s_textureHashTable.Find(0xA2BB0616), 0, 0);
        }
        return 1;

    case WM_KEYDOWN:
        return 0;

    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_MINIMIZE:
            if (RedInputManager::GetMouse(0))
            {
                if (RedInputManager::GetMouse(0)->Exists())
                {
                    RedInputManager::GetMouse(0)->Remove();
                }
            }
            if (!PauseMenu::GetPaused(0))
            {
                SoundEnable(false);
            }
            return DefWindowProcA(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, lParam);

        case SC_MAXIMIZE:
        case SC_RESTORE:
            if (!PauseMenu::GetPaused(0))
            {
                SoundEnable(true);
            }
            break;

        case SC_CLOSE:
            gIsQuitting = true;
            return DefWindowProcA(hWnd, WM_SYSCOMMAND, SC_CLOSE, lParam);

        case SC_TASKLIST:
        case SC_SCREENSAVE:
        case SC_MONITORPOWER:
            return 0;
        }
        return DefWindowProcA(hWnd, WM_SYSCOMMAND, wParam, lParam);
    }

    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

// FUNCTION: SWBF2 0x006174e0
void ActivateFrontline(bool bActivate)
{
    if (bActivate)
    {
        if (!PauseMenu::GetPaused(0))
        {
            SoundEnable(true);
        }
        SetCursor(0);
        if (!pcWindowed)
        {
            ShowWindow(hGlobalWnd, SW_RESTORE);
        }
        return;
    }

    if (gIsQuitting)
    {
        return;
    }

    if (GameState::GetState() != 0x11E1FC01 && PauseMenu::IsPauseAllowed())
    {
        LoadDisplay* pLoadDisplay = LoadDisplay::sInstance;

        if (pLoadDisplay &&
            pLoadDisplay->m_loadState ==
                LoadDisplay::LOAD_STATE_FINISHED)
        {
            PauseMenu::SetPaused(0, true);
            GameLoop::Pause();
        }
    }

    if (RedInputManager::GetMouse(0))
    {
        if (RedInputManager::GetMouse(0)->Exists())
        {
            RedInputManager::GetMouse(0)->Remove();
        }
    }

    SetCursor(LoadCursorA(hGlobalInstance, IDC_ARROW));
    if (!pcWindowed)
    {
        ShowWindow(hGlobalWnd, SW_MINIMIZE);
    }
}

// FUNCTION: SWBF2 0x006175d0
int __stdcall GameWinMain(HINSTANCE hInstance, HINSTANCE, char* lpCmdLine, int)
{

    char szKey[64];
    strncpy(szKey, "952F41D039DB4507A84B40488E76932F 0123456789ABCDECAT0123456IPA", 0x3D);

    bool bRestarting = false;
    if (!gf::_sap()->Init(0x17AC, bRestarting) && bRestarting)
    {
        return 0;
    }

    atexit(ReleaseMutex);

    bool bDedicated = strstr(lpCmdLine, "/autonet") != 0 &&
                      strstr(lpCmdLine, "dedicated") != 0;
    if (!bDedicated)
    {
        if (!AcquireMutex())
        {
            return 0;
        }
    }

    RedWarning::Init();
    GameLoop::SetNextMission(0);

    g_bNoRender = false;
    g_bNoWait = false;
    g_bNoSound = false;
    g_fFixedRate = 0.0f;
    ParseAutoNetCommandLine(lpCmdLine);

    ParseCommandLine(0, lpCmdLine);

    if (!g_bNoRender)
    {
        HWND hRunning = FindWindowA(appName, 0);
        if (hRunning)
        {
            ShowWindow(hRunning, SW_RESTORE);
            return 0;
        }
    }

    RedWarning::SetSeverityToDestination(
        RED_LOG_DESTINATION_CONSOLE | RED_LOG_DESTINATION_TRACE, RedWarning::RED_SEVERITY_NONE);
    if (pcEnableLogging)
    {
        RedWarning::SetLogFileOverwrite(false);
        if (RedWarning::GetSeverityToDestination(RED_LOG_DESTINATION_INDEX_FILE) >
            RedWarning::RED_SEVERITY_WARNING)
        {
            RedWarning::SetSeverityToDestination(RED_LOG_DESTINATION_FILE,
                                                 RedWarning::RED_SEVERITY_WARNING);
        }
    }

    GameState::SetState(GameLoop::mMissionScript[0] ? 0x8FF60339 : 0x11E1FC01);

    if (g_bNoRender || bDedicated)
    {
        strcat(appName, " Dedicated");
    }

    WNDCLASSA wc;
    wc.style = CS_BYTEALIGNCLIENT | CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconA(hInstance, MAKEINTRESOURCEA(101));
    wc.hCursor = 0;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = appName;
    if (!RegisterClassA(&wc))
    {
        return 0;
    }

    DWORD dwStyle = pcWindowed ? (WS_CAPTION | WS_THICKFRAME) : (WS_POPUP | WS_VISIBLE);
    DWORD dwExStyle = pcWindowed ? 0 : WS_EX_TOPMOST;

    RECT rc;
    SetRect(&rc, 0, 0, pcWidth ? pcWidth : 800, pcHeight ? pcHeight : 600);
    AdjustWindowRect(&rc, dwStyle, FALSE);

    RECT rcDesktop;
    GetClientRect(GetDesktopWindow(), &rcDesktop);

    int nWidth = rc.right - rc.left;
    int nHeight = rc.bottom - rc.top;
    int X = (rcDesktop.left + rcDesktop.right - nWidth) / 2;
    int Y = (rcDesktop.top + rcDesktop.bottom - nHeight) / 2;

    char szTitle[264];
    strcpy(szTitle, "Star Wars Battlefront II");
    if (strcmp(autoNetSession, "") != 0)
    {
        strcat(szTitle, " : ");
        strcat(szTitle, autoNetSession);
    }

    hGlobalInstance = hInstance;
    hGlobalWnd = CreateWindowExA(dwExStyle, appName, "Star Wars Battlefront II", dwStyle,
                                 X, Y, nWidth, nHeight, 0, 0, hInstance, 0);

    if (pcPostInstall)
    {
        Snd::File::SystemOpen(0x7FFFFFFF);
        Snd::Globals::smAlloc = malloc;
        Snd::Globals::smFree = free;
        IDirect3D9* pD3D = Direct3DCreate9(D3D_SDK_VERSION);
        RedVideoConfig::VidMode_Init(pD3D);
        pD3D->Release();
        Snd::File::SystemClose();
        return 0;
    }

    GetDeviceGammaRamp(GetDC(hGlobalWnd), &s_originalGammaRamp);
    SetICMMode(GetDC(hGlobalWnd), ICM_ON);

    MSG msg;
    bool bActive = true;
    msg.message = 0;
    PeekMessageA(&msg, 0, 0, 0, PM_NOREMOVE);
    if (msg.message != WM_QUIT)
    {
        for (;;)
        {
            if (bActive != gAppActive)
            {
                bActive = gAppActive;

                ActivateFrontline(bActive);
            }

            if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            }
            else
            {
                if (!bActive)
                {
                    Sleep(10);
                }
                if (!GameState::Update())
                {
                    SendMessageA(hGlobalWnd, WM_CLOSE, 0, 0);
                }
            }

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
    }

    InitializeMainCameraState();
    return 0;
}

// FUNCTION: SWBF2 0x00617ac0
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, char* lpCmdLine, int)
{
    bool bDebuggerPresent = IsDebuggerPresent() != 0;
    __try
    {
        return GameWinMain(hInstance, 0, lpCmdLine, 0);
    }
    __except (GetExceptionCode() == EXCEPTION_FLT_DIVIDE_BY_ZERO ||
              (!bDebuggerPresent && GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION))
    {
        _exit(0);
    }
}

// FUNCTION: SWBF2 0x00617b50
void PumpWaitingMessages()
{
    MSG msg;
    while (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

// FUNCTION: SWBF2 0x00617bb0
void* GetGalaxyNetworking()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->Networking();
}

// FUNCTION: SWBF2 0x00617bd0
void* GetGalaxyHostNetworking()
{
    if (!galaxy::api::GalaxyFactory::GetInstance())
    {
        return 0;
    }
    galaxy::api::IGalaxy* pGalaxy = galaxy::api::GalaxyFactory::GetInstance();
    return pGalaxy->ServerNetworking();
}

// FUNCTION: SWBF2 0x006ccc90
void InitializeMainCameraState()
{
}

// GLOBAL: SWBF2 0x01faa8f8

void* sHeapScratchBuffer;

// GLOBAL: SWBF2 0x01faa8fc

bool sHeapScratchAllocated;

// FUNCTION: SWBF2 0x00766ed0
void __cdecl ReleaseHeapScratchBuffer()
{
    if (sHeapScratchAllocated)
    {
        ::operator delete[](sHeapScratchBuffer);
    }
}
