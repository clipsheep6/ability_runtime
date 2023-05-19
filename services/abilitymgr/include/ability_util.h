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
#include "app_jump_control_rule.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "erms_mgr_interface.h"
#include "erms_mgr_param.h"
#include "hilog_wrapper.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_verification.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {
namespace AbilityUtil {
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;

constexpr const char* SYSTEM_BASIC = "system_basic";
constexpr const char* SYSTEM_CORE = "system_core";
constexpr const char* DLP_BUNDLE_NAME = "com.ohos.dlpmanager";
constexpr const char* DLP_ABILITY_NAME = "ViewAbility";
constexpr const char* DLP_PARAMS_SANDBOX = "ohos.dlp.params.sandbox";
constexpr const char* DLP_PARAMS_BUNDLE_NAME = "ohos.dlp.params.bundleName";
constexpr const char* DLP_PARAMS_MODULE_NAME = "ohos.dlp.params.moduleName";
constexpr const char* DLP_PARAMS_ABILITY_NAME = "ohos.dlp.params.abilityName";
constexpr int32_t TYPE_HARMONEY_INVALID = 0;
constexpr int32_t TYPE_HARMONEY_APP = 1;
constexpr int32_t TYPE_HARMONEY_SERVICE  = 2;
const std::string MARKET_BUNDLE_NAME = "com.huawei.hmos.appgallery";
const std::string BUNDLE_NAME_SELECTOR_DIALOG = "com.ohos.amsdialog";
const std::string JUMP_INTERCEPTOR_DIALOG_CALLER_PKG = "interceptor_callerPkg";
// dlp White list
const std::unordered_set<std::string> WHITE_LIST_DLP_SET = { BUNDLE_NAME_SELECTOR_DIALOG };

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

[[maybe_unused]] static sptr<AppExecFwk::IEcologicalRuleManager> CheckEcologicalRuleMgr()
{
    // should remove when AG SA online
    int32_t ECOLOGICAL_RULE_SA_ID = 9999;
    auto remoteObject =
            OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->CheckSystemAbility(ECOLOGICAL_RULE_SA_ID);
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s error, failed to check ecological rule manager service.", __func__);
        return nullptr;
    }

    return iface_cast<AppExecFwk::IEcologicalRuleManager>(remoteObject);
}

[[maybe_unused]] static bool ParseJumpInterceptorWant(Want &targetWant, const std::string callerPkg)
{
    if (callerPkg.empty()) {
        HILOG_ERROR("%{public}s error, get empty callerPkg.", __func__);
        return false;
    }
    targetWant.SetParam(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG, callerPkg);
    return true;
}

[[maybe_unused]] static bool CheckJumpInterceptorWant(const Want &targetWant, std::string &callerPkg,
    std::string &targetPkg)
{
    if (!targetWant.HasParameter(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG)) {
        HILOG_ERROR("%{public}s error, the interceptor parameter invalid.", __func__);
        return false;
    }
    callerPkg = targetWant.GetStringParam(JUMP_INTERCEPTOR_DIALOG_CALLER_PKG);
    targetPkg = targetWant.GetElement().GetBundleName();
    return !callerPkg.empty() && !targetPkg.empty();
}

[[maybe_unused]] static bool AddAbilityJumpRuleToBms(const std::string &callerPkg, const std::string &targetPkg,
    int32_t userId)
{
    if (callerPkg.empty() || targetPkg.empty()) {
        HILOG_ERROR("get invalid inputs");
        return false;
    }
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }
    auto appControlMgr = bms->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed");
        return false;
    }
    int ret = IN_PROCESS_CALL(appControlMgr->ConfirmAppJumpControlRule(callerPkg, targetPkg, userId));
    return ret == ERR_OK;
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

[[maybe_unused]] static bool IsStartIncludeAtomicService(const Want &want, const int32_t userId)
{
    auto bms = GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bms->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        HILOG_ERROR("Get targetAppInfo failed in check atomic service.");
        return false;
    }
    if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_INFO("the target is atomic service");
        return true;
    }

    int callerUid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bms->GetNameForUid(callerUid, callerBundleName));
    if (err != ERR_OK) {
        HILOG_ERROR("Get bms failed in check atomic service.");
        return false;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bms->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        HILOG_ERROR("Get callerAppInfo failed in check atomic service.");
        return false;
    }
    if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_INFO("the caller is atomic service");
        return true;
    }
    return false;
}

[[maybe_unused]] static void GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, -1);
    callerInfo.targetAppType = TYPE_HARMONEY_INVALID;
    callerInfo.callerAppType = TYPE_HARMONEY_INVALID;

    auto bms = GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bms->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        HILOG_ERROR("Get targetAppInfo failed.");
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the target type is atomic service");
        callerInfo.targetAppType = TYPE_HARMONEY_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the target type is app");
        callerInfo.targetAppType = TYPE_HARMONEY_APP;
    } else {
        HILOG_DEBUG("the target type is invalid type");
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bms->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        HILOG_ERROR("Get callerBundleName failed.");
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bms->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        HILOG_ERROR("Get callerAppInfo failed.");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the caller type is atomic service");
        callerInfo.callerAppType = TYPE_HARMONEY_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the caller type is app");
        callerInfo.callerAppType = TYPE_HARMONEY_APP;
    } else {
        HILOG_DEBUG("the caller type is invalid type");
    }
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
