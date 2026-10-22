#pragma once

#include "RedEngineFL/Application/pc/pcRedParticle.h"

class ParticleEmitter;

namespace ParticleSystem
{

void Init();

bool PrepareForRender(ParticleEmitter* pEmitter, const PblVector3& vPos, int iUnused, float fAlpha,
                      unsigned int uiFlags, float fRadius, const PblVector3& vMin, const PblVector3& vMax,
                      float fMaxDist);
void FlushParticleCache();

extern float mMinParticleSize;
extern float mCacheBlurValue;
extern unsigned int uiRenderFlags;
extern int mCacheBlurParam;
extern float mFadeVal;
extern unsigned int mNumCachedParticles;
extern float mSizeMultiplier;
extern int mLod;
extern float mLodFadeVal;
extern int mCurrentLod;
extern RedBlendMode mCacheBlendMode;
extern unsigned int mCacheTexture;
extern PblBox mBoundingBox;

}
