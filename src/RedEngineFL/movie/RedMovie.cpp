#include "RedMovie.h"

#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Interface/RedFont.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "SoundFL/source/sndbus.h"

#include <stdlib.h>

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// FUNCTION: SWBF2 0x006f4c40
RedMovie::Properties::Properties()
    : mHashID(0), mMovieID(0), mFadeInTime(0.0f), mFadeOutTime(0.0f), mSelList(&mList),
      mPrevSegmentInfo(0), mFlags(0)
{
    sPropertyList.Add(this, this);
}

// FUNCTION: SWBF2 0x006f4cd0
RedMovie::RedMovie()
{
    mPlayTime = 0.0f;
    mNumLoops = 0;

    mPropertiesCur = 0;
    mSegmentCur = 0;
    mPropertiesPrev = 0;
    mPropertiesNext = 0;
    mSegmentNext = 0;
    mPosX = 0;
    mPosY = 0;
    mWidth = 0;
    mHeight = 0;
    mAudioBus = 0;
    mChangedSegmentTime = 0.0f;

    mCurSubtitles = 0;
    mDisplaySubtitle = 0;
    mFlags = 0;

    SetNotifyCallback(MovieNotification, this);

    FLRenderer::AddInterfaceElement(&mSubtitleGroup, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::TOP, false);

    mSubtitleGroup.AddElement(&mSubtitles);

    if (0.75f > RedRenderer::GetScreenAspectRatio())
    {
        mSubtitleGroup.m_bUsePerspectiveHack = true;
    }

    mSubtitles.Enable(true);
}

// FUNCTION: SWBF2 0x006f4ea0
RedMovie::SegmentInfo::SegmentInfo(unsigned int uiHashID, float fPlayProbability, int iNumLoops,
                                   PblList<SegmentInfo>& list)
    : mHashID(uiHashID), mPlayProbability(fPlayProbability), mNumLoops(iNumLoops)
{
    list.Add(this, this);
}

// FUNCTION: SWBF2 0x006f4f00
RedMovie::SegmentSubtitles::SegmentSubtitles()
    : mSegmentID(0), mSubtitles(0), mSelSubtitles(0), mNumSubtitles(0), mColor(RedColor::YELLOW)
{
    sList.Add(this, this);
}

// FUNCTION: SWBF2 0x006f5010
RedMovie::~RedMovie()
{
    if (IsOpen())
    {
        Close();
    }
}

// GLOBAL: SWBF2 0x007e1590
PblList<RedMovie::SegmentSubtitles> RedMovie::SegmentSubtitles::sList;

// GLOBAL: SWBF2 0x007e15a4
PblList<RedMovie::Properties> RedMovie::Properties::sPropertyList;

// GLOBAL: SWBF2 0x009c8444
// SYNTHETIC: SWBF2 0x00409160
// `dynamic initializer for 'RedMovie::Properties::sRandom''
PblRandom RedMovie::Properties::sRandom;

// SYNTHETIC: SWBF2 0x004091a0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x004091c0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''

// GLOBAL: SWBF2 0x009c8434
bool RedMovie::sSubtitlesEnabled;

// SYNTHETIC: SWBF2 0x006f5110
// RedMovie::`scalar deleting destructor'

// TEMPLATE: SWBF2 0x006f5140
// Snd::QueueCircular<RedMovie::Properties const *,11>::Add

// TEMPLATE: SWBF2 0x006f51b0
// Snd::QueueCircular<RedMovie::SegmentInfo const *,11>::Add

// TEMPLATE: SWBF2 0x006f53c0
// RedPipe<RedTexture const *,1>::Flow

// TEMPLATE: SWBF2 0x006f5660
// RedPipePointer<RedTexture const *,1>::GetRecent

// FUNCTION: SWBF2 0x006f5220
void RedMovie::Close()
{
    Stop(true);
    RedMoviePlayer::Close();
}

// FUNCTION: SWBF2 0x006f5240
int RedMovie::Subtitle::CompareSearch(const void* pKey, const void* pElement)
{
    const float fTime = *(const float*)pKey;
    const Subtitle* pSubtitle = (const Subtitle*)pElement;

    if (fTime < pSubtitle->mEnableTime || pSubtitle->mDisableTime < fTime)
    {
        return (int)((fTime - pSubtitle->mEnableTime) * 100.0f);
    }
    return 0;
}

