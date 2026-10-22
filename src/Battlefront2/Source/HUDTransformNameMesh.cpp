#include <stdlib.h>

#include "HUDTransformNameMesh.h"
#include "RedEngineFL/Graphics/RedModel.h"

HUD::TransformNameMesh::Factory::Factory()
    : HUD::Item::Factory(0x67c3c715u)
{
    mName = "TransformNameMesh";
}

// GLOBAL: SWBF2 0x007ebabc
// SYNTHETIC: SWBF2 0x004040c0
// `dynamic initializer for 'HUD::TransformNameMesh::sTransformNameMeshList''
// SYNTHETIC: SWBF2 0x00768c30
// `dynamic atexit destructor for 'HUD::TransformNameMesh::sTransformNameMeshList''
PblListSingle HUD::TransformNameMesh::sTransformNameMeshList;

static const unsigned int kNameMeshHash = 0x63cc538f;
static const unsigned int kTransformNameMeshHash = 0x67c3c715;

static const unsigned int kMaxMappings = 256;
static const unsigned int kMaxInheritNames = 16;

// FUNCTION: SWBF2 0x005673c0
HUD::TransformNameMesh::NameMesh::NameMesh()
{
    mDisplayedWarning = false;
    mInherited = false;
    mMeshIsID = false;
    mHashID = 0;
    mName = 0;
    mMeshName = 0;
    mMesh.mesh = 0;
}

// FUNCTION: SWBF2 0x005673f0
HUD::TransformNameMesh::NameMesh::~NameMesh()
{
    HUD::FreeName(&mName);
    HUD::FreeName(&mMeshName);
}

// FUNCTION: SWBF2 0x00567410
void HUD::TransformNameMesh::NameMesh::Init(const char* pName, const char* pMeshName)
{
    mMesh.meshID = PblHash(pMeshName);
    mMeshIsID = true;
    mHashID = PblHash(pName);
    GetMesh(pName);
    HUD::CopyName(&mName, pName, 1);
    HUD::CopyName(&mMeshName, pMeshName, 1);
}

// FUNCTION: SWBF2 0x00567470
void HUD::TransformNameMesh::NameMesh::Init(unsigned int uHashID, unsigned int uMeshID,
                                            bool bInherited)
{
    mHashID = uHashID;
    mMesh.meshID = uMeshID;
    mInherited = bInherited;
    mMeshIsID = true;
}

// FUNCTION: SWBF2 0x005674a0
const RedModel* HUD::TransformNameMesh::NameMesh::GetMesh(const char* const)
{
    if (!mMeshIsID)
    {
        return mMesh.mesh;
    }

    RedModel* pMesh = RedModel::_HashTable.Find(mMesh.meshID);
    if (pMesh)
    {
        mMeshIsID = false;
        mMesh.mesh = pMesh;
        return pMesh;
    }
    if (mName && mMeshName && !mDisplayedWarning)
    {
        mDisplayedWarning = true;
    }
    return 0;
}

// FUNCTION: SWBF2 0x00567500
unsigned int HUD::TransformNameMesh::NameMesh::GetNameHash() const
{
    return mHashID;
}

// FUNCTION: SWBF2 0x00567510
unsigned int HUD::TransformNameMesh::NameMesh::GetMeshNameHash() const
{
    if (mMeshIsID)
    {
        return mMesh.meshID;
    }
    return mMesh.mesh->GetName();
}

// FUNCTION: SWBF2 0x00567520
const char* HUD::TransformNameMesh::NameMesh::GetName() const
{
    return mName;
}

// FUNCTION: SWBF2 0x00567530
const char* HUD::TransformNameMesh::NameMesh::GetMeshName() const
{
    return mMeshName;
}

// FUNCTION: SWBF2 0x00567540
bool HUD::TransformNameMesh::NameMesh::IsInherited() const
{
    return mInherited;
}

// FUNCTION: SWBF2 0x00567550
HUD::TransformNameMesh::NameMesh& HUD::TransformNameMesh::NameMesh::operator=(NameMesh& other)
{
    mHashID = other.mHashID;
    mMesh = other.mMesh;
    mMeshIsID = other.mMeshIsID;
    mName = other.mName;
    mMeshName = other.mMeshName;
    other.mName = 0;
    other.mMeshName = 0;
    mInherited = other.mInherited;
    return *this;
}

// FUNCTION: SWBF2 0x005675a0
int __cdecl HUD::TransformNameMesh::NameMesh::CompareSort(const void* p1, const void* p2)
{
    unsigned int uHash1 = ((const NameMesh*)p1)->mHashID;
    unsigned int uHash2 = ((const NameMesh*)p2)->mHashID;
    if (uHash1 > uHash2)
    {
        return 1;
    }
    return uHash1 < uHash2 ? -1 : 0;
}

// FUNCTION: SWBF2 0x005675c0
int __cdecl HUD::TransformNameMesh::NameMesh::CompareSearch(const void* p1, const void* p2)
{
    unsigned int uHash1 = ((const NameMesh*)p1)->mHashID;
    unsigned int uHash2 = ((const NameMesh*)p2)->mHashID;
    if (uHash1 > uHash2)
    {
        return 1;
    }
    return uHash1 < uHash2 ? -1 : 0;
}

