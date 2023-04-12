#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2023 The LineageOS Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

DEVICE=alice
VENDOR=huawei

# Load extractutils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "$MY_DIR" ]]; then MY_DIR="$PWD"; fi

LINEAGE_ROOT="$MY_DIR"/../../..

HELPER="$LINEAGE_ROOT"/vendor/lineage/build/tools/extract_utils.sh
if [ ! -f "$HELPER" ]; then
    echo "Unable to find helper script at $HELPER"
    exit 1
fi
. "$HELPER"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

while [ "$1" != "" ]; do
    case $1 in
        -n | --no-cleanup )     CLEAN_VENDOR=false
                                ;;
        -s | --section )        shift
                                SECTION=$1
                                CLEAN_VENDOR=false
                                ;;
        * )                     SRC=$1
                                ;;
    esac
    shift
done

if [ -z "$SRC" ]; then
    SRC=adb
fi

function blob_fixup() {
    case "${1}" in
        etc/camera/*)
	    sed -i 's/gb2312/iso-8859-1/g' "${2}"
	    sed -i 's/GB2312/iso-8859-1/g' "${2}"
	    sed -i 's/xmlversion/xml version/g' "${2}"
            ;;
        bin/chargemonitor)
            sed -i 's/\([Uu][Cc][Nn][Vv]_[A-Za-z_]*\)_55/\1_63/g' "${2}"
            ;;
        bin/glgps)
            sed -i 's/\([Uu][Cc][Nn][Vv]_[A-Za-z_]*\)_55/\1_63/g' "${2}"
            sed -i "s/SSLv3_client_method/SSLv23_method\x00\x00\x00\x00\x00\x00/" "${2}"
            ;;
        lib*/hw/audio.primary.hi6210sft.so)
            sed -i 's/\([Uu][Cc][Nn][Vv]_[A-Za-z_]*\)_55/\1_63/g' "${2}"
            ;;
        lib/libril.so)
            sed -i 's/\([Uu][Cc][Nn][Vv]_[A-Za-z_]*\)_55/\1_63/g' "${2}"
            ;;
        lib*/libcamera_core.so)
            sed -i 's/libskia.so/libhwui.so/g' "${2}"
            ;;
        lib*/libcustpwmanager_jni.so)
            sed -i 's/libskia.so/libhwui.so/g' "${2}"
            ;;
        lib*/libdrmbitmap.huawei.so)
            sed -i 's/libskia.so/libhwui.so/g' "${2}"
            ;;
        lib*/sensors.alice.so)
            sed -i 's|/system/etc/permissions/|/vendor/etc/permissions/|g' "${2}"
            ;;
        lib/libcamera_post_mediaserver.so)
            sed -i 's/libskia.so/libhwui.so/g' "${2}"
            ;;
        lib/lib_k3_omx_mpeg4.so)
            # OmxMpeg4DecComponent::getParameter()
            xxd -p "${2}" | sed "s/144821e05c69182c/144821e05c69642c/g" | xxd -r -p > "${2}".patched
            mv "${2}".patched "${2}"
            ;;
        vendor/lib/mediadrm/libwvdrmengine.so)
            sed -i "s/libprotobuf-cpp-lite.so/libprotobuf-cpp-N.so\x00\x00\x00/" "${2}"
            ;;
    esac
}

# Initialize the helper
setup_vendor "$DEVICE" "$VENDOR" "$LINEAGE_ROOT" false "$CLEAN_VENDOR"

extract "$MY_DIR"/proprietary-files.txt "$SRC" "$SECTION"

"$MY_DIR"/setup-makefiles.sh
