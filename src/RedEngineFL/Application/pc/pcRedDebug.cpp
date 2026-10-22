#include <stdarg.h>
#include "PebbleFL/Common/PblVector.h"

#include "pcRedDebug.h"
#include "pcRedShape.h"

#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Application/pc/pcRenderHelpers.h"
#include "RedEngineFL/Application/pc/pcRenderStateHelpers.h"
#include "RedEngineFL/Graphics/PC/pcRedPrimitive.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);

void RemoveInterfaceElement(RedScreenGroupElement* pElement);
}

void __cdecl PblTraceP(const char* Format, ...);

extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV,
                                                         const PblMatrix* pM);

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// GLOBAL: SWBF2 0x009ae64c
// SYNTHETIC: SWBF2 0x00408f10
// `dynamic initializer for 'RedDebug::_PrintfColor''
RedColor RedDebug::_PrintfColor = RedColor::WHITE;

// GLOBAL: SWBF2 0x009af450
// SYNTHETIC: SWBF2 0x00408f40
// `dynamic initializer for 'RedDebug::_ScreenGroup''
// SYNTHETIC: SWBF2 0x00769b30
// `dynamic atexit destructor for 'RedDebug::_ScreenGroup''
RedScreenGroupElement RedDebug::_ScreenGroup("_DebugGroup");

struct RedDebug::Radius
{
    Radius();

    PblVector3 _vPos;
    RedColor _color;
    float _fRadius;
    float _fHeight;
};
 

struct RedDebug::SolidOBB
{
    SolidOBB();

    PblBox _vBox;
    PblMatrix _mBox;
    RedColor _color;
};
 
 

// FUNCTION: SWBF2 0x006f0780
RedDebug::Radius::Radius()
{
}

// FUNCTION: SWBF2 0x006f08a0
RedDebug::SolidOBB::SolidOBB()
{
}

// GLOBAL: SWBF2 0x009c7c10
// SYNTHETIC: SWBF2 0x00408f80
// `dynamic initializer for 'RedDebug::_aMarkers3D''
// SYNTHETIC: SWBF2 0x00769b40
// `dynamic atexit destructor for 'RedDebug::_aMarkers3D''
RedTetrahedronShape RedDebug::_aMarkers3D[16];

// GLOBAL: SWBF2 0x009ae650
// SYNTHETIC: SWBF2 0x00408fb0
// `dynamic initializer for 'RedDebug::_aOBB''
RedDebug::SolidOBB RedDebug::_aOBB[32];

// GLOBAL: SWBF2 0x009b57d0
// SYNTHETIC: SWBF2 0x00408fd0
// `dynamic initializer for 'RedDebug::_aRadii''
RedDebug::Radius RedDebug::_aRadii[32];

// GLOBAL: SWBF2 0x009c7410
// SYNTHETIC: SWBF2 0x00408ff0
// `dynamic initializer for 'RedDebug::_aSpheres''
// SYNTHETIC: SWBF2 0x00769b60
// `dynamic atexit destructor for 'RedDebug::_aSpheres''
RedSphereShape RedDebug::_aSpheres[16];

// GLOBAL: SWBF2 0x009b23d0
// SYNTHETIC: SWBF2 0x00409020
// `dynamic initializer for 'RedDebug::_aText3D''
// SYNTHETIC: SWBF2 0x00769b80
// `dynamic atexit destructor for 'RedDebug::_aText3D''
RedDebug::Text3D RedDebug::_aText3D[64];

// GLOBAL: SWBF2 0x009acfb0
// SYNTHETIC: SWBF2 0x00409050
// `dynamic initializer for 'RedDebug::_aTextLines''
// SYNTHETIC: SWBF2 0x00769ba0
// `dynamic atexit destructor for 'RedDebug::_aTextLines''
RedTextElement RedDebug::_aTextLines[30];

// GLOBAL: SWBF2 0x009ae630
// SYNTHETIC: SWBF2 0x00409080
// `dynamic initializer for 'RedDebug::_kStaticRenderable''
// SYNTHETIC: SWBF2 0x00769bc0
// `dynamic atexit destructor for 'RedDebug::_kStaticRenderable''
RedDebug::RedDebugRenderable RedDebug::_kStaticRenderable;

