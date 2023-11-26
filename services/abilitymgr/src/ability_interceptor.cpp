/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_interceptor.h"

#include <chrono>

#include "ability_info.h"
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_jump_control_rule.h"
#include "app_running_control_rule_result.h"
#include "bundlemgr/bundle_mgr_interface.h"
#include "bundle_constants.h"
#ifndef SUPPORT_ERMS
#include "erms_mgr_interface.h"
#include "erms_mgr_param.h"
#endif
#include "hilog_wrapper.h"
#include "iservice_registry.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_dialog_scheduler.h"
#include "want.h"
namespace OHOS {
namespace AAFwk {
#ifdef SUPPORT_ERMS
using namespace OHOS::EcologicalRuleMgrService;

constexpr int32_t TYPE_HARMONY_INVALID = 0;
constexpr int32_t TYPE_HARMONY_APP = 1;
constexpr int32_t TYPE_HARMONY_SERVICE = 2;
#else
using ErmsCallerInfo = OHOS::AppExecFwk::ErmsParams::CallerInfo;
using ExperienceRule = OHOS::AppExecFwk::ErmsParams::ExperienceRule;
#endif

const std::string ACTION_MARKET_CROWDTEST = "ohos.want.action.marketCrowdTest";
const std::string ACTION_MARKET_DISPOSED = "ohos.want.action.marketDisposed";
const std::string PERMISSION_MANAGE_DISPOSED_APP_STATUS = "ohos.permission.MANAGE_DISPOSED_APP_STATUS";
const std::string JUMP_DIALOG_CALLER_BUNDLE_NAME = "interceptor_callerBundleName";
const std::string JUMP_DIALOG_CALLER_MODULE_NAME = "interceptor_callerModuleName";
const std::string JUMP_DIALOG_CALLER_LABEL_ID = "interceptor_callerLabelId";
const std::string JUMP_DIALOG_TARGET_MODULE_NAME = "interceptor_targetModuleName";
const std::string JUMP_DIALOG_TARGET_LABEL_ID = "interceptor_targetLabelId";
const std::string UNREGISTER_EVENT_TASK = "unregister event task";
constexpr int KILL_PROCESS_DELAYTIME_MICRO_SECONDS = 5000;

ErrCode CrowdTestInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    if (CheckCrowdtest(want, userId)) {
        HILOG_ERROR("Crowdtest expired.");
#ifdef SUPPORT_GRAPHICS
        if (isForeground) {
            int ret = IN_PROCESS_CALL(AbilityUtil::StartAppgallery(requestCode, userId, ACTION_MARKET_CROWDTEST));
            if (ret != ERR_OK) {
                HILOG_ERROR("Crowdtest implicit start appgallery failed.");
                return ret;
            }
        }
#endif
        return ERR_CROWDTEST_EXPIRED;
    }
    return ERR_OK;
}

bool CrowdTestInterceptor::CheckCrowdtest(const Want &want, int32_t userId)
{
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    // get crowdtest status and time
    std::string bundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bms->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
            userId, callerAppInfo)
    );
    if (!result) {
        HILOG_ERROR("GetApplicaionInfo from bms failed.");
        return false;
    }

    auto appDistributionType = callerAppInfo.appDistributionType;
    auto appCrowdtestDeadline = callerAppInfo.crowdtestDeadline;
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if (appDistributionType == AppExecFwk::Constants::APP_DISTRIBUTION_TYPE_CROWDTESTING &&
        appCrowdtestDeadline < now) {
        HILOG_INFO("The application is expired, expired time is %{public}s",
            std::to_string(appCrowdtestDeadline).c_str());
        return true;
    }
    return false;
}

ErrCode ControlInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    AppExecFwk::AppRunningControlRuleResult controlRule;
    if (CheckControl(want, userId, controlRule)) {
        HILOG_INFO("The target application is intercpted. %{public}s", controlRule.controlMessage.c_str());
#ifdef SUPPORT_GRAPHICS
        if (isForeground && controlRule.controlWant != nullptr) {
            int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*controlRule.controlWant,
                requestCode, userId));
            if (ret != ERR_OK) {
                HILOG_ERROR("Control implicit start appgallery failed.");
                return ret;
            }
        }
#endif
        if (controlRule.isEdm) {
            return ERR_EDM_APP_CONTROLLED;
        }
        return ERR_APP_CONTROLLED;
    }
    return ERR_OK;
}

