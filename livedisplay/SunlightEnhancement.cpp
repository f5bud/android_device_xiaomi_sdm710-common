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

#define LOG_TAG "SunlightEnhancementService"

#include <fstream>

#include <android-base/logging.h>
#include <android-base/properties.h>
#include <android-base/strings.h>
#include <utils/Errors.h>

#include "SunlightEnhancement.h"

namespace vendor {
namespace lineage {
namespace livedisplay {
namespace V2_0 {
namespace implementation {

static constexpr const char* kDispParamPath = "/sys/class/drm/card0-DSI-1/disp_param";
static constexpr const char* kHbmStatusPath = "/sys/class/drm/card0-DSI-1/hbm_status";

static constexpr const char* kDispParamHbmOff    = "0xF0000";
static constexpr const char* kDispParamHbmOn     = "0x10000";

static constexpr const char* kDispParamHbmFodOff = "0xE0000";
static constexpr const char* kDispParamHbmFodOn  = "0x20000";

bool hasFingerprintOnDisplay()
{
    std::string device = android::base::GetProperty("ro.product.device", "");

    return device == "grus" || device == "pyxis" || device == "vela";
}

bool SunlightEnhancement::isSupported()
{
    std::ofstream disp_param_file(kDispParamPath);
    std::ifstream hbm_status_file(kHbmStatusPath);

    if (!disp_param_file.is_open())
	{
        LOG(ERROR) << "Failed to open " << kDispParamPath << ", error=" << errno << " (" << strerror(errno) << ")";
        return false;
    }

    if (!hbm_status_file.is_open())
	{
        LOG(ERROR) << "Failed to open " << kHbmStatusPath << ", error=" << errno << " (" << strerror(errno) << ")";
        return false;
    }

    return true;
}

Return<bool> SunlightEnhancement::isEnabled()
{
    int result = -1;
    std::ifstream hbm_status_file(kHbmStatusPath);

    hbm_status_file >> result;

    return !hbm_status_file.fail() && result > 0;
}

Return<bool> SunlightEnhancement::setEnabled(bool enabled)
{
    std::ofstream disp_param_file(kDispParamPath);

    if (hasFingerprintOnDisplay())
	{
        disp_param_file << (enabled ? kDispParamHbmFodOn : kDispParamHbmFodOff);
    }
	else
	{
        disp_param_file << (enabled ? kDispParamHbmOn : kDispParamHbmOff);
    }

    LOG(DEBUG) << "setEnabled fail " << disp_param_file.fail();

    return !disp_param_file.fail();
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace livedisplay
}  // namespace lineage
}  // namespace vendor
