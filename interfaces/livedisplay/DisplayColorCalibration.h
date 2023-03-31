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

#ifndef VENDOR_LINEAGE_LIVEDISPLAY_V2_0_DISPLAYCOLORCALIBRATION_H
#define VENDOR_LINEAGE_LIVEDISPLAY_V2_0_DISPLAYCOLORCALIBRATION_H

#include <vendor/lineage/livedisplay/2.0/IDisplayColorCalibration.h>

#include <sys/ioctl.h>

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace hisi {

using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;

#define FILE_RGB "/sys/devices/virtual/graphics/fb0/lcd_color_temperature"
#define LCD_IOR(num, dtype)     _IOR('L', num, dtype)
#define LCD_TUNING_DCPR         LCD_IOR(34, unsigned int)
#define LCD_TUNING_DEV          "/dev/pri_lcd"

class DisplayColorCalibration : public IDisplayColorCalibration {
   public:
    bool isSupported();

    // Methods from ::vendor::lineage::livedisplay::V2_0::IDisplayColorCalibration follow.
    Return<int32_t> getMaxValue() override;
    Return<int32_t> getMinValue() override;
    Return<void> getCalibration(getCalibration_cb _hidl_cb) override;
    Return<bool> setCalibration(const hidl_vec<int32_t>& rgb) override;
private:
    static std::vector<int32_t> sCurColors;
};

}  // namespace hisi
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor

#endif  // VENDOR_LINEAGE_LIVEDISPLAY_V2_0_DISPLAYCOLORCALIBRATION_H
