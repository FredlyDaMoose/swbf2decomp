#pragma once

class PblMemChunk;
class ZephyrAnimBank;

class ZephyrAnimBankReader
{
public:

    ZephyrAnimBankReader() : m_pBank(0) {}
    ZephyrAnimBank* GetBank() { return m_pBank; }

    bool ReadFile(PblMemChunk& kChunk);

    ZephyrAnimBank* m_pBank;
};
 
