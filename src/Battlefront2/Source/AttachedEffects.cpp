#include "AttachedEffects.h"
#include "PebbleFL/Common/PblVector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Entity.h"
#include "EntityGeometry.h"
#include "EntityLight.h"
#include "FLEffect.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblRandom.h"

class GameObject
{
public:

    char mInheritedObjectStorage[0x204];
    unsigned int mHandleId;
};

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

AttachedEffectsClass::AttachClassData AttachedEffectsClass::s_aAttachClassData[MAX_ATTACHED];
unsigned int AttachedEffectsClass::s_uiNumAttached;
bool AttachedEffectsClass::s_bDynamic;

AttachedEffectsClass::AttachedEffectsClass()
{
    m_uiNumAttached = 0;
    m_bDynamic = 0;
}

// FUNCTION: SWBF2 0x00447910
AttachedEffectsClass* AttachedEffectsClass::BuildAttachedEffectsClass()
{
    if (!s_uiNumAttached)
    {
        return 0;
    }

    AttachedEffectsClass* pClass = new AttachedEffectsClass;
    pClass->m_uiNumAttached = s_uiNumAttached;
    pClass->m_aAttachClassData = new AttachClassData[s_uiNumAttached];
    memcpy(pClass->m_aAttachClassData, s_aAttachClassData, s_uiNumAttached * sizeof(AttachClassData));
    pClass->m_bDynamic = s_bDynamic;

    s_uiNumAttached = 0;
    s_aAttachClassData[0].bIsOdf = 0;
    s_aAttachClassData[0].pOdfClass = 0;
    s_bDynamic = false;
    return pClass;
}

// FUNCTION: SWBF2 0x00446e60
AttachedEffects::AttachedEffects(const AttachedEffectsClass* pClass, const EntityGeometry* pGeometry,
                                 bool bDynamic)
    : m_pClass(const_cast<AttachedEffectsClass*>(pClass)),
      m_pGameObject(pGeometry->GetGameObject()),
      m_pEntityGeometry(0),
      m_bDynamic(bDynamic)
{
    if (!(const GameObject*)m_pGameObject)
    {
        m_pEntityGeometry = const_cast<EntityGeometry*>(pGeometry);
        if (!m_bDynamic)
        {
            bool bTimed = false;
            for (unsigned int i = 0; i < m_pClass->m_uiNumAttached; ++i)
            {
                const AttachedEffectsClass::AttachClassData& classData = m_pClass->m_aAttachClassData[i];
                if (!classData.bIsOdf && classData.uiNumParams >= 1)
                {
                    bTimed = true;
                    break;
                }
            }
            if (!bTimed)
            {
                return;
            }
        }
    }
    ActivateThread((int)0x80000002, PER_FRAME);
}

// FUNCTION: SWBF2 0x00447220
AttachedEffects* AttachedEffectsClass::BuildEffects(const EntityGeometry* pGeometry) const
{
    if (!m_uiNumAttached)
    {
        return 0;
    }

    AttachedEffects* pEffects = new AttachedEffects(this, pGeometry, m_bDynamic);
    pEffects->m_aAttachData = new AttachedEffects::AttachData[m_uiNumAttached];

    for (unsigned int i = 0; i < m_uiNumAttached; ++i)
    {
        const AttachClassData& classData = m_aAttachClassData[i];
        AttachedEffects::AttachData& data = pEffects->m_aAttachData[i];

        if (classData.bIsOdf)
        {
            EntityDesc desc;
            desc.mMatrix = _IdentityMatrix;
            desc.mMatrix = pGeometry->GetMatrix();
            Entity* pEntity = classData.pOdfClass->Build(desc);
            if (pEntity->IsRtti(EntityLight::GetRtti()))
            {
                data.pEffect = static_cast<EntityLight*>(pEntity);
            }
            data.fTimer = 0.0f;
            if (data.pEffect && classData.uiNumParams >= 1)
            {
                char acBuffer[64];
                sprintf(acBuffer, "%f", classData.afParam[0] > 0.0f ? classData.afParam[0] : 0.0f);
                pEntity->SetProperty(0xE6915CF6, acBuffer);
            }
        }
        else
        {
            bool bDelayed = false;
            if (classData.uiNumParams >= 1)
            {
                const float fMin = classData.afParam[0];
                const float fMax = classData.uiNumParams >= 2 ? classData.afParam[1] : fMin;
                if (fMax > 0.0f && fMin >= 0.0f)
                {
                    data.fTimer = -(PblRandom::GetGlobalFloat() * fMax);
                    data.pEffect = 0;
                    bDelayed = true;
                }
            }
            if (!bDelayed)
            {
                data.fTimer = 0.0f;
                data.pEffect = classData.pEffectClass->BuildEffect();
            }
        }

        if (data.pEffect)
        {
            data.pEffect->AttachEffectToObject(pGeometry, classData.uiBoneName, false);
        }
    }
    return pEffects;
}

// FUNCTION: SWBF2 0x00447650
AttachedEffects::AttachData::AttachData()
{
    pEffect.mObject = 0;
    pEffect.mSavedHandleId = 0;
}

