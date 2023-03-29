#include <SkStream.h>
#include <libexif/exif-entry.h>

extern "C" void exif_entry_gps_initialize(ExifEntry * e, ExifTag tag) {
    exif_entry_initialize(e,tag);
}

extern "C" void _ZN22SkDynamicMemoryWStreamC1Ev(SkDynamicMemoryWStream* self) {
    new(self) SkDynamicMemoryWStream();
}
