#include "SkyDome.h"

#include "SkyObject.h"
#include "GameLoop.h"
#include "PebbleFL/Common/PblChunk.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "PropModelMesh.h"
#include "RedEngineFL/Application/pc/pcFLRender.h"
#include "RedEngineFL/Graphics/PC/Shaders/pcTerrainShader2.h"

namespace SkyManager
{
    void Init();
    void Read(PblFileChunk& chunk);
    RedColor ReadColor(const PblConfig::Data& data);
    void ReadSkyInfo_(PblConfig& config, PblConfig::Data& data);

    void ReadSplitOptions(PblConfig& config);
    void ReadSkyInfo(PblConfig& config);

    void ReadSunInfo(PblConfig& config);
}

// FUNCTION: SWBF2 0x00638730
void SkyManager::Init()
{
    SkyDome::Create();
}

// FUNCTION: SWBF2 0x00638ab0
void SkyManager::ReadSplitOptions(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (GameLoop::mNumCameras > 1)
        {
            ReadSkyInfo_(config, data);
        }
    }
}

// FUNCTION: SWBF2 0x00638750
RedColor SkyManager::ReadColor(const PblConfig::Data& data)
{
    RedColor color;
    color.r = data.GetUint8Arg(0);
    color.g = data.GetUint8Arg(1);
    color.b = data.GetUint8Arg(2);
    color.a = data.GetNumArgs() > 3 ? data.GetUint8Arg(3) : 0xFF;
    return color;
}

// FUNCTION: SWBF2 0x006387f0
void SkyManager::ReadSkyInfo_(PblConfig& config, PblConfig::Data& data)
{
    switch (data.GetId())
    {
    case 0x23E10337u:
    {
        PblConfig scope(config.ReadNextScope(), true);
        ReadSplitOptions(scope);
        break;
    }
    case 0x023B1FCEu:
    {
        RedColor color = ReadColor(data);
        color.a = 0xFF;
        FLRenderer::SetFogColor(color);
        break;
    }
    case 0x01192B7Bu:
        if (data.GetNumArgs() == 1)
        {
            FLRenderer::SetFarSceneRange(data.GetFloatArg(0) * 0.5f, data.GetFloatArg(0) * 1.5f);
        }
        else if (data.GetNumArgs() == 2)
        {
            FLRenderer::SetFarSceneRange(data.GetFloatArg(0), data.GetFloatArg(1));
        }
        break;
    case 0x30C322FEu:
        PropModelMesh::SetDensity(data.GetFloatArg(0));
        break;
    case 0x50551C9Au:
        if (data.GetNumArgs() == 3)
        {
            FLRenderer::SetNearSceneRange(0.5f,
                                          data.GetFloatArg(1) - 100.0f, data.GetFloatArg(1) + 100.0f,
                                          data.GetFloatArg(2) - 100.0f, data.GetFloatArg(2) + 100.0f);
        }
        else if (data.GetNumArgs() == 4)
        {
            FLRenderer::SetNearSceneRange(0.5f, data.GetFloatArg(0), data.GetFloatArg(1),
                                          data.GetFloatArg(2), data.GetFloatArg(3));
        }
        else if (data.GetNumArgs() == 5)
        {
            FLRenderer::SetNearSceneRange(data.GetFloatArg(0), data.GetFloatArg(1),
                                          data.GetFloatArg(2), data.GetFloatArg(3),
                                          data.GetFloatArg(4));
        }
        break;
    case 0x89796252u:
        if (data.GetNumArgs() == 1)
        {
            pcTerrainShader2::SetEnabled(!data.GetBoolArg(0));
        }
        break;
    case 0x91B20CC9u:
        PropModelMesh::SetFadeAdj(data.GetFloatArg(0));
        break;
    case 0xB1BFDD49u:
        if (data.GetFloatArg(0) == 0.0f)
        {
            PropModelMesh::SetEnable(false);
        }
        break;
    case 0x91E9C28Bu:
    {
        RedColor color = ReadColor(data);
        color.a = 0xFF;
        FLRenderer::SetReflectionFogColor(color);
        break;
    }
    case 0xC15D0860u:
        FLRenderer::SetFadeAdjustWithZoom(data.GetInt32Arg(0) != 0);
        break;
    case 0xCF49A8D4u:
        FLRenderer::SetFogRange(data.GetFloatArg(0), data.GetFloatArg(1));
        break;
    case 0xD6C706F2u:
        FLRenderer::SetWorldFogRange(data.GetFloatArg(0), data.GetFloatArg(1));
        break;
    }
}

// FUNCTION: SWBF2 0x00638b10
void SkyManager::ReadSkyInfo(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        ReadSkyInfo_(config, data);
    }
}

// FUNCTION: SWBF2 0x00638b70
void SkyManager::ReadSunInfo(PblConfig& config)
{
    while (!config.IsDone())
    {
        config.ReadNextData();
    }
}

// FUNCTION: SWBF2 0x00638bc0
void SkyManager::Read(PblFileChunk& chunk)
{
    PblConfig config(chunk);
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3780C8F9u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            ReadSunInfo(scope);
            break;
        }
        case 0x06C0D3E6u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            ReadSkyInfo(scope);
            break;
        }
        case 0x185AB6C2u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            if (SkyDome::sInstance)
            {
                SkyDome::sInstance->Read(scope);
            }
            break;
        }
        case 0xB7E0C3F5u:
        {
            PblConfig scope(config.ReadNextScope(), true);
            SkyObjectManager::Load(scope);
            break;
        }
        case 0xA99AEB4Fu:
        {
            PblHash hash(data.GetStringArg(0));
            break;
        }
        default:
        {
            PblConfig scope(config.ReadNextScope(), true);
            break;
        }
        }
    }
}

namespace SkyManager {

// FUNCTION: SWBF2 0x00638740
void Reset()
{
    SkyDome::Destroy();
}

}
