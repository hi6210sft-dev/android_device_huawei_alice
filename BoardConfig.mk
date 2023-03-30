#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

## Device Path
DEVICE_PATH := device/huawei/alice

# Arch
TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_VARIANT := cortex-a53

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv8-a
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi
TARGET_2ND_CPU_VARIANT := cortex-a53

# Apexes
TARGET_FLATTEN_APEX := true

# Audio
USE_XML_AUDIO_POLICY_CONF := 1

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := hi6210sft

# Boot
BOARD_KERNEL_BASE := 0x07478000
BOARD_KERNEL_CMDLINE := hisi_dma_print=0 vmalloc=384M maxcpus=8 coherent_pool=512K no_irq_affinity androidboot.selinux=permissive ate_enable=true
BOARD_KERNEL_CMDLINE += androidboot.init_fatal_reboot_target=recovery
BOARD_KERNEL_IMAGE_NAME := Image
BOARD_KERNEL_PAGESIZE := 2048
BOARD_MKBOOTIMG_ARGS := --kernel_offset 0x00008000 --ramdisk_offset 0x07b88000 --second_offset 0x00e88000 --tags_offset 0x02988000

TARGET_KERNEL_CONFIG := lineage_alice_defconfig
TARGET_KERNEL_SOURCE := kernel/huawei/alice
TARGET_KERNEL_CLANG_COMPILE := false
TARGET_KERNEL_CROSS_COMPILE_PREFIX := aarch64-linux-android-
KERNEL_TOOLCHAIN := $(shell pwd)/prebuilts/gcc/$(HOST_OS)-x86/aarch64/aarch64-linux-android-4.9/bin

# Build system
BUILD_BROKEN_DUP_RULES := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth

# Camera
TARGET_HAS_LEGACY_CAMERA_HAL1 := true

# Dexpreopt
ifeq ($(HOST_OS),linux)
  ifneq ($(TARGET_BUILD_VARIANT),eng)
    ifeq ($(WITH_DEXPREOPT),)
      WITH_DEXPREOPT := true
      WITH_DEXPREOPT_BOOT_IMG_AND_SYSTEM_SERVER_ONLY := true
    endif
  endif
endif

# Filesystems
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
TARGET_USERIMAGES_USE_EXT4 := true

# Graphics
TARGET_FORCE_HWC_FOR_VIRTUAL_DISPLAYS := true

# Headers
TARGET_SPECIFIC_HEADER_PATH := $(DEVICE_PATH)/include

# Init
TARGET_INIT_VENDOR_LIB := //$(DEVICE_PATH):libinit_huawei_hi6210sft
TARGET_RECOVERY_DEVICE_MODULES := libinit_huawei_hi6210sft

# Malloc
MALLOC_SVELTE := true

# Partitions
BOARD_BOOTIMAGE_PARTITION_SIZE := 25165824
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 67108864
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2684354560
BOARD_USERDATAIMAGE_PARTITION_SIZE := 11605639168
BOARD_CACHEIMAGE_PARTITION_SIZE := 268435456
BOARD_FLASH_BLOCK_SIZE := 131072

# Platform
TARGET_BOARD_PLATFORM := hi6210sft

# Properties
TARGET_SYSTEM_PROP += $(DEVICE_PATH)/configs/props/system.prop

# Recovery
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/rootdir/etc/fstab.hi6210sft

# RIL
ENABLE_VENDOR_RIL_SERVICE := true
BOARD_PROVIDES_LIBRIL := true

# Root
BOARD_ROOT_EXTRA_FOLDERS += \
    3rdmodem \
    3rdmodemnvm \
    3rdmodemnvmbkp \
    mnvm1:0 \
    mnvm2:0 \
    mnvm3:0 \
    modem_fw \
    modem_log \
    sec_storage \
    splash2 \
    log

BOARD_ROOT_EXTRA_SYMLINKS := \
    /splash2:/log

# Shims
TARGET_LD_SHIM_LIBS := \
    /system/bin/glgps|libshim_sensorlistener.so \
    /system/lib/hw/audio.primary.hi6210sft.so|libshim_audio.so \
    /system/lib64/hw/audio.primary.hi6210sft.so|libshim_audio.so \
    /system/lib/hw/gralloc.hi6210sft.so|libion-hisi.so \
    /system/lib64/hw/gralloc.hi6210sft.so|libion-hisi.so \
    /system/lib/libcusteyeprotection_jni.so|libshim_gui.so \
    /system/lib64/libcusteyeprotection_jni.so|libshim_gui.so \
    /system/lib/libeyeprotection_jni.so|libshim_gui.so \
    /system/lib64/libeyeprotection_jni.so|libshim_gui.so \
    /system/lib/libhwsmartdisplay_jni.so|libshim_gui.so \
    /system/lib64/libhwsmartdisplay_jni.so|libshim_gui.so \
    /system/lib/libcamera_core.so|libshim_camera.so \
    /system/lib/libcamera_core.so|libshim_sensorlistener.so \
    /system/lib/libcamera_core.so|libsensor.so \
    /system/lib/hw/camera.hi6210sft.so|libcamera_parameters_ext_alice.so \
    /system/lib64/hw/camera.hi6210sft.so|libcamera_parameters_ext_alice.so

# SDK
TARGET_PROCESS_SDK_VERSION_OVERRIDE += \
    /system/vendor/bin/hw/rild=27 \
    /system/bin/mediaserver=23

# Vintf
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/manifest.xml

# Wifi
BOARD_WLAN_DEVICE := bcmdhd
BOARD_HOSTAPD_DRIVER := NL80211
BOARD_HOSTAPD_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_WPA_SUPPLICANT_DRIVER := NL80211
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
WPA_SUPPLICANT_VERSION := VER_0_8_X
WIFI_DRIVER_FW_PATH_PARAM := "/sys/module/bcmdhd/parameters/firmware_path"
WIFI_DRIVER_FW_PATH_STA   := "/system/vendor/firmware/fw_bcm4343.bin"
WIFI_DRIVER_FW_PATH_AP    := "/system/vendor/firmware/fw_bcm4343s_apsta_hw.bin"
WIFI_DRIVER_FW_PATH_P2P   := "/system/vendor/firmware/fw_bcm4343s_p2p_hw.bin"

# SELinux
BOARD_VENDOR_SEPOLICY_DIRS += $(DEVICE_PATH)/sepolicy/vendor
