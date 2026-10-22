#include "pcRedTexture.h"

#include <d3dx9.h>
#include <string.h>

#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "PebbleFL/Common/PblUnicode.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"

// GLOBAL: SWBF2 0x008eed8c
// SYNTHETIC: SWBF2 0x00407750
// `dynamic initializer for 'RedTexture::s_textureHashTable''

PblHashTable<RedTexture, 4096> RedTexture::s_textureHashTable;

// GLOBAL: SWBF2 0x008eed88
int RedTexture::s_textureHashTableNumEntries;

// GLOBAL: SWBF2 0x007de9a0
PblList<RedTexture> RedTexture::s_defaultPoolTextures;

// GLOBAL: SWBF2 0x007de950
const char* kLOCALIZE_TEXTURE_PREFIX = "_LOCALIZE_";

// FUNCTION: SWBF2 0x006b9f90
RedTexture::RedTexture(unsigned int uiWidth, unsigned int uiHeight, _D3DFORMAT format,
                       unsigned int uiLevels, unsigned int uiUsage, _D3DPOOL pool,
                       unsigned int uiNameHash)
{
    m_uiLastFrameUsed = 0;
    m_uiNameHash = 0;
    m_uiHeight = 0;
    m_uiWidth = 0;
    m_uiDepth = 0;
    m_uiNumMipMaps = 0;
    m_format = (_D3DFORMAT)0;
    m_type = PC_REDTEXTURE_TYPE_NONE;
    m_pTexture = 0;
    m_pool = pool;
    m_usage = uiUsage;
    m_sizeInBytes = 0;

    pcInit(uiWidth, uiHeight, format, uiLevels, uiUsage, pool, uiNameHash);
}

// FUNCTION: SWBF2 0x006ba030
RedTexture::RedTexture(IDirect3DTexture9* pTexture, unsigned int uiNameHash)
{
    m_uiLastFrameUsed = 0;
    m_uiNameHash = 0;
    m_uiHeight = 0;
    m_uiWidth = 0;
    m_uiDepth = 0;
    m_uiNumMipMaps = 0;
    m_format = (_D3DFORMAT)0;
    m_type = PC_REDTEXTURE_TYPE_NONE;
    m_pTexture = 0;
    m_pool = D3DPOOL_MANAGED;
    m_usage = 0;
    m_sizeInBytes = 0;

    pcInit(pTexture, uiNameHash);
}

// FUNCTION: SWBF2 0x006ba0c0
RedTexture::RedTexture()
{
    m_uiLastFrameUsed = 0;
    m_uiNameHash = 0;
    m_uiHeight = 0;
    m_uiWidth = 0;
    m_uiDepth = 0;
    m_uiNumMipMaps = 0;
    m_format = (_D3DFORMAT)0;
    m_type = PC_REDTEXTURE_TYPE_NONE;
    m_pTexture = 0;
    m_pool = D3DPOOL_MANAGED;
    m_usage = 0;
    m_sizeInBytes = 0;
}

// FUNCTION: SWBF2 0x006ba140
RedTexture::~RedTexture()
{
    if (m_uiNameHash && s_textureHashTable.Remove(m_uiNameHash))
    {
        --s_textureHashTableNumEntries;
    }

    if (m_pTexture)
    {
        m_pTexture->Release();
        m_pTexture = 0;
    }

    if (m_listNode.IsInList())
    {
        s_defaultPoolTextures.Remove(&m_listNode);
    }
}

// FUNCTION: SWBF2 0x006ba260
template <>
void PblList<RedTexture>::Clear()
{
    while (_iCount > 0)
    {
        Node* pNode = _head._pNext;
        Remove(pNode);
    }
}

// FUNCTION: SWBF2 0x006ba2a0
void RedTexture::Init()
{
    s_textureHashTable.Clear();
    s_textureHashTableNumEntries = 0;

    RedRenderer::pcRegisterResetFunction(&RedTexture::pcReset);

    s_defaultPoolTextures.Clear();
}

