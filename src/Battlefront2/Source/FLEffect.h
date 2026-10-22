#pragma once

#include <stddef.h>
#include "RedEngineFL/Application/Renderable.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"
#include "Thread.h"
#include "PebbleFL/Common/PblHandle.h"
#include "PebbleFL/Common/PblMatrix.h"

class EntityGeometry;
class FLEffectClass;
class GameObject;
class PblFileChunk;
class PblVector3;
class PblConfig;
class PblMatrix;

// VTABLE: SWBF2 0x00796a34
class FLEffect : public Renderable
{
public:

    FLEffect(unsigned int uiName, int ePhase)
        : Renderable(ePhase)
        , m_bEnabled(true)
    {
        RegisterManager(this, uiName);
    }

    // FUNCTION: SWBF2 0x0042cfc0
    virtual ~FLEffect() { UnregisterManager(this); }

    // FUNCTION: SWBF2 0x0042d010
    virtual void Init() {}
    // FUNCTION: SWBF2 0x0042d020
    virtual void Cleanup() {}
    // FUNCTION: SWBF2 0x0042d030
    virtual void Update(float) {}
    // FUNCTION: SWBF2 0x0042d040
    virtual void ReadConfig(PblConfig&) {}
    // FUNCTION: SWBF2 0x0042d050
    virtual FLEffectClass* BuildClass(PblConfig&) const { return 0; }

    void Enable(bool bEnable);

    bool IsEnabled() const { return m_bEnabled; }

    static void InitAll(bool bMission);

    static void CleanupAll();

    static void Read(PblFileChunk& chunk);

    static void Unregister(unsigned int uiClassName);

    static FLEffectClass* Get(unsigned int uHash);

    static int s_EffectClassesNumEntries;
    static PblHashTable<FLEffectClass, 256> s_EffectClasses;

    static int s_EffectFactoriesNumEntries;
    static PblHashTable<FLEffect, 32> s_EffectFactories;

    static bool s_bInitRender;

    // VTABLE: SWBF2 0x0079ef34
    class FLEffectThread : public Thread
    {
    public:
        FLEffectThread() {}
        virtual ~FLEffectThread() {}

        virtual bool Update(float fDeltaTime);
    };

    static FLEffectThread _Thread;

    bool m_bEnabled;
    int m_renderPriority;

protected:

    static void RegisterManager(FLEffect* pEffect, unsigned int uiName);

    void SetRenderPriority(int iPriority)
    {
        m_renderPriority = iPriority;
        if (m_bEnabled)
        {
            Deactivate();
            Activate(m_renderPriority);
        }
    }

    static void UnregisterManager(FLEffect* pEffect);
};
 
 
 
 
  class FLEffectObject : public Thread, public PblHandled
{
public:

    enum ATTACH_FLAGS
    {
        ATTACHED_DYNAMIC = 1,
        ATTACHED_TO_GAMEOBJECT = 2,
        ATTACHED_TO_VECTOR = 4,
        UPDATE_REQUIRED = 8,
        KILL_ON_NEXT_UPDATE = 16,
    };

    FLEffectObject()
        : m_uiAttachFlags(0)
        , m_pmAttachedToMatrix(0)
        , m_pmLocalOffsetMatrix(0)
    {
        m_hobjAttachedToObject.mObject = 0;
        m_hobjAttachedToObject.mSavedHandleId = 0;
    }

    virtual ~FLEffectObject();
    virtual bool IsEffectActive() const = 0;
    virtual void ActivateEffect();
    virtual void DeactivateEffect();
    virtual void StopAndFinish();
protected:
    virtual bool Update(float fDeltaTime);
    virtual void UpdatedWorldMatrix();
    virtual void InterpolateWorldMatrix();
public:

    void AttachEffectToVector(const PblVector3* pVector, bool b);
    void AttachEffectToVector(const PblVector3* pVector, const PblVector3& vDir,
                              bool bDynamic);

    void AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix* pMatrix,
                              const PblMatrix& mOffset, bool bDynamic);
    void AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix* pMatrix,
                              bool bDynamic);
    void AttachEffectToObject(const EntityGeometry* pObject, const PblMatrix& mOffset,
                              bool bDynamic);
    void AttachEffectToObject(const EntityGeometry* pObject, bool bDynamic);
    void AttachEffectToObject(const EntityGeometry* pObject, unsigned int uiBone,
                              const PblMatrix& mOffset, bool bDynamic);
    void AttachEffectToObject(const EntityGeometry* pObject, unsigned int uiBone,
                              bool bDynamic);
    void AttachEffectToMatrix(const PblMatrix* pMatrix, const PblMatrix& mOffset,
                              bool bDynamic);

    void SetLocalOffsetMatrix(const PblMatrix& mOffset);
    void SetMatrix(const PblMatrix& mMatrix);
    void SetOrientation(const PblVector3& vDir);
    void SetPosition(const PblVector3& vPos);

private:
    void _Attach(const PblMatrix& mOffset);
    const EntityGeometry* _GetObject() const;
    void _SetObject(const EntityGeometry* pObject);
    void _UpdateWorldMatrix();

public:

    unsigned int m_uiAttachFlags;
    PblHandle<GameObject const> m_hobjAttachedToObject;
    union
    {
        PblMatrix* m_pmAttachedToMatrix;
        PblVector3* m_pvAttachedToVector;
    };
    PblMatrix* m_pmLocalOffsetMatrix;
    PblMatrix m_mWorldMatrix;

    static void Init();

    static MemoryPool s_OffsetMatrixPool;

    void AttachEffectToMatrix(const PblMatrix* pMatrix, bool b);
};
 
 
 
 
 
 // VTABLE: SWBF2 0x0079ef4c
class FLEffectClass
{
public:
    FLEffectClass();

    virtual ~FLEffectClass();
    virtual FLEffectObject* BuildEffect() = 0;

    void SetClassName(unsigned int uiClassName) { m_uiClassName = uiClassName; }

    unsigned char mType;
    unsigned int m_uiClassName;
};
 
 
 
 