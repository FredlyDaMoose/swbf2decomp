#pragma once

#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblHashTable.h"

class RedSkeleton;
class GameObject;
template <int N> class ZephyrSkeleton;
template <int N> class ZephyrPoseStatic;

class RedPose
{
public:

    // FUNCTION: SWBF2 0x00443940
    RedPose() { _Table.Clear(); _iNumEntries = 0; m_pSkeleton = 0; }

    const PblMatrix* Find(unsigned int uHash) const { return _Table.Find(uHash); }
    void Store(unsigned int uHash, const PblMatrix* pMatrix)
    {
        if (_Table.Store(uHash, pMatrix))
        {
            ++_iNumEntries;
        }
    }
    void Remove(unsigned int uHash)
    {
        if (_Table.Remove(uHash))
        {
            --_iNumEntries;
        }
    }

    template <int N> unsigned int ConvertFromZephyrPose(const ZephyrSkeleton<N>& kSkel);

    template <int N> unsigned int ConvertFromZephyrPose(const ZephyrPoseStatic<N>& kPose);

    void BindToSkeleton(RedSkeleton const* pSkeleton) const;
    void GetHardSkinMatrices(PblMatrix* pMatrices, unsigned char const* pIndices,
                             unsigned int uiCount) const;
    void GetSkinMatrices(PblMatrix* pMatrices, unsigned char const* pIndices,
                         unsigned int uiCount) const;

private:

    friend class RedModel;
    friend class RedStencilShadow;
    friend void RemoveChildren(const GameObject*, RedPose&, int);
    PblMatrix const& _GetBoneMatrix(int iBone) const;

public:
    int _iNumEntries;
    PblHashTable<const PblMatrix, 128> _Table;

    mutable RedSkeleton* m_pSkeleton;
};

 
