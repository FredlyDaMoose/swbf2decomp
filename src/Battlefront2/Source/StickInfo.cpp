#include "StickInfo.h"

// GLOBAL: SWBF2 0x01f6deb8
// SYNTHETIC: SWBF2 0x004064b0
// `dynamic initializer for 'StickInfo::sMemoryPool''
// SYNTHETIC: SWBF2 0x007694f0
// `dynamic atexit destructor for 'StickInfo::sMemoryPool''
MemoryPool StickInfo::sMemoryPool("StickInfo", sizeof(StickInfo));
