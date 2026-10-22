#include <math.h>
#include "PebbleFL/Common/PblVector.h"
#include <stdio.h>

#include "Cloth.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Memory/RedMemory.h"
#include "PebbleFL/Common/PblMath.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Util/ext/hashtable.h"
#include "RedEngineFL/Util/ext/ListAllocator.h"
#include <string.h>
#include "PebbleFL/Common/PblRef.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Graphics/RedModel.h"
#include "RedEngineFL/Graphics/RedSkeleton.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedLight.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"
#include "NetGame.h"

#include "PebbleFL/Common/PblRegion.h"
#include "SoundFL/source/win/sndengine.h"
#include "PebbleFL/Common/PblTime.h"
#include "FLInputManager.h"
#include "GameVoiceChat.h"
#include "GameVoiceChatNode.h"
#include "GameVoiceChatUtil.h"
#include "VoiceChatNode.h"
#include "VoiceChatTransport.h"
#include "Battlefront2/Platform/Pc/pcSpawnDisplay.h"
#include "RedEngineFL/input/RedKeyboard.h"
#include "SoundFL/source/sndthreadupdate.h"
#include "BootDisplay.h"
#include "BoundaryManager.h"
#include "CaptureDisplay.h"
#include "CollisionMesh.h"
#include "EntityHologram.h"
#include "GameLoop.h"
#include "GrassPatch.h"
#include "LeafPatch.h"
#include "LockOnManager.h"
#include "LuaScript.h"
#include "MessageDisplay.h"
#include "MuzzleFlash.h"
#include "OrdnanceGrapplingHook.h"
#include "OrdnanceTowCable.h"
#include "ParticleEmitter.h"
#include "ParticleEmitterObject.h"
#include "PatrolPath.h"
#include "PauseMenu.h"
#include "ScreenTransition.h"
#include "SkyDome.h"
#include "SpectatorDisplay.h"
#include "StatIconDisplay.h"
#include "Tracker.h"
#include "VehicleSpawn.h"
#include "Battlefront2/Platform/Pc/pcGrassPatch.h"
#include "Battlefront2/Platform/Pc/pcLeafPatch.h"
#include "RedEngineFL/Application/pc/pcRedShadowProxy.h"
#include "RedEngineFL/Application/RedTerrain.h"
#include "RedEngineFL/Interface/RedGlyphCacheElement.h"
#include "RedEngineFL/Interface/RedInterfaceElement.h"
#include "RedEngineFL/Util/RedLocalizeDB.h"
#include "RedEngineFL/Util/ext/IAllocator.h"
#include "CollisionManager.h"
#include "CommandLine.h"
#include "GameMovie.h"
#include "ProceduralAnimation.h"
#include "ShellLoop.h"
#include "RedEngineFL/Interface/RedShadowBlob.h"

// GLOBAL: SWBF2 0x01eb1080
// SYNTHETIC: SWBF2 0x004023e0
// `dynamic initializer for 'EntityCloth::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766df0
// `dynamic atexit destructor for 'EntityCloth::sMemoryPool''
MemoryPool EntityCloth::sMemoryPool("EntityCloth", 0x120);
// GLOBAL: SWBF2 0x01eb10d0
// SYNTHETIC: SWBF2 0x004023c0
// `dynamic initializer for 'ClothData::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766de0
// `dynamic atexit destructor for 'ClothData::sMemoryPool''
MemoryPool ClothData::sMemoryPool("ClothData", 0xa0);

// GLOBAL: SWBF2 0x01eb1120
int ClothData::s_clothDataHashTableNumEntries;
// GLOBAL: SWBF2 0x01eb1124
// SYNTHETIC: SWBF2 0x00402400
// `dynamic initializer for 'ClothData::s_clothDataHashTable''

PblHashTable<ClothData, 64> ClothData::s_clothDataHashTable;

// FUNCTION: SWBF2 0x00459010
void ClothData::Init()
{
    EntityCloth::sMemoryPool.Setup(0x10, EntityCloth::sMemoryPool.mSize,
                                   EntityCloth::sMemoryPool.mGrow);
    ClothData::sMemoryPool.Setup(0x20, ClothData::sMemoryPool.mSize,
                                 ClothData::sMemoryPool.mGrow);

    s_clothDataHashTable.Clear();
    s_clothDataHashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x00459370
void ClothData::_Read(PblFileChunk& chunk)
{
    m_uiNumCollisionVolumes = 0;
    while (chunk.HasMoreChildren())
    {
        PblFileChunk child = chunk.ReadNextChild();
        PblFile* pFile = child._pFile;
        char acString[128];
        switch (child._iId)
        {
        case 0x4D524658:
            for (int r = 0; r < 4; r++)
            {
                float fX = pFile->ReadFloat();
                float fY = pFile->ReadFloat();
                float fZ = pFile->ReadFloat();
                m_Transform.m[r][0] = fX;
                m_Transform.m[r][1] = fY;
                m_Transform.m[r][2] = fZ;
                m_Transform.m[r][3] = (r == 3) ? 1.0f : 0.0f;
            }
            break;
        case 0x41544144:
        {
            child.ReadString(acString, sizeof(acString));
            m_uiTextureNameHash = PblHash(acString);
            m_uiNumParticles = pFile->Read32();
            m_pPos = (PblVector3*)RedAllocFromHeap(__RedCurrHeap, m_uiNumParticles * sizeof(PblVector3), 0);
            m_pTexCoords = (PblVector3*)RedAllocFromHeap(__RedCurrHeap, m_uiNumParticles * sizeof(PblVector3), 0);
            for (unsigned int i = 0; i < m_uiNumParticles; i++)
            {
                float fX = pFile->ReadFloat();
                float fY = pFile->ReadFloat();
                float fZ = pFile->ReadFloat();
                m_pPos[i].x = fX;
                m_pPos[i].y = fY;
                m_pPos[i].z = fZ;
            }
            for (unsigned int i = 0; i < m_uiNumParticles; i++)
            {
                float fU = pFile->ReadFloat();
                float fV = pFile->ReadFloat();
                m_pTexCoords[i].x = fU;
                m_pTexCoords[i].y = fV;
                m_pTexCoords[i].z = 0.0f;
            }
            m_uiNumFixedPoints = pFile->Read32();
            if (pFile->Read32())
            {
                m_pFixedPointsBones = (unsigned int*)RedAllocFromHeap(__RedCurrHeap, m_uiNumFixedPoints * sizeof(unsigned int), 0);
                for (unsigned int i = 0; i < m_uiNumFixedPoints; i++)
                {
                    child.ReadString(acString, sizeof(acString));
                    m_pFixedPointsBones[i] = PblTEMPHash(acString);
                }
            }
            else
            {
                m_pFixedPointsBones = 0;
            }
            m_uiNumTriangles = pFile->Read32();
            m_pTriangleIndices = (unsigned short*)RedAllocFromHeap(__RedCurrHeap, m_uiNumTriangles * 3 * sizeof(unsigned short), 0);
            for (unsigned int i = 0; i < m_uiNumTriangles; i++)
            {
                m_pTriangleIndices[i * 3 + 0] = (unsigned short)pFile->Read32();
                m_pTriangleIndices[i * 3 + 1] = (unsigned short)pFile->Read32();
                m_pTriangleIndices[i * 3 + 2] = (unsigned short)pFile->Read32();
            }
            m_uiNumStretchPairs = pFile->Read32();
            m_pStretchPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumStretchPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumStretchPairs; i++)
            {
                m_pStretchPairs[i].i0 = (unsigned short)pFile->Read32();
                m_pStretchPairs[i].i1 = (unsigned short)pFile->Read32();
            }
            m_uiNumBendPairs = pFile->Read32();
            m_pBendPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumBendPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumBendPairs; i++)
            {
                m_pBendPairs[i].i0 = (unsigned short)pFile->Read32();
                m_pBendPairs[i].i1 = (unsigned short)pFile->Read32();
            }
            m_uiNumCrossPairs = pFile->Read32();
            m_pCrossPairs = (ClothConstraint*)RedAllocFromHeap(__RedCurrHeap, m_uiNumCrossPairs * sizeof(ClothConstraint), 0);
            for (unsigned int i = 0; i < m_uiNumCrossPairs; i++)
            {
                m_pCrossPairs[i].i0 = (unsigned short)pFile->Read32();
                m_pCrossPairs[i].i1 = (unsigned short)pFile->Read32();
            }
            break;
        }
        case 0x454D414E:
            child.ReadString(acString, sizeof(acString));
            m_uiNameHash = PblHash(acString);
            break;
        case 0x544E5250:
            child.ReadString(acString, sizeof(acString));
            m_uiParentHash = PblTEMPHash(acString);
            break;
        case 0x4F464E49:
            child.ReadString(acString, sizeof(acString));
            m_uiModelHash = PblHash(acString);
            break;
        case 0x4C4C4F43:
        {
            m_uiNumCollisionVolumes = pFile->Read32();
            m_pCollisionVolumes = new ClothCollision[m_uiNumCollisionVolumes];
            for (unsigned int i = 0; i < m_uiNumCollisionVolumes; i++)
            {
                char acParent[128];
                child.ReadString(acParent, sizeof(acParent));
                ClothCollision& coll = m_pCollisionVolumes[i];
                coll.parent = PblTEMPHash(acParent);
                coll.type = (ClothCollisionType)pFile->Read32();
                coll.width = pFile->ReadFloat();
                coll.height = pFile->ReadFloat();
                coll.depth = pFile->ReadFloat();
                for (int r = 0; r < 4; r++)
                {
                    float fX = pFile->ReadFloat();
                    float fY = pFile->ReadFloat();
                    float fZ = pFile->ReadFloat();
                    m_pCollisionVolumes[i].localMat.m[r][0] = fX;
                    m_pCollisionVolumes[i].localMat.m[r][1] = fY;
                    m_pCollisionVolumes[i].localMat.m[r][2] = fZ;
                    m_pCollisionVolumes[i].localMat.m[r][3] = (r == 3) ? 1.0f : 0.0f;
                }
            }
            break;
        }
        }
    }
}

