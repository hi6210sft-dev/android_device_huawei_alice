#define LOG_TAG "mploader"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <android-base/properties.h>

#define DEFAULT_ID "0X00000000"
#define SYS_PROP_READY "sys.rilprops_ready"

#define CMDLINE "/proc/cmdline"
#define PHONE_PROP "/vendor/phone.prop"
#define HW_OEM_BASE "/system/hw_oem/"

#include <string>
#include <vector>
#include <fstream>

std::string ReadProductId() {
    std::string prid = DEFAULT_ID;
    std::string cmdline;

    if (android::base::ReadFileToString(CMDLINE, &cmdline)) {
        for (const auto& i : android::base::Split(android::base::Trim(cmdline), " ")) {
            std::vector<std::string> parts = android::base::Split(i, "=");
            if (parts.size() == 2 && parts.at(0) == "productid") {
                prid = parts.at(1);
                std::transform(prid.begin(), prid.end(), prid.begin(),
                               [](unsigned char c){ return toupper(c); });
            }
        }
    }

    return prid;
}

int LoadPhoneProperties(std::string prid) {
    int ret = -1;
    std::string line;
    std::ifstream file(PHONE_PROP);

    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (ret == 0 && line.length() == 0) break;
            if (line.find(prid) != std::string::npos) ret = 0;

            if (ret == 0) {
                std::vector<std::string> parts = android::base::Split(line, "=");
                if (parts.size() == 2) {
                    LOG(INFO) << "Setting property: " << parts.at(0);
                    android::base::SetProperty(parts.at(0), parts.at(1));
                }
            }
        }
    }

    return ret;
}

int LoadHWOemProperties(std::string model) {
    std::string line;
    std::string hw_oem_prop = HW_OEM_BASE + model + "/prop/local.prop";

    std::ifstream file(hw_oem_prop);
    if (file.is_open()) {
        while (std::getline(file, line)) {
            if (line.find("#") == 0) continue; // Ignore possible comments.
            std::vector<std::string> parts = android::base::Split(line, "=");
            if (parts.size() == 2) {
                LOG(INFO) << "Setting property: " << parts.at(0);
                android::base::SetProperty(parts.at(0), parts.at(1));
            }
        }
    }

    return 0;
}

int main() {
    int ret = -1;
    std::string productModel = android::base::GetProperty("ro.product.model", "");
    std::string productId = ReadProductId();

    if (productModel != "") {
        if ((ret = LoadHWOemProperties(productModel)) == 0) {
            LOG(INFO) << "Successfully loaded hw_oem properties for " << productModel;
        }
    }

    if (productId != DEFAULT_ID) {
        if ((ret = LoadPhoneProperties(productId)) == 0) {
            LOG(INFO) << "Successfully loaded phone properties for " << productId;
            android::base::SetProperty(SYS_PROP_READY, "1");
        }
    }

    return ret;
}