// FUNCTION: SWBF2 0x006f5280
int RedMovie::Subtitle::CompareSort(const void* pElement1, const void* pElement2)
{
    return (int)(((const Subtitle*)pElement1)->mEnableTime -
                 ((const Subtitle*)pElement2)->mEnableTime);
}

// FUNCTION: SWBF2 0x006f4fb0
RedMovie::Properties::~Properties()
{
    GetList()->Remove(this);
    for (PblList<SegmentInfo>::Itor it(*mSelList); it != mSelList->End();)
    {
        SegmentInfo* pInfo = it;
        ++it;
        delete pInfo;
    }
}

// FUNCTION: SWBF2 0x006f50b0
RedMovie::SegmentInfo::~SegmentInfo()
{
    GetList()->Remove(this);
}

// FUNCTION: SWBF2 0x006f50e0
RedMovie::SegmentSubtitles::~SegmentSubtitles()
{
    GetList()->Remove(this);
    FreeSubtitles();
}

// FUNCTION: SWBF2 0x006f52a0
void RedMovie::Properties::DestroyAll()
{
    while (sPropertyList.GetCount())
    {
        delete sPropertyList.GetFirst()->_pObject;
    }
}

// FUNCTION: SWBF2 0x006f52f0
void RedMovie::SegmentSubtitles::DestroyAll()
{
    while (sList.GetCount())
    {
        delete sList.GetFirst()->_pObject;
    }
}

// FUNCTION: SWBF2 0x006f52e0
void RedMovie::DestroyAll()
{
    Properties::DestroyAll();
    SegmentSubtitles::DestroyAll();
}

