#pragma once

class PblHash
{
public:
    PblHash(const char* pcText);
    PblHash(unsigned int uiSeed, const char* pcText);

    // FUNCTION: SWBF2 0x00411c40
    operator unsigned int() const { return _uiValue; }

    unsigned int _uiValue;

private:
    static unsigned int __cdecl _MakeHash(const char* pcText);
    static unsigned int __cdecl _MakeHash(const char* pcText, unsigned int uiSeed);
};
 

class PblHashTableCode
{
protected:
    static void __cdecl _Clear(unsigned int* pTable, int iCount);
    static void* __cdecl _Find(const unsigned int* pTable, int iCount, unsigned int uiHash);
    static bool __cdecl _Store(unsigned int* pTable, int iCount, unsigned int uiHash, void* pValue);
    static bool __cdecl _Remove(unsigned int* pTable, int iCount, unsigned int uiHash);
};
 

template <class TYPE, int SLOTS>
class PblHashTable : public PblHashTableCode
{
public:

    void Clear() { _Clear(_table, SLOTS); }
    TYPE* Find(unsigned int uiHash) const { return (TYPE*)_Find(_table, 2 * SLOTS, uiHash); }

    bool Store(unsigned int uiHash, TYPE* pValue)
    {
        return _Store(_table, 2 * SLOTS, uiHash, (void*)pValue);
    }
    bool Remove(unsigned int uiHash) { return _Remove(_table, 2 * SLOTS, uiHash); }

    class Itor
    {
    public:
        Itor() : _table(0), _pos(0) {}
        Itor(const PblHashTable* pTable) : _table(const_cast<PblHashTable*>(pTable)), _pos(0)
        {
            while (_pos < SLOTS && !_table->_table[_pos])
            {
                ++_pos;
            }
        }

        operator TYPE*() const { return (TYPE*)_table->_table[SLOTS + _pos]; }
        TYPE& operator*() const { return *(TYPE*)_table->_table[SLOTS + _pos]; }

        unsigned int GetKey() const { return _table->_table[_pos]; }

        const Itor& operator++()
        {
            ++_pos;
            while (_pos < SLOTS && !_table->_table[_pos])
            {
                ++_pos;
            }
            return *this;
        }

    private:
        friend class PblHashTable;
        Itor(const PblHashTable* pTable, unsigned int uiPos)
            : _table(const_cast<PblHashTable*>(pTable)), _pos(uiPos) {}

        PblHashTable* _table;
        unsigned int _pos;
    };

    Itor Begin() const { return Itor(this); }
    Itor End() const { return Itor(this, SLOTS); }

    unsigned int _table[2 * SLOTS];
};

unsigned int __cdecl PblTEMPHash(const char* pcText);
unsigned int __cdecl CalcCRC(const void* pData, unsigned int uiSize, unsigned int uiCRC);
