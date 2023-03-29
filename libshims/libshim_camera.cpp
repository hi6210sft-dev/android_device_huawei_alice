#include <SkStream.h>

extern "C" void _ZN22SkDynamicMemoryWStreamC1Ev(SkDynamicMemoryWStream* self) {
    new(self) SkDynamicMemoryWStream();
}
