/*
 * hi6210sft hwcomposer wrapper, based on Meticulus libhwcomposer.
 * Copyright (c) 2017 Jonathan Dennis [Meticulus]
 *                               theonejohnnyd@gmail.com
 * Copyright (c) 2023 Roger Ortiz [R0rt1z2]
 *                               me@r0rt1z2.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#define LOG_TAG "HWComposerWrapper"

#include <cstdint>
#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <dlfcn.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/resource.h>

#include <hardware/hardware.h>
#include <utils/ThreadDefs.h>
#include <utils/Timers.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hwcomposer.h>
#include <hardware/hardware.h>

#include <EGL/egl.h>

#include "hisi_dss.h"

/* VSYNC defines follow */
#define FAKE_VSYNC
#define NSEC_PER_SEC    1000000000L
#define TIMESTAMP_FILE "/sys/devices/virtual/graphics/fb0/vsync_timestamp"

#ifdef FAKE_VSYNC
#ifndef REFRESH_RATE
#define REFRESH_RATE (60.0) // 60 Hz
#endif // REFRESH_RATE
#define REFRESH_PERIOD ((int64_t)(NSEC_PER_SEC / REFRESH_RATE))
#endif // FAKE_VSYNC

/* HWC defines follow */
#define FB0_FILE "/dev/graphics/fb0"
#define FB1_FILE "/dev/graphics/fb1"
#define FB2_FILE "/dev/graphics/fb2"
#define ADE_FILE "/dev/graphics/ade"

struct fb_ctx_t {
    int id;
    int available;
    int fd;
    int ade_fd;
    int vsyncfd;
    int vsync_on;
    int vsync_stop;
    int vthread_running;
    int fake_vsync;
    pthread_t vthread;
    hwc_procs_t const *hwc_procs;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int32_t xres;
    int32_t yres;
    int32_t xdpi;
    int32_t ydpi;
};

struct hwc_context_t {
    hwc_composer_device_1_t device;
    hwc_composer_device_1_t *orghwc;
    fb_ctx_t disp[3];
};

/*
 * Retrieve the stock HWC module.
 */
static hwc_module_t* get_hwc(void) {
    // The original library implementation.
    static hwc_module_t* orig_hwc = {NULL};
    const char *orig_path = "/system/lib64/hw/hwcomposer.alice.so";

    if (!orig_hwc) {
        void *handle = NULL;
        ALOGI("Loading original HWC from %s", orig_path);

        handle = dlopen(orig_path, RTLD_LAZY);
        if (!handle) {
            ALOGE("Failed to load original HWC: %s", dlerror());
            return NULL;
        }

        // Clear any previous errors.
        dlerror();

        // Fetch the HAL module information symbol.
        orig_hwc = (hwc_module_t*)dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR);
        if (!orig_hwc) {
            ALOGE("Unable to resolve symbol %s: %s", HAL_MODULE_INFO_SYM_AS_STR, dlerror());
            return NULL;
        }

        // If we got here, we have a valid handle and a valid module.
        // Print the module information and return the handle.
        ALOGI("Successfully loaded %s (%s), version %d!", orig_hwc->common.name,
              orig_hwc->common.author, orig_hwc->common.module_api_version);
    }
    return orig_hwc;
}

/*
 * Dumps the given layer.
 */
static void dump_layer(hwc_layer_1_t const* l) {
    ALOGD("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
          l->compositionType, l->flags, l->handle, l->transform, l->blending,
          l->sourceCrop.left,
          l->sourceCrop.top,
          l->sourceCrop.right,
          l->sourceCrop.bottom,
          l->displayFrame.left,
          l->displayFrame.top,
          l->displayFrame.right,
          l->displayFrame.bottom);
}

static int wrapper_prepare(hwc_composer_device_1_t *dev, size_t numDisplays, hwc_display_contents_1_t** displays) {
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original prepare function.
    ALOGI("%s: Calling original method with %zu displays", __func__, numDisplays);
    return context->orghwc->prepare(context->orghwc, numDisplays, displays);
}

static int wrapper_set(hwc_composer_device_1_t *dev, size_t numDisplays, hwc_display_contents_1_t** displays)
{
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original set function.
    ALOGI("%s: Calling original method with %zu displays", __func__, numDisplays);
    return context->orghwc->set(context->orghwc, numDisplays, displays);
}

