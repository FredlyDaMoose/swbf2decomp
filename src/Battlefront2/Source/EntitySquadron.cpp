#include "EntitySquadron.h"
#include "EntityFormation.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblListDoubleSorted.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Util/RedStack.h"

#include <string.h>

// FUNCTION: SWBF2 0x004fc6e0
EntitySquadronClass::EntitySquadronClass()
{
    mNumFormations = 0;
    mSlideTime = 5.0f;
    mMember = 0;
}

const ODFPropertySet::ODFProperty EntitySquadronClass::sPropertySet[1] = {
    { 0x7adbb520, ODFPropertySet::type_Float, offsetof(EntitySquadronClass, mSlideTime) },
};

// FUNCTION: SWBF2 0x004fc700
bool EntitySquadronClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    switch (uProperty)
    {
    case 0x959feb6b:
        mMember = PblHash(pszValue);
        return true;
    case 0xe5108b93:
        mFormations[mNumFormations] = PblHash(pszValue);
        mNumFormations++;
        return true;
    default:
        return ODFPropertySet::SetProperty(sPropertySet, sizeof(sPropertySet) / sizeof(sPropertySet[0]),
                                           this, uProperty, pszValue);
    }
}

// FUNCTION: SWBF2 0x004fbe20
EntitySquadron::EntitySquadron()
    : mFormation(0)
    , mFormationSlot(0)
    , mLeader(0)
    , mTime(0.0f)
    , mOffset(0.0f, 0.0f, 0.0f)
{
}

// FUNCTION: SWBF2 0x004fbea0
EntitySquadron::~EntitySquadron()
{
    if (InSquadron())
    {
        RemoveFromSquadron();
    }

    if (mNode.IsAttached())
    {
        mNode.Remove();
    }
}

// FUNCTION: SWBF2 0x004fbf20
bool EntitySquadron::CreateSquadron(unsigned int uFormation)
{
    const EntityFormation* pFormation = GetEntitySquadronClass()->SelectFormation(uFormation);
    if (!pFormation)
    {
        return false;
    }

    mFormation = const_cast<EntityFormation*>(pFormation);
    mLeader = this;
    return AddToSquadron(this, mFormation->RootIsSlot() ? mFormation->GetRootIndex() : -1);
}

// FUNCTION: SWBF2 0x004fbf80
bool EntitySquadron::IsSquadronLeader() const
{
    return GetSquadronLeader() == this;
}

// FUNCTION: SWBF2 0x004fbf90
void EntitySquadron::SetSquadronLeader(EntitySquadron* pLeader)
{
    pLeader->mNode.Remove();
    pLeader->mLeader = pLeader;
    pLeader->mList.AddLast(&pLeader->mNode);

    int i = 0;
    PblListDouble::Node* pNode = mList.mTerminator.mNext;
    while (pNode != &mList.mTerminator && i < 10)
    {
        EntitySquadron* pMember = pNode == &mList.mTerminator
                                      ? 0
                                      : (EntitySquadron*)((char*)pNode - offsetof(EntitySquadron, mNode));
        pNode = pNode->mNext;

        if (pMember != pLeader)
        {
            pMember->mNode.Remove();
            pLeader->mList.AddLast(&pMember->mNode);
            pMember->mLeader = pLeader;
        }
        i++;
    }
}

// FUNCTION: SWBF2 0x004fc050
EntitySquadron* EntitySquadron::GetSquadronLeader() const
{
    return mLeader;
}

// FUNCTION: SWBF2 0x004fc060
bool EntitySquadron::InSquadron() const
{
    return GetFormation() != 0;
}

// FUNCTION: SWBF2 0x004fc070
const EntityFormation* EntitySquadron::GetFormation() const
{
    return mFormation;
}

