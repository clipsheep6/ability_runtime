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

#include "js_error_manager.h"

#include <cstdint>

#include "app_data_manager.h"
#include "hilog_wrapper.h"
#include "js_error_observer.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE = -1;
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
        HILOG_INFO("Register errorObserver is called.");
        // only support one
        if (info.argc != ARGC_ONE) {
            HILOG_ERROR("The param is invalid, observers need.");
            return engine.CreateUndefined();
        }

        // unwarp observer
        std::shared_ptr<JsErrorObserver> observer = std::make_shared<JsErrorObserver>(engine);
        observer->SetJsObserverObject(info.argv[0]);
        int64_t observerId = serialNumber_;
        if (serialNumber_ < INT64_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        HILOG_INFO("%{public}s create observer", __func__);
        AsyncTask::CompleteCallback complete =
            [&observerId, observer](NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("RegisterApplicationStateObserver callback begin");
                if (!DelayedSingleton<AppExecFwk::AppDataManager>::GetInstance()->AddErrorObservers(
                    observerId, observer)) {
                    observerId = -1;
                }
            };

        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, nullptr, nullptr, std::move(complete), &result));
        if (observerId == -1) {
            return engine.CreateUndefined();
        }
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
        napi_get_value_int64(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
        HILOG_INFO("unregister errorObserver called, observer:%{public}d", (int32_t)observerId);

        AsyncTask::CompleteCallback complete =
            [observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("Unregister errorObserver called.");
                if (DelayedSingleton<AppExecFwk::AppDataManager>::GetInstance()->RemoveErrorObservers(observerId)) {
                    task.Resolve(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE, "observer is not exist!"));
                }
            };

        NativeValue* lastParam = (info.argc == ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
        NativeValue* result = nullptr;
        AsyncTask::Schedule(
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    int64_t serialNumber_ = 0;
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
        HILOG_INFO("object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsErrorManager> jsErrorManager = std::make_unique<JsErrorManager>();
    object->SetNativePointer(jsErrorManager.release(), JsErrorManager::Finalizer, nullptr);

    HILOG_INFO("JsErrorManager BindNativeFunction called");
    BindNativeFunction(*engine, *object, "registerErrorObserver", JsErrorManager::RegisterErrorObserver);
    BindNativeFunction(*engine, *object, "unregisterErrorObserver", JsErrorManager::UnregisterErrorObserver);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS