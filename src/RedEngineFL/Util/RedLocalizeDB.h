#pragma once

class PblFileChunk;

enum eGAMELANGUAGE
{
    eLANG_UNRESTRICTED = -2,
    eLANG_AUTODETERMINE = -1,
    eLANG_ENGLISH = 0,
    eLANG_SPANISH = 1,
    eLANG_ITALIAN = 2,
    eLANG_FRENCH = 3,
    eLANG_GERMAN = 4,
    eLANG_JAPANESE = 5,
    eLANG_UK = 6,
    eLANG_THAI = 7,
    eLANG_POLISH = 8,
    eLANG_MAX = 9
};

namespace RedLocalizeDB
{

eGAMELANGUAGE GetLanguageID();

void PickLanguage();

char const* GetLanguageStr();

bool Read(PblFileChunk& chunk);

void ReadMasterLocalizeFile(PblFileChunk& chunk);

const unsigned short* Find(unsigned int uiHash, bool bNullIfMissing);

void GetLocalizedString(unsigned short* pusBuffer, unsigned int uiSize, unsigned int uiHash);

void Reset();

}

struct FLLocalizeEntry
{
    int mEntryHash;
    unsigned short mEntryLen;
    unsigned short mData[1];
};