bool ControlInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::AppRunningControlRuleResult &controlRule)
{
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bms->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed");
        return false;
    }

    auto ret = IN_PROCESS_CALL(appControlMgr->GetAppRunningControlRule(bundleName, userId, controlRule));
    if (ret != ERR_OK) {
        HILOG_DEBUG("Get No AppRunningControlRule");
        return false;
    }
    return true;
}

ErrCode DisposedRuleInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    HILOG_DEBUG("Call");
    AppExecFwk::DisposedRule disposedRule;
    if (CheckControl(want, userId, disposedRule)) {
        HILOG_INFO("The target ability is intercpted.");
#ifdef SUPPORT_GRAPHICS
        if (isForeground && disposedRule.want != nullptr
            && disposedRule.disposedType != AppExecFwk::DisposedType::NON_BLOCK
            && disposedRule.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
            int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule.want,
                requestCode, userId));
            if (ret != ERR_OK) {
                HILOG_ERROR("DisposedRuleInterceptor start ability failed.");
                return ret;
            }
        }
#endif
        if (disposedRule.isEdm) {
            return ERR_EDM_APP_CONTROLLED;
        }
        return ERR_APP_CONTROLLED;
    }
    if (disposedRule.disposedType == AppExecFwk::DisposedType::NON_BLOCK) {
        HILOG_INFO("not block");
        auto disposedObserver = sptr<DisposedObserver>::MakeSptr(disposedRule);
        if (!disposedObserver) {
            HILOG_ERROR("disposedObserver is nullptr");
            return ERR_INVALID_VALUE;
        }
        sptr<OHOS::AppExecFwk::IAppMgr> appManager = disposedObserver->GetAppMgr();
        std::vector<std::string> bundleNameList;
        bundleNameList.push_back(want.GetBundle());
        int32_t ret = IN_PROCESS_CALL(appManager->RegisterApplicationStateObserver(disposedObserver, bundleNameList));
        if (ret != 0) {
            HILOG_ERROR("register to appmanager failed. err:%{public}d", ret);
            disposedObserver = nullptr;
            return ret;
        }
        auto unregisterTask = [appManager, disposedObserver] () {
            HILOG_ERROR("unregister observer timeout, need unregister again");
            IN_PROCESS_CALL(appManager->UnregisterApplicationStateObserver(disposedObserver));
        };
        taskHandler_->SubmitTask(unregisterTask, UNREGISTER_EVENT_TASK, KILL_PROCESS_DELAYTIME_MICRO_SECONDS);
    }
    return ERR_OK;
}

bool DisposedRuleInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::DisposedRule &disposedRule)
{
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bms->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed");
        return false;
    }
    std::vector<AppExecFwk::DisposedRule> disposedRuleList;

    auto ret = IN_PROCESS_CALL(appControlMgr->GetAbilityRunningControlRule(bundleName,
        userId, disposedRuleList));
    if (ret != ERR_OK || disposedRuleList.empty()) {
        HILOG_DEBUG("Get No DisposedRule");
        return false;
    }

    for (auto &rule:disposedRuleList) {
        if (CheckDisposedRule(want, rule)) {
            disposedRule = rule;
            return true;
        }
    }
    int priority = -1;
    for (auto &rule : disposedRuleList) {
        if (rule.disposedType != AppExecFwk::DisposedType::NON_BLOCK) {
            return false;
        }
        if (rule.priority > priority) {
            priority = rule.priority;
            disposedRule = rule;
        }
    }
    return false;
}

bool DisposedRuleInterceptor::CheckDisposedRule(const Want &want, AppExecFwk::DisposedRule &disposedRule)
{
    if (disposedRule.disposedType == AppExecFwk::DisposedType::NON_BLOCK) {
        return false;
    }
    bool isAllowed = disposedRule.controlType == AppExecFwk::ControlType::ALLOWED_LIST;
    if (disposedRule.disposedType == AppExecFwk::DisposedType::BLOCK_APPLICATION) {
        return !isAllowed;
    }

    std::string moduleName = want.GetElement().GetModuleName();
    std::string abilityName = want.GetElement().GetAbilityName();

    for (auto elementName : disposedRule.elementList) {
        if (moduleName == elementName.GetModuleName()
            && abilityName == elementName.GetAbilityName()) {
            return !isAllowed;
        }
    }
    return isAllowed;
}

ErrCode EcologicalRuleInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    ErmsCallerInfo callerInfo;
    ExperienceRule rule;
#ifdef SUPPORT_ERMS
    GetEcologicalCallerInfo(want, callerInfo, userId);
    int ret = IN_PROCESS_CALL(EcologicalRuleMgrServiceClient::GetInstance()->QueryStartExperience(want,
        callerInfo, rule));
    if (ret != ERR_OK) {
        HILOG_ERROR("check ecological rule failed, keep going.");
        return ERR_OK;
    }
