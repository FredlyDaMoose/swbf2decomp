#pragma once

#include <stddef.h>

#include "RedEngineFL/Application/Renderable.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "RedEngineFL/Interface/RedTextElement.h"
#include "PebbleFL/Common/PblVector.h"

class PblMatrix;
class RedSphereShape;
class RedTetrahedronShape;

class RedDebug
{
public:

    // VTABLE: SWBF2 0x00783ed8
    class RedDebugRenderable : public Renderable
    {
        friend class RedDebug;

        // FUNCTION: SWBF2 0x006f0790
        RedDebugRenderable();

    public:

        virtual ~RedDebugRenderable() {}

        // SYNTHETIC: SWBF2 0x006f08f0
        // RedDebug::RedDebugRenderable::`scalar deleting destructor'

        virtual void Render(int value);

        // FUNCTION: SWBF2 0x006f2510
        void Init();

        // FUNCTION: SWBF2 0x006f09b0
        void Cleanup();
    };

    struct Text3D
    {
        // FUNCTION: SWBF2 0x006f08b0
        Text3D();
        // FUNCTION: SWBF2 0x006f08e0
        ~Text3D();

        RedTextElement text;
        PblVector3 pos;
    };

    static void Init()
    {
        _kStaticRenderable.Init();
        _kStaticRenderable.Activate(0);
    }

    static void Cleanup() { _kStaticRenderable.Cleanup(); }

    // FUNCTION: SWBF2 0x006f2bd0
    static void SetPrintfData(const RedColor& color, RedGlyphCacheElement::TextStyleT eStyle);

    // FUNCTION: SWBF2 0x006f2670
    static void Printf3D(const PblVector3& vPos, const char* pcFormat, ...);

    // FUNCTION: SWBF2 0x006f16f0
    static void DrawSphere(PblVector3 vCenter, float fRadius, RedColor color);

    // FUNCTION: SWBF2 0x006f1300
    static void DrawOBB(const PblBox& box, const PblMatrix& matrix, RedColor color);
    // FUNCTION: SWBF2 0x006f0ee0
    static void DrawCone(const PblMatrix& matrix, float fHeight, float fRadius0, float fRadius1, RedColor color);

    // FUNCTION: SWBF2 0x006f0a70
    static void DrawAABB(const PblBox& box, RedColor color);
    // FUNCTION: SWBF2 0x006f12a0
    static void DrawLine3D(PblVector3 v0, PblVector3 v1, RedColor color);

    struct Radius;
    struct SolidOBB;

private:

    static RedDebugRenderable _kStaticRenderable;
    static RedColor _PrintfColor;
    static RedGlyphCacheElement::TextStyleT _ePrintfStyle;
    static RedScreenGroupElement _ScreenGroup;
    static RedTextElement _aTextLines[30];
    static Text3D _aText3D[64];
    static char _aLineBuffer[256];
    static char* _pCurrLinePos;
    static unsigned int _uiNumTextLines;
    static unsigned int _uiNumText3D;

    static unsigned int _n3DLines;
    static RedColor _aColor[3000];
    static PblVector3 _a3DLineVectors[6000];

    static SolidOBB _aOBB[32];
    static Radius _aRadii[32];
    static RedSphereShape _aSpheres[16];
    static RedTetrahedronShape _aMarkers3D[16];

    static unsigned int _nOBB;
    static unsigned int _nRadii;
    static unsigned int _nSpheres;
    static unsigned int _nMarkers3D;

    static void _DrawPrintf();
    static void _DrawPrintf3D();
    static void _DrawRadius(const Radius& radius);
    static void _DrawSolidOBB(const SolidOBB& obb);

    static void _DrawLine3D(PblVector3 v0, PblVector3 v1, RedColor color);
};
 
 
 
 
 