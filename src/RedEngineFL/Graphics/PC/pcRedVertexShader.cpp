#include "pcRedVertexShader.h"

#include "pcRedRenderer.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Util/ext/Globals.h"

// FUNCTION: SWBF2 0x006fe5d0
void pcRedVertexShader::Read(PblFileChunk& chunk)
{
    const int iSize = chunk._pFile->Read32();
    if (iSize)
    {
        char* pcCode = new char[iSize];
        chunk._pFile->ReadData(pcCode, iSize);
        RedRenderer::pcGetDevice()->CreateVertexShader((const DWORD*)pcCode, &m_pD3DVertexShader);
        delete[] pcCode;

        static util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >& s_load =
            util::Globals::GetInstance().GetSet("load");
        static int& s_iVertexShaders = s_load.GetInt("VertexShaders");
        ++s_iVertexShaders;
    }
}

// FUNCTION: SWBF2 0x006fe5c0
pcRedVertexShader::~pcRedVertexShader()
{
    if (m_pD3DVertexShader)
    {
        m_pD3DVertexShader->Release();
    }
}
