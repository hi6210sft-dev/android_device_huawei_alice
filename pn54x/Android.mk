#
# Copyright (C) 2011 The Android Open Source Project
#               2019 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := nfc_nci.hi6210sft
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
    $(call all-c-files-under, common) \
    $(call all-c-files-under, dnld) \
    $(call all-c-files-under, hal) \
    $(call all-c-files-under, log) \
    $(call all-c-files-under, self-test) \
    $(call all-c-files-under, tml) \
    $(call all-c-files-under, utils) \
    $(call all-cpp-files-under, utils) \
    nfc_nci.c

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libhardware

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/common \
    $(LOCAL_PATH)/dnld \
    $(LOCAL_PATH)/hal \
    $(LOCAL_PATH)/inc \
    $(LOCAL_PATH)/log \
    $(LOCAL_PATH)/self-test \
    $(LOCAL_PATH)/tml \
    $(LOCAL_PATH)/utils

LOCAL_CFLAGS := \
    -DNFC_NXP_CHIP_TYPE=PN547C2 \
    -DNFC_NXP_HFO_SETTINGS=FALSE \
    -DNXP_HW_SELF_TEST

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE       := libnfc-nci.conf
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE       := libnfc-nxp.conf
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_VENDOR_ETC)
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