// GLOBAL: SWBF2 0x009ace98
char RedDebug::_aLineBuffer[256];

// GLOBAL: SWBF2 0x009acf98
char* RedDebug::_pCurrLinePos;

// GLOBAL: SWBF2 0x009acf9c
unsigned int RedDebug::_uiNumTextLines;

// GLOBAL: SWBF2 0x009acfa0
unsigned int RedDebug::_uiNumText3D;

// GLOBAL: SWBF2 0x009acfa4
RedGlyphCacheElement::TextStyleT RedDebug::_ePrintfStyle;

// GLOBAL: SWBF2 0x009c8410
pcRedShader* s_debugShader;

// GLOBAL: SWBF2 0x009ace84
unsigned int RedDebug::_n3DLines;
// GLOBAL: SWBF2 0x009ace88
unsigned int RedDebug::_nSpheres;
// GLOBAL: SWBF2 0x009ace8c
unsigned int RedDebug::_nMarkers3D;
// GLOBAL: SWBF2 0x009ace90
unsigned int RedDebug::_nRadii;
// GLOBAL: SWBF2 0x009ace94
unsigned int RedDebug::_nOBB;

// GLOBAL: SWBF2 0x009af4f0
RedColor RedDebug::_aColor[3000];

// GLOBAL: SWBF2 0x009b5ad0
// SYNTHETIC: SWBF2 0x00408f60
// `dynamic initializer for 'RedDebug::_a3DLineVectors''

PblVector3 RedDebug::_a3DLineVectors[6000];

// FUNCTION: SWBF2 0x006f0790
RedDebug::RedDebugRenderable::RedDebugRenderable()
    : Renderable(7)
{
}

// FUNCTION: SWBF2 0x006f08b0
RedDebug::Text3D::Text3D()
{
}

// FUNCTION: SWBF2 0x006f08e0
RedDebug::Text3D::~Text3D()
{
}

// FUNCTION: SWBF2 0x006f07b0
RedShape::RedShape()
    : Renderable(6)
{
    _Matrix = _IdentityMatrix;
    _pMatrix = &_Matrix;
    _vMin = PblVector3(-1.0f, -1.0f, -1.0f);
    _vMax = PblVector3(1.0f, 1.0f, 1.0f);
    SetColor(RedColor::WHITE);
}

// FUNCTION: SWBF2 0x006f0860
RedSphereShape::RedSphereShape()
{
}

// FUNCTION: SWBF2 0x006f0880
RedTetrahedronShape::RedTetrahedronShape()
{
}

// FUNCTION: SWBF2 0x006f09b0
void RedDebug::RedDebugRenderable::Cleanup()
{
    FLRenderer::RemoveInterfaceElement(&_ScreenGroup);

    for (int i = 0; i < 30; i++)
    {
        RedTextElement& line = _aTextLines[i];
        line.SetFont("");
        line.Cleanup();
        _ScreenGroup.RemoveElement(&line);
    }

    for (int i = 0; i < 64; i++)
    {
        RedTextElement& text = _aText3D[i].text;
        text.SetFont("");
        text.Cleanup();
        _ScreenGroup.RemoveElement(&text);
    }

    if (s_debugShader)
    {
        s_debugShader->Release();
        s_debugShader = 0;
    }
}

// FUNCTION: SWBF2 0x006f0a70
void RedDebug::DrawAABB(const PblBox& box, RedColor color)
{
    const PblVector3& vMin = box._vMin;
    const PblVector3& vMax = box._vMax;
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMin.y, vMin.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMin.x, vMax.y, vMax.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMin.x, vMin.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMin.y, vMax.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMin.x, vMax.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMax.x, vMin.y, vMin.z), color);
    DrawLine3D(PblVector3(vMax.x, vMax.y, vMin.z), PblVector3(vMax.x, vMax.y, vMax.z), color);
}