// FUNCTION: SWBF2 0x006ba1b0
void RedTexture::Cleanup()
{
    RedRenderer::pcUnregisterResetFunction(&RedTexture::pcReset);

    for (PblHashTable<RedTexture, 4096>::Itor it = s_textureHashTable.Begin();
         (RedTexture*)it != (RedTexture*)s_textureHashTable.End(); ++it)
    {
        delete (RedTexture*)it;
    }

    s_textureHashTable.Clear();
    s_textureHashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x006ba370
void RedTexture::Remove(unsigned int uiHash)
{
    RedTexture* pTexture = Find(uiHash);
    if (pTexture)
    {
        delete pTexture;
    }
}

// FUNCTION: SWBF2 0x006ba2d0
RedTexture* RedTexture::Read(PblFileChunk& chunk)
{
    RedTexture* pTexture = new RedTexture;

    if (!pTexture->_Read(chunk))
    {
        delete pTexture;
        return 0;
    }

    RedTexture* pOld = s_textureHashTable.Find(pTexture->m_uiNameHash);
    if (pOld)
    {
        delete pOld;
    }

    if (s_textureHashTable.Store(pTexture->m_uiNameHash, pTexture))
    {
        ++s_textureHashTableNumEntries;
    }

    return pTexture;
}

// FUNCTION: SWBF2 0x006ba3b0
bool RedTexture::_Read(PblFileChunk& chunk)
{
    _D3DFORMAT format = (_D3DFORMAT)0;
    bool bResult = true;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        switch (child._iId)
        {
        case 0x454D414Eu:
        {
            char acName[256];
            child.ReadString(acName, 256);

            const size_t uiPrefix = strlen(kLOCALIZE_TEXTURE_PREFIX);
            if (!_strnicmp(acName, kLOCALIZE_TEXTURE_PREFIX, uiPrefix))
            {
                const char* pcLanguage = RedLocalizeDB::GetLanguageStr();
                const size_t uiLanguage = strlen(pcLanguage);
                char* pcRest = acName + uiPrefix;
                if (!_strnicmp(pcRest, pcLanguage, uiLanguage))
                {
                    char acTemp[256];
                    strlcpy(acTemp, pcRest + uiLanguage + 1, 256);
                    strlcpy(acName, acTemp, 256);
                }
                else
                {
                    bResult = false;
                }
            }

            m_uiNameHash = PblHash(acName);
            break;
        }

        case 0x4F464E49u:
        {
            _D3DFORMAT aFormats[10];
            const int iNumFormats = child._pFile->Read32();
            for (int i = 0; i < iNumFormats; i++)
            {
                aFormats[i] = (_D3DFORMAT)child._pFile->Read32();
            }
            format = pcChooseFormat(aFormats, iNumFormats);
            break;
        }

        case 0x5F544D46u:
            pcReadFormat(child, format);
            break;
        }
    }

    return bResult;
}

// FUNCTION: SWBF2 0x006ba590
_D3DFORMAT RedTexture::pcChooseFormat(const _D3DFORMAT* aFormats, int iNumFormats)
{
    if (iNumFormats == 1)
    {
        D3DFORMAT format = aFormats[0];
        if (D3DXCheckTextureRequirements(RedRenderer::pcGetDevice(), 0, 0, 0, 0, &format,
                                         D3DPOOL_MANAGED) != D3D_OK
            || format != aFormats[0])
        {
            return D3DFMT_UNKNOWN;
        }
        return aFormats[0];
    }

    const TextureCapsT textureCaps = RedRenderer::pcGetRenderCaps().textureCaps;
    const TextureQualityT textureQuality = RedRenderer::pcGetRenderOptions().textureQuality;

    int iLuminosity = -1;
    int iCompressed = -1;
    int iSmall = -1;
    int iLarge = -1;

    for (int i = 0; i < iNumFormats; i++)
    {
        const int iBytes = pcGetBytesFromFormat(aFormats[i]);
        D3DFORMAT format = aFormats[i];
        if (FAILED(D3DXCheckTextureRequirements(RedRenderer::pcGetDevice(), 0, 0, 0, 0,
                                                &format, D3DPOOL_MANAGED)))
        {
            continue;
        }
        if (format != aFormats[i])
        {
            continue;
        }

        if (pcIsLuminosityFormat(aFormats[i]))
        {
            iLuminosity = i;
        }
        else if (pcIsCompressedFormat(aFormats[i]))
        {
            if (textureCaps == TEXTURE_CAPS_COMPRESSED)
            {
                iCompressed = i;
            }
        }
        else if (iBytes <= 2)
        {
            iSmall = i;
        }
        else if (iBytes <= 4)
        {
            iLarge = i;
        }
    }

    switch (textureQuality)
    {
    case TEXTURE_QUALITY_16BITS:
        if (iLuminosity >= 0)
        {
            return aFormats[iLuminosity];
        }
        if (iSmall >= 0)
        {
            return aFormats[iSmall];
        }
        if (iCompressed >= 0)
        {
            return aFormats[iCompressed];
        }
        if (iLarge >= 0)
        {
            return aFormats[iLarge];
        }
        return D3DFMT_UNKNOWN;

    case TEXTURE_QUALITY_32BITS:
        if (iLuminosity >= 0)
        {
            return aFormats[iLuminosity];
        }
        if (iLarge >= 0)
        {
            return aFormats[iLarge];
        }
        if (iSmall >= 0)
        {
            return aFormats[iSmall];
        }
        if (iCompressed >= 0)
        {
            return aFormats[iCompressed];
        }
        return D3DFMT_UNKNOWN;

    case TEXTURE_QUALITY_COMPRESSED:
        if (iLuminosity >= 0)
        {
            return aFormats[iLuminosity];
        }
        if (iCompressed >= 0)
        {
            return aFormats[iCompressed];
        }
        if (iSmall >= 0)
        {
            return aFormats[iSmall];
        }
        if (iLarge >= 0)
        {
            return aFormats[iLarge];
        }
        return D3DFMT_UNKNOWN;
    }

    return D3DFMT_UNKNOWN;
}