#else
    int ret = CheckRule(want, callerInfo, rule);
    if (!ret) {
        HILOG_ERROR("check ecological rule failed, keep going.");
        return ERR_OK;
    }
#endif
    HILOG_DEBUG("check ecological rule success");
    if (rule.isAllow) {
        HILOG_ERROR("ecological rule is allow, keep going.");
        return ERR_OK;
    }
#ifdef SUPPORT_GRAPHICS
    if (isForeground && rule.replaceWant) {
        (const_cast<Want &>(want)) = *rule.replaceWant;
        (const_cast<Want &>(want)).SetParam("isReplaceWantExist", true);
    }
#endif
    return ERR_ECOLOGICAL_CONTROL_STATUS;
}

#ifdef SUPPORT_ERMS
void EcologicalRuleInterceptor::GetEcologicalCallerInfo(const Want &want, ErmsCallerInfo &callerInfo, int32_t userId)
{
    callerInfo.packageName = want.GetStringParam(Want::PARAM_RESV_CALLER_BUNDLE_NAME);
    callerInfo.uid = want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid());
    callerInfo.pid = want.GetIntParam(Want::PARAM_RESV_CALLER_PID, IPCSkeleton::GetCallingPid());
    callerInfo.targetAppType = TYPE_HARMONY_INVALID;
    callerInfo.callerAppType = TYPE_HARMONY_INVALID;

    auto bms = AbilityUtil::GetBundleManager();
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
        HILOG_DEBUG("the target type  is atomic service");
        callerInfo.targetAppType = TYPE_HARMONY_SERVICE;
    } else if (targetAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the target type is app");
        callerInfo.targetAppType = TYPE_HARMONY_APP;
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
        HILOG_DEBUG("Get callerAppInfo failed.");
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::ATOMIC_SERVICE) {
        HILOG_DEBUG("the caller type  is atomic service");
        callerInfo.callerAppType = TYPE_HARMONY_SERVICE;
    } else if (callerAppInfo.bundleType == AppExecFwk::BundleType::APP) {
        HILOG_DEBUG("the caller type is app");
        callerInfo.callerAppType = TYPE_HARMONY_APP;
    } else {
        HILOG_DEBUG("the caller type is invalid type");
    }
}
#else
bool EcologicalRuleInterceptor::CheckRule(const Want &want, ErmsCallerInfo &callerInfo, ExperienceRule &rule)
{
    HILOG_DEBUG("Enter Erms CheckRule.");
    auto erms = AbilityUtil::CheckEcologicalRuleMgr();
    if (!erms) {
        HILOG_ERROR("CheckEcologicalRuleMgr failed.");
        return false;
    }
    int ret = IN_PROCESS_CALL(erms->QueryStartExperience(want, callerInfo, rule));
    if (ret != ERR_OK) {
        HILOG_ERROR("Failed to query start experience from erms.");
        return false;
    }

    return true;
}
#endif

ErrCode AbilityJumpInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground)
{
    if (!isForeground) {
        HILOG_INFO("This startup is not foreground, keep going.");
        return ERR_OK;
    }
    bool isStartIncludeAtomicService = AbilityUtil::IsStartIncludeAtomicService(want, userId);
    if (isStartIncludeAtomicService) {
        HILOG_INFO("This startup contain atomic service, keep going.");
        return ERR_OK;
    }
    // get bms
    auto bms = AbilityUtil::GetBundleManager();
    if (!bms) {
        HILOG_ERROR("GetBundleManager failed");
        return ERR_OK;
    }
    AppExecFwk::AbilityInfo targetAbilityInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bms->QueryAbilityInfo(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, targetAbilityInfo));
    if (targetAbilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_INFO("Target is not page Ability, keep going, abilityType:%{public}d", targetAbilityInfo.type);
        return ERR_OK;
    }
    AppExecFwk::AppJumpControlRule controlRule;
    if (CheckControl(bms, want, userId, controlRule)) {
#ifdef SUPPORT_GRAPHICS
        HILOG_INFO("app jump need to be intercepted, caller:%{public}s, target:%{public}s",
            controlRule.callerPkg.c_str(), controlRule.targetPkg.c_str());
        auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
        Want targetWant = want;
        Want dialogWant = sysDialogScheduler->GetJumpInterceptorDialogWant(targetWant);
        AbilityUtil::ParseJumpInterceptorWant(dialogWant, controlRule.callerPkg);
        LoadAppLabelInfo(bms, dialogWant, controlRule, userId);
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(dialogWant,
            requestCode, userId));
        if (ret != ERR_OK) {
            HILOG_INFO("AppInterceptor Dialog StartAbility error, ret:%{public}d", ret);
            return ret;
        }