// FUNCTION: SWBF2 0x006f0ee0
void RedDebug::DrawCone(const PblMatrix& matrix, float fHeight, float fRadius0, float fRadius1,
                        RedColor color)
{
    const float fStep = PblMath::fTwoPi * 0.125f;
    const PblVector3 vBase(matrix.m[3][0], matrix.m[3][1], matrix.m[3][2]);

    PblVector3 aCircle[8];
    for (int i = 0; i < 8; i++)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * fStep);
        aCircle[i] = PblVector3(matrix.m[0][0] * fSin + matrix.m[2][0] * fCos,
                                matrix.m[0][1] * fSin + matrix.m[2][1] * fCos,
                                matrix.m[0][2] * fSin + matrix.m[2][2] * fCos);
    }

    const PblVector3 vTop(vBase.x + matrix.m[1][0] * fHeight,
                          vBase.y + matrix.m[1][1] * fHeight,
                          vBase.z + matrix.m[1][2] * fHeight);

    for (int i = 0; i < 8; i++)
    {
        const PblVector3& c0 = aCircle[i];
        const PblVector3& c1 = aCircle[(i + 1) % 8];
        DrawLine3D(PblVector3(vBase.x + c0.x * fRadius0, vBase.y + c0.y * fRadius0, vBase.z + c0.z * fRadius0),
                   PblVector3(vTop.x + c0.x * fRadius1, vTop.y + c0.y * fRadius1, vTop.z + c0.z * fRadius1),
                   color);
        DrawLine3D(PblVector3(vBase.x + c0.x * fRadius0, vBase.y + c0.y * fRadius0, vBase.z + c0.z * fRadius0),
                   PblVector3(vBase.x + c1.x * fRadius0, vBase.y + c1.y * fRadius0, vBase.z + c1.z * fRadius0),
                   color);
        DrawLine3D(PblVector3(vTop.x + c0.x * fRadius1, vTop.y + c0.y * fRadius1, vTop.z + c0.z * fRadius1),
                   PblVector3(vTop.x + c1.x * fRadius1, vTop.y + c1.y * fRadius1, vTop.z + c1.z * fRadius1),
                   color);
    }
}

// FUNCTION: SWBF2 0x006f12a0
void RedDebug::DrawLine3D(PblVector3 v0, PblVector3 v1, RedColor color)
{
    if (_n3DLines < 6000 - 2)
    {
        _aColor[_n3DLines / 2] = color;
        _a3DLineVectors[_n3DLines++] = v0;
        _a3DLineVectors[_n3DLines++] = v1;
    }
}

// FUNCTION: SWBF2 0x006f1300
void RedDebug::DrawOBB(const PblBox& box, const PblMatrix& matrix, RedColor color)
{
    PblVector3 aCorners[8];
    for (int i = 0; i < 8; i++)
    {
        PblVector3 vCorner((i & 4) ? box._vMax.x : box._vMin.x,
                           (i & 2) ? box._vMax.y : box._vMin.y,
                           (i & 1) ? box._vMax.z : box._vMin.z);
        PblVector3 vOut;
        D3DXVec3TransformCoord(&vOut, &vCorner, &matrix);
        aCorners[i] = vOut;
    }

    DrawLine3D(aCorners[0], aCorners[1], color);
    DrawLine3D(aCorners[0], aCorners[2], color);
    DrawLine3D(aCorners[0], aCorners[4], color);
    DrawLine3D(aCorners[3], aCorners[1], color);
    DrawLine3D(aCorners[3], aCorners[2], color);
    DrawLine3D(aCorners[3], aCorners[7], color);
    DrawLine3D(aCorners[5], aCorners[1], color);
    DrawLine3D(aCorners[5], aCorners[4], color);
    DrawLine3D(aCorners[5], aCorners[7], color);
    DrawLine3D(aCorners[6], aCorners[2], color);
    DrawLine3D(aCorners[6], aCorners[4], color);
    DrawLine3D(aCorners[6], aCorners[7], color);
}