// FUNCTION: SWBF2 0x006ba720
int RedTexture::pcGetBytesFromFormat(_D3DFORMAT format)
{
    switch (format)
    {
    case D3DFMT_R8G8B8:
        return 3;

    case D3DFMT_A8R8G8B8:
    case D3DFMT_X8R8G8B8:
    case D3DFMT_A2B10G10R10:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_X8B8G8R8:
    case D3DFMT_G16R16:
    case D3DFMT_A2R10G10B10:
        return 4;

    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_A8R3G3B2:
    case D3DFMT_X4R4G4B4:
    case D3DFMT_A8L8:
    case D3DFMT_L16:
        return 2;

    case D3DFMT_R3G3B2:
    case D3DFMT_A8:
    case D3DFMT_L8:
    case D3DFMT_A4L4:
        return 1;

    case D3DFMT_A16B16G16R16:
        return 8;

    default:
        return 0;
    }
}

// FUNCTION: SWBF2 0x006ba7c0
void RedTexture::pcInit(unsigned int uiWidth, unsigned int uiHeight, _D3DFORMAT format,
                        unsigned int uiLevels, unsigned int uiUsage, _D3DPOOL pool,
                        unsigned int uiNameHash)
{
    IDirect3DTexture9* pTexture;

    RedRenderer::pcGetDevice()->CreateTexture(uiWidth, uiHeight, uiLevels, uiUsage,
                                             format, pool, &pTexture, 0);
    pcInit(pTexture, uiNameHash);
    pTexture->Release();
}

// FUNCTION: SWBF2 0x006ba810
void RedTexture::pcInit(IDirect3DBaseTexture9* pTexture, unsigned int uiNameHash)
{
    if (m_pTexture)
    {
        m_pTexture->Release();
    }

    m_pTexture = pTexture;
    pTexture->AddRef();

    m_uiNumMipMaps = m_pTexture->GetLevelCount();

    D3DRESOURCETYPE eResourceType = m_pTexture->GetType();

    switch (eResourceType)
    {
    case D3DRTYPE_VOLUMETEXTURE:
    {
        D3DVOLUME_DESC desc;
        m_type = PC_REDTEXTURE_TYPE_VOLUME;
        ((IDirect3DVolumeTexture9*)m_pTexture)->GetLevelDesc(0, &desc);
        m_uiWidth = desc.Width;
        m_uiHeight = desc.Height;
        m_uiDepth = desc.Depth;
        m_format = desc.Format;
        m_usage = desc.Usage;
        m_pool = desc.Pool;
        break;
    }

    case D3DRTYPE_TEXTURE:
    case D3DRTYPE_CUBETEXTURE:
    {
        D3DSURFACE_DESC desc;
        m_type = (eResourceType == D3DRTYPE_TEXTURE) ? PC_REDTEXTURE_TYPE_TEXTURE
                                                     : PC_REDTEXTURE_TYPE_CUBEMAP;
        ((IDirect3DTexture9*)m_pTexture)->GetLevelDesc(0, &desc);
        m_uiWidth = desc.Width;
        m_uiHeight = desc.Height;
        m_format = desc.Format;
        m_usage = desc.Usage;
        m_uiDepth = 1;
        m_pool = desc.Pool;
        break;
    }

    default:
        break;
    }

    m_uiNameHash = uiNameHash;
    if (uiNameHash)
    {
        if (s_textureHashTable.Find(uiNameHash))
        {
            if (s_textureHashTable.Remove(uiNameHash))
            {
                --s_textureHashTableNumEntries;
            }
        }
        if (s_textureHashTable.Store(uiNameHash, this))
        {
            ++s_textureHashTableNumEntries;
        }
    }

    if (m_pool == D3DPOOL_DEFAULT)
    {
        s_defaultPoolTextures.Add(&m_listNode, this);
    }
}

