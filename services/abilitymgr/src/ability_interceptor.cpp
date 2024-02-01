/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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
#include "ability_record.h"
#include "accesstoken_kit.h"
#include "app_jump_control_rule.h"
#include "app_running_control_rule_result.h"
#include "bundle_constants.h"
#include "ecological_rule/ability_ecological_rule_mgr_service.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "in_process_call_wrapper.h"
#include "ipc_skeleton.h"
#include "modal_system_ui_extension.h"
#include "parameters.h"
#include "permission_constants.h"
#include "permission_verification.h"
#include "system_dialog_scheduler.h"
#include "want.h"
#include "want_params_wrapper.h"
namespace OHOS {
namespace AAFwk {

const std::string ACTION_MARKET_CROWDTEST = "ohos.want.action.marketCrowdTest";
const std::string ACTION_MARKET_DISPOSED = "ohos.want.action.marketDisposed";
const std::string PERMISSION_MANAGE_DISPOSED_APP_STATUS = "ohos.permission.MANAGE_DISPOSED_APP_STATUS";
const std::string JUMP_DIALOG_CALLER_BUNDLE_NAME = "interceptor_callerBundleName";
const std::string JUMP_DIALOG_CALLER_MODULE_NAME = "interceptor_callerModuleName";
const std::string JUMP_DIALOG_CALLER_LABEL_ID = "interceptor_callerLabelId";
const std::string JUMP_DIALOG_TARGET_MODULE_NAME = "interceptor_targetModuleName";
const std::string JUMP_DIALOG_TARGET_LABEL_ID = "interceptor_targetLabelId";
const std::string UNREGISTER_EVENT_TASK = "unregister event task";
const std::string UNREGISTER_TIMEOUT_OBSERVER_TASK = "unregister timeout observer task";
const std::string ABILITY_SUPPORT_ECOLOGICAL_RULEMGRSERVICE = "persist.sys.abilityms.support.ecologicalrulemgrservice";
const std::string IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
const std::string INTERCEPT_PARAMETERS = "intercept_parammeters";
const std::string INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
const std::string INTERCEPT_ABILITY_NAME = "intercept_abilityName";
const std::string INTERCEPT_MODULE_NAME = "intercept_moduleName";
constexpr int UNREGISTER_OBSERVER_MICRO_SECONDS = 5000;
#define RETURN_BY_ISEDM(object)                 \
    if (object) {                               \
        return ERR_EDM_APP_CONTROLLED;          \
    }                                           \
    return ERR_APP_CONTROLLED

ErrCode CrowdTestInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
    const sptr<IRemoteObject> &callerToken)
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
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return false;
    }

    // get crowdtest status and time
    std::string bundleName = want.GetBundle();
    AppExecFwk::ApplicationInfo callerAppInfo;
    bool result = IN_PROCESS_CALL(
        bundleMgrHelper->GetApplicationInfo(bundleName, AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO,
            userId, callerAppInfo)
    );
    if (!result) {
        HILOG_DEBUG("GetApplicaionInfo from bms failed.");
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

ErrCode ControlInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
    const sptr<IRemoteObject> &callerToken)
{
    AppExecFwk::AppRunningControlRuleResult controlRule;
    if (CheckControl(want, userId, controlRule)) {
        HILOG_INFO("The target application is intercpted. %{public}s", controlRule.controlMessage.c_str());
#ifdef SUPPORT_GRAPHICS
        if (!isForeground || controlRule.controlWant == nullptr) {
            HILOG_ERROR("Can not start control want");
            RETURN_BY_ISEDM(controlRule.isEdm);
        }
        if (controlRule.controlWant->GetBoolParam(IS_FROM_PARENTCONTROL, false)) {
            auto controlWant = controlRule.controlWant;
            auto controlParam = controlWant->GetParams();
            sptr<AAFwk::IWantParams> interceptParam = WantParamWrapper::Box(want.GetParams());
            if (interceptParam != nullptr) {
                controlParam.SetParam(INTERCEPT_PARAMETERS, interceptParam);
            }
            controlWant->SetParams(controlParam);
            controlWant->SetParam(INTERCEPT_BUNDLE_NAME, want.GetElement().GetBundleName());
            controlWant->SetParam(INTERCEPT_ABILITY_NAME, want.GetElement().GetAbilityName());
            controlWant->SetParam(INTERCEPT_MODULE_NAME, want.GetElement().GetModuleName());
            controlRule.controlWant = controlWant;
        }
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*controlRule.controlWant,
            requestCode, userId));
        if (ret != ERR_OK) {
            HILOG_ERROR("Control implicit start appgallery failed.");
            return ret;
        }
