#include "RedAmbientLight.h"

// GLOBAL: SWBF2 0x007e2410
// SYNTHETIC: SWBF2 0x0040a900
// `dynamic initializer for 'RedAmbientLight::BLACK''
// SYNTHETIC: SWBF2 0x0076a090
// `dynamic atexit destructor for 'RedAmbientLight::BLACK''
RedAmbientLight RedAmbientLight::BLACK(RedColorValue::BLACK, RedColorValue::BLACK,
                                       RedColorValue::BLACK);

// GLOBAL: SWBF2 0x007e23a0
// SYNTHETIC: SWBF2 0x0040a990
// `dynamic initializer for 'RedAmbientLight::WHITE''
// SYNTHETIC: SWBF2 0x0076a0b0
// `dynamic atexit destructor for 'RedAmbientLight::WHITE''
RedAmbientLight RedAmbientLight::WHITE(RedColorValue::WHITE, RedColorValue::WHITE,
                                       RedColorValue::WHITE);
