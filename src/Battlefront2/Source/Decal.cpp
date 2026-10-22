#include <math.h>

#include "Decal.h"
#include "PebbleFL/Common/PblConfig.h"
#include "PebbleFL/Common/PblHashTable.h"
#include "RedEngineFL/Graphics/PC/pcRedAutoSettings.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/PC/pcRedShader.h"

// FUNCTION: SWBF2 0x0048aaf0
FLEffectObject* DecalClass::BuildEffect()
{
    return 0;
}

// FUNCTION: SWBF2 0x0048b4e0
bool DecalClass::IsBumped()
{
    return m_bumpTextureHash != 0;
}

// GLOBAL: SWBF2 0x01eb97b8
// SYNTHETIC: SWBF2 0x004027d0
// `dynamic initializer for 'Decal::sMemoryPool''
// SYNTHETIC: SWBF2 0x00766ec0
// `dynamic atexit destructor for 'Decal::sMemoryPool''
MemoryPool Decal::sMemoryPool("Decal", 0x40);

// GLOBAL: SWBF2 0x01e30944
DecalManager* DecalManager::s_pInstance;

// FUNCTION: SWBF2 0x0048ab00
void DecalManager::Create()
{
    if (!s_pInstance)
    {
        s_pInstance = new DecalManager();
    }
}

// FUNCTION: SWBF2 0x0048ab70
void DecalManager::Destroy()
{
    if (s_pInstance)
    {
        delete s_pInstance;
    }
    s_pInstance = 0;
}

// FUNCTION: SWBF2 0x0048ab90
DecalManager::DecalManager()
    : FLEffect(0xDE15F6AEu, 5)
{
    Enable(true);
}

// FUNCTION: SWBF2 0x0048aca0
void DecalManager::Init()
{
    m_numDecals = 0;
    Decal::sMemoryPool.Create(0, Decal::sMemoryPool.GetSize(), Decal::sMemoryPool.GetGrow());
}

// FUNCTION: SWBF2 0x0048ac50
DecalManager::~DecalManager()
{
}

// FUNCTION: SWBF2 0x0048acc0
void DecalManager::Cleanup()
{
}

// FUNCTION: SWBF2 0x0048acd0
void DecalManager::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        if (data.GetId() == 0xAF8BB8CEu)
        {
            Enable(data.GetUint32Arg(0) != 0);
        }
    }
}

// FUNCTION: SWBF2 0x0048ad50
FLEffectClass* DecalManager::BuildClass(PblConfig& config) const
{
    return new DecalClass(config);
}

// FUNCTION: SWBF2 0x0048adb0
void DecalManager::AddClass(DecalClass* pClass)
{
    m_decalClasses.AddLast(pClass, pClass);
}

// FUNCTION: SWBF2 0x0048adf0
void DecalManager::RemoveClass(DecalClass* pClass)
{
    if (pClass->IsInList())
    {
        m_decalClasses.Remove(pClass);
    }
}

// FUNCTION: SWBF2 0x0048ae30
void DecalManager::Update(float fDeltaTime)
{
    for (PblList<DecalClass>::Itor itClass(m_decalClasses); itClass != m_decalClasses.End();
         ++itClass)
    {
        DecalClass* pClass = itClass;
        PblList<Decal>::Itor it(pClass->m_decals);
        while (it != pClass->m_decals.End())
        {
            Decal* pDecal = it;
            ++it;

            pDecal->fTime += fDeltaTime;
            if (pClass->m_fadeInTime > 0.0f && pDecal->fTime < pClass->m_fadeInTime)
            {
                pDecal->color.a =
                    (unsigned char)(int)(pDecal->fTime / pClass->m_fadeInTime * 255.0f);
            }
            else if (pDecal->fTime > pClass->m_lifeTime)
            {
                pClass->m_decals.Remove(pDecal);
                delete pDecal;
                --m_numDecals;
            }
            else
            {
                float fRemaining = pClass->m_lifeTime - pDecal->fTime;
                if (fRemaining < pClass->m_fadeOutTime)
                {
                    pDecal->color.a =
                        (unsigned char)(int)(fRemaining / pClass->m_fadeOutTime * 255.0f);
                }
            }
        }
    }
}