// FUNCTION: SWBF2 0x006ba980
bool RedTexture::pcIsCompressedFormat(_D3DFORMAT format)
{
    return format == D3DFMT_DXT1 || format == D3DFMT_DXT2 || format == D3DFMT_DXT3
        || format == D3DFMT_DXT4 || format == D3DFMT_DXT5;
}

// FUNCTION: SWBF2 0x006ba9c0
bool RedTexture::pcIsLuminosityFormat(_D3DFORMAT format)
{
    switch (format)
    {
    case D3DFMT_A8:
    case D3DFMT_L8:
    case D3DFMT_A8L8:
    case D3DFMT_A4L4:
    case D3DFMT_L16:
        return true;

    default:
        return false;
    }
}

// FUNCTION: SWBF2 0x006baa30
void RedTexture::pcReadFormat(PblFileChunk& chunk, _D3DFORMAT format)
{
    int iFace = -1;
    unsigned int uiSkip = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x45434146u)
        {
            ++iFace;
            while (child.HasMoreChildren())
            {
                PblFileChunk level = child.ReadNextChild();
                if (level._iId == 0x5F4C564Cu)
                {
                    pcReadLevel(level, uiSkip, iFace);
                }
            }
        }
        else if (child._iId == 0x4F464E49u)
        {
            const _D3DFORMAT fileFormat = (_D3DFORMAT)child._pFile->Read32();
            if (fileFormat != format)
            {
                return;
            }

            m_format = fileFormat;
            m_uiWidth = child._pFile->Read16();
            m_uiHeight = child._pFile->Read16();
            m_uiDepth = child._pFile->Read16();
            const int iNumMipMaps = child._pFile->Read16();
            const unsigned int uiFlags = child._pFile->Read32();
            m_type = (pcRedTextureTypeT)(uiFlags & 0xff);
            const unsigned int uiBias = (uiFlags >> 8) & 0xff;

            const QualityT textureDetail = RedRenderer::pcGetRenderOptions().textureDetail;
            if (iNumMipMaps == 1 || textureDetail == QUALITY_HIGH)
            {
                uiSkip = 0;
            }
            else
            {
                unsigned int uiSize = m_uiHeight * m_uiDepth * m_uiWidth;
                if (m_format == D3DFMT_DXT1)
                {
                    uiSize >>= 1;
                }
                else if (!pcIsCompressedFormat(m_format))
                {
                    uiSize *= pcGetBytesFromFormat(m_format);
                }

                const unsigned int uiLimit = textureDetail ? 0x80000 : 0x40000;
                for (uiSize >>= uiBias; uiSize > uiLimit; uiSize >>= 2)
                {
                    uiSkip++;
                }
            }

            if (uiSkip > (unsigned int)(iNumMipMaps - 1))
            {
                uiSkip = iNumMipMaps - 1;
            }

            m_uiWidth >>= uiSkip;
            m_uiNumMipMaps = iNumMipMaps - uiSkip;
            if (m_uiWidth < 1)
            {
                m_uiWidth = 1;
            }
            m_uiHeight >>= uiSkip;
            if (m_uiHeight < 1)
            {
                m_uiHeight = 1;
            }
            m_uiDepth >>= uiSkip;
            if (m_uiDepth < 1)
            {
                m_uiDepth = 1;
            }

            HRESULT hr;
            switch (m_type)
            {
            case PC_REDTEXTURE_TYPE_VOLUME:
            {
                IDirect3DVolumeTexture9* pVolume;
                hr = RedRenderer::pcGetDevice()->CreateVolumeTexture(
                    m_uiWidth, m_uiHeight, m_uiDepth, m_uiNumMipMaps, 0, m_format,
                    D3DPOOL_MANAGED, &pVolume, 0);
                m_pTexture = pVolume;
                if (FAILED(hr))
                {
                    return;
                }
                break;
            }

            case PC_REDTEXTURE_TYPE_CUBEMAP:
            {
                IDirect3DCubeTexture9* pCube;
                hr = RedRenderer::pcGetDevice()->CreateCubeTexture(
                    m_uiWidth, m_uiNumMipMaps, 0, m_format, D3DPOOL_MANAGED, &pCube, 0);
                m_pTexture = pCube;
                if (FAILED(hr))
                {
                    return;
                }
                break;
            }

            case PC_REDTEXTURE_TYPE_TEXTURE:
            {
                IDirect3DTexture9* pTexture;
                hr = RedRenderer::pcGetDevice()->CreateTexture(
                    m_uiWidth, m_uiHeight, m_uiNumMipMaps, 0, m_format, D3DPOOL_MANAGED,
                    &pTexture, 0);
                m_pTexture = pTexture;
                if (FAILED(hr))
                {
                    return;
                }
                break;
            }
            }
        }
    }
}