// FUNCTION: SWBF2 0x004fc080
void EntitySquadron::UpdatePosition(const EntitySquadronClass* const pClass, float fElapsed,
                                    const PblMatrix& matrix, PblVector3& vPosition)
{
    float fSlideTime = pClass->mSlideTime;
    if (mFormation && mFormationSlot != mFormation->GetRootIndex())
    {
        PblVector3 vSlot;
        mFormation->CalculatePoint(mFormationSlot, matrix, vSlot);
        if (fSlideTime > mTime)
        {
            float fT = mTime / fSlideTime;
            mTime += fElapsed;
            mOffset = Interpolate(mOffset, vSlot, fT);
        }
        else
        {
            mOffset = vSlot;
        }
        vPosition.x += mOffset.x;
        vPosition.y += mOffset.y;
        vPosition.z += mOffset.z;
    }
    else if (fSlideTime > mTime)
    {
        mOffset = Interpolate(mOffset, PblVector3(0.0f, 0.0f, 0.0f), mTime / fSlideTime);
        vPosition.x += mOffset.x;
        vPosition.y += mOffset.y;
        vPosition.z += mOffset.z;
        mTime += fElapsed;
    }
}

// FUNCTION: SWBF2 0x004fc210
int EntitySquadron::GetFormationSlot() const
{
    return mFormationSlot;
}

