#include <stdio.h>
#include <string.h>

#include "DamageRegion.h"
#include "Damageable.h"
#include "GameLoop.h"
#include "GameObject.h"
#include "TriggerRegionManager.h"

static Damageable* GetDamageable(GameObject* pObject)
{
    return static_cast<Damageable*>(pObject);
}

// GLOBAL: SWBF2 0x007eb788
PblList<DamageRegion> DamageRegion::sList;

// GLOBAL: SWBF2 0x01e30940
DamageRegion::UpdateThread* DamageRegion::mUpdateThread;

// GLOBAL: SWBF2 0x007ec444
static const char* sDamageParameterNames[8] =
{
    "damagerate",
    "personscale",
    "animalscale",
    "droidscale",
    "vehiclescale",
    "buildingscale",
    "buildingdeadscale",
    "buildingunbuiltscale"
};

// FUNCTION: SWBF2 0x00489ed0
bool DamageRegion::UpdateThread::Update(float fDeltaTime)
{
    DamageRegion::UpdateAll(fDeltaTime);
    return true;
}

// FUNCTION: SWBF2 0x00489ef0
DamageRegion::DamageRegion(RedRegionDesc& desc, const Damage& damage)
    : RedRegion(desc), mDamage(damage)
{
    sList.Add(&mDamageListNode, this);
}

// FUNCTION: SWBF2 0x00489f80
void DamageRegion::Update(float fDeltaTime) const
{
    GameObject* apObjects[64];
    int iCount = TriggerRegionManager::GetObjectsInRegion(this, apObjects, 64);
    if (!iCount)
    {
        return;
    }

    DamageDesc damage(DamageOwner(NULL, NULL, 0, NULL, NULL, 1.0f), 0.0f);
    damage.mBaseValue = mDamage.damageRate * fDeltaTime;
    for (int i = 0; i < 8; ++i)
    {
        damage.mHealthScale[i] = mDamage.healthScale[i];
    }
    damage.mShieldScale = mDamage.shieldScale;

    do
    {
        GameObject* pObject = apObjects[--iCount];
        Damageable* pDamageable = GetDamageable(pObject);
        if (pDamageable->mIsAlive && GameLoop::GetMissionTime() > pDamageable->mDamageRegionTime)
        {
            pDamageable->SetDamageRegionTime();
            pDamageable->ApplyDamage(damage, 0xFFFFFFFB);
            pObject->PlayDamageRegionSound(mId);
        }
    } while (iCount);
}

// FUNCTION: SWBF2 0x0048a0b0
void DamageRegion::ClearList()
{
    sList.RemoveAll();
}

// FUNCTION: SWBF2 0x0048a260
template void PblList<DamageRegion>::RemoveAll();

// FUNCTION: SWBF2 0x0048a0c0
void DamageRegion::UpdateAll(float fDeltaTime)
{
    if (fDeltaTime != 0.0f)
    {
        for (PblList<DamageRegion>::Itor it(sList); (DamageRegion*)it; ++it)
        {
            it->Update(fDeltaTime);
        }
    }
}

// FUNCTION: SWBF2 0x0048a110
void DamageRegion::Create()
{
    mUpdateThread = new UpdateThread;
    mUpdateThread->ActivateThread(0, PER_FRAME);
}

// SYNTHETIC: SWBF2 0x0048a150
// DamageRegion::UpdateThread::`scalar deleting destructor'

// FUNCTION: SWBF2 0x0048a180
void DamageRegion::Destroy()
{
    ClearList();
    mUpdateThread = 0;
}

// FUNCTION: SWBF2 0x0048a190
void* DamageRegionFactory::CreateRegion(RedRegionDesc& desc, const char* pcName) const
{
    DamageRegion::Damage damage;
    memset(&damage, 0, sizeof(damage));
    ProcessRegionName(pcName, &damage);
    return new DamageRegion(desc, damage);
}

// FUNCTION: SWBF2 0x0048a220
void DamageRegionFactory::ReadParameter(const char* pcRegionName, const char* pcParameter,
                                        unsigned int uiIndex, unsigned int uiCount,
                                        const char* pcValue, unsigned int uiValueIndex,
                                        void* pDesc) const
{
    unsigned int uiParam = GetParameterIndex(sDamageParameterNames, 8, pcParameter, uiIndex);
    if (uiParam)
    {
        sscanf(pcValue, "%f", (float*)pDesc + (uiParam - 1));
    }
}
