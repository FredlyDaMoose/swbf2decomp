#pragma once

#include <stddef.h>

#include "RedInterfaceElement.h"

class pcInterfaceShader;

// VTABLE: SWBF2 0x007728f4

class RedVectorElement : public RedInterfaceElement
{
public:

    enum VectorStyleT
    {
        VECTOR_STYLE_NORMAL = 0,
        VECTOR_STYLE_SHADOW = 1
    };

    struct Point
    {

        Point();

        float x;
        float y;
        float z;
        RedColor color;
    };

    // FUNCTION: SWBF2 0x006bfbb0
    RedVectorElement();

    // SYNTHETIC: SWBF2 0x006bfa00
    // RedVectorElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006bf9a0
    virtual ~RedVectorElement();

    // FUNCTION: SWBF2 0x006bfb30
    virtual void SetBlendMode(BlendModeT eBlendMode);

    // FUNCTION: SWBF2 0x006bfaf0
    virtual BlendModeT GetBlendMode() const;

    // FUNCTION: SWBF2 0x006bfc40
    virtual void Cleanup();

    // FUNCTION: SWBF2 0x006bfc90
    void Clear();

protected:
    // FUNCTION: SWBF2 0x006bfd80
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:

    // FUNCTION: SWBF2 0x006bfbf0
    void Begin();
    // FUNCTION: SWBF2 0x006bfcd0
    void End();
    // FUNCTION: SWBF2 0x006bfd30
    void Init();
    // FUNCTION: SWBF2 0x006bfa30
    void DrawLine(float fX, float fY);
    // FUNCTION: SWBF2 0x006bfa80
    void DrawLine(float fX, float fY, RedColor color);
    // FUNCTION: SWBF2 0x006bfb00
    void MovePen(float fX, float fY);

private:
    // FUNCTION: SWBF2 0x006bfe60
    static Point* _AddPoint(const Point& point, bool bConnected);
    // FUNCTION: SWBF2 0x006bfb40
    static Point* _AddPoint();
    // FUNCTION: SWBF2 0x006bfb60
    static void _Begin();
    // FUNCTION: SWBF2 0x006bfb80
    static void _End();
    // FUNCTION: SWBF2 0x006bfb90
    static unsigned int _GetNumPoints();
    // FUNCTION: SWBF2 0x006bfba0
    static const Point* _GetPoints();

public:

    static unsigned int s_uiNumPoints;
    static bool s_bPointsInUse;
    static Point s_aPoints[1024];

    BlendModeT m_eBlendMode;
    VectorStyleT m_eStyle;
    Point m_curPoint;
    unsigned int m_uiNumVertices;
    Point* m_pVertices;
    pcInterfaceShader* m_pInterfaceShader;
};

 
 
 
 
 
 
 
 
