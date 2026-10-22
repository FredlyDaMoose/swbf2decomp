#pragma once

class CollisionMesh;
class CollisionMeshMask;
class CollisionObject;
class CollisionMeshCache;
class PblBox;
class PblMatrix;
class PblVector3;

// VTABLE: SWBF2 0x00796e90
template <int N> class PolyListContainer;
class CollisionPrimitive
{
public:

    int GetClosestPoint(PblVector3& vMidPoint, float& fMaxDist, PolyListContainer<100>& triList,
                        PblVector3& vOutPoint, PblVector3& vClosestCylPoint, const PblMatrix&,
                        const PblMatrix&) const;
    enum PrimitiveType
    {
        SPHERE = 0,
        BUBBLE = 1,
        BOX = 2,
        AXIS_ALIGNED_CYLINDER = 3,
        CYLINDER = 4
    };

    virtual bool CheckForCollision(const PblMatrix& mMatrix, const PblVector3& vVelocity,
                                   const CollisionPrimitive& other, const PblMatrix& otherMatrix,
                                   const PblVector3& otherVelocity, PblVector3& vNormal,
                                   float& fDepth) const;

    static void EnterClient();
    static void LeaveClient();

    CollisionPrimitive();
    bool RayTest(const PblMatrix&, const PblVector3&, const PblVector3&, float);
    float RayHit(const PblMatrix&, const PblVector3&, const PblVector3&, float, PblVector3*, float);
    bool CollideTerrain(const PblMatrix&, PblVector3&, float&, PblVector3&) const;
    void Render(const PblMatrix&) const;
    bool InitMeshCollision(const PblMatrix&, const CollisionMesh*, const PblMatrix&, CollisionObject*,
                           CollisionMeshCache*&) const;
    bool CollideMesh(const PblMatrix&, const CollisionMesh*, const PblMatrix&, PblVector3&, float&,
                     float&) const;
    static void MoveMeshTestPos(const PblVector3&);

    void SetSphere(float fRadius);
    void SetAACylinder(float fRadius, float fHeight);
    void SetCylinder(float fRadius, float fHeight);
    void SetBox(float fWidth, float fHeight, float fDepth, float fRadius);

    void SetBubble(const PblVector3& vSize);

    PblBox GetWorldspaceAABB(const PblMatrix& matrix) const;

    bool CollideCylinder(const PblVector3&, const PblVector3&, const PblMatrix&, const PblMatrix&,
                         float, float, PblVector3&, float&) const;
    bool CollideAACylinder(const PblVector3&, const PblVector3&, const PblMatrix&, const PblVector3&,
                           float, float, PblVector3&, float&) const;
    bool CollideSphere(const PblVector3&, const PblVector3&, const PblMatrix&, const PblVector3&,
                       float, PblVector3&, float&) const;
    bool CollideBox(const PblVector3&, const PblVector3&, const PblMatrix&, const PblMatrix&,
                    float, float, float, PblVector3&, float&) const;

    unsigned int mName;
    float mRadius;
    float mHeight;
    float mWidth;
    float mDepth;
    PrimitiveType mType;
    float mMeshCollisionRadius;

    static CollisionMeshMask* sCurrentCollisionMeshMask;
};

namespace CollisionUtil {
float RayHitBox(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fWidth, float fDepth, float fHeight, PblVector3* pNormal);
}
namespace CollisionUtil {
float RayHitAxisAlignedCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal);
}
namespace CollisionUtil {
float RayHitCylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblMatrix& matrix,
                 float fRadius, float fHalfHeight, PblVector3* pNormal);
}

float RayHitAACylinder(const PblVector3& vStart, const PblVector3& vDir, float fLength, const PblVector3& vCenter,
                 float fRadius, float fHalfHeight, PblVector3* pNormal);

class CollisionAABB;
class CollisionTreeNode;
class EntitySoldier;
template <int N> class PolyListContainer;

extern PolyListContainer<100>* sCollTriList;
extern PolyListContainer<100>* sTriList;