// FUNCTION: SWBF2 0x006bacb0
void RedTexture::pcReadLevel(PblFileChunk& chunk, unsigned int uiSkip, int iFace)
{
    int iSize = 0;
    unsigned int uiLevel = 0;

    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        if (child._iId == 0x4F464E49u)
        {
            const unsigned int uiFileLevel = child._pFile->Read32();
            iSize = child._pFile->Read32();
            if (uiFileLevel < uiSkip)
            {
                return;
            }
            uiLevel = uiFileLevel - uiSkip;
        }
        else if (child._iId == 0x59444F42u)
        {
            switch (m_type)
            {
            case PC_REDTEXTURE_TYPE_VOLUME:
            {
                IDirect3DVolumeTexture9* pVolume = static_cast<IDirect3DVolumeTexture9*>(m_pTexture);
                D3DLOCKED_BOX box;
                D3DVOLUME_DESC desc;
                pVolume->LockBox(uiLevel, &box, 0, 0);
                pVolume->GetLevelDesc(uiLevel, &desc);

                const bool bCompressed = pcIsCompressedFormat(desc.Format);
                const int iRowBytes = bCompressed ? 0 : pcGetBytesFromFormat(desc.Format) * desc.Width;
                const int iSliceBytes =
                    bCompressed ? 0 : pcGetBytesFromFormat(desc.Format) * desc.Height * desc.Width;

                if ((box.SlicePitch && iSliceBytes % box.SlicePitch)
                    || (box.RowPitch && iRowBytes % box.RowPitch))
                {
                    char* pSlice = (char*)box.pBits;
                    for (unsigned int z = 0; z < desc.Depth; z++)
                    {
                        m_sizeInBytes += desc.Height * iRowBytes;
                        char* pRow = pSlice;
                        for (unsigned int y = 0; y < desc.Height; y++)
                        {
                            child._pFile->ReadData(pRow, iRowBytes);
                            pRow += box.RowPitch;
                        }
                        pSlice += box.SlicePitch;
                    }
                }
                else
                {
                    m_sizeInBytes += iSize;
                    child._pFile->ReadData(box.pBits, iSize);
                }

                pVolume->UnlockBox(uiLevel);
                break;
            }

            case PC_REDTEXTURE_TYPE_CUBEMAP:
            {
                IDirect3DCubeTexture9* pCube = static_cast<IDirect3DCubeTexture9*>(m_pTexture);
                D3DLOCKED_RECT rect;
                D3DSURFACE_DESC desc;
                pCube->LockRect((D3DCUBEMAP_FACES)iFace, uiLevel, &rect, 0, 0);
                pCube->GetLevelDesc(uiLevel, &desc);

                const int iRowBytes = pcIsCompressedFormat(desc.Format)
                                          ? 0
                                          : pcGetBytesFromFormat(desc.Format) * desc.Width;

                if (rect.Pitch && iRowBytes % rect.Pitch)
                {
                    m_sizeInBytes += iRowBytes * desc.Height;
                    char* pRow = (char*)rect.pBits;
                    for (unsigned int y = 0; y < desc.Height; y++)
                    {
                        child._pFile->ReadData(pRow, iRowBytes);
                        pRow += rect.Pitch;
                    }
                }
                else
                {
                    child._pFile->ReadData(rect.pBits, iSize);
                    m_sizeInBytes += iSize;
                }

                pCube->UnlockRect((D3DCUBEMAP_FACES)iFace, uiLevel);
                break;
            }

            case PC_REDTEXTURE_TYPE_TEXTURE:
            {
                IDirect3DTexture9* pTexture = static_cast<IDirect3DTexture9*>(m_pTexture);
                D3DLOCKED_RECT rect;
                D3DSURFACE_DESC desc;
                pTexture->LockRect(uiLevel, &rect, 0, 0);
                pTexture->GetLevelDesc(uiLevel, &desc);

                const int iRowBytes = pcIsCompressedFormat(desc.Format)
                                          ? 0
                                          : pcGetBytesFromFormat(desc.Format) * desc.Width;

                if (rect.Pitch && iRowBytes % rect.Pitch)
                {
                    m_sizeInBytes += iRowBytes * desc.Height;
                    char* pRow = (char*)rect.pBits;
                    for (unsigned int y = 0; y < desc.Height; y++)
                    {
                        child._pFile->ReadData(pRow, iRowBytes);
                        pRow += rect.Pitch;
                    }
                }
                else
                {
                    child._pFile->ReadData(rect.pBits, iSize);
                    m_sizeInBytes += iSize;
                }

                pTexture->UnlockRect(uiLevel);
                break;
            }
            }
        }
    }
}