static inline PblVector3 VecAdd(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
static inline PblVector3 VecSub(const PblVector3& a, const PblVector3& b)
{
    return PblVector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

// FUNCTION: SWBF2 0x006f16f0
void RedDebug::DrawSphere(PblVector3 vCenter, float fRadius, RedColor color)
{
    const float fDiag = fRadius * 0.7071f;

    const PblVector3 vX(fRadius, 0.0f, 0.0f);
    const PblVector3 vY(0.0f, fRadius, 0.0f);
    const PblVector3 vZ(0.0f, 0.0f, fRadius);
    const PblVector3 vXY(fDiag, fDiag, 0.0f);
    const PblVector3 vXnY(fDiag, -fDiag, 0.0f);
    const PblVector3 vYZ(0.0f, fDiag, fDiag);
    const PblVector3 vYnZ(0.0f, fDiag, -fDiag);
    const PblVector3 vXZ(fDiag, 0.0f, fDiag);
    const PblVector3 vnXZ(-fDiag, 0.0f, fDiag);

    DrawLine3D(VecSub(vCenter, vX), VecAdd(vCenter, vX), color);
    DrawLine3D(VecSub(vCenter, vY), VecAdd(vCenter, vY), color);
    DrawLine3D(VecSub(vCenter, vZ), VecAdd(vCenter, vZ), color);

    DrawLine3D(VecAdd(vCenter, vY), VecAdd(vCenter, vXY), color);
    DrawLine3D(VecAdd(vCenter, vXY), VecAdd(vCenter, vX), color);
    DrawLine3D(VecAdd(vCenter, vX), VecAdd(vCenter, vXnY), color);
    DrawLine3D(VecAdd(vCenter, vXnY), VecSub(vCenter, vY), color);
    DrawLine3D(VecSub(vCenter, vY), VecSub(vCenter, vXY), color);
    DrawLine3D(VecSub(vCenter, vXY), VecSub(vCenter, vX), color);
    DrawLine3D(VecSub(vCenter, vX), VecSub(vCenter, vXnY), color);
    DrawLine3D(VecSub(vCenter, vXnY), VecAdd(vCenter, vY), color);

    DrawLine3D(VecAdd(vCenter, vZ), VecAdd(vCenter, vYZ), color);
    DrawLine3D(VecAdd(vCenter, vYZ), VecAdd(vCenter, vY), color);
    DrawLine3D(VecAdd(vCenter, vY), VecAdd(vCenter, vYnZ), color);
    DrawLine3D(VecAdd(vCenter, vYnZ), VecSub(vCenter, vZ), color);
    DrawLine3D(VecSub(vCenter, vZ), VecSub(vCenter, vYZ), color);
    DrawLine3D(VecSub(vCenter, vYZ), VecSub(vCenter, vY), color);
    DrawLine3D(VecSub(vCenter, vY), VecSub(vCenter, vYnZ), color);
    DrawLine3D(VecSub(vCenter, vYnZ), VecAdd(vCenter, vZ), color);

    DrawLine3D(VecAdd(vCenter, vX), VecAdd(vCenter, vXZ), color);
    DrawLine3D(VecAdd(vCenter, vXZ), VecAdd(vCenter, vZ), color);
    DrawLine3D(VecAdd(vCenter, vZ), VecAdd(vCenter, vnXZ), color);
    DrawLine3D(VecAdd(vCenter, vnXZ), VecSub(vCenter, vX), color);
    DrawLine3D(VecSub(vCenter, vX), VecSub(vCenter, vXZ), color);
    DrawLine3D(VecSub(vCenter, vXZ), VecSub(vCenter, vZ), color);
    DrawLine3D(VecSub(vCenter, vZ), VecSub(vCenter, vnXZ), color);
    DrawLine3D(VecSub(vCenter, vnXZ), VecAdd(vCenter, vX), color);
}

// FUNCTION: SWBF2 0x006f2510
void RedDebug::RedDebugRenderable::Init()
{
    _n3DLines = 0;
    _nSpheres = 0;
    _nMarkers3D = 0;
    _nRadii = 0;

    _pCurrLinePos = _aLineBuffer;
    *_pCurrLinePos = 0;
    _uiNumText3D = 0;
    _uiNumTextLines = 0;

    FLRenderer::AddInterfaceElement(&_ScreenGroup, RedInterfaceScreen::LEFT,
                                    RedInterfaceScreen::TOP, false);

    for (int i = 0; i < 30; i++)
    {
        RedTextElement& line = _aTextLines[i];
        line.SetFont("systemfont");
        line.SetIntercharacterSpacing(0);
        line.Enable(false);
        line.BringToFront();
        _ScreenGroup.AddElement(&line);
    }

    for (int i = 0; i < 64; i++)
    {
        RedTextElement& text = _aText3D[i].text;
        text.SetFont("systemfont");
        text.SetTextAlign(RedTextElement::TEXT_HALIGN_CENTER, RedTextElement::TEXT_VALIGN_CENTER);
        text.Enable(false);
        text.SendToBack();
        _ScreenGroup.AddElement(&text);
    }

    SetPrintfData(RedColor::WHITE, RedGlyphCacheElement::TEXT_STYLE_NORMAL);

    s_debugShader = pcRedShader::Create(PblHash("lightbeam"), 0x1082, 0, 0, 0x200,
                                        RedColor::WHITE, RedColor::BLACK, 32.0f);
}

struct CompressedPos
{
    short x;
    short y;
    short z;
    short w;

    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};

struct DebugVertex
{
    CompressedPos position;
    RedColor color;
};
 
 

// FUNCTION: SWBF2 0x006f2dd0
void RedDebug::_DrawRadius(const Radius& radius)
{
    const PblVector3& vCamera = RedRenderer::GetCamera()->GetMatrix().GetTranslation();
    PblMatrix mWorld = _IdentityMatrix;
    mWorld.m[3][0] = vCamera.x;
    mWorld.m[3][1] = vCamera.y;
    mWorld.m[3][2] = vCamera.z;

    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x1082, 12, 66, 0, false);
    pPrimitive->m_bbox._vMin = PblVector3(-512.0f, -512.0f, -512.0f);
    pPrimitive->m_bbox._vMax = PblVector3(512.0f, 512.0f, 512.0f);
    CompressedPos::s_vMin = PblVector3{ -512.0f, -512.0f, -512.0f };
    CompressedPos::s_vScale = PblVector3{ 0.015625238f, 0.015625238f, 0.015625238f };
    CompressedPos::s_vScaleInv = PblVector3{ 63.999023f, 63.999023f, 63.999023f };

    DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
    if (!pVertex)
    {
        return;
    }
    const float fX = radius._vPos.x - vCamera.x;
    const float fZ = radius._vPos.z - vCamera.z;
    const float fY = radius._vPos.y - vCamera.y;
    const float fHalfHeight = radius._fHeight * 0.5f;
    for (unsigned int i = 0; i <= 32; ++i)
    {
        float fSin, fCos;
        PblMath::SinCos(&fSin, &fCos, (float)i * 0.19634955f);
        const float x = fX + radius._fRadius * fSin;
        const float z = fZ + radius._fRadius * fCos;
        for (int side = 0; side < 2; ++side)
        {
            const float y = side ? fY + fHalfHeight : fY - fHalfHeight;
            short* p = (short*)&pVertex->position;
            p[0] = (short)((x + 512.0f) * CompressedPos::s_vScaleInv.x - 32767.5f);
            p[1] = (short)((y + 512.0f) * CompressedPos::s_vScaleInv.y - 32767.5f);
            p[2] = (short)((z + 512.0f) * CompressedPos::s_vScaleInv.z - 32767.5f);
            pVertex->color = radius._color;
            ++pVertex;
        }
    }
    pPrimitive->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   s_debugShader,
                                   mWorld,
                                   RedColor::WHITE,
                                   0, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006f3170
void RedDebug::_DrawSolidOBB(const SolidOBB& obb)
{
    static const unsigned char order[10] = { 0, 2, 1, 3, 5, 7, 4, 6, 0, 2 };
    pcRedPrimitive* pPrimitive = pcRedPrimitive::Create(
        D3DPT_TRIANGLESTRIP, 0x1082, 12, 10, 0, false);
    pPrimitive->m_bbox._vMin = PblVector3(-512.0f, -512.0f, -512.0f);
    pPrimitive->m_bbox._vMax = PblVector3(512.0f, 512.0f, 512.0f);
    CompressedPos::s_vMin = PblVector3{ -512.0f, -512.0f, -512.0f };
    CompressedPos::s_vScale = PblVector3{ 0.015625238f, 0.015625238f, 0.015625238f };
    CompressedPos::s_vScaleInv = PblVector3{ 63.999023f, 63.999023f, 63.999023f };
    DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
    if (!pVertex)
    {
        return;
    }
    for (int i = 0; i < 10; ++i)
    {
        const unsigned char corner = order[i];
        const float x = (corner & 4) ? obb._vBox._vMax.x : obb._vBox._vMin.x;
        const float y = (corner & 2) ? obb._vBox._vMax.y : obb._vBox._vMin.y;
        const float z = (corner & 1) ? obb._vBox._vMax.z : obb._vBox._vMin.z;
        short* p = (short*)&pVertex->position;
        p[0] = (short)((x + 512.0f) * CompressedPos::s_vScaleInv.x - 32767.5f);
        p[1] = (short)((y + 512.0f) * CompressedPos::s_vScaleInv.y - 32767.5f);
        p[2] = (short)((z + 512.0f) * CompressedPos::s_vScaleInv.z - 32767.5f);
        pVertex->color = obb._color;
        ++pVertex;
    }
    pPrimitive->EndVertexData();
    RedRenderer::pcRenderPrimitive(pPrimitive,
                                   s_debugShader,
                                   obb._mBox,
                                   RedColor::WHITE,
                                   0, 0, 0, 0);
    pPrimitive->Release();
}

// FUNCTION: SWBF2 0x006f2710
void RedDebug::RedDebugRenderable::Render(int)
{
    if (_n3DLines >= 2)
    {
        PblBox box;
        box._vMin = _a3DLineVectors[0];
        box._vMax = _a3DLineVectors[0];
        for (unsigned int i = 1; i < _n3DLines; i++)
        {
            box.AddPoint(_a3DLineVectors[i]);
        }

        unsigned int uiNumLines = _n3DLines / 2;
        box._vMax.x += 1.0f;
        box._vMax.y += 1.0f;
        box._vMax.z += 1.0f;
        box._vMin.x -= 1.0f;
        box._vMin.y -= 1.0f;
        box._vMin.z -= 1.0f;

        pcRedPrimitive* pPrimitive =
            pcRedPrimitive::Create(D3DPT_LINELIST, 0x1082, 12, uiNumLines * 2, 0, false);
        pPrimitive->m_bbox = box;

        PblVector3 vMin = { box._vMin.x, box._vMin.y, box._vMin.z };
        PblVector3 vExtent = { box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y,
                                    box._vMax.z - box._vMin.z };
        CompressedPos::s_vMin = vMin;
        CompressedPos::s_vScale = vExtent * (1.0f / 65535.0f);
        CompressedPos::s_vScaleInv.x = 1.0f / CompressedPos::s_vScale.x;
        CompressedPos::s_vScaleInv.y = 1.0f / CompressedPos::s_vScale.y;
        CompressedPos::s_vScaleInv.z = 1.0f / CompressedPos::s_vScale.z;

        DebugVertex* pVertex = (DebugVertex*)pPrimitive->BeginVertexData();
        if (!pVertex)
        {
            return;
        }

        for (unsigned int i = 0; i < uiNumLines; i++)
        {
            for (int j = 0; j < 2; j++)
            {
                const PblVector3& v = _a3DLineVectors[i * 2 + j];
                short* p = (short*)&pVertex->position;
                p[0] = (short)((v.x - CompressedPos::s_vMin.x) * CompressedPos::s_vScaleInv.x - 32767.5f);
                p[1] = (short)((v.y - CompressedPos::s_vMin.y) * CompressedPos::s_vScaleInv.y - 32767.5f);
                p[2] = (short)((v.z - CompressedPos::s_vMin.z) * CompressedPos::s_vScaleInv.z - 32767.5f);
                pVertex->color = _aColor[i];
                ++pVertex;
            }
        }

        pPrimitive->EndVertexData();
        RedRenderer::pcRenderPrimitive(pPrimitive, s_debugShader, _IdentityMatrix,
                                       RedColor::WHITE, 0x200, 0, 0, 0);
        pPrimitive->Release();
    }

    _n3DLines = 0;

    for (unsigned int i = _nSpheres; i < 16; i++)
    {
        FLRenderer::RemoveRenderable(&_aSpheres[i]);
    }
    for (unsigned int i = _nMarkers3D; i < 16; i++)
    {
        FLRenderer::RemoveRenderable(&_aMarkers3D[i]);
    }

    for (unsigned int i = 0; i < _nRadii; i++)
    {
        _DrawRadius(_aRadii[i]);
    }
    for (unsigned int i = 0; i < _nOBB; i++)
    {
        _DrawSolidOBB(_aOBB[i]);
    }

    _DrawPrintf();
    _DrawPrintf3D();

    _n3DLines = 0;
    _nSpheres = 0;
    _nMarkers3D = 0;
    _nRadii = 0;
    _nOBB = 0;
    _pCurrLinePos = _aLineBuffer;
    _uiNumText3D = 0;
    _uiNumTextLines = 0;
}

// FUNCTION: SWBF2 0x006f2bb0
void RedSphereShape::Render(int)
{
}

// FUNCTION: SWBF2 0x006f2bc0
void RedTetrahedronShape::Render(int)
{
}

// FUNCTION: SWBF2 0x006f2670
void RedDebug::Printf3D(const PblVector3& vPos, const char* pcFormat, ...)
{
    if (_uiNumText3D == 64)
    {
        PblTraceP("RedDebug::Printf3D !WARNING! Array of 3D positions is full\n");
        return;
    }

    Text3D& text3D = _aText3D[_uiNumText3D];
    text3D.text.SetColor(_PrintfColor);
    text3D.text.m_eStyle = _ePrintfStyle;

    va_list args;
    va_start(args, pcFormat);
    text3D.text.SetTextFormattedV(pcFormat, args);
    va_end(args);

    _aText3D[_uiNumText3D].pos = vPos;
    _uiNumText3D++;
}

// FUNCTION: SWBF2 0x006f2bd0
void RedDebug::SetPrintfData(const RedColor& color, RedGlyphCacheElement::TextStyleT eStyle)
{
    _PrintfColor = color;
    _ePrintfStyle = eStyle;
}

// FUNCTION: SWBF2 0x006f2bf0
void RedDebug::_DrawPrintf3D()
{
    unsigned int i;
    for (i = 0; i < _uiNumText3D; i++)
    {
        const PblVector3 vScreen = RedRenderer::TransformProjectionPointToScreenSpace(
            RedRenderer::TransformWorldPointToProjectionSpace(_aText3D[i].pos));
        _aText3D[i].text.SetPosition(vScreen.x, vScreen.y - 25.0f);
        _aText3D[i].text.Enable(true);
    }
    for (; i < 64; i++)
    {
        _aText3D[i].text.Enable(false);
    }
}

// FUNCTION: SWBF2 0x006f2cb0
void RedDebug::_DrawPrintf()
{
    if (_pCurrLinePos > _aLineBuffer)
    {
        _aTextLines[_uiNumTextLines % 30].SetText(_aLineBuffer, true);
        _uiNumTextLines++;
        _pCurrLinePos = _aLineBuffer;
        *_aLineBuffer = 0;
    }

    unsigned int uiFirst = 0;
    if (_uiNumTextLines > 30)
    {
        uiFirst = _uiNumTextLines - 29;
    }

    unsigned int i;
    for (i = 0; i < _uiNumTextLines - uiFirst; i++)
    {
        RedTextElement& line = _aTextLines[(i + uiFirst) % 30];
        line.SetPosition(10.0f, (float)i * 15.0f + 10.0f);
        line.Enable(true);
    }
    for (; i < 30; i++)
    {
        _aTextLines[(i + uiFirst) % 30].Enable(false);
    }
}

// SYNTHETIC: SWBF2 0x004090a0
// `dynamic initializer for 'PblMath::fOneOverHalfPi''

// SYNTHETIC: SWBF2 0x004090c0
// `dynamic initializer for 'PblMath::fOneOverTwoPi''
