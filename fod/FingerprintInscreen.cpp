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

#define LOG_TAG "FingerprintInscreenService"

#include "FingerprintInscreen.h"

#include <fstream>
#include <cmath>
#include <thread>

#include <fcntl.h>
#include <poll.h>
#include <sys/stat.h>

#define COMMAND_NIT 10
#define PARAM_NIT_FOD 1
#define PARAM_NIT_NONE 0

//#define DISPPARAM_PATH "/sys/devices/platform/soc/ae00000.qcom,mdss_mdp/drm/card0/card0-DSI-1/disp_param"

#define DISPPARAM_PATH "/sys/class/drm/card0-DSI-1/disp_param"
#define DISPPARAM_HBM_FOD_ON "0x20000"
#define DISPPARAM_HBM_FOD_OFF "0xE0000"

#define FOD_STATUS_PATH "/sys/devices/virtual/touch/tp_dev/fod_status"
#define FOD_STATUS_ON 1
#define FOD_STATUS_OFF 0

#define FOD_UI_PATH "/sys/devices/platform/soc/soc:qcom,dsi-display-primary/fod_ui"

#define FOD_SENSOR_X 445
#define FOD_SENSOR_Y 1910
#define FOD_SENSOR_SIZE 190

namespace vendor {
namespace lineage {
namespace biometrics {
namespace fingerprint {
namespace inscreen {
namespace V1_0 {
namespace implementation {

template <typename T> static void set(const std::string& path, const T& value)
{
    std::ofstream file(path);
    file << value;
}

FingerprintInscreen::FingerprintInscreen()
{
    xiaomiFingerprintService = IXiaomiFingerprint::getService();
/*
    std::thread([this]()
    {
        struct pollfd fodUiPoll =
        {
            .fd = open(FOD_UI_PATH, O_RDONLY),
            .events = POLLERR | POLLPRI,
            .revents = 0,
        };

        while (fodUiPoll.fd > 0)
        {
            char c = '0';

            if (poll(&(fodUiPoll), 1, -1) < 0) continue;

            lseek(fodUiPoll.fd, 0, SEEK_SET);
            read(fodUiPoll.fd, &(c), sizeof(c));

            xiaomiFingerprintService->extCmd(COMMAND_NIT, (c != '0') ? PARAM_NIT_FOD : PARAM_NIT_NONE);
        }

    }).detach();
*/
}

Return<int32_t> FingerprintInscreen::getPositionX()
{
    return FOD_SENSOR_X;
}

Return<int32_t> FingerprintInscreen::getPositionY()
{
    return FOD_SENSOR_Y;
}

Return<int32_t> FingerprintInscreen::getSize()
{
    return FOD_SENSOR_SIZE;
}

Return<void> FingerprintInscreen::onShowFODView()
{
    set(FOD_STATUS_PATH, FOD_STATUS_ON);

    return Void();
}

Return<void> FingerprintInscreen::onHideFODView()
{
    set(FOD_STATUS_PATH, FOD_STATUS_OFF);

    return Void();
}

Return<void> FingerprintInscreen::onPress()
{
    set(DISPPARAM_PATH, DISPPARAM_HBM_FOD_ON);

    xiaomiFingerprintService->extCmd(COMMAND_NIT, PARAM_NIT_FOD);

    return Void();
}

Return<void> FingerprintInscreen::onRelease()
{
    set(DISPPARAM_PATH, DISPPARAM_HBM_FOD_OFF);

    xiaomiFingerprintService->extCmd(COMMAND_NIT, PARAM_NIT_NONE);

    return Void();
}

Return<void> FingerprintInscreen::onStartEnroll()
{
    return Void();
}

Return<void> FingerprintInscreen::onFinishEnroll()
{
    return Void();
}

Return<bool> FingerprintInscreen::handleAcquired(int32_t acquiredInfo, int32_t vendorCode)
{
    std::lock_guard<std::mutex> _lock(mCallbackLock);

    if ((mCallback != nullptr) && (acquiredInfo == 6))
    {
        if (vendorCode == 22) return mCallback->onFingerDown().isOk();
        if (vendorCode == 23) return mCallback->onFingerUp().isOk();
    }

    return false;
}

Return<bool> FingerprintInscreen::handleError(int32_t, int32_t) //(int32_t error, int32_t vendorCode)
{
    return false;
}

Return<void> FingerprintInscreen::setLongPressEnabled(bool)
{
    return Void();
}

Return<int32_t> FingerprintInscreen::getDimAmount(int32_t brightness)
{
    LOG(INFO) << "FingerprintInscreen.brightness: " << brightness;

    return 255 - brightness; //230;
}

Return<bool> FingerprintInscreen::shouldBoostBrightness()
{
    return true;
}

Return<void> FingerprintInscreen::setCallback(const sp<IFingerprintInscreenCallback>& callback)
{
    std::lock_guard<std::mutex> _lock(mCallbackLock);
    mCallback = callback;
    
    return Void();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace inscreen
}  // namespace fingerprint
}  // namespace biometrics
}  // namespace lineage
}  // namespace vendor