// FUNCTION: SWBF2 0x00459ab0
EntityClothClass::EntityClothClass(unsigned int uHash)
    : EntityClass(uHash)
{
    if (mNetIndex != -1)
    {
        --sCounter;
        mNetIndex = -1;
    }

    mModelHash = 0;
    mWindDirection.x = 1.0f;
    mWindDirection.y = 0.0f;
    mWindDirection.z = 0.0f;
    mWindSpeed = 10.0f;
    mDampening = 0.5f;
    mDrag = -2.0f;
    mParticleMass = 1.0f;
    mMaxAcceleration = 20.0f;
    mMaxVelocity = 10.0f;
    mPriority = 0.0f;
    m_uiDoCrossConstraints = 1;
    m_uiDoBendConstraints = 1;
    m_uiDoStretchConstraints = 1;
    m_uiHardEdge = 0;
    m_uiTransparent = 0;
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x007e5eee
bool enabled_ = true;

typedef util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> > ClothDataT;

// GLOBAL: SWBF2 0x01eb1330
// SYNTHETIC: SWBF2 0x00402420
// `dynamic initializer for 'ClothBE::clothData_''
// SYNTHETIC: SWBF2 0x00766e00
// `dynamic atexit destructor for 'ClothBE::clothData_''
ClothDataT clothData_;
}

// FUNCTION: SWBF2 0x00458cb0
template ClothBE::ClothDataT::~HashTable();

// SYNTHETIC: SWBF2 0x0045c1f0
// util::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >::HashTable<ClothBank *,unsigned int,util::TNewArrayAlloc<ClothBank *> >

// FUNCTION: SWBF2 0x0045c2b0
template void util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                               ClothBE::ClothDataT::KeyVecT,
                               ClothBE::ClothDataT::HashVecT>::Clear();

// TEMPLATE: SWBF2 0x0045c660
// util::BaseArray<ClothBank *,util::TypedArrayAllocator<ClothBank *,util::TNewArrayAlloc<ClothBank *> > >::Reserve_

// TEMPLATE: SWBF2 0x0045c720
// util::BaseArray<unsigned int,util::TypedArrayAllocator<unsigned int,util::TNewArrayAlloc<ClothBank *> > >::Reserve_

// GLOBAL: SWBF2 0x01eb0fcc
// SYNTHETIC: SWBF2 0x004023a0
// `dynamic initializer for 'EntityCloth::rttiHashEntityCloth''
PblHash EntityCloth::rttiHashEntityCloth("EntityCloth");
char EntityCloth::rttiNameEntityCloth[] = "EntityCloth";

// GLOBAL: SWBF2 0x01eb062c
// SYNTHETIC: SWBF2 0x004023b0
// `dynamic initializer for 'EntityClothClass::rttiHashEntityClothClass''
PblHash EntityClothClass::rttiHashEntityClothClass("EntityClothClass");
char EntityClothClass::rttiNameEntityClothClass[] = "EntityClothClass";

