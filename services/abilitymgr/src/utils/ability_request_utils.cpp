/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "ability_request_utils.h"

#include "ability_manager_errors.h"
#include "ability_start_setting.h"
#include "ability_util.h"
#include "application_info.h"
#include "app_recovery/default_recovery_config.h"
#include "app_utils.h"
#include "debug_app_utils.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "permission_verification.h"
#include "server_constant.h"
#include "startup_util.h"

namespace OHOS {
namespace AAFwk {
const std::string DEBUG_APP = "debugApp";
constexpr const char* PROCESS_SUFFIX = "embeddable";

void SetWantDlpIndex(std::shared_ptr<AbilityRecord> abilityRecord, const Want& want)
{
    if (abilityRecord && abilityRecord->GetAppIndex() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        (const_cast<Want &>(want)).SetParam(AbilityRuntime::ServerConstant::DLP_INDEX, abilityRecord->GetAppIndex());
    }
}

void SetAbilityRequestSetting(const Want& want, AbilityRequest &request)
{
    auto setting = AbilityStartSetting::GetEmptySetting();
    if (setting != nullptr) {
        auto bundleName = want.GetElement().GetBundleName();
        auto defaultRecovery = AbilityRuntime::DefaultRecoveryConfig::GetInstance()
            .IsBundleDefaultRecoveryEnabled(bundleName);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "bundleName: %{public}s, defaultRecovery: %{public}d.", bundleName.c_str(),
            defaultRecovery);
        setting->AddProperty(AbilityStartSetting::DEFAULT_RECOVERY_KEY, defaultRecovery ? "true" : "false");
        setting->AddProperty(AbilityStartSetting::IS_START_BY_SCB_KEY, "false"); // default is false
        request.startSetting = std::make_shared<AbilityStartSetting>(*(setting.get()));
    }
}

int InitialAbilityRequest(AbilityRequest &request, const StartAbilityInfo &abilityInfo)
{
    request.abilityInfo = abilityInfo.abilityInfo;
    request.extensionProcessMode = abilityInfo.extensionProcessMode;
    if (request.abilityInfo.applicationInfo.name.empty() || request.abilityInfo.applicationInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get app info failed.");
        return RESOLVE_APP_ERR;
    }
    request.appInfo = request.abilityInfo.applicationInfo;
    request.uid = request.appInfo.uid;
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "GenerateExtensionAbilityRequest end, app name: %{public}s, bundle name: %{public}s, uid: %{public}d.",
        request.appInfo.name.c_str(), request.appInfo.bundleName.c_str(), request.uid);
    request.want.SetParam("send_to_erms_targetBundleType",
        static_cast<int>(request.abilityInfo.applicationInfo.bundleType));
    request.want.SetParam("send_to_erms_targetAppProvisionType", request.abilityInfo.applicationInfo.appProvisionType);

    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "GenerateExtensionAbilityRequest, moduleName: %{public}s.", request.abilityInfo.moduleName.c_str());
    request.want.SetModuleName(request.abilityInfo.moduleName);

    return ERR_OK;
}

void SetRequestCollaboratorType(AbilityRequest &request)
{
    if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::RESERVE_TYPE)) {
        request.collaboratorType = CollaboratorType::RESERVE_TYPE;
    } else if (request.abilityInfo.applicationInfo.codePath == std::to_string(CollaboratorType::OTHERS_TYPE)) {
        request.collaboratorType = CollaboratorType::OTHERS_TYPE;
    }
}

int AbilityRequestUtils::GenerateAbilityRequest(const Want &want, int requestCode, AbilityRequest &request,
    sptr<IRemoteObject> callerToken, int32_t userId, bool isNeedSetDebugApp)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    SetWantDlpIndex(abilityRecord, want);

    if (abilityRecord != nullptr) {
        (const_cast<Want &>(want)).SetParam(DEBUG_APP, abilityRecord->IsDebugApp());
    }

    request.want = want;
    request.requestCode = requestCode;
    request.callerToken = callerToken;
    SetAbilityRequestSetting(want, request);

    auto abilityInfo = StartAbilityUtils::startAbilityInfo;
    int32_t err = StartAbilityUtils::GenerateStartAbilityInfo(want, userId, abilityInfo);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GenerateStartAbilityInfo returns errcode=%{public}d.", err);
        return err;
    }
    auto result = InitialAbilityRequest(request, *abilityInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "InitialAbilityRequest returns errcode=%{public}d.", result);
        return result;
    }
    request.want.SetParam("send_to_erms_targetAppDistType", request.abilityInfo.applicationInfo.appDistributionType);

    SetRequestCollaboratorType(request);

    if (request.abilityInfo.type == AppExecFwk::AbilityType::SERVICE && request.abilityInfo.isStageBasedModel) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Stage mode, abilityInfo SERVICE type reset EXTENSION.");
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    }

    if (want.GetIntParam(AAFwk::SCREEN_MODE_KEY, ScreenMode::IDLE_SCREEN_MODE) == ScreenMode::JUMP_SCREEN_MODE &&
        (request.abilityInfo.applicationInfo.bundleType != AppExecFwk::BundleType::ATOMIC_SERVICE ||
        request.abilityInfo.launchMode != AppExecFwk::LaunchMode::SINGLETON)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The interface of starting atomicService can start only atomicService.");
        return TARGET_ABILITY_NOT_SERVICE;
    }

    request.startRecent = (want.GetBoolParam(Want::PARAM_RESV_START_RECENT, false) &&
        AAFwk::PermissionVerification::GetInstance()->VerifyStartRecentAbilityPermission());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "startRecent=%{public}d.", static_cast<int>(request.startRecent));
    if (isNeedSetDebugApp) {
        DebugAppUtils::SetDebugAppByWaitingDebugFlag(want, request.want,
            request.appInfo.bundleName, request.appInfo.debug);
    }
    return ERR_OK;
}

int AbilityRequestUtils::GenerateExtensionAbilityRequest(const Want &want, AbilityRequest &request,
    sptr<IRemoteObject> callerToken, int32_t userId)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    SetWantDlpIndex(abilityRecord, want);
    request.want = want;
    request.callerToken = callerToken;
    request.startSetting = nullptr;

    auto abilityInfo = StartAbilityUtils::startAbilityInfo;
    int32_t err = StartAbilityUtils::GenerateStartExtensionInfo(want, userId, abilityInfo);
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GenerateStartExtensionInfo returns errcode=%{public}d.", err);
        return err;
    }

    auto result = InitialAbilityRequest(request, *abilityInfo);
    DebugAppUtils::SetDebugAppByWaitingDebugFlag(want, request.want,
        request.appInfo.bundleName, request.appInfo.debug);
    return result;
}

int32_t AbilityRequestUtils::GenerateEmbeddableUIAbilityRequest(const Want &want,
    AbilityRequest &request, sptr<IRemoteObject> callerToken, int32_t userId)
{
    int32_t screenMode = want.GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);
    int32_t result = ERR_OK;
    if (screenMode == AAFwk::EMBEDDED_FULL_SCREEN_MODE) {
        result = GenerateAbilityRequest(want,
            -1, request, callerToken, userId);
        request.abilityInfo.isModuleJson = true;
        request.abilityInfo.isStageBasedModel = true;
        request.abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
        request.abilityInfo.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
        struct timespec time = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &time);
        int64_t times = static_cast<int64_t>(time.tv_sec);
        request.abilityInfo.process = request.abilityInfo.bundleName + PROCESS_SUFFIX + std::to_string(times);
    } else {
        result = GenerateExtensionAbilityRequest(want, request, callerToken, userId);
    }
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS
