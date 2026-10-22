#include "EnergyBar.h"

#include <stdio.h>

// GLOBAL: SWBF2 0x01eb9810
// SYNTHETIC: SWBF2 0x00402810
// `dynamic initializer for 'EnergyBar::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766ef0
// `dynamic atexit destructor for 'EnergyBar::sMemoryPool''
MemoryPool EnergyBar::sMemoryPool("EnergyBar", 0xc);

// FUNCTION: SWBF2 0x00491470
void EnergyBarClass::Init(float fEnergyMax, float fEnergyOverheat)
{
    if (!(fEnergyMax > 0.0f))
    {
        fEnergyMax = 0.0f;
    }

    m_fEnergyMax = fEnergyMax;

    if (0.0f > fEnergyOverheat)
    {
        m_fEnergyOverheat = 0.0f;
    }
    else if (fEnergyOverheat > fEnergyMax)
    {
        m_fEnergyOverheat = fEnergyMax;
    }
    else
    {
        m_fEnergyOverheat = fEnergyOverheat;
    }
}

// FUNCTION: SWBF2 0x004914a0
bool EnergyBarClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x97105D6A:
        sscanf(pszValue, "%f", &m_fEnergyMax);
        if (0.0f > m_fEnergyMax)
        {
            m_fEnergyMax = 0.0f;
        }
        else if (m_fEnergyOverheat > m_fEnergyMax)
        {
            m_fEnergyOverheat = m_fEnergyMax;
        }
        return true;

    case 0x5621511E:
        sscanf(pszValue, "%f", &m_fEnergyOverheat);
        if (m_fEnergyOverheat >= 1.0f)
        {
            m_fEnergyOverheat = 0.0f;
        }
        else if (m_fEnergyOverheat > 0.0f)
        {
            m_fEnergyOverheat = 1.0f - m_fEnergyOverheat;
        }
        else
        {
            m_fEnergyOverheat = 1.0f;
        }
        return true;

    case 0xA733FBF5:
    case 0xF03FFABB:
        sscanf(pszValue, "%f", &m_fEnergyOverheat);
        if (0.0f > m_fEnergyOverheat)
        {
            m_fEnergyOverheat = 0.0f;
        }
        else if (m_fEnergyOverheat > m_fEnergyMax)
        {
            m_fEnergyMax = m_fEnergyOverheat;
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x004915e0
EnergyBar* EnergyBar::AddRef()
{
    if (!m_uiRefCount)
    {
        return 0;
    }
    ++m_uiRefCount;
    return this;
}

// FUNCTION: SWBF2 0x004915b0
void EnergyBarClass::SetPropertyEnergyCost(float fCost)
{
    if (fCost > -m_fEnergyMin)
    {
        m_fEnergyMin = -fCost;
    }
}

// FUNCTION: SWBF2 0x00491690
void EnergyBar::AddEnergy(float fEnergy)
{
    m_fEnergy += fEnergy;
    if (m_fEnergy >= m_pClass->m_fEnergyOverheat)
    {
        m_bOverheat = 0;
        if (m_fEnergy > m_pClass->m_fEnergyMax)
        {
            m_fEnergy = m_pClass->m_fEnergyMax;
        }
    }
}

// FUNCTION: SWBF2 0x00491650
void EnergyBar::SetEnergy(float fEnergy)
{
    m_fEnergy = fEnergy;
    if (0.0f > fEnergy)
    {
        m_bOverheat = 1;
        if (m_pClass->m_fEnergyMin > fEnergy)
        {
            m_fEnergy = m_pClass->m_fEnergyMin;
        }
    }
    else if (fEnergy >= m_pClass->m_fEnergyOverheat)
    {
        m_bOverheat = 0;
        if (fEnergy > m_pClass->m_fEnergyMax)
        {
            m_fEnergy = m_pClass->m_fEnergyMax;
        }
    }
}

// FUNCTION: SWBF2 0x004915d0
EnergyBar::~EnergyBar()
{
    m_uiRefCount = 0;
}

// FUNCTION: SWBF2 0x004916c0
bool EnergyBar::SpendEnergy(float fEnergy)
{
    if (fEnergy > 0.0f)
    {
        if (m_bOverheat)
        {
            return false;
        }
        m_fEnergy -= fEnergy;
        if (0.0f > m_fEnergy)
        {
            m_bOverheat = 1;
            if (m_pClass->m_fEnergyMin > m_fEnergy)
            {
                m_fEnergy = m_pClass->m_fEnergyMin;
            }
        }
    }
    return true;
}

// FUNCTION: SWBF2 0x00491610
unsigned int EnergyBar::Release()
{
    if (!m_uiRefCount)
    {
        return 0;
    }
    if (m_uiRefCount == 1)
    {
        this->~EnergyBar();
        sMemoryPool.Free(this);
        return 0;
    }
    return --m_uiRefCount;
}
