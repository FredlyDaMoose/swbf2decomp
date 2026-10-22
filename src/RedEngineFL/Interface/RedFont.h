#pragma once

class PblFileChunk;

#ifndef SWBF2_REDENGINEFL_INTERFACE_REDFONT_H
#define SWBF2_REDENGINEFL_INTERFACE_REDFONT_H

#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Util/ext/hashtable.h"

class RedFont;
class RedTexture;
struct CustomFontData;

// VTABLE: SWBF2 0x00783218
class RedFont
{
public:

    unsigned char GetHeight() const { return m_uiHeight; }

    static void Read(PblFileChunk& chunk);

    struct Glyph
    {
        unsigned short value;
        unsigned char width;
        unsigned char texnum;
        unsigned char left;
        unsigned char right;
        unsigned char top;
        unsigned char bottom;
        float uLeft;
        float uRight;
        float vTop;
        float vBottom;
    };

    RedFont();

    virtual ~RedFont();

    virtual unsigned char GetNumTextures() const = 0;
    virtual unsigned char GetBaseNumTextures() const = 0;
    virtual const RedTexture* GetTexture(unsigned char) const = 0;
    virtual const Glyph* FindGlyph(unsigned short) const = 0;
    virtual bool IsPrintingChar(unsigned short) const = 0;

    virtual void SetNextFont(RedFont* pFont);

    static CustomFontData& GetMagicCharFontData();

    static void Init();

    static void PostCommonInit();
    static void Cleanup();

    static int s_fontHashTableNumEntries;
    static PblHashTable<RedFont, 16> s_fontHashTable;

    static bool sJapaneseOnly;

    static unsigned int GAMEFONT_LARGE;
    static unsigned int GAMEFONT_MEDIUM;
    static unsigned int GAMEFONT_SMALL;
    static unsigned int GAMEFONT_TINY;
    static unsigned int GAMEFONT_SUPER_TINY;

    // FUNCTION: SWBF2 0x0055ad50
    static RedFont* Find(unsigned int uiName)
    {
        if (uiName != PblHash("systemfont") && sJapaneseOnly)
        {
            return s_fontHashTable.Find(PblHash("japanese"));
        }
        return s_fontHashTable.Find(uiName);
    }

    // FUNCTION: SWBF2 0x0055ace0
    static RedFont* Find(const char* pcName) { return Find(PblHash(pcName)); }

    static const unsigned short s_pcSystemFontBitmapData[32768];

    unsigned int m_crcName;
    Glyph* m_aGlyphs;
    unsigned short m_uiNumGlyphs;
    unsigned char m_uiHeight;
    char m_iAscent;
    char m_iDescent;
    RedFont* m_pNextFont;
};
 
 
 
 
 
 
 struct CustomFontData
{

    CustomFontData()
        : nameHash_(0), numGlyphs_(0), glyphHeight_(0), glyphs_(0),
          textureHashes_(0), numTextures_(0), oneTexPerGlyph(true)
    {
    }

    ~CustomFontData()
    {
        delete[] glyphs_;
        delete[] textureHashes_;
    }

    unsigned int nameHash_;
    unsigned int numGlyphs_;
    unsigned int glyphHeight_;
    RedFont::Glyph* glyphs_;
    unsigned int* textureHashes_;
    unsigned int numTextures_;
    bool oneTexPerGlyph;

    util::HashTable<unsigned short, unsigned int, util::NewAllocator> defaultMap_;
};
 
 
 
 

// VTABLE: SWBF2 0x00783238
class RedFixedWidthFont : public RedFont
{
public:

    RedFixedWidthFont(const char* pcName, const RedTexture* pTexture,
                      unsigned char uiCharWidth, unsigned char uiCharHeight,
                      char iAscent, unsigned int uiMaxGlyphs);

    virtual ~RedFixedWidthFont();

    virtual unsigned char GetNumTextures() const;
    virtual unsigned char GetBaseNumTextures() const;
    virtual const RedTexture* GetTexture(unsigned char uiTexture) const;
    virtual const Glyph* FindGlyph(unsigned short) const;
    virtual bool IsPrintingChar(unsigned short) const;

    const RedTexture* m_pTexture;
};
 
 
 // VTABLE: SWBF2 0x00783258
class RedVariableWidthFont : public RedFont
{
public:

    RedVariableWidthFont();

    virtual ~RedVariableWidthFont();

    virtual unsigned char GetNumTextures() const;
    virtual unsigned char GetBaseNumTextures() const;
    virtual const RedTexture* GetTexture(unsigned char uiTexture) const;
    virtual const Glyph* FindGlyph(unsigned short) const;
    virtual bool IsPrintingChar(unsigned short) const;

    void CreateCustom(const CustomFontData& cfd);

    void LoadCustomTextures(const CustomFontData& cfd);

    void Read(PblFileChunk& chunk);

    struct Range
    {
        unsigned short first;
        unsigned short last;
    };

    RedTexture** m_apTextures;
    unsigned char m_uiNumTextures;
    Range* m_aRanges;
    unsigned int m_uiNumRanges;
};
 
 
 
 
 
 
 #endif
