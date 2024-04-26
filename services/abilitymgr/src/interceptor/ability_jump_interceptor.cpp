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

#include "interceptor/ability_jump_interceptor.h"

#include "ability_util.h"
#include "accesstoken_kit.h"
#include "app_jump_control_rule.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "start_ability_utils.h"
#include "system_dialog_scheduler.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string JUMP_DIALOG_CALLER_BUNDLE_NAME = "interceptor_callerBundleName";
const std::string JUMP_DIALOG_CALLER_MODULE_NAME = "interceptor_callerModuleName";
const std::string JUMP_DIALOG_CALLER_LABEL_ID = "interceptor_callerLabelId";
const std::string JUMP_DIALOG_TARGET_MODULE_NAME = "interceptor_targetModuleName";
const std::string JUMP_DIALOG_TARGET_LABEL_ID = "interceptor_targetLabelId";
}
ErrCode AbilityJumpInterceptor::DoProcess(AbilityInterceptorParam param)
{
    if (!param.isWithUI) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "This startup is not foreground, keep going.");
        return ERR_OK;
    }
    bool isStartIncludeAtomicService = AbilityUtil::IsStartIncludeAtomicService(param.want, param.userId);
    if (isStartIncludeAtomicService) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "This startup contain atomic service, keep going.");
        return ERR_OK;
    }
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
        return ERR_OK;
    }
    AppExecFwk::AbilityInfo targetAbilityInfo;
    if (StartAbilityUtils::startAbilityInfo != nullptr) {
        targetAbilityInfo = StartAbilityUtils::startAbilityInfo->abilityInfo;
    } else {
        IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(param.want,
            AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, param.userId, targetAbilityInfo));
    }
    if (targetAbilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "Target is not page Ability, keep going, abilityType:%{public}d.", targetAbilityInfo.type);
        return ERR_OK;
    }
    AppExecFwk::AppJumpControlRule controlRule;
    if (CheckControl(bundleMgrHelper, param.want, param.userId, controlRule)) {
#ifdef SUPPORT_GRAPHICS
        TAG_LOGI(AAFwkTag::ABILITYMGR, "app jump need to be intercepted, caller:%{public}s, target:%{public}s",
            controlRule.callerPkg.c_str(), controlRule.targetPkg.c_str());
        auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
        Want targetWant = param.want;
        Want dialogWant = sysDialogScheduler->GetJumpInterceptorDialogWant(targetWant);
        AbilityUtil::ParseJumpInterceptorWant(dialogWant, controlRule.callerPkg);
        LoadAppLabelInfo(dialogWant, controlRule, param.userId);
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(dialogWant,
            param.requestCode, param.userId));
        if (ret != ERR_OK) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "appInterceptor Dialog StartAbility error, ret:%{public}d", ret);
            return ret;
        }
#endif
        return ERR_APP_JUMP_INTERCEPTOR_STATUS;
    }
    return ERR_OK;
}

bool AbilityJumpInterceptor::CheckControl(std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper,
    const Want &want, int32_t userId, AppExecFwk::AppJumpControlRule &controlRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    int callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bundleMgrHelper->GetNameForUid(callerUid, callerBundleName));
    std::string targetBundleName = want.GetBundle();
    controlRule.callerPkg = callerBundleName;
    controlRule.targetPkg = targetBundleName;
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleName from bms fail.");
        return false;
    }
    if (controlRule.callerPkg.empty() || controlRule.targetPkg.empty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "This startup is not explicitly, keep going.");
        return false;
    }
    if (controlRule.callerPkg == controlRule.targetPkg) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Jump within the same app.");
        return false;
    }
    if (CheckIfJumpExempt(controlRule, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Jump from or to system or exempt apps.");
        return false;
    }
    // get disposed status
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Get appControlMgr failed.");
        return false;
    }

    if (IN_PROCESS_CALL(appControlMgr->GetAppJumpControlRule(callerBundleName, targetBundleName,
        userId, controlRule)) != ERR_OK) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "No jump control rule found.");
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Get appJumpControlRule, jumpMode:%d.", controlRule.jumpMode);
    return controlRule.jumpMode != AppExecFwk::AbilityJumpMode::DIRECT;
}

bool AbilityJumpInterceptor::CheckIfJumpExempt(AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    if (CheckIfExemptByBundleName(controlRule.callerPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_CALLER, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Jump from exempt caller app, No need to intercept.");
        return true;
    }
    if (CheckIfExemptByBundleName(controlRule.targetPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_TARGET, userId)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Jump to exempt target app, No need to intercept.");
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Third-party apps jump to third-party apps.");
    return false;
}

bool AbilityJumpInterceptor::CheckIfExemptByBundleName(const std::string &bundleName,
    const std::string &permission, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo appInfo;
    if (!StartAbilityUtils::GetApplicationInfo(bundleName, userId, appInfo)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get application info.");
        return false;
    }

    if (appInfo.isSystemApp) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Bundle:%{public}s is system app.", bundleName.c_str());
        return true;
    }
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission, false);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "VerifyPermission %{public}d: PERMISSION_DENIED.", appInfo.accessTokenId);
        return false;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "Bundle:%{public}s verify permission:%{public}s successed.", bundleName.c_str(), permission.c_str());
    return true;
}

bool AbilityJumpInterceptor::LoadAppLabelInfo(Want &want,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo callerAppInfo;
    StartAbilityUtils::GetApplicationInfo(controlRule.callerPkg, userId, callerAppInfo);
    AppExecFwk::ApplicationInfo targetAppInfo;
    StartAbilityUtils::GetApplicationInfo(controlRule.targetPkg, userId, callerAppInfo);
    want.SetParam(JUMP_DIALOG_CALLER_BUNDLE_NAME, controlRule.callerPkg);
    want.SetParam(JUMP_DIALOG_CALLER_MODULE_NAME, callerAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_CALLER_LABEL_ID, callerAppInfo.labelId);
    want.SetParam(JUMP_DIALOG_TARGET_MODULE_NAME, targetAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_TARGET_LABEL_ID, targetAppInfo.labelId);
    return true;
}
} // namespace AAFwk
} // namespace OHOS