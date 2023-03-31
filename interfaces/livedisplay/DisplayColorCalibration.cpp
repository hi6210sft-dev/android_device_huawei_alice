/*
 * Copyright (C) 2019 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef LIVES_IN_SYSTEM
#define LOG_TAG "lineage.livedisplay@2.0-impl-hisi"
#else
#define LOG_TAG "vendor.lineage.livedisplay@2.0-impl-hisi"
#endif

#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/logging.h>

#include <fstream>
#include <fcntl.h>

#include "DisplayColorCalibration.h"

using android::base::ReadFileToString;
using android::base::Split;
using android::base::Trim;
using android::base::WriteStringToFile;

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace hisi {

std::vector<int32_t> DisplayColorCalibration::sCurColors = {256, 256, 256};

bool DisplayColorCalibration::isSupported() {
    // Always true, use GPU mode if not.
    return true;
}

// Methods from ::vendor::lineage::livedisplay::V2_0::IDisplayColorCalibration follow.
Return<int32_t> DisplayColorCalibration::getMaxValue() {
    return 256;
}

Return<int32_t> DisplayColorCalibration::getMinValue() {
    return 1;
}

Return<void> DisplayColorCalibration::getCalibration(getCalibration_cb _hidl_cb) {
    _hidl_cb(DisplayColorCalibration::sCurColors);
    return Void();
}

Return<bool> DisplayColorCalibration::setCalibration(const hidl_vec<int32_t>& rgb) {
    int fd = -1, ret = -1;
    uint32_t ctValue[9] = {
        static_cast<uint32_t>(rgb[0]), 0, 0, 0,
        static_cast<uint32_t>(rgb[1]), 0, 0, 0,
        static_cast<uint32_t>(rgb[2])
    };

    LOG(DEBUG) << "Setting color calibration to: " << rgb[0] << " " << rgb[1] << " " << rgb[2];
    fd = open(LCD_TUNING_DEV, O_RDONLY);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open " << LCD_TUNING_DEV << ": " << strerror(errno);
        return false;
    }

    for (int i = 0; i < 9; i += 4) {
        if (ctValue[i] > 256)
            ctValue[i] = 256;
        else
            if (ctValue[i] < 1)
                ctValue[i] = 1;
    }

    ret = ioctl(fd, LCD_TUNING_DCPR, ctValue);
    close(fd);
    if (ret < 0) {
        LOG(ERROR) << "Failed to set display color calibration: " << strerror(errno);
        return false;
    }

    sCurColors[0] = rgb[0];
    sCurColors[1] = rgb[1];
    sCurColors[2] = rgb[2];

    return true;
}

}  // namespace hisi
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
