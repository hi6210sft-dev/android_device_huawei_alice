#define LOG_TAG "mac_nvme"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <android-base/properties.h>

// We expect the user to set both properties
// before running the executable. If they are
// not set, we will use the default values.
#define MAC_WIFI_PROP      "ro.wifi.wifiaddr_path"
#define MAC_BLUETOOTH_PROP "ro.bt.bdaddr_path"

// Default paths to the files containing the
// MAC addresses. These are the same as the
// ones used by the stock binary.
#define MAC_WIFI_PATH      "/data/vendor/wifi/macwlan"
#define MAC_BLUETOOTH_PATH "/data/vendor/bluedroid/macbt"

// Default types for bluetooth and wifi.
#define NVE_MACWLAN		0xC1
#define NVE_MACBT		0xC2

#include <string>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <nvme.h>

#include <sys/ioctl.h>

int get_mac_address(int type, char *address) {
    int fd = -1;
    struct nve_info_user info = {
        .nv_operation = 0x1,
        .nv_number = static_cast<uint32_t>(type),
        .valid_size = 20,
        .nv_data = {0}
    };

    fd = open("/dev/nve0", O_RDWR);
    if (fd < 0) {
        LOG(ERROR) << "Failed to open /dev/nve0: " << strerror(errno);
    } else {
        if (ioctl(fd, NVEACCESSDATA, &info) == 0) {
            close(fd);
        } else {
            // ioctl failed, close the file descriptor and try raw method
            LOG(WARNING) << "Failed to read MAC address (type " << type << "): " << strerror(errno);
            close(fd);
            fd = -1;
        }
    }

    if (fd == -1) {
        LOG(INFO) << "Retrying with raw read operation...";
        fd = open(NVE_PARTITION_PATH, O_RDWR);
        if (fd < 0) {
            LOG(ERROR) << "Failed to open " << NVE_PARTITION_PATH << ": " << strerror(errno);
            return -1;
        }

        char *tmp_buffer = nve_read(fd, (type == 0xC1 ? "MACWLAN" : (type == 0xC2 ? "MACBT" : "")));
        if (tmp_buffer != nullptr) {
            std::snprintf(address, 18, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
                      tmp_buffer[0], tmp_buffer[1], tmp_buffer[2],
                      tmp_buffer[3], tmp_buffer[4], tmp_buffer[5],
                      tmp_buffer[6], tmp_buffer[7], tmp_buffer[8],
                      tmp_buffer[9], tmp_buffer[10], tmp_buffer[11]);
            free(tmp_buffer);
            return 0;
        }
    }

    close(fd);
    std::snprintf(address, 18, "%c%c:%c%c:%c%c:%c%c:%c%c:%c%c",
                  info.nv_data[0], info.nv_data[1], info.nv_data[2],
                  info.nv_data[3], info.nv_data[4], info.nv_data[5],
                  info.nv_data[6], info.nv_data[7], info.nv_data[8],
                  info.nv_data[9], info.nv_data[10], info.nv_data[11]);

    return 0;
}

int write_mac_address(std::string mac_path, char *mac_address) {
    if (!android::base::WriteStringToFile(mac_address, mac_path)) {
        LOG(ERROR) << "Failed to write MAC address to " << mac_path << ": " << strerror(errno);
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    std::string mac_wifi_path = android::base::GetProperty(MAC_WIFI_PROP, "");
    std::string mac_bluetooth_path = android::base::GetProperty(MAC_BLUETOOTH_PROP, "");

    if (mac_wifi_path.empty()) {
        mac_wifi_path = MAC_WIFI_PATH;
    }

    if (mac_bluetooth_path.empty()) {
        mac_bluetooth_path = MAC_BLUETOOTH_PATH;
    }

    LOG(INFO) << "Using " << mac_wifi_path << " for wifi MAC";
    LOG(INFO) << "Using " << mac_bluetooth_path << " for bluetooth MAC";

    char mac_wifi[18] = {0};
    if (get_mac_address(NVE_MACWLAN, mac_wifi) != 0) {
        LOG(ERROR) << "Failed to get WiFi MAC address";
        return -1;
    }

    char mac_bluetooth[18] = {0};
    if (get_mac_address(NVE_MACBT, mac_bluetooth) != 0) {
        LOG(ERROR) << "Failed to get Bluetooth MAC address";
        return -1;
    }

    LOG(INFO) << "WiFi MAC address: " << mac_wifi;
    LOG(INFO) << "Bluetooth MAC address: " << mac_bluetooth;

    if (write_mac_address(mac_wifi_path, mac_wifi) < 0) {
        return -1;
    }

    if (write_mac_address(mac_bluetooth_path, mac_bluetooth) < 0) {
        return -1;
    }

    LOG(INFO) << "Successfully read MACs!";

    return 0;
}
