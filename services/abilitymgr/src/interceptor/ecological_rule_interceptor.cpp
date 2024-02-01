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

#include "ability_util.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "parameters.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE = "persist.sys.abilityms.support.ecologicalrulemgrservice";
}
ErrCode EcologicalRuleInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
    const sptr<IRemoteObject> &callerToken)
{
    if (want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME) ==
        want.GetElement().GetBundleName()) {
        HILOG_DEBUG("The same bundle, do not intercept.");
        return ERR_OK;
    }
    ErmsCallerInfo callerInfo;
    ExperienceRule rule;
    GetEcologicalCallerInfo(want, callerInfo, userId);
    std::string supportErms = OHOS::system::GetParameter(ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE, "true");
    if (supportErms == "false") {
        HILOG_ERROR("Abilityms not support Erms between applications.");
        return ERR_OK;
    }

    int ret = IN_PROCESS_CALL(AbilityEcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(want,
        callerInfo, rule));
    if (ret != ERR_OK) {
        HILOG_DEBUG("check ecological rule failed, keep going.");
        return ERR_OK;
    }
    HILOG_DEBUG("check ecological rule success");
    if (rule.isAllow) {
        HILOG_ERROR("ecological rule is allow, keep going.");
        return ERR_OK;
    }
#ifdef SUPPORT_GRAPHICS
    if (isForeground && rule.replaceWant) {
        (const_cast<Want &>(want)) = *rule.replaceWant;
        (const_cast<Want &>(want)).SetParam("queryWantFromErms", true);
    }
#endif
    return ERR_ECOLOGICAL_CONTROL_STATUS;
}

void EcologicalRuleInterceptor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingPid());
    callerInfo.targetAppType = ErmsCallerInfo::TYPE_INVALID;
    callerInfo.callerAppType = ErmsCallerInfo::TYPE_INVALID;

    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return;
    }

    std::string targetBundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo targetAppInfo;
    bool getTargetResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(targetBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    if (!getTargetResult) {
        HILOG_ERROR("Get targetAppInfo failed.");
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the target type  is atomic service");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the target type is app");
        callerInfo.targetAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
    }

    std::string callerBundleName;
    ErrCode err = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerInfo.uid, callerBundleName));
    if (err != ERR_OK) {
        HILOG_ERROR("Get callerBundleName failed.");
        return;
    }
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool getCallerResult = IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(callerBundleName,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    if (!getCallerResult) {
        HILOG_DEBUG("Get callerAppInfo failed.");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the caller type  is atomic service");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_ATOM_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the caller type is app");
        callerInfo.callerAppType = ErmsCallerInfo::TYPE_HARMONY_APP;
        if (callerInfo.packageName == "") {
            callerInfo.packageName = callerAppInfo.name;
        }
    }
}
} // namespace AAFwk
} // namespace OHOS