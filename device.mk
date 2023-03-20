#
# Copyright (C) 2023 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

## Device Path
DEVICE_PATH := device/huawei/alice

## Inherit vendor blobs
$(call inherit-product, vendor/huawei/alice/alice-vendor.mk)

# AAPT
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := xhdpi

# Overlay
PRODUCT_ENFORCE_RRO_TARGETS := *

# Display
TARGET_SCREEN_DENSITY := 320
TARGET_SCREEN_HEIGHT := 1280
TARGET_SCREEN_WIDTH := 720

# Soong namespaces
PRODUCT_SOONG_NAMESPACES += $(DEVICE_PATH)

# Ramdisk
PRODUCT_PACKAGES += \
    fstab.hi6210sft \
    fstab.hi6210sft_ramdisk \
    init.hi6210sft.rc \
    init.hi6210sft.usb.rc \
    init.platform.rc \
    init.connectivity.rc \
    init.extmodem.rc \
    init.manufacture.rc \
    init.tee.rc \
    ueventd.hi6210sft.rc

# Recovery
PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/rootdir/etc/init.recovery.hi6210sft.rc:$(TARGET_RECOVERY_OUT)/root/init.recovery.hi6210sft.rc \
    $(DEVICE_PATH)/rootdir/etc/init.hi6210sft.usb.rc:$(TARGET_RECOVERY_OUT)/root/init.hi6210sft.usb.rc
