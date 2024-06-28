/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent_driver.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_callback_interface.h"
#include "insight_intent_host_client.h"
#include "insight_intent_execute_result.h"
#include "js_error_utils.h"
#include "js_insight_intent_driver_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_param.h"
#include "napi_common_util.h"
#include "native_engine/native_value.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
}
class JsInsightIntentExecuteCallbackClient : public InsightIntentExecuteCallbackInterface,
    public std::enable_shared_from_this<JsInsightIntentExecuteCallbackClient> {
public:
    JsInsightIntentExecuteCallbackClient(napi_env env, napi_deferred nativeDeferred, napi_ref callbackRef)
        : env_(env), nativeDeferred_(nativeDeferred), callbackRef_(callbackRef) {}

    virtual ~JsInsightIntentExecuteCallbackClient() = default;

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override
    {
        NapiAsyncTask::CompleteCallback complete = [resultCode = resultCode, executeResult = executeResult]
            (napi_env env, NapiAsyncTask &task, int32_t status) {
            if (resultCode != 0) {
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(resultCode)));
            } else {
                task.ResolveWithNoError(env, CreateJsExecuteResult(env, executeResult));
            }
        };
        std::unique_ptr<NapiAsyncTask> asyncTask = nullptr;
        if (nativeDeferred_) {
            asyncTask = std::make_unique<NapiAsyncTask>(nativeDeferred_, nullptr,
                std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)));
        } else {
            asyncTask = std::make_unique<NapiAsyncTask>(callbackRef_, nullptr,
                std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)));
        }
        NapiAsyncTask::Schedule("JsInsightIntentDriver::OnExecute", env_, std::move(asyncTask));
    }
private:
    napi_env env_;
    napi_deferred nativeDeferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
};

class JsInsightIntentDriver {
public:
    JsInsightIntentDriver() = default;
    ~JsInsightIntentDriver() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGI(AAFwkTag::INTENT, "JsInsightIntentDriver::Finalizer is called");
        std::unique_ptr<JsInsightIntentDriver>(static_cast<JsInsightIntentDriver*>(data));
    }

    static napi_value Execute(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnExecute);
    }

private:
    napi_value OnExecute(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        InsightIntentExecuteParam param;
        if (!UnwrapExecuteParam(env, info.argv[INDEX_ZERO], param)) {
            TAG_LOGE(AAFwkTag::INTENT, "CheckOnOffType, Parse on off type failed");
            ThrowInvalidParamError(env, "Parameter error: Parse param failed, param must be a ExecuteParam.");
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[INDEX_ONE];
        napi_valuetype type = napi_undefined;
        napi_typeof(env, lastParam, &type);

        napi_value result = nullptr;
        napi_deferred nativeDeferred = nullptr;
        napi_ref callbackRef = nullptr;
        std::unique_ptr<NapiAsyncTask> asyncTask = nullptr;
        if (lastParam == nullptr || type != napi_function) {
            napi_create_promise(env, &nativeDeferred, &result);
            asyncTask = std::make_unique<NapiAsyncTask>(nativeDeferred, nullptr, nullptr);
        } else {
            napi_get_undefined(env, &result);
            napi_create_reference(env, lastParam, 1, &callbackRef);
            asyncTask = std::make_unique<NapiAsyncTask>(callbackRef, nullptr, nullptr);
        }

        if (asyncTask == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "asyncTask is nullptr");
            return CreateJsUndefined(env);
        }
        auto client = std::make_shared<JsInsightIntentExecuteCallbackClient>(env, nativeDeferred, callbackRef);
        uint64_t key = InsightIntentHostClient::GetInstance()->AddInsightIntentExecute(client);
        auto err = AbilityManagerClient::GetInstance()->ExecuteIntent(key,
            InsightIntentHostClient::GetInstance(), param);
        if (err != 0) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(err)));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        }
        return result;
    }
};

napi_value JsInsightIntentDriverInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "JsInsightIntentDriverInit is called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Invalid input parameters");
        return nullptr;
    }

    std::unique_ptr<JsInsightIntentDriver> jsIntentDriver = std::make_unique<JsInsightIntentDriver>();
    napi_wrap(env, exportObj, jsIntentDriver.release(), JsInsightIntentDriver::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsInsightIntentDriver";
    BindNativeFunction(env, exportObj, "execute", moduleName, JsInsightIntentDriver::Execute);
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
