#include <tinyalsa/asoundlib.h>

extern "C" void pop_seq_init(void) {
    // This is supposed to initialize whatever
    // g_pst_cfg is by using memset(). We don't
    // support this so stub out the function.
    return;
}

extern "C" char *pop_seq_set(struct mixer *mixer, char *path) {
    // We don't support this either so stub out
    // the function, too (respecting the params).
    return (char *)0x0;
}

extern "C" int force_flush_set(struct mixer *mixer, char* state, char *p) {
    // We don't support this either so stub out
    // the function, too (respecting the params).
    return 0;
}

extern "C" int32_t _ZN7android11AudioPlayer5startEb(void *thisptr, bool sourceAlreadyStarted);

extern "C" int32_t _ZN7android11AudioPlayer9start_lppEv(void *thisptr) {
    return _ZN7android11AudioPlayer5startEb(thisptr, false);
}
