#include "SkyStars.h"
#include "PebbleFL/Common/PblVector.h"

#include <math.h>
#include <string.h>

#include <d3dx9math.h>

#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblMath.h"
#include "PebbleFL/Common/PblRandom.h"
#include "RedEngineFL/Graphics/RedCamera.h"
#include "CameraManager.h"

namespace FLRenderer
{

unsigned int GetCurrentViewport();
}

static const PblMatrix _IdentityMatrix(
    PblVector4{ 1.0f, 0.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 1.0f, 0.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 1.0f, 0.0f },
    PblVector4{ 0.0f, 0.0f, 0.0f, 1.0f });

// FUNCTION: SWBF2 0x00639960
SkyStars::SkyStars()
    : m_uiNumStars(0), m_uiRandomSeed(0), m_fBrightStarRatio(0.08f),
      m_fMinBrightness(0.05f), m_fColorSaturation(1.0f), m_color(RedColor::WHITE),
      m_fTwinkleFactor(0.0f), m_fTwinkleFrequency(0.0f), m_fTwinkleTimer(0.0f),
      m_fSinAngleMin(-1.0f), m_fSinAngleFade(-1.0f), m_bRenderBottom(false),
      m_pShader(0)
{
    memset(m_aStarFields, 0, sizeof(m_aStarFields));
}

// SYNTHETIC: SWBF2 0x00639a10
// SkyStars::`scalar deleting destructor'

// FUNCTION: SWBF2 0x00639a40
SkyStars::~SkyStars()
{
    DeactivateThread();
    _PlatformCleanup();
    for (int i = 0; i < NUM_STAR_FIELDS; ++i)
    {
        delete[] m_aStarFields[i].aStar;
    }
    memset(m_aStarFields, 0, sizeof(m_aStarFields));
}

namespace {

inline unsigned char ClampColorComponent(float f)
{
    if (f < 0.0f)
    {
        f = 0.0f;
    }
    else if (f > 255.0f)
    {
        f = 255.0f;
    }
    return (unsigned char)(int)f;
}
}

// FUNCTION: SWBF2 0x00639ad0
void SkyStars::Read(PblConfig& config)
{
    m_uiNumStars = 0;
    m_uiRandomSeed = 0;
    m_fBrightStarRatio = 0.08f;
    m_fMinBrightness = 0.05f;
    m_fColorSaturation = 1.0f;
    m_color = RedColor::WHITE;
    m_fTwinkleFactor = 0.5f;
    m_fTwinkleFrequency = 20.0f;
    m_fTwinkleTimer = 0.0f;
    m_fSinAngleMin = -1.0f;
    m_fSinAngleFade = -1.0f;
    m_bRenderBottom = false;

    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x836BDE54u:
            m_uiNumStars = data.GetUint32Arg(0);
            break;
        case 0x69BF5463u:
            m_uiRandomSeed = data.GetUint32Arg(0);
            break;
        case 0x736837DEu:
            m_fBrightStarRatio = data.GetFloatArg(0) / 100.0f;
            break;
        case 0x7DE4197Du:
            m_fMinBrightness = data.GetFloatArg(0) / 255.0f;
            break;
        case 0x0D7EC5C0u:
            m_fColorSaturation = data.GetFloatArg(0);
            break;
        case 0x3D7E6258u:
            if (data.GetNumArgs() >= 3)
            {
                m_color.r = ClampColorComponent(data.GetFloatArg(0));
                m_color.g = ClampColorComponent(data.GetFloatArg(1));
                m_color.b = ClampColorComponent(data.GetFloatArg(2));
                if (data.GetNumArgs() >= 4)
                {
                    m_color.a = ClampColorComponent(data.GetFloatArg(3));
                }
            }
            break;
        case 0xA435AAA4u:
            m_fTwinkleFactor = data.GetFloatArg(0);
            break;
        case 0xA9EEE423u:
            m_fTwinkleFrequency = data.GetFloatArg(0);
            break;
        case 0x1EE51ED4u:
            m_fSinAngleMin = PblMath::Sin(data.GetFloatArg(0) * 0.017453292f);
            break;
        case 0xF947A04Eu:
            m_fSinAngleFade = PblMath::Sin(data.GetFloatArg(0) * 0.017453292f);
            break;
        case 0x02FF3675u:
            m_bRenderBottom = (int)data.GetFloatArg(0) != 0;
            break;
        }
    }

    if (m_uiNumStars)
    {
        _BuildStarFields();
        _PlatformInit();
        ActivateThread(0, PER_FRAME);
    }
}

