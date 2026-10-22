#pragma once

#include <stddef.h>

#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"

class AttachedEffectsClass;
class EntityClass;
class EntityGeometry;
class FLEffectClass;
class FLEffectObject;
class GameObject;

// VTABLE: SWBF2 0x00796ef0
class AttachedEffects : public Thread
{
public:

    struct AttachData
    {
        AttachData();
        ~AttachData();

        PblHandle<FLEffectObject> pEffect;
        float fTimer;
    };

    AttachedEffects(const AttachedEffectsClass* pClass, const EntityGeometry* pGeometry,
                    bool bDynamic);

    // SYNTHETIC: SWBF2 0x00446f40
    // AttachedEffects::`scalar deleting destructor'

    virtual bool Update(float fDeltaTime);

    AttachedEffectsClass* m_pClass;
    PblHandle<GameObject const> m_pGameObject;
    EntityGeometry* m_pEntityGeometry;
    bool m_bDynamic;
    AttachData* m_aAttachData;
};
 
 
 
 
 

class AttachedEffectsClass
{
public:
    enum { MAX_ATTACHED = 64, MAX_PARAMS = 2 };

    struct AttachClassData
    {
        unsigned int uiBoneName;
        union
        {
            EntityClass* pOdfClass;
            FLEffectClass* pEffectClass;
        };
        unsigned int bIsOdf : 1;
        unsigned int uiNumParams : 7;
        unsigned int uiFlags : 24;
        float afParam[MAX_PARAMS];
    };

    AttachedEffectsClass();

    static AttachedEffectsClass* BuildAttachedEffectsClass();
    AttachedEffects* BuildEffects(const EntityGeometry* pGeometry) const;
    static bool SetProperty(unsigned int uiProperty, const char* pszValue);

    AttachClassData* m_aAttachClassData;
    unsigned int m_uiNumAttached : 8;
    unsigned int m_bDynamic : 1;
    unsigned int z_uiReserved0 : 23;

    // GLOBAL: SWBF2 0x01e2fde8
    static AttachClassData s_aAttachClassData[MAX_ATTACHED];

    // GLOBAL: SWBF2 0x01e2fde0
    static unsigned int s_uiNumAttached;

    // GLOBAL: SWBF2 0x01e2fde4
    static bool s_bDynamic;
};
 
 
 
