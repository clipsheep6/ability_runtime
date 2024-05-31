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
#include "bundle_constants.h"
#include "bundle_mgr_helper.h"
#include "global_constant.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "server_constant.h"
#include "startup_util.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* SCREENSHOT_BUNDLE_NAME = "com.huawei.ohos.screenshot";
constexpr const char* SCREENSHOT_ABILITY_NAME = "com.huawei.ohos.screenshot.ServiceExtAbility";
}
thread_local std::shared_ptr<StartAbilityInfo> StartAbilityUtils::startAbilityInfo;
thread_local std::shared_ptr<StartAbilityInfo> StartAbilityUtils::callerAbilityInfo;
thread_local bool StartAbilityUtils::skipCrowTest = false;
thread_local bool StartAbilityUtils::skipStartOther = false;
thread_local bool StartAbilityUtils::skipErms = false;

bool StartAbilityUtils::GetAppIndex(const Want &want, sptr<IRemoteObject> callerToken, int32_t &appIndex)
{
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord && abilityRecord->GetAppIndex() > AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX &&
        abilityRecord->GetApplicationInfo().bundleName == want.GetElement().GetBundleName()) {
        appIndex = abilityRecord->GetAppIndex();
        return true;
    }
    return AbilityRuntime::StartupUtil::GetAppIndex(want, appIndex);
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

bool StartAbilityUtils::GetCallerAbilityInfo(const sptr<IRemoteObject> &callerToken,
    AppExecFwk::AbilityInfo &abilityInfo)
{
    if (StartAbilityUtils::callerAbilityInfo) {
        abilityInfo = StartAbilityUtils::callerAbilityInfo->abilityInfo;
    } else {
        if (callerToken == nullptr) {
            return false;
        }
        auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
        if (abilityRecord == nullptr) {
            return false;
        }
        abilityInfo = abilityRecord->GetAbilityInfo();
    }
    return true;
}

StartAbilityInfoWrap::StartAbilityInfoWrap(const Want &want, int32_t validUserId, int32_t appIndex,
    const sptr<IRemoteObject> &callerToken, bool isExtension)
{
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "startAbilityInfo has been created");
    }
    // This is for special goal and could be removed later.
    auto element = want.GetElement();
    if (element.GetAbilityName() == SCREENSHOT_ABILITY_NAME &&
        element.GetBundleName() == SCREENSHOT_BUNDLE_NAME) {
        isExtension = true;
        StartAbilityUtils::skipErms = true;
    }
    if (isExtension) {
        StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartExtensionInfo(want,
            validUserId, appIndex);
    } else {
        StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartAbilityInfo(want,
            validUserId, appIndex);
    }
    if (StartAbilityUtils::startAbilityInfo != nullptr &&
        StartAbilityUtils::startAbilityInfo->abilityInfo.type == AppExecFwk::AbilityType::EXTENSION) {
        StartAbilityUtils::skipCrowTest = true;
        StartAbilityUtils::skipStartOther = true;
    }

    if (StartAbilityUtils::callerAbilityInfo != nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callerAbilityInfo has been created");
    }
    StartAbilityUtils::callerAbilityInfo = StartAbilityInfo::CreateCallerAbilityInfo(callerToken);
}

StartAbilityInfoWrap::~StartAbilityInfoWrap()
{
    StartAbilityUtils::startAbilityInfo.reset();
    StartAbilityUtils::callerAbilityInfo.reset();
    StartAbilityUtils::skipCrowTest = false;
    StartAbilityUtils::skipStartOther = false;
    StartAbilityUtils::skipErms = false;
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartAbilityInfo(const Want &want, int32_t userId,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, nullptr);
    auto abilityInfoFlag = static_cast<uint32_t>(AbilityRuntime::StartupUtil::BuildAbilityInfoFlag()) |
        static_cast<uint32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL);
    auto request = std::make_shared<StartAbilityInfo>();
    if (appIndex > 0 && appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneAbilityInfo(want.GetElement(), abilityInfoFlag, appIndex,
            request->abilityInfo, userId));
        if (request->abilityInfo.name.empty() || request->abilityInfo.bundleName.empty()) {
            FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, request);
        }
        return request;
    }
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
        AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, request->abilityInfo);
    }
    return request;
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateStartExtensionInfo(const Want &want, int32_t userId,
    int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bms, nullptr);
    auto abilityInfoFlag = static_cast<uint32_t>(AbilityRuntime::StartupUtil::BuildAbilityInfoFlag()) |
        static_cast<uint32_t>(AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_SKILL);
    auto abilityInfo = std::make_shared<StartAbilityInfo>();
    if (appIndex > 0 && appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
        FindExtensionInfo(want, abilityInfoFlag, userId, appIndex, abilityInfo);
        return abilityInfo;
    }

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    if (appIndex == 0) {
        IN_PROCESS_CALL_WITHOUT_RET(bms->QueryExtensionAbilityInfos(want, abilityInfoFlag, userId, extensionInfos));
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bms->GetSandboxExtAbilityInfos(want, appIndex,
            abilityInfoFlag, userId, extensionInfos));
    }
    if (extensionInfos.size() <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CreateStartExtensionInfo error. Get extension info failed.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return abilityInfo;
    }

    AppExecFwk::ExtensionAbilityInfo extensionInfo = extensionInfos.front();
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return abilityInfo;
    }
    abilityInfo->extensionProcessMode = extensionInfo.extensionProcessMode;
    // For compatibility translates to AbilityInfo
    AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo->abilityInfo);

    return abilityInfo;
}

void StartAbilityInfo::FindExtensionInfo(const Want &want, int32_t flags, int32_t userId,
    int32_t appIndex, std::shared_ptr<StartAbilityInfo> abilityInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    CHECK_POINTER_LOG(abilityInfo, "abilityInfo is invalid.");
    auto bms = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_LOG(bms, "bms is invalid.");
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bms->QueryCloneExtensionAbilityInfoWithAppIndex(want.GetElement(),
        flags, appIndex, extensionInfo, userId));
    if (extensionInfo.bundleName.empty() || extensionInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "extensionInfo empty.");
        abilityInfo->status = RESOLVE_ABILITY_ERR;
        return;
    }
    if (AbilityRuntime::StartupUtil::IsSupportAppClone(extensionInfo.type)) {
        abilityInfo->extensionProcessMode = extensionInfo.extensionProcessMode;
        // For compatibility translates to AbilityInfo
        AbilityRuntime::StartupUtil::InitAbilityInfoFromExtension(extensionInfo, abilityInfo->abilityInfo);
    } else {
        abilityInfo->status = ERR_APP_CLONE_INDEX_INVALID;
    }
}

std::shared_ptr<StartAbilityInfo> StartAbilityInfo::CreateCallerAbilityInfo(const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (callerToken == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "not call from context.");
        return nullptr;
    }
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "can not find abilityRecord");
        return nullptr;
    }
    auto request = std::make_shared<StartAbilityInfo>();
    request->abilityInfo = abilityRecord->GetAbilityInfo();
    return request;
}
}
}