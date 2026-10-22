#include <math.h>
#include <d3dx9math.h>

#include "TentacleSimulator.h"
#include "NetGame.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "RedEngineFL/Graphics/RedPose.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Application/pc/pcRedDebug.h"

extern bool netFrameLock;
extern bool netIsLocalTurn;

// GLOBAL: SWBF2 0x01faa750
int sDrawCollisionShapes;

// GLOBAL: SWBF2 0x0078b630
static const unsigned int TENTACLE_NAMES[20] =
{
    0x24cb9e5e, 0x2988b887, 0x2d49a530, 0x330ef535, 0x37cfe882,
    0x3a8cce5b, 0x3e4dd3ec, 0x06026e51, 0x02c373e6, 0x257cb3e7,
    0x21bdae50, 0x2cfe8889, 0x283f953e, 0x3678c53b, 0x32b9d88c,
    0x3ffafe55, 0x3b3be3e2, 0x03745e5f, 0x07b543e8, 0x5797ec34,
};

static const float gTentacleAngle = 0.52359879f;

// GLOBAL: SWBF2 0x01f992e0
// SYNTHETIC: SWBF2 0x004069d0
// `dynamic initializer for 'TentacleSimulator::sMemoryPool''
// SYNTHETIC: SWBF2 0x00769590
// `dynamic atexit destructor for 'TentacleSimulator::sMemoryPool''
MemoryPool TentacleSimulator::sMemoryPool("TentacleSimulator", 0x268);

// FUNCTION: SWBF2 0x00656e70
void TentacleSimulator::EnforceBoxCollision(const PblMatrix& matrix, float fX, float fY, float fZ)
{
    const float* m = (const float*)&matrix;
    for (int t = 0; t < mNumTentacles; ++t)
    {
        for (int b = 1; b <= mBonesPerTentacle; ++b)
        {
            PblVector3& pos = tPos[t][b];
            const float dx = pos.x - m[12];
            const float dy = pos.y - m[13];
            const float dz = pos.z - m[14];
            const float projX = m[0] * dx + m[1] * dy + m[2] * dz;
            const float depthX = fX - fabsf(projX);
            if (depthX <= 0.0f)
            {
                continue;
            }
            const float projY = m[4] * dx + m[5] * dy + m[6] * dz;
            const float depthY = fY - fabsf(projY);
            if (depthY <= 0.0f)
            {
                continue;
            }
            const float projZ = m[8] * dx + m[9] * dy + m[10] * dz;
            const float depthZ = fZ - fabsf(projZ);
            if (depthZ <= 0.0f)
            {
                continue;
            }
            const float* axis = m;
            float penetration = depthX;
            float projection = projX;
            if (depthY <= penetration)
            {
                axis = m + 4;
                penetration = depthY;
                projection = projY;
            }
            if (depthZ <= penetration)
            {
                axis = m + 8;
                penetration = depthZ;
                projection = projZ;
            }
            const float signedDepth = projection >= 0.0f ? penetration : -penetration;
            pos.x += axis[0] * signedDepth;
            pos.y += axis[1] * signedDepth;
            pos.z += axis[2] * signedDepth;
        }
    }
}

