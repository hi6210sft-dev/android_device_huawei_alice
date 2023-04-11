#include <sys/ioctl.h>
#include <sys/mman.h>

#define NV_NAME_LENGTH		8
#define NVE_NV_DATA_SIZE	104
#define NV_READ			1	/* NV read  operation */
#define NV_WRITE		0	/* NV write operation */
#define NVE_PARTITION_PATH      "/dev/block/mmcblk0p7"

struct nve_info_user {
    uint32_t nv_operation;		/* 0 - write, 1 - read */
    uint32_t nv_number;		/* NV number you want to visit */
    char nv_name[NV_NAME_LENGTH];
    uint32_t valid_size;
    u_char nv_data[NVE_NV_DATA_SIZE];
};

#define NVEACCESSDATA		_IOWR('M', 25, struct nve_info_user)

char *nve_read(int fd, const char *nve_name) {
    uint32_t block_size = 0, offset = 0;
    unsigned char *data_buffer = nullptr, *ptr = nullptr;
    char* nve_data = static_cast<char*>(malloc(NVE_NV_DATA_SIZE));

    data_buffer = (unsigned char *)mmap(NULL, 1, PROT_READ, MAP_SHARED, fd, 0);
    if (data_buffer == MAP_FAILED) return nullptr;

    if (data_buffer[0] == 0 || data_buffer[0] == 85) {
        // This is a legacy NVE image, skip the junk.
        block_size = 0x00020000;
    }

    munmap(data_buffer, 1);
    while (block_size >= 0) {
        data_buffer = (unsigned char *)mmap(NULL, 0x00020000, PROT_READ, MAP_SHARED, fd, block_size);
        if (data_buffer == MAP_FAILED) {
            // Unmap the previous data_buffer if mapping failed
            if (block_size > 0) {
                munmap(data_buffer, 0x00020000);
            }
            return nullptr;
        }
        ptr = static_cast<unsigned char *>(memmem(data_buffer, 0x00020000, nve_name, strlen(nve_name)));
        if (ptr) {
            offset = (ptr - data_buffer) + NV_NAME_LENGTH + 12;
            if (offset) {
                memcpy(nve_data, data_buffer + offset, NVE_NV_DATA_SIZE);
                munmap(data_buffer, 0x00020000);
                return nve_data;
            }
        } else {
            // This block does not contain the NVE item we are looking for.
            munmap(data_buffer, 0x00020000);
            block_size += 0x00020000;
        }
    }

    return nullptr;
}
