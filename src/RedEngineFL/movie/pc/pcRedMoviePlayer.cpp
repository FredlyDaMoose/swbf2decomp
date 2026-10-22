#include <windows.h>
#include <string.h>

#include "pcRedMoviePlayer.h"
#include "RedEngineFL/Graphics/PC/pcRedTexture.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblMath.h"

void __cdecl PblTraceP(const char* Format, ...);

extern "C"
{

    typedef long(__stdcall* BINKIOOPEN)(BINKIO* io, const char* name, unsigned long flags);
    typedef void*(__stdcall* BINKMEMALLOC)(unsigned int bytes);
    typedef void(__stdcall* BINKMEMFREE)(void* ptr);

    int __stdcall BinkSetIOSize(unsigned int iosize);
    BINKIOOPEN __stdcall BinkSetIO(BINKIOOPEN io);
    void __stdcall BinkSetMemory(BINKMEMALLOC a, BINKMEMFREE f);

    typedef void*(__stdcall* BINKSNDSYSOPEN)(unsigned long param);
    void* __stdcall BinkOpenDirectSound(unsigned long param);
    int __stdcall BinkSetSoundSystem(BINKSNDSYSOPEN open, unsigned long param);
    BINK* __stdcall BinkOpen(const char* name, unsigned int flags);
    void __stdcall BinkClose(BINK* bink);
    char* __stdcall BinkGetError();
    int __stdcall BinkWait(BINK* bink);
    int __stdcall BinkDoFrame(BINK* bink);
    void __stdcall BinkNextFrame(BINK* bink);
    int __stdcall BinkCopyToBuffer(BINK* bink, void* dest, int destpitch,
                                   unsigned int destheight, unsigned int destx,
                                   unsigned int desty, unsigned int flags);
    int __stdcall BinkDX9SurfaceType(IDirect3DSurface9* surface);
    void __stdcall BinkSetVolume(BINK* bink, unsigned int trackid, int volume);
}

void* __stdcall local_alloc(unsigned int bytes);
void __stdcall local_free(void* ptr);

// FUNCTION: SWBF2 0x006f4c00
void* __stdcall local_alloc(unsigned int bytes)
{
    return RedAllocFromHeap(__RedCurrHeap, bytes, 0);
}

// FUNCTION: SWBF2 0x006f4c20
void __stdcall local_free(void* ptr)
{
    RedFreeToHeap(__RedCurrHeap, ptr, 0);
}

// GLOBAL: SWBF2 0x009c8420
RedMoviePlayer* RedMoviePlayer::sMoviePlayer;

// GLOBAL: SWBF2 0x009c8424
unsigned int RedMoviePlayer::sReadSize;

// FUNCTION: SWBF2 0x006f41d0
RedMoviePlayer::RedMoviePlayer()
    : RedMoviePlayerBase(0x800)
{
    mBink = 0;
    mFlags = 0;

    sMoviePlayer = this;

    BinkSetIOSize(0);
    BinkSetIO(BinkFileOpen);
    BinkSetMemory(local_alloc, local_free);
}

// FUNCTION: SWBF2 0x006f4270
RedMoviePlayer::~RedMoviePlayer()
{
    sMoviePlayer = 0;
}

// SYNTHETIC: SWBF2 0x006f42e0
// RedMoviePlayer::`scalar deleting destructor'

// FUNCTION: SWBF2 0x006f4310
RedTexture* RedMoviePlayer::AllocTexture(const unsigned int uiNameHash,
                                         const unsigned int uiWidth,
                                         const unsigned int uiHeight,
                                         const unsigned int uiFormat,
                                         const unsigned int uiIndex,
                                         const unsigned char ucFlags)
{
    int iOldHeap = RedSetCurrentHeap(GetInfo()->GetRedHeap());
    RedTexture* pTexture = new RedTexture(uiWidth, uiHeight, D3DFMT_X8R8G8B8, 1, 0,
                                          D3DPOOL_MANAGED, uiNameHash);
    RedSetCurrentHeap(iOldHeap);
    return pTexture;
}

// FUNCTION: SWBF2 0x006f43a0
BINKFILE* RedMoviePlayer::BinkFile(BINKIO* bio)
{
    return (BINKFILE*)bio->iodata;
}

