#pragma once

#include "PebbleFL/Common/PblHashTable.h"
#include "PebbleFL/Common/PblMatrix.h"

class PblFileChunk;
class RedPose;

class RedSkeleton
{
public:

    class Connection
    {
    public:
        char iParent;
        char iChild;
        char iSibling;
        char _reserved_;

        void Reset() { iParent = iChild = iSibling = -1; }
    };

    RedSkeleton()
        : m_amDefaultPose(0),
          m_amDefaultPoseInv(0),
          m_aConnections(0),
          m_auiNameHash(0),
          m_uiNumBones(0),
          m_uiNameHash(0)
    {
    }

    ~RedSkeleton();

    unsigned int GetNumBones() const { return m_uiNumBones; }
    int GetParentBone(unsigned int uiBone) const { return m_aConnections[uiBone].iParent; }

    static RedSkeleton* Read(PblFileChunk& chunk);

    void GetSkinMatrices(PblMatrix* pMatrices, const unsigned char* pIndices,
                         unsigned int uiCount) const;
    void SetupPose(RedPose& pose) const;

    PblMatrix* GetBoneMatrixByName(unsigned int uiName) const;

    PblMatrix* GetInvBoneMatrixByName(unsigned int uiName) const;

    unsigned int GetBoneNameHash(unsigned int uiBone) const { return m_auiNameHash[uiBone]; }
    const PblMatrix& GetDefaultPoseMatrix(unsigned int uiBone) const { return m_amDefaultPose[uiBone]; }

private:
    void _Read(PblFileChunk& chunk);

    unsigned int _PostProcess(const PblMatrix& mParent, unsigned int uiBone);

public:
    PblMatrix* m_amDefaultPose;
    PblMatrix* m_amDefaultPoseInv;
    Connection* m_aConnections;
    unsigned int* m_auiNameHash;
    unsigned int m_uiNumBones;
    unsigned int m_uiNameHash;

    static RedSkeleton* s_pCurrentSkeleton;

    static PblHashTable<RedSkeleton, 512> _HashTable;

    static const RedSkeleton* Find(unsigned int uiName) { return _HashTable.Find(uiName); }
};

 
