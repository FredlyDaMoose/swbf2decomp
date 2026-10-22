#pragma once

class PblMemChunk;
class ZephyrSkinShared;

class ZephyrSkinReader
{
public:

    ZephyrSkinReader() : m_pSkin(0) {}
    ZephyrSkinShared* GetSkin() { return m_pSkin; }

    bool ReadFile(PblMemChunk& kChunk);

    ZephyrSkinShared* m_pSkin;
};
 
