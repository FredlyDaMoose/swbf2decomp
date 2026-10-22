#pragma once

#include <stddef.h>

#include "RedHotSpot.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"

// VTABLE: SWBF2 0x00772970

class RedGroupElement;
class PblVector3;

class RedInterfaceElement
{
public:

    enum BlendModeT
    {
        BLEND_ALPHA = 0,
        BLEND_ADDITIVE = 1
    };

    // FUNCTION: SWBF2 0x006bff90
    RedInterfaceElement();

    // FUNCTION: SWBF2 0x006bfeb0
    RedInterfaceElement(const char* pcName);

    // SYNTHETIC: SWBF2 0x006c0100
    // RedInterfaceElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x006c0070
    virtual ~RedInterfaceElement();

    // FUNCTION: SWBF2 0x0048c8f0
    virtual void Enable(bool bEnable) { m_bEnabled = bEnable; }

    // FUNCTION: SWBF2 0x0048c910
    virtual void Visible(bool bVisible) { m_bVisible = bVisible; }

    // FUNCTION: SWBF2 0x0048c930
    virtual void SetBlendMode(BlendModeT) {}

    // FUNCTION: SWBF2 0x0048c940
    virtual BlendModeT GetBlendMode() const { return BLEND_ALPHA; }

    // FUNCTION: SWBF2 0x0048c950
    virtual bool IsGroup() const { return false; }

    // FUNCTION: SWBF2 0x0048c960
    virtual bool IsScreenGroup() const { return false; }

    // FUNCTION: SWBF2 0x0048c970
    virtual bool IsText() const { return false; }

    // FUNCTION: SWBF2 0x0048c980
    virtual bool IsFlashyText() const { return false; }

    // FUNCTION: SWBF2 0x0048c990
    virtual bool IsBitmap() const { return false; }

    // FUNCTION: SWBF2 0x0048c9a0
    virtual bool IsMaskedBitmap() const { return false; }

    // FUNCTION: SWBF2 0x0048c9b0
    virtual bool IsBorderElement() const { return false; }

    // FUNCTION: SWBF2 0x0048c9c0
    virtual bool IsModel() const { return false; }

    // FUNCTION: SWBF2 0x0048c9d0
    virtual void* GetParent() { return 0; }

    // FUNCTION: SWBF2 0x0048ca80
    virtual void CreateHotSpot(float fRadius)
    {
        if (!m_pHotSpot)
        {
            m_pHotSpot = new RedHotSpot;
        }
        m_pHotSpot->SetRect(-fRadius, -fRadius, fRadius, fRadius);
    }

    // FUNCTION: SWBF2 0x0048cb20
    virtual void Cleanup() {}

    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color) = 0;

    // FUNCTION: SWBF2 0x006c07e0
    void SetName(const char* pcName);

    void AddToSearchList(const char* pcName);

    // FUNCTION: SWBF2 0x006c0750
    void SetMatrixPosScale(float fX, float fY, float fScaleX, float fScaleY);

    void SetMatrix(const PblMatrix& matrix);

    const PblMatrix& GetMatrix() const { return m_mLocal; }
    void SetAlphaOnly(unsigned char uAlpha) { m_color.a = uAlpha; }

    void Scale(float fScaleX, float fScaleY);
    void Scale(float fScale) { Scale(fScale, fScale); }

    void Rotate(float fAngle);
    void Rotate(float fX, float fY, float fZ);
    void CreateHotSpot(float fLeft, float fTop, float fRight, float fBottom);

    void SetColor(RedColor color) { m_color = color; }

    // FUNCTION: SWBF2 0x00429210
    bool TestHotSpot(float fX, float fY) const { return m_pHotSpot && m_pHotSpot->IsPointInside(fX, fY); }

    void RemoveHotSpot()
    {
        delete m_pHotSpot;
        m_pHotSpot = 0;
    }

    void SetColorOnly(RedColor color)
    {
        m_color.b = color.b;
        m_color.g = color.g;
        m_color.r = color.r;
    }

    void SetMatrixPosRotScale(float fX, float fY, float fRotation, float fScaleX,
                              float fScaleY);

    bool IsEnabled() const { return m_bEnabled; }

    static RedInterfaceElement* Find(const char* pcName);

    static RedInterfaceElement* Find(unsigned int uiNameHash);

    // FUNCTION: SWBF2 0x006c0880
    void SetZOrder(unsigned char uZOrder);

    // FUNCTION: SWBF2 0x006c0170
    void BringToFront();
    // FUNCTION: SWBF2 0x006c0590
    void SendToBack();

    // FUNCTION: SWBF2 0x006c0820
    void SetPosition(float fX, float fY);

    void SetPosition(float fX, float fY, float fZ);
    void SetPosition(const PblVector3& vPos);

    RedGroupElement* GetGroup() const;

    const PblVector3& GetPosition() const { return *(const PblVector3*)m_mLocal.m[3]; }

protected:
    // FUNCTION: SWBF2 0x006c0de0
    void Render(const PblMatrix& mParent, const RedColor& color);

    friend class RedGroupElement;

    friend class RedInterfaceScreen;

public:

    unsigned int m_crcName;

    unsigned int m_zorder : 8;
    unsigned int m_bEnabled : 1;
    unsigned int m_bInAllObjectList : 1;
    unsigned int m_bVisible : 1;
    unsigned int m_uiFlags : 21;

    RedHotSpot* m_pHotSpot;
    PblList<RedInterfaceElement>::Node m_groupNode;
    RedColor m_color;
    PblMatrix m_mLocal;

    static PblHashTable<RedInterfaceElement, 32768> sAllObjectsList;
    static int sAllObjectsListNumEntries;
    static void Reset();

    static unsigned int s_uiUnnamedCRC;
};

 
 
 
 
 
 
