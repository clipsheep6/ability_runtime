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

#include "js_startup_task_executor.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "js_startup_task_result.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
int32_t JsStartupTaskExecutor::RunOnMainThread(JsRuntime &jsRuntime,
    const std::unique_ptr<NativeReference> &startup, const std::shared_ptr<NativeReference> &context,
    std::unique_ptr<StartupTaskResultCallback> callback)
{
    HILOG_DEBUG("RunOnMainThread start.");
    HandleScope handleScope(jsRuntime);
    auto env = jsRuntime.GetNapiEnv();

    napi_value returnVal = nullptr;
    int32_t resultCode = CallStartupInit(env, startup, context, callback, returnVal);
    if (resultCode != ERR_OK) {
        return resultCode;
    }
    return HandleReturnVal(env, returnVal, callback);
}

int32_t JsStartupTaskExecutor::RunOnTaskPool(JsRuntime &jsRuntime,
    const std::unique_ptr<NativeReference> &startup, const std::shared_ptr<NativeReference> &context,
    const std::unique_ptr<NativeReference> asyncTaskExcutor)
{
    HILOG_DEBUG("RunOnTaskPool start.");
    HandleScope handleScope(jsRuntime);
    auto env = jsRuntime.GetNapiEnv();

    if (startup == nullptr || context == nullptr) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
        //     "startup task is null or context is null.");
        HILOG_ERROR("xxxxxxxxxxx 0000000000000000");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupValue = startup->GetNapiValue();
    if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
        //     "startup task is not napi object.");
        HILOG_ERROR("xxxxxxxxxxx 11111111111111111");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupInit = nullptr;
    napi_get_named_property(env, startupValue, "init", &startupInit);
    if (startupInit == nullptr) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
        //     "failed to get property init from startup task.");
        HILOG_ERROR("xxxxxxxxxxx 2222222222222222");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isCallableInit = false;
    napi_is_callable(env, startupInit, &isCallableInit);
    if (!isCallableInit) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
        //     "startup task init is not callable.");
        HILOG_ERROR("xxxxxxxxxxx 33333333333333333");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    else {
        HILOG_ERROR("xxxxxxxxxxx 444444444444444444");
    }


    if (asyncTaskExcutor == nullptr || context == nullptr || startup == nullptr) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
        //     "startup task is null or context is null.");
        HILOG_ERROR("RunOnTaskPool111111111111");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value excutorValue = asyncTaskExcutor->GetNapiValue();
    if (!CheckTypeForNapiValue(env, excutorValue, napi_object)) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
        //     "startup task is not napi object.");
        HILOG_ERROR("RunOnTaskPool2222222222221");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value pushTask = nullptr;
    napi_get_named_property(env, excutorValue, "AsyncPushTask", &pushTask);
    if (pushTask == nullptr) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
        //     "failed to get property AnsycPushTask from startup task.");
        HILOG_ERROR("RunOnTaskPool23333333333331");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isCallable = false;
    napi_is_callable(env, pushTask, &isCallable);
    if (!isCallable) {
        // ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
        //     "startup task PushTask is not callable.");
        HILOG_ERROR("RunOnTaskPool44444444444444");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isSendable = false;
    napi_is_sendable_object(env, pushTask, &isSendable);
    if (isSendable) {
        HILOG_ERROR("pushTask is sendable");
    } else {
        HILOG_ERROR("pushTask is not sendable");
    }

    napi_value argv[1] = { startupValue };
    napi_value returnVal = nullptr;
    napi_call_function(env, excutorValue, pushTask, 1, argv, &returnVal);
    return ERR_OK;
}

int32_t JsStartupTaskExecutor::CallStartupInit(napi_env env, const std::unique_ptr<NativeReference> &startup,
    const std::shared_ptr<NativeReference> &context, std::unique_ptr<StartupTaskResultCallback> &callback,
    napi_value &returnVal)
{
    if (startup == nullptr || context == nullptr) {
        ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
            "startup task is null or context is null.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupValue = startup->GetNapiValue();
    if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
        ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
            "startup task is not napi object.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    napi_value startupInit = nullptr;
    napi_get_named_property(env, startupValue, "init", &startupInit);
    if (startupInit == nullptr) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "failed to get property init from startup task.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    bool isCallable = false;
    napi_is_callable(env, startupInit, &isCallable);
    if (!isCallable) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "startup task init is not callable.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }
    napi_value argv[1] = { context->GetNapiValue() };
    napi_call_function(env, startupValue, startupInit, 1, argv, &returnVal);
    return ERR_OK;
}

// int32_t JsStartupTaskExecutor::CallStartupPushTask(napi_env env, const std::unique_ptr<NativeReference> &startup,
//     const std::shared_ptr<NativeReference> &context, std::unique_ptr<StartupTaskResultCallback> &callback,
//     napi_value &returnVal, const std::unique_ptr<NativeReference> &asynctaskexcutor)
// {
//     if (asynctaskexcutor == nullptr || context == nullptr) {
//         ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
//             "startup task is null or context is null.");
//         return ERR_STARTUP_INTERNAL_ERROR;
//     }
//     napi_value startupValue = asynctaskexcutor->GetNapiValue();
//     if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
//         ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
//             "startup task is not napi object.");
//         return ERR_STARTUP_INTERNAL_ERROR;
//     }
//     napi_value startupPush = nullptr;
//     napi_get_named_property(env, startupValue, "AsyncPushTask", &startupPush);
//     if (startupPush == nullptr) {
//         ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
//             "failed to get property AnsycPushTask from startup task.");
//         return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
//     }
//     bool isCallable = false;
//     napi_is_callable(env, startupPush, &isCallable);
//     if (!isCallable) {
//         ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
//             "startup task PushTask is not callable.");
//         return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
//     }
//     // napi_value argv[1] = { startupValue };
//     napi_call_function(env, startupValue, startupPush, 0, nullptr, &returnVal);
//     return ERR_OK;
// }

int32_t JsStartupTaskExecutor::HandleReturnVal(napi_env env, napi_value returnVal,
    std::unique_ptr<StartupTaskResultCallback> &callback)
{
    bool isPromise = false;
    napi_is_promise(env, returnVal, &isPromise);
    if (!isPromise) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "the return value of startup task init is not promise.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    auto *callbackPtr = callback.release();
    napi_value promiseThen = nullptr;
    napi_get_named_property(env, returnVal, "then", &promiseThen);
    napi_value promiseResolveCallback = nullptr;
    napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        ResolveResultCallback, callbackPtr, &promiseResolveCallback);
    napi_value argvThen[1] = { promiseResolveCallback };
    napi_call_function(env, returnVal, promiseThen, 1, argvThen, nullptr);

    napi_value promiseCatch = nullptr;
    napi_get_named_property(env, returnVal, "catch", &promiseCatch);
    napi_value promiseRejectCallback = nullptr;
    napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        RejectResultCallback, callbackPtr, &promiseRejectCallback);
    napi_value argvCatch[1] = { promiseRejectCallback };
    napi_call_function(env, returnVal, promiseCatch, 1, argvCatch, nullptr);
    return ERR_OK;
}

