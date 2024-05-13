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
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"
#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE = "persist.sys.abilityms.support.ecologicalrulemgrservice";
const std::string BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
}
ErrCode EcologicalRuleInterceptor::DoProcess(AbilityInterceptorParam param)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (StartAbilityUtils::skipErms) {
        return ERR_OK;
    }
    if (param.want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) ==
        param.want.GetElement().GetBundleName()) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "The same bundle, do not intercept.");
        return ERR_OK;
    }
    ErmsCallerInfo callerInfo;
    ExperienceRule rule;
    if (param.callerToken != nullptr) {
        auto abilityRecord = Token::GetAbilityRecordByToken(param.callerToken);
        if (abilityRecord && !abilityRecord->GetAbilityInfo().isStageBasedModel) {
            TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "callerModelType is FA.");
            callerInfo.callerModelType = ErmsCallerInfo::MODEL_FA;
        }
    }
    AAFwk::Want newWant = param.want;
    newWant.RemoveAllFd();
    GetEcologicalCallerInfo(newWant, callerInfo, param.userId);
    std::string supportErms = OHOS::system::GetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    if (supportErms == "false") {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Abilityms not support Erms between applications.");
        return ERR_OK;
    }

    int ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(newWant,
        callerInfo, rule));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule failed, keep going.");
        return ERR_OK;
    }
    TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule success");
    if (rule.isAllow) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "ecological rule is allow, keep going.");
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
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "The same bundle, do not intercept.");
        return true;
    }
    std::string supportErms = OHOS::system::GetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    if (supportErms == "false") {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Abilityms not support Erms between applications.");
        return true;
    }

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    CHECK_POINTER_AND_RETURN(bundleMgrHelper, false);
    Want launchWant;
    auto errCode = IN_PROCESS_CALL(bundleMgrHelper->GetLaunchWantForBundle(want.GetBundle(), launchWant, userId));
    if (errCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "GetLaunchWantForBundle returns %{public}d.", errCode);
        return false;
    }
    want.SetElement(launchWant.GetElement());

    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(want.GetBundle(),
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getCallerResult) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Get targetApplicationInfo failed.");
        return false;
    }
    want.SetParam("send_to_erms_targetAppProvisionType", targetAppInfo.appProvisionType);
    want.SetParam("send_to_erms_targetBundleType", static_cast<int32_t>(targetAppInfo.bundleType));

    ErmsCallerInfo callerInfo;
    GetEcologicalCallerInfo(want, callerInfo, userId);
    ExperienceRule rule;
    AAFwk::Want newWant = want;
    newWant.RemoveAllFd();
    auto ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(newWant,
        callerInfo, rule));
    if (ret != ERR_OK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "check ecological rule failed, keep going.");
        return true;
    }
    return rule.isAllow;
}

void EcologicalRuleInterceptor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    InitErmsCallerInfo(const_cast<Want &>(want), callerInfo);

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "The bundleMgrHelper is nullptr.");
        return;
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        TAG_LOGE(AAFwkTag::ECOLOGICAL_RULE, "Get callerBundleName failed,uid: %{public}d.", callerInfo.uid);
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "Get callerAppInfo failed.");
        return;
    }
    callerInfo.callerAppProvisionType = callerAppInfo.appProvisionType;
    if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the caller type  is atomic service");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the caller type is app");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
        if (callerInfo.packageName == "" && callerAppInfo.name == BUNDLE_NAME_SCENEBOARD) {
            callerInfo.packageName = BUNDLE_NAME_SCENEBOARD;
        }
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP_SERVICE_FWK) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the caller type is app service");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_APP_SERVICE;
    }
}

void EcologicalRuleInterceptor::InitErmsCallerInfo(Want &want, ErmsCallerInfo &callerInfo) const
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingRealPid());
    callerInfo.targetAppType = ErmsCallerInfo::TYPE_INVALID;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_INVALID;
    callerInfo.targetLinkFeature = want.GetStringParam("send_to_erms_targetLinkFeature");
    callerInfo.targetAppDistType = want.GetStringParam("send_to_erms_targetAppDistType");
    callerInfo.targetLinkType = want.GetIntParam("send_to_erms_targetLinkType", 0);
    want.RemoveParam("send_to_erms_targetLinkFeature");
    want.RemoveParam("send_to_erms_targetAppDistType");
    want.RemoveParam("send_to_erms_targetLinkType");
    HILOG_INFO(
        "get callerInfo targetLinkFeature is %{public}s, targetAppDistType is %{public}s, targetLinkType is %{public}d",
        callerInfo.targetLinkFeature.c_str(), callerInfo.targetAppDistType.c_str(), callerInfo.targetLinkType);
    callerInfo.embedded = want.GetIntParam("send_to_erms_embedded", 0);
    callerInfo.targetAppProvisionType = want.GetStringParam("send_to_erms_targetAppProvisionType");

    auto targetBundleType = want.GetIntParam("send_to_erms_targetBundleType", -1);
    want.RemoveParam("send_to_erms_targetBundleType");
    if (targetBundleType == static_cast<int32_t>(AppExecFwk::BundleType::ATOMIC_SERVICE)) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the target type  is atomic service");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    }
    if (targetBundleType == static_cast<int32_t>(AppExecFwk::BundleType::APP)) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the target type is app");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    }
    if (targetBundleType == static_cast<int32_t>(AppExecFwk::BundleType::APP_SERVICE_FWK)) {
        TAG_LOGD(AAFwkTag::ECOLOGICAL_RULE, "the target type is app service");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_APP_SERVICE;
    }
}
} // namespace AAFwk
} // namespace OHOS