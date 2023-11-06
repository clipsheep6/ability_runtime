/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
    using InsightIntentExecuteTask = std::function<void(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult)>;
    explicit JsInsightIntentExecuteCallbackClient(InsightIntentExecuteTask &&task) : task_(std::move(task))
    {
        handler_ = std::make_shared<EventHandler>(EventRunner::GetMainEventRunner());
    }

    virtual ~JsInsightIntentExecuteCallbackClient() = default;

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override
    {
        if (handler_) {
            handler_->PostSyncTask([client = weak_from_this(), resultCode, executeResult] () {
                auto impl = client.lock();
                if (impl == nullptr) {
                    return;
                }
                impl->task_(resultCode, executeResult);
            });
        }
    }
private:
    InsightIntentExecuteTask task_;
    std::shared_ptr<EventHandler> handler_ = nullptr;
};

class JsInsightIntentDriver {
public:
    JsInsightIntentDriver() = default;
    ~JsInsightIntentDriver() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        HILOG_INFO("JsInsightIntentDriver::Finalizer is called");
        std::unique_ptr<JsInsightIntentDriver>(static_cast<JsInsightIntentDriver*>(data));
    }

    static napi_value Execute(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnExecute);
    }

private:
    napi_value OnExecute(napi_env env, NapiCallbackInfo& info)
    {
        HILOG_DEBUG("called");
        if (info.argc < ARGC_ONE) {
            HILOG_ERROR("Params not match");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        InsightIntentExecuteParam param;
        if (!UnwrapExecuteParam(env, info.argv[INDEX_ZERO], param)) {
            HILOG_ERROR("CheckOnOffType, Parse on off type failed");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return CreateJsUndefined(env);
        }

        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[INDEX_ONE];
        napi_value result = nullptr;

        std::unique_ptr<NapiAsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(
            env, lastParam, nullptr, nullptr, &result);
        std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
        if (asyncTask == nullptr) {
            HILOG_ERROR("asyncTask is nullptr");
            return CreateJsUndefined(env);
        }
        JsInsightIntentExecuteCallbackClient::InsightIntentExecuteTask task = [env, asyncTask]
            (int32_t resultCode, InsightIntentExecuteResult executeResult) {
            if (resultCode != 0) {
                asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(resultCode)));
                return;
            } else {
                asyncTask->ResolveWithNoError(env, CreateJsExecuteResult(env, executeResult));
            }
        };
        auto client = std::make_shared<JsInsightIntentExecuteCallbackClient>(std::move(task));
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
    HILOG_DEBUG("JsInsightIntentDriverInit is called");
    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Invalid input parameters");
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
