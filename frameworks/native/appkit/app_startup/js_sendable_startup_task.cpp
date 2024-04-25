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

#include "js_sendable_startup_task.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
JsSendableStartupTask::JsSendableStartupTask(const std::string &name, JsRuntime &jsRuntime,
    std::unique_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef)
    : StartupTask(name), jsRuntime_(jsRuntime), startupJsRef_(std::move(startupJsRef)), contextJsRef_(contextJsRef)
{
}

JsSendableStartupTask::~JsSendableStartupTask() = default;

int32_t JsSendableStartupTask::Init()
{
    HILOG_DEBUG("%{public}s, dump: %{public}d%{public}d%{public}d, dep: %{public}s", name_.c_str(),
        callCreateOnMainThread_, waitOnMainThread_, isExcludeFromAutoStart_, DumpDependencies().c_str());

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (contextJsRef_ == nullptr) {
        HILOG_ERROR("%{public}s, context is null", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value object = contextJsRef_->GetNapiValue();
    BindNativeProperty(env, object, "HandleInitComplete", JsSendableStartupTask::HandleInitComplete);
    return ERR_OK;
}

int32_t JsSendableStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    // startupTaskResultCallback_ = std::move(callback);
    if (state_ != State::CREATED) {
        HILOG_ERROR("%{public}s, state is wrong %{public}d", name_.c_str(), static_cast<int32_t>(state_));
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    state_ = State::INITIALIZING;
    callback->Push([weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
        auto startupTask = weak.lock();
        if (startupTask == nullptr) {
            HILOG_ERROR("startupTask is nullptr.");
            return;
        }
        startupTask->SaveResult(result);
        startupTask->CallExtraCallback(result);
    });
    HILOG_DEBUG("%{public}s, RunOnTaskPool", name_.c_str());
    if(startupJsRef_ == NULL) {
        HILOG_ERROR("startupJsRef_ is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    asynctaskexcutorJsRef_ = JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskExcutor", &object, 1);
    if (asynctaskexcutorJsRef_ == nullptr) {
        HILOG_ERROR("asynctaskexcutorJsRef_ is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    return JsStartupTaskExecutor::RunOnTaskPool(jsRuntime_, startupJsRef_, contextJsRef_, std::move(asynctaskexcutorJsRef_));
}

int32_t JsSendableStartupTask::RunTaskOnDependencyCompleted(const std::string &dependencyName,
    const std::shared_ptr<StartupTaskResult> &result)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (startupJsRef_ == nullptr) {
        HILOG_ERROR("%{public}s, startup task is null", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupValue = startupJsRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
        HILOG_ERROR("%{public}s, startup task is not napi object", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupOnDepCompleted = nullptr;
    napi_get_named_property(env, startupValue, "onDependencyCompleted", &startupOnDepCompleted);
    if (startupOnDepCompleted == nullptr) {
        HILOG_ERROR("%{public}s, failed to get property onDependencyCompleted from startup task.", name_.c_str());
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isCallable = false;
    napi_is_callable(env, startupOnDepCompleted, &isCallable);
    if (!isCallable) {
        HILOG_ERROR("%{public}s, startup task onDependencyCompleted is not callable.", name_.c_str());
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    napi_value jsResult = GetDependencyResult(env, dependencyName, result);
    napi_value dependency = CreateJsValue(env, dependencyName);
    constexpr size_t argc = 2;
    napi_value argv[argc] = { dependency, jsResult };
    napi_call_function(env, startupValue, startupOnDepCompleted, argc, argv, nullptr);
    return ERR_OK;
}

napi_value JsSendableStartupTask::GetDependencyResult(napi_env env, const std::string &dependencyName,
    const std::shared_ptr<StartupTaskResult> &result)
{
    if (result == nullptr || result->GetResultType() != StartupTaskResult::ResultType::JS) {
        return CreateJsUndefined(env);
    } else {
        std::shared_ptr<JsStartupTaskResult> jsResultPtr = std::static_pointer_cast<JsStartupTaskResult>(result);
        if (jsResultPtr == nullptr) {
            HILOG_ERROR("%{public}s, failed to convert to js result.", dependencyName.c_str());
            return CreateJsUndefined(env);
        }
        std::shared_ptr<NativeReference> jsResultRef = jsResultPtr->GetJsStartupResultRef();
        if (jsResultRef == nullptr) {
            return CreateJsUndefined(env);
        }
        return jsResultRef->GetNapiValue();
    }
}

napi_value JsSendableStartupTask::HandleInitComplete(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsSendableStartupTask, OnHandleInitComplete, nullptr);
}

napi_value JsSendableStartupTask::OnHandleInitComplete(napi_env env, NapiCallbackInfo &info)
{
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
