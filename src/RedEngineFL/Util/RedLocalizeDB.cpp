#include <windows.h>
#include <string.h>

#include "RedLocalizeDB.h"

#include "Battlefront2/Source/GameMemory.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Memory/RedMemory.h"

// GLOBAL: SWBF2 0x00940bf0
FLLocalizeEntry* mPatchLocalizeFile;

void __cdecl PblTraceP(const char* Format, ...);

FLLocalizeEntry* mMasterLocalizeFile;

bool gbDeterminedLang;
eGAMELANGUAGE gUseLang;
int mSkipCount;

int sMasterLocalizeHashNumEntries;
// GLOBAL: SWBF2 0x00940c04
// SYNTHETIC: SWBF2 0x00407d60
// `dynamic initializer for 'sMasterLocalizeHash''

PblHashTable<FLLocalizeEntry, 16384> sMasterLocalizeHash;

char const* gLanguageStr;

eGAMELANGUAGE gForceLang = eLANG_AUTODETERMINE;

namespace PblBuildType {
// GLOBAL: SWBF2 0x007e2e00
eGAMELANGUAGE gForceLang1 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e04
eGAMELANGUAGE gForceLang2 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e08
eGAMELANGUAGE gForceLang3 = eLANG_UNRESTRICTED;
// GLOBAL: SWBF2 0x007e2e10
bool gDemoEnglishOnly = true;
// GLOBAL: SWBF2 0x009cef71
bool gDemoBuild;
}

namespace RedLocalizeDB
{

// FUNCTION: SWBF2 0x006c5c30
void PickLanguage()
{
    gUseLang = eLANG_ENGLISH;

    eGAMELANGUAGE eLang;
    switch (PRIMARYLANGID(GetUserDefaultLangID()))
    {
    case LANG_SPANISH:  eLang = eLANG_SPANISH;  break;
    case LANG_ITALIAN:  eLang = eLANG_ITALIAN;  break;
    case LANG_FRENCH:   eLang = eLANG_FRENCH;   break;
    case LANG_GERMAN:   eLang = eLANG_GERMAN;   break;
    case LANG_JAPANESE: eLang = eLANG_JAPANESE; break;
    case LANG_THAI:     eLang = eLANG_THAI;     break;
    case LANG_POLISH:   eLang = eLANG_POLISH;   break;
    default:            eLang = eLANG_ENGLISH;  break;
    }

    if (PblBuildType::gDemoBuild && PblBuildType::gDemoEnglishOnly)
    {
        eLang = eLANG_ENGLISH;
    }

    if (PblBuildType::gForceLang1 >= 0 && eLang != PblBuildType::gForceLang1
        && eLang != PblBuildType::gForceLang2 && eLang != PblBuildType::gForceLang3)
    {
        eLang = PblBuildType::gForceLang1;
    }

    if (gForceLang != eLANG_AUTODETERMINE)
    {
        eLang = gForceLang;
    }

    gUseLang = eLang;

    switch (gUseLang)
    {
    case eLANG_SPANISH:  gLanguageStr = "spanish";    break;
    case eLANG_ITALIAN:  gLanguageStr = "italian";    break;
    case eLANG_FRENCH:   gLanguageStr = "french";     break;
    case eLANG_GERMAN:   gLanguageStr = "german";     break;
    case eLANG_JAPANESE: gLanguageStr = "japanese";   break;
    case eLANG_UK:       gLanguageStr = "uk_english"; break;
    case eLANG_THAI:     gLanguageStr = "thai";       break;
    case eLANG_POLISH:   gLanguageStr = "polish";     break;
    default:             gLanguageStr = "english";    break;
    }

    gbDeterminedLang = true;
}

// FUNCTION: SWBF2 0x006c5b30
const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing)
{
    if (uiHash == 0xb0a4d342)
    {
        return (const unsigned short*)L"Cannot join: does not exist";
    }
    if (uiHash == 0xe6407c4d)
    {
        return (const unsigned short*)L"Cannot join: unable to connect";
    }

    FLLocalizeEntry* pEntry = sMasterLocalizeHash.Find(uiHash);
    if (pEntry)
    {
        return pEntry->mData;
    }
    return bNullIfMissing ? 0 : (const unsigned short*)L"[NULL]";
}

// FUNCTION: SWBF2 0x006c5bf0
void GetLocalizedString(unsigned short* pusBuffer, unsigned int uiSize, unsigned int uiHash)
{
    memset(pusBuffer, 0, uiSize);
    const unsigned short* pusString = Find(uiHash, false);
    if (pusString)
    {
        PblUnicode::ustrncpy(pusBuffer, pusString, uiSize - 1);
    }
}

// FUNCTION: SWBF2 0x006c5bb0
eGAMELANGUAGE GetLanguageID()
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    return gUseLang;
}

// FUNCTION: SWBF2 0x006c5bd0
char const* GetLanguageStr()
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    return gLanguageStr;
}

// FUNCTION: SWBF2 0x006c5e50
bool Read(PblFileChunk& chunk)
{
    if (!gbDeterminedLang)
    {
        PickLanguage();
    }

    bool bLanguage = false;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x454D414Eu)
        {
            char acName[256];
            child.ReadString(acName, 256);
            bLanguage = _stricmp(acName, gLanguageStr) == 0;
        }
        else if (child._iId == 0x59444F42u)
        {
            if (bLanguage)
            {
                if (mSkipCount)
                {
                    --mSkipCount;
                }
                else
                {
                    ReadMasterLocalizeFile(child);
                }
            }
        }
        else
        {
            PblTraceP("Uhoh? Unknown chunk %08X\n", child._iId);
        }
    }

    return bLanguage;
}

// FUNCTION: SWBF2 0x006c5f40
void ReadMasterLocalizeFile(PblFileChunk& chunk)
{
    const int iSize = chunk._iSize;
    mMasterLocalizeFile = (FLLocalizeEntry*)RedAllocFromHeap(GameMemory::RunTimeHeap, iSize, 0);
    chunk._pFile->ReadData(mMasterLocalizeFile, iSize);

    for (FLLocalizeEntry* pEntry = mMasterLocalizeFile; pEntry->mEntryLen;
         pEntry = (FLLocalizeEntry*)((char*)pEntry + pEntry->mEntryLen))
    {
        if (!sMasterLocalizeHash.Find(pEntry->mEntryHash)
            && sMasterLocalizeHash.Store(pEntry->mEntryHash, pEntry))
        {
            ++sMasterLocalizeHashNumEntries;
        }
    }
}

}

// FUNCTION: SWBF2 0x006c5fd0
void RedLocalizeDB::Reset()
{
    sMasterLocalizeHash.Clear();
    sMasterLocalizeHashNumEntries = 0;
    mMasterLocalizeFile = 0;
    mPatchLocalizeFile = 0;
}
