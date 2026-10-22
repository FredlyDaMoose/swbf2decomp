#pragma once

#include <stddef.h>

class PblMatrix;

class RedHotSpot
{
public:

    // FUNCTION: SWBF2 0x006d64e0
    RedHotSpot();

    // FUNCTION: SWBF2 0x006d6520
    ~RedHotSpot();

    // FUNCTION: SWBF2 0x006d6830
    void SetRect(float fLeft, float fTop, float fRight, float fBottom);

protected:

    // FUNCTION: SWBF2 0x006d67a0
    void SetContext(const PblMatrix& mContext);

public:

    bool IsPointInside(float fX, float fY) const;

    bool GetLocalCoordinates(float fX, float fY, float& rfLocalX, float& rfLocalY) const;
    bool GetLocalScaledCoordinates(float fX, float fY, float& rfLocalX, float& rfLocalY) const;
protected:

    friend class RedInterfaceElement;

public:

    float m_fLeft;
    float m_fRight;
    float m_fTop;
    float m_fBottom;
    float m_mWorld[3][3];

    static float s_fCotHalfFOVx;
    static float s_fCotHalfFOVy;

    static void UpdateCamera();
};

 
 
 
 
 
 
