#pragma once

#include "RedEngineFL/Util/RedSceneManager.h"

// VTABLE: SWBF2 0x00796f08
class FrontlineSceneManager : public RedSceneManager
{
public:

    FrontlineSceneManager();
    virtual ~FrontlineSceneManager();

protected:
    virtual void BeginSceneRender();
    virtual void EndSceneRender();
};
 