// FUNCTION: SWBF2 0x006f43b0
void RedMoviePlayer::BinkFileClose(BINKIO* bio)
{
    BinkIOSuspend(bio);
    BinkIOResume(bio);
}

// FUNCTION: SWBF2 0x006f43d0
unsigned long RedMoviePlayer::BinkFileGetBufferSize(BINKIO* bio, unsigned long uiSize)
{
    return (uiSize / sReadSize) * sReadSize;
}

// FUNCTION: SWBF2 0x006f43f0
unsigned long RedMoviePlayer::BinkFileIdle(BINKIO* bio)
{
    if (BinkIOSuspendTry(bio))
    {
        bio->DoingARead = 1;
        bio->DoingARead = 0;
        return 0;
    }
    BinkIOIdleRun(bio);
    return (unsigned long)-1;
}

// FUNCTION: SWBF2 0x006f4430
long RedMoviePlayer::BinkFileOpen(BINKIO* bio, const char* pName, unsigned long uiFlags)
{
    memset(bio, 0, sizeof(BINKIO));
    BINKFILE* pFile = BinkFile(bio);
    pFile->fileHandle = (unsigned long)sMoviePlayer;
    pFile->dontClose = 1;
    pFile->startFile = 0;
    bio->ReadHeader = BinkFileReadHeader;
    bio->ReadFrame = BinkFileReadFrame;
    bio->GetBufferSize = BinkFileGetBufferSize;
    bio->SetInfo = BinkFileSetInfo;
    bio->Idle = BinkFileIdle;
    bio->Close = BinkFileClose;
    return 1;
}

// FUNCTION: SWBF2 0x006f44a0
unsigned long RedMoviePlayer::BinkFileReadFrame(BINKIO* bio, unsigned long uiFrame,
                                                long iOffset, void* pDest,
                                                unsigned long uiSize)
{
    RedMoviePlayer* pPlayer = (RedMoviePlayer*)BinkFile(bio)->fileHandle;
    BinkIOSuspend(bio);
    while (pPlayer->BufferGetUsed() < uiSize)
    {
        if (pPlayer->IsDiscontinuity())
        {
            pPlayer->mFlags |= 4;
        }
        else if (pPlayer->IsEnd())
        {
            pPlayer->mFlags |= 2;
        }
        if (pPlayer->mFlags & 6)
        {
            return 0;
        }
        pPlayer->RedMoviePlayerBase::Update();
        Sleep(0);
    }
    unsigned long uiRead = pPlayer->BufferReadData(pDest, uiSize, true);
    BinkIOResume(bio);
    return uiRead;
}

// FUNCTION: SWBF2 0x006f4550
unsigned long RedMoviePlayer::BinkFileReadHeader(BINKIO* bio, long iOffset, void* pDest,
                                                 unsigned long uiSize)
{
    unsigned long uiRead = 0;
    RedMoviePlayer* pPlayer = (RedMoviePlayer*)BinkFile(bio)->fileHandle;
    BinkIOSuspend(bio);
    if (pPlayer->BufferGetUsed() >= uiSize)
    {
        uiRead = pPlayer->BufferReadData(pDest, uiSize, true);
    }
    BinkIOResume(bio);
    return uiRead;
}

// FUNCTION: SWBF2 0x006f45a0
void RedMoviePlayer::BinkFileSetInfo(BINKIO* bio, void* pBuffer, unsigned long uiSize,
                                     unsigned long uiFileSize, unsigned long uiSimulate)
{
    BINKFILE* pFile = BinkFile(bio);
    BinkIOSuspend(bio);
    pFile->buffer = (unsigned char*)pBuffer;
    pFile->bufPos = (unsigned char*)pBuffer;
    pFile->bufBack = (unsigned char*)pBuffer;
    pFile->bufEnd = (unsigned char*)pBuffer;
    pFile->fileSize = uiFileSize;
    pFile->bufEmpty = uiSize;
    pFile->simulate = uiSimulate;
    bio->BufSize = uiSize;
    bio->CurBufUsed = 0;
    BinkIOResume(bio);
}

