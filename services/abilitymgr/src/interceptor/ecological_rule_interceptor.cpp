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

#include "interceptor/ecological_rule_interceptor.h"

#include "ability_record.h"
#include "ability_util.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE =
    "persist.sys.abilityms.support.ecologicalrulemgrservice";
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;
}
ErrCode EcologicalRuleInterceptor::DoProcess(AbilityInterceptorParam param)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (StartAbilityUtils::skipErms) {
        return ERR_OK;
    }
    if (param.want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) ==
        param.want.GetElement().GetBundleName()) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "same bundle");
        return ERR_OK;
    }
    ErmsCallerInfo callerInfo;
    ExperienceRule rule;
    if (param.callerToken != nullptr) {
        auto abilityRecord = Token::GetAbilityRecordByToken(param.callerToken);
        if (abilityRecord && !abilityRecord->GetAbilityInfo().isStageBasedModel) {
            TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "FA callerModelType");
            callerInfo.callerModelType = ErmsCallerInfo::MODEL_FA;
        }
    }
    AAFwk::Want newWant = param.want;
    newWant.RemoveAllFd();
    InitErmsCallerInfo(newWant, param.abilityInfo, callerInfo, param.userId, param.callerToken);

    int ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(newWant,
        callerInfo, rule));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule failed");
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule success");
    StartAbilityUtils::ermsResultCode = rule.resultCode;
    if (rule.resultCode == ERMS_ISALLOW_RESULTCODE) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "allow ecological rule");
        return ERR_OK;
    }
    
    std::string supportErms = OHOS::system::GetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    if (supportErms == "false") {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "erms between apps not supported");
        return ERR_OK;
    }
#ifdef SUPPORT_GRAPHICS
    if (param.isWithUI && rule.replaceWant) {
        (const_cast<Want &>(param.want)) = *rule.replaceWant;
        (const_cast<Want &>(param.want)).SetParam("queryWantFromErms", true);
    }
#endif
    return ERR_ECOLOGICAL_CONTROL_STATUS;
}

bool EcologicalRuleInterceptor::DoProcess(Want &want, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) == want.GetElement().GetBundleName()) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "same bundle");
        return true;
    }
    std::string supportErms = OHOS::system::GetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    if (supportErms == "false") {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Erms between apps not supported");
        return true;
    }

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);
    Want launchWant;
    auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(want.GetBundle(), launchWant, userId));
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "GetLaunchWantForBundle err: %{public}d", errCode);
        return false;
    }
    want.SetElement(launchWant.GetElement());

    int32_t appIndex = 0;
    StartAbilityUtils::startAbilityInfo = StartAbilityInfo::CreateStartAbilityInfo(want,
        userId, appIndex);
    if (StartAbilityUtils::startAbilityInfo->status != ERR_OK) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Get targetApplicationInfo failed");
        return false;
    }

    ErmsCallerInfo callerInfo;
    InitErmsCallerInfo(want, nullptr, callerInfo, userId);

    ExperienceRule rule;
    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    auto ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(newWant,
        callerInfo, rule));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule failed");
        return true;
    }
    return rule.resultCode == ERMS_ISALLOW_RESULTCODE;
}

void EcologicalRuleInterceptor::GetEcologicalTargetInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo, ErmsCallerInfo &callerInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    callerInfo.targetLinkFeature = want.GetStringParam("send_to_erms_targetLinkFeature");
    callerInfo.targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    if (StartAbilityUtils::startAbilityInfo) {
        AppExecFwk::AbilityInfo targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
        callerInfo.targetAppDistType = targetAbilityInfo.applicationInfo.appDistributionType;
        callerInfo.targetAppProvisionType = targetAbilityInfo.applicationInfo.appProvisionType;
        callerInfo.targetAppType = GetAppTypeByBundleType(static_cast<int32_t>(
            targetAbilityInfo.applicationInfo.bundleType));
        callerInfo.targetAbilityType = targetAbilityInfo.type;
        callerInfo.targetExtensionAbilityType = targetAbilityInfo.extensionAbilityType;
    } else if (abilityInfo != nullptr) {
        callerInfo.targetAppDistType = abilityInfo->applicationInfo.appDistributionType;
        callerInfo.targetAppProvisionType = abilityInfo->applicationInfo.appProvisionType;
        callerInfo.targetAppType = GetAppTypeByBundleType(static_cast<int32_t>(
            abilityInfo->applicationInfo.bundleType));
        callerInfo.targetAbilityType = abilityInfo->type;
        callerInfo.targetExtensionAbilityType = abilityInfo->extensionAbilityType;
    }
}

void EcologicalRuleInterceptor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId,
    const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);

    AppExecFwk::ApplicationInfo callerAppInfo;
    AppExecFwk::AbilityInfo callerAbilityInfo;
    if (StartAbilityUtils::GetCallerAbilityInfo(callerToken, callerAbilityInfo)) {
        callerAppInfo = callerAbilityInfo.applicationInfo;
        callerInfo.callerAbilityType = callerAbilityInfo.type;
        callerInfo.callerExtensionAbilityType = callerAbilityInfo.extensionAbilityType;
    } else {
        auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
        if (bundleMgrHelper == nullptr) {
            TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "null bundleMgrHelper");
            return;
        }

        std::string callerBundleName;
        ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
        if (err != ERR_OK) {
            TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Get callerBundleName failed,uid: %{public}d", callerInfo.uid);
            return;
        }
        bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
            AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
        if (!getCallerResult) {
            TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "Get callerAppInfo failed");
            return;
        }
    }
    
    callerInfo.callerAppProvisionType = callerAppInfo.appProvisionType;
    if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "atomic service caller type");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "app caller type");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
        if (callerInfo.packageName == "" && callerAppInfo.name == BUNDLE_NAME_SCENEBOARD) {
            callerInfo.packageName = BUNDLE_NAME_SCENEBOARD;
        }
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP_SERVICE_FWK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "app service caller type");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_APP_SERVICE;
    }
}

void EcologicalRuleInterceptor::InitErmsCallerInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
    ErmsCallerInfo &callerInfo, int32_t userId, const sptr<IRemoteObject> &callerToken)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingPid());
    callerInfo.embedded = want.GetIntParam("send_to_erms_embedded", 0);
    
    GetEcologicalTargetInfo(want, abilityInfo, callerInfo);
    GetEcologicalCallerInfo(want, callerInfo, userId, callerToken);
    TAG_LOGI(AAFwkTag::ECOLOGICAL_RULE, "ERMS's %{public}s", callerInfo.ToString().c_str());
}

int32_t EcologicalRuleInterceptor::GetAppTypeByBundleType(int32_t bundleType)
{
    if (bundleType == static_cast<int32_t>(AppExecFwk::BundleType::ATOMIC_SERVICE)) {
        return ErmsCallerInfo::TYPE_ATOM_SERVICE;
    }
    if (bundleType == static_cast<int32_t>(AppExecFwk::BundleType::APP)) {
        return ErmsCallerInfo::TYPE_HARMONY_APP;
    }
    if (bundleType == static_cast<int32_t>(AppExecFwk::BundleType::APP_SERVICE_FWK)) {
        return ErmsCallerInfo::TYPE_APP_SERVICE;
    }
    return ErmsCallerInfo::TYPE_INVALID;
}
} // namespace AAFwk
} // namespace OHOS