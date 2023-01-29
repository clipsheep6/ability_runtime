/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H

#include <string>
#include <unordered_set>

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_manager_client.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_verification.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
constexpr const char* DLP_ABILITY_NAME = "ViewAbility";
constexpr const char* DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
constexpr const char* DLP_PARAMS_BUNDLE_NAME = "ohos.dlp.params.bundleName";
constexpr const char* DLP_PARAMS_MODULE_NAME = "ohos.dlp.params.moduleName";
constexpr const char* DLP_PARAMS_ABILITY_NAME = "ohos.dlp.params.abilityName";
const char* MARKET_BUNDLE_NAME = "com.huawei.hmos.appgallery";
const char* BUNDLE_NAME_SELECTOR_DIALOG = "com.ohos.amsdialog";
// dlp White list
const std::unordered_set<std::string> WHITE_LIST_DLP_SET = { BUNDLE_NAME_SELECTOR_DIALOG };

static constexpr unsigned int CHANGE_CONFIG_ALL_CHANGED = 0xFFFFFFFF;
static constexpr unsigned int CHANGE_CONFIG_NONE = 0x00000000;
static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
static constexpr unsigned int CHANGE_CONFIG_LAYOUT = 0x00000002;
static constexpr unsigned int CHANGE_CONFIG_FONTSIZE = 0x00000004;
static constexpr unsigned int CHANGE_CONFIG_ORIENTATION = 0x00000008;
static constexpr unsigned int CHANGE_CONFIG_DENSITY = 0x00000010;

#define CHECK_POINTER_CONTINUE(object)      \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        continue;                           \
    }

#define CHECK_POINTER_IS_NULLPTR(object)    \
    if (object == nullptr) {                \
        HILOG_ERROR("pointer is nullptr."); \
        return;                             \
    }

#define CHECK_POINTER(object)               \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        return;                             \
    }

#define CHECK_POINTER_LOG(object, log)   \
    if (!object) {                       \
        HILOG_ERROR("%{public}s:", log); \
        return;                          \
    }

#define CHECK_POINTER_AND_RETURN(object, value) \
    if (!object) {                              \
        HILOG_ERROR("pointer is nullptr.");     \
        return value;                           \
    }

#define CHECK_POINTER_AND_RETURN_LOG(object, value, log) \
    if (!object) {                                       \
        HILOG_ERROR("%{public}s:", log);                 \
        return value;                                    \
    }

#define CHECK_POINTER_RETURN_BOOL(object)   \
    if (!object) {                          \
        HILOG_ERROR("pointer is nullptr."); \
        return false;                       \
    }

#define CHECK_RET_RETURN_RET(object, log)                         \
    if (object != ERR_OK) {                                       \
        HILOG_ERROR("%{public}s, ret : %{public}d", log, object); \
        return object;                                            \
    }

#define CHECK_TRUE_RETURN_RET(object, value, log) \
    if (object) {                                 \
        HILOG_WARN("%{public}s", log);            \
        return value;                             \
    }

[[maybe_unused]] static bool IsSystemDialogAbility(const std::string &bundleName, const std::string &abilityName)
{
    if (abilityName == AbilityConfig::SYSTEM_DIALOG_NAME && bundleName == AbilityConfig::SYSTEM_UI_BUNDLE_NAME) {
        return true;
    }

    if (abilityName == AbilityConfig::DEVICE_MANAGER_NAME && bundleName == AbilityConfig::DEVICE_MANAGER_BUNDLE_NAME) {
        return true;
    }

    return false;
}

[[maybe_unused]] static std::string ConvertBundleNameSingleton(const std::string &bundleName, const std::string &name,
    const std::string &moduleName, const int32_t appIndex = 0)
{
    std::string strName;
    if (appIndex == 0) {
        strName = AbilityConfig::MISSION_NAME_MARK_HEAD + bundleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + moduleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + name;
    } else {
        strName = AbilityConfig::MISSION_NAME_MARK_HEAD + bundleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + std::to_string(appIndex) +
            AbilityConfig::MISSION_NAME_SEPARATOR + moduleName +
            AbilityConfig::MISSION_NAME_SEPARATOR + name;
    }

    return strName;
}

static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
[[maybe_unused]] static int64_t SystemTimeMillis()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (int64_t)((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS;
}

[[maybe_unused]] static int64_t UTCTimeSeconds()
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (int64_t)(t.tv_sec);
}

[[maybe_unused]] static bool IsStartFreeInstall(const Want &want)
{
    auto flags = want.GetFlags();
    if ((flags & Want::FLAG_INSTALL_ON_DEMAND) == Want::FLAG_INSTALL_ON_DEMAND) {
        return true;
    }
    return false;
}

[[maybe_unused]] static sptr<AppExecFwk::IBundleMgr> GetBundleManager()
{
    auto bundleObj =
        OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bundleObj == nullptr) {
        HILOG_ERROR("failed to get bundle manager service");
        return nullptr;
    }
    return iface_cast<AppExecFwk::IBundleMgr>(bundleObj);
}

[[maybe_unused]] static bool HandleDlpApp(Want &want)
{
    if (WHITE_LIST_DLP_SET.find(want.GetBundle()) != WHITE_LIST_DLP_SET.end()) {
        HILOG_INFO("%{public}s, enter special app", __func__);
        return false;
    }

    AppExecFwk::ElementName element = want.GetElement();
    if (want.GetBoolParam(DLP_PARAMS_SANDBOX, false) && !element.GetBundleName().empty() &&
        !element.GetAbilityName().empty()) {
        want.SetElementName(DLP_BUNDLE_NAME, DLP_ABILITY_NAME);
        want.SetParam(DLP_PARAMS_BUNDLE_NAME, element.GetBundleName());
        want.SetParam(DLP_PARAMS_MODULE_NAME, element.GetModuleName());
        want.SetParam(DLP_PARAMS_ABILITY_NAME, element.GetAbilityName());
        want.RemoveParam(DLP_PARAMS_SANDBOX);
        return true;
    }

    return false;
}

inline int StartAppgallery(const int requestCode, const int32_t userId, const std::string &action)
{
    Want want;
    want.SetElementName(MARKET_BUNDLE_NAME, "");
    want.SetAction(action);
    return AbilityManagerClient::GetInstance()->StartAbility(want, userId, requestCode);
}
}  // namespace AbilityUtil
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_UTIL_H