// SYNTHETIC: SWBF2 0x00567640
// HUD::TransformNameMesh::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00567670
HUD::TransformNameMesh::~TransformNameMesh()
{
    if (mMapping)
    {
        delete[] mMapping;
        mNumMappings = 0;
    }
    for (unsigned int i = 0; i < mNumInheritNames; ++i)
    {
        HUD::FreeName(&mInheritNames[i]);
    }
    if (mTransformNameMeshNode.mNext)
    {
        mTransformNameMeshNode.Remove();
    }
}

// FUNCTION: SWBF2 0x00567730
void HUD::TransformNameMesh::Read(PblConfig& config, Factory::ItemCallback pCallback,
                                  void* const pUserData)
{
    NameMesh aMapping[kMaxMappings];
    mMapping = aMapping;
    Item::Read(config, pCallback, pUserData);
    if (mNumMappings)
    {
        mMapping = new NameMesh[mNumMappings];
        if (mMapping)
        {
            qsort(aMapping, mNumMappings, sizeof(NameMesh), NameMesh::CompareSort);
            for (unsigned int i = 0; i < mNumMappings; ++i)
            {
                mMapping[i] = aMapping[i];
            }
        }
    }
    else
    {
        mMapping = 0;
    }
}

// FUNCTION: SWBF2 0x00567880
bool HUD::TransformNameMesh::ReadData(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case kNameMeshHash:
        if (mNumMappings < kMaxMappings)
        {
            mMapping[mNumMappings++].Init(data.GetStringArg(0), data.GetStringArg(1));
        }
        return true;
    case kTransformNameMeshHash:
    {
        TransformNameMesh* pInherit = FindByHashID(PblHash(data.GetStringArg(0)));
        if (pInherit)
        {
            for (unsigned int i = 0; i < pInherit->mNumMappings; ++i)
            {
                if (mNumMappings >= kMaxMappings)
                {
                    break;
                }
                const NameMesh& inherited = pInherit->mMapping[i];
                mMapping[mNumMappings++].Init(inherited.GetNameHash(),
                                              inherited.GetMeshNameHash(), true);
            }
            if (mNumInheritNames < kMaxInheritNames)
            {
                HUD::CopyName(&mInheritNames[mNumInheritNames++], pInherit->GetName(), 1);
            }
        }
        return true;
    }
    }
    return Transform::ReadData(config, data, EventClass::type_Model);
}

// FUNCTION: SWBF2 0x005679b0
void HUD::TransformNameMesh::WriteData(HUD::File& file, unsigned int uTabs)
{
    for (unsigned int i = 0; i < mNumMappings; ++i)
    {
        const NameMesh& mapping = mMapping[i];
        if (!mapping.IsInherited())
        {
            file.WriteTabs(uTabs);
            file.WriteStringf("NameMesh(\"%s\", \"%s\")\n", mapping.GetName(),
                              mapping.GetMeshName());
        }
    }
    for (unsigned int i = 0; i < mNumInheritNames; ++i)
    {
        file.WriteProperty("TransformNameMesh", uTabs, mInheritNames[i], "");
    }
    Transform::WriteData(file, uTabs);
}

// FUNCTION: SWBF2 0x00567b40
HUD::TransformNameMesh* HUD::TransformNameMesh::FindByHashID(unsigned int uHashID)
{
    for (PblListSingle::Node* pNode = sTransformNameMeshList.mHead.mNext;
         pNode != &sTransformNameMeshList.mHead; pNode = pNode->mNext)
    {
        TransformNameMesh* pItem = (TransformNameMesh*)((char*)pNode -
            offsetof(TransformNameMesh, mTransformNameMeshNode));
        if (pItem->GetNameHash() == uHashID)
        {
            return pItem;
        }
    }
    return 0;
}

// FUNCTION: SWBF2 0x005675e0
HUD::TransformNameMesh::TransformNameMesh(const char* const pName,
                                          Factory::ItemCallback pCallback,
                                          void* const pUserData)
    : HUD::Transform(pName, pCallback, pUserData),
      mMapping(0),
      mNumMappings(0),
      mNumInheritNames(0)
{
    sTransformNameMeshList.AddFirst(&mTransformNameMeshNode);
    mEventInput.Init(EventInput, this);
}

// FUNCTION: SWBF2 0x00567a60
HUD::TransformNameMesh::NameMesh* HUD::TransformNameMesh::FindNameMesh(unsigned int uHashID)
{
    NameMesh key;
    key.Init(uHashID, 0, false);
    return (NameMesh*)bsearch(&key, mMapping, mNumMappings, sizeof(NameMesh), NameMesh::CompareSearch);
}

// FUNCTION: SWBF2 0x00567ab0
void HUD::TransformNameMesh::EventInput(const HUD::Event& event, void* const pData)
{
    unsigned int uHashID = 0;
    if (event.GetClass()->GetType() == EventClass::type_Uint)
    {
        uHashID = event.GetData().uintValue;
    }

    TransformNameMesh* pTransform = (TransformNameMesh*)pData;
    if (!pTransform->mEventClassOutput || !uHashID)
    {
        return;
    }

    const RedModel* pMesh = 0;
    if (pTransform->mNumMappings)
    {
        NameMesh* pNameMesh = pTransform->FindNameMesh(uHashID);
        if (pNameMesh)
        {
            pMesh = pNameMesh->GetMesh(0);
        }
    }
    if (!pMesh)
    {
        pMesh = RedModel::_HashTable.Find(uHashID);
    }
    if (pMesh)
    {
        HUD::Event(pTransform->mEventClassOutput, pMesh).Send();
    }
}
