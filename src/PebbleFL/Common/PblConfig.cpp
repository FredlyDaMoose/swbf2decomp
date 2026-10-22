#include "PblConfig.h"

#include "PblFile.h"

// FUNCTION: SWBF2 0x00727d60
PblConfig::PblConfig(unsigned int uiNameHash, PblFileChunk& chunk)
    : m_Chunk(chunk), m_pChunk(&chunk), m_CurrentChild(chunk), m_uiNameHash(uiNameHash)
{
}

// FUNCTION: SWBF2 0x00727da0
PblConfig::PblConfig(PblFileChunk& chunk)
    : m_Chunk(chunk), m_pChunk(&chunk), m_CurrentChild(chunk.ReadNextChild())
{
    m_uiNameHash = m_CurrentChild._pFile->Read32();
}

// FUNCTION: SWBF2 0x00727de0
PblConfig::PblConfig(const PblConfig& config, bool bShareChunk)
    : m_Chunk(*config.m_pChunk),
      m_pChunk(bShareChunk ? config.m_pChunk : &m_Chunk),
      m_CurrentChild(config.m_CurrentChild),
      m_uiNameHash(config.m_uiNameHash)
{
}

// FUNCTION: SWBF2 0x00727e30
PblConfig::Data PblConfig::ReadNextData()
{
    m_CurrentChild = m_pChunk->ReadNextChild();

    Data data;
    data.m_uiId = m_CurrentChild._pFile->Read32();
    data.m_uiNumArgs = m_CurrentChild._pFile->Read8();
    m_CurrentChild._pFile->ReadData(data.m_auiArgs, data.m_uiNumArgs * 4);
    m_CurrentChild._pFile->ReadData(&data.m_auiArgs[data.m_uiNumArgs],
                                    m_CurrentChild._pFile->Read32());
    return data;
}

// FUNCTION: SWBF2 0x00727eb0
PblConfig PblConfig::ReadNextScope()
{
    m_CurrentChild = m_pChunk->ReadNextChild();
    return PblConfig(PblConfig(m_uiNameHash, m_CurrentChild), true);
}