// FUNCTION: SWBF2 0x00458ae0
bool EntityCloth::IsRtti(unsigned int uRtti) const
{
    return uRtti == rttiHashEntityCloth || Entity::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00458b10
unsigned int EntityCloth::GetDerivedRtti() const
{
    return rttiHashEntityCloth;
}

// FUNCTION: SWBF2 0x00458b20
const char* EntityCloth::GetDerivedRttiName() const
{
    return rttiNameEntityCloth;
}

// FUNCTION: SWBF2 0x00458b40
EntityClass* EntityCloth::GetEntityClass()
{
    return mClass;
}

// FUNCTION: SWBF2 0x00458b50
const EntityClass* EntityCloth::GetEntityClass() const
{
    return mClass;
}

// FUNCTION: SWBF2 0x00458b60
bool EntityClothClass::IsRtti(unsigned int uRtti)
{
    return uRtti == rttiHashEntityClothClass || EntityClass::IsRtti(uRtti);
}

// FUNCTION: SWBF2 0x00458b90
unsigned int EntityClothClass::GetDerivedRtti()
{
    return rttiHashEntityClothClass;
}

// FUNCTION: SWBF2 0x00458ba0
const char* EntityClothClass::GetDerivedRttiName()
{
    return rttiNameEntityClothClass;
}

// FUNCTION: SWBF2 0x00458bb0
void ClothBank::Push(EntityCloth* pCloth, float fPriority)
{
    if (!Find(pCloth))
    {
        cloth_.Add(pCloth);
    }
}

// FUNCTION: SWBF2 0x00458c10
bool ClothBank::Find(EntityCloth* pCloth)
{
    return cloth_.FindFirst(pCloth) != cloth_.End();
}

// FUNCTION: SWBF2 0x00458c60
bool ClothBank::Remove(EntityCloth* pCloth)
{
    return cloth_.FindRemove(pCloth);
}

EntityCloth* ClothBank::GetTop(float fPriority)
{
    lowPriority_ = 1000000.0f;
    for (int i = 0; i < cloth_.GetSize(); i++)
    {
        if (cloth_[i])
        {
            float fTop = cloth_[i]->GetPriority();
            lowPriority_ = util::Min_(lowPriority_, fTop);
            if (fPriority * 0.98f > fTop)
            {
                EntityCloth* pTop = cloth_[i];
                cloth_[i] = cloth_.End()[-1];
                cloth_._numElements--;
                return pTop;
            }
        }
    }
    lowPriority_ = 0.0f;
    return 0;
}

// FUNCTION: SWBF2 0x00458d60
bool EntityCloth::IsEnabled()
{
    return ClothBE::enabled_;
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x007e5efc
unsigned int clothBaseMem_ = 0x8000;
// GLOBAL: SWBF2 0x007e5f00
unsigned int clothMaxMem_ = 0x8000;
extern util::IAllocator* gMultiAllocator_;
}

// FUNCTION: SWBF2 0x00458d70
void EntityCloth::SetEnable(bool bEnable)
{
    ClothBE::enabled_ = bEnable;
}

// FUNCTION: SWBF2 0x00458d80
void EntityCloth::SetMemUseage(unsigned int uBaseMem, unsigned int uMaxMem)
{
    ClothBE::clothBaseMem_ = uBaseMem;
    ClothBE::clothMaxMem_ = uMaxMem;
    if (GetSpecialAllocator())
    {
        GetSpecialAllocator()->SetMaxMem(uMaxMem);
    }
}

// FUNCTION: SWBF2 0x00458db0
util::IAllocator* EntityCloth::GetSpecialAllocator()
{
    if (ClothBE::clothBaseMem_ + ClothBE::clothMaxMem_ < 0x1000)
    {
        return 0;
    }
    if (!ClothBE::gMultiAllocator_)
    {
        ClothBE::gMultiAllocator_ = util::ListAllocator::Create(ClothBE::clothBaseMem_, ClothBE::clothMaxMem_);
    }
    return ClothBE::gMultiAllocator_;
}

// SYNTHETIC: SWBF2 0x00459b50
// EntityClothClass::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00459b80
EntityClothClass::~EntityClothClass()
{
}

static const PblVector3 RotateY(const PblVector3& v, float fAngle)
{
    float fSin, fCos;
    PblMath::SinCos(&fSin, &fCos, fAngle);
    return PblVector3(fCos * v.x - fSin * v.z, v.y, fSin * v.x + fCos * v.z);
}

// FUNCTION: SWBF2 0x00459b90
void EntityClothClass::SetProperty(unsigned int uProperty, const char* pszValue)
{
    int iValue;

    switch (uProperty)
    {
    case 0x13901A72:
        sscanf(pszValue, "%f", &mWindSpeed);
        break;
    case 0x415E2B30:
    {
        float fAngleZ = 0.0f;
        float fAngleY = 0.0f;
        sscanf(pszValue, "%f %f", &fAngleZ, &fAngleY);
        fAngleZ *= 0.017453292f;
        mWindDirection = RotateY(PblVector3(-sinf(fAngleZ), cosf(fAngleZ), 0.0f),
                                 fAngleY * 0.017453292f);
        break;
    }
    case 0x4E994B61:
        sscanf(pszValue, "%f", &mMaxAcceleration);
        break;
    case 0x5CAF15E1:
        sscanf(pszValue, "%d", &iValue);
        m_uiHardEdge = iValue > 1 ? 1 : iValue;
        break;
    case 0x6EB957F3:
        sscanf(pszValue, "%f", &mParticleMass);
        break;
    case 0x8311CD99:
        sscanf(pszValue, "%d", &iValue);
        m_uiDoStretchConstraints = iValue > 1 ? 1 : iValue;
        break;
    case 0x94DD79B9:
        sscanf(pszValue, "%d", &iValue);
        m_uiDoBendConstraints = iValue > 1 ? 1 : iValue;
        break;
    case 0x94E4E309:
        sscanf(pszValue, "%f", &mPriority);
        break;
    case 0xAFE34731:
        sscanf(pszValue, "%d", &iValue);
        m_uiTransparent = iValue > 1 ? 1 : iValue;
        break;
    case 0xB88C0D43:
        sscanf(pszValue, "%f", &mDrag);
        break;
    case 0xC1133964:
        sscanf(pszValue, "%f", &mDampening);
        break;
    case 0xDE112858:
        sscanf(pszValue, "%d", &iValue);
        m_uiDoCrossConstraints = (iValue > 1 ? 1 : iValue) != 0;
        break;
    case 0xE90ECACC:
        mModelHash = PblHash(pszValue);
        break;
    }
}

// FUNCTION: SWBF2 0x00459eb0
EntityClass* EntityClothClass::Derive(unsigned int uHash)
{
    return new EntityClothClass(uHash);
}

// FUNCTION: SWBF2 0x00459f10
Entity* EntityClothClass::Build(const EntityDesc& desc)
{
    if (!EntityCloth::IsEnabled())
    {
        return 0;
    }

    EntityCloth::GetSpecialAllocator()->ClearMemHash();

    unsigned int uHashID = 0;
    EntityCloth* pCloth = new EntityCloth(this, desc);
    pCloth->SetPriority(mPriority + desc.mPriority);

    ClothBank* pBank = ClothBE::clothData_[uHashID];
    if (!pBank)
    {
        pBank = new ClothBank(uHashID);
        ClothBE::clothData_[uHashID] = pBank;
    }
    if (pCloth)
    {
        pCloth->bank_ = pBank;
        if (pBank)
        {
            pBank->Push(pCloth, pCloth->GetPriority());
        }
    }
    pCloth->CreateData();
    return pCloth;
}

// FUNCTION: SWBF2 0x0045a090
void EntityCloth::CreateData_()
{
    if (!m_pPos)
    {
        Init(mClass, mDesc);
        if (m_pPos && bank_)
        {
            bank_->Push(this, GetPriority());
        }
    }
}

// FUNCTION: SWBF2 0x0045a0d0
void EntityCloth::CreateData()
{
    float fPriority = GetPriority();
    ClothBank* pBank = bank_;

    CreateData_();
    while (!m_pPos)
    {
        ClearData();
        if (!pBank)
        {
            return;
        }
        EntityCloth* pTop = pBank->GetTop(fPriority);
        if (!pTop)
        {
            return;
        }
        pTop->ClearData();
        CreateData_();
        if (m_pPos)
        {
            return;
        }
        ClearData();
    }
}

// FUNCTION: SWBF2 0x0045a1b0
void EntityCloth::SetPriority(float fPriority)
{
    priority_ = fPriority;
}

// FUNCTION: SWBF2 0x0045a1c0
float EntityCloth::GetPriority()
{
    float fFresh = 1.0f - (float)(RedRenderer::GetFrameNumber() - m_lastFrameVisible) / 30.0f;
    float fFloor = 0.1f;
    return (priority_ + 1.0f) * util::Max_(fFresh, fFloor) * 100.0f;
}

// SYNTHETIC: SWBF2 0x0045cb32
// [thunk]:EntityCloth::`scalar deleting destructor' adjustor{4}
// SYNTHETIC: SWBF2 0x0045a230
// EntityCloth::`scalar deleting destructor'

namespace util { void Deallocate_(void* p, unsigned int uiSize, util::IAllocator& rAlloc); }

// FUNCTION: SWBF2 0x0045a610
EntityCloth::~EntityCloth()
{
    if (IsEnabled())
    {
        if (bank_)
        {
            bank_->Remove(this);
        }
        ClearData();
    }
}

// FUNCTION: SWBF2 0x0045a690
void EntityCloth::ClearData()
{
    DeactivateThread();
    util::Deallocate_(m_pPos, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pOldPos, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pNormal, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pAcceleration, ((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    util::Deallocate_(m_pFixedPos, ((ClothData*)mData)->GetNumFixedPoints_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pFixedPos = 0;
    m_pAcceleration = 0;
    m_pNormal = 0;
    m_pOldPos = 0;
    m_pPos = 0;
}

extern "C" PblMatrix* __stdcall D3DXMatrixMultiply(PblMatrix* pOut, const PblMatrix* pM1, const PblMatrix* pM2);
extern "C" PblVector3* __stdcall D3DXVec3TransformCoord(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);
extern "C" PblVector3* __stdcall D3DXVec3Normalize(PblVector3* pOut, const PblVector3* pV);

// FUNCTION: SWBF2 0x0040fe10
void ClothData::PlatformBindInit(EntityClothClass* pClass)
{
    ClothData* pData = (ClothData*)this;
    if (pData->m_pShader)
    {
        pData->m_pShader->Release();
    }

    unsigned int uiMode = 1;
    if (pClass->m_uiHardEdge)
    {
        uiMode = 3;
    }
    else if (pClass->m_uiTransparent)
    {
        uiMode = 5;
    }

    pData->m_pShader = pcRedShader::Create(PblHash("Normal"), 0x222, &pData->m_uiTextureNameHash, 1, uiMode,
                                           RedColor::WHITE, RedColor::BLACK, 32.0f);
}

// FUNCTION: SWBF2 0x0045a260
EntityCloth::EntityCloth(EntityClothClass* pClass, const EntityDesc& desc)
    : bank_(0),
      m_pPos(0),
      m_pOldPos(0),
      m_pNormal(0),
      m_pAcceleration(0),
      m_pFixedPos(0),
      priority_(0.0f),
      mClass(pClass)
{
    mDesc = desc;
    CreateData_();
}

// FUNCTION: SWBF2 0x0045a3a0
unsigned int ClothData::GetNumParticles_() const
{
    const ClothData* pData = (const ClothData*)this;
    return pData->m_uiNumParticles;
}

// FUNCTION: SWBF2 0x0045a3b0
unsigned int ClothData::GetNumFixedPoints_() const
{
    const ClothData* pData = (const ClothData*)this;
    return pData->m_uiNumFixedPoints;
}

// FUNCTION: SWBF2 0x0045a3c0
bool EntityCloth::Init(EntityClothClass* pClass, const EntityDesc& desc)
{
    if (m_pPos)
    {
        return true;
    }
    if (!IsEnabled())
    {
        return false;
    }

    mUpdateDt = 0.039f;
    mRenderDt = 0.0f;
    mData = ClothData::s_clothDataHashTable.Find(pClass->mId);
    ((ClothData*)mData)->PlatformBindInit(pClass);

    mOldVelocity = mVelocity = mAcceleration = PblVector3(0.0f, 0.0f, 0.0f);
    m_pPos = m_pOldPos = m_pNormal = m_pAcceleration = m_pFixedPos = 0;

    m_pPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pOldPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pNormal = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pAcceleration = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumParticles_() * sizeof(PblVector3), *GetSpecialAllocator());
    m_pFixedPos = (PblVector3*)util::Allocate_(((ClothData*)mData)->GetNumFixedPoints_() * sizeof(PblVector3), *GetSpecialAllocator());

    if (m_pPos && m_pOldPos && m_pNormal && m_pAcceleration && m_pFixedPos)
    {
        memcpy(m_pPos, mData->m_pPos, mData->m_uiNumParticles * sizeof(PblVector3));
        memcpy(m_pOldPos, mData->m_pPos, mData->m_uiNumParticles * sizeof(PblVector3));
        SetupFixedPos();
        mOldPosition = desc.mMatrix.GetTranslation();
        SetWorldMatrix(desc.mMatrix, mUpdateDt);
        ComputeNormals();
        m_lastFrameUpdated = 0;
        m_lastFrameVisible = 0;
        ActivateThread(0, PER_LOCAL_TURN);
        return true;
    }

    ClearData();
    return false;
}

// FUNCTION: SWBF2 0x0045a770
bool EntityCloth::Update(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return false;
    }
    mUpdateDt = (fDeltaTime > 0.039f) ? 0.039f : fDeltaTime;
    mRenderDt = 0.0f;
    return true;
}

// FUNCTION: SWBF2 0x0045a7c0
void EntityCloth::SetupFixedPos()
{
    EntityCloth* pCloth = (EntityCloth*)this;
    if (!pCloth->m_pPos)
    {
        pCloth->DeactivateThread();
        return;
    }

    ClothData* pData = pCloth->mData;
    if (!pData->m_pFixedPointsBones)
    {
        return;
    }

    const RedSkeleton* pSkeleton =
        RedModel::_HashTable.Find(pCloth->mClass->mModelHash)->GetSkeleton();
    const PblMatrix* pParent = pSkeleton->GetBoneMatrixByName(pData->m_uiParentHash);

    for (unsigned int i = 0; i < pData->m_uiNumFixedPoints; ++i)
    {
        unsigned int uiBone = pData->m_pFixedPointsBones ? pData->m_pFixedPointsBones[i] : 0;
        PblMatrix mBoneInv = *((RedSkeleton*)pSkeleton)->GetInvBoneMatrixByName(uiBone);
        PblMatrix mLocal;
        PblMatrix mFull;
        D3DXMatrixMultiply(&mLocal, &pData->m_Transform, pParent);
        D3DXMatrixMultiply(&mFull, &mLocal, &mBoneInv);
        PblVector3 vFixed;
        D3DXVec3TransformCoord(&vFixed, &pCloth->m_pPos[i], &mFull);
        pCloth->m_pFixedPos[i] = vFixed;
    }
}

// FUNCTION: SWBF2 0x0045ab20
void EntityCloth::SetWorldMatrix(const PblMatrix& mWorld, float fDeltaTime)
{
    EntityCloth* pCloth = (EntityCloth*)this;
    const float fInvDt = 1.0f / fDeltaTime;
    EntityClothClass* pClass = pCloth->mClass;

    pCloth->m_worldMat = mWorld;
    pCloth->mOldVelocity = pCloth->mVelocity;

    const PblVector3& vPos = pCloth->m_worldMat.GetTranslation();
    pCloth->mVelocity = PblVector3((vPos.x - pCloth->mOldPosition.x) * fInvDt,
                                   (vPos.y - pCloth->mOldPosition.y) * fInvDt,
                                   (vPos.z - pCloth->mOldPosition.z) * fInvDt);
    PblVector3& v = pCloth->mVelocity;
    float fLength2 = v.x * v.x + v.y * v.y + v.z * v.z;
    if (fLength2 > pClass->mMaxVelocity * pClass->mMaxVelocity)
    {
        float fScale = 1.0f / sqrtf(fLength2);
        v = PblVector3(v.x * fScale * pClass->mMaxVelocity, v.y * fScale * pClass->mMaxVelocity,
                       v.z * fScale * pClass->mMaxVelocity);
    }
    pCloth->mOldPosition = vPos;

    pCloth->mAcceleration = PblVector3((v.x - pCloth->mOldVelocity.x) * fInvDt,
                                       (v.y - pCloth->mOldVelocity.y) * fInvDt,
                                       (v.z - pCloth->mOldVelocity.z) * fInvDt);
    PblVector3& a = pCloth->mAcceleration;
    fLength2 = a.x * a.x + a.y * a.y + a.z * a.z;
    if (fLength2 > pClass->mMaxAcceleration * pClass->mMaxAcceleration)
    {
        float fScale = 1.0f / sqrtf(fLength2);
        a = PblVector3(a.x * fScale * pClass->mMaxAcceleration, a.y * fScale * pClass->mMaxAcceleration,
                       a.z * fScale * pClass->mMaxAcceleration);
    }
}

// FUNCTION: SWBF2 0x0045bd50
void EntityCloth::ComputeNormals()
{
    EntityCloth* pCloth = (EntityCloth*)this;
    ClothData* pData = pCloth->mData;
    const unsigned short* pIndices = pData->m_pTriangleIndices;
    memset(pCloth->m_pNormal, 0, pData->m_uiNumParticles * sizeof(PblVector3));

    for (unsigned int t = 0; t < pData->m_uiNumTriangles; ++t, pIndices += 3)
    {
        const PblVector3* pPos = pCloth->m_pPos;
        const unsigned int i0 = pIndices[0];
        const unsigned int i1 = pIndices[1];
        const unsigned int i2 = pIndices[2];

        const float e1x = pPos[i1].x - pPos[i0].x;
        const float e1y = pPos[i1].y - pPos[i0].y;
        const float e1z = pPos[i1].z - pPos[i0].z;
        const float e2x = pPos[i2].x - pPos[i0].x;
        const float e2y = pPos[i2].y - pPos[i0].y;
        const float e2z = pPos[i2].z - pPos[i0].z;

        const float nx = e1y * e2z - e1z * e2y;
        const float ny = e1z * e2x - e2z * e1x;
        const float nz = e2y * e1x - e1y * e2x;

        PblVector3* pN = pCloth->m_pNormal;
        pN[i0].x += nx;
        pN[i0].y += ny;
        pN[i0].z += nz;
        pN[i1].x += nx;
        pN[i1].y += ny;
        pN[i1].z += nz;
        pN[i2].x += nx;
        pN[i2].y += ny;
        pN[i2].z += nz;
    }

    for (unsigned int i = 0; i < pData->m_uiNumParticles; ++i)
    {
        PblVector3 vNormal;
        D3DXVec3Normalize(&vNormal, &pCloth->m_pNormal[i]);
        pCloth->m_pNormal[i] = vNormal;
    }
}

// FUNCTION: SWBF2 0x0045c180
template ClothBank*& util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> >::operator[](
    const unsigned int& rKey);

// FUNCTION: SWBF2 0x0045c3c0
template ClothBank*& util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                                      ClothBE::ClothDataT::KeyVecT,
                                      ClothBE::ClothDataT::HashVecT>::Get(const unsigned int& rKey);

// FUNCTION: SWBF2 0x0045c420
template void util::HashTable<ClothBank*, unsigned int, util::TNewArrayAlloc<ClothBank*> >::Reallocate(int iSize);

// FUNCTION: SWBF2 0x0045c600
template unsigned int util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                                       ClothBE::ClothDataT::KeyVecT,
                                       ClothBE::ClothDataT::HashVecT>::FindIndex_(unsigned int uiHash) const;

// FUNCTION: SWBF2 0x0045c7e0
template bool util::BaseArray<EntityCloth*, util::TNewArrayAlloc<EntityCloth*> >::Reserve_(int iCount, bool bForce);

// FUNCTION: SWBF2 0x0045c8a0
template bool util::Vector<ClothBank*, util::BaseArray<ClothBank*, util::TypedArrayAllocator<ClothBank*, util::TNewArrayAlloc<ClothBank*> > > >::SetSize_(
    int iSize, bool bForce);

// FUNCTION: SWBF2 0x0045c9a0
template bool util::Vector<unsigned int, util::BaseArray<unsigned int, util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<ClothBank*> > > >::SetSize_(
    int iSize, bool bForce);

// FUNCTION: SWBF2 0x0045c950
template ClothBank** util::TypedArrayAllocator<ClothBank*, util::TNewArrayAlloc<ClothBank*> >::Allocate_(int& riCount);

// FUNCTION: SWBF2 0x0045ca50
template unsigned int* util::TypedArrayAllocator<unsigned int, util::TNewArrayAlloc<ClothBank*> >::Allocate_(int& riCount);

// FUNCTION: SWBF2 0x0045caa0
template EntityCloth** util::TypedArrayAllocator<EntityCloth*, util::NewAllocator>::Allocate_(int& riCount);

// FUNCTION: SWBF2 0x0045cb10
template const float& util::Max_<float>(const float&, const float&);

extern "C" PblVector3* __stdcall D3DXVec3TransformNormal(PblVector3* pOut, const PblVector3* pV, const PblMatrix* pM);

// GLOBAL: SWBF2 0x007eb77c
static PblVector3 sGravity(0.0f, -9.8f, 0.0f);

static PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x0045b3b0
void EntityCloth::EnforceSphereCollision(const PblMatrix& mMatrix, float fRadius)
{
    EntityCloth* pCloth = (EntityCloth*)this;
    ClothData* pData = pCloth->mData;
    const float fRadiusSq = fRadius * fRadius;
    for (unsigned int i = pData->m_uiNumFixedPoints; i < pData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = pCloth->m_pPos[i];
        const float x = vPos.x - mMatrix.m[3][0];
        const float y = vPos.y - mMatrix.m[3][1];
        const float z = vPos.z - mMatrix.m[3][2];
        const float fDistSq = x * x + y * y + z * z;
        if (fDistSq < fRadiusSq)
        {
            const float fScale = fRadius / sqrtf(fDistSq);
            vPos.x = mMatrix.m[3][0] + x * fScale;
            vPos.y = mMatrix.m[3][1] + y * fScale;
            vPos.z = mMatrix.m[3][2] + z * fScale;
        }
    }
}

// FUNCTION: SWBF2 0x0045b4e0
void EntityCloth::EnforceBoxCollision(const PblMatrix& mMatrix, float fWidth, float fHeight, float fDepth)
{
    EntityCloth* pCloth = (EntityCloth*)this;
    ClothData* pData = pCloth->mData;
    for (unsigned int i = pData->m_uiNumFixedPoints; i < pData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = pCloth->m_pPos[i];
        const float dx = vPos.x - mMatrix.m[3][0];
        const float dy = vPos.y - mMatrix.m[3][1];
        const float dz = vPos.z - mMatrix.m[3][2];
        const float d0 = mMatrix.m[0][0] * dx + mMatrix.m[0][1] * dy + mMatrix.m[0][2] * dz;
        const float p0 = fWidth - fabsf(d0);
        if (p0 <= 0.0f)
        {
            continue;
        }
        const float d1 = mMatrix.m[1][0] * dx + mMatrix.m[1][1] * dy + mMatrix.m[1][2] * dz;
        const float p1 = fHeight - fabsf(d1);
        if (p1 <= 0.0f)
        {
            continue;
        }
        const float d2 = mMatrix.m[2][0] * dx + mMatrix.m[2][1] * dy + mMatrix.m[2][2] * dz;
        const float p2 = fDepth - fabsf(d2);
        if (p2 <= 0.0f)
        {
            continue;
        }
        const int axis = p1 <= p0 ? (p2 > p1 ? 1 : 2) : (p2 <= p0 ? 2 : 0);
        const float penetration = axis == 0 ? p0 : axis == 1 ? p1 : p2;
        const float side = (axis == 0 ? d0 : axis == 1 ? d1 : d2) >= 0.0f ? 1.0f : -1.0f;
        const float offset = penetration * side;
        vPos.x += mMatrix.m[axis][0] * offset;
        vPos.y += mMatrix.m[axis][1] * offset;
        vPos.z += mMatrix.m[axis][2] * offset;
    }
}

// FUNCTION: SWBF2 0x0045bf20
void EntityCloth::Render(const PblMatrix& mMatrix, const RedPose* pPose, unsigned char ucViewport, RedColor color,
                         unsigned int uiFlags, unsigned int uArg)
{
    m_lastFrameVisible = RedRenderer::GetFrameNumber();
    CreateData();
    if (!m_pPos || !IsEnabled())
    {
        return;
    }

    const PblMatrix* pParent = 0;
    if (pPose)
    {
        pParent = pPose->Find(mData->m_uiParentHash);
    }
    else if (mData->m_pFixedPointsBones)
    {
        return;
    }

    if (mUpdateDt > 0.0f)
    {
        float fRatio = NetGame::GetTurnRatio();
        if (fRatio <= 0.0f)
        {
            fRatio = 1.0f;
        }
        const float fStep = fRatio * mUpdateDt - mRenderDt;
        mRenderDt += fStep;
        if (fStep > 0.0f)
        {
            PblMatrix mLocal;
            if (pParent)
            {
                PblMatrix mTemp;
                D3DXMatrixMultiply(&mTemp, &mData->m_Transform, pParent);
                mLocal = mTemp;
            }
            else
            {
                mLocal = mData->m_Transform;
            }
            PblMatrix mWorld;
            D3DXMatrixMultiply(&mWorld, &mLocal, &mMatrix);
            SetWorldMatrix(mWorld, fStep);
            InternalUpdate(mLocal, pPose, uArg, fStep);
        }
    }

    const float fRadius = mData->m_fBoundingRadius;
    const PblSphere& modelSphere = RedModel::_HashTable.Find(mData->m_uiModelHash)->_BoundingSphere;
    PblSphere sphere;
    sphere.x = modelSphere.x + m_worldMat.m[3][0];
    sphere.y = modelSphere.y + m_worldMat.m[3][1];
    sphere.z = modelSphere.z + m_worldMat.m[3][2];
    sphere.w = fRadius;

    const RedLightingState* pLightingState = 0;
    if ((int)uiFlags >= 0)
    {
        if (uiFlags & 0x4000000)
        {
            pLightingState = &RedInterfaceScreen::s_lightingState;
        }
        else
        {
            pLightingState = RedLight::GetLightingState(sphere, 4, false, false, false);
        }
    }
    PlatformRender(ucViewport, color, uiFlags, pLightingState, sphere);
}

// FUNCTION: SWBF2 0x0045adb0
bool EntityCloth::InternalUpdate(const PblMatrix& mMatrix, const RedPose* pPose, unsigned int uIterations,
                                 float fDeltaTime)
{
    if (IsEnabled() && m_lastFrameUpdated < RedRenderer::GetFrameNumber())
    {
        m_lastFrameUpdated = RedRenderer::GetFrameNumber();
        AccumulateForces(fDeltaTime);
        Verlet(fDeltaTime);
        SatisfyConstraints(mMatrix, pPose, uIterations);
        ComputeNormals();
    }
    return true;
}

// FUNCTION: SWBF2 0x0045ae10
void EntityCloth::Verlet(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }

    const float fDt2 = fDeltaTime * fDeltaTime;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = m_pPos[i];
        PblVector3& vOld = m_pOldPos[i];
        const PblVector3& vAccel = m_pAcceleration[i];
        const PblVector3 vTemp = vPos;
        vPos.x += (vPos.x - vOld.x) + vAccel.x * fDt2;
        vPos.y += (vPos.y - vOld.y) + vAccel.y * fDt2;
        vPos.z += (vPos.z - vOld.z) + vAccel.z * fDt2;
        vOld = vTemp;
    }
}

// FUNCTION: SWBF2 0x0045af10
void EntityCloth::AccumulateForces(float fDeltaTime)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }

    const float fWind = PblRandom::_Global.GetFloat() * mClass->mWindSpeed;
    const float fDrag = mClass->mDrag;
    const PblMatrix mInverse = m_worldMat.GetInverseFast();

    PblVector3 vWind;
    D3DXVec3TransformNormal(&vWind, &mClass->mWindDirection, &mInverse);
    const float fDampening = mClass->mDampening;
    const PblVector3 vWorldGravity(sGravity.x - mAcceleration.x * fDampening, sGravity.y - mAcceleration.y * fDampening,
                                   sGravity.z - mAcceleration.z * fDampening);
    PblVector3 vGravity;
    D3DXVec3TransformNormal(&vGravity, &vWorldGravity, &mInverse);
    PblVector3 vVelocity;
    D3DXVec3TransformNormal(&vVelocity, &mVelocity, &mInverse);

    const float fInvDt = 1.0f / fDeltaTime;
    const float fInvMass = 1.0f / mClass->mParticleMass;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        const float fDragX = (vVelocity.x + (m_pPos[i].x - m_pOldPos[i].x) * fInvDt) * fDrag;
        const float fDragY = (vVelocity.y + (m_pPos[i].y - m_pOldPos[i].y) * fInvDt) * fDrag;
        const float fDragZ = (vVelocity.z + (m_pPos[i].z - m_pOldPos[i].z) * fInvDt) * fDrag;
        const PblVector3& vNormal = m_pNormal[i];
        const float fWindForce = (vNormal.x * vWind.x + vNormal.y * vWind.y + vNormal.z * vWind.z) * fWind;
        m_pAcceleration[i].x = vGravity.x + (vNormal.x * fWindForce + fDragX) * fInvMass;
        m_pAcceleration[i].y = vGravity.y + (vNormal.y * fWindForce + fDragY) * fInvMass;
        m_pAcceleration[i].z = vGravity.z + (vNormal.z * fWindForce + fDragZ) * fInvMass;
    }
}