// FUNCTION: SWBF2 0x00657140
void TentacleSimulator::EnforceCylinderCollision(const PblMatrix& matrix, float fHalfScale, float fRadius)
{
    const float* m = (const float*)&matrix;
    for (int t = 0; t < mNumTentacles; ++t)
    {
        for (int b = 1; b <= mBonesPerTentacle; ++b)
        {
            PblVector3& pos = tPos[t][b];
            const float dx = pos.x - m[12];
            const float dy = pos.y - m[13];
            const float dz = pos.z - m[14];
            const float along = m[4] * dx + m[5] * dy + m[6] * dz;
            const float axialDepth = fHalfScale * 2.0f - fabsf(along);
            if (axialDepth <= 0.0f)
            {
                continue;
            }
            const float radialX = m[0] * dx + m[1] * dy + m[2] * dz;
            const float radialZ = m[8] * dx + m[9] * dy + m[10] * dz;
            const float radialDistance = sqrtf(radialX * radialX + radialZ * radialZ);
            const float radialDepth = fRadius - radialDistance;
            if (radialDepth <= 0.0f)
            {
                continue;
            }
            if (axialDepth <= radialDepth)
            {
                pos.x += m[4] * axialDepth;
                pos.y += m[5] * axialDepth;
                pos.z += m[6] * axialDepth;
            }
            else
            {
                const float scale = radialDepth / radialDistance;
                pos.x += (m[0] * radialX + m[8] * radialZ) * scale;
                pos.y += (m[1] * radialX + m[9] * radialZ) * scale;
                pos.z += (m[2] * radialX + m[10] * radialZ) * scale;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00655750
void TentacleSimulator::Init()
{
    sMemoryPool.Setup(0x22, sMemoryPool.mSize, sMemoryPool.mGrow);
}

// FUNCTION: SWBF2 0x00655770
TentacleSimulator::TentacleSimulator(unsigned int uiNumTentacles, unsigned int uiBonesPerTentacle,
                                     unsigned int uiCollType)
{
    oldVelocity.z = 0.0f;
    oldVelocity.y = 0.0f;
    oldVelocity.x = 0.0f;
    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 0; b < mBonesPerTentacle; b++)
        {
            oldPos[t][b].x = 0.0f;
            oldPos[t][b].y = 0.0f;
            oldPos[t][b].z = 0.0f;
            tPos[t][b].x = 0.0f;
            tPos[t][b].y = 0.0f;
            tPos[t][b].z = 0.0f;
        }
    }
    mNumTentacles = uiNumTentacles;
    mBonesPerTentacle = uiBonesPerTentacle;
    mCollType = uiCollType;
    mInternalTimer = 0.0f;
    mTimeSinceLastUpdate = 0.0f;
    mTimerOffset = 0.0f;
    mFirstUpdate = true;
}

// FUNCTION: SWBF2 0x00655840
void TentacleSimulator::UpdateTimer(float fDeltaTime)
{
    if (!NetGame::IsEnabled() || netFrameLock)
    {
        mInternalTimer = fDeltaTime + mInternalTimer;
    }
    else if (netIsLocalTurn)
    {
        if (mTimeSinceLastUpdate > 0.0f)
        {
            mTimerOffset = mInternalTimer - mTimeSinceLastUpdate;
            mTimeSinceLastUpdate = 0.0f;
            mInternalTimer = fDeltaTime;
        }
        else
        {
            mTimeSinceLastUpdate = 0.0f;
            mInternalTimer = fDeltaTime + mInternalTimer;
        }
    }
    else
    {
        mInternalTimer = 0.0f;
        mTimeSinceLastUpdate = 0.0f;
    }
}

// FUNCTION: SWBF2 0x006558f0
void TentacleSimulator::DoTentacles(RedPose* pPose, const PblMatrix& mParent,
                                    const PblVector3& vVelocity, PblMatrix* pOutMatrices,
                                    float fDeltaTime)
{
    if (!NetGame::IsEnabled() || netFrameLock)
    {
        fDeltaTime = mInternalTimer;
        if (fDeltaTime > 0.039f)
        {
            fDeltaTime = 0.039f;
        }
    }
    else if (!(fDeltaTime >= 0.0f))
    {
        fDeltaTime = NetGame::GetTurnRatio() * mInternalTimer + mTimerOffset;
        if (fDeltaTime > mTimeSinceLastUpdate)
        {
            fDeltaTime = fDeltaTime - mTimeSinceLastUpdate;
        }
        mTimerOffset = 0.0f;
        mTimeSinceLastUpdate = NetGame::GetTurnRatio() * mInternalTimer;
    }

    if (!pPose)
    {
        return;
    }

    if (!NetGame::IsEnabled() || netFrameLock)
    {
        mInternalTimer = 0.0f;
    }

    if (!pPose->_Table.Find(TENTACLE_NAMES[0]))
    {
        return;
    }

    const PblMatrix* apBones[4][5];
    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 0; b < mBonesPerTentacle; b++)
        {
            apBones[t][b] = pPose->_Table.Find(TENTACLE_NAMES[t * mBonesPerTentacle + b]);
        }
    }

    if (mFirstUpdate)
    {
        for (int t = 0; t < mNumTentacles; t++)
        {
            for (int b = 0; b < mBonesPerTentacle; b++)
            {
                const PblMatrix* pBone = apBones[t][b];
                oldPos[t][b] = *(const PblVector3*)pBone->m[3];
                tPos[t][b] = *(const PblVector3*)pBone->m[3];
            }
        }
        mFirstUpdate = false;
    }

    if (fDeltaTime > 0.0f)
    {
        UpdatePositions(fDeltaTime, vVelocity, pPose, mParent, apBones);
        EnforceCollisions(pPose, mParent);
    }
    UpdatePose(pPose, mParent, apBones, pOutMatrices);
}

// FUNCTION: SWBF2 0x00655b60
void TentacleSimulator::UpdatePose(RedPose* pPose, const PblMatrix& mParent,
                                   const PblMatrix* (*const pBones)[5], PblMatrix* pOutMatrices)
{
    if (!pOutMatrices)
    {
        return;
    }

    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 0; b < mBonesPerTentacle; b++)
        {
            const float fCosAngle = cos(gTentacleAngle);

            PblMatrix& m = pOutMatrices[t * mBonesPerTentacle + b];
            m = *pBones[t][b];
            if (b > 0)
            {
                const PblMatrix mInverse = pBones[t][b - 1]->GetInverseFast();
                PblMatrix mLocal;
                D3DXMatrixMultiply((D3DXMATRIX*)&mLocal, (const D3DXMATRIX*)&m,
                                   (const D3DXMATRIX*)&mInverse);
                PblMatrix mResult;
                D3DXMatrixMultiply((D3DXMATRIX*)&mResult, (const D3DXMATRIX*)&mLocal,
                                   (const D3DXMATRIX*)(&m - 1));
                m = mResult;
            }

            const D3DXVECTOR3 vStart = *(const D3DXVECTOR3*)&tPos[t][b];
            D3DXVECTOR3 vEnd = *(const D3DXVECTOR3*)&tPos[t][b + 1];
            D3DXVECTOR3 vDir = vEnd - vStart;
            D3DXVECTOR3 vNormal;
            D3DXVec3Normalize(&vNormal, &vDir);

            const D3DXVECTOR3* pZ = (const D3DXVECTOR3*)m.m[2];
            float fDot = D3DXVec3Dot(pZ, &vNormal);
            if (fDot < fCosAngle)
            {
                D3DXVECTOR3 vAxis;
                D3DXVec3Cross(&vAxis, pZ, &vNormal);
                float fAngle = acos(fDot);
                if (sqrtf(D3DXVec3Dot(&vAxis, &vAxis)) * fAngle < 0.0f)
                {
                    fAngle = -fAngle;
                }

                bool bBend = false;
                if (fAngle > gTentacleAngle)
                {
                    fAngle = gTentacleAngle - fAngle;
                    bBend = true;
                }
                else if (fAngle < -gTentacleAngle)
                {
                    fAngle = fAngle + gTentacleAngle;
                    bBend = true;
                }

                if (bBend)
                {
                    D3DXMATRIX mRotation;
                    D3DXMatrixRotationAxis(&mRotation, &vAxis, fAngle);
                    D3DXVECTOR3 vSegment = vEnd - vStart;
                    D3DXVECTOR3 vRotated;
                    D3DXVec3TransformNormal(&vRotated, &vSegment, &mRotation);
                    vEnd = vRotated + vStart;
                    *(D3DXVECTOR3*)&tPos[t][b + 1] = vEnd;
                }
            }

            D3DXVECTOR3 vSegment = vEnd - vStart;
            D3DXVECTOR3 vZ;
            D3DXVec3Normalize(&vZ, &vSegment);
            *(D3DXVECTOR3*)m.m[2] = vZ;
            m.m[2][3] = 0.0f;

            D3DXVECTOR3 vCross;
            D3DXVECTOR3 vX;
            D3DXVec3Cross(&vCross, (const D3DXVECTOR3*)m.m[1], (const D3DXVECTOR3*)m.m[2]);
            D3DXVec3Normalize(&vX, &vCross);
            *(D3DXVECTOR3*)m.m[0] = vX;
            m.m[0][3] = 0.0f;

            D3DXVECTOR3 vY;
            D3DXVec3Cross(&vCross, (const D3DXVECTOR3*)m.m[2], (const D3DXVECTOR3*)m.m[0]);
            D3DXVec3Normalize(&vY, &vCross);
            *(D3DXVECTOR3*)m.m[1] = vY;
            m.m[1][3] = 0.0f;

            unsigned int uiName = TENTACLE_NAMES[mBonesPerTentacle * t + b];
            if (pPose->_Table.Remove(uiName))
            {
                pPose->_iNumEntries--;
            }
            if (pPose->_Table.Store(uiName, &m))
            {
                pPose->_iNumEntries++;
            }
        }
    }
}

