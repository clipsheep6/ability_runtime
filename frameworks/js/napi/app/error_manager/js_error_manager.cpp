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
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char* ON_OFF_TYPE = "error";
constexpr const char* ON_OFF_TYPE_SYNC = "errorEvent";

class JsErrorManager final {
public:
    JsErrorManager() {}
    ~JsErrorManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint)
    {
        HILOG_INFO("JsErrorManager Finalizer is called");
        std::unique_ptr<JsErrorManager>(static_cast<JsErrorManager*>(data));
    }

    static NativeValue* On(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsErrorManager* me = CheckParamsAndGetThis<JsErrorManager>(engine, info);
        return (me != nullptr) ? me->OnOn(*engine, *info) : nullptr;
    }

    static NativeValue* Off(NativeEngine* engine, NativeCallbackInfo* info)
    {
        JsErrorManager* me = CheckParamsAndGetThis<JsErrorManager>(engine, info);
        return (me != nullptr) ? me->OnOff(*engine, *info) : nullptr;
    }

private:
    NativeValue* OnOn(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        std::string type = ParseParamType(engine, info);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(engine, info);
        }
        return OnOnOld(engine, info);
    }

    NativeValue* OnOnOld(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        if (info.argc != ARGC_TWO) {
            HILOG_ERROR("The param is invalid, observers need.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            HILOG_ERROR("Parse type failed");
            return engine.CreateUndefined();
        }
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
        observer_->AddJsObserverObject(observerId, info.argv[INDEX_ONE]);
        return engine.CreateNumber(observerId);
    }

    NativeValue* OnOnNew(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        if (info.argc < ARGC_TWO) {
            HILOG_ERROR("The param is invalid, observers need.");
            ThrowTooFewParametersError(engine);
            return engine.CreateUndefined();
        }

        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_OBJECT) {
            HILOG_ERROR("Invalid param");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

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
        observer_->AddJsObserverObject(observerId, info.argv[INDEX_ONE], true);
        return engine.CreateNumber(observerId);
    }

    NativeValue* OnOff(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        std::string type = ParseParamType(engine, info);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(engine, info);
        }
        return OnOffOld(engine, info);
    }

    NativeValue* OnOffOld(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        int32_t observerId = -1;
        if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
            ThrowTooFewParametersError(engine);
            HILOG_ERROR("unregister errorObserver error, not enough params.");
        } else {
            napi_get_value_int32(reinterpret_cast<napi_env>(&engine),
                reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), &observerId);
            HILOG_INFO("unregister errorObserver called, observer:%{public}d", observerId);
        }

        std::string type;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            HILOG_ERROR("Parse type failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }

        AsyncTask::CompleteCallback complete =
            [&observer = observer_, observerId](
                NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_INFO("Unregister errorObserver called.");
                if (observerId == -1) {
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                    return;
                }
                if (observer && observer->RemoveJsObserverObject(observerId)) {
                    task.ResolveWithNoError(engine, engine.CreateUndefined());
                } else {
                    task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_ID));
                }
                if (observer && observer->IsEmpty()) {
                    AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
                    observer = nullptr;
                }
            };

        NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
        NativeValue* result = nullptr;
        AsyncTask::Schedule("JSErrorManager::OnUnregisterErrorObserver",
            engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    NativeValue* OnOffNew(NativeEngine& engine, NativeCallbackInfo& info)
    {
        HILOG_DEBUG("called.");
        if (info.argc < ARGC_TWO) {
            ThrowTooFewParametersError(engine);
            HILOG_ERROR("unregister errorObserver error, not enough params.");
            return engine.CreateUndefined();
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], observerId)) {
            HILOG_ERROR("Parse observerId failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return engine.CreateUndefined();
        }
        if (observer_ == nullptr) {
            HILOG_ERROR("observer is nullptr");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }
        if (observer_->RemoveJsObserverObject(observerId, true)) {
            HILOG_DEBUG("RemoveJsObserverObject success");
        } else {
            HILOG_ERROR("RemoveJsObserverObject failed");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_ID);
        }
        if (observer_->IsEmpty()) {
            AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
            observer_ = nullptr;
        }
        return engine.CreateUndefined();
    }

    std::string ParseParamType(NativeEngine& engine, const NativeCallbackInfo& info)
    {
        std::string type;
        if (info.argc > INDEX_ZERO && ConvertFromJsValue(engine, info.argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
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
    BindNativeFunction(*engine, *object, "on", moduleName, JsErrorManager::On);
    BindNativeFunction(*engine, *object, "off", moduleName, JsErrorManager::Off);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