// FUNCTION: SWBF2 0x0045bc40
void EntityCloth::SatisfyConstraints(const PblMatrix& mMatrix, const RedPose* pPose, unsigned int uIterations)
{
    ClothConstraint* pStretchPairs = mData->m_pStretchPairs;
    ClothConstraint* pBendPairs = mData->m_pBendPairs;
    ClothConstraint* pCrossPairs = mData->m_pCrossPairs;
    RestoreFixedPoints(pPose);

    for (; uIterations > 0; --uIterations)
    {
        if (mClass->m_uiDoCrossConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumCrossPairs; ++i)
            {
                EnforceConstraint(pCrossPairs[i], 1.0f);
            }
        }
        if (mClass->m_uiDoBendConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumBendPairs; ++i)
            {
                EnforceConstraint(pBendPairs[i], 1.0f);
            }
        }
        if (mClass->m_uiDoStretchConstraints)
        {
            for (unsigned int i = 0; i < mData->m_uiNumStretchPairs; ++i)
            {
                EnforceConstraint(pStretchPairs[i], 1.0f);
            }
        }
        EnforceCollisions(mMatrix, pPose);
    }
}

// FUNCTION: SWBF2 0x0045a910
void EntityCloth::RestoreFixedPoints(const RedPose* pPose)
{
    if (!m_pPos)
    {
        DeactivateThread();
        return;
    }
    if (!mData->m_pFixedPointsBones)
    {
        return;
    }

    const PblMatrix* pParent = pPose->Find(mData->m_uiParentHash);
    PblMatrix mParentInverse;
    mParentInverse.Reset();
    PblMatrix mTransformInverse;
    mTransformInverse.Reset();
    if (pParent)
    {
        mParentInverse = pParent->GetInverseFast();
        mTransformInverse = mData->m_Transform.GetInverseFast();
    }

    for (unsigned int i = 0; i < mData->m_uiNumFixedPoints; ++i)
    {
        const unsigned int uiBone = mData->m_pFixedPointsBones ? mData->m_pFixedPointsBones[i] : 0;
        const PblMatrix* pBone = pPose->Find(uiBone);
        PblMatrix mBone;
        mBone.Reset();
        if (pBone)
        {
            PblMatrix mTemp;
            D3DXMatrixMultiply(&mTemp, pBone, &mParentInverse);
            PblMatrix mFull;
            D3DXMatrixMultiply(&mFull, &mTemp, &mTransformInverse);
            mBone = mFull;
        }
        PblVector3 vPos;
        D3DXVec3TransformCoord(&vPos, &m_pFixedPos[i], &mBone);
        m_pPos[i] = vPos;
    }
}

