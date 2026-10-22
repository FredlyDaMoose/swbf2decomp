#include <string.h>

#include "StateManager_DX.h"

#include "RedEngineFL/Memory/RedMemory.h"

namespace dx {

// GLOBAL: SWBF2 0x009caafc
StateManager_DX* localInstance_;

// FUNCTION: SWBF2 0x00715ab0
StateManagerState::StateManagerState()
{
}

// FUNCTION: SWBF2 0x006b29a0
void StateManagerState::CaptureDeviceState(IDirect3DDevice9* pD3DDev)
{
    pD3DDev->GetFVF(&m_FVF);

    memset(m_ValidRenderStates, 0, sizeof(m_ValidRenderStates));
    memset(m_ValidTextureStates, 0, sizeof(m_ValidTextureStates));
    memset(m_ValidSamplerStates, 0, sizeof(m_ValidSamplerStates));

    for (int i = 7;   i <= 9;   i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 14;  i <= 16;  i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 19;  i <= 20;  i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 22;  i <= 29;  i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 34;  i <= 38;  i++)
    {
        m_ValidRenderStates[i] = true;
    }
    m_ValidRenderStates[48] = true;
    for (int i = 52;  i <= 60;  i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 128; i <= 137; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 139; i <= 143; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 145; i <= 148; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 151; i <= 152; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 154; i <= 163; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 165; i <= 168; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 170; i <= 176; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 178; i <= 195; i++)
    {
        m_ValidRenderStates[i] = true;
    }
    for (int i = 198; i <= 209; i++)
    {
        m_ValidRenderStates[i] = true;
    }

    for (int i = 1;  i <= 11; i++)
    {
        m_ValidTextureStates[i] = true;
    }
    for (int i = 22; i <= 24; i++)
    {
        m_ValidTextureStates[i] = true;
    }
    for (int i = 26; i <= 28; i++)
    {
        m_ValidTextureStates[i] = true;
    }
    m_ValidTextureStates[32] = true;

    for (int i = 1; i <= 13; i++)
    {
        m_ValidSamplerStates[i] = true;
    }

    for (unsigned long i = 1; i < 256; i++)
    {
        if (!m_ValidRenderStates[i])
        {
            continue;
        }
        if (pD3DDev->GetRenderState((D3DRENDERSTATETYPE)i, &m_RenderStates[i]) != D3D_OK)
        {
            m_RenderStates[i] = 0;
            m_ValidRenderStates[i] = false;
        }
    }

    for (unsigned long i = 0; i < 8; i++)
    {
        pD3DDev->GetTexture(i, &m_Textures[i]);
        if (m_Textures[i])
        {
            m_Textures[i]->Release();
        }
    }

    for (unsigned long uiStage = 0; uiStage < 8; uiStage++)
    {
        for (unsigned long uiType = 0; uiType < 128; uiType++)
        {
            if (m_ValidTextureStates[uiType])
            {
                pD3DDev->GetTextureStageState(uiStage, (D3DTEXTURESTAGESTATETYPE)uiType,
                                              &m_TextureStates[uiStage][uiType]);
            }
        }
    }

    for (unsigned long uiSampler = 0; uiSampler < 16; uiSampler++)
    {
        for (unsigned long uiType = 0; uiType < 128; uiType++)
        {
            if (m_ValidSamplerStates[uiType])
            {
                pD3DDev->GetSamplerState(uiSampler, (D3DSAMPLERSTATETYPE)uiType,
                                         &m_SamplerStates[uiSampler][uiType]);
            }
        }
    }
}

// FUNCTION: SWBF2 0x006b3240
void StateManagerState::ResetState()
{
    memset(m_RenderStates, 0, sizeof(m_RenderStates));
    memset(m_TextureStates, 0, sizeof(m_TextureStates));

    for (int i = 0; i < 16; i++)
    {
        m_StreamData[i].m_pStreamData = 0;
        m_StreamData[i].m_Offset = 0;
        m_StreamData[i].m_Stride = 0;
    }
    m_IndexData.m_pIndexData = 0;
    m_IndexData.m_BaseVertexIndex = 0;

    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            m_Matrices[i][j] = 0.0f;
        }
        m_Matrices[i][0] = 1.0f;
        m_Matrices[i][5] = 1.0f;
        m_Matrices[i][10] = 1.0f;
        m_Matrices[i][15] = 1.0f;
    }

    memset(m_VertexShaderConstants, 0, sizeof(m_VertexShaderConstants));
    memset(m_PixelShaderConstants, 0, sizeof(m_PixelShaderConstants));

    for (int i = 0; i < 8; i++)
    {
        m_Textures[i] = 0;
    }
    m_FVF = 0;
    m_PixelShader = 0;
    m_VertexShader = 0;
    m_VertexShaderDecl = 0;

    memset(m_Matrices, 0, sizeof(m_Matrices));
}

// FUNCTION: SWBF2 0x006b4130
void StateManager_DX::ResetAndCaptureStates()
{
    m_CurrentState.ResetState();
    m_CopyState.ResetState();
    m_ChipState.ResetState();

    m_CurrentState.CaptureDeviceState(m_pD3DDev);
    m_CopyState.CaptureDeviceState(m_pD3DDev);
    m_ChipState.CaptureDeviceState(m_pD3DDev);
}

// FUNCTION: SWBF2 0x00715b00
StateManager_DX::StateManager_DX(IDirect3DDevice9* pD3DDev)
{
    caching_ = true;

    m_pD3DDev = pD3DDev;
    m_pD3DDev->AddRef();

    m_CurrentState.ResetState();
    m_CopyState.ResetState();
    m_ChipState.ResetState();

    m_bForce = false;
}

// FUNCTION: SWBF2 0x00715c90
StateManager_DX::~StateManager_DX()
{
    if (m_pD3DDev)
    {
        m_pD3DDev->Release();
    }
    m_pD3DDev = 0;
}

// FUNCTION: SWBF2 0x00715e60
void StateManager_DX::CreateInstance(IDirect3DDevice9* pD3DDev)
{
    if (localInstance_)
    {
        delete localInstance_;
        localInstance_ = 0;
    }

    localInstance_ = new StateManager_DX(pD3DDev);
}

// FUNCTION: SWBF2 0x00715ef0
StateManager_DX& StateManager_DX::GetInstance()
{
    return *localInstance_;
}

}
