#pragma once

#ifndef SWBF2_GRAPHICS_PC_STATEMANAGER_DX_H
#define SWBF2_GRAPHICS_PC_STATEMANAGER_DX_H

#include <d3d9.h>
#include <new>
#include <stddef.h>
#include <stdio.h>

#include "RedEngineFL/Memory/RedMemory.h"
#include "RedEngineFL/Util/ext/basevector.h"
#include "RedEngineFL/Util/ext/types.h"

namespace util {

template <class ELEM, class KEY>
struct TKeyPair_
{
    TKeyPair_() : key_(0) {}
    TKeyPair_(ELEM const& elem, KEY key) : elem_(elem), key_(key) {}

    bool operator>=(TKeyPair_ const& rOther) const { return key_ >= rOther.key_; }

    ELEM elem_;
    KEY key_;
};

}

namespace dx {

class NVStateID
{
public:
    NVStateID() {}

    NVStateID(unsigned long type, unsigned long value0, unsigned long value1)
        : m_Type(type), m_dwValue0(value0), m_dwValue1(value1) {}

    unsigned long m_Type;
    union
    {
        unsigned long m_FVF;
        unsigned long m_dwValue0;
        unsigned long m_dwStage;
        _D3DRENDERSTATETYPE m_RenderStateType;
        _D3DTRANSFORMSTATETYPE m_TransformStateType;
    };
    union
    {
        unsigned long m_dwValue1;
        _D3DTEXTURESTAGESTATETYPE m_TextureStageStateType;
        _D3DSAMPLERSTATETYPE m_SamplerStateType;
    };
};

class NVStreamData
{
public:
    NVStreamData() : m_pStreamData(0), m_Offset(0), m_Stride(0) {}

    IDirect3DVertexBuffer9* m_pStreamData;
    unsigned int m_Offset;
    unsigned int m_Stride;
};

class NVIndexData
{
public:
    NVIndexData() : m_pIndexData(0), m_BaseVertexIndex(0) {}

    void* m_pIndexData;
    unsigned int m_BaseVertexIndex;
};

class StateManagerState
{
public:

    StateManagerState();

    void ResetState();

    void CaptureDeviceState(IDirect3DDevice9* pD3DDev);

    bool m_ValidRenderStates[256];
    unsigned long m_RenderStates[256];
    bool m_ValidTextureStates[128];

    unsigned long m_TextureStates[8][128];
    bool m_ValidSamplerStates[128];
    unsigned long m_SamplerStates[16][128];
    float m_VertexShaderConstants[1024][4];
    float m_PixelShaderConstants[1024][4];
    IDirect3DBaseTexture9* m_Textures[8];
    unsigned long m_FVF;
    IDirect3DPixelShader9* m_PixelShader;
    IDirect3DVertexShader9* m_VertexShader;
    IDirect3DVertexDeclaration9* m_VertexShaderDecl;
    float m_Matrices[300][16];
    _D3DMATERIAL9 m_D3DMaterial;
    NVStreamData m_StreamData[16];
    NVIndexData m_IndexData;
};

class StateManager_DX
{
public:

    StateManager_DX(IDirect3DDevice9* pD3DDev);

    virtual ~StateManager_DX();

    // SYNTHETIC: SWBF2 0x00715e10
    // dx::StateManager_DX::`scalar deleting destructor'

    static void CreateInstance(IDirect3DDevice9* pD3DDev);

    static StateManager_DX& GetInstance();

    void ResetAndCaptureStates();

    void SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix);

    void SetIndices(IDirect3DIndexBuffer9* pIndexData);
    void SetPixelShader(IDirect3DPixelShader9* pShader);
    void SetSamplerState(unsigned long ulSampler, D3DSAMPLERSTATETYPE eType,
                         unsigned long ulValue);
    void SetStreamSource(unsigned int uiStream, IDirect3DVertexBuffer9* pStreamData,
                         unsigned int uiOffset, unsigned int uiStride);
    bool SetTexture(unsigned long ulStage, IDirect3DBaseTexture9* pTexture);
    void SetTextureStageState(unsigned long ulStage, D3DTEXTURESTAGESTATETYPE eType,
                              unsigned long ulValue);
    void SetVertexShader(IDirect3DVertexShader9* pShader);

    void FlushState();
    void FlushStateEx_();

    long DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, unsigned int StartVertex,
                       unsigned int PrimitiveCount)
    {
        FlushState();
        return m_pD3DDev->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
    }
    long DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, unsigned int BaseVertexIndex,
                              unsigned int MinVertexIndex, unsigned int NumVertices,
                              unsigned int startIndex, unsigned int primCount)
    {
        FlushState();
        return m_pD3DDev->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices,
                                               startIndex, primCount);
    }

    StateManagerState m_BackupState;
    StateManagerState m_ChipState;
    StateManagerState m_CurrentState;
    StateManagerState m_CopyState;

    util::Vector<NVStateID> m_DirtyStates;
    util::Vector<NVStateID> renderStates_;
    util::Vector<NVStateID> textureStageStates_;
    util::Vector<NVStateID> samplerStates_;
    util::Vector<NVStateID> textureStates_;
    bool m_bForce;
    bool caching_;
    char pad_40932[2];
    IDirect3DDevice9* m_pD3DDev;
    D3DCAPS9 capabilities_;
};

extern StateManager_DX* localInstance_;

}

 
 
 

 
 
 
 
 
 
 
 
 

 
 
 
 
 

 
 
 
 
 
 
 
 
 
 
 

#endif
