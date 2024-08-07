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

#include "interceptor/disposed_rule_interceptor.h"

#include "ability_record.h"
#include "global_constant.h"
#include "hitrace_meter.h"
#include "iservice_registry.h"
#include "modal_system_ui_extension.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UNREGISTER_EVENT_TASK = "unregister event task";
constexpr const char* UNREGISTER_TIMEOUT_OBSERVER_TASK = "unregister timeout observer task";
constexpr int UNREGISTER_OBSERVER_MICRO_SECONDS = 5000;
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* INTERCEPT_PARAMETERS = "intercept_parammeters";
constexpr const char* INTERCEPT_BUNDLE_NAME = "intercept_bundleName";
constexpr const char* INTERCEPT_ABILITY_NAME = "intercept_abilityName";
constexpr const char* INTERCEPT_MODULE_NAME = "intercept_moduleName";
constexpr const char* IS_FROM_PARENTCONTROL = "ohos.ability.isFromParentControl";
}

ErrCode DisposedRuleInterceptor::DoProcess(AbilityInterceptorParam param)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    AppExecFwk::DisposedRule disposedRule;
    if (CheckControl(param.want, param.userId, disposedRule, param.appIndex)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "The target ability is intercpted, disposedType is %{public}d, controlType is %{public}d, "
            "componentType is %{public}d.", disposedRule.disposedType, disposedRule.controlType,
            disposedRule.componentType);
#ifdef SUPPORT_GRAPHICS
        if (!param.isWithUI || disposedRule.want == nullptr
            || disposedRule.disposedType == AppExecFwk::DisposedType::NON_BLOCK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not start disposed want");
            return AbilityUtil::EdmErrorType(disposedRule.isEdm);
        }
        if (disposedRule.want->GetBundle() == param.want.GetBundle()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not start disposed want with same bundleName");
            return AbilityUtil::EdmErrorType(disposedRule.isEdm);
        }
        SetInterceptInfo(param.want, disposedRule);
        if (disposedRule.componentType == AppExecFwk::ComponentType::UI_ABILITY) {
            int ret = IN_PROCESS_CALL(AbilityManagerClient::GetInstance()->StartAbility(*disposedRule.want,
                param.requestCode, param.userId));
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "DisposedRuleInterceptor start ability failed.");
                return ret;
            }
        }
        if (disposedRule.componentType == AppExecFwk::ComponentType::UI_EXTENSION) {
            int ret = CreateModalUIExtension(*disposedRule.want, param.callerToken);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to start disposed UIExtension");
                return ret;
            }
        }
#endif
        return AbilityUtil::EdmErrorType(disposedRule.isEdm);
    }
    if (disposedRule.disposedType != AppExecFwk::DisposedType::NON_BLOCK) {
        return ERR_OK;
    }
    return StartNonBlockRule(param.want, disposedRule);
}

bool DisposedRuleInterceptor::CheckControl(const Want &want, int32_t userId,
    AppExecFwk::DisposedRule &disposedRule, int32_t appIndex)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    // get bms
    auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The bundleMgrHelper is nullptr.");
        return false;
    }

    // get disposed status
    std::string bundleName = want.GetBundle();
    auto appControlMgr = bundleMgrHelper->GetAppControlProxy();
    if (appControlMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "The appControlMgr is nullptr.");
        return false;
    }
    std::vector<AppExecFwk::DisposedRule> disposedRuleList;
    {
        HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, "GetAbilityRunningControlRule");
        int32_t ret = ERR_OK;
        if (appIndex > 0 && appIndex <= AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX) {
            ret = IN_PROCESS_CALL(appControlMgr->GetAbilityRunningControlRule(bundleName,
                userId, disposedRuleList, appIndex));
        } else {
            ret = IN_PROCESS_CALL(appControlMgr->GetAbilityRunningControlRule(bundleName,
                userId, disposedRuleList, 0));
        }
        if (ret != ERR_OK || disposedRuleList.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Get No DisposedRule");
            return false;
        }
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "not block");
    if (disposedRule.want == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not start disposed app, want is nullptr");
        return ERR_OK;
    }
    if (disposedRule.want->GetBundle() == want.GetBundle()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not start disposed app with same bundleName");
        return ERR_OK;
    }
    SetInterceptInfo(want, disposedRule);
    std::string bundleName = want.GetBundle();
    {
        std::lock_guard<ffrt::mutex> guard(observerLock_);
        if (disposedObserverMap_.find(bundleName) != disposedObserverMap_.end()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "start same disposed app, do not need to register again");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "register to appmanager failed. err:%{public}d", ret);
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
            TAG_LOGE(AAFwkTag::ABILITYMGR, "start disposed app time out, need to unregister observer");
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
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get systemAbilityManager failed");
        return nullptr;
    }
    OHOS::sptr<OHOS::IRemoteObject> object = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    if (!object) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get systemAbilityManager failed");
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    taskHandler_->CancelTask(UNREGISTER_TIMEOUT_OBSERVER_TASK);
    auto unregisterTask = [bundleName, interceptor = shared_from_this()] () {
        std::lock_guard<ffrt::mutex> guard{interceptor->observerLock_};
        auto iter = interceptor->disposedObserverMap_.find(bundleName);
        if (iter == interceptor->disposedObserverMap_.end()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Can not find observer");
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
        (const_cast<Want &>(want)).SetParam(UIEXTENSION_MODAL_TYPE, 1);
        return IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(want)) ? ERR_OK : INNER_ERR;
    } else {
        return abilityRecord->CreateModalUIExtension(want);
    }
}

void DisposedRuleInterceptor::SetInterceptInfo(const Want &want, AppExecFwk::DisposedRule &disposedRule)
{
    if (disposedRule.want == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "disposedWant is nullptr");
        return;
    }
    if (disposedRule.want->GetBoolParam(IS_FROM_PARENTCONTROL, false)) {
        disposedRule.want->SetParam(INTERCEPT_BUNDLE_NAME, want.GetElement().GetBundleName());
        disposedRule.want->SetParam(INTERCEPT_ABILITY_NAME, want.GetElement().GetAbilityName());
        disposedRule.want->SetParam(INTERCEPT_MODULE_NAME, want.GetElement().GetModuleName());
    }
}
} // namespace AAFwk
} // namespace OHOS