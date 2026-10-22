#pragma once

class PblVector3;

struct CompressedPos
{
    short x;
    short y;
    short z;
    short w;

    void SetPos(float fX, float fY, float fZ);

    static void __cdecl SetBoundingBox(const class PblBox* pBox);

    static PblVector3 s_vMin;
    static PblVector3 s_vScale;
    static PblVector3 s_vScaleInv;
};

 
