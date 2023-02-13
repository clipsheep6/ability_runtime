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

#include "ability_business_error.h"
#include "application_data_manager.h"
#include "hilog_wrapper.h"
#include "js_error_observer.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifndef ENABLE_ERRCODE
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t INVALID_PARAM = -2;
#endif
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_TWO = 2;
#ifdef ENABLE_ERRCODE
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_THREE = 3;
#else
constexpr size_t ARGC_ONE = 1;
#endif

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
    NativeValue* OnRegisterErrorObserver(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_INFO("Register errorObserver is called.");
#ifdef ENABLE_ERRCODE
        if (info.argc != ARGC_TWO) {
            HILOG_ERROR("The param is invalid, observers need.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != "error") {
            HILOG_ERROR("Parse type failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
#else
        // only support one
        if (info.argc != ARGC_ONE) {
            HILOG_ERROR("The param is invalid, observers need.");
            return engine.CreateUndefined();
        }
#endif
        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            // create observer
            observer_ = std::make_shared<JsErrorObserver>(engine);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
#ifdef ENABLE_ERRCODE
        observer_->AddJsObserverObject(observerId, info.argv[INDEX_ONE]);
#else
        observer_->AddJsObserverObject(observerId, info.argv[INDEX_ZERO]);
#endif
        return engine.CreateNumber(observerId);
    }

    NativeValue* OnUnregisterErrorObserver(NativeEngine& engine, NativeCallbackInfo& info)
    {
        int32_t observerId = -1;
#ifdef ENABLE_ERRCODE
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            ThrowTooFewParametersError(engine);
            HILOG_ERROR("unregister errorObserver error, not enough params.");
        } else {
            napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), &observerId);
            HILOG_INFO("unregister errorObserver called, observer:%{public}d", observerId);
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != "error") {
            HILOG_ERROR("Parse type failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
#else
        // only support one or two params
        if (info.argc != ARGC_ONE && info.argc != ARGC_TWO) {
            HILOG_ERROR("unregister errorObserver error, not enough params.");
        } else {
            napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), &observerId);
            HILOG_INFO("unregister errorObserver called, observer:%{public}d", observerId);
        }
#endif

        std::weak_ptr<JsErrorObserver> observerWptr(observer_);
        AsyncTask::CompleteCallback complete =
            [observerWptr, observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
                HILOG_INFO("Unregister errorObserver called.");
                if (observerId == -1) {
#ifdef ENABLE_ERRCODE
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
#else
                    task.Reject(engine, CreateJsError(engine, INVALID_PARAM, "param is invalid!"));
#endif
                    return;
                }
                auto observer = observerWptr.lock();
                bool isEmpty = false;
                if (observer && observer->RemoveJsObserverObject(observerId, isEmpty)) {
#ifdef ENABLE_ERRCODE
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
#else
                    task.Resolve(engine, engine.CreateUndefined());
#endif
                } else {
#ifdef ENABLE_ERRCODE
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_ID));
#else
                    task.Reject(engine, CreateJsError(engine, ERROR_CODE, "observer is not exist!"));
#endif
                }
                if (isEmpty) {
                    AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
                }
            };

#ifdef ENABLE_ERRCODE
        NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
#else
        NativeValue* lastParam = (info.argc <= ARGC_ONE) ? nullptr : info.argv[INDEX_ONE];
#endif
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSErrorManager::OnUnregisterErrorObserver",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    int32_t serialNumber_ = 0;
    std::shared_ptr<JsErrorObserver> observer_;
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
    const char *moduleName = "JsErrorManager";
#ifdef ENABLE_ERRCODE
    BindNativeFunction(*engine, *object, "on", moduleName, JsErrorManager::RegisterErrorObserver);
    BindNativeFunction(*engine, *object, "off", moduleName, JsErrorManager::UnregisterErrorObserver);
#else
    BindNativeFunction(*engine, *object, "registerErrorObserver", moduleName, JsErrorManager::RegisterErrorObserver);
    BindNativeFunction(*engine, *object, "unregisterErrorObserver",
        moduleName, JsErrorManager::UnregisterErrorObserver);
#endif
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
