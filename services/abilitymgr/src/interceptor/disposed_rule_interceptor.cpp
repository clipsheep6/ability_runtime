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
#include "ability_util.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "in_process_call_wrapper.h"
#include "iservice_registry.h"
#include "modal_system_ui_extension.h"
#include "task_utils_wrap.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string UNREGISTER_EVENT_TASK = "unregister event task";
const std::string UNREGISTER_TIMEOUT_OBSERVER_TASK = "unregister timeout observer task";
constexpr int UNREGISTER_OBSERVER_MICRO_SECONDS = 5000;
}
#define RETURN_BY_ISEDM(object)                 \
    if (object) {                               \
        return ERR_EDM_APP_CONTROLLED;          \
    }                                           \
    return ERR_APP_CONTROLLED;

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
} // namespace AAFwk
} // namespace OHOS