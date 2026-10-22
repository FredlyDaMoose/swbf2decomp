#include "pcRedPixelShader.h"

#include "pcRedRenderer.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblFile.h"
#include "RedEngineFL/Util/ext/Globals.h"

// FUNCTION: SWBF2 0x006fe6e0
void pcRedPixelShader::Read(PblFileChunk& chunk)
{
    const unsigned int uiSize = (unsigned int)chunk._pFile->Read32();
    DWORD* pdwCode = new DWORD[uiSize];
    chunk._pFile->ReadData(pdwCode, (int)uiSize);
    RedRenderer::pcGetDevice()->CreatePixelShader(pdwCode, &m_pD3DPixelShader);
    delete[] pdwCode;

    static util::VariableSet<util::SingletonAllocator<util::DefaultStaticAllocator> >& s_load =
        util::Globals::GetInstance().GetSet("load");
    static int& s_iPixelShaders = s_load.GetInt("PixelShaders");
    ++s_iPixelShaders;
}

// FUNCTION: SWBF2 0x006fe6d0
pcRedPixelShader::~pcRedPixelShader()
{
    if (m_pD3DPixelShader)
    {
        m_pD3DPixelShader->Release();
    }
}
