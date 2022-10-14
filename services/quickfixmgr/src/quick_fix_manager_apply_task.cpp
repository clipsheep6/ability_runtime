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

#include "quick_fix_manager_apply_task.h"

#include "application_state_observer_stub.h"
#include "common_event_data.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "quick_fix_error_utils.h"
#include "quick_fix_manager_service.h"
#include "quick_fix/quick_fix_status_callback_host.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace {
// same with quick_fix_result_info
constexpr const char *QUICK_FIX_BUNDLE_NAME = "bundleName";
constexpr const char *QUICK_FIX_BUNDLE_VERSION_CODE = "bundleVersionCode";
constexpr const char *QUICK_FIX_PATCH_VERSION_CODE = "patchVersionCode";
constexpr const char *QUICK_FIX_IS_SO_CONTAINED = "isSoContained";
constexpr const char *QUICK_FIX_TYPE = "type";
constexpr const char *QUICK_FIX_MODULE_NAME = "moduleNames";

// common event key
constexpr const char *APPLY_RESULT = "applyResult";
constexpr const char *APPLY_RESULT_INFO = "applyResultInfo";
constexpr const char *BUNDLE_NAME = "bundleName";
constexpr const char *BUNDLE_VERSION = "bundleVersion";
constexpr const char *PATCH_VERSION = "patchVersion";

// timeout task
constexpr const char *TIMEOUT_TASK_NAME = "timeoutTask";
constexpr int64_t TIMEOUT_TASK_DELAY_TIME = 5000;
} // namespace

