#include "GameMovie.h"

#include <string.h>

#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/movie/RedMovie.h"
#include "SoundFL/source/sndbus.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "GameMemory.h"
#include "LoadUtil.h"
#include "GameState.h"
#include "CameraManager.h"
#include "Character.h"
#include "HUDManager.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "SoundFL/source/sndqueue.h"
#include "GameSoundEngine.h"
#include "PebbleFL/Common/PblTime.h"

#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Interface/Red3DModelElement.h"
#include "RedEngineFL/Interface/RedBitmapElement.h"
#include "RedEngineFL/Interface/RedBorderElement.h"
#include "RedEngineFL/Interface/RedHotSpot.h"
#include "RedEngineFL/Interface/RedIFScreen.h"
#include "FLJoystick.h"
#include "RedEngineFL/input/RedKeyboard.h"

static const float g_MovieStopDelayTime = 2.0f;

// GLOBAL: SWBF2 0x01e56278
unsigned __int64 g_InGameMovieStartTime;

extern bool g_InGameMoviePlayerUpdateEnable;

// GLOBAL: SWBF2 0x01e56274
RedMovie* GameMovie::sRedMovie;

// GLOBAL: SWBF2 0x01e56136
bool GameMovie::sMemoryReserved;

// GLOBAL: SWBF2 0x007e67c8
int GameMovie::sDefaultHeap = -1;

// GLOBAL: SWBF2 0x007e67cc
int GameMovie::sDataHeap = -1;

// GLOBAL: SWBF2 0x007e68e0
int GameMovie::sTextureHeap = -1;

// GLOBAL: SWBF2 0x01e56284
char* GameMovie::sFilename;

// GLOBAL: SWBF2 0x01faa680
unsigned int gMovieOpenFailures;

// FUNCTION: SWBF2 0x005343f0
void GameMovie::Create()
{
    if (GameMovie::sUseMovies)
    {

        sDefaultHeap = GameMemory::RunTimeHeap;
        int iOldHeap = RedSetCurrentHeap(GameMemory::RunTimeHeap);

        sRedMovie = new RedMovie;

        RedSetCurrentHeap(iOldHeap);

        if (GameState::GetState() != 0x11E1FC01)
        {

            Snd::Bus* pBus = Snd::Bus::FindByHashID(0xF4EB2B40);
            if (!pBus)
            {
                pBus = &Snd::Bus::smRootBus;
            }

            sRedMovie->SetAudioBus(pBus);
        }

        sMemoryReserved = (sRedMovie != 0);
    }
}

// FUNCTION: SWBF2 0x005344c0
void GameMovie::Destroy(bool bDestroyAll)
{
    if (bDestroyAll)
    {
        RedMovie::DestroyAll();
    }

    if (sRedMovie)
    {
        if (sRedMovie->IsOpen())
        {
            Close();
        }

        sFilename = 0;
        int iOldHeap = RedSetCurrentHeap(sDefaultHeap);
        delete sRedMovie;
        RedSetCurrentHeap(iOldHeap);

        sMemoryReserved = false;
        sDefaultHeap = -1;
        sRedMovie = 0;
    }
}

// FUNCTION: SWBF2 0x00534540
void GameMovie::DestroyKeepProperty()
{
    Destroy(false);
}

// FUNCTION: SWBF2 0x00534550
void GameMovie::Open()
{
    if (GameMovie::sUseMovies)
    {
        Open(sFilename, sFilename + strlen(sFilename), GameMovie::sDataHeap, GameMovie::sTextureHeap);
    }
}

// FUNCTION: SWBF2 0x00534590
void GameMovie::Open(const char* const pszFile, const char* const pszName, int iHeap1, int iHeap2)
{
    if (!GameMovie::sUseMovies)
    {
        return;
    }

    if (sFilename != pszFile)
    {
        const char* pszPath = pszFile;
        bool bDiscPrefix = _strnicmp(pszPath, "dc:", 3) == 0;
        if (bDiscPrefix)
        {
            pszPath += 3;
        }

        char szFullName[260];
        LoadUtil::MakeFullName(pszPath, LoadUtil::FILE_TYPE_NONE, szFullName, 128, bDiscPrefix);

        if (sFilename)
        {
            RedFreeToHeap(sDefaultHeap, sFilename, 0);
            sFilename = 0;
        }

        sFilename = (char*)RedAllocFromHeap(sDefaultHeap, strlen(szFullName) + strlen(pszName) + 2, 0);
        strlcpy(sFilename, szFullName, strlen(szFullName) + 1);
        strlcpy(sFilename + strlen(szFullName) + 1, pszName, strlen(pszName) + 1);
    }

    GameMovie::sDataHeap = iHeap1 == -1 ? sDefaultHeap : iHeap1;
    GameMovie::sTextureHeap = iHeap2 == -1 ? sDefaultHeap : iHeap2;

    if (sRedMovie && !GameMemory::TempLoadHeap)
    {
        if (!sRedMovie->Open(sFilename, GameMovie::sDataHeap, GameMovie::sTextureHeap, PblHash(pszName), 0xFFFFFFFFu,
                             0xFFFFFFFFu, 3))
        {
            ++gMovieOpenFailures;
        }
    }
}

