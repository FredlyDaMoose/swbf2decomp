#include "RedVectorElement.h"

// GLOBAL: SWBF2 0x008fa470
unsigned int RedVectorElement::s_uiNumPoints;

// GLOBAL: SWBF2 0x008fa474
bool RedVectorElement::s_bPointsInUse;

// GLOBAL: SWBF2 0x008fa478
// SYNTHETIC: SWBF2 0x00407970
// `dynamic initializer for 'RedVectorElement::s_aPoints''
RedVectorElement::Point RedVectorElement::s_aPoints[1024];

// FUNCTION: SWBF2 0x006bfa30
void RedVectorElement::DrawLine(float fX, float fY)
{
    DrawLine(fX, fY, m_curPoint.color);
}

// FUNCTION: SWBF2 0x006bfa80
void RedVectorElement::DrawLine(float fX, float fY, RedColor color)
{
    if (!m_curPoint.color.a && !color.a)
    {
        return;
    }

    if (m_curPoint.x != fX || m_curPoint.y != fY)
    {
        m_curPoint.x = fX;
        m_curPoint.y = fY;
        m_curPoint.color = color;
        _AddPoint(m_curPoint, true);
    }
}

// FUNCTION: SWBF2 0x006bfb00
void RedVectorElement::MovePen(float fX, float fY)
{
    m_curPoint.x = fX;
    m_curPoint.y = fY;
    _AddPoint(m_curPoint, false);
}

// FUNCTION: SWBF2 0x006bfb40
RedVectorElement::Point* RedVectorElement::_AddPoint()
{
    return &s_aPoints[s_uiNumPoints++];
}

// FUNCTION: SWBF2 0x006bfb60
void RedVectorElement::_Begin()
{
    s_bPointsInUse = true;
    s_uiNumPoints = 0;
}

// FUNCTION: SWBF2 0x006bfb80
void RedVectorElement::_End()
{
    s_bPointsInUse = false;
}

// FUNCTION: SWBF2 0x006bfb90
unsigned int RedVectorElement::_GetNumPoints()
{
    return s_uiNumPoints;
}

// FUNCTION: SWBF2 0x006bfba0
const RedVectorElement::Point* RedVectorElement::_GetPoints()
{
    return s_aPoints;
}