class QuickFixManagerStatusCallback : public AppExecFwk::QuickFixStatusCallbackHost {
public:
    explicit QuickFixManagerStatusCallback(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : applyTask_(applyTask)
    {}

    virtual ~QuickFixManagerStatusCallback() = default;

    void OnPatchDeployed(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        HILOG_DEBUG("function called.");
        if (applyTask_ == nullptr) {
            HILOG_ERROR("Apply task is nullptr, result is %{public}s.", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                HILOG_ERROR("Deploy quick fix failed, result is %{public}s.", result->ToString().c_str());
                ret = QUICK_FIX_DEPLOY_FAILED;
                break;
            }

            if (!applyTask_->SetQuickFixInfo(result)) {
                HILOG_ERROR("Set quick fix info failed");
                ret = QUICK_FIX_SET_INFO_FAILED;
                break;
            }

            applyTask_->HandlePatchDeployed();
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

    void OnPatchSwitched(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        HILOG_DEBUG("function called.");
        if (applyTask_ == nullptr) {
            HILOG_ERROR("Apply task is nullptr, result is %{public}s.", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                HILOG_ERROR("Switch quick fix failed, result is %{public}s.", result->ToString().c_str());
                ret = QUICK_FIX_SWICH_FAILED;
                break;
            }

            applyTask_->HandlePatchSwitched();
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

    void OnPatchDeleted(const std::shared_ptr<AppExecFwk::QuickFixResult> &result) override
    {
        HILOG_DEBUG("function called.");
        if (applyTask_ == nullptr) {
            HILOG_ERROR("Apply task is nullptr, result is %{public}s.", result->ToString().c_str());
            return;
        }

        int32_t ret = QUICK_FIX_OK;
        do {
            if (result->GetResCode() != 0) {
                HILOG_ERROR("Delete quick fix failed, result is %{public}s.", result->ToString().c_str());
                ret = QUICK_FIX_DELETE_FAILED;
                break;
            }

            applyTask_->HandlePatchDeleted();
        } while (0);

        if (ret != QUICK_FIX_OK) {
            applyTask_->NotifyApplyStatus(ret);
            applyTask_->RemoveSelf();
        }
        applyTask_->RemoveTimeoutTask();
    }

private:
    std::shared_ptr<QuickFixManagerApplyTask> applyTask_;
};

class QuickFixMgrAppStateObserver : public AppExecFwk::ApplicationStateObserverStub {
public:
    explicit QuickFixMgrAppStateObserver(std::shared_ptr<QuickFixManagerApplyTask> applyTask)
        : applyTask_(applyTask)
    {}

    virtual ~QuickFixMgrAppStateObserver() = default;

    void OnProcessDied(const AppExecFwk::ProcessData &processData) override
    {
        HILOG_INFO("process died, bundle name is %{public}s.", processData.bundleName.c_str());

        if (applyTask_ == nullptr) {
            HILOG_ERROR("Apply task is nullptr, bundle name is %{public}s.", processData.bundleName.c_str());
            return;
        }

        bool isRunning = applyTask_->GetRunningState();
        if (!isRunning) {
            applyTask_->HandlePatchDeployed();
        }
    }

private:
    std::shared_ptr<QuickFixManagerApplyTask> applyTask_;
};

void QuickFixManagerApplyTask::Run(const std::vector<std::string> &quickFixFiles)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_INFO("Run apply task.");
    PostDeployQuickFixTask(quickFixFiles);
}

void QuickFixManagerApplyTask::HandlePatchDeployed()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    if (appMgr_ == nullptr) {
        HILOG_ERROR("Appmgr is nullptr.");
        NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
        RemoveSelf();
        return;
    }

    isRunning_ = GetRunningState();
    if (isRunning_ && isSoContained_) {
        HILOG_INFO("Start to register application state observer.");
        std::vector<std::string> bundleNameList;
        bundleNameList.push_back(bundleName_);
        sptr<AppExecFwk::IApplicationStateObserver> callback = new QuickFixMgrAppStateObserver(shared_from_this());
        auto ret = appMgr_->RegisterApplicationStateObserver(callback, bundleNameList);
        if (ret != 0) {
            HILOG_ERROR("Register application state observer failed.");
            NotifyApplyStatus(QUICK_FIX_REGISTER_OBSERVER_FAILED);
            RemoveSelf();
        }
        HILOG_DEBUG("Register application state observer succeed.");
        return;
    } else if (isRunning_ && !isSoContained_) {
        ApplicationQuickFixInfo quickFixInfo;
        auto service = quickFixMgrService_.promote();
        if (service == nullptr) {
            HILOG_ERROR("Quick fix service is nullptr.");
            NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
            RemoveSelf();
            return;
        }

        auto ret = service->GetApplyedQuickFixInfo(bundleName_, quickFixInfo);
        if (ret == QUICK_FIX_OK && !quickFixInfo.appqfInfo.hqfInfos.empty()) {
            // if there exist old version hqfInfo, need to unload.
            HILOG_DEBUG("Need unload patch firstly.");
            ret = appMgr_->NotifyUnLoadRepairPatch(bundleName_);
            if (ret != 0) {
                HILOG_ERROR("Notify app unload patch failed.");
                NotifyApplyStatus(QUICK_FIX_NOTIFY_UNLOAD_PATCH_FAILED);
                RemoveSelf();
                return;
            }
        }
    }

    PostSwitchQuickFixTask();
}

void QuickFixManagerApplyTask::HandlePatchSwitched()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    if (isRunning_ && !isSoContained_) {
        if (appMgr_ == nullptr) {
            HILOG_ERROR("Appmgr is nullptr.");
            NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
            RemoveSelf();
            return;
        }

        auto ret = appMgr_->NotifyLoadRepairPatch(bundleName_);
        if (ret != 0) {
            HILOG_ERROR("Notify app load patch failed.");
            NotifyApplyStatus(QUICK_FIX_NOTIFY_LOAD_PATCH_FAILED);
            RemoveSelf();
            return;
        }
    }

    PostDeleteQuickFixTask();
}

void QuickFixManagerApplyTask::HandlePatchDeleted()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    if (isRunning_ && !isSoContained_ && type_ == AppExecFwk::QuickFixType::HOT_RELOAD) {
        if (appMgr_ == nullptr) {
            HILOG_ERROR("Appmgr is nullptr.");
            NotifyApplyStatus(QUICK_FIX_APPMGR_INVALID);
            RemoveSelf();
            return;
        }

        auto ret = appMgr_->NotifyHotReloadPage(bundleName_);
        if (ret != 0) {
            HILOG_ERROR("Notify app reload page failed.");
            NotifyApplyStatus(QUICK_FIX_NOTIFY_RELOAD_PAGE_FAILED);
            RemoveSelf();
            return;
        }
    }

    NotifyApplyStatus(QUICK_FIX_OK);
    RemoveSelf();
}

void QuickFixManagerApplyTask::PostDeployQuickFixTask(const std::vector<std::string> &quickFixFiles)
{
    sptr<AppExecFwk::IQuickFixStatusCallback> callback = new QuickFixManagerStatusCallback(shared_from_this());
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto deployTask = [thisWeakPtr, quickFixFiles, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            HILOG_ERROR("PostDeployQuickFixTask, Apply task is nullptr.");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            HILOG_ERROR("PostDeployQuickFixTask, Bundle quick fix manager is nullptr.");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->bundleQfMgr_->DeployQuickFix(quickFixFiles, callback);
        if (ret != 0) {
            HILOG_ERROR("PostDeployQuickFixTask, Deploy quick fix failed with %{public}d.", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_DEPLOY_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(deployTask)) {
        HILOG_ERROR("Post deploy task failed.");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostSwitchQuickFixTask()
{
    sptr<AppExecFwk::IQuickFixStatusCallback> callback = new QuickFixManagerStatusCallback(shared_from_this());
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto switchTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            HILOG_ERROR("PostSwitchQuickFixTask, Apply task is nullptr.");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            HILOG_ERROR("PostSwitchQuickFixTask, Bundle quick fix manager is nullptr.");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->bundleQfMgr_->SwitchQuickFix(applyTask->bundleName_, true, callback);
        if (ret != 0) {
            HILOG_ERROR("PostSwitchQuickFixTask, Switch quick fix failed with %{public}d.", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_SWICH_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(switchTask)) {
        HILOG_ERROR("Post switch task failed.");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostDeleteQuickFixTask()
{
    auto callback = new (std::nothrow) QuickFixManagerStatusCallback(shared_from_this());
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto deleteTask = [thisWeakPtr, callback]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            HILOG_ERROR("PostDeleteQuickFixTask, Apply task is nullptr.");
            return;
        }

        if (applyTask->bundleQfMgr_ == nullptr) {
            HILOG_ERROR("PostDeleteQuickFixTask, Bundle quick fix manager is nullptr.");
            applyTask->NotifyApplyStatus(QUICK_FIX_BUNDLEMGR_INVALID);
            applyTask->RemoveSelf();
            return;
        }

        auto ret = applyTask->bundleQfMgr_->DeleteQuickFix(applyTask->bundleName_, callback);
        if (ret != 0) {
            HILOG_ERROR("PostDeleteQuickFixTask, Delete quick fix failed with %{public}d.", ret);
            applyTask->NotifyApplyStatus(QUICK_FIX_DELETE_FAILED);
            applyTask->RemoveSelf();
            return;
        }
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(deleteTask)) {
        HILOG_ERROR("Post delete task failed.");
    }
    PostTimeOutTask();
}

void QuickFixManagerApplyTask::PostTimeOutTask()
{
    std::weak_ptr<QuickFixManagerApplyTask> thisWeakPtr(weak_from_this());
    auto timeoutTask = [thisWeakPtr]() {
        auto applyTask = thisWeakPtr.lock();
        if (applyTask == nullptr) {
            HILOG_ERROR("Apply task is nullptr.");
            return;
        }

        applyTask->NotifyApplyStatus(QUICK_FIX_PROCESS_TIMEOUT);
        applyTask->RemoveSelf();
    };
    if (eventHandler_ == nullptr || !eventHandler_->PostTask(timeoutTask, TIMEOUT_TASK_NAME, TIMEOUT_TASK_DELAY_TIME)) {
        HILOG_ERROR("Post delete task failed.");
    }
}

void QuickFixManagerApplyTask::RemoveTimeoutTask()
{
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("event handler is nullptr.");
        return;
    }
    eventHandler_->RemoveTask(TIMEOUT_TASK_NAME);
}

bool QuickFixManagerApplyTask::SetQuickFixInfo(const std::shared_ptr<AppExecFwk::QuickFixResult> &result)
{
    auto resultJson = nlohmann::json::parse(result->ToString());
    const auto &jsonObjectEnd = resultJson.end();
    if ((resultJson.find(QUICK_FIX_BUNDLE_NAME) == jsonObjectEnd)
        || (resultJson.find(QUICK_FIX_BUNDLE_VERSION_CODE) == jsonObjectEnd)
        || (resultJson.find(QUICK_FIX_PATCH_VERSION_CODE) == jsonObjectEnd)
        || (resultJson.find(QUICK_FIX_IS_SO_CONTAINED) == jsonObjectEnd)
        || (resultJson.find(QUICK_FIX_TYPE) == jsonObjectEnd)
        || (resultJson.find(QUICK_FIX_MODULE_NAME) == jsonObjectEnd)) {
        HILOG_ERROR("Incomplete result.");
        return false;
    }

    bundleName_ = resultJson.at(QUICK_FIX_BUNDLE_NAME).get<std::string>();
    bundleVersionCode_ = resultJson.at(QUICK_FIX_BUNDLE_VERSION_CODE).get<int>();
    patchVersionCode_ = resultJson.at(QUICK_FIX_PATCH_VERSION_CODE).get<int>();
    isSoContained_ = resultJson.at(QUICK_FIX_IS_SO_CONTAINED).get<bool>();
    type_ = static_cast<AppExecFwk::QuickFixType>(resultJson.at(QUICK_FIX_TYPE).get<int32_t>());
    if (type_ != AppExecFwk::QuickFixType::PATCH && type_ != AppExecFwk::QuickFixType::HOT_RELOAD) {
        HILOG_ERROR("Quick fix type is invalid.");
        return false;
    }
    moduleNames_ = resultJson.at(QUICK_FIX_MODULE_NAME).get<std::vector<std::string>>();

    HILOG_INFO("bundleName: %{public}s, bundleVersion: %{public}d, patchVersion: %{public}d, soContained: %{public}d, "
               "type: %{public}d.", bundleName_.c_str(), bundleVersionCode_, patchVersionCode_, isSoContained_,
               static_cast<int32_t>(type_));
    return true;
}

bool QuickFixManagerApplyTask::GetRunningState()
{
    if (appMgr_ == nullptr) {
        HILOG_ERROR("App manager is nullptr.");
        return false;
    }

    auto ret = appMgr_->GetAppRunningStateByBundleName(bundleName_);
    HILOG_INFO("Process running state of [%{public}s] is %{public}d.", bundleName_.c_str(), ret);
    return ret;
}

void QuickFixManagerApplyTask::NotifyApplyStatus(int32_t applyResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("function called.");

    Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_QUICK_FIX_APPLY_RESULT);
    want.SetParam(APPLY_RESULT, QuickFixErrorUtil::GetErrorCode(applyResult));
    want.SetParam(APPLY_RESULT_INFO, QuickFixErrorUtil::GetErrorMessage(applyResult));
    want.SetParam(BUNDLE_NAME, bundleName_);
    want.SetParam(BUNDLE_VERSION, bundleVersionCode_);
    want.SetParam(PATCH_VERSION, patchVersionCode_);

    std::string moduleName = std::accumulate(moduleNames_.begin(), moduleNames_.end(), std::string(""),
        [moduleName = moduleNames_](const std::string &name, const std::string &str) {
            return (str == moduleName.front()) ? (name + str) : (name + "," + str);
        });
    want.SetModuleName(moduleName);

    EventFwk::CommonEventData commonData {want};
    EventFwk::CommonEventManager::PublishCommonEvent(commonData);
}

void QuickFixManagerApplyTask::RemoveSelf()
{
    auto service = quickFixMgrService_.promote();
    if (service) {
        service->RemoveApplyTask(shared_from_this());
    }
}
} // namespace AAFwk
} // namespace OHOS