// FUNCTION: SWBF2 0x00656270
void TentacleSimulator::UpdatePositions(float fDeltaTime, const PblVector3& vVelocity,
                                        const RedPose* pPose, const PblMatrix& mParent,
                                        const PblMatrix* (*const pBones)[5])
{
    float fInvDeltaTime = 1.0f / fDeltaTime;

    PblVector3 vVel = vVelocity;
    float fLengthSq = vVel.x * vVel.x + vVel.y * vVel.y + vVel.z * vVel.z;
    if (fLengthSq > 100.0f)
    {
        float fInvLength = 1.0f / (float)sqrt((double)fLengthSq);
        vVel.x = vVel.x * fInvLength * 10.0f;
        vVel.y = vVel.y * fInvLength * 10.0f;
        vVel.z = vVel.z * fInvLength * 10.0f;
    }

    PblVector3 vAccel;
    vAccel.x = (vVel.x - oldVelocity.x) * fInvDeltaTime;
    vAccel.y = (vVel.y - oldVelocity.y) * fInvDeltaTime;
    vAccel.z = (vVel.z - oldVelocity.z) * fInvDeltaTime;
    fLengthSq = vAccel.x * vAccel.x + vAccel.y * vAccel.y + vAccel.z * vAccel.z;
    if (fLengthSq > 400.0f)
    {
        float fInvLength = 1.0f / (float)sqrt((double)fLengthSq);
        vAccel.x = vAccel.x * fInvLength * 20.0f;
        vAccel.y = vAccel.y * fInvLength * 20.0f;
        vAccel.z = vAccel.z * fInvLength * 20.0f;
    }
    oldVelocity = vVel;

    static const PblVector3 vGravity(0.0f, -9.8f, 0.0f);
    static const float fDragScale = 1.0f;

    const PblMatrix mInverse = mParent.GetInverseFast();
    PblVector3 vForce;
    vForce.x = vGravity.x - vAccel.x * 0.5f;
    vForce.y = vGravity.y - vAccel.y * 0.5f;
    vForce.z = vGravity.z - vAccel.z * 0.5f;
    PblVector3 vLocalForce;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocalForce, (const D3DXVECTOR3*)&vForce,
                            (const D3DXMATRIX*)&mInverse);
    PblVector3 vLocalVelocity;
    D3DXVec3TransformNormal((D3DXVECTOR3*)&vLocalVelocity, (const D3DXVECTOR3*)&vVelocity,
                            (const D3DXMATRIX*)&mInverse);

    PblVector3 avAccel[4][6];
    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 1; b < mBonesPerTentacle + 1; b++)
        {
            float fVx = (tPos[t][b].x - oldPos[t][b].x) * fInvDeltaTime;
            float fVy = (tPos[t][b].y - oldPos[t][b].y) * fInvDeltaTime;
            float fVz = (tPos[t][b].z - oldPos[t][b].z) * fInvDeltaTime;
            avAccel[t][b].x = vLocalForce.x + (vLocalVelocity.x + fVx) * -5.0f * fDragScale;
            avAccel[t][b].y = vLocalForce.y + (vLocalVelocity.y + fVy) * -5.0f * fDragScale;
            avAccel[t][b].z = vLocalForce.z + (vLocalVelocity.z + fVz) * -5.0f * fDragScale;
        }
    }

    float fDeltaTimeSq = fDeltaTime * fDeltaTime;
    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 1; b < mBonesPerTentacle + 1; b++)
        {
            PblVector3 vPos = tPos[t][b];
            tPos[t][b].x = vPos.x - oldPos[t][b].x + fDeltaTimeSq * avAccel[t][b].x + vPos.x;
            tPos[t][b].y = vPos.y - oldPos[t][b].y + fDeltaTimeSq * avAccel[t][b].y + vPos.y;
            tPos[t][b].z = vPos.z - oldPos[t][b].z + fDeltaTimeSq * avAccel[t][b].z + vPos.z;
            oldPos[t][b] = vPos;
        }
    }

    for (int t = 0; t < mNumTentacles; t++)
    {
        tPos[t][0] = *(const PblVector3*)pBones[t][0]->m[3];
    }

    for (int t = 0; t < mNumTentacles; t++)
    {
        for (int b = 0; b < mBonesPerTentacle; b++)
        {
            PblVector3 vPos = tPos[t][b + 1];
            float fDx = vPos.x - tPos[t][b].x;
            float fDy = vPos.y - tPos[t][b].y;
            float fDz = vPos.z - tPos[t][b].z;
            float fLength = sqrtf(fDx * fDx + fDy * fDy + fDz * fDz);

            float fRestLength = 0.25f;
            if (b != mBonesPerTentacle - 1)
            {
                const float* p0 = pBones[t][b]->m[3];
                const float* p1 = pBones[t][b + 1]->m[3];
                float fBx = p1[0] - p0[0];
                float fBy = p1[1] - p0[1];
                float fBz = p1[2] - p0[2];
                fRestLength = sqrtf(fBx * fBx + fBy * fBy + fBz * fBz);
            }

            float fScale = (fLength - fRestLength) / fLength;
            tPos[t][b + 1].x = vPos.x - fDx * fScale;
            tPos[t][b + 1].y = vPos.y - fDy * fScale;
            tPos[t][b + 1].z = vPos.z - fDz * fScale;
        }
    }
}

