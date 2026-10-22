#pragma once

#include <stddef.h>

#include "Entity.h"
#include "Thread.h"
#include "RedEngineFL/Util/ext/basevector.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblVector.h"
#include "RedEngineFL/Memory/RedMemoryPool.h"

class ClothBank;
class ClothData;
class EntityClothClass;
class RedColor;
class RedPose;
struct ClothConstraint;
struct RedLightingState;
namespace util { class IAllocator; }

// VTABLE: SWBF2 0x007972b4
class EntityCloth : public Entity, public Thread
{
public:

    void SetupFixedPos();
    void SetWorldMatrix(const PblMatrix& mWorld, float fDeltaTime);
    void EnforceSphereCollision(const PblMatrix& mMatrix, float fRadius);
    void EnforceBoxCollision(const PblMatrix& mMatrix, float fWidth, float fHeight, float fDepth);
    void ComputeNormals();
    virtual bool IsRtti(unsigned int uRtti) const;
    virtual unsigned int GetDerivedRtti() const;
    virtual const char* GetDerivedRttiName() const;
    virtual ~EntityCloth();
    virtual EntityClass* GetEntityClass();
    virtual const EntityClass* GetEntityClass() const;
    virtual bool Update(float fDeltaTime);

    EntityCloth(EntityClothClass* pClass, const EntityDesc& desc);

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* p) { sMemoryPool.Free(p); }

    static bool IsEnabled();
    static void ClearSpecialAllocator();

    static void SetEnable(bool bEnable);
    static void SetMemUseage(unsigned int uBaseMem, unsigned int uMaxMem);

    static util::IAllocator* GetSpecialAllocator();
    bool Init(EntityClothClass* pClass, const EntityDesc& desc);
    void ClearData();
    void CreateData();
    void CreateData_();
    float GetPriority();
    void SetPriority(float fPriority);

    void Render(const PblMatrix& mMatrix, const RedPose* pPose, unsigned char ucViewport, RedColor color,
                unsigned int uiFlags, unsigned int uArg);

    ClothBank* bank_;
    PblVector3* m_pPos;
    PblVector3* m_pOldPos;
    PblVector3* m_pNormal;
    PblVector3* m_pAcceleration;
    PblVector3* m_pFixedPos;
    float priority_;
    PblMatrix m_worldMat;
    int m_lastFrameUpdated;
    int m_lastFrameVisible;
    PblVector3 mOldPosition;
    PblVector3 mOldVelocity;
    PblVector3 mVelocity;
    PblVector3 mAcceleration;
    EntityDesc mDesc;
    EntityClothClass* mClass;
    ClothData* mData;
    float mUpdateDt;
    float mRenderDt;

    static PblHash rttiHashEntityCloth;
    static char rttiNameEntityCloth[];

    static MemoryPool sMemoryPool;

private:

    bool InternalUpdate(const PblMatrix& mMatrix, const RedPose* pPose, unsigned int uIterations,
                        float fDeltaTime);
    void AccumulateForces(float fDeltaTime);
    void Verlet(float fDeltaTime);
    void SatisfyConstraints(const PblMatrix& mMatrix, const RedPose* pPose,
                            unsigned int uIterations);
    void RestoreFixedPoints(const RedPose* pPose);
    void EnforceConstraint(ClothConstraint& constraint, float fRestitution);
    void EnforceCollisions(const PblMatrix& mMatrix, const RedPose* pPose);
    void EnforceCylinderCollision(const PblMatrix& mMatrix, float fHeight, float fRadius);
    void PlatformRender(unsigned char ucViewport, RedColor color, unsigned int uiFlags,
                        const RedLightingState* pLightingState, const PblSphere& sphere);
};
 
 
 
 
 
 
 
 

class ClothBank
{
public:

    ClothBank(unsigned int uHashID) : hashID_(uHashID), lowPriority_(0.0f) {}