// FUNCTION: SWBF2 0x00534750
void GameMovie::Close()
{
    if (GameMovie::sUseMovies)
    {
        if (sRedMovie && sRedMovie->IsOpen())
        {
            sRedMovie->Close();
        }

        if (sFilename)
        {
            RedFreeToHeap(sDefaultHeap, sFilename, 0);
            sFilename = 0;
        }
    }
}

// FUNCTION: SWBF2 0x00534a30
void GameMovie::Render()
{
    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen() && sRedMovie->IsPlaying())
    {
        sRedMovie->Render();
    }
}

// FUNCTION: SWBF2 0x00534a70
void GameMovie::EndFrame()
{
    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen() && sRedMovie->IsPlaying())
    {
        sRedMovie->EndFrame();
    }
}

// FUNCTION: SWBF2 0x00534960
void GameMovie::Stop()
{
    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen() && sRedMovie->IsPlaying())
    {
        sRedMovie->Stop(true);
        FLRenderer::EnableCursor(true);
    }
}

// FUNCTION: SWBF2 0x00534730
bool GameMovie::IsOpen()
{
    return GameMovie::sUseMovies && GameMovie::sRedMovie && GameMovie::sRedMovie->IsOpen();
}

extern int g_InGameMoviePlayerFrame;
// GLOBAL: SWBF2 0x01e56280
bool g_InGameMovieSpawnDisplayActive[1];

extern char ingame_movie_idname[256];
extern char g_InGameMovieFilename[256];
extern Snd::Queue<unsigned int, 12> g_InGameMovieQueue;

extern int g_InGameMoviePlayerState;

// FUNCTION: SWBF2 0x00534ab0
void EnableSceneAndInterface(bool bEnable)
{
    CameraManager* pCameras = CameraManager::sInstance;
    for (unsigned int i = 0; i < pCameras->m_iNumCam; ++i)
    {
        unsigned int iView = pCameras->m_hView[i];
        FLRenderer::SetSceneEnable(iView, bEnable);
        FLRenderer::SetInterfaceEnable(iView, bEnable);
    }
    HUD::gScreenGroup->Enable(bEnable);

    SpawnDisplay* pSpawnDisplay = SpawnDisplay::Instance(0);
    if (!pSpawnDisplay)
    {
        return;
    }
    const unsigned int numCameras = CameraManager::sInstance->m_iNumCam;
    for (unsigned int i = 0; i < numCameras; ++i)
    {
        Character* pCharacter = 0;
        if (Character::sCharacters && (int)i < Character::sCharacterLimit)
        {
            Character* pCandidate = &Character::sCharacters[i];
            if (pCandidate->mPlayerId == (int)i)
            {
                pCharacter = pCandidate;
            }
        }

        if (bEnable)
        {
            if (g_InGameMovieSpawnDisplayActive[i])
            {
                SpawnDisplay::Instance(0)->Show(pCharacter);
                g_InGameMovieSpawnDisplayActive[i] = false;
            }
        }
        else
        {
            pSpawnDisplay = SpawnDisplay::Instance(0);
            g_InGameMovieSpawnDisplayActive[i] = pSpawnDisplay->mMode == SpawnDisplay::eCHANGECLASS;
            pSpawnDisplay->Hide(pCharacter);
        }
    }
}

// FUNCTION: SWBF2 0x00534d40
void UpdatInGameMovie()
{
    if (g_InGameMoviePlayerState == 0)
    {
        if (GameMovie::IsOpen())
        {
            StopInGameMoviePlay();
        }
        return;
    }

    switch (g_InGameMoviePlayerState)
    {
    case 1:
        if (g_InGameMoviePlayerFrame == 0)
        {
            GameSoundEngine::StopAll();
            GameMovie::Create();
            GameMovie::Open(g_InGameMovieFilename, ingame_movie_idname,
                            GameMemory::RunTimeHeap, GameMemory::RunTimeHeap);
            g_InGameMoviePlayerState = 2;
            EnableSceneAndInterface(false);
        }
        ++g_InGameMoviePlayerFrame;
        break;

    case 2:
        if (g_InGameMoviePlayerFrame == 4)
        {
            GameMovie::Play(ingame_movie_idname, "", 0, 0,
                            RedInterfaceScreen::GetScreenWidth(),
                            RedInterfaceScreen::GetScreenHeight(), false);
            while (!g_InGameMovieQueue.IsEmpty())
            {
                unsigned int name;
                g_InGameMovieQueue.Get(name);
                const RedMovie::Properties* pProperty = RedMovie::Properties::FindByHashID(name);
                if (pProperty)
                {
                    GameMovie::sRedMovie->Play(pProperty, true);
                }
            }
            g_InGameMoviePlayerState = 3;
        }
        ++g_InGameMoviePlayerFrame;
        break;

    case 3:
        if (GameMovie::IsOpen())
        {
            GameMovie::Update(1.0f);
        }
        ++g_InGameMoviePlayerFrame;
        if (!GameMovie::IsPlaying())
        {
            StopInGameMoviePlay();
        }
        break;
    }
}

