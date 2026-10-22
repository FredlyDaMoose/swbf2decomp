#include "RedHotSpot.h"

#include <math.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedInterfaceScreen.h"

// GLOBAL: SWBF2 0x009982b0
// SYNTHETIC: SWBF2 0x00408270
// `dynamic initializer for 'SQRT3''
float SQRT3 = (float)sqrt(3.0);
// GLOBAL: SWBF2 0x009982b4
// SYNTHETIC: SWBF2 0x00408230
// `dynamic initializer for 'RedHotSpot::s_fCotHalfFOVx''
float RedHotSpot::s_fCotHalfFOVx = SQRT3;
// GLOBAL: SWBF2 0x009982c0
// SYNTHETIC: SWBF2 0x00408250
// `dynamic initializer for 'RedHotSpot::s_fCotHalfFOVy''
float RedHotSpot::s_fCotHalfFOVy = SQRT3 * 1.3333333f;

// FUNCTION: SWBF2 0x006d64e0
RedHotSpot::RedHotSpot()
{
    m_fLeft = 0.0f;
    m_fRight = 0.0f;
    m_fTop = 0.0f;
    m_fBottom = 0.0f;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            m_mWorld[i][j] = 0.0f;
        }
    }
}

// FUNCTION: SWBF2 0x006d6520
RedHotSpot::~RedHotSpot()
{
}

// FUNCTION: SWBF2 0x006d6760
bool RedHotSpot::IsPointInside(float fX, float fY) const
{
    float fLocalX, fLocalY;
    return GetLocalCoordinates(fX, fY, fLocalX, fLocalY);
}

// FUNCTION: SWBF2 0x006d6830
void RedHotSpot::SetRect(float fLeft, float fTop, float fRight, float fBottom)
{
    m_fLeft = fLeft;
    m_fTop = fTop;
    m_fRight = fRight;
    m_fBottom = fBottom;
}

// FUNCTION: SWBF2 0x006d6860
void RedHotSpot::UpdateCamera()
{
    float fNear, fFar, fWidth, fHeight;
    RedRenderer::GetCamera()->GetFrustum(fNear, fFar, fWidth, fHeight);
    const float fTwoNear = fNear * 2.0f;
    s_fCotHalfFOVx = fTwoNear / fWidth;
    s_fCotHalfFOVy = fTwoNear / fHeight;
}

// FUNCTION: SWBF2 0x006d67a0
void RedHotSpot::SetContext(const PblMatrix& mContext)
{
    m_mWorld[0][0] = mContext.m[0][0] * s_fCotHalfFOVx;
    m_mWorld[1][0] = mContext.m[1][0] * s_fCotHalfFOVx;
    m_mWorld[2][0] = mContext.m[3][0] * s_fCotHalfFOVx;

    m_mWorld[0][1] = mContext.m[0][1] * s_fCotHalfFOVy;
    m_mWorld[1][1] = mContext.m[1][1] * s_fCotHalfFOVy;
    m_mWorld[2][1] = mContext.m[3][1] * s_fCotHalfFOVy;

    m_mWorld[0][2] = mContext.m[0][2];
    m_mWorld[1][2] = mContext.m[1][2];
    m_mWorld[2][2] = mContext.m[3][2];
}

// FUNCTION: SWBF2 0x006d6530
bool RedHotSpot::GetLocalCoordinates(float fX, float fY, float& rfLocalX, float& rfLocalY) const
{
    const float fNX = fX * 2.0f / (float)RedInterfaceScreen::GetScreenWidth() - 1.0f;
    const float fNY = 1.0f - fY * 2.0f / (float)RedInterfaceScreen::GetScreenHeight();

    const float fAx = m_mWorld[0][2] * fNX + m_mWorld[0][0];
    const float fAy = m_mWorld[0][2] * fNY + m_mWorld[0][1];
    const float fBx = m_mWorld[1][2] * fNX + m_mWorld[1][0];
    const float fBy = m_mWorld[1][2] * fNY + m_mWorld[1][1];
    const float fDet = fBy * fAx - fBx * fAy;
    if ((float)fabs((double)fDet) < 0.0001f)
    {
        rfLocalY = FLT_MAX;
        rfLocalX = FLT_MAX;
        return false;
    }

    const float fInvDet = 1.0f / fDet;
    const float fCx = m_mWorld[2][2] * fNX + m_mWorld[2][0];
    const float fCy = m_mWorld[2][2] * fNY + m_mWorld[2][1];
    rfLocalX = (fCy * fBx - fCx * fBy) * fInvDet;
    rfLocalY = (fCx * fAy - fCy * fAx) * fInvDet;
    return rfLocalX >= m_fLeft && m_fRight >= rfLocalX && rfLocalY >= m_fTop && m_fBottom >= rfLocalY;
}

// FUNCTION: SWBF2 0x006d66c0
bool RedHotSpot::GetLocalScaledCoordinates(float fX, float fY, float& rfLocalX, float& rfLocalY) const
{
    float fLocalX, fLocalY;
    const bool bInside = GetLocalCoordinates(fX, fY, fLocalX, fLocalY);
    if (fLocalX == FLT_MAX)
    {
        rfLocalY = FLT_MAX;
        rfLocalX = FLT_MAX;
        return false;
    }
    rfLocalX = (fLocalX - m_fLeft) / (m_fRight - m_fLeft);
    rfLocalY = (fLocalY - m_fTop) / (m_fBottom - m_fTop);
    return bInside;
}
