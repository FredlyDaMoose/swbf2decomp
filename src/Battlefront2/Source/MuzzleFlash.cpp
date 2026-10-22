#include <math.h>

#include "MuzzleFlash.h"
#include "RedEngineFL/Application/pc/pcRedParticle.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x01e579f0
MuzzleFlashRenderer* sMuzzleFlashRenderer;

class Aimer
{
public:

    char mUnmodeledPrefix[0x88];
    PblVector3 mFirePos;
};

// FUNCTION: SWBF2 0x005a9f70
MuzzleFlashRenderer::MuzzleFlashRenderer()
{
    mNumMuzzleFlashes = 0;
    mCurrentBuffer = 0;
    mOldNumMuzzleFlashes = 0;
    Activate(250);
}

MuzzleFlashRenderer::~MuzzleFlashRenderer()
{
    Deactivate();
}

// SYNTHETIC: SWBF2 0x005aa670
// MuzzleFlashRenderer::`scalar deleting destructor'

void MuzzleFlashRenderer::RenderFlash(MuzzleFlashData const& data, RedColor const& color) const
{
    const PblVector3 vPos = data.m_pAimer->mFirePos;
    const PblVector3 vMin(vPos.x - 50.0f, vPos.y - 50.0f, vPos.z - 50.0f);
    const PblVector3 vMax(vPos.x + 50.0f, vPos.y + 50.0f, vPos.z + 50.0f);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;
    RedParticleRenderer::SubmitParticle(PIT_PARTICLE, vPos, color, data.mRadius, 1.0f, texCoords,
                                        gVectorUnitX, gVectorUnitY);
}

// FUNCTION: SWBF2 0x005a9fe0
void MuzzleFlashRenderer::RenderStreak(MuzzleFlashData const& data, RedColor const& color) const
{
    const PblVector3 vPos = data.m_pAimer->mFirePos;
    const PblVector3 vMin(vPos.x - 50.0f, vPos.y - 50.0f, vPos.z - 50.0f);
    const PblVector3 vMax(vPos.x + 50.0f, vPos.y + 50.0f, vPos.z + 50.0f);
    RedParticleRenderer::SubmitBounds(vMin, vMax);
    RedParticleRenderer::TexCoords texCoords;
    texCoords.u0 = 0;
    texCoords.v0 = 0;
    texCoords.u1 = 0xff;
    texCoords.v1 = 0xff;
    RedParticleRenderer::SubmitParticle(PIT_STREAK_HEAD, vPos, color, 0.15f, 0.0f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
    const float fHalfLength = data.mLength * 0.5f;
    const PblVector3 vMid(vPos.x + data.mDir.x * fHalfLength,
                          vPos.y + data.mDir.y * fHalfLength,
                          vPos.z + data.mDir.z * fHalfLength);
    RedParticleRenderer::SubmitParticle(PIT_STREAK, vMid, color, data.mWidth, 0.5f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
    const PblVector3 vEnd(vPos.x + data.mDir.x * data.mLength,
                          vPos.y + data.mDir.y * data.mLength,
                          vPos.z + data.mDir.z * data.mLength);
    RedParticleRenderer::SubmitParticle(PIT_STREAK_TAIL, vEnd, color, data.mWidth, 1.0f,
                                        texCoords, gVectorUnitX, gVectorUnitY);
}

// FUNCTION: SWBF2 0x005aa190
void MuzzleFlashRenderer::Render(int)
{
    if (mNumMuzzleFlashes == 0 && mOldNumMuzzleFlashes == 0)
    {
        return;
    }

    const PblMatrix mCamera = RedRenderer::GetCamera()->GetMatrix();
    RedParticleRenderer::SubmitTexture(0x504865B4u, BLEND_ADDITIVE, 0);

    for (int i = 0; i < mNumMuzzleFlashes; i++)
    {
        const MuzzleFlashData& data = mMuzzleFlashData[mCurrentBuffer][i];
        RedColor color = data.mColor;
        const float fDot = data.mDir.x * mCamera.m[2][0] + data.mDir.y * mCamera.m[2][1] +
                           data.mDir.z * mCamera.m[2][2];
        color.a = (unsigned char)(int)((1.0f - fabsf(fDot) * 0.3f) * 255.0f);
        if (color.a)
        {
            RenderStreak(data, color);
        }
    }
    for (int i = 0; i < mOldNumMuzzleFlashes; i++)
    {
        const MuzzleFlashData& data = mMuzzleFlashData[1 - mCurrentBuffer][i];
        RedColor color = data.mColor;
        const float fDot = data.mDir.x * mCamera.m[2][0] + data.mDir.y * mCamera.m[2][1] +
                           data.mDir.z * mCamera.m[2][2];
        color.a = (unsigned char)(int)((1.0f - fabsf(fDot) * 0.3f) * 128.0f);
        if (color.a)
        {
            RenderStreak(data, color);
        }
    }

    RedParticleRenderer::SubmitTexture(0xCFCCCBCEu, BLEND_ADDITIVE, 0);

    for (int i = 0; i < mNumMuzzleFlashes; i++)
    {
        const MuzzleFlashData& data = mMuzzleFlashData[mCurrentBuffer][i];
        RenderFlash(data, data.mColor);
    }
    for (int i = 0; i < mOldNumMuzzleFlashes; i++)
    {
        const MuzzleFlashData& data = mMuzzleFlashData[1 - mCurrentBuffer][i];
        RedColor color = data.mColor;
        color.a >>= 1;
        RenderFlash(data, color);
    }

    mCurrentBuffer = 1 - mCurrentBuffer;
    mOldNumMuzzleFlashes = mNumMuzzleFlashes;
    mNumMuzzleFlashes = 0;
}

// FUNCTION: SWBF2 0x005aa750
bool MuzzleFlash::Destroy()
{
    if (!sMuzzleFlashRenderer)
    {
        return false;
    }

    delete sMuzzleFlashRenderer;
    sMuzzleFlashRenderer = 0;
    return true;
}

namespace MuzzleFlash {

// FUNCTION: SWBF2 0x005aa6e0
bool Init()
{
    if (sMuzzleFlashRenderer)
    {
        return false;
    }
    sMuzzleFlashRenderer = new MuzzleFlashRenderer;
    return true;
}

}
