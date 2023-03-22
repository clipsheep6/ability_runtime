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

#include "js_error_observer.h"

#include <cstdint>

#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t ARGC_ONE = 1;
JsErrorObserver::JsErrorObserver(NativeEngine &engine) : engine_(engine) {}

JsErrorObserver::~JsErrorObserver() = default;

void JsErrorObserver::OnUnhandledException(const std::string errMsg)
{
    HILOG_DEBUG("OnUnhandledException come.");
    std::weak_ptr<JsErrorObserver> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([thisWeakPtr, errMsg](NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsErrorObserver> jsObserver = thisWeakPtr.lock();
            if (jsObserver) {
                jsObserver->HandleOnUnhandledException(errMsg);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsErrorObserver::OnUnhandledException",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsErrorObserver::HandleOnUnhandledException(const std::string &errMsg)
{
    HILOG_DEBUG("HandleOnUnhandledException come.");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* value = (item.second)->Get();
        NativeValue* argv[] = { CreateJsValue(engine_, errMsg) };
        CallJsFunction(value, "onUnhandledException", argv, ARGC_ONE);
    }
}

void JsErrorObserver::CallJsFunction(NativeValue* value, const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin, method:%{public}s", methodName);
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get method");
        return;
    }
    engine_.CallFunction(value, method, argv, argc);
}

void JsErrorObserver::AddJsObserverObject(const int32_t observerId, NativeValue* jsObserverObject)
{
    jsObserverObjectMap_.emplace(
        observerId, std::shared_ptr<NativeReference>(engine_.CreateReference(jsObserverObject, 1)));
}

bool JsErrorObserver::RemoveJsObserverObject(const int32_t observerId, bool &isEmpty)
{
    bool result = (jsObserverObjectMap_.erase(observerId) == 1);
    isEmpty = jsObserverObjectMap_.empty();
    return result;
}

void JsErrorObserver::OnExceptionObject(const AppExecFwk::ErrorObject &errorObj)
{
    HILOG_DEBUG("OnExceptionObject come.");
    std::weak_ptr<JsErrorObserver> thisWeakPtr(shared_from_this());
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([thisWeakPtr, errorObj](NativeEngine &engine, AsyncTask &task, int32_t status) {
            std::shared_ptr<JsErrorObserver> jsObserver = thisWeakPtr.lock();
            if (jsObserver) {
                jsObserver->HandleException(errorObj);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule("JsErrorObserver::OnExceptionObject",
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsErrorObserver::HandleException(const AppExecFwk::ErrorObject &errorObj)
{
    HILOG_DEBUG("HandleException come.");
    auto tmpMap = jsObserverObjectMap_;
    for (auto &item : tmpMap) {
        NativeValue* jsObj = (item.second)->Get();
        NativeValue* jsValue[] = { CreateJsErrorObject(engine_, errorObj) };
        CallJsFunction(jsObj, "onException", jsValue, ARGC_ONE);
    }
}

NativeValue* JsErrorObserver::CreateJsErrorObject(NativeEngine &engine, const AppExecFwk::ErrorObject &errorObj)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_WARN("invalid object.");
        return objValue;
    }

    object->SetProperty("name", CreateJsValue(engine, errorObj.name));
    object->SetProperty("message", CreateJsValue(engine, errorObj.message));
    if (!errorObj.stack.empty()) {
        object->SetProperty("stack", CreateJsValue(engine, errorObj.stack));
    }

    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