// FUNCTION: SWBF2 0x0045b1b0
void EntityCloth::EnforceConstraint(ClothConstraint& constraint, float fRestitution)
{
    const unsigned int i0 = constraint.i0;
    const unsigned int i1 = constraint.i1;
    PblVector3& v0 = m_pPos[i0];
    PblVector3& v1 = m_pPos[i1];
    const float fDeltaX = v1.x - v0.x;
    const float fDeltaY = v1.y - v0.y;
    const float fDeltaZ = v1.z - v0.z;
    const float fLength = sqrtf(fDeltaX * fDeltaX + fDeltaY * fDeltaY + fDeltaZ * fDeltaZ);
    const float fDiff = (fLength - constraint.restLength) / fLength;

    const unsigned int uiNumFixed = mData->m_uiNumFixedPoints;
    if (i0 < uiNumFixed)
    {
        v1.x -= fDeltaX * fDiff;
        v1.y -= fDeltaY * fDiff;
        v1.z -= fDeltaZ * fDiff;
    }
    else if (i1 < uiNumFixed)
    {
        v0.x += fDeltaX * fDiff;
        v0.y += fDeltaY * fDiff;
        v0.z += fDeltaZ * fDiff;
    }
    else
    {
        const float fHalfX = fDeltaX * 0.5f;
        const float fHalfY = fDeltaY * 0.5f;
        const float fHalfZ = fDeltaZ * 0.5f;
        v0.x += fHalfX * fDiff;
        v0.y += fHalfY * fDiff;
        v0.z += fHalfZ * fDiff;
        v1.x -= fHalfX * fDiff;
        v1.y -= fHalfY * fDiff;
        v1.z -= fHalfZ * fDiff;
    }
}