#endif
        RETURN_BY_ISEDM(controlRule.isEdm);
    }
    return ERR_OK;
}

bool ControlInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::AppRunningControlRuleResult &controlRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("The appControlMgr is nullptr.");
        return false;
    }

    auto ret = IN_PROCESS_CALL(appControlMgr->GetAppRunningControlRule(bundleName, userId, controlRule));
    if (ret != ERR_OK) {
        HILOG_DEBUG("Get No AppRunningControlRule.");
        return false;
    }
    return true;
}

ErrCode DisposedRuleInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
    const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("Call");
    AppExecFwk::DisposedRule disposedRule;
    if (CheckControl(want, userId, disposedRule)) {
        HILOG_INFO("The target ability is intercpted.");
#ifdef SUPPORT_GRAPHICS
        if (!isForeground || disposedRule.want == nullptr
            || disposedRule.disposedType == AppExecFwk::DisposedType::NON_BLOCK) {
            HILOG_ERROR("Can not start disposed want");
            RETURN_BY_ISEDM(disposedRule.isEdm);
        }
        if (disposedRule.want->GetBundle() == want.GetBundle()) {
            HILOG_ERROR("Can not start disposed want with same bundleName");
            RETURN_BY_ISEDM(disposedRule.isEdm);
        }
        if (disposedRule.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
            int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule.want,
                requestCode, userId));
            if (ret != ERR_OK) {
                HILOG_ERROR("DisposedRuleInterceptor start ability failed.");
                return ret;
            }
        }
        if (disposedRule.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
            int ret = CreateModalUIExtension(*disposedRule.want, callerToken);
            if (ret != ERR_OK) {
                HILOG_ERROR("failed to start disposed UIExtension");
                return ret;
            }
        }
#endif
        RETURN_BY_ISEDM(disposedRule.isEdm);
    }
    if (disposedRule.disposedType != AppExecFwk::DisposedType::NON_BLOCK) {
        return ERR_OK;
    }
    return StartNonBlockRule(want, disposedRule);
}

bool DisposedRuleInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::DisposedRule &disposedRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("The appControlMgr is nullptr.");
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

ErrCode DisposedRuleInterceptor::StartNonBlockRule(const Want &want, AppExecFwk::DisposedRule &disposedRule)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("not block");
    if (disposedRule.want == nullptr) {
        HILOG_ERROR("Can not start disposed app, want is nullptr");
        return ERR_OK;
    }
    if (disposedRule.want->GetBundle() == want.GetBundle()) {
        HILOG_ERROR("Can not start disposed app with same bundleName");
        return ERR_OK;
    }
    std::string bundleName = want.GetBundle();
    {
        std::lock_guard<ffrt::mutex> guard(observerLock_);
        if (disposedObserverMap_.find(bundleName) != disposedObserverMap_.end()) {
            HILOG_DEBUG("start same disposed app, do not need to register again");
            return ERR_OK;
        }
    }
    auto disposedObserver = sptr<DisposedObserver>::MakeSptr(disposedRule, shared_from_this());
    CHECK_POINTER_AND_RETURN(disposedObserver, ERR_INVALID_VALUE);
    sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppMgr();
    CHECK_POINTER_AND_RETURN(appManager, ERR_INVALID_VALUE);
    std::vector<std::string> bundleNameList;
    bundleNameList.push_back(bundleName);
    int32_t ret = IN_PROCESS_CALL(appManager->RegisterApplicationStateObserver(disposedObserver, bundleNameList));
    if (ret != 0) {
        HILOG_ERROR("register to appmanager failed. err:%{public}d", ret);
        disposedObserver = nullptr;
        return ret;
    }
    {
        std::lock_guard<ffrt::mutex> guard(observerLock_);
        disposedObserverMap_.emplace(bundleName, disposedObserver);
    }
    auto unregisterTask = [appManager, bundleName, interceptor = shared_from_this()] () {
        std::lock_guard<ffrt::mutex> guard{interceptor->observerLock_};
        auto iter = interceptor->disposedObserverMap_.find(bundleName);
        if (iter != interceptor->disposedObserverMap_.end()) {
            HILOG_ERROR("start disposed app time out, need to unregister observer");
            IN_PROCESS_CALL(appManager->UnregisterApplicationStateObserver(iter->second));
            interceptor->disposedObserverMap_.erase(iter);
        }
    };
    taskHandler_->SubmitTask(unregisterTask, UNREGISTER_TIMEOUT_OBSERVER_TASK, UNREGISTER_OBSERVER_MICRO_SECONDS);
    return ERR_OK;
}

