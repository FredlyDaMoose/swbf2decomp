#pragma once

#include <stddef.h>
#include "RedBitmapElement.h"

class pcInterfaceShader;

class RedMaskedBitmapElement : public RedBitmapElement
{
public:
    RedMaskedBitmapElement();

    RedMaskedBitmapElement(const char* pcName);
    virtual ~RedMaskedBitmapElement();
    // SYNTHETIC: SWBF2 0x006f7eb0
    // RedMaskedBitmapElement::`scalar deleting destructor'

    // FUNCTION: SWBF2 0x0054cb00
    virtual bool IsMaskedBitmap() const { return true; }
    virtual void Cleanup();
protected:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);
public:
    virtual void Init();
    virtual void SetTexture(unsigned int uiTextureHash);
    void SetMaskTexture(unsigned int uHash);
    const RedTexture* GetMaskTexture() const;

    void GetMaskTexCoords(float& rf0, float& rf1, float& rf2, float& rf3) const;
    void SetMaskTexCoords(float fU0, float fU1, float fV0, float fV1);

    pcInterfaceShader* m_pInterfaceMaskedBitmapShader;
};
 
 
