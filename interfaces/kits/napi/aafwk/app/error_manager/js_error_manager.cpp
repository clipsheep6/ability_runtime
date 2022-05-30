/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_error_manager.h"

#include <cstdint>

#include "app_data_manager.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "event_runner.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

class JsErrorManager final {
public:
    JsErrorManager() {}
    ~JsErrorManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsErrorManager Finalizer is called");
        std::unique_ptr<JsErrorManager>(static_cast<JsErrorManager*>(data));
    }

    static NativeValue* RegisterErrorObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsErrorManager* me = CheckParamsAndGetThis<JsErrorManager>(engine, info);
        return (me != nullptr) ? me->OnRegisterErrorObserver(*engine, *info) : nullptr;
    }

    static NativeValue* UnregisterErrorObserver(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsErrorManager* me = CheckParamsAndGetThis<JsErrorManager>(engine, info);
        return (me != nullptr) ? me->OnUnregisterErrorObserver(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnRegisterErrorObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_INFO("Register errorObserver is called");
        // only support one
        if (info.argc != ARGC_ONE) {
            HILOG_ERROR("The param is invalid, observers need.");
            return engine.CreateUndefined();
        }

        // unwarp observer
        std::shared_ptr<JsErrorObserver> observer = std::make_shared<JsErrorObserver>(engine);
        observer->SetJsObserverObject(info.argv[0]);
        int64_t observerId = serialNumber_;
        observerIds_.emplace(observerId, observer);
        if (serialNumber_ < INT64_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s create observer", __func__);
        AsyncTask::CompleteCallback complete =
            [observerId, observer](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("RegisterApplicationStateObserver callback begin");
                DelayedSingleton<AppExecFwk::AppDataManager>::GetInstance()->AddErrorObservers(observerId, observer);
            };

        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        return engine.CreateNumber(observerId);
    }

    NativeValue* OnUnregisterErrorObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("unregister errorObserver error, not enough params.");
            return engine.CreateUndefined();
        }

        // unwrap connectId
        int64_t observerId = -1;
        std::shared_ptr<JsErrorObserver> observer = nullptr;
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
        HILOG_INFO("unregister errorObserver called, observer:%{public}d", (int32_t)observerId);
        auto item = observerIds_.find(observerId);
        if (item != observerIds_.end()) {
            // match id
            observer = item->second;
            HILOG_INFO("%{public}s find observer exist", __func__);
        } else {
            HILOG_INFO("%{public}s not find observer exist.", __func__);
        }

        AsyncTask::CompleteCallback complete =
            [observer, observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("unregister errorObserver called.");
                if (observer == nullptr) {
                    HILOG_ERROR("observer is nullptr");
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "observer is nullptr"));
                    return;
                }
                DelayedSingleton<AppExecFwk::AppDataManager>::GetInstance()->RemoveErrorObservers(observerId);
                task.Resolve(engine, engine.CreateUndefined());
                observerIds_.erase(observerId);
                HILOG_INFO("UnregisterApplicationStateObserver erase size:%{public}zu", observerIds_.size());
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }
};
} // namespace

NativeValue* JsErrorManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("Js error manager Init.");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_INFO("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_INFO("object null");
        return nullptr;
    }

    std::unique_ptr<JsErrorManager> jsErrorManager = std::make_unique<JsErrorManager>();
    object->SetNativePointer(jsErrorManager.release(), JsErrorManager::Finalizer, nullptr);

    //make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    HILOG_INFO("JsErrorManager BindNativeFunction called");
    BindNativeFunction(*engine, *object, "registerErrorObserver", JsErrorManager::RegisterErrorObserver);
    BindNativeFunction(*engine, *object, "unregisterErrorObserver", JsErrorManager::UnregisterErrorObserver);
    return engine->CreateUndefined();
}

JsErrorObserver::JsErrorObserver(NativeEngine& engine) : engine_(engine) {}

JsErrorObserver::~JsErrorObserver() = default;

void JsErrorObserver::OnUnhandledException(std::string errMsg)
{
    HILOG_DEBUG("OnUnhandledException come.");
    std::unique_ptr<AsyncTask::CompleteCallback> complete = std::make_unique<AsyncTask::CompleteCallback>
        ([jsObserver = this, errMsg](NativeEngine &engine, AsyncTask &task, int32_t status) {
            if (jsObserver) {
                jsObserver->HandleOnUnhandledException(errMsg);
            }
        });
    NativeReference* callback = nullptr;
    std::unique_ptr<AsyncTask::ExecuteCallback> execute = nullptr;
    AsyncTask::Schedule(
        engine_, std::make_unique<AsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsErrorObserver::HandleOnUnhandledException(const std::string &errMsg)
{
    HILOG_DEBUG("HandleOnUnhandledException come.");
    NativeValue* argv[] = { CreateJsValue(engine_, errMsg) };
    CallJsFunction("onUnhandledException", argv, ARGC_ONE);
}

void JsErrorObserver::CallJsFunction(const char* methodName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("CallJsFunction begin, method:%{public}s", methodName);
    if (jsObserverObject_ == nullptr) {
        HILOG_ERROR("jsObserverObject_ nullptr");
        return;
    }
    NativeValue* value = jsObserverObject_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return;
    }

    NativeValue* method = obj->GetProperty(methodName);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get from object");
        return;
    }
    HILOG_INFO("CallJsFunction CallFunction success");
    engine_.CallFunction(value, method, argv, argc);
    HILOG_INFO("CallJsFunction end");
}

void JsErrorObserver::SetJsObserverObject(NativeValue* jsObserverObject)
{
    jsObserverObject_ = std::unique_ptr<NativeReference>(engine_.CreateReference(jsObserverObject, 1));
}
}  // namespace AbilityRuntime
}  // namespace OHOS