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

#include "js_startup_task.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsStartupTask::JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
    std::unique_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef)
    : StartupTask(name), jsRuntime_(jsRuntime), startupJsRef_(std::move(startupJsRef)), contextJsRef_(contextJsRef) {}

JsStartupTask::~JsStartupTask() = default;

int32_t JsStartupTask::Init()
{
    // init dependencies_, callCreateOnMainThread_, waitOnMainThread_, isExcludeFromAutoStart_
    HILOG_DEBUG("%{public}s, dump: %{public}d%{public}d%{public}d, dep: %{public}s", name_.c_str(),
        callCreateOnMainThread_, waitOnMainThread_, isExcludeFromAutoStart_, DumpDependencies().c_str());
    return ERR_OK;
}

int32_t JsStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
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

    HILOG_DEBUG("%{public}s, RunTaskInit", name_.c_str());
    if (callCreateOnMainThread_) {
        return JsStartupTaskExecutor::RunOnMainThread(jsRuntime_, startupJsRef_, contextJsRef_, std::move(callback));
    } else {
        if (LoadJsAsyncTaskExcutor() != ERR_OK) {
            HILOG_ERROR("LoadJsAsyncTaskExcutor is failed");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        if (LoadJsAsyncTaskCallback() != ERR_OK) {
            HILOG_ERROR("LoadJsAsyncTaskCallback is failed");
            return ERR_STARTUP_INTERNAL_ERROR;
        }
        return JsStartupTaskExecutor::RunOnTaskPool(jsRuntime_, startupJsRef_, contextJsRef_, AsyncTaskExcutorJsRef_,
            AsyncTaskExcutorCallbackJsRef_);
    }
}

int32_t JsStartupTask::LoadJsAsyncTaskExcutor()
{
    HILOG_INFO("Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Object is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    AsyncTaskExcutorJsRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskExcutor", &object, 1);
    return ERR_OK;
}

int32_t JsStartupTask::LoadJsAsyncTaskCallback()
{
    HILOG_INFO("Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value callbackObject = nullptr;
    napi_create_object(env, &callbackObject);
    if (callbackObject == nullptr) {
        HILOG_ERROR("CallbackObject is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    std::string value = "This is callback value";
    napi_set_named_property(env, callbackObject, "config", CreateJsValue(env, value));

    std::unique_ptr<AsyncTaskCallBack> AsyncCallback = std::make_unique<AsyncTaskCallBack>();
    if (AsyncCallback == nullptr) {
        HILOG_ERROR("Async callback is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_wrap(env, callbackObject, AsyncCallback.get(), AsyncTaskCallBack::Finalizer, nullptr, nullptr);
    const char *moduleName = "AsyncTaskCallback";
    BindNativeFunction(env, callbackObject, "onAsyncTaskCompleted", moduleName, AsyncTaskCallBack::AsyncTaskCompleted);
    AsyncTaskExcutorCallbackJsRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskCallback", &callbackObject, 1);
    return ERR_OK;
}

int32_t JsStartupTask::RunTaskOnDependencyCompleted(const std::string &dependencyName,
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

napi_value JsStartupTask::GetDependencyResult(napi_env env, const std::string &dependencyName,
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

napi_value AsyncTaskCallBack::AsyncTaskCompleted(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Called.");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, AsyncTaskCallBack, onAsyncTaskCompleted, nullptr);
}

napi_value AsyncTaskCallBack::onAsyncTaskCompleted(napi_env env, NapiCallbackInfo &info)
{
    HILOG_INFO("Called.");
    return CreateJsUndefined(env);
}

void AsyncTaskCallBack::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("Called.");
    std::unique_ptr<AsyncTaskCallBack>(static_cast<AsyncTaskCallBack*>(data));
}
} // namespace AbilityRuntime
} // namespace OHOS
