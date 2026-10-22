#pragma once

#include <stddef.h>
#include "FLEffect.h"
#include "ParticleSpawner.h"
#include "ParticleTransformer.h"
#include "PebbleFL/Common/PblConfig.h"

class ParticleEmitterClass;
class ParticleGeometry;

class PE_ChunkLoader
{
public:

    static ParticleEmitterClass* Read(PblConfig& config);

    static ParticleEmitterClass* LoadEmitter(PblConfig& config);

    static void LoadTransformer(ParticleTransformer& transformer, PblConfig& config);
    static void LoadGeometry(ParticleGeometry& geometry, PblConfig& config);
    static ParticleTransformer::PositionTransformer* LoadPositionTrans(PblConfig& config);
    static ParticleTransformer::SizeTransformer* LoadSizeTrans(PblConfig& config);

    static ParticleTransformer::RotationTransformer* LoadRotationTrans(PblConfig& config);
    static ParticleTransformer::ColorTransformer* LoadColorTrans(PblConfig& config);
    static void LoadDataTrans(ParticleTransformer::DataTransformer& trans, PblConfig::Data& data);

    static void LoadSpawner(ParticleSpawner& spawner, PblConfig& config);
    static void LoadSpreadData(ParticleSpawner::SpreadData& data, PblConfig& config);
    static void LoadRotationData(ParticleSpawner::RotationData& data, PblConfig& config);
};

// VTABLE: SWBF2 0x007ad7b0
class ParticleEmitterEffect : public FLEffect
{
public:

    ParticleEmitterEffect()
        : FLEffect(0xCEBB215Fu, 3)
    {
    }

    virtual ~ParticleEmitterEffect();

    virtual void Render(int value);
    virtual FLEffectClass* BuildClass(PblConfig& config) const;

    static void Create();

    static void Destroy();

    static ParticleEmitterEffect* s_pInstance;
};
 
 