static void * vsync_thread(void * arg) {
    setpriority(PRIO_PROCESS, 0, HAL_PRIORITY_URGENT_DISPLAY +
                                 android::PRIORITY_MORE_FAVORABLE);
    fb_ctx_t *fb = (fb_ctx_t *) arg;
    ALOGD("vsync thread enter id = %d fake = %d", fb->id, fb->fake_vsync);
    fb->vthread_running = 1;
    int retval = -1;
    int64_t timestamp = 0;
    char read_result[20];
    if(fb->fake_vsync) {
        while(true) {
            if(!fb->vsync_on) {
                ALOGD("vsync thread sleeping id = %d fake = 1", fb->id);
                while(!fb->vsync_on && !fb->vsync_stop)
                    pthread_cond_wait(&fb->cond,&fb->mutex);
                ALOGD("vsync thread woke up id = %d fake = 1", fb->id);
                if(fb->vsync_stop)
                    break;
                if(!fb->vsync_on)
                    continue;
            }
            timestamp = systemTime();
            fb->hwc_procs->vsync(fb->hwc_procs,fb->id,timestamp);
            usleep(16666);
        }
    } else {
        while(true) {
            if(!fb->vsync_on) {
                ALOGD("vsync thread sleeping id = %d fake = 0", fb->id);
                while(!fb->vsync_on && !fb->vsync_stop)
                    pthread_cond_wait(&fb->cond,&fb->mutex);
                ALOGD("vsync thread woke up id = %d fake = 0", fb->id);
                if(fb->vsync_stop)
                    break;
                if(!fb->vsync_on)
                    continue;
            }
            if(pread(fb->vsyncfd,read_result,20,0)) {
                timestamp = atol(read_result);
                fb->hwc_procs->vsync(fb->hwc_procs, fb->id, timestamp);
                usleep(16666);
            } else { goto error; }
        }
    }

    retval = 0;
    error:
    fb->vthread_running = 0;
    ALOGD("vsync thread exit");
    return NULL;

}

static void signal_vsync_thread(fb_ctx_t *fb) {
    if (fb->vthread_running) {
        pthread_mutex_lock(&fb->mutex);
        pthread_cond_signal(&fb->cond);
        pthread_mutex_unlock(&fb->mutex);
    }
}

static void start_vsync_thread(fb_ctx_t *fb) {
    if (!fb->fake_vsync)
        ioctl(fb->fd,HISIFB_VSYNC_CTRL, &fb->vsync_on);

    if (!fb->vthread_running) {
        pthread_create(&fb->vthread,NULL,&vsync_thread, fb);
    } else {
        signal_vsync_thread(fb);
    }
}

static int hwc_event_control(struct hwc_composer_device_1* dev, int disp,
                             int event, int enabled) {
    if (event == HWC_EVENT_VSYNC) {
        struct hwc_context_t *context = (hwc_context_t *)dev;

        if (!context->disp[disp].available)
            return -EINVAL;
        context->disp[disp].vsync_on = enabled;
        if (!context->disp[disp].vthread_running && enabled)
            start_vsync_thread(&context->disp[disp]);
    }
    return 0;
}

static int wrapper_blank(struct hwc_composer_device_1* dev, int disp, int blank) {
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original blank function.
    ALOGI("%s: Calling original method with disp=%d, blank=%d", __func__, disp, blank);
    return context->orghwc->blank(context->orghwc, disp, blank);
}

static void register_procs(struct hwc_composer_device_1* dev,
                           hwc_procs_t const* procs) {
    struct hwc_context_t *context = (hwc_context_t *)dev;
    context->disp[HWC_DISPLAY_PRIMARY].hwc_procs = procs;
    context->disp[HWC_DISPLAY_EXTERNAL].hwc_procs = procs;
    context->disp[HWC_DISPLAY_VIRTUAL].hwc_procs = procs;
    ALOGI("%s: Registered procs", __func__);
}

static int wrapper_getDisplayAttributes(hwc_composer_device_1 *dev, int disp, uint32_t config,
                                        const uint32_t *attributes, int32_t *values)
{
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original getDisplayAttributes function.
    ALOGI("%s: Calling original method with disp=%d, config=%zu, attributes=%p, values=%p", __func__,
          disp,config, attributes, values);
    return context->orghwc->getDisplayAttributes(context->orghwc, disp, config, attributes, values);
    return 0;
}

static int wrapper_getDisplayConfigs(hwc_composer_device_1 *dev, int disp, uint32_t *config, size_t *numConfigs)
{
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original getDisplayConfigs function.
    ALOGI("%s: Calling original method with disp=%d, config=%p, numConfigs=%p", __func__,
          disp, config, numConfigs);
    return context->orghwc->getDisplayConfigs(context->orghwc, disp, config, numConfigs);
}

static int wrapper_query(struct hwc_composer_device_1* dev, int what, int* value) {
    struct hwc_context_t *context = (hwc_context_t *)dev;
    // For now, use the original query function.
    ALOGI("%s: Calling original method with what=%d, value=%p", __func__, what, value);
    return context->orghwc->query(context->orghwc, what, value);
}

static int hwc_device_close(struct hw_device_t *dev)
{
    struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

    // Close the framebuffer devices.
    for (int i = 0; i < 3; i++) {
        if (ctx->disp[i].fd >= 0) {
            ALOGI("Closing framebuffer device %d", i);
            close(ctx->disp[i].fd);
            ctx->disp[i].fd = -1;
        }
    }

    // Close the ADE device.
    if (ctx->disp[HWC_DISPLAY_PRIMARY].ade_fd >= 0) {
        ALOGI("Closing ADE device");
        close(ctx->disp[HWC_DISPLAY_PRIMARY].ade_fd);
        ctx->disp[HWC_DISPLAY_PRIMARY].ade_fd = -1;
    }

    // Stop the VSYNC threads.
    for (int i = 0; i < 3; i++) {
        if (ctx->disp[i].vthread_running) {
            ALOGE("Stopping VSYNC thread %d", i);
            ctx->disp[i].vsync_stop = 1;
            signal_vsync_thread(&ctx->disp[i]);
            pthread_join(ctx->disp[i].vthread, NULL);
            ctx->disp[i].vthread_running = 0;
        }
        if (ctx->disp[i].vsyncfd >= 0) {
            ALOGD("Closing VSYNC device %d", i);
            close(ctx->disp[i].vsyncfd);
            ctx->disp[i].vsyncfd = -1;
        }
    }

    // Call the original close function, too.
    if (ctx->orghwc->common.close)
        ctx->orghwc->common.close((hw_device_t *)ctx->orghwc);

    return 0;
}

/*****************************************************************************/

static int hwc_device_open(const struct hw_module_t* module, const char* name,
                           struct hw_device_t** device)
{
    int status = 0;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        // First of all, try to fetch the original HWC.
        hwc_module_t *hwc_module = get_hwc();
        if (!hwc_module) {
            // Hey that's not good, we can't continue without the original HWC.
            ALOGE("Failed to fetch original HWC module, killing myself!");
            return -EINVAL;
        }

        // Initialize our state here.
        struct hwc_context_t *dev;
        dev = (hwc_context_t*) malloc(sizeof(*dev));
        memset(dev, 0, sizeof(*dev));

        // Call the original open() here.
        status = hwc_module->common.methods->open(&hwc_module->common, name, (struct hw_device_t**)&dev->orghwc);
        if (status < 0) {
            ALOGE("Failed to open original HWC module, killing myself!");
            free(dev);
            return status;
        }

        // Make sure the stock HWC reports the correct API version (1.4).
        if (dev->orghwc->common.version != HWC_DEVICE_API_VERSION_1_4) {
            ALOGE("Original HWC module reports wrong API version, killing myself!");
            free(dev);
            return -EINVAL;
        }

        // Fill the new struct.
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_4;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        // Set the HWC callbacks.
        dev->device.prepare = wrapper_prepare;
        dev->device.set = wrapper_set;
        dev->device.blank = wrapper_blank;
        dev->device.eventControl = hwc_event_control;
        dev->device.registerProcs = register_procs;
        dev->device.getDisplayAttributes = wrapper_getDisplayAttributes;
        dev->device.getDisplayConfigs = wrapper_getDisplayConfigs;
        dev->device.query = wrapper_query;

        // Reset the file descriptors.
        // We'll open them later.
        for (int i = 0; i < 3; i++) {
            dev->disp[i].fd = -1;
            dev->disp[i].ade_fd = -1;
            dev->disp[i].vsyncfd = -1;
        }

        // Initialize mutexes.
        for (int i = 0; i < 3; i++) {
            dev->disp[i].mutex = PTHREAD_MUTEX_INITIALIZER;
            dev->disp[i].cond = PTHREAD_COND_INITIALIZER;
        }

        // Initialize VSYNC.
        for (int i = 0; i < 3; i++) {
            dev->disp[i].vthread_running = 0;
            dev->disp[i].vsync_stop = 0;
            dev->disp[i].vsync_on = 0;
            dev->disp[i].fake_vsync = 0;
        }

