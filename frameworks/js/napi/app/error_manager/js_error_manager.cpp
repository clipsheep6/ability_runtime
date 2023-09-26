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

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        HILOG_INFO("JsErrorManager Finalizer is called");
        std::unique_ptr<JsErrorManager>(static_cast<JsErrorManager*>(data));
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOff);
    }

private:
    napi_value OnOn(napi_env env, const size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        }
        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, const size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        if (argc != ARGC_TWO) {
            HILOG_ERROR("The param is invalid, observers need.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            HILOG_ERROR("Parse type failed");
            return CreateJsUndefined(env);
        }
        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            // create observer
            observer_ = std::make_shared<JsErrorObserver>(env);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
        observer_->AddJsObserverObject(observerId, argv[INDEX_ONE]);
        return CreateJsValue(env, observerId);
    }

    napi_value OnOnNew(napi_env env, const size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        if (argc < ARGC_TWO) {
            HILOG_ERROR("The param is invalid, observers need.");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            HILOG_ERROR("Invalid param");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            // create observer
            observer_ = std::make_shared<JsErrorObserver>(env);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
        observer_->AddJsObserverObject(observerId, argv[INDEX_ONE], true);
        return CreateJsValue(env, observerId);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        }
        return OnOffOld(env, argc, argv);
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        int32_t observerId = -1;
        if (argc != ARGC_TWO && argc != ARGC_THREE) {
            ThrowTooFewParametersError(env);
            HILOG_ERROR("unregister errorObserver error, not enough params.");
        } else {
            napi_get_value_int32(env, argv[INDEX_ONE], &observerId);
            HILOG_INFO("unregister errorObserver called, observer:%{public}d", observerId);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            HILOG_ERROR("Parse type failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::CompleteCallback complete =
            [&observer = observer_, observerId](
                napi_env env, NapiAsyncTask& task, int32_t status) {
            HILOG_INFO("Unregister errorObserver called.");
                if (observerId == -1) {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                    return;
                }
                if (observer && observer->RemoveJsObserverObject(observerId)) {
                    task.ResolveWithNoError(env, CreateJsUndefined(env));
                } else {
                    task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID));
                }
                if (observer && observer->IsEmpty()) {
                    AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
                    observer = nullptr;
                }
            };

        napi_value lastParam = (argc <= ARGC_TWO) ? nullptr : argv[INDEX_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSErrorManager::OnUnregisterErrorObserver",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        HILOG_DEBUG("called.");
        if (argc < ARGC_TWO) {
            ThrowTooFewParametersError(env);
            HILOG_ERROR("unregister errorObserver error, not enough params.");
            return CreateJsUndefined(env);
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], observerId)) {
            HILOG_ERROR("Parse observerId failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }
        if (observer_ == nullptr) {
            HILOG_ERROR("observer is nullptr");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (observer_->RemoveJsObserverObject(observerId, true)) {
            HILOG_DEBUG("RemoveJsObserverObject success");
        } else {
            HILOG_ERROR("RemoveJsObserverObject failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID);
        }
        if (observer_->IsEmpty()) {
            AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
            observer_ = nullptr;
        }
        return CreateJsUndefined(env);
    }

    std::string ParseParamType(napi_env env, const size_t argc, napi_value* argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }

    int32_t serialNumber_ = 0;
    std::shared_ptr<JsErrorObserver> observer_;
};
} // namespace

napi_value JsErrorManagerInit(napi_env env, napi_value exportObj)
{
    HILOG_INFO("Js error manager Init.");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_INFO("env or exportObj null");
        return nullptr;
    }
    std::unique_ptr<JsErrorManager> jsErrorManager = std::make_unique<JsErrorManager>();
    napi_wrap(env, exportObj, jsErrorManager.release(), JsErrorManager::Finalizer, nullptr, nullptr);

    HILOG_INFO("JsErrorManager BindNativeFunction called");
    const char *moduleName = "JsErrorManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsErrorManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsErrorManager::Off);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
