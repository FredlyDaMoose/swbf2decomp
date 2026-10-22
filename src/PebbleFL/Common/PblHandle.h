#pragma once

class PblHandled
{
public:

    PblHandled();

    virtual ~PblHandled() { mHandleId = 0; }

    // FUNCTION: SWBF2 0x00423e90
    unsigned int GetHandleId() const { return mHandleId; }

    static unsigned int SetNextHandleId(unsigned int uId);

    unsigned int mHandleId;
};

template <class TYPE>
class PblHandle
{
public:

    // FUNCTION: SWBF2 0x0043a120
    TYPE* GetValidObject() const { return GetObject(); }

    TYPE* operator()() const { return GetObject(); }

    TYPE* mObject;
    unsigned int mSavedHandleId;

    // TEMPLATE: SWBF2 0x00439a80
    // PblHandle<GameObject>::PblHandle
    PblHandle(TYPE* pObject) : mObject(pObject), mSavedHandleId(pObject ? pObject->mHandleId : 0) {}
    PblHandle() : mObject(0), mSavedHandleId(0) {}

    // TEMPLATE: SWBF2 0x0042cd10
    // PblHandle<GameObject>::PblHandle<GameObject>

    // TEMPLATE: SWBF2 0x004479c0
    // PblHandle<FLEffectObject>::PblHandle<FLEffectObject>

    // TEMPLATE: SWBF2 0x004b97c0
    // PblHandle<ParticleEmitterObject>::PblHandle<ParticleEmitterObject>

    // TEMPLATE: SWBF2 0x004479d0
    // PblHandle<FLEffectObject>::~PblHandle<FLEffectObject>

    // TEMPLATE: SWBF2 0x004b97d0
    // PblHandle<ParticleEmitterObject>::~PblHandle<ParticleEmitterObject>

    // TEMPLATE: SWBF2 0x0042cd20
    // PblHandle<GameObject>::~PblHandle<GameObject>
    ~PblHandle() {}

    // TEMPLATE: SWBF2 0x005d3700
    // PblHandle<GameObject>::GetHandleId
    unsigned int GetHandleId() const { return mSavedHandleId; }

    // TEMPLATE: SWBF2 0x0043cd00
    // PblHandle<GameObject const>::operator==

    // TEMPLATE: SWBF2 0x00470880
    // PblHandle<GameObject>::operator==

    // TEMPLATE: SWBF2 0x00489c20
    // PblHandle<FLEffectObject>::operator==
    bool operator==(TYPE* pObject) const
    {
        return GetObject() == pObject;
    }

    // TEMPLATE: SWBF2 0x00424b00
    // PblHandle<GameObject>::operator->
    TYPE* operator->() const { return GetObject(); }

    // TEMPLATE: SWBF2 0x0043a150
    // PblHandle<GameObject const>::operator GameObject const *

    // TEMPLATE: SWBF2 0x00428e60
    // PblHandle<ParticleEmitterObject>::operator ParticleEmitterObject *
    operator TYPE*() const { return GetObject(); }

    // TEMPLATE: SWBF2 0x00447a10
    // PblHandle<FLEffectObject>::operator!=

    // TEMPLATE: SWBF2 0x0043a0f0
    // PblHandle<GameObject const >::operator!=
    bool operator!=(TYPE* pObject) const { return GetObject() != pObject; }

    // TEMPLATE: SWBF2 0x0043ccd0
    // PblHandle<GameObject const>::operator=

    // TEMPLATE: SWBF2 0x0042cd30
    // PblHandle<GameObject>::operator=
    void operator=(TYPE* pObject)
    {
        mObject = pObject;
        mSavedHandleId = pObject ? pObject->mHandleId : 0;
    }

private:
    TYPE* GetObject() const
    {
        if (mObject && mObject->mHandleId != mSavedHandleId)
        {
            PblHandle* pThis = const_cast<PblHandle*>(this);
            pThis->mObject = 0;
            pThis->mSavedHandleId = 0;
        }
        return mObject;
    }
};