// FUNCTION: SWBF2 0x00639e00
void SkyStars::_BuildStarFields()
{
    static const float fBand2Top = (float)sqrt(0.75);
    static const float fBand1Ratio = 0.5f / 16;
    static const float fBand2Ratio = (fBand2Top - 0.5f) / 16;
    static const float fBand3Ratio = (1.0f - fBand2Top) / 4;
    static const float fSector12 = PblMath::fTwoPi / 16;
    static const float fSector3 = PblMath::fTwoPi / 4;

    PblRandom rand(m_uiRandomSeed ^ 0xC71C2830u);

    float fRemainder = 0.0f;
    unsigned int i = 0;

    const float fBand1Stars = (float)m_uiNumStars * fBand1Ratio;
    for (; i < 16; ++i)
    {
        StarField& field = m_aStarFields[i];
        const float fStars = fBand1Stars + fRemainder;
        field.uiNumStars = (unsigned int)floor(fStars + 0.5f);
        fRemainder = fStars - (float)field.uiNumStars;
        if (!field.uiNumStars)
        {
            field.aStar = NULL;
            continue;
        }
        field.aStar = new Star[field.uiNumStars];
        const float fBaseAngle = (float)i * fSector12;
        for (unsigned int j = 0; j < field.uiNumStars; ++j)
        {
            Star& star = field.aStar[j];
            const float fAngle = rand.GetFloat();
            const float fHeight = rand.GetFloat() * 0.5f;
            const float fRadius = (float)sqrt(1.0f - fHeight * fHeight);
            float fSin, fCos;
            PblMath::SinCos(&fSin, &fCos, fAngle * fSector12 + fBaseAngle);
            star.vPos.x = fCos * fRadius;
            star.vPos.y = fHeight;
            star.vPos.z = fSin * fRadius;
            const float fColor = rand.GetFloat();
            star.color = _GetStarColor(rand.GetFloat(), fColor);
            if (m_fSinAngleFade > fHeight)
            {
                const float fFade = fHeight > m_fSinAngleMin
                                        ? (fHeight - m_fSinAngleMin) / (m_fSinAngleFade - m_fSinAngleMin)
                                        : 0.0f;
                star.color.a = (unsigned char)(int)((float)star.color.a * fFade);
            }
        }
    }

    const float fBand2Stars = (float)m_uiNumStars * fBand2Ratio;
    for (; i < 32; ++i)
    {
        StarField& field = m_aStarFields[i];
        const float fStars = fBand2Stars + fRemainder;
        field.uiNumStars = (unsigned int)floor(fStars + 0.5f);
        fRemainder = fStars - (float)field.uiNumStars;
        if (!field.uiNumStars)
        {
            field.aStar = NULL;
            continue;
        }
        field.aStar = new Star[field.uiNumStars];
        const float fBaseAngle = (float)(i - 16) * fSector12;
        for (unsigned int j = 0; j < field.uiNumStars; ++j)
        {
            Star& star = field.aStar[j];
            const float fAngle = rand.GetFloat();
            const float fHeight = (fBand2Top - 0.5f) * rand.GetFloat() + 0.5f;
            const float fRadius = (float)sqrt(1.0f - fHeight * fHeight);
            float fSin, fCos;
            PblMath::SinCos(&fSin, &fCos, fAngle * fSector12 + fBaseAngle);
            star.vPos.x = fCos * fRadius;
            star.vPos.y = fHeight;
            star.vPos.z = fSin * fRadius;
            const float fColor = rand.GetFloat();
            star.color = _GetStarColor(rand.GetFloat(), fColor);
            if (m_fSinAngleFade > fHeight)
            {
                const float fFade = fHeight > m_fSinAngleMin
                                        ? (fHeight - m_fSinAngleMin) / (m_fSinAngleFade - m_fSinAngleMin)
                                        : 0.0f;
                star.color.a = (unsigned char)(int)((float)star.color.a * fFade);
            }
        }
    }

    const float fBand3Stars = (float)m_uiNumStars * fBand3Ratio;
    for (; i < NUM_STAR_FIELDS; ++i)
    {
        StarField& field = m_aStarFields[i];
        const float fStars = fBand3Stars + fRemainder;
        field.uiNumStars = (unsigned int)floor(fStars + 0.5f);
        fRemainder = fStars - (float)field.uiNumStars;
        if (!field.uiNumStars)
        {
            field.aStar = NULL;
            continue;
        }
        field.aStar = new Star[field.uiNumStars];
        const float fBaseAngle = (float)(i - 32) * fSector3;
        for (unsigned int j = 0; j < field.uiNumStars; ++j)
        {
            Star& star = field.aStar[j];
            const float fAngle = rand.GetFloat();
            const float fHeight = (1.0f - fBand2Top) * rand.GetFloat() + fBand2Top;
            const float fRadius = (float)sqrt(1.0f - fHeight * fHeight);
            float fSin, fCos;
            PblMath::SinCos(&fSin, &fCos, fAngle * fSector3 + fBaseAngle);
            star.vPos.x = fCos * fRadius;
            star.vPos.y = fHeight;
            star.vPos.z = fSin * fRadius;
            const float fColor = rand.GetFloat();
            star.color = _GetStarColor(rand.GetFloat(), fColor);
            if (m_fSinAngleFade > fHeight)
            {
                const float fFade = fHeight > m_fSinAngleMin
                                        ? (fHeight - m_fSinAngleMin) / (m_fSinAngleFade - m_fSinAngleMin)
                                        : 0.0f;
                star.color.a = (unsigned char)(int)((float)star.color.a * fFade);
            }
        }
    }
}

