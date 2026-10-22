#include "GameCamera.h"
#include "RedEngineFL/Graphics/PC/pcRedRenderer.h"

// GLOBAL: SWBF2 0x01ec0c40
// SYNTHETIC: SWBF2 0x00403c60
// `dynamic initializer for 'sAspectRatio''
float sAspectRatio = RedRenderer::GetScreenAspectRatio();