// FUNCTION: SWBF2 0x006f45f0
void RedMoviePlayer::BinkIOIdleRun(BINKIO* bio)
{
    if (bio->idle_on_callback)
    {
        bio->idle_on_callback(bio);
    }
}

// FUNCTION: SWBF2 0x006f4610
void RedMoviePlayer::BinkIOResume(BINKIO* bio)
{
    if (bio->resume_callback)
    {
        bio->resume_callback(bio);
    }
}

// FUNCTION: SWBF2 0x006f4630
void RedMoviePlayer::BinkIOSuspend(BINKIO* bio)
{
    if (bio->suspend_callback)
    {
        bio->suspend_callback(bio);
    }
}

// FUNCTION: SWBF2 0x006f4650
bool RedMoviePlayer::BinkIOSuspendTry(BINKIO* bio)
{
    return bio->try_suspend_callback && bio->try_suspend_callback(bio) != 0;
}

// FUNCTION: SWBF2 0x006f4670
void RedMoviePlayer::Close()
{
    DestroyBinkDecoder(true);
    CloseRenderer(GetInfo()->GetRedHeap());
    RedFreeToHeap(GetInfo()->GetRedHeap(), BufferGetBuffer(), 0);
    RedMoviePlayerBase::Close();
}

// FUNCTION: SWBF2 0x006f46c0
bool RedMoviePlayer::CreateBinkDecoder()
{
    return CreateTextures(mBink->Width, mBink->Height, 0x20, 3);
}

// FUNCTION: SWBF2 0x006f46e0
void RedMoviePlayer::DestroyBinkDecoder(bool bForce)
{
    if (mBink)
    {
        BinkClose(mBink);
        mBink = 0;
        DestroyTextures(bForce, 3);
    }
}

void PumpWaitingMessages();

// FUNCTION: SWBF2 0x006f4720
void RedMoviePlayer::EndFrame()
{
    RedMoviePlayerRenderer::EndFrame();
    PumpWaitingMessages();
}

// FUNCTION: SWBF2 0x006f4730
void RedMoviePlayer::Flush()
{
    RedMoviePlayerBase::Flush();
    DestroyBinkDecoder(false);
}

// FUNCTION: SWBF2 0x006f4750
void RedMoviePlayer::FreeTexture(RedTexture* pTexture, const unsigned int uiIndex,
                                 const unsigned char ucFlags)
{
    int iOldHeap = RedSetCurrentHeap(GetInfo()->GetRedHeap());
    delete pTexture;
    RedSetCurrentHeap(iOldHeap);
}

// FUNCTION: SWBF2 0x006f47a0
float RedMoviePlayer::GetPlaybackTime() const
{
    if (!mBink)
    {
        return 0.0f;
    }
    return (float)mBink->FrameRateDiv * (float)mBink->FrameNum / (float)mBink->FrameRate;
}

// FUNCTION: SWBF2 0x006f4820
bool RedMoviePlayer::Open(const char* const pFileName, unsigned int uiNumTextures, int iHeap,
                          int iUnused, unsigned int uiArg4, unsigned int uiBufferSize,
                          unsigned int uiReadSize, unsigned int uiNumBitmaps)
{
    char* pBlock = (char*)RedAllocFromHeap(iHeap, uiBufferSize + 4 * (uiNumBitmaps + 4 * uiNumTextures), 0);
    if (pBlock)
    {
        if (OpenRenderer(iHeap, (RedBitmapElement**)(pBlock + 16 * uiNumTextures + uiBufferSize),
                         uiNumBitmaps))
        {
            if (RedMoviePlayerBase::Open(pFileName, (RedTexture**)(pBlock + uiBufferSize),
                                         uiNumTextures, iHeap, uiArg4, pBlock, uiBufferSize,
                                         uiReadSize, 0xffffffff))
            {
                sReadSize = uiReadSize;
                BinkSetSoundSystem(BinkOpenDirectSound,
                                   (unsigned long)Snd::Engine::smDirectSound);
                return true;
            }
            CloseRenderer(iHeap);
        }
        RedFreeToHeap(iHeap, pBlock, 0);
    }
    return false;
}