// FUNCTION: SWBF2 0x00447660
AttachedEffects::AttachData::~AttachData()
{
}

// FUNCTION: SWBF2 0x00447670
bool AttachedEffectsClass::SetProperty(unsigned int uiProperty, const char* pszValue)
{
    switch (uiProperty)
    {
    case 0x51e2c845:
        s_bDynamic = atoi(pszValue) != 0;
        return true;

    case 0x3be7b80a:
        if (s_uiNumAttached < MAX_ATTACHED && s_aAttachClassData[s_uiNumAttached].pOdfClass)
        {
            char acBone[256];
            AttachClassData& data = s_aAttachClassData[s_uiNumAttached];
            const int iNumRead =
                sscanf(pszValue, "%s %f %f", acBone, &data.afParam[0], &data.afParam[1]);
            if (iNumRead >= 1)
            {
                s_aAttachClassData[s_uiNumAttached].uiBoneName = PblTEMPHash(acBone);
                s_aAttachClassData[s_uiNumAttached].uiNumParams = iNumRead - 1;
                s_aAttachClassData[s_uiNumAttached].uiFlags = 0;
                ++s_uiNumAttached;
                if (s_uiNumAttached < MAX_ATTACHED)
                {
                    s_aAttachClassData[s_uiNumAttached].bIsOdf =
                        s_aAttachClassData[s_uiNumAttached - 1].bIsOdf;
                    s_aAttachClassData[s_uiNumAttached].pOdfClass =
                        s_aAttachClassData[s_uiNumAttached - 1].pOdfClass;
                }
            }
        }
        return true;

    case 0x576b09cd:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            char acEffect[256];
            char acBone[256];
            if (sscanf(pszValue, "%s %s", acEffect, acBone) >= 2)
            {
                s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
                s_aAttachClassData[s_uiNumAttached].pEffectClass =
                    FLEffect::s_EffectClasses.Find(PblHash(acEffect));
                if (s_aAttachClassData[s_uiNumAttached].pEffectClass)
                {
                    s_aAttachClassData[s_uiNumAttached].uiBoneName = PblTEMPHash(acBone);
                    s_aAttachClassData[s_uiNumAttached].uiNumParams = 0;
                    s_aAttachClassData[s_uiNumAttached].uiFlags = 0;
                    ++s_uiNumAttached;
                    if (s_uiNumAttached < MAX_ATTACHED)
                    {
                        s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
                        s_aAttachClassData[s_uiNumAttached].pOdfClass = 0;
                    }
                }
            }
        }
        return true;

    case 0xa9d0d48b:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            AttachClassData& data = s_aAttachClassData[s_uiNumAttached];
            data.bIsOdf = 1;
            data.pOdfClass = EntityClass::Find(PblHash(pszValue));
            if (data.pOdfClass && !data.pOdfClass->IsRtti(EntityLightClass::GetRtti()))
            {
                data.bIsOdf = 0;
                data.pOdfClass = 0;
            }
        }
        return true;

    case 0x6a6c7e0d:
        if (s_uiNumAttached < MAX_ATTACHED)
        {
            s_aAttachClassData[s_uiNumAttached].bIsOdf = 0;
            s_aAttachClassData[s_uiNumAttached].pEffectClass =
                FLEffect::s_EffectClasses.Find(PblHash(pszValue));
        }
        return true;
    }
    return false;
}

// FUNCTION: SWBF2 0x00446f70
bool AttachedEffects::Update(float fDeltaTime)
{
    const EntityGeometry* pGeometry = m_pEntityGeometry;
    if (!pGeometry)
    {
        if (!(const GameObject*)m_pGameObject)
        {
            return false;
        }
        pGeometry = reinterpret_cast<const EntityGeometry*>((const GameObject*)m_pGameObject);
    }

    for (unsigned int i = 0; i < m_pClass->m_uiNumAttached; ++i)
    {
        const AttachedEffectsClass::AttachClassData& classData = m_pClass->m_aAttachClassData[i];
        AttachData& data = m_aAttachData[i];

        if (!classData.bIsOdf && !data.pEffect)
        {
            data.fTimer += fDeltaTime;
            if (classData.uiNumParams >= 1)
            {
                if (data.fTimer - fDeltaTime > 0.0f)
                {
                    const float fMin = classData.afParam[0] > 0.0f ? classData.afParam[0] : 0.0f;
                    float fMax = fMin;
                    if (classData.uiNumParams >= 2 && classData.afParam[1] > fMin)
                    {
                        fMax = classData.afParam[1];
                    }
                    data.fTimer = -fMin;
                    if (fMax > fMin)
                    {
                        data.fTimer -= PblRandom::GetGlobalFloat() * (fMax - fMin);
                    }
                }
                else if (data.fTimer > 0.0f)
                {
                    data.pEffect = classData.pEffectClass->BuildEffect();
                    if (data.pEffect)
                    {
                        data.pEffect->AttachEffectToObject(pGeometry, classData.uiBoneName, false);
                    }
                }
            }
        }
        else if (m_pClass->m_bDynamic && data.pEffect)
        {
            data.pEffect->AttachEffectToObject(pGeometry, classData.uiBoneName, false);
        }
    }
    return true;
}