// FUNCTION: SWBF2 0x006569c0
void TentacleSimulator::EnforceCollisions(const RedPose* pPose, const PblMatrix& mParent)
{
    const PblMatrix* pBone = pPose->_Table.Find(0xe5b51072);
    if (!pBone)
    {
        return;
    }

    PblMatrix mCollision = *pBone;
    float fOffset = 0.0f;
    if (mCollType == 1)
    {
        fOffset = 0.98f;
    }
    mCollision.m[3][3] = 1.0f;
    float fX = mCollision.m[3][0] + mCollision.m[0][0] * 0.0f + mCollision.m[1][0] * fOffset + mCollision.m[2][0] * 0.0f;
    float fY = mCollision.m[3][1] + mCollision.m[0][1] * 0.0f + mCollision.m[1][1] * fOffset + mCollision.m[2][1] * 0.0f;
    float fZ = mCollision.m[3][2] + mCollision.m[0][2] * 0.0f + mCollision.m[1][2] * fOffset + mCollision.m[2][2] * 0.0f;
    mCollision.m[3][0] = fX;
    mCollision.m[3][1] = fY;
    mCollision.m[3][2] = fZ;

    switch (mCollType)
    {
    case 0:
        EnforceBoxCollision(mCollision, 0.35f, 0.18f, 0.5f);
        break;
    case 1:
        EnforceSphereCollision(mCollision, 1.234f);
        break;
    case 2:
        EnforceCylinderCollision(mCollision, 0.18f, 1.234f);
        break;
    }

    if (sDrawCollisionShapes)
    {
        D3DXMatrixMultiply((D3DXMATRIX*)&mCollision, (const D3DXMATRIX*)&mCollision,
                           (const D3DXMATRIX*)&mParent);
        switch (mCollType)
        {
        case 0:
        {
            PblBox box;
            box._vMin = PblVector3(-0.35f, -0.18f, -0.5f);
            box._vMax = PblVector3(0.35f, 0.18f, 0.5f);
            RedDebug::DrawOBB(box, mCollision, RedColor::WHITE);
            break;
        }
        case 1:
            RedDebug::DrawSphere(PblVector3(mCollision.m[3][0], mCollision.m[3][1], mCollision.m[3][2]),
                                 1.234f, RedColor::WHITE);
            break;
        case 2:
            RedDebug::DrawCone(mCollision, 0.18f, 1.234f, 1.234f, RedColor::WHITE);
            break;
        }
    }
}

// FUNCTION: SWBF2 0x00656d00
void TentacleSimulator::EnforceSphereCollision(const PblMatrix& mMatrix, float fRadius)
{
    const PblVector3& vCenter = mMatrix.GetTranslation();
    for (int i = 0; i < mNumTentacles; ++i)
    {
        const float fRadiusSq = fRadius * fRadius;
        for (int j = 1; j < mBonesPerTentacle + 1; ++j)
        {
            PblVector3& vPos = tPos[i][j];
            const float dx = vPos.x - vCenter.x;
            const float dy = vPos.y - vCenter.y;
            const float dz = vPos.z - vCenter.z;
            const float fDistSq = dx * dx + dy * dy + dz * dz;
            if (fRadiusSq > fDistSq)
            {
                const float fScale = fRadius / sqrtf(fDistSq);
                vPos.x = dx * fScale + vCenter.x;
                vPos.y = dy * fScale + vCenter.y;
                vPos.z = dz * fScale + vCenter.z;
            }
        }
    }
}