// FUNCTION: SWBF2 0x006f48e0
void RedMoviePlayer::Render(const RedTexture* const pTexture, int iX, int iY,
                            unsigned int uiWidth, unsigned int uiHeight, unsigned char ucFlags)
{
    RedMoviePlayerRenderer::Render(pTexture, iX, iY, uiWidth, uiHeight, ucFlags);
}

// FUNCTION: SWBF2 0x006f48f0
void RedMoviePlayer::SeekSegment(const RedMoviePlayerBase::Segment* const pSegment, bool bFlag)
{
    RedMoviePlayerBase::SeekSegment(pSegment, bFlag);
}

// FUNCTION: SWBF2 0x006f4900
void RedMoviePlayer::SetGain(float fGain)
{
    if (mBink)
    {
        for (unsigned int i = 0; i < (unsigned int)mBink->NumTracks; ++i)
        {
            BinkSetVolume(mBink, i, (int)(fGain * 32767.0f));
        }
    }
}

// GLOBAL: SWBF2 0x009c8430
static bool sOpeningBinkDecoder;

// FUNCTION: SWBF2 0x006f4950
void RedMoviePlayer::Update()
{
    switch (GetDesiredPlayState())
    {
    case playState_stopped:
        Disable();
        SetCurrentPlayState(playState_stopped);
        DestroyBinkDecoder(false);
        Flush();
        break;
    case playState_paused:
        SetCurrentPlayState(playState_pausing);
        break;
    case playState_playing:
        SetCurrentPlayState(playState_playing);
        break;
    }

    PlayState eState = GetPlayState();
    if (eState < playState_pausing)
    {
        return;
    }
    if (eState > playState_paused)
    {
        if (eState != playState_playing)
        {
            return;
        }

        if (GetNumReallocTextures() && mBink)
        {
            ReallocTextures(mBink->Width, mBink->Height, 0x20, 3);
        }

        if (mFlags & 6)
        {
            if (!GetNumUsedTextures())
            {
                bool bEnd = (mFlags & 2) != 0;
                DestroyBinkDecoder(false);
                if (!GetLooping())
                {
                    Stop();
                }
                Notify(bEnd ? notifyType_End : notifyType_Discontinuity);
                mFlags &= ~7;
                Update();
                mFlags |= 1;
            }
        }
        else if (mBink && (mFlags & 1) && !BinkWait(mBink))
        {
            RedTexture* pTexture = GetTexture();
            if (pTexture && !BinkDoFrame(mBink))
            {
                IDirect3DSurface9* pSurface;
                ((IDirect3DTexture9*)pTexture->m_pTexture)->GetSurfaceLevel(0, &pSurface);
                D3DLOCKED_RECT rect;
                pSurface->LockRect(&rect, 0, 0);
                if (!BinkCopyToBuffer(mBink, rect.pBits, rect.Pitch, mBink->Height, 0, 0,
                                      BinkDX9SurfaceType(pSurface) | 0x80000000))
                {
                    TextureComplete(pTexture);
                }
                pSurface->UnlockRect();
                pSurface->Release();
                if (mBink->FrameNum >= mBink->Frames)
                {
                    mFlags |= IsDiscontinuity() ? 4 : 2;
                }
                else
                {
                    BinkNextFrame(mBink);
                }
            }
        }
    }

    if (GetPlayState() == playState_pausing && GetReadBufferFull())
    {
        SetCurrentPlayState(playState_paused);
    }

    RedMoviePlayerBase::Update();

    if (!mBink && GetReadBufferFull() && !sOpeningBinkDecoder)
    {
        sOpeningBinkDecoder = true;
        if (IsEnd() || IsDiscontinuity())
        {
            ResetStart();
        }
        BinkSetIO(BinkFileOpen);
        mBink = BinkOpen("", 0x0B000000);
        if (!mBink)
        {
            PblTraceP("Bink error: %s\n", BinkGetError());
        }
        mFlags |= 1;
        sOpeningBinkDecoder = false;
        CreateBinkDecoder();
    }
}

// SYNTHETIC: SWBF2 0x00409120
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x00409140
// `dynamic initializer for 'PblMath::fOneOverTwoPi''
