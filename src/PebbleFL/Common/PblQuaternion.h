#pragma once

class PblMatrix;
class PblVector3;

class PblQuaternion
{
public:

    // FUNCTION: SWBF2 0x0043ce60
    PblQuaternion() : _fX(0.0f), _fY(0.0f), _fZ(0.0f), _fS(1.0f) {}
    PblQuaternion(float fX, float fY, float fZ, float fS) : _fX(fX), _fY(fY), _fZ(fZ), _fS(fS) {}
    PblQuaternion(const PblMatrix& rMatrix);

    PblQuaternion Inverse() const
    {
        PblQuaternion q;
        q._fX = -_fX;
        q._fY = -_fY;
        q._fZ = -_fZ;
        q._fS = _fS;
        return q;
    }

    PblVector3 RotateVector(const PblVector3& v) const;

    float _fX;
    float _fY;
    float _fZ;
    float _fS;
};

PblQuaternion Interpolate(const PblQuaternion& q0, const PblQuaternion& q1, float fT);

PblQuaternion operator*(const PblQuaternion& q0, const PblQuaternion& q1);

 