#endif
        return ERR_APP_JUMP_INTERCEPTOR_STATUS;
    }
    return ERR_OK;
}

bool AbilityJumpInterceptor::CheckControl(sptr<AppExecFwk::IBundleMgr> &bms, const Want &want, int32_t userId,
    AppExecFwk::AppJumpControlRule &controlRule)
{
    int callerUid = IPCSkeleton::GetCallingUid();
    std::string callerBundleName;
    auto result = IN_PROCESS_CALL(bms->GetNameForUid(callerUid, callerBundleName));
    std::string targetBundleName = want.GetBundle();
    controlRule.callerPkg = callerBundleName;
    controlRule.targetPkg = targetBundleName;
    if (result != ERR_OK) {
        HILOG_ERROR("GetBundleName from bms fail.");
        return false;
    }
    if (controlRule.callerPkg.empty() || controlRule.targetPkg.empty()) {
        HILOG_INFO("This startup is not explicitly, keep going.");
        return false;
    }
    if (controlRule.callerPkg == controlRule.targetPkg) {
        HILOG_INFO("jump within the same app.");
        return false;
    }
    if (CheckIfJumpExempt(bms, controlRule, userId)) {
        HILOG_INFO("jump from or to system or exempt apps");
        return false;
    }
    // get disposed status
    auto appControlMgr = bms->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed");
        return false;
    }

    if (IN_PROCESS_CALL(appControlMgr->GetAppJumpControlRule(callerBundleName, targetBundleName,
        userId, controlRule)) != ERR_OK) {
        HILOG_INFO("no jump control rule found");
        return true;
    }
    HILOG_INFO("get appJumpControlRule, jumpMode:%d", controlRule.jumpMode);
    return controlRule.jumpMode != AppExecFwk::AbilityJumpMode::DIRECT;
}

bool AbilityJumpInterceptor::CheckIfJumpExempt(sptr<AppExecFwk::IBundleMgr> &bms,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    if (CheckIfExemptByBundleName(bms, controlRule.callerPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_CALLER, userId)) {
        HILOG_INFO("Jump from exempt caller app, No need to intercept");
        return true;
    }
    if (CheckIfExemptByBundleName(bms, controlRule.targetPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_TARGET, userId)) {
        HILOG_INFO("Jump to exempt target app, No need to intercept");
        return true;
    }
    HILOG_INFO("Third-party apps jump to third-party apps");
    return false;
}

bool AbilityJumpInterceptor::CheckIfExemptByBundleName(sptr<AppExecFwk::IBundleMgr> &bms,
    const std::string &bundleName, const std::string &permission, int32_t userId)
{
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bms->GetApplicationInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        userId, appInfo))) {
        HILOG_ERROR("VerifyPermission failed to get application info");
        return false;
    }
    if (appInfo.isSystemApp) {
        HILOG_INFO("bundle:%{public}s is system app", bundleName.c_str());
        return true;
    }
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_DEBUG("VerifyPermission %{public}d: PERMISSION_DENIED", appInfo.accessTokenId);
        return false;
    }
    HILOG_INFO("bundle:%{public}s verify permission:%{public}s successed", bundleName.c_str(), permission.c_str());
    return true;
}

bool AbilityJumpInterceptor::LoadAppLabelInfo(sptr<AppExecFwk::IBundleMgr> &bms, Want &want,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    AppExecFwk::ApplicationInfo callerAppInfo;
    IN_PROCESS_CALL(bms->GetApplicationInfo(controlRule.callerPkg,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    AppExecFwk::ApplicationInfo targetAppInfo;
    IN_PROCESS_CALL(bms->GetApplicationInfo(controlRule.targetPkg,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, targetAppInfo));
    want.SetParam(JUMP_DIALOG_CALLER_BUNDLE_NAME, controlRule.callerPkg);
    want.SetParam(JUMP_DIALOG_CALLER_MODULE_NAME, callerAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_CALLER_LABEL_ID, callerAppInfo.labelId);
    want.SetParam(JUMP_DIALOG_TARGET_MODULE_NAME, targetAppInfo.labelResource.moduleName);
    want.SetParam(JUMP_DIALOG_TARGET_LABEL_ID, targetAppInfo.labelId);
    return true;
}
} // namespace AAFwk
} // namespace OHOS
