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

#include "js_insight_intent_context.h"

#include "ability_window_configuration.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "js_error_utils.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr static char CONTEXT_MODULE_NAME[] = "InsightIntentContext";
}

void JsInsightIntentContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("enter");
    std::unique_ptr<JsInsightIntentContext>(static_cast<JsInsightIntentContext*>(data));
}

napi_value JsInsightIntentContext::StartAbiity(napi_env env, napi_callback_info info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentContext, OnStartAbility);
}

napi_value JsInsightIntentContext::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_DEBUG("enter");
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (info.argc == 0) {
        HILOG_ERROR("not enough args");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    // unwrap want
    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, info.argv[0], want);

    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("invalid context");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    // verify if bundleName is empty or invalid
    auto bundleNameFromWant = want.GetElement().GetBundleName();
    if (bundleNameFromWant.empty() || bundleNameFromWant != context->GetBundleName()) {
        HILOG_ERROR("bundleName is empty or invalid");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
        return CreateJsUndefined(env);
    }
    // modify windowmode setting
    auto windowMode = context->GetCurrentWindowMode();
    if (windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY ||
        windowMode == AAFwk::AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY) {
        want.SetParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, windowMode);
    }

    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    // create execute task
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_ERROR("context is released");
            *innerErrCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrCode = context->StartAbilityByInsightIntent(want);
    };
    // create complete task
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrCode == ERR_OK) {
            HILOG_DEBUG("StartAbility success.");
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
        }
    };

    napi_value lastParam = (info.argc > 1) ? info.argv[1] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsInsightIntentContext::OnStartAbility", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    HILOG_DEBUG("end");
    return result;
}

napi_value CreateJsInsightIntentContext(napi_env env, const std::shared_ptr<InsightIntentContext>& context)
{
    HILOG_DEBUG("enter");
    napi_value contextObj;
    napi_create_object(env, &contextObj);

    std::unique_ptr<JsInsightIntentContext> jsInsightIntentContext = std::make_unique<JsInsightIntentContext>(context);
    napi_wrap(env, contextObj, jsInsightIntentContext.release(), JsInsightIntentContext::Finalizer, nullptr, nullptr);

    BindNativeFunction(env, contextObj, "startAbility", CONTEXT_MODULE_NAME, JsInsightIntentContext::StartAbiity);
    HILOG_DEBUG("end");
    return contextObj;
}
} // namespace AbilityRuntime
} // namespace OHOS