// FUNCTION: SWBF2 0x0048af30
void DecalManager::Render(int value)
{
    const RedCamera* pCamera = RedRenderer::GetCamera();
    float fNear, fFar, fWidth, fHeight;
    pCamera->GetFrustum(fNear, fFar, fWidth, fHeight);

    for (PblList<DecalClass>::Itor itClass(m_decalClasses); itClass != m_decalClasses.End();
         ++itClass)
    {
        itClass->m_totalFrameVertices = 0;
        itClass->m_totalFrameIndices = 0;
        itClass->m_totalVisible = 0;
        for (PblList<Decal>::Itor it(itClass->m_decals); it != itClass->m_decals.End(); ++it)
        {
            Decal* pDecal = it;
            pDecal->visibility = pCamera->ClassifySphere(pDecal->vPos, pDecal->fRadius, -1.0f);
            pDecal->fadeVal = 0.0f;
            if (pDecal->visibility == RedCamera::VISIBILITY_NONE)
            {
                continue;
            }

            const PblMatrix& mCamera = RedRenderer::GetCamera()->GetMatrix();
            const float fX = pDecal->vPos.x - mCamera.m[3][0];
            const float fY = pDecal->vPos.y - mCamera.m[3][1];
            const float fZ = pDecal->vPos.z - mCamera.m[3][2];
            const float fDist = sqrtf(fX * fX + fY * fY + fZ * fZ);

            static float sFadeRange = 50.0f;
            if (fDist < 100.0f)
            {
                itClass->m_totalFrameVertices += pDecal->numVertices;
                itClass->m_totalFrameIndices += pDecal->numIndices;
                ++itClass->m_totalVisible;
                float fFade = (100.0f - fDist) / sFadeRange;
                if (fFade < 0.0f)
                {
                    fFade = 0.0f;
                }
                else if (fFade > 1.0f)
                {
                    fFade = 1.0f;
                }
                pDecal->fadeVal = fFade;
            }
        }
    }
    PlatformRender(value);
}

// FUNCTION: SWBF2 0x0048b0d0
Decal::~Decal()
{
    delete[] vertices;
    delete[] indices;
}

// FUNCTION: SWBF2 0x0048b120
DecalClass::DecalClass(PblConfig& config)
{
    m_lifeTime = 10.0f;
    m_fadeInTime = 0.0f;
    m_fadeOutTime = 6.0f;
    m_color = RedColor::WHITE;
    m_maxDecals = 40;
    m_textureHash = 0;
    m_bumpTextureHash = 0;
    ReadConfig(config);

    unsigned int auTextures[2];
    auTextures[0] = m_textureHash;
    auTextures[1] = IsBumped() ? m_bumpTextureHash : 0;
    if (IsBumped() && RedRenderer::pcGetRenderCaps().pipeline == PIPELINE_FIXED)
    {
        m_pShader = 0;
    }
    else
    {
        m_pShader = pcRedShader::Create(0xDE15F6AEu, 0x282, auTextures, 2, 0x10004,
                                        RedColor::WHITE, RedColor::BLACK, 32.0f);
        DecalManager::s_pInstance->AddClass(this);
    }
}

// FUNCTION: SWBF2 0x0048b260
DecalClass::~DecalClass()
{
    DecalManager::s_pInstance->RemoveClass(this);
    while (m_decals.GetCount())
    {
        PblList<Decal>::Node* pNode = m_decals.GetFirst();
        Decal* pDecal = pNode->_pObject;
        m_decals.Remove(pNode);
        delete pDecal;
    }
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

// FUNCTION: SWBF2 0x0048b310
void DecalClass::ReadConfig(PblConfig& config)
{
    while (!config.IsDone())
    {
        PblConfig::Data data = config.ReadNextData();
        switch (data.GetId())
        {
        case 0x3C6468F4u:
            m_textureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x08AF6C24u:
            m_bumpTextureHash = PblHash(data.GetStringArg(0));
            break;
        case 0x0786DC43u:
            m_maxDecals = (int)data.GetFloatArg(0);
            break;
        case 0x3D7E6258u:
        {
            RedColor color;
            color.r = ClampColorComponent(data.GetFloatArg(0));
            color.g = ClampColorComponent(data.GetFloatArg(1));
            color.b = ClampColorComponent(data.GetFloatArg(2));
            if (data.GetNumArgs() > 3)
            {
                color.a = ClampColorComponent(data.GetFloatArg(3));
            }
            else
            {
                color.a = 0xFF;
            }
            m_color = color;
            break;
        }
        case 0x5212ABEEu:
            m_lifeTime = data.GetFloatArg(0);
            break;
        case 0x5083ECD9u:
            m_fadeInTime = data.GetFloatArg(0);
            break;
        case 0x798DC484u:
            m_fadeOutTime = data.GetFloatArg(0);
            break;
        }
    }
}
