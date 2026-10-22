#pragma once

#include <d3d9.h>

class PblFileChunk;

class pcRedPixelShader
{
public:
    pcRedPixelShader() : m_pD3DPixelShader(0) {}

    ~pcRedPixelShader();

    void Read(PblFileChunk& chunk);

    IDirect3DPixelShader9* m_pD3DPixelShader;
};
 