// FUNCTION: SWBF2 0x006f5330
const RedMovie::Properties* RedMovie::Properties::FindByHashID(unsigned int uiHashID)
{
    for (PblList<Properties>::Itor it(sPropertyList); it != sPropertyList.End(); ++it)
    {
        if (it->GetHashID() == uiHashID)
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006f5380
const RedMovie::SegmentSubtitles* RedMovie::SegmentSubtitles::FindBySegmentID(
    unsigned int uiSegmentID)
{
    for (PblList<SegmentSubtitles>::Itor it(sList); it; ++it)
    {
        if (uiSegmentID == it->GetSegmentID())
        {
            return it;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x006f5400
void RedMovie::Flush(bool bFlushPlayer)
{
    const RedTexture* pTexture = 0;
    const RedTexture* pLocked = 0;
    do
    {
        UnlockTexture(pTexture);
        pTexture = pLocked;
        pLocked = LockTexture();
    } while (pLocked);

    if (!pTexture)
    {
        pTexture = mTextures.GetRecent();
    }

    if (pTexture)
    {
        const RedTexture* pOut;
        do
        {
            pOut = mFadeTextures.Flow(pTexture);
            UnlockTexture(pOut);
        } while (pOut != pTexture);
    }

    if (bFlushPlayer)
    {
        RedMoviePlayer::Flush();
    }
}

// FUNCTION: SWBF2 0x006f54b0
void RedMovie::FlushPipe(RedPipePointer<const RedTexture*, 1>& pipe)
{
    const RedTexture* pTexture;
    while ((pTexture = pipe.Flow(0)) != 0)
    {
        UnlockTexture(pTexture);
    }
}

// FUNCTION: SWBF2 0x006f5500
void RedMovie::SegmentSubtitles::FreeSubtitles()
{
    if (mSubtitles)
    {
        delete[] mSubtitles;
        mSelSubtitles = 0;
        mSubtitles = 0;
        mNumSubtitles = 0;
    }
}

// FUNCTION: SWBF2 0x006f5530
RedColor RedMovie::SegmentSubtitles::GetColor() const
{
    return mColor;
}

// FUNCTION: SWBF2 0x006f5560
float RedMovie::Properties::GetFadeInTime() const
{
    return mFadeInTime;
}

// FUNCTION: SWBF2 0x006f5570
float RedMovie::Properties::GetFadeOutTime() const
{
    return mFadeOutTime;
}

// FUNCTION: SWBF2 0x006f5580
unsigned int RedMovie::SegmentSubtitles::GetFontID() const
{
    return mFontID;
}

// FUNCTION: SWBF2 0x006f5590
unsigned int RedMovie::Properties::GetHashID() const
{
    return mHashID;
}

// FUNCTION: SWBF2 0x006f55a0
unsigned int RedMovie::SegmentInfo::GetHashID() const
{
    return mHashID;
}

// FUNCTION: SWBF2 0x006f55b0
unsigned int RedMovie::Properties::GetMovieID() const
{
    return mMovieID;
}

// FUNCTION: SWBF2 0x006f55c0
int RedMovie::SegmentInfo::GetNumLoops() const
{
    return mNumLoops;
}

// FUNCTION: SWBF2 0x006f55d0
const RedMovie::Properties* RedMovie::GetProperties() const
{
    return mPropertiesCur;
}

// FUNCTION: SWBF2 0x006f55e0
bool RedMovie::GetPropertiesScheduled(const Properties* const pProperties) const
{
    if (pProperties == GetProperties())
    {
        return true;
    }

    if (!mPropertyQueue.IsEmpty())
    {
        const unsigned short usNumItems = mPropertyQueue.GetNumItems();
        for (unsigned short i = 0; i < usNumItems; ++i)
        {
            if (pProperties == *mPropertyQueue.Peek(i))
            {
                return true;
            }
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x006f5680
unsigned int RedMovie::SegmentSubtitles::GetSegmentID() const
{
    return mSegmentID;
}

// FUNCTION: SWBF2 0x006f5690
const RedMovie::SegmentInfo* RedMovie::Properties::GetSegmentInfo() const
{
    if (mPrevSegmentInfo && (mFlags & 1))
    {
        PblList<SegmentInfo>::Node* pNode = mPrevSegmentInfo->_pNext;
        if (pNode->_pObject == mSelList->_head._pObject)
        {
            pNode = mSelList->_head._pNext;
        }
        mPrevSegmentInfo = pNode->_pObject;
        if (mPrevSegmentInfo)
        {
            return mPrevSegmentInfo;
        }
    }

    const float fRandom = sRandom.GetFloat();
    SegmentInfo* pSegmentInfo = mSelList->GetFirst()->_pObject;
    for (PblList<SegmentInfo>::Itor it(*mSelList); it != mSelList->End(); ++it)
    {
        pSegmentInfo = it;
        if (fRandom >= pSegmentInfo->GetPlayProbability())
        {
            break;
        }
    }
    mPrevSegmentInfo = pSegmentInfo;
    return pSegmentInfo;
}

// FUNCTION: SWBF2 0x006f5740
const RedMovie::Subtitle* RedMovie::SegmentSubtitles::GetSubtitle(float fTime) const
{
    return (const Subtitle*)bsearch(&fTime, mSubtitles, mNumSubtitles, sizeof(Subtitle),
                                    Subtitle::CompareSearch);
}

// FUNCTION: SWBF2 0x006f5770
bool RedMovie::IsOpen() const
{
    return RedMoviePlayerBase::IsOpen();
}

// FUNCTION: SWBF2 0x006f5780
bool RedMovie::IsPlaying() const
{
    return (mFlags & flags_Playing) != 0;
}

// FUNCTION: SWBF2 0x006f5790
void RedMovie::Load(PblFileChunk& chunk, bool bOne)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId != 0x41544144)
        {
            continue;
        }

        const unsigned int uiClass = (unsigned int)child._pFile->Read32();
        child = chunk.ReadNextChild();

        switch (uiClass)
        {
        case 0xb0e44c26:
        case 0xaef39ccf:
            (new Properties)->Read(child);
            break;
        case 0x1dcef1fe:
        case 0xe396fe20:
            (new SegmentSubtitles)->Read(child);
            break;
        default:
            continue;
        }

        if (bOne)
        {
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006f5890
void RedMovie::Loop(const SegmentInfo* const pSegmentInfo)
{
    if (pSegmentInfo->GetNumLoops() == -1 || (int)mNumLoops < pSegmentInfo->GetNumLoops())
    {
        ++mNumLoops;
        mPropertiesNext = mPropertiesPrev;
        mSegmentNext = (SegmentInfo*)pSegmentInfo;
    }
    else if (mFlags & flags_Loop)
    {
        Play(mPropertiesPrev, true);
    }
}

// FUNCTION: SWBF2 0x006f5900
void RedMovie::MovieNotification(RedMoviePlayerBase* pPlayer, NotifyType eType, void* pData)
{
    RedMovie* pMovie = (RedMovie*)pData;
    const SegmentInfo* pSegmentInfo = pMovie->mSegmentCur;
    bool bEmpty = pMovie->mPropertyQueue.IsEmpty();

    switch (eType)
    {
    case notifyType_BufferingComplete:
        if (!bEmpty)
        {
            pSegmentInfo = *pMovie->mSegmentInfoQueue.Peek(0);
            if (pSegmentInfo->GetHashID() != pPlayer->GetCurrentSegment()->GetHashID())
            {
                return;
            }

            const Properties* pProperties;
            pMovie->mPropertyQueue.Get(pProperties);
            pMovie->mPropertiesPrev = (Properties*)pProperties;
            pMovie->mSegmentInfoQueue.Get(pSegmentInfo);
        }
        if (pMovie->mPropertyQueue.GetNumItems() == 0)
        {
            pMovie->Loop(pSegmentInfo);
        }
        break;

    case notifyType_End:
    case notifyType_Discontinuity:
        if (pMovie->mNumLoops)
        {
            pMovie->mPropertyQueue.Add(pMovie->mPropertiesNext);
            pMovie->mSegmentInfoQueue.Add(pMovie->mSegmentNext);
            pMovie->mPropertiesNext = 0;
            pMovie->mSegmentNext = 0;
            bEmpty = false;
        }
        pMovie->UpdateCurrent();
        pMovie->Flush(false);
        pMovie->mFlags &= ~flags_Rendering;
        pMovie->mPlayTime = 0.0f;
        if (bEmpty)
        {
            pMovie->Stop(false);
        }
        break;
    }
}

// FUNCTION: SWBF2 0x006f5a50
bool RedMovie::Open(const char* const pFileName, int iHeap, int iUnused, unsigned int uiArg4,
                    unsigned int uiBufferSize, unsigned int uiReadSize,
                    unsigned int uiNumTextures)
{
    RedMoviePlayerBase::SetLooping(true);
    return RedMoviePlayer::Open(pFileName, uiNumTextures, iHeap, iUnused, uiArg4,
                                uiBufferSize == 0xffffffffu ? 0x40000 : uiBufferSize,
                                uiReadSize == 0xffffffffu ? 0x20000 : uiReadSize, 2);
}

// FUNCTION: SWBF2 0x006f5aa0
bool RedMovie::PipeTextureExists(const RedTexture* pTexture) const
{
    return mTextures.Exists(pTexture) || mFadeTextures.Exists(pTexture);
}

// FUNCTION: SWBF2 0x006f5af0
void RedMovie::Play(const Properties* const pProperties, const SegmentInfo* const pSegmentInfo,
                    bool bResetLoops, bool bPlayAtEnd)
{
    const Segment* pSegment = GetInfo()->FindSegmentByHashID(pSegmentInfo->GetHashID());
    if (!pSegment)
    {
        RedMoviePlayerBase::Stop();
        return;
    }

    if (bResetLoops)
    {
        mNumLoops = 0;
    }

    mPropertyQueue.Add(pProperties);
    mSegmentInfoQueue.Add(pSegmentInfo);
    UpdateCurrent();
    SeekSegment(pSegment, bPlayAtEnd);

    if (GetPlayState() == playState_stopped)
    {
        Pause();
    }
    else
    {
        RedMoviePlayerBase::Play();
    }

    mFlags |= flags_Playing;
}

// FUNCTION: SWBF2 0x006f5ba0
void RedMovie::Play(const Properties* const pProperties, bool bPlayAtEnd)
{
    const SegmentInfo* pSegmentInfo = pProperties->GetSegmentInfo();
    if (pSegmentInfo)
    {
        Play(pProperties, pSegmentInfo, !bPlayAtEnd, bPlayAtEnd);
    }
}

// FUNCTION: SWBF2 0x006f5bd0
void RedMovie::Properties::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch ((unsigned int)child._pFile->Read32())
        {
        case 0x5083ecd9:
            child._pFile->Read8();
            SetFadeInTime(child._pFile->ReadFloat());
            break;
        case 0x4d5af670:
            child._pFile->Read8();
            SetCyclePlayback(child._pFile->Read32() > 0);
            break;
        case 0x0a8325f2:
            child = chunk.ReadNextChild();
            ReadSegments(child);
            mSelList = &mList;
            break;
        case 0x798dc484:
            child._pFile->Read8();
            SetFadeOutTime(child._pFile->ReadFloat());
            break;
        case 0x8d39bde6:
            child._pFile->Read8();
            mHashID = child._pFile->Read32();
            break;
        case 0xaef39ccf:
            child._pFile->Read8();
            SetMovieID(child._pFile->Read32());
            break;
        case 0xca04efe0:
        {
            child._pFile->Read8();
            const Properties* pBase = FindByHashID(child._pFile->Read32());
            if (pBase)
            {
                SetMovieID(pBase->GetMovieID());
                SetFadeInTime(pBase->GetFadeInTime());
                SetFadeOutTime(pBase->GetFadeOutTime());
                mSelList = pBase->mSelList;
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x006f5d90
void RedMovie::SegmentSubtitles::Read(PblFileChunk& chunk)
{
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch ((unsigned int)child._pFile->Read32())
        {
        case 0x274e1290:
            child._pFile->Read8();
            mFontID = child._pFile->Read32();
            break;
        case 0x09836605:
            child = chunk.ReadNextChild();
            ReadSubtitles(child);
            break;
        case 0x3d7e6258:
        {
            const char cNumValues = child._pFile->Read8();
            mColor.r = cNumValues >= 1 ? (unsigned char)child._pFile->ReadFloat() : 0xff;
            mColor.g = cNumValues >= 2 ? (unsigned char)child._pFile->ReadFloat() : 0xff;
            mColor.b = cNumValues >= 3 ? (unsigned char)child._pFile->ReadFloat() : 0xff;
            mColor.a = cNumValues >= 4 ? (unsigned char)child._pFile->ReadFloat() : 0xff;
            break;
        }
        case 0xf3c342e6:
            child._pFile->Read8();
            mSegmentID = child._pFile->Read32();
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006f5f00
void RedMovie::Properties::ReadSegments(PblFileChunk& chunk)
{
    float fTotalProbability = 0.0f;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if ((unsigned int)child._pFile->Read32() != 0xf3c342e6)
        {
            continue;
        }

        child._pFile->Read8();
        const unsigned int uiHashID = child._pFile->Read32();
        const float fProbability = child._pFile->ReadFloat();
        const float fNumLoops = child._pFile->ReadFloat();
        fTotalProbability += fProbability;
        new SegmentInfo(uiHashID, fTotalProbability, (int)fNumLoops, mList);
    }

    if (fTotalProbability > 0.0f)
    {
        for (PblList<SegmentInfo>::Itor it(mList); it != mList.End(); ++it)
        {
            it->mPlayProbability = 1.0f - it->mPlayProbability / fTotalProbability;
        }
    }
}

// FUNCTION: SWBF2 0x006f6010
void RedMovie::SegmentSubtitles::ReadSubtitles(PblFileChunk& chunk)
{
    Subtitle subtitles[256];
    unsigned int uiNumSubtitles = 0;

    FreeSubtitles();

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if ((unsigned int)child._pFile->Read32() != 0x115bfcb9)
        {
            continue;
        }

        child._pFile->Read8();
        const unsigned int uiStringID = child._pFile->Read32();
        const float fStart = child._pFile->ReadFloat();
        const float fDuration = child._pFile->ReadFloat();
        if (uiNumSubtitles < 256)
        {
            subtitles[uiNumSubtitles++].Init(uiStringID, fStart, fStart + fDuration);
        }
    }

    if (uiNumSubtitles)
    {
        mSubtitles = new Subtitle[uiNumSubtitles];
        if (mSubtitles)
        {
            for (unsigned int i = 0; i < uiNumSubtitles; ++i)
            {
                mSubtitles[i] = subtitles[i];
            }
            mSelSubtitles = mSubtitles;
            mNumSubtitles = uiNumSubtitles;
            qsort(mSubtitles, uiNumSubtitles, sizeof(Subtitle), Subtitle::CompareSort);
        }
    }
}

// FUNCTION: SWBF2 0x006f62d0
void RedMovie::Render()
{
    if (!IsPlaying())
    {
        return;
    }

    if (GetPlayState() != playState_playing)
    {
        const RedTexture* pTexture = mTextures.Flow(mTextures.GetRecent());
        if (pTexture)
        {
            RedMoviePlayer::Render(pTexture, mPosX, mPosY, mWidth, mHeight, 0x80);
        }
        return;
    }

    const Properties* pProperties = GetProperties();

    const RedTexture* pFadeTexture = mFadeTextures.Peek(0);
    if (pFadeTexture)
    {
        if (mPropertiesPrev && mPropertiesPrev->GetFadeOutTime() > mPlayTime)
        {
            const float fFade = 1.0f - mPlayTime / mPropertiesPrev->GetFadeOutTime();
            mFadeTextures.Flow(pFadeTexture);

            float fAlpha = fFade * 255.0f;
            if (0.0f > fAlpha)
            {
                fAlpha = 0.0f;
            }
            else if (fAlpha > 255.0f)
            {
                fAlpha = 255.0f;
            }
            RedMoviePlayer::Render(pFadeTexture, mPosX, mPosY, mWidth, mHeight,
                                   (unsigned char)fAlpha);
        }
        else
        {
            UnlockTexture(mFadeTextures.Flow(0));
        }
    }

    const RedTexture* pTexture = LockTexture();

    unsigned char ucAlpha = 0xff;
    if (pProperties->GetFadeInTime() > mPlayTime)
    {
        float fAlpha = mPlayTime / pProperties->GetFadeInTime() * 255.0f;
        if (0.0f > fAlpha)
        {
            fAlpha = 0.0f;
        }
        else if (fAlpha > 255.0f)
        {
            fAlpha = 255.0f;
        }
        ucAlpha = (unsigned char)fAlpha;
    }

    if (pTexture)
    {
        mFlags |= flags_Rendering;
    }
    else
    {
        pTexture = mTextures.GetRecent();
    }

    if (pTexture)
    {
        RedMoviePlayer::Render(pTexture, mPosX, mPosY, mWidth, mHeight, ucAlpha);
        if (mNumTextures > 1)
        {
            pTexture = mTextures.Flow(pTexture);
        }
        UnlockTexture(pTexture);
    }
}

// FUNCTION: SWBF2 0x006f6560
void RedMovie::SetAudioBus(Snd::Bus* pBus)
{
    mAudioBus = pBus;
}

// FUNCTION: SWBF2 0x006f6570
void RedMovie::Properties::SetCyclePlayback(bool bCycle)
{
    if (bCycle)
    {
        mFlags |= 1;
    }
    else
    {
        mFlags &= ~1;
    }
}

// FUNCTION: SWBF2 0x006f6590
void RedMovie::SetDimensions(int iX, int iY, unsigned int uiWidth, unsigned int uiHeight,
                             bool bSafeArea)
{
    mPosX = iX;
    mPosY = iY;
    mWidth = uiWidth;
    mHeight = uiHeight;

    float fLeft, fTop, fRight, fBottom;
    if (bSafeArea)
    {
        RedInterfaceScreen::s_screenFull.GetSafeArea(fLeft, fTop, fRight, fBottom);
    }
    else
    {
        fLeft = (float)iX;
        fTop = (float)iY;
        fRight = (float)(int)uiWidth + fLeft;
        fBottom = (float)(int)uiHeight + fTop;
    }

    float fWidth = fRight - fLeft;
    float fHeight = fBottom - fTop;

    const float fPosY = (float)mPosY;
    const float fPosX = (float)mPosX;
    mSubtitles.SetPosition(fLeft > fPosX ? fLeft : fPosX, fTop > fPosY ? fTop : fPosY);
    mSubtitles.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER,
                            RedTextElement::TEXT_VALIGN_BOTTOM);

    if (!((float)mHeight > fHeight))
    {
        fHeight = (float)mHeight;
    }
    if (!((float)mWidth > fWidth))
    {
        fWidth = (float)mWidth;
    }
    mSubtitles.SetTextBox((unsigned int)fWidth, (unsigned int)fHeight);
}

// FUNCTION: SWBF2 0x006f6710
void RedMovie::Properties::SetFadeInTime(float fTime)
{
    mFadeInTime = fTime;
}

// FUNCTION: SWBF2 0x006f6730
void RedMovie::Properties::SetFadeOutTime(float fTime)
{
    mFadeOutTime = fTime;
}

// FUNCTION: SWBF2 0x006f6750
void RedMovie::SetLooping(bool bLoop)
{
    if (bLoop)
    {
        mFlags |= flags_Loop;
    }
    else
    {
        mFlags &= ~flags_Loop;
    }
}

// FUNCTION: SWBF2 0x006f6770
void RedMovie::Properties::SetMovieID(unsigned int uiMovieID)
{
    mMovieID = uiMovieID;
}

// FUNCTION: SWBF2 0x006f6780
void RedMovie::Stop(bool bUpdate)
{
    FlushPipe(mTextures);
    FlushPipe(mFadeTextures);
    mPropertyQueue.Empty(0, 0);
    mSegmentInfoQueue.Empty(0, 0);
    UpdateCurrent();
    RedMoviePlayerBase::Stop();

    if (bUpdate)
    {
        RedMoviePlayer::Update();
    }

    mFlags &= ~(flags_Playing | flags_Rendering);
    mPlayTime = 0.0f;
    mPropertiesCur = 0;
    mSegmentCur = 0;
    mPropertiesPrev = 0;
    mCurSubtitles = 0;
    mDisplaySubtitle = 0;
    mSubtitleGroup.Enable(false);
}

// FUNCTION: SWBF2 0x006f6840
void RedMovie::SubtitlesEnable(bool bEnable)
{
    sSubtitlesEnabled = bEnable;
}

// FUNCTION: SWBF2 0x006f6850
void RedMovie::UnlockTexture(const RedTexture* const pTexture)
{
    if (pTexture && !PipeTextureExists(pTexture))
    {
        RedMoviePlayerBase::UnlockTexture(pTexture);
    }
}

// FUNCTION: SWBF2 0x006f6880
void RedMovie::Update(float fDeltaTime)
{
    Properties::sRandom.GetInt();

    RedMoviePlayer::Update();
    mChangedSegmentTime = 0.0f;

    if (!IsPlaying())
    {
        mSubtitleGroup.Enable(false);
        return;
    }

    if (mAudioBus)
    {
        SetGain(mAudioBus->mFinalGain);
    }

    if (mCurSubtitles)
    {
        const Subtitle* pSubtitle = mCurSubtitles->GetSubtitle(GetPlaybackTime());
        if (mDisplaySubtitle != pSubtitle)
        {
            bool bEnable = false;
            if (pSubtitle)
            {
                const unsigned short* pText = RedLocalizeDB::Find(pSubtitle->mStringID, false);
                if (pText)
                {
                    mSubtitles.SetText(pText, true);
                    if (sSubtitlesEnabled)
                    {
                        bEnable = true;
                    }
                }
            }
            mSubtitleGroup.Enable(bEnable);
            mDisplaySubtitle = (Subtitle*)pSubtitle;
        }
    }
    else
    {
        mSubtitleGroup.Enable(false);
    }

    switch (GetPlayState())
    {
    case playState_stopped:
        mSubtitleGroup.Enable(false);
        mSubtitles.SetText((const unsigned short*)0, true);
        break;
    case playState_paused:
        RedMoviePlayerBase::Play();
        break;
    case playState_playing:
        if (mFlags & flags_Rendering)
        {
            mPlayTime += fDeltaTime;
        }
        break;
    }
}

// FUNCTION: SWBF2 0x006f69f0
void RedMovie::UpdateCurrent()
{
    mPropertiesCur = mPropertyQueue.IsEmpty() ? 0 : (Properties*)*mPropertyQueue.Peek(0);
    mSegmentCur = mSegmentInfoQueue.IsEmpty() ? 0 : (SegmentInfo*)*mSegmentInfoQueue.Peek(0);
    mCurSubtitles = mSegmentCur ? (SegmentSubtitles*)SegmentSubtitles::FindBySegmentID(
                                      mSegmentCur->GetHashID())
                                : 0;
    mDisplaySubtitle = 0;

    if (mCurSubtitles)
    {
        mSubtitles.SetColor(mCurSubtitles->GetColor());
        const RedFont* pFont = RedFont::Find(mCurSubtitles->GetFontID());
        if (!pFont)
        {
            pFont = RedFont::Find(RedFont::GAMEFONT_TINY);
        }
        mSubtitles.SetFont(pFont);
    }
}

// FUNCTION: SWBF2 0x006f4f90
void __fastcall DestroyMovieQueueOnCtorUnwind(void* pThis)
{
    ((Snd::Queue<const RedMovie::Properties*, 11>*)pThis)->~Queue();
}

// FUNCTION: SWBF2 0x006f4fa0
void __fastcall DestroyMovieQueueOnTeardown(void* pThis)
{
    ((Snd::Queue<const RedMovie::SegmentInfo*, 11>*)pThis)->~Queue();
}
