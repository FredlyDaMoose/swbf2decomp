#pragma once

#include <stddef.h>

class PblVector3;
class PblVector4;
class PblQuaternion;

class __declspec(align(16)) PblMatrix
{
public:

    // FUNCTION: SWBF2 0x0040fe00
    PblMatrix() {}

    PblMatrix(const PblMatrix& other);

    PblMatrix* CopyFrom(const PblMatrix* pSrc);

    PblMatrix(const PblQuaternion& rotation, const PblVector3& translation);

    PblMatrix(const PblVector4& x, const PblVector4& y,
              const PblVector4& z, const PblVector4& t);

    PblMatrix(const PblVector3& x, const PblVector3& y,
              const PblVector3& z, const PblVector3& t);

    const PblMatrix GetInverseFast() const;

    void Reset();

    void Transpose4x4()
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = r + 1; c < 4; c++)
            {
                float t = m[r][c];
                m[r][c] = m[c][r];
                m[c][r] = t;
            }
        }
    }

    // FUNCTION: SWBF2 0x00410890
    const PblVector3& GetTranslation() const { return *(const PblVector3*)&m[3][0]; }

    const PblVector3& GetXAxis() const { return *(const PblVector3*)&m[0][0]; }
    const PblVector3& GetYAxis() const { return *(const PblVector3*)&m[1][0]; }

    // FUNCTION: SWBF2 0x0040cb70
    const PblVector3& GetZAxis() const { return *(const PblVector3*)&m[2][0]; }

    float m[4][4];
};

 
 
