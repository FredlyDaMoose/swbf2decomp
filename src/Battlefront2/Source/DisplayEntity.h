#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblQuaternion.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"

class GameModel;

class EntityElement : public RedInterfaceElement
{
public:
    EntityElement();
    virtual ~EntityElement();

protected:
    virtual void RenderUsingContext(const PblMatrix& mContext, const RedColor& color);

public:
    virtual bool Setup() = 0;
    virtual bool Update(float fElapsed) = 0;
    virtual PblVector3 GetPos() = 0;
    virtual void SetScale(float fScale);
    virtual void SetSelected(bool bSelected);

    void SetModel(GameModel* pModel) { mModel = pModel; }
    void SetDepth(float fDepth) { mDepth = fDepth; }

    float mScale;
    float mDepth;
    PblQuaternion mRotation;
    PblVector3 mTranslation;
    GameModel* mModel;
};

 
 
 
