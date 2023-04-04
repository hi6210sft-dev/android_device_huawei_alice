#include <sys/ioctl.h>

#define NV_NAME_LENGTH		8
#define NVE_NV_DATA_SIZE	104
#define NV_READ			1	/* NV read  operation */
#define NV_WRITE		0	/* NV write operation */

struct nve_info_user {
    uint32_t nv_operation;		/* 0 - write, 1 - read */
    uint32_t nv_number;		/* NV number you want to visit */
    char nv_name[NV_NAME_LENGTH];
    uint32_t valid_size;
    u_char nv_data[NVE_NV_DATA_SIZE];
};

#define NVEACCESSDATA		_IOWR('M', 25, struct nve_info_user)
