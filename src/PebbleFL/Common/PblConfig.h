#pragma once

#include "PblChunk.h"
#include "PblHashTable.h"

class PblConfig
{
public:

    PblConfig(PblFileChunk& chunk);

    unsigned int GetNameHash() const { return m_uiNameHash; }

    class Data
    {
    public:

        unsigned int GetId() const { return m_uiId; }

        unsigned int GetNumArgs() const { return m_uiNumArgs; }

        unsigned int GetUint32Arg(unsigned int i) const
        {
            return (unsigned int)*(const float*)&m_auiArgs[i];
        }

        int GetInt32Arg(unsigned int i) const
        {
            return (int)*(const float*)&m_auiArgs[i];
        }

        float GetFloatArg(unsigned int i) const
        {
            return *(const float*)&m_auiArgs[i];
        }

        const char* GetStringArg(unsigned int i) const
        {
            return (const char*)m_auiArgs + m_auiArgs[i];
        }

        // FUNCTION: SWBF2 0x004708e0
        bool GetBoolArg(unsigned int i) const { return GetFloatArg(i) != 0.0f; }

        // FUNCTION: SWBF2 0x0042cee0
        unsigned char GetUint8Arg(unsigned int i) const
        {
            float f = GetFloatArg(i);
            if (f < 0.0f)
            {
                f = 0.0f;
            }
            else if (f > 255.0f)
            {
                f = 255.0f;
            }
            return (unsigned char)(int)f;
        }

        bool IsStringArg(unsigned int i) const;

        bool IsFloatArg(unsigned int i) const { return i < m_uiNumArgs; }

        unsigned int m_uiId;
        unsigned int m_uiNumArgs;
        unsigned int m_auiArgs[128];
    };

    bool IsDone() const { return !m_pChunk->HasMoreChildren(); }

    bool IsScopeNext() const
    {
        return m_pChunk->HasMoreChildren() && m_pChunk->PeekNextChildId() == 0x504f4353;
    }

    Data ReadNextData();

    PblConfig(const PblConfig& config, bool bShareChunk);

    PblConfig ReadNextScope();

private:

    PblConfig(unsigned int uiNameHash, PblFileChunk& chunk);

public:

    PblFileChunk m_Chunk;
    PblFileChunk* m_pChunk;
    PblFileChunk m_CurrentChild;
    unsigned int m_uiNameHash;
};
 

class PblConfigSR
{
public:
    PblConfigSR(PblConfig::Data& data, const char* pcName)
        : data_(data), hash_(PblHash(pcName)) {}

    bool GetNode(const PblConfig::Data& data) { return data.GetId() == hash_; }

    PblConfig::Data& data_;
    unsigned int hash_;
};