sptr<OHOS::AppExecFwk::IAppMgr> DisposedRuleInterceptor::GetAppMgr()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!systemAbilityManager) {
        HILOG_ERROR("get systemAbilityManager failed");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    if (!object) {
        HILOG_ERROR("get systemAbilityManager failed");
        return nullptr;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = iface_cast<AppExecFwk::IAppMgr>(object);
    if (!appMgr || !appMgr->AsObject()) {
        return nullptr;
    }
    return appMgr;
}

void DisposedRuleInterceptor::UnregisterObserver(const std::string &bundleName)
{
    HILOG_DEBUG("Call");
    taskHandler_->CancelTask(UNREGISTER_TIMEOUT_OBSERVER_TASK);
    auto unregisterTask = [bundleName, interceptor = shared_from_this()] () {
        std::lock_guard<ffrt::mutex> guard{interceptor->observerLock_};
        auto iter = interceptor->disposedObserverMap_.find(bundleName);
        if (iter == interceptor->disposedObserverMap_.end()) {
            HILOG_ERROR("Can not find observer");
        } else {
            auto disposedObserver = iter->second;
            CHECK_POINTER(disposedObserver);
            sptr<OHOS::AppExecFwk::IAppMgr> appManager = interceptor->GetAppMgr();
            CHECK_POINTER(appManager);
            IN_PROCESS_CALL(appManager->UnregisterApplicationStateObserver(disposedObserver));
            interceptor->disposedObserverMap_.erase(iter);
        }
    };
    taskHandler_->SubmitTask(unregisterTask, UNREGISTER_EVENT_TASK);
}

ErrCode DisposedRuleInterceptor::CreateModalUIExtension(const Want &want, const sptr<IRemoteObject> &callerToken)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    auto abilityRecord = Token::GetAbilityRecordByToken(callerToken);
    if (abilityRecord == nullptr) {
        auto systemUIExtension = std::make_shared<OHOS::Rosen::ModalSystemUiExtension>();
        return systemUIExtension->CreateModalUIExtension(want);
    } else {
        return abilityRecord->CreateModalUIExtension(want);
    }
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
        if (callerInfo.packageName == "") {
            callerInfo.packageName = targetAppInfo.name;
        }
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
    }
}

