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

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

namespace {
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
}
namespace OHOS {
namespace AbilityRuntime {
std::map<std::string, std::weak_ptr<StartupTask>> AsyncTaskCallBack::jsStartupTaskObjects_;
JsStartupTask::JsStartupTask(const std::string &name, JsRuntime &jsRuntime,
    std::unique_ptr<NativeReference> &startupJsRef, std::shared_ptr<NativeReference> &contextJsRef)
    : StartupTask(name), jsRuntime_(jsRuntime), startupJsRef_(std::move(startupJsRef)), contextJsRef_(contextJsRef) {}

JsStartupTask::~JsStartupTask() = default;

int32_t JsStartupTask::Init()
{
    // init dependencies_, callCreateOnMainThread_, waitOnMainThread_, isExcludeFromAutoStart_
    TAG_LOGD(AAFwkTag::STARTUP, "%{public}s, dump: %{public}d%{public}d%{public}d, dep: %{public}s", name_.c_str(),
        callCreateOnMainThread_, waitOnMainThread_, isExcludeFromAutoStart_, DumpDependencies().c_str());
    return ERR_OK;
}

int32_t JsStartupTask::RunTaskInit(std::unique_ptr<StartupTaskResultCallback> callback)
{
    if (state_ != State::CREATED) {
        TAG_LOGE(AAFwkTag::STARTUP,
            "%{public}s, state is wrong %{public}d", name_.c_str(), static_cast<int32_t>(state_));
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    state_ = State::INITIALIZING;
    callback->Push([weak = weak_from_this()](const std::shared_ptr<StartupTaskResult> &result) {
        auto startupTask = weak.lock();
        if (startupTask == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "startupTask is nullptr.");
            return;
        }
        startupTask->SaveResult(result);
        startupTask->CallExtraCallback(result);
    });

    if (callCreateOnMainThread_) {
        return JsStartupTaskExecutor::RunOnMainThread(jsRuntime_, startupJsRef_, contextJsRef_, std::move(callback));
    }

    if (LoadJsAsyncTaskExcutor() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "Load async task excutor is failed.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    LoadJsAsyncTaskCallback();

    startupTaskResultCallback_ = std::move(callback);

    auto startupName = GetName();
    auto result = JsStartupTaskExecutor::RunOnTaskPool(
        jsRuntime_, startupJsRef_, contextJsRef_, AsyncTaskExcutorJsRef_, AsyncTaskExcutorCallbackJsRef_, startupName);
    if (result == ERR_OK) {
        AsyncTaskCallBack::jsStartupTaskObjects_.emplace(startupName, shared_from_this());
    }
    return result;
}

int32_t JsStartupTask::LoadJsAsyncTaskExcutor()
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object is nullptr.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    AsyncTaskExcutorJsRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskExcutor", &object, 1);
    return ERR_OK;
}

void JsStartupTask::LoadJsAsyncTaskCallback()
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value config;
    std::string value = "This is callback value";
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, value.c_str(), value.length(), &config));

    napi_property_descriptor props[] = {
        DECLARE_NAPI_STATIC_FUNCTION("onAsyncTaskCompleted", AsyncTaskCallBack::AsyncTaskCompleted),
        DECLARE_NAPI_INSTANCE_PROPERTY("config", config),
    };
    napi_value asyncTaskCallbackClass = nullptr;
    napi_define_sendable_class(env, "AsyncTaskCallback", NAPI_AUTO_LENGTH, AsyncTaskCallBack::Constructor,
        nullptr, sizeof(props) / sizeof(props[0]), props, nullptr, &asyncTaskCallbackClass);
    AsyncTaskExcutorCallbackJsRef_ =
        JsRuntime::LoadSystemModuleByEngine(env, "app.appstartup.AsyncTaskCallback", &asyncTaskCallbackClass, 1);
}

void JsStartupTask::OnAsyncTaskCompleted(const std::shared_ptr<StartupTaskResult>  &result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    if (startupTaskResultCallback_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Startup task result callback object is nullptr.");
        return;
    }
    startupTaskResultCallback_->Call(result);
}

int32_t JsStartupTask::RunTaskOnDependencyCompleted(const std::string &dependencyName,
    const std::shared_ptr<StartupTaskResult> &result)
{
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    if (startupJsRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, startup task is null", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupValue = startupJsRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, startup task is not napi object", name_.c_str());
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupOnDepCompleted = nullptr;
    napi_get_named_property(env, startupValue, "onDependencyCompleted", &startupOnDepCompleted);
    if (startupOnDepCompleted == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP,
            "%{public}s, failed to get property onDependencyCompleted from startup task.", name_.c_str());
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isCallable = false;
    napi_is_callable(env, startupOnDepCompleted, &isCallable);
    if (!isCallable) {
        TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, startup task onDependencyCompleted is not callable.", name_.c_str());
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
            TAG_LOGE(AAFwkTag::STARTUP, "%{public}s, failed to convert to js result.", dependencyName.c_str());
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
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    size_t argc = ARGC_TWO;
    napi_value argv[ARGC_TWO] = { nullptr };
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    std::string startupName;
    if (!ConvertFromJsValue(env, argv[INDEX_ZERO], startupName)) {
        TAG_LOGE(AAFwkTag::STARTUP, "Convert from js startupName error.");
        return CreateJsUndefined(env);
    }

    napi_value resultJs = argv[INDEX_ONE];
    napi_ref resultRef = nullptr;
    napi_create_reference(env, resultJs, INDEX_ONE, &resultRef);
    std::shared_ptr<NativeReference> result(reinterpret_cast<NativeReference*>(resultRef));
    std::shared_ptr<StartupTaskResult> callbackResult = std::make_shared<JsStartupTaskResult>(result);

    std::shared_ptr<StartupTask> startupTask;
    for (auto iter : AsyncTaskCallBack::jsStartupTaskObjects_) {
        if (iter.first == startupName) {
            startupTask = iter.second.lock();
        }
    }

    if (startupTask != nullptr) {
        startupTask->OnAsyncTaskCompleted(callbackResult);
        AsyncTaskCallBack::jsStartupTaskObjects_.erase(startupName);
    }

    return CreateJsUndefined(env);
}

napi_value AsyncTaskCallBack::Constructor(napi_env env, napi_callback_info cbinfo)
{
    TAG_LOGD(AAFwkTag::STARTUP, "Called.");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
