#include "RedAnimation.h"
#include "Zephyr/Source/ZephyrAnimBank.h"
#include "Zephyr/Source/ZephyrEvent.h"
#include "PebbleFL/Common/PblChunk.h"
#include <new>
#include <string.h>
#include "Zephyr/Source/ZephyrSkinShared.h"

// GLOBAL: SWBF2 0x0099e80c
// SYNTHETIC: SWBF2 0x004086d0
// `dynamic initializer for 'RedAnimation::_HashTable''

PblHashTable<RedAnimation, 1024> RedAnimation::_HashTable;

// GLOBAL: SWBF2 0x0099e808
int RedAnimation_HashTableNumEntries;

void RedAnimation::ClearAll()
{
    _HashTable.Clear();
    RedAnimation_HashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x006dd620
RedAnimation::RedAnimation()
{
    _uiNameHash = 0;
    _pSkeleton = 0;
    _pSkeletonShared = 0;
    _pZephyrAnimBank = 0;
    _pEventBank = 0;
}

// FUNCTION: SWBF2 0x006dd6a0
RedAnimation::~RedAnimation()
{
}

// FUNCTION: SWBF2 0x00443990
const ZephyrAnim* RedAnimation::FindAnimation(const char* pszName) const
{
    if (!_pZephyrAnimBank)
    {
        return 0;
    }
    return _pZephyrAnimBank->Find(PblTEMPHash(pszName));
}

// FUNCTION: SWBF2 0x006dd660
ZephyrEventBank::ZephyrEventBank()
{
    _HashTable.Clear();
    _HashTableNumEntries = 0;
    _Lists = 0;
    _iNumLists = 0;
}

// FUNCTION: SWBF2 0x006dd940
void RedAnimation::_ReadEvents(PblMemChunk& chunk)
{
    if (_pEventBank)
    {
        return;
    }

    int iNumAnims = chunk.Read32();
    if (!iNumAnims)
    {
        return;
    }

    _pEventBank = new ZephyrEventBank;
    if (!_pEventBank)
    {
        return;
    }
    _pEventBank->Init(static_cast<short>(iNumAnims));
    char name[100];
    while (iNumAnims-- > 0)
    {
        chunk.ReadString(name, sizeof(name));
        unsigned int uiAnimHash = PblHash(name);
        short iNumEvents = static_cast<short>(chunk.Read32());
        const ZephyrAnim* pAnim = _pZephyrAnimBank->Find(uiAnimHash);
        unsigned int uiNumFrames = pAnim->GetNumFrames();
        ZephyrEventList* pEvents = _pEventBank->New(uiAnimHash);
        pEvents->Init(iNumEvents);
        while (iNumEvents-- > 0)
        {
            int iFrame = chunk.Read32();
            chunk.ReadString(name, sizeof(name));
            pEvents->AddEvent(static_cast<float>(iFrame) /
                                  static_cast<float>(uiNumFrames), PblHash(name));
        }
    }
}

// FUNCTION: SWBF2 0x0072ee50
void ZephyrEventList::AddEvent(float fTime, unsigned int uiMessage)
{
    for (int i = 0; i < _iNumEvents; ++i)
    {
        if (_pEventMessages[i] == 0)
        {
            _pEventTimes[i] = fTime;
            _pEventMessages[i] = uiMessage;
            return;
        }
    }
}

// FUNCTION: SWBF2 0x0072ef00
void ZephyrEventList::Init(short iNumEvents)
{
    _iNumEvents = iNumEvents;
    if (_iNumEvents > 0)
    {
        _pEventTimes = static_cast<float*>(::operator new(4u * _iNumEvents));
        _pEventMessages = static_cast<unsigned int*>(::operator new(4u * _iNumEvents));
        memset(_pEventTimes, 0, 4u * _iNumEvents);
        memset(_pEventMessages, 0, 4u * _iNumEvents);
    }
}

// FUNCTION: SWBF2 0x006ddab0
void RedAnimation::_ReadZaa(PblMemChunk& kChunk)
{
    if (!_pZephyrAnimBank)
    {
        _kZephyrAnimBankReader.ReadFile(kChunk);
        _pZephyrAnimBank = _kZephyrAnimBankReader.GetBank();
    }
}

// FUNCTION: SWBF2 0x006ddae0
void RedAnimation::_ReadZaf(PblMemChunk& kChunk)
{
    if (!_pSkeletonShared)
    {
        _kZephyrSkinReader.ReadFile(kChunk);
        _pSkeletonShared = _kZephyrSkinReader.GetSkin()->GetSkeleton();
    }
}

// FUNCTION: SWBF2 0x006dd6e0
void RedAnimation::ReadZaa(PblMemChunk& kChunk)
{
    RedAnimation* pAnim = 0;
    while (!kChunk.IsDone())
    {
        PblMemChunk kChild = kChunk.ReadNextChild();
        switch (kChild.GetId())
        {
        case 0x454D414E:
        {
            char szName[48];
            kChild.ReadString(szName, 48);
            unsigned int uiHash = PblHash(szName);
            pAnim = RedAnimation::_HashTable.Find(uiHash);
            if (!pAnim)
            {
                pAnim = new RedAnimation;
                pAnim->_uiNameHash = uiHash;
                if (RedAnimation::_HashTable.Store(uiHash, pAnim))
                {
                    ++RedAnimation_HashTableNumEntries;
                }
            }
            break;
        }
        case 0x544E5645:
            pAnim->_ReadEvents(kChild);
            break;
        case 0x5F4E4942:
            pAnim->_ReadZaa(kChild);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x006dd820
void RedAnimation::ReadZaf(PblMemChunk& kChunk)
{
    RedAnimation* pAnim = 0;
    while (!kChunk.IsDone())
    {
        PblMemChunk kChild = kChunk.ReadNextChild();
        switch (kChild.GetId())
        {
        case 0x454D414E:
        {
            char szName[48];
            kChild.ReadString(szName, 48);
            unsigned int uiHash = PblHash(szName);
            pAnim = RedAnimation::_HashTable.Find(uiHash);
            if (!pAnim)
            {
                pAnim = new RedAnimation;
                pAnim->_uiNameHash = uiHash;
                if (RedAnimation::_HashTable.Store(uiHash, pAnim))
                {
                    ++RedAnimation_HashTableNumEntries;
                }
            }
            break;
        }
        case 0x5F4E4942:
            pAnim->_ReadZaf(kChild);
            break;
        }
    }
}