ErrCode AbilityJumpInterceptor::DoProcess(const Want &want, int requestCode, int32_t userId, bool isForeground,
    const sptr<IRemoteObject> &callerToken)
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
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        HILOG_ERROR("The bundleMgrHelper is nullptr.");
        return ERR_OK;
    }
    AppExecFwk::AbilityInfo targetAbilityInfo;
    IN_PROCESS_CALL_WITHOUT_RET(bundleMgrHelper->QueryAbilityInfo(want,
        AppExecFwk::AbilityInfoFlag::GET_ABILITY_INFO_WITH_APPLICATION, userId, targetAbilityInfo));
    if (targetAbilityInfo.type != AppExecFwk::AbilityType::PAGE) {
        HILOG_INFO("Target is not page Ability, keep going, abilityType:%{public}d.", targetAbilityInfo.type);
        return ERR_OK;
    }
    AppExecFwk::AppJumpControlRule controlRule;
    if (CheckControl(bundleMgrHelper, want, userId, controlRule)) {
#ifdef SUPPORT_GRAPHICS
        HILOG_INFO("app jump need to be intercepted, caller:%{public}s, target:%{public}s",
            controlRule.callerPkg.c_str(), controlRule.targetPkg.c_str());
        auto sysDialogScheduler = DelayedSingleton<SystemDialogScheduler>::GetInstance();
        Want targetWant = want;
        Want dialogWant = sysDialogScheduler->GetJumpInterceptorDialogWant(targetWant);
        AbilityUtil::ParseJumpInterceptorWant(dialogWant, controlRule.callerPkg);
        LoadAppLabelInfo(bundleMgrHelper, dialogWant, controlRule, userId);
        int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(dialogWant,
            requestCode, userId));
        if (ret != ERR_OK) {
            HILOG_INFO("appInterceptor Dialog StartAbility error, ret:%{public}d", ret);
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
        HILOG_ERROR("GetBundleName from bms fail.");
        return false;
    }
    if (controlRule.callerPkg.empty() || controlRule.targetPkg.empty()) {
        HILOG_INFO("This startup is not explicitly, keep going.");
        return false;
    }
    if (controlRule.callerPkg == controlRule.targetPkg) {
        HILOG_INFO("Jump within the same app.");
        return false;
    }
    if (CheckIfJumpExempt(bundleMgrHelper, controlRule, userId)) {
        HILOG_INFO("Jump from or to system or exempt apps.");
        return false;
    }
    // get disposed status
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        HILOG_ERROR("Get appControlMgr failed.");
        return false;
    }

    if (IN_PROCESS_CALL(appControlMgr->GetAppJumpControlRule(callerBundleName, targetBundleName,
        userId, controlRule)) != ERR_OK) {
        HILOG_INFO("No jump control rule found.");
        return true;
    }
    HILOG_INFO("Get appJumpControlRule, jumpMode:%d.", controlRule.jumpMode);
    return controlRule.jumpMode != AppExecFwk::AbilityJumpMode::DIRECT;
}

bool AbilityJumpInterceptor::CheckIfJumpExempt(std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    if (CheckIfExemptByBundleName(bundleMgrHelper, controlRule.callerPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_CALLER, userId)) {
        HILOG_INFO("Jump from exempt caller app, No need to intercept.");
        return true;
    }
    if (CheckIfExemptByBundleName(bundleMgrHelper, controlRule.targetPkg,
        PermissionConstants::PERMISSION_EXEMPT_AS_TARGET, userId)) {
        HILOG_INFO("Jump to exempt target app, No need to intercept.");
        return true;
    }
    HILOG_INFO("Third-party apps jump to third-party apps.");
    return false;
}

bool AbilityJumpInterceptor::CheckIfExemptByBundleName(std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper,
    const std::string &bundleName, const std::string &permission, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo appInfo;
    if (!IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(bundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT,
        userId, appInfo))) {
        HILOG_ERROR("VerifyPermission failed to get application info.");
        return false;
    }
    if (appInfo.isSystemApp) {
        HILOG_INFO("Bundle:%{public}s is system app.", bundleName.c_str());
        return true;
    }
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(appInfo.accessTokenId, permission);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        HILOG_DEBUG("VerifyPermission %{public}d: PERMISSION_DENIED.", appInfo.accessTokenId);
        return false;
    }
    HILOG_INFO("Bundle:%{public}s verify permission:%{public}s successed.", bundleName.c_str(), permission.c_str());
    return true;
}

bool AbilityJumpInterceptor::LoadAppLabelInfo(std::shared_ptr<AppExecFwk::BundleMgrHelper> &bundleMgrHelper, Want &want,
    AppExecFwk::AppJumpControlRule &controlRule, int32_t userId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppExecFwk::ApplicationInfo callerAppInfo;
    IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(controlRule.callerPkg,
        AppExecFwk::ApplicationFlag::GET_BASIC_APPLICATION_INFO, userId, callerAppInfo));
    AppExecFwk::ApplicationInfo targetAppInfo;
    IN_PROCESS_CALL(bundleMgrHelper->GetApplicationInfo(controlRule.targetPkg,
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