// FUNCTION: SWBF2 0x006bafd0
void RedTexture::pcReset(ResetFlagsT eFlags)
{
    if (eFlags & RESETFLAGS_FREE_RESOURCES)
    {
        for (PblList<RedTexture>::Itor it = s_defaultPoolTextures.Begin();
             (RedTexture*)it != (RedTexture*)s_defaultPoolTextures.End(); ++it)
        {
            if (it->m_pTexture)
            {
                it->m_pTexture->Release();
                it->m_pTexture = 0;
            }
        }
    }

    if (eFlags & RESETFLAGS_CREATE_RESOURCES)
    {
        for (PblList<RedTexture>::Itor it = s_defaultPoolTextures.Begin();
             (RedTexture*)it != (RedTexture*)s_defaultPoolTextures.End(); ++it)
        {
            if (it->m_type == PC_REDTEXTURE_TYPE_VOLUME)
            {
                IDirect3DVolumeTexture9* pVolume;
                RedRenderer::pcGetDevice()->CreateVolumeTexture(
                    it->m_uiWidth, it->m_uiHeight, it->m_uiDepth, it->m_uiNumMipMaps,
                    it->m_usage, it->m_format, it->m_pool, &pVolume, 0);
                it->m_pTexture = pVolume;
            }
            else if (it->m_type == PC_REDTEXTURE_TYPE_CUBEMAP)
            {
                IDirect3DCubeTexture9* pCube;
                RedRenderer::pcGetDevice()->CreateCubeTexture(
                    it->m_uiWidth, it->m_uiNumMipMaps, it->m_usage, it->m_format,
                    it->m_pool, &pCube, 0);
                it->m_pTexture = pCube;
            }
            else if (it->m_type == PC_REDTEXTURE_TYPE_TEXTURE)
            {
                IDirect3DTexture9* pTexture;
                RedRenderer::pcGetDevice()->CreateTexture(
                    it->m_uiWidth, it->m_uiHeight, it->m_uiNumMipMaps, it->m_usage,
                    it->m_format, it->m_pool, &pTexture, 0);
                it->m_pTexture = pTexture;
            }
        }
    }
}

// LIBRARY: SWBF2 0x007548f6
// D3DXCheckTextureRequirements
