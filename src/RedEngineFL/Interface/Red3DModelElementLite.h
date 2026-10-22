#pragma once

#include <stddef.h>
#include "RedInterfaceElement.h"

class RedModel;

class Red3DModelElementLite : public RedInterfaceElement
{
public:

    Red3DModelElementLite();

    Red3DModelElementLite(const char* const pcName);

    virtual ~Red3DModelElementLite();

    void SetModel(unsigned int uiModelHash);
    void SetModel(const char* const pcModelName);

    void SetModel(const RedModel* const pModel);

    void SetBone(unsigned int uiBoneHash);

    void SetBone(const char* const pcBoneName);

    const RedModel* GetModel() const { return m_Model; }
    unsigned int GetBone() const { return m_boneHash; }
    bool IsLightningEnabled() const { return m_bLightingEnabled; }
    void EnableLighting(bool bEnable) { m_bLightingEnabled = bEnable; }
    float GetDepth() const { return m_fDepth; }

    static float GetDepthMultiplier(bool bScreenScale);
    void SetDepth(float fDepth) { m_fDepth = fDepth; }

    // FUNCTION: SWBF2 0x00555c40
    virtual bool IsModel() const { return true; }

    static void SetPerspectiveHack(bool bEnable);
    static bool sPerspectiveHackEnabled;

    bool FindModel();
    unsigned int GetRenderFlags() const;
    void LocalToWorld(PblMatrix& mMatrix) const;

protected:

    void Init();

    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:
    float m_fDepth;
    unsigned int m_uiModelHash;
    RedModel* m_Model;
    unsigned int m_boneHash;
    bool m_bLightingEnabled : 1;
    bool m_searchForModel : 1;
};
 
 
 
 
 
