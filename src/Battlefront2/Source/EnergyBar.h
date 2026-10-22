#pragma once

#include <stddef.h>
#include "RedEngineFL/Memory/RedMemoryPool.h"

class EnergyBarClass
{
public:
    void Init(float fEnergyMax, float fEnergyOverheat);
    bool SetProperty(unsigned int uProperty, const char* pszValue);

    void SetPropertyEnergyCost(float fCost);

    float m_fEnergyMax;
    float m_fEnergyMin;
    float m_fEnergyOverheat;
};
 
 
 
 

class NetPktGroup;

class EnergyBar
{
public:

    EnergyBar(const EnergyBarClass* pClass)
        : m_pClass(pClass), m_fEnergy(pClass->m_fEnergyMax),
          m_bOverheat(0), m_uiRefCount(1) {}
    ~EnergyBar();

    EnergyBar* AddRef();
    unsigned int Release();
    void AddEnergy(float fEnergy);
    bool SpendEnergy(float fEnergy);

    void SetEnergy(float fEnergy);
    void Write(NetPktGroup& group) const;
    void Read(NetPktGroup& group);

    float GetEnergy() const { return m_fEnergy; }
    float GetEnergyMax() const { return m_pClass->m_fEnergyMax; }
    bool IsOverheating() const { return m_bOverheat != 0; }
    static void SetupMemoryPool(unsigned int count, unsigned int size,
                                unsigned int grow) { sMemoryPool.Setup(count, size, grow); }
    static MemoryPool sMemoryPool;

    const EnergyBarClass* m_pClass;
    float m_fEnergy;
    unsigned int m_bOverheat : 1;
    unsigned int m_uiRefCount : 31;
};
 
 
 
