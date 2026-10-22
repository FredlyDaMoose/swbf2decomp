#include "RedColor.h"

#include "RedMaterial.h"

#pragma init_seg(lib)

// GLOBAL: SWBF2 0x007de144

RedColor RedColor::WHITE = { 0xff, 0xff, 0xff, 0xff };

// GLOBAL: SWBF2 0x007de148

RedColor RedColor::LTGREY = { 0xc0, 0xc0, 0xc0, 0xff };

// GLOBAL: SWBF2 0x007de14c

RedColor RedColor::MDGREY = { 0x80, 0x80, 0x80, 0xff };

// GLOBAL: SWBF2 0x007de150

RedColor RedColor::DKGREY = { 0x40, 0x40, 0x40, 0xff };

// GLOBAL: SWBF2 0x007de154

RedColor RedColor::BLACK = { 0x00, 0x00, 0x00, 0xff };

// GLOBAL: SWBF2 0x007de158

RedColor RedColor::RED = { 0xdf, 0x20, 0x20, 0xff };

// GLOBAL: SWBF2 0x007de15c

RedColor RedColor::GREEN = { 0x20, 0xdf, 0x20, 0xff };

// GLOBAL: SWBF2 0x007de160

RedColor RedColor::BLUE = { 0x20, 0x20, 0xdf, 0xff };

// GLOBAL: SWBF2 0x007de164

RedColor RedColor::CYAN = { 0x20, 0xdf, 0xdf, 0xff };

// GLOBAL: SWBF2 0x007de168

RedColor RedColor::MAGENTA = { 0xdf, 0x20, 0xdf, 0xff };

// GLOBAL: SWBF2 0x007de16c

RedColor RedColor::YELLOW = { 0xdf, 0xdf, 0x20, 0xff };

// GLOBAL: SWBF2 0x007de170

RedColorValue RedColorValue::WHITE(1.0f, 1.0f, 1.0f, 1.0f);

// GLOBAL: SWBF2 0x007de1b0

RedColorValue RedColorValue::BLACK(0.0f, 0.0f, 0.0f, 1.0f);

// FUNCTION: SWBF2 0x004bd900
RedColorValue::RedColorValue(const RedColor& color)
{
    const float kInv255 = 1.0f / 255.0f;
    r = color.r * kInv255;
    g = color.g * kInv255;
    b = color.b * kInv255;
    a = color.a * kInv255;
}

// SYNTHETIC: SWBF2 0x0048ebf0
// RedColorValue::RedColorValue