        // Initialize the primary display.
        dev->disp[HWC_DISPLAY_PRIMARY].id = HWC_DISPLAY_PRIMARY;
        dev->disp[HWC_DISPLAY_PRIMARY].fd = open(FB0_FILE, O_WRONLY);
        if (dev->disp[HWC_DISPLAY_PRIMARY].fd < 0) {
            ALOGE("Failed to open framebuffer device %s: %s", FB0_FILE, strerror(errno));
            free(dev);
            return -EINVAL;
        }

        dev->disp[HWC_DISPLAY_PRIMARY].available = 1;
        dev->disp[HWC_DISPLAY_PRIMARY].vsyncfd = open(TIMESTAMP_FILE, O_RDONLY);
        if (dev->disp[HWC_DISPLAY_PRIMARY].vsyncfd < 0) {
            ALOGE("Failed to open vsync device %s: %s", TIMESTAMP_FILE, strerror(errno));
            dev->disp[HWC_DISPLAY_PRIMARY].fake_vsync = 1;
        }

        dev->disp[HWC_DISPLAY_PRIMARY].ade_fd = open(ADE_FILE, O_RDWR);
        if (dev->disp[HWC_DISPLAY_PRIMARY].ade_fd < 0) {
            ALOGE("Failed to open ADE device %s: %s", ADE_FILE, strerror(errno));
            free(dev);
            return -EINVAL;
        }

        // Initialize the external display.
        dev->disp[HWC_DISPLAY_EXTERNAL].id = HWC_DISPLAY_EXTERNAL;
        dev->disp[HWC_DISPLAY_EXTERNAL].available = 1;
        dev->disp[HWC_DISPLAY_EXTERNAL].fd = open(FB1_FILE, O_WRONLY);
        if (dev->disp[HWC_DISPLAY_EXTERNAL].fd < 0) {
            ALOGE("Failed to open framebuffer device %s: %s", FB1_FILE, strerror(errno));
            dev->disp[HWC_DISPLAY_EXTERNAL].available = 0;
        }

        // We MUST fake VSYNC for the external display.
        dev->disp[HWC_DISPLAY_EXTERNAL].fake_vsync = 1;

        // Initialize the virtual display.
        dev->disp[HWC_DISPLAY_VIRTUAL].id = HWC_DISPLAY_VIRTUAL;
        dev->disp[HWC_DISPLAY_VIRTUAL].available = 1;
        dev->disp[HWC_DISPLAY_VIRTUAL].fd = open(FB2_FILE, O_WRONLY);
        if (dev->disp[HWC_DISPLAY_VIRTUAL].fd < 0) {
            ALOGE("Failed to open framebuffer device %s: %s", FB2_FILE, strerror(errno));
            dev->disp[HWC_DISPLAY_VIRTUAL].available = 0;
        }

        // We MUST fake VSYNC for the virtual display.
        dev->disp[HWC_DISPLAY_VIRTUAL].fake_vsync = 1;

        // Loop over all the displays to get lcdinfo;
        struct fb_var_screeninfo lcdinfo;

        // We currently support only 3 types of displays.
        for (int i = 0; i < 3; i++) {
            if (dev->disp[i].available) {
                if (ioctl(dev->disp[i].fd, FBIOGET_VSCREENINFO, &lcdinfo) < 0) {
                    ALOGE("FBIOGET_VSCREENINFO failed for display %d: %s", i, strerror(errno));
                    status = -EINVAL;
                }
                dev->disp[i].xres = lcdinfo.xres;
                dev->disp[i].yres = lcdinfo.yres;
                dev->disp[i].xdpi = 1000 * (lcdinfo.xres * 25.4f) / lcdinfo.width;
                dev->disp[i].ydpi = 1000 * (lcdinfo.yres * 25.4f) / lcdinfo.height;
                ALOGI("Using display %d: xres=%d, yres=%d, xdpi=%d, ydpi=%d", i,
                      dev->disp[i].xres, dev->disp[i].yres, dev->disp[i].xdpi, dev->disp[i].ydpi);
            }
        }

        *device = &dev->device.common;
    }
    return status;
}

static struct hw_module_methods_t hwc_module_methods = {
        .open = hwc_device_open
};

hwc_module_t HAL_MODULE_INFO_SYM = {
        .common = {
                .tag = HARDWARE_MODULE_TAG,
                .version_major = 1,
                .version_minor = 4,
                .id = HWC_HARDWARE_MODULE_ID,
                .name = "HWComposer wrapper for hi6210sft",
                .author = "R0rt1z2",
                .methods = &hwc_module_methods,
        }
};
