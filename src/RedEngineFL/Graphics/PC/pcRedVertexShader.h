#pragma once

#include <d3d9.h>

class PblFileChunk;

class pcRedVertexShader
{
public:
    pcRedVertexShader() : m_pD3DVertexShader(0) {}

    ~pcRedVertexShader();

    void Read(PblFileChunk& chunk);

    IDirect3DVertexShader9* m_pD3DVertexShader;
};
 
