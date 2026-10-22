#include "RedShadowBlob.h"
#include "PebbleFL/Common/PblRandom.h"

// GLOBAL: SWBF2 0x009c8954
RedShadowBlob* RedShadowBlob::sInstance;

// GLOBAL: SWBF2 0x009c8958
// SYNTHETIC: SWBF2 0x00409520
// `dynamic initializer for 'shadowBlobRandom''
PblRandom shadowBlobRandom;

// FUNCTION: SWBF2 0x006fc810
bool RedShadowBlob::Create()
{
    return false;
}

// GLOBAL: SWBF2 0x007e1900

float gShadowBlurNeighborWeight = 0.1f;
// GLOBAL: SWBF2 0x007e1904

int gShadowBlurPassCount = 3;

// FUNCTION: SWBF2 0x006fc840
void RedShadowBlob::Update()
{

    BlobVertex (*data)[15] = mData;
    for (int row = 0; row < 15; ++row)
    {
        for (int col = 0; col < 20; ++col)
        {
            data[col][row].intensity = data[col][row].setValue;
            data[col][row].blurValue = data[col][row].setValue;
        }
    }

    for (int pass = 0; pass < gShadowBlurPassCount; ++pass)
    {
        for (int row = 1; row < 14; ++row)
        {
            for (int col = 1; col < 19; ++col)
            {
                BlobVertex& v = data[col][row];
                float intensity = data[col + 1][row].blurValue * gShadowBlurNeighborWeight;
                intensity += v.intensity;
                intensity += data[col][row + 1].blurValue * gShadowBlurNeighborWeight;
                intensity += data[col - 1][row].blurValue * gShadowBlurNeighborWeight;
                intensity += data[col][row - 1].blurValue * gShadowBlurNeighborWeight;
                if (intensity >= 1.0f)
                {
                    intensity = 1.0f;
                }
                v.intensity = intensity;
            }
        }
        for (int row = 1; row < 14; ++row)
        {
            for (int col = 1; col < 19; ++col)
            {
                data[col][row].blurValue = data[col][row].intensity;
            }
        }
    }

    for (int row = 0; row < 15; ++row)
    {
        for (int col = 0; col < 20; ++col)
        {
            float& value = data[col][row].setValue;
            value += (0.0f - value) * 0.02f;
        }
    }
    SetZOrder(0xfe);
}

// FUNCTION: SWBF2 0x006fc820
bool RedShadowBlob::Destroy()
{
    if (!RedShadowBlob::sInstance)
    {
        return false;
    }
    delete RedShadowBlob::sInstance;
    RedShadowBlob::sInstance = 0;
    return true;
}
