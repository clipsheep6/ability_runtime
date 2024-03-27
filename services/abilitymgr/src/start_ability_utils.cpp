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

#include "start_ability_utils.h"

#include "ability_record.h"
#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AAFwk {
namespace {
const char* DLP_INDEX = "ohos.dlp.params.index";
}
thread_local std::shared_ptr<StartAbilityInfo> StartAbilityUtils::startAbilityInfo;

int32_t StartAbilityUtils::GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() != 0 &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        return abilityRecord->GetAppIndex();
    }
    return want.GetIntParam(DLP_INDEX, 0);
}

bool StartAbilityUtils::GetApplicationInfo(const std::string &bundleName, int32_t userId,
    AppExecFwk::ApplicationInfo &appInfo)
{
    if (StartAbilityUtils::startAbilityInfo &&
        StartAbilityUtils::startAbilityInfo->GetAppBundleName() == bundleName) {
        appInfo = StartAbilityUtils::startAbilityInfo->abilityInfo.applicationInfo;
    } else {
        auto bms = AbilityUtil::GetBundleManagerHelper();
        CHECK_POINTER_AND_RETURN(bms, false);
        bool result = IN_PROCESS_CALL(
            bms->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
                userId, appInfo)
        );
        if (!result) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Get app info from bms failed: %{public}s", bundleName.c_str());
            return false;
        }
    }
    return true;
}

StartAbilityInfoWrap::StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex)
{
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "startAbilityInfo has been created");
    }
    StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartAbilityInfo(want,
        validUserId, appIndex);
}

StartAbilityInfoWrap::~StartAbilityInfoWrap()
{
    StartAbilityUtils::startAbilityInfo.reset();
}

void StartAbilityInfo::InitAbilityInfoFromExtension(AppExecFwk::ExtensionAbilityInfo &extensionInfo,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    abilityInfo.applicationName = extensionInfo.applicationInfo.name;
    abilityInfo.applicationInfo = extensionInfo.applicationInfo;
    abilityInfo.bundleName = extensionInfo.bundleName;
    abilityInfo.package = extensionInfo.moduleName;
    abilityInfo.moduleName = extensionInfo.moduleName;
    abilityInfo.name = extensionInfo.name;
    abilityInfo.srcEntrance = extensionInfo.srcEntrance;
    abilityInfo.srcPath = extensionInfo.srcEntrance;
    abilityInfo.iconPath = extensionInfo.icon;
    abilityInfo.iconId = extensionInfo.iconId;
    abilityInfo.label = extensionInfo.label;
    abilityInfo.labelId = extensionInfo.labelId;
    abilityInfo.description = extensionInfo.description;
    abilityInfo.descriptionId = extensionInfo.descriptionId;
    abilityInfo.priority = extensionInfo.priority;
    abilityInfo.permissions = extensionInfo.permissions;
    abilityInfo.readPermission = extensionInfo.readPermission;
    abilityInfo.writePermission = extensionInfo.writePermission;
    abilityInfo.uri = extensionInfo.uri;
    abilityInfo.extensionAbilityType = extensionInfo.type;
    abilityInfo.visible = extensionInfo.visible;
    abilityInfo.resourcePath = extensionInfo.resourcePath;
    abilityInfo.enabled = extensionInfo.enabled;
    abilityInfo.isModuleJson = true;
    abilityInfo.isStageBasedModel = true;
    abilityInfo.process = extensionInfo.process;
    abilityInfo.metadata = extensionInfo.metadata;
    abilityInfo.compileMode = extensionInfo.compileMode;
    abilityInfo.type = AppExecFwk::AbilityType::EXTENSION;
    abilityInfo.extensionTypeName = extensionInfo.extensionTypeName;
    if (!extensionInfo.hapPath.empty()) {
        abilityInfo.hapPath = extensionInfo.hapPath;
    }
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartAbilityInfo(const Want &want, int32_t userId,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, nullptr);
    auto abilityInfoFlag = (AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_PERMISSION |
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_METADATA);
    auto request = std::make_shared<StartAbilityInfo>();
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryAbilityInfo(want, abilityInfoFlag, userId, request->abilityInfo));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxAbilityInfo(want, appIndex,
            abilityInfoFlag, userId, request->abilityInfo));
    }
    if (request->abilityInfo.name.empty() || request->abilityInfo.bundleName.empty()) {
        // try to find extension
        std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
        if (appIndex == 0) {
            IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag,
                userId, extensionInfos));
        } else {
            IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
                abilityInfoFlag, userId, extensionInfos));
        }
        if (extensionInfos.size() <= 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Get extension info failed.");
            request->status = RESOLVE_ABILITY_ERR;
            return request;
        }

        AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
        if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
            request->status = RESOLVE_ABILITY_ERR;
            return request;
        }
        request->extensionProcessMode = extensionInfo.extensionProcessMode;
        // For compatibility translates to AbilityInfo
        InitAbilityInfoFromExtension(extensionInfo, request->abilityInfo);
    }
    return request;
}
}
}