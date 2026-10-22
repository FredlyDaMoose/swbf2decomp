#pragma once

class PblFileChunk;

#ifndef SWBF2_GRAPHICS_PC_PCREDTEXTURE_H
#define SWBF2_GRAPHICS_PC_PCREDTEXTURE_H

#include <d3d9.h>
#include <stddef.h>

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblList.h"

#include "pcRedRenderer.h"

class RedTexture
{
public:

    static RedTexture* Read(PblFileChunk& chunk);

    // FUNCTION: SWBF2 0x0040f780
    static RedTexture* Find(unsigned int uiHash) { return s_textureHashTable.Find(uiHash); }

    unsigned int GetWidth() const { return m_uiWidth; }
    unsigned int GetHeight() const { return m_uiHeight; }

    unsigned int GetName() const { return m_uiNameHash; }

    static const RedTexture* GetFirst();
    static const RedTexture* GetLast();
    static const RedTexture* GetNext(const RedTexture* const pTexture);
    static const RedTexture* GetPrev(const RedTexture* const pTexture);

    RedTexture();

    ~RedTexture();

    bool _Read(PblFileChunk& chunk);

    static _D3DFORMAT pcChooseFormat(const _D3DFORMAT* aFormats, int iNumFormats);

    static int pcGetBytesFromFormat(_D3DFORMAT format);

    static bool pcIsCompressedFormat(_D3DFORMAT format);

    static bool pcIsLuminosityFormat(_D3DFORMAT format);

    void pcReadFormat(PblFileChunk& chunk, _D3DFORMAT format);

    void pcReadLevel(PblFileChunk& chunk, unsigned int uiSkip, int iFace);

    enum pcRedTextureTypeT
    {
        PC_REDTEXTURE_TYPE_NONE = 0,
        PC_REDTEXTURE_TYPE_TEXTURE = 1,
        PC_REDTEXTURE_TYPE_CUBEMAP = 2,
        PC_REDTEXTURE_TYPE_VOLUME = 3,
        PC_REDTEXTURE_TYPE_FORCEDWORD = 0x7fffffff
    };

    RedTexture(unsigned int uiWidth, unsigned int uiHeight, _D3DFORMAT format,
               unsigned int uiLevels, unsigned int uiUsage, _D3DPOOL pool,
               unsigned int uiNameHash);

    RedTexture(IDirect3DTexture9* pTexture, unsigned int uiNameHash);

    void pcInit(unsigned int uiWidth, unsigned int uiHeight, _D3DFORMAT format,
                unsigned int uiLevels, unsigned int uiUsage, _D3DPOOL pool,
                unsigned int uiNameHash);

    void pcInit(IDirect3DBaseTexture9* pTexture, unsigned int uiNameHash);

    static void Init();

    static void Cleanup();

    static void Remove(unsigned int uiHash);

    static void pcReset(ResetFlagsT eFlags);

    unsigned int m_uiLastFrameUsed;
    unsigned int m_uiNameHash;
    unsigned int m_uiHeight;
    unsigned int m_uiWidth;
    unsigned int m_uiDepth;
    unsigned int m_uiNumMipMaps;

    _D3DFORMAT m_format;
    pcRedTextureTypeT m_type;
    IDirect3DBaseTexture9* m_pTexture;
    _D3DPOOL m_pool;
    unsigned int m_usage;
    unsigned int m_sizeInBytes;
    PblList<RedTexture>::Node m_listNode;

    static PblHashTable<RedTexture, 4096> s_textureHashTable;

    static int s_textureHashTableNumEntries;

    static PblList<RedTexture> s_defaultPoolTextures;
};

 
 
 
 
 
 
 
 
 
 
 
 

template <> void PblList<RedTexture>::Clear();

#endif