// FUNCTION: SWBF2 0x004fc220
bool EntitySquadron::AddToSquadron(EntitySquadron* const pMember, int iSlot)
{
    RedStack stack;
    const EntitySquadronClass* pClass = pMember->GetEntitySquadronClass();
    if (!pClass->FormationSupported(mFormation->GetName()))
    {
        return false;
    }

    const unsigned int uNumSlots = mFormation->GetNumPoints();
    unsigned int uUsed = 0;
    bool* pbUsed = (bool*)stack.Alloc(uNumSlots);
    memset(pbUsed, 0, uNumSlots);
    if (!mFormation->RootIsSlot())
    {
        uUsed = 1;
        pbUsed[mFormation->GetRootIndex()] = true;
    }

    int i = 0;
    for (PblListDouble::Node* pNode = mList.mTerminator.mNext; pNode != &mList.mTerminator; pNode = pNode->mNext)
    {
        EntitySquadron* pOther = (EntitySquadron*)((char*)pNode - offsetof(EntitySquadron, mNode));
        ++i;
        ++uUsed;
        pbUsed[pOther->mFormationSlot] = true;
        if (i > 10)
        {
            break;
        }
    }

    if (uUsed >= uNumSlots)
    {
        return false;
    }

    if (iSlot >= 0)
    {
        if (pbUsed[iSlot] && mFormation->MemberIsSupported(pClass->mMember, iSlot))
        {
            return false;
        }
    }
    else
    {
        bool bSupported = false;
        for (iSlot = 0; (unsigned int)iSlot < uNumSlots; ++iSlot)
        {
            if (!pbUsed[iSlot])
            {
                bSupported = mFormation->MemberIsSupported(pClass->mMember, iSlot);
                if (bSupported)
                {
                    break;
                }
            }
        }
        if (!bSupported)
        {
            return false;
        }
    }

    pMember->mFormationSlot = iSlot;
    mList.AddLast(&pMember->mNode);
    pMember->mFormation = mFormation;
    pMember->mLeader = this;
    pMember->mTime = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x004fc3a0
void EntitySquadron::RemoveFromSquadron()
{
    if (IsSquadronLeader())
    {
        class EntityDistance
        {
        public:
            void* operator new(size_t, void* pMemory) { return pMemory; }
            void operator delete(void*, void*) {}

            EntitySquadron* mEntity;
            PblListDoubleSorted::Node mNode;
        };

        RedStack stack;
        PblListDoubleSorted members;
        int iNumSlots = mFormation->GetNumSlots();
        EntityDistance* pDistances = (EntityDistance*)stack.Alloc(iNumSlots * sizeof(EntityDistance));
        for (int i = 0; i < iNumSlots; ++i)
        {
            new (&pDistances[i]) EntityDistance;
        }

        const PblVector3& vLeader = mFormation->GetPointPosition(mFormationSlot);
        EntityDistance* pDistance = pDistances;
        unsigned int uCount = 0;
        int i = 0;
        for (PblListDouble::Node* pNode = mList.mTerminator.mNext; pNode != &mList.mTerminator;
             pNode = pNode->mNext, ++i)
        {
            if (i >= 10)
            {
                break;
            }
            EntitySquadron* pMember = pNode == &mList.mTerminator
                                          ? 0
                                          : (EntitySquadron*)((char*)pNode - offsetof(EntitySquadron, mNode));
            if (pMember != this)
            {
                pDistance->mEntity = pMember;
                const PblVector3& vSlot = mFormation->GetPointPosition(pMember->mFormationSlot);
                pDistance->mNode.mSortValue = (vLeader.y - vSlot.y) * (vLeader.y - vSlot.y) +
                                              (vLeader.x - vSlot.x) * (vLeader.x - vSlot.x) +
                                              (vLeader.z - vSlot.z) * (vLeader.z - vSlot.z);
                members.Add(&pDistance->mNode);
                ++uCount;
                ++pDistance;
            }
        }

        if (!uCount)
        {
            mLeader = 0;
            RemoveFromSquadron();
            return;
        }

        EntityDistance* pFirst =
            (EntityDistance*)members.GetFirst()->GetObjectA(&members, offsetof(EntityDistance, mNode));
        mLeader->mLeader = 0;
        i = 0;
        for (PblListDouble::Node* pNode = mList.mTerminator.mNext; pNode != &mList.mTerminator && i < 10;)
        {
            EntitySquadron* pMember = pNode == &mList.mTerminator
                                          ? 0
                                          : (EntitySquadron*)((char*)pNode - offsetof(EntitySquadron, mNode));
            pNode = pNode->mNext;
            ++i;
            pMember->RemoveFromSquadron();
        }

        if (uCount > 1)
        {
            EntitySquadron* pNewLeader = pFirst->mEntity;
            if (pNewLeader->CreateSquadron(uCount))
            {
                for (PblListDouble::Node* pLink = members.GetFirst()->mNode.mNext; pLink != &members.mTerminator;
                     pLink = pLink->mNext)
                {
                    PblListDoubleSorted::Node* pNode =
                        (PblListDoubleSorted::Node*)((char*)pLink - offsetof(PblListDoubleSorted::Node, mNode));
                    if (pNode == members.GetTerminator())
                    {
                        break;
                    }
                    pNewLeader->AddToSquadron(
                        ((EntityDistance*)pNode->GetObjectA(&members, offsetof(EntityDistance, mNode)))->mEntity, -1);
                }
            }
        }
        members.Clear();
        return;
    }

    EntitySquadron* pLeader = mLeader;
    mLeader = 0;
    mFormation = 0;
    mFormationSlot = 0;
    mTime = 0.0f;
    mNode.Remove();
    if (pLeader && pLeader->InSquadron() && pLeader->mList.GetCount() <= 1)
    {
        pLeader->RemoveFromSquadron();
    }
}

// FUNCTION: SWBF2 0x004fc8b0
bool EntitySquadronClass::FormationSupported(unsigned int uFormation) const
{
    for (unsigned int i = 0; i < mNumFormations; ++i)
    {
        if (mFormations[i] == uFormation)
        {
            return true;
        }
    }
    return false;
}

// FUNCTION: SWBF2 0x004fc770
const EntityFormation* EntitySquadronClass::SelectFormation(unsigned int uNumSlots) const
{
    RedStack stack;
    const EntityFormation** ppFormations = (const EntityFormation**)stack.Alloc(mNumFormations);
    int iNumFound = 0;
    for (unsigned int i = 0; i < mNumFormations; ++i)
    {
        const EntityFormation* pFormation = EntityFormation::Find(mFormations[i]);
        if (pFormation && (unsigned int)pFormation->GetNumSlots() == uNumSlots)
        {
            ppFormations[iNumFound++] = pFormation;
        }
    }
    if (!iNumFound)
    {
        return 0;
    }

    float fIndex = PblRandom::GetGlobalFloat() * (float)iNumFound;
    float fMax = (float)iNumFound - 1.0f;
    if (0.0f > fIndex)
    {
        fIndex = 0.0f;
    }
    else if (fIndex > fMax)
    {
        fIndex = fMax;
    }
    return ppFormations[(int)fIndex];
}