    EntityCloth* GetTop(float fPriority);
    bool Find(EntityCloth* pCloth);
    void Push(EntityCloth* pCloth, float fPriority);
    bool Remove(EntityCloth* pCloth);

    util::Vector<EntityCloth*> cloth_;
    unsigned int hashID_;
    float lowPriority_;
};
 
 

enum ClothCollisionType
{
    CLOTH_COLLISION_SPHERE = 0,
    CLOTH_COLLISION_CYLINDER = 1,
    CLOTH_COLLISION_BOX = 2,
    CLOTH_COLLISION_FORCE_DWORD = 0x7fffffff
};

struct ClothCollision
{
    PblMatrix localMat;
    unsigned int parent;
    ClothCollisionType type;
    float width;
    float height;
    float depth;
};
 
 

struct ClothConstraint
{
    unsigned short i0;
    unsigned short i1;
    float restLength;
};
 

class pcRedShader;

class ClothData
{
public:

    void PlatformBindInit(EntityClothClass* pClass);
    unsigned int GetNumParticles_() const;
    unsigned int GetNumFixedPoints_() const;

    static void Init();
    static void Cleanup();

    static MemoryPool sMemoryPool;

    static int s_clothDataHashTableNumEntries;
    static PblHashTable<ClothData, 64> s_clothDataHashTable;

    void _Read(PblFileChunk& chunk);

    static void Read(PblFileChunk& chunk);

    ClothData();

    ~ClothData();

    void PlatformCleanup();

    static void* operator new(unsigned int uSize) { return sMemoryPool.Allocate(uSize); }
    static void operator delete(void* p) { sMemoryPool.Free(p); }

    void PostReadInit();

    unsigned int m_uiNumParticles;
    unsigned int m_uiNumFixedPoints;
    unsigned int m_uiNumTriangles;
    unsigned int m_uiNumStretchPairs;
    unsigned int m_uiNumBendPairs;
    unsigned int m_uiNumCrossPairs;
    unsigned int m_uiNameHash;
    unsigned int m_uiParentHash;
    unsigned int m_uiModelHash;
    PblVector3* m_pPos;
    PblVector3* m_pTexCoords;
    unsigned short* m_pTriangleIndices;
    ClothConstraint* m_pStretchPairs;
    ClothConstraint* m_pBendPairs;
    ClothConstraint* m_pCrossPairs;
    unsigned int* m_pFixedPointsBones;
    PblMatrix m_Transform;
    unsigned int m_uiTextureNameHash;
    float m_fBoundingRadius;
    unsigned int m_uiNumCollisionVolumes;
    ClothCollision* m_pCollisionVolumes;
    pcRedShader* m_pShader;
};
 
 
 
 
 
 
 
 
 
 
 

// VTABLE: SWBF2 0x00797290
class EntityClothClass : public EntityClass
{
public:
    EntityClothClass(unsigned int uHash);
    virtual ~EntityClothClass();

    virtual EntityClass* Derive(unsigned int uHash);
    virtual Entity* Build(const EntityDesc& desc);
    virtual bool IsRtti(unsigned int uRtti);
    virtual unsigned int GetDerivedRtti();
    virtual const char* GetDerivedRttiName();
    virtual void SetProperty(unsigned int uProperty, const char* pszValue);

    unsigned int mModelHash;
    PblVector3 mWindDirection;
    float mWindSpeed;
    float mDampening;
    float mDrag;
    float mParticleMass;
    float mMaxAcceleration;
    float mMaxVelocity;
    float mPriority;
    unsigned int m_uiDoCrossConstraints : 1;
    unsigned int m_uiDoBendConstraints : 1;
    unsigned int m_uiDoStretchConstraints : 1;
    unsigned int m_uiHardEdge : 1;
    unsigned int m_uiTransparent : 1;

    static PblHash rttiHashEntityClothClass;
    static char rttiNameEntityClothClass[];
};
  
 
 
 
 
 
 
 
 
 
