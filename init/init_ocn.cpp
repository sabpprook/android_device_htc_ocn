/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.

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

#include <stdlib.h>

#include "vendor_init.h"
#include "property_service.h"
#include "log.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mount.h>

#define DEV_BLOCK_SYSTEM "/dev/block/bootdevice/by-name/system"

void set_props_from_file(const char *filename)
{
    FILE *fp = fopen(filename, "r");

    if (fp) {
        char line[1024];

        char propname[PROP_NAME_MAX];
        char propvalue[PROP_VALUE_MAX];
        char *pch;

        while ((fgets(line, sizeof(line), fp))) {
            if (line[0] == '\n' || line[0] == '#') continue;

            pch = strtok(line, "=\n");
            if (!pch || strlen(pch) >= PROP_NAME_MAX) continue;
            strcpy(propname, pch);

            pch = strtok(NULL, "=\n");
            if (!pch || strlen(pch) >= PROP_VALUE_MAX) continue;
            strcpy(propvalue, pch);

            if (strcmp(propname, "ro.build.fingerprint") == 0 || strcmp(propname, "ro.product.device") == 0) {
                property_set(propname, propvalue);
            }
        }
        fclose(fp);
    }
}

void set_props_from_build(void)
{
    if (access("/system/build.prop", R_OK) == 0) {
        set_props_from_file("/system/build.prop");
        return;
    }

    if (mkdir("/tmpsys", 777) != 0)
        return;

    int is_mounted = mount(DEV_BLOCK_SYSTEM, "/tmpsys", "ext4", MS_RDONLY | MS_NOATIME , "") == 0;

    if (!is_mounted)
        is_mounted = mount(DEV_BLOCK_SYSTEM, "/tmpsys", "f2fs", MS_RDONLY | MS_NOATIME , "") == 0;

    if (is_mounted) {
        set_props_from_file("/tmpsys/build.prop");
        umount("/tmpsys");
    }
    rmdir("/tmpsys");
}

void vendor_load_properties()
{
    std::string platform;
    std::string bootmid;
    std::string bootcid;
    std::string device;

    platform = property_get("ro.board.platform");
    if (platform != ANDROID_TARGET)
        return;

    bootmid = property_get("ro.boot.mid");
    bootcid = property_get("ro.boot.cid");

    if (bootmid == "2PZC10000") {
    	/* Asia / EMEA (Single SIM) - OCN_UHL */
        property_set("ro.build.product", "htc_ocnuhl");
        property_set("ro.product.model", "HTC U11");
    } else if (bootmid == "2PZC20000") {
    	/* Japan (Single SIM) - OCN_UHLJAPAN */
        property_set("ro.build.product", "htc_ocnuhljapan");
        property_set("ro.product.model", "601HT");
    } else if (bootmid == "2PZC30000") {
    	/* Asia / EMEA (Dual SIM) - OCN_DUGL */
        property_set("ro.build.product", "htc_ocndugl");
        property_set("ro.product.model", "HTC_U-3u");
    } else if (bootmid == "2PZC40000") {
    	/* China (Dual SIM) - OCN_DTWL */
        property_set("ro.build.product", "htc_ocndtwl");
        property_set("ro.product.model", "HTC U-3w");
    } else if (bootmid == "2PZC50000") {
        /* Asia / EMEA (Single SIM) - OCN_UHL */
        property_set("ro.build.product", "htc_ocnwhl");
        if (bootcid == "SPCS_001")
        	property_set("ro.product.model", "2PZC5");
        else
        	property_set("ro.product.model", "HTC U11");
    } else {
        /* GSM (OCN_UL) */
        property_set("ro.build.product", "htc_ocnuhl");
        property_set("ro.product.model", "HTC U11");
    }

    set_props_from_build();

    device = property_get("ro.product.device");
    ERROR("Found bootmid %s setting build properties for %s device\n", bootmid.c_str(), device.c_str());
}
