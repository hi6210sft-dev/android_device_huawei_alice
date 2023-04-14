
#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
int property_get(const char *key, char *value, const char *default_value);
#ifdef __cplusplus
}
#endif

inline const char* BtmGetDefaultName()
{
	char device[92];
	property_get("ro.product.model", device, "");

        if (strstr(device, "ALE") != NULL) {
            return "Huawei P8 Lite 2015";
        } else if (strstr(device, "CAM") != NULL) {
            return "Huawei Y6II";
        }

	return "Huawei";
}

#define BTM_DEF_LOCAL_NAME   BtmGetDefaultName()
#define BTM_BYPASS_EXTRA_ACL_SETUP TRUE

#endif
