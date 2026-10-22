#include <math.h>

// LIBRARY: SWBF2 0x0075315e
// _libm_sse2_pow_precise

#include "Battlefront2/Source/ColorControl.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"
#include "RedEngineFL/Graphics/RedColor.h"
#include "RedEngineFL/Interface/RedInterfaceScreen.h"

namespace FLRenderer
{
void AddInterfaceElement(RedScreenGroupElement* pElement, float fX, float fY,
                         bool bUseSafeZone);
}

// FUNCTION: SWBF2 0x00410220
void ColorControl::InitGammaRampDisplay()
{
    m_graph.Enable(false);
    m_graph.SetMatrixPosScale(0.0f, 0.0f, 0.4f, -0.4f);
    FLRenderer::AddInterfaceElement(&m_graph, RedInterfaceScreen::LEFT, 0.75f, true);

    m_graph.AddElement(&m_axes);
    m_graph.AddElement(&m_red);
    m_graph.AddElement(&m_green);
    m_graph.AddElement(&m_blue);

    m_axes.Enable(true);
    m_axes.SetColor(RedColor::YELLOW);
    m_axes.Begin();
    m_axes.MovePen(0.0f, 0.0f);
    m_axes.DrawLine(255.0f, 0.0f);
    m_axes.MovePen(0.0f, 0.0f);
    m_axes.DrawLine(0.0f, 255.0f);
    m_axes.End();

    m_red.Enable(true);
    m_red.SetColor(RedColor::RED);
    m_green.Enable(true);
    m_green.SetColor(RedColor::GREEN);
    m_blue.Enable(true);
    m_blue.SetColor(RedColor::BLUE);
}

// FUNCTION: SWBF2 0x00410420
void ColorControl::UpdateDisplay(D3DGAMMARAMP& ramp)
{
    int i;

    m_red.Begin();
    m_red.MovePen(0.0f, 0.0f);
    for (i = 0; i < 256; i++)
    {
        m_red.DrawLine((float)i, (float)ramp.red[i] * (255.0f / 65535.0f));
    }
    m_red.End();

    m_green.Begin();
    m_green.MovePen(0.0f, 0.0f);
    for (i = 0; i < 256; i++)
    {
        m_green.DrawLine((float)i, (float)ramp.green[i] * (255.0f / 65535.0f));
    }
    m_green.End();

    m_blue.Begin();
    m_blue.MovePen(0.0f, 0.0f);
    for (i = 0; i < 256; i++)
    {
        m_blue.DrawLine((float)i, (float)ramp.blue[i] * (255.0f / 65535.0f));
    }
    m_blue.End();
}

// FUNCTION: SWBF2 0x00410590
void ColorControl::PlatformInit()
{
    for (int i = 0; i < 3; i++)
    {
        m_levelInternal[i] = 0.5f;
        m_level[i] = 0.5f;
    }

    for (unsigned int i = 0; i < 256; i++)
    {
        const WORD wValue = (WORD)(i * 0x101);
        m_gammaRamp.blue[i] = wValue;
        m_gammaRamp.green[i] = wValue;
        m_gammaRamp.red[i] = wValue;
    }

    RedRenderer::pcGetDevice()->SetGammaRamp(0, 0, &m_gammaRamp);
    InitGammaRampDisplay();
    UpdateDisplay(m_gammaRamp);
}

// FUNCTION: SWBF2 0x00410650
void ColorControl::PlatformEnable(bool bEnable)
{
    Enable(bEnable);
    if (!IsEnabled())
    {
        m_levelInternal[0] = 0.5f;
        m_levelInternal[1] = 0.5f;
        m_levelInternal[2] = 0.5f;
        for (unsigned int i = 0; i < 256; i++)
        {
            const WORD wValue = (WORD)(i * 0x101);
            m_gammaRamp.blue[i] = wValue;
            m_gammaRamp.green[i] = wValue;
            m_gammaRamp.red[i] = wValue;
        }
        RedRenderer::pcGetDevice()->SetGammaRamp(0, 0, &m_gammaRamp);
        UpdateDisplay(m_gammaRamp);
    }
}

// FUNCTION: SWBF2 0x004106d0
void ColorControl::Update(float fDeltaTime)
{
    if (!IsEnabled())
    {
        return;
    }

    if (m_level[0] != m_levelInternal[0] || m_level[1] != m_levelInternal[1] ||
        m_level[2] != m_levelInternal[2])
    {
        const float fScale = m_level[0] * 1.8f + 0.1f;
        const float fOffset = m_level[1] * 1.5f - 0.75f;
        const float fExponent = m_level[2] * 1.8f + 0.1f;

        for (int i = 0; i < 256; i++)
        {
            float fValue = pow(((float)i * (1.0f / 255.0f) - 0.5f + fOffset) * fScale + 0.5f,
                               fExponent);
            if (fValue < 0.0f)
            {
                fValue = 0.0f;
            }
            else if (fValue > 1.0f)
            {
                fValue = 1.0f;
            }

            const WORD wValue = (WORD)(int)(fValue * 65535.0f);
            m_gammaRamp.red[i] = wValue;
            m_gammaRamp.blue[i] = wValue;
            m_gammaRamp.green[i] = wValue;
        }

        UpdateDisplay(m_gammaRamp);
        RedRenderer::pcGetDevice()->SetGammaRamp(0, 0, &m_gammaRamp);

        m_levelInternal[0] = m_level[0];
        m_levelInternal[1] = m_level[1];
        m_levelInternal[2] = m_level[2];
    }
}

// FUNCTION: SWBF2 0x00410850
void ColorControl::Render(int value)
{
}

void ColorControl::PlatformReadConfigData(PblConfig::Data& data)
{
}

// FUNCTION: SWBF2 0x00410620
void ColorControl::SetLevel(ControlT eControl, float fLevel)
{
    if (fLevel < 0.0f)
    {
        fLevel = 0.0f;
    }
    else if (fLevel > 1.0f)
    {
        fLevel = 1.0f;
    }

    m_level[eControl] = fLevel;
}
