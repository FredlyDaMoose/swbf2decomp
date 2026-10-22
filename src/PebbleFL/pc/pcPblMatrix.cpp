#include <d3dx9math.h>
#include <string.h>

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

// FUNCTION: SWBF2 0x0040cb30
PblMatrix::PblMatrix(const PblMatrix& other)
{
    memcpy(m, other.m, sizeof(m));
}

// FUNCTION: SWBF2 0x0043a8c0
PblMatrix::PblMatrix(const PblQuaternion& rotation, const PblVector3& translation)
{
    D3DXQUATERNION normalized;
    D3DXMatrixRotationQuaternion((D3DXMATRIX*)this,
        D3DXQuaternionNormalize(&normalized, (const D3DXQUATERNION*)&rotation));
    memcpy(&m[3][0], &translation, sizeof(translation));
    m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x0062c130
PblMatrix::PblMatrix(const PblVector4& x, const PblVector4& y,
                     const PblVector4& z, const PblVector4& t)
{
    memcpy(m[0], &x, sizeof(x));
    memcpy(m[1], &y, sizeof(y));
    memcpy(m[2], &z, sizeof(z));
    memcpy(m[3], &t, sizeof(t));
}

#pragma warning(disable : 4073)
#pragma init_seg(lib)
// SYNTHETIC: SWBF2 0x0040ae50
// `dynamic initializer for '_IdentityMatrix''
// GLOBAL: SWBF2 0x009caee0
PblMatrix gMatrixIdentity(PblVector4(1.0f, 0.0f, 0.0f, 0.0f), PblVector4(0.0f, 1.0f, 0.0f, 0.0f),
                          PblVector4(0.0f, 0.0f, 1.0f, 0.0f), PblVector4(0.0f, 0.0f, 0.0f, 1.0f));

// FUNCTION: SWBF2 0x004384f0
PblMatrix::PblMatrix(const PblVector3& x, const PblVector3& y,
                     const PblVector3& z, const PblVector3& t)
{
    memcpy(m[0], &x, sizeof(x));
    m[0][3] = 0.0f;
    memcpy(m[1], &y, sizeof(y));
    m[1][3] = 0.0f;
    memcpy(m[2], &z, sizeof(z));
    m[2][3] = 0.0f;
    memcpy(m[3], &t, sizeof(t));
    m[3][3] = 1.0f;
}

// FUNCTION: SWBF2 0x00411dd0
const PblMatrix PblMatrix::GetInverseFast() const
{
    PblMatrix out;

    out = *this;

    float fSwap;
    fSwap = out.m[0][1];
    out.m[0][1] = out.m[1][0];
    out.m[1][0] = fSwap;
    fSwap = out.m[0][2];
    out.m[0][2] = out.m[2][0];
    out.m[2][0] = fSwap;
    fSwap = out.m[1][2];
    out.m[1][2] = out.m[2][1];
    out.m[2][1] = fSwap;

    D3DXVECTOR3 vRotated;
    D3DXVec3TransformNormal(&vRotated,
                            (const D3DXVECTOR3*)out.m[3],
                            (const D3DXMATRIX*)&out);

    out.m[3][0] = -vRotated.x;
    out.m[3][1] = -vRotated.y;
    out.m[3][2] = -vRotated.z;
    out.m[3][3] = 1.0f;

    return out;
}

// FUNCTION: SWBF2 0x00458960
void PblMatrix::Reset()
{
    for (int r = 0; r < 4; r++)
    {
        for (int c = 0; c < 4; c++)
        {
            m[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }
}