// FUNCTION: SWBF2 0x0045ba40
void EntityCloth::EnforceCollisions(const PblMatrix& mMatrix, const RedPose* pPose)
{
    for (unsigned int i = 0; i < mData->m_uiNumCollisionVolumes; ++i)
    {
        ClothCollision& collision = mData->m_pCollisionVolumes[i];
        const PblMatrix* pBone = pPose ? pPose->Find(collision.parent) : 0;

        PblMatrix mCollision;
        if (pBone)
        {
            PblMatrix mTemp;
            D3DXMatrixMultiply(&mTemp, &collision.localMat, pBone);
            const PblMatrix mInverse = mMatrix.GetInverseFast();
            D3DXMatrixMultiply(&mCollision, &mTemp, &mInverse);
        }
        else
        {
            PblMatrix mBone = _IdentityMatrix;
            RedModel::_HashTable.Find(mData->m_uiModelHash)->GetBoneMatrix(collision.parent, mBone);
            PblMatrix mTemp;
            D3DXMatrixMultiply(&mTemp, &collision.localMat, &mBone);
            const PblMatrix mInverse = mMatrix.GetInverseFast();
            D3DXMatrixMultiply(&mCollision, &mTemp, &mInverse);
        }

        const PblMatrix mLocal = mCollision;
        switch (collision.type)
        {
        case CLOTH_COLLISION_SPHERE:
            EnforceSphereCollision(mLocal, collision.width);
            break;
        case CLOTH_COLLISION_CYLINDER:
            EnforceCylinderCollision(mLocal, collision.height, collision.width);
            break;
        case CLOTH_COLLISION_BOX:
            EnforceBoxCollision(mLocal, collision.width, collision.height, collision.depth);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x0045b7c0
void EntityCloth::EnforceCylinderCollision(const PblMatrix& mMatrix, float fHeight, float fRadius)
{
    const float fLength = fHeight * 2.0f;
    for (unsigned int i = mData->m_uiNumFixedPoints; i < mData->m_uiNumParticles; ++i)
    {
        PblVector3& vPos = m_pPos[i];
        const float fDX = vPos.x - mMatrix.m[3][0];
        const float fDY = vPos.y - mMatrix.m[3][1];
        const float fDZ = vPos.z - mMatrix.m[3][2];
        const float fDepth =
            fLength - fabsf(mMatrix.m[1][0] * fDX + mMatrix.m[1][1] * fDY + mMatrix.m[1][2] * fDZ);
        if (fDepth > 0.0f)
        {
            const PblVector3 vRadial(mMatrix.m[0][0] * fDX + mMatrix.m[0][1] * fDY + mMatrix.m[0][2] * fDZ, 0.0f,
                                     mMatrix.m[2][0] * fDX + mMatrix.m[2][1] * fDY + mMatrix.m[2][2] * fDZ);
            const float fDist = sqrtf(vRadial.x * vRadial.x + vRadial.y * vRadial.y + vRadial.z * vRadial.z);
            const float fPenetration = fRadius - fDist;
            if (fPenetration > 0.0f)
            {
                if (fDepth > fPenetration)
                {
                    const float fScale = fPenetration / fDist;
                    vPos.x += (mMatrix.m[0][0] * vRadial.x + mMatrix.m[2][0] * vRadial.z) * fScale;
                    vPos.y += (mMatrix.m[0][1] * vRadial.x + mMatrix.m[2][1] * vRadial.z) * fScale;
                    vPos.z += (mMatrix.m[0][2] * vRadial.x + mMatrix.m[2][2] * vRadial.z) * fScale;
                }
                else
                {
                    vPos.x += mMatrix.m[1][0] * fDepth;
                    vPos.y += mMatrix.m[1][1] * fDepth;
                    vPos.z += mMatrix.m[1][2] * fDepth;
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x00458df0
void EntityCloth::ClearSpecialAllocator()
{
    delete ClothBE::gMultiAllocator_;
    ClothBE::gMultiAllocator_ = 0;

    for (ClothBE::ClothDataT::Itor it = ClothBE::clothData_.Begin();
         it != ClothBE::clothData_.End(); ++it)
    {
        delete *it;
    }

    ClothBE::clothData_.Clear();
    ClothBE::enabled_ = true;
}

// FUNCTION: SWBF2 0x00458f00
ClothData::ClothData()
{
    m_uiNumParticles = 0;
    m_uiNumFixedPoints = 0;
    m_uiNumTriangles = 0;
    m_uiNumStretchPairs = 0;
    m_uiNumBendPairs = 0;
    m_uiNumCrossPairs = 0;
    m_uiNameHash = 0;
    m_uiParentHash = 0;
    m_pPos = 0;
    m_pTexCoords = 0;
    m_pTriangleIndices = 0;
    m_pStretchPairs = 0;
    m_pBendPairs = 0;
    m_pCrossPairs = 0;
    m_fBoundingRadius = 0.0f;
    m_pShader = 0;
}

// FUNCTION: SWBF2 0x00458f80
ClothData::~ClothData()
{
    PlatformCleanup();
    RedFreeToHeap(__RedCurrHeap, m_pPos, 0);
    RedFreeToHeap(__RedCurrHeap, m_pTexCoords, 0);
    RedFreeToHeap(__RedCurrHeap, m_pTriangleIndices, 0);
    RedFreeToHeap(__RedCurrHeap, m_pStretchPairs, 0);
    RedFreeToHeap(__RedCurrHeap, m_pBendPairs, 0);
    RedFreeToHeap(__RedCurrHeap, m_pCrossPairs, 0);
    if (m_pFixedPointsBones)
    {
        RedFreeToHeap(__RedCurrHeap, m_pFixedPointsBones, 0);
    }
}

// FUNCTION: SWBF2 0x00459060
void ClothData::Cleanup()
{
    s_clothDataHashTable.Clear();
    s_clothDataHashTableNumEntries = 0;
}

// FUNCTION: SWBF2 0x00459080
void ClothData::PostReadInit()
{
    for (unsigned int i = 0; i < m_uiNumStretchPairs; ++i)
    {
        const PblVector3& v0 = m_pPos[m_pStretchPairs[i].i0];
        const PblVector3& v1 = m_pPos[m_pStretchPairs[i].i1];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pStretchPairs[i].restLength = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }
    for (unsigned int i = 0; i < m_uiNumBendPairs; ++i)
    {
        const PblVector3& v0 = m_pPos[m_pBendPairs[i].i0];
        const PblVector3& v1 = m_pPos[m_pBendPairs[i].i1];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pBendPairs[i].restLength = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }
    for (unsigned int i = 0; i < m_uiNumCrossPairs; ++i)
    {
        const PblVector3& v0 = m_pPos[m_pCrossPairs[i].i0];
        const PblVector3& v1 = m_pPos[m_pCrossPairs[i].i1];
        PblVector3 vDelta(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
        m_pCrossPairs[i].restLength = sqrtf(vDelta.x * vDelta.x + vDelta.y * vDelta.y + vDelta.z * vDelta.z);
    }

    PblBox box;
    for (unsigned int i = 0; i < m_uiNumParticles; ++i)
    {
        box.AddPoint(m_pPos[i]);
    }
    PblVector3 vDiagonal(box._vMax.x - box._vMin.x, box._vMax.y - box._vMin.y, box._vMax.z - box._vMin.z);
    float fRadius = sqrtf(vDiagonal.x * vDiagonal.x + vDiagonal.y * vDiagonal.y + vDiagonal.z * vDiagonal.z) * 0.5f;
    m_fBoundingRadius = 2.0f * fRadius;
}

// FUNCTION: SWBF2 0x0045c310
template util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                          ClothBE::ClothDataT::KeyVecT, ClothBE::ClothDataT::HashVecT>::Itor
util::HashTable_<ClothBank*, unsigned int, ClothBE::ClothDataT::ElemVecT,
                 ClothBE::ClothDataT::KeyVecT, ClothBE::ClothDataT::HashVecT>::Begin();

// FUNCTION: SWBF2 0x004592e0
void ClothData::Read(PblFileChunk& chunk)
{
    ClothData* pData = new ClothData;
    pData->_Read(chunk);
    if (ClothData::s_clothDataHashTable.Find(pData->m_uiNameHash))
    {
        delete pData;
        return;
    }
    if (ClothData::s_clothDataHashTable.Store(pData->m_uiNameHash, pData))
    {
        ++ClothData::s_clothDataHashTableNumEntries;
    }
    pData->PostReadInit();
}

// FUNCTION: SWBF2 0x00437500
PblRegion* __cdecl FindClothRegion(unsigned int uId)
{
    return PblRegion::Find(uId);
}

namespace ClothBE
{
// GLOBAL: SWBF2 0x01e30354
util::IAllocator* gMultiAllocator_;
}