napi_value JsStartupTaskExecutor::ResolveResultCallback(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("enter");
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    void *data = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    auto *callback = static_cast<StartupTaskResultCallback *>(data);
    napi_value resultJs = argv[0];
    napi_ref resultRef = nullptr;
    napi_create_reference(env, resultJs, 1, &resultRef);
    std::shared_ptr<NativeReference> result(reinterpret_cast<NativeReference*>(resultRef));
    ReplySucceeded(callback, result);
    return nullptr;
}

napi_value JsStartupTaskExecutor::RejectResultCallback(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("enter");
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
    auto *callback = static_cast<StartupTaskResultCallback *>(data);
    ReplyFailed(callback, ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
        "the promise of startup task init is reject.");
    return nullptr;
}

void JsStartupTaskExecutor::ReplyFailed(StartupTaskResultCallback *callback,
    int32_t resultCode, const std::string &resultMessage)
{
    HILOG_DEBUG("enter");
    if (callback == nullptr) {
        return;
    }
    std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultCode, resultMessage);
    callback->Call(result);
    delete callback;
}

void JsStartupTaskExecutor::ReplyFailed(std::unique_ptr<StartupTaskResultCallback> callback,
    int32_t resultCode, const std::string &resultMessage)
{
    HILOG_ERROR("Failed to execute: %{public}s", resultMessage.c_str());
    if (callback == nullptr) {
        return;
    }
    std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultCode, resultMessage);
    callback->Call(result);
}

void JsStartupTaskExecutor::ReplySucceeded(StartupTaskResultCallback *callback,
    const std::shared_ptr<NativeReference> &resultRef)
{
    HILOG_DEBUG("enter");
    if (callback == nullptr) {
        return;
    }
    std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultRef);
    callback->Call(result);
    delete callback;
}
} // namespace AbilityRuntime
} // namespace OHOS
