#pragma once

#include <stddef.h>

#include "PebbleFL/Common/PblList.h"
#include "PebbleFL/Common/PblMatrix.h"
#include "PebbleFL/Common/PblVector.h"

class PblFileChunk;
class RedCamera;
class RedModel;
class RedPose;
class pcRedPrimitive;
class pcRedShader;
class ShadowRenderable;

class RedStencilShadow
{
public:

    enum VisibilityTestT
    {
        TEST_NOT_VISIBLE = 0,
        TEST_VISIBLE = 1,
        TEST_CONTAINS_CAMERA = 2
    };

    class ShadowSegment : public PblList<ShadowSegment>::Node
    {
    public:
        ShadowSegment();
        ~ShadowSegment();

        pcRedPrimitive* m_pPrimitive;
        unsigned char* m_puiBoneIndexMap;
        ShadowRenderable* m_pShadowRenderable;
        pcRedShader* m_pGPUShadowShader;
        pcRedShader* m_pCPUShadowShader;
        unsigned int m_uiBoneIndex;
        unsigned int m_uiNumBones;
        bool m_hardSkin;

        void Render(const PblMatrix* pMatrix, const PblMatrix* pBoneMatrices,
                    int iNumBones, unsigned int uiFlags) const;
    };

    RedStencilShadow();
    virtual ~RedStencilShadow();

    void Read(PblFileChunk& chunk);

    static bool GetShadowEnable();

    static void SetVisibilityTestCamera(const RedCamera* pCamera);

    static VisibilityTestT ShadowVolumeVisibilityTest(const PblVector3& vCenter,
                                                      float fRadius, float fLength);

    static PblVector4 s_aTestPlane[5];
    static PblVector3 s_vNearPlaneCenter;
    static PblVector3 s_vProjection;

    static float s_fNearPlaneRadius;
    static int s_iNumTestPlanes;
    static float s_fShadowVolumeLength;
    static float s_fShadowVolumeOffset;

    void Render(const PblMatrix* pMatrix, const PblMatrix** ppBoneMatrices,
                const RedPose* pPose, bool bFlag) const;

protected:
    void ReadCPUShadowData(PblFileChunk& chunk, ShadowSegment* pSegment);
    void ReadGPUShadowData(PblFileChunk& chunk, ShadowSegment* pSegment);

public:
    RedModel* m_pModel;
    PblList<ShadowSegment> m_segmentList;
public:
    static bool s_bShadowsEnabled;
};
 
 
 
 

struct ShadowData
{
public:

    struct ShadowEdge
    {
        unsigned short vertex;
        unsigned short next;
        unsigned short twin;
        unsigned char mark;
        unsigned char visible;
    };

    struct SilhouetteVertex
    {
        unsigned short vertex;
        unsigned char bStartSilhouette;
    };

    ShadowData();
    virtual ~ShadowData();

    void Read(PblFileChunk& chunk);
    void AddSilhouetteVertex(unsigned short uiVertex, bool bStartSilhouette);
    unsigned short GenerateSilhouette(const PblMatrix* pMatrix, const PblVector3& vLight);
    void GenerateSkin(const PblMatrix* pBoneMatrices, int iNumBoneMatrices, unsigned int uiMaxBones);

    unsigned short _iNumVertices;
    PblVector3* _pVertices;
    unsigned short _edgeCount;
    ShadowEdge* _edgeList;
    unsigned char* _puiSkinData;
    unsigned int _uiSkinNumBones;
    unsigned short _faceCount;
    unsigned short _triCount;
    unsigned short* _facePlanesEntry;
    float _fSizeInvSqr;
    PblVector4* _facePlanes;
    PblVector3* _pSkinnedVertices;
    unsigned short _silhouetteAllocated;
    unsigned short _silhouetteCount;
    SilhouetteVertex* _silhouette;

    static unsigned int _iShadowMem;
    static unsigned int _iShadowSilhouetteVertexCount;
};
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 
 

class ShadowRenderable
{
public:
    ShadowRenderable(ShadowData* pShadowData);
    ~ShadowRenderable();

private:
    void CreateSilhouette();
    void ProjectEdges();

public:
    static PblVector3 s_vProjectionUnlit;
    static PblVector3 s_vProjectionLit;
    void Render(pcRedShader* pShader, const PblMatrix& matrix,
                const PblMatrix* pBoneMatrices, int iNumBones, unsigned int uiFlags);
    void SetWorldMatrix(const PblMatrix& matrix);
    ShadowData* m_pShadowData;
    PblMatrix m_mWorldMatrix;
    PblVector3 m_vProjectionUnlitLocal;
    PblVector3 m_vProjectionLitLocal;
    bool m_bSilhouetteChanged;
    bool m_bProjectionChanged;
    int m_iNumSilhouetteVertices;
    int m_iNumSilhouetteVerticesAllocated;
    PblVector3* m_pSilhouetteVertices;
    int m_iNumIndicesCapLit;
    int m_iNumIndicesCapUnlit;
    unsigned short* m_pIndices;
};
 
 
 
