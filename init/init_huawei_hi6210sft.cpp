/*
   Copyright (C) 2007, The Android Open Source Project
   Copyright (c) 2016, The CyanogenMod Project
   Copyright (c) 2023, The LineageOS Project
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.
   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <filesystem>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"
#include "property_service.h"

std::string kPartitionMap[] = {
        "", "vrl", "vrl_bkup", "mcuimage", "reserved0", "fastboot",
        "modemnvbkup", "nvme", "oeminfo", "splash", "modemnvm1",
        "modemnvm2", "modemnvm3", "securetystorage", "3rdmodemnvm", "3rdmodemnvmback",
        "reserved1", "modemlog", "splash2", "misc", "modemnvreserved", "recovery2",
        "reserved2", "teeos", "trustfirmware", "sensorhub", "hifi", "boot", "recovery",
        "dtimage", "modemimage", "dsp", "dfx", "3rdmodem", "cache", "hisitest0", "hisitest1",
        "hisitest2", "system", "cust", "userdata"
};

void fix_symlinks() {
    const std::string kPathPrefix = "/dev/block/mmcblk0p";
    const std::string kLinkPrefix = "/dev/block/platform/hi_mci.0/by-name";

    if (access(kLinkPrefix.c_str(), F_OK) != 0) {
        if (!std::filesystem::create_directories(kLinkPrefix)) {
            LOG(ERROR) << "Failed to create directory: " << kLinkPrefix;
            return;
        }
    }

    for (int i = 1; i < sizeof(kPartitionMap)/sizeof(kPartitionMap[0]); ++i) {
        std::string path = kPathPrefix + std::to_string(i);
        std::string link = kLinkPrefix + "/" + kPartitionMap[i];

        if (symlink(path.c_str(), link.c_str()) != 0) {
            LOG(ERROR) << "Failed to create symlink: " << path << " -> " << link;
        } else {
            LOG(INFO) << "Created symlink: " << path << " -> " << link;
        }
    }
}

void vendor_load_properties() {
    fix_symlinks();
}