// FUNCTION: SWBF2 0x00738a20
void* Snd::QueueBase::Get(unsigned int uStride, char* pData, unsigned int uCapacity)
{
    unsigned short* pQueue = (unsigned short*)this;
    if (pQueue[0] == pQueue[1] && !(pQueue[2] & 1))
    {
        return 0;
    }
    const unsigned int uIndex = pQueue[1];
    pQueue[1] = (unsigned short)(uIndex + 1);
    if (pQueue[1] == uCapacity)
    {
        pQueue[1] = 0;
    }
    pQueue[2] &= ~1u;
    return pData + uStride * uIndex;
}

// FUNCTION: SWBF2 0x005347a0
void GameMovie::Play(const char* const pcProperties, const char* const pcPropertiesAtEnd,
                     unsigned int uiY, unsigned int uiX, unsigned int uiWidth,
                     unsigned int uiHeight, bool bLoop)
{
    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen())
    {
        const RedMovie::Properties* pProperties =
            RedMovie::Properties::FindByHashID(PblHash(pcProperties));
        const RedMovie::Properties* pPropertiesAtEnd = 0;
        if (pcPropertiesAtEnd && strlen(pcPropertiesAtEnd))
        {
            pPropertiesAtEnd = RedMovie::Properties::FindByHashID(PblHash(pcPropertiesAtEnd));
        }

        if (pProperties)
        {
            FLRenderer::EnableCursor(false);
            sRedMovie->SetDimensions(uiX, uiY, uiWidth, uiHeight, true);
            sRedMovie->Play(pProperties, false);
            sRedMovie->SetLooping(bLoop);
            if (pPropertiesAtEnd)
            {
                sRedMovie->Play(pPropertiesAtEnd, true);
            }
        }
    }
}

// FUNCTION: SWBF2 0x00534880
bool GameMovie::IsPlaying()
{
    if (g_InGameMoviePlayerState == 1 || g_InGameMoviePlayerState == 2)
    {
        return true;
    }

    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen())
    {
        return sRedMovie->IsPlaying();
    }

    return false;
}

// FUNCTION: SWBF2 0x005348c0
bool GameMovie::PropertiesPlaying(const char* const pcProperties, bool bCurrent)
{
    if (!GameMovie::sUseMovies || !sRedMovie || !sRedMovie->IsOpen())
    {
        return false;
    }

    if (bCurrent)
    {
        const RedMovie::Properties* pCurrent = sRedMovie->GetProperties();
        if (!pCurrent)
        {
            return false;
        }
        return pCurrent->GetHashID() == PblHash(pcProperties);
    }

    const RedMovie::Properties* pProperties =
        RedMovie::Properties::FindByHashID(PblHash(pcProperties));
    if (pProperties)
    {
        return sRedMovie->GetPropertiesScheduled(pProperties);
    }
    return false;
}

// FUNCTION: SWBF2 0x005349b0
void GameMovie::SetAudioBus(const char* const pcBusName)
{
    if (GameMovie::sUseMovies && sRedMovie)
    {
        sRedMovie->SetAudioBus(Snd::Bus::FindByHashID(PblHash(pcBusName)));
    }
}

// FUNCTION: SWBF2 0x005349f0
void GameMovie::Update(float fDeltaTime)
{
    if (GameMovie::sUseMovies && sRedMovie && sRedMovie->IsOpen())
    {
        sRedMovie->Update(fDeltaTime);
    }
}

// FUNCTION: SWBF2 0x00534ca0
void StopInGameMoviePlay()
{
    if (g_InGameMoviePlayerState)
    {
        LARGE_INTEGER liNow;
        QueryPerformanceCounter(&liNow);
        const float fElapsed = (float)(liNow.QuadPart - g_InGameMovieStartTime);
        if (fElapsed / (float)PblTime::ticksPerSecond > g_MovieStopDelayTime)
        {
            g_InGameMoviePlayerState = 0;
            g_InGameMoviePlayerFrame = 0;
            g_InGameMoviePlayerUpdateEnable = false;
            EnableSceneAndInterface(true);
            GameMovie::Stop();
            GameMovie::Close();
            GameMovie::DestroyKeepProperty();
        }
    }
}

// GLOBAL: SWBF2 0x01e5616c
int g_InGameMoviePlayerState;

// GLOBAL: SWBF2 0x01e56270
int g_InGameMoviePlayerFrame;

// GLOBAL: SWBF2 0x01ec0c44
// SYNTHETIC: SWBF2 0x00403da0
// `dynamic initializer for 'g_InGameMovieQueue''
// SYNTHETIC: SWBF2 0x00768a80
// `dynamic atexit destructor for 'g_InGameMovieQueue''
Snd::Queue<unsigned int, 12> g_InGameMovieQueue;