// FUNCTION: SWBF2 0x0063a930
RedColor SkyStars::_GetStarColor(float fBrightness, float fColor) const
{
    const float fDimRange = 1.0f - m_fBrightStarRatio;
    float fIntensity;
    if (fBrightness > fDimRange)
    {
        fIntensity = 1.0f;
    }
    else
    {
        const float t = fBrightness / fDimRange;
        const float t3 = t * t * t;
        fIntensity = (1.0f - t3) * m_fMinBrightness + t3;
    }

    float fR = fIntensity;
    float fG = fIntensity;
    float fB = fIntensity;
    if (fColor > 0.7f)
    {
        fR = (1.0f - (fColor - 0.7f) * m_fColorSaturation / 0.3f) * fIntensity;
        if (fColor > 0.85f)
        {
            fG = (1.0f - (fColor - 0.85f) * m_fColorSaturation * 2.0f / 0.3f) * fIntensity;
        }
    }
    if (fColor < 0.4f)
    {
        fB = fIntensity * (1.0f - (0.4f - fColor) * m_fColorSaturation / 0.4f);
        if (fColor < 0.26668f)
        {
            fG = fG * (1.0f - (0.26668f - fColor) * m_fColorSaturation * 1.5f / 0.4f);
        }
    }

    RedColor color;
    color.a = 255;
    color.b = (unsigned char)(int)(fB * 255.0f);
    color.g = (unsigned char)(int)(fG * 255.0f);
    color.r = (unsigned char)(int)(fR * 255.0f);
    return color;
}

// FUNCTION: SWBF2 0x0063aa70
void SkyStars::Render(unsigned int uiFlags) const
{
    static const float fBand2Top = (float)sqrt(0.75);

    if (!m_uiNumStars)
    {
        return;
    }

    CameraManager* pCameraManager = CameraManager::sInstance;
    RedCamera* pCamera = pCameraManager->GetCamera(
        pCameraManager->GetCameraIdForView(FLRenderer::GetCurrentViewport()));

    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);
    const float fTanHalfWidth = fWidth * 0.5f / fNear;
    const float fTanHalfHeight = fHeight * 0.5f / fNear;

    const PblMatrix* apMatrices[2] = { NULL, NULL };
    const PblMatrix& mCamera = pCamera->GetMatrix();

    if (!m_bRenderBottom)
    {
        const float fRight = mCamera.m[0][1] * fTanHalfWidth;
        const float fUp = mCamera.m[1][1] * fTanHalfHeight;
        const float fFront = -mCamera.m[2][1];
        if (fFront + fRight + fUp < 0.0f && fFront - fRight + fUp < 0.0f &&
            fFront + fRight - fUp < 0.0f && fFront - fRight - fUp < 0.0f)
        {
            return;
        }
    }

    const float fScale = fNear * 2.0f;
    PblMatrix mScale;

    PblMatrix mTop = _IdentityMatrix;
    D3DXMatrixScaling((D3DXMATRIX*)&mScale, fScale, fScale, fScale);
    D3DXMatrixMultiply((D3DXMATRIX*)&mTop, (D3DXMATRIX*)&mTop, (D3DXMATRIX*)&mScale);
    mTop.m[3][0] = mCamera.m[3][0];
    mTop.m[3][1] = mCamera.m[3][1];
    mTop.m[3][2] = mCamera.m[3][2];
    mTop.m[3][3] = 1.0f;

    PblMatrix mBottom;
    mBottom.m[0][0] = 1.0f; mBottom.m[0][1] = 0.0f;  mBottom.m[0][2] = 0.0f; mBottom.m[0][3] = 0.0f;
    mBottom.m[1][0] = 0.0f; mBottom.m[1][1] = -1.0f; mBottom.m[1][2] = 0.0f; mBottom.m[1][3] = 0.0f;
    mBottom.m[2][0] = 0.0f; mBottom.m[2][1] = 0.0f;  mBottom.m[2][2] = 1.0f; mBottom.m[2][3] = 0.0f;
    mBottom.m[3][0] = 0.0f; mBottom.m[3][1] = 0.0f;  mBottom.m[3][2] = 0.0f; mBottom.m[3][3] = 1.0f;
    D3DXMatrixScaling((D3DXMATRIX*)&mScale, fScale, fScale, fScale);
    D3DXMatrixMultiply((D3DXMATRIX*)&mBottom, (D3DXMATRIX*)&mBottom, (D3DXMATRIX*)&mScale);
    mBottom.m[3][0] = mCamera.m[3][0];
    mBottom.m[3][1] = mCamera.m[3][1];
    mBottom.m[3][2] = mCamera.m[3][2];
    mBottom.m[3][3] = 1.0f;

    PblVector3 aBand1[5], aBand1Bottom[5];
    PblVector3 aBand2[5], aBand2Bottom[5];
    PblVector3 aBand3[10], aBand3Bottom[10];
    aBand1[0] = PblVector3(1.0f, 0.0f, 0.0f);
    aBand1[1] = PblVector3(fBand2Top, 0.5f, 0.0f);
    aBand1Bottom[0] = PblVector3(1.0f, 0.0f, 0.0f);
    aBand1Bottom[1] = PblVector3(fBand2Top, -0.5f, 0.0f);
    aBand2[0] = PblVector3(fBand2Top, 0.5f, 0.0f);
    aBand2[1] = PblVector3(0.5f, fBand2Top, 0.0f);
    aBand2Bottom[0] = PblVector3(fBand2Top, 0.5f, 0.0f);
    aBand2Bottom[1] = PblVector3(0.5f, -fBand2Top, 0.0f);
    aBand3[0] = PblVector3(0.0f, 1.0f, 0.0f);
    aBand3[1] = PblVector3(0.5f, fBand2Top, 0.0f);
    aBand3Bottom[0] = PblVector3(0.0f, -1.0f, 0.0f);
    aBand3Bottom[1] = PblVector3(0.5f, -fBand2Top, 0.0f);

    PblMatrix mView;
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 4; ++c)
        {
            mView.m[r][c] = mCamera.m[r][c];
        }
    }
    mView.m[3][0] = 0.0f;
    mView.m[3][1] = 0.0f;
    mView.m[3][2] = 0.0f;
    mView.m[3][3] = 1.0f;

    RedCamera camera(*pCamera);
    camera.SetNearFarPlane(0.1f, 10.0f);
    camera.SetMatrix(mView);

    static const float fSector = PblMath::fTwoPi / 16;

    for (int i = 0; i < 16; ++i)
    {
        float fSin, fCos, fMidSin, fMidCos;
        PblMath::SinCos(&fSin, &fCos, (float)(i + 1) * fSector);
        PblMath::SinCos(&fMidSin, &fMidCos, ((float)i + 0.5f) * fSector);

        aBand1[2] = PblVector3(fCos, 0.0f, fSin);
        aBand1[3] = PblVector3(fCos * fBand2Top, 0.5f, fSin * fBand2Top);
        aBand1[4] = PblVector3(fMidCos, 0.0f, fMidSin);
        aBand1Bottom[2] = PblVector3(fCos, 0.0f, fSin);
        aBand1Bottom[3] = PblVector3(fCos * fBand2Top, -0.5f, fSin * fBand2Top);
        aBand1Bottom[4] = PblVector3(fMidCos, 0.0f, fMidSin);

        const StarField& field1 = m_aStarFields[i];
        int iNumMatrices = 0;
        if (field1.uiNumStars && camera.ClassifyPoints(aBand1, 5) != RedCamera::VISIBILITY_NONE)
        {
            apMatrices[iNumMatrices++] = &mTop;
        }
        if (m_bRenderBottom && field1.uiNumStars &&
            camera.ClassifyPoints(aBand1Bottom, 5) != RedCamera::VISIBILITY_NONE)
        {
            apMatrices[iNumMatrices++] = &mBottom;
        }
        if (iNumMatrices > 0)
        {
            _PlatformRenderField(field1, uiFlags, apMatrices, iNumMatrices);
        }

        aBand2[2] = PblVector3(fCos * fBand2Top, 0.5f, fSin * fBand2Top);
        aBand2[3] = PblVector3(fCos * 0.5f, fBand2Top, fSin * 0.5f);
        aBand2[4] = PblVector3(fMidCos * fBand2Top, 0.5f, fMidSin * fBand2Top);
        aBand2Bottom[2] = PblVector3(fCos * fBand2Top, -0.5f, fSin * fBand2Top);
        aBand2Bottom[3] = PblVector3(fCos * 0.5f, -fBand2Top, fSin * 0.5f);
        aBand2Bottom[4] = PblVector3(fMidCos * fBand2Top, -0.5f, fMidSin * fBand2Top);

        const StarField& field2 = m_aStarFields[16 + i];
        iNumMatrices = 0;
        if (field2.uiNumStars && camera.ClassifyPoints(aBand2, 5) != RedCamera::VISIBILITY_NONE)
        {
            apMatrices[iNumMatrices++] = &mTop;
        }
        if (m_bRenderBottom && field2.uiNumStars &&
            camera.ClassifyPoints(aBand2Bottom, 5) != RedCamera::VISIBILITY_NONE)
        {
            apMatrices[iNumMatrices++] = &mBottom;
        }
        if (iNumMatrices > 0)
        {
            _PlatformRenderField(field2, uiFlags, apMatrices, iNumMatrices);
        }

        const int iSub = i & 3;
        aBand3[2 + 2 * iSub] = PblVector3(fMidCos * 0.5f, fBand2Top, fMidSin * 0.5f);
        aBand3[3 + 2 * iSub] = PblVector3(fCos * 0.5f, fBand2Top, fSin * 0.5f);
        aBand3Bottom[2 + 2 * iSub] = PblVector3(fMidCos * 0.5f, -fBand2Top, fMidSin * 0.5f);
        aBand3Bottom[3 + 2 * iSub] = PblVector3(fCos * 0.5f, -fBand2Top, fSin * 0.5f);
        if (iSub == 3)
        {
            const StarField& field3 = m_aStarFields[32 + (i >> 2)];
            iNumMatrices = 0;
            if (field3.uiNumStars && camera.ClassifyPoints(aBand3, 10) != RedCamera::VISIBILITY_NONE)
            {
                apMatrices[iNumMatrices++] = &mTop;
            }
            if (m_bRenderBottom && field3.uiNumStars &&
                camera.ClassifyPoints(aBand3Bottom, 10) != RedCamera::VISIBILITY_NONE)
            {
                apMatrices[iNumMatrices++] = &mBottom;
            }
            if (iNumMatrices > 0)
            {
                _PlatformRenderField(field3, uiFlags, apMatrices, iNumMatrices);
            }
        }

        aBand1[0] = aBand1[2];
        aBand1[1] = aBand1[3];
        aBand1Bottom[0] = aBand1Bottom[2];
        aBand1Bottom[1] = aBand1Bottom[3];
        aBand2[0] = aBand2[2];
        aBand2[1] = aBand2[3];
        aBand2Bottom[0] = aBand2Bottom[2];
        aBand2Bottom[1] = aBand2Bottom[3];
    }
}

// FUNCTION: SWBF2 0x0063b660
bool SkyStars::Update(float fDeltaTime)
{
    m_fTwinkleTimer += fDeltaTime;
    return true;
}
