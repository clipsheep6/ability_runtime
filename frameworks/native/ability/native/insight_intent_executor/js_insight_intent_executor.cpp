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

#include "js_insight_intent_executor.h"

#include <want_params.h>

#include "ability_transaction_callback_info.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS::AbilityRuntime {

std::shared_ptr<JsInsightIntentExecutor> JsInsightIntentExecutor::Create(JsRuntime& runtime)
{
    std::shared_ptr<JsInsightIntentExecutor> ptr(new (std::nothrow) JsInsightIntentExecutor(runtime));
    return ptr;
}

using State = JsInsightIntentExecutor::State;

JsInsightIntentExecutor::JsInsightIntentExecutor(JsRuntime& runtime) : runtime_(runtime)
{ }

JsInsightIntentExecutor::~JsInsightIntentExecutor()
{
    state_ = State::DESTROYED;
    TAG_LOGI(AAFwkTag::INTENT, "JsInsightIntentExecutor destructor");
}

bool JsInsightIntentExecutor::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    jsObj_ = JsInsightIntentExecutor::LoadJsCode(insightIntentInfo, runtime_);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "LoadJsCode failed.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Napi env invalid.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Context invalid.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    napi_value contextObj = CreateJsInsightIntentContext(env, context);
    contextObj_ = JsRuntime::LoadSystemModuleByEngine(env, "app.ability.InsightIntentContext", &contextObj, 1);
    if (contextObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Load system module failed.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto executorNapiVal = jsObj_->GetNapiValue();
    auto contextNapiVal = contextObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, contextNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "context", contextNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "Set property failed.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    return true;
}

bool JsInsightIntentExecutor::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentExecutor::HandleExecuteIntent(
    InsightIntentExecuteMode mode,
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "HandleExecuteIntent invalid callback.");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    callback_ = std::move(callback);
    bool successful = false;
    switch (mode) {
        case InsightIntentExecuteMode::UIABILITY_FOREGROUND:
            if (!JsInsightIntentExecutor::CheckParametersUIAbilityForeground(pageLoader)) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIAbilityForeground error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIAbilityForeground(name, param, pageLoader);
            break;
        case InsightIntentExecuteMode::UIABILITY_BACKGROUND:
            if (!JsInsightIntentExecutor::CheckParametersUIAbilityBackground()) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIAbilityBackground error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIAbilityBackground(name, param);
            break;
        case InsightIntentExecuteMode::UIEXTENSION_ABILITY:
            if (!JsInsightIntentExecutor::CheckParametersUIExtension(pageLoader)) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersUIExtension error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentUIExtension(name, param, pageLoader);
            break;
        case InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY:
            if (!JsInsightIntentExecutor::CheckParametersServiceExtension()) {
                TAG_LOGE(AAFwkTag::INTENT, "CheckParametersServiceExtension error");
                return ExecuteIntentCheckError();
            }
            successful = ExecuteInsightIntentServiceExtension(name, param);
            break;
        default:
            TAG_LOGE(AAFwkTag::INTENT, "InsightIntentExecuteMode not supported yet");
            return ExecuteIntentCheckError();
    }
    isAsync = isAsync_;
    return successful;
}

std::unique_ptr<NativeReference> JsInsightIntentExecutor::LoadJsCode(
    const InsightIntentExecutorInfo& info,
    JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Execute param invalid.");
        return std::unique_ptr<NativeReference>();
    }

    std::string moduleName(executeParam->moduleName_);
    std::string srcPath(executeParam->moduleName_ + "/" + info.srcEntry);
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");

    std::unique_ptr<NativeReference> jsCode(
        runtime.LoadModule(moduleName, srcPath, info.hapPath, info.esmodule));
    return jsCode;
}

bool JsInsightIntentExecutor::CallJsFunctionWithResult(
    napi_env env,
    napi_value obj,
    const char* funcName,
    size_t argc,
    const napi_value* argv,
    napi_value& result)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    napi_value method = AppExecFwk::GetPropertyValueByPropertyName(
        env,
        obj,
        funcName,
        napi_valuetype::napi_function);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "CallJsFunctionWithResult error");
        return false;
    }
    napi_call_function(
        env,
        obj,
        method,
        argc,
        argv,
        &result);
    return true;
}

bool JsInsightIntentExecutor::CallJsFunctionWithResultInner(
    const char* funcName,
    size_t argc,
    const napi_value* argv,
    napi_value& result)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    auto* env = runtime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "CallJsFunctionWithResultInner Type error");
        return false;
    }
    return JsInsightIntentExecutor::CallJsFunctionWithResult(
        env,
        obj,
        funcName,
        argc,
        argv,
        result);
}

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> JsInsightIntentExecutor::GetResultFromJs(
    napi_env env, napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    if (!UnwrapExecuteResult(env, resultJs, *resultCpp)) {
        return nullptr;
    }
    return resultCpp;
}

napi_value JsInsightIntentExecutor::ResolveCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data);
    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        JsInsightIntentExecutor::ReplyFailed(callback);
        return nullptr;
    }
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp =
        JsInsightIntentExecutor::GetResultFromJs(env, resultJs);
    JsInsightIntentExecutor::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

napi_value JsInsightIntentExecutor::RejectCbCpp(napi_env env, napi_callback_info info)
{
    void* data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    JsInsightIntentExecutor::ReplyFailed(callback);
    return nullptr;
}

void JsInsightIntentExecutor::ReplyFailed(InsightIntentExecutorAsyncCallback* callback,
    InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    if (callback == nullptr) {
        return;
    }
    AppExecFwk::InsightIntentExecuteResult errorResult{};
    errorResult.innerErr = innerErr;
    callback->Call(errorResult);
    delete callback;
}

void JsInsightIntentExecutor::ReplySucceeded(InsightIntentExecutorAsyncCallback* callback,
    std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    if (callback == nullptr) {
        return;
    }
    if (resultCpp == nullptr) {
        ReplyFailed(callback);
        return;
    }
    resultCpp->innerErr = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    callback->Call(*resultCpp);
    delete callback;
}

void JsInsightIntentExecutor::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    state_ = JsInsightIntentExecutor::State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentExecutor::ReplyFailed(callback, innerErr);
}

void JsInsightIntentExecutor::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    state_ = JsInsightIntentExecutor::State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentExecutor::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentExecutor::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        ReplyFailedInner();
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }
    bool isPromise = false;
    napi_is_promise(env, resultJs, &isPromise);

    isAsync_ = isPromise;

    if (isPromise) {
        TAG_LOGI(AAFwkTag::INTENT, "Is promise");
        auto* callback = callback_.release();

        napi_value then = nullptr;
        napi_get_named_property(env, resultJs, "then", &then);
        napi_value resolveCbJs = nullptr;
        napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            ResolveCbCpp, callback, &resolveCbJs);
        constexpr size_t argcThen = 1;
        napi_value argvThen[argcThen] = { resolveCbJs };
        napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr);

        napi_value promiseCatch = nullptr;
        napi_get_named_property(env, resultJs, "catch", &promiseCatch);
        napi_value rejectCbJs = nullptr;
        napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            RejectCbCpp, callback, &rejectCbJs);
        constexpr size_t argcCatch = 1;
        napi_value argvCatch[argcCatch] = { rejectCbJs };
        napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr);
    } else {
        TAG_LOGI(AAFwkTag::INTENT, "Not promise");
        auto resultCpp = JsInsightIntentExecutor::GetResultFromJs(env, resultJs);
        if (resultCpp == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "Result invalid");
            ReplyFailedInner();
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
        TAG_LOGD(AAFwkTag::INTENT, "Call succeed");
        ReplySucceededInner(resultCpp);
    }
    return true;
}

bool JsInsightIntentExecutor::CheckParametersUIAbilityForeground(const std::shared_ptr<NativeReference>& windowStage)
{
    return windowStage != nullptr;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIAbilityForeground(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& windowStageJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIABILITY_FOREGROUND);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs, windowStageJs->GetNapiValue() };
    napi_value result = nullptr;

    if (!CallJsFunctionWithResultInner(funcName, argc, argv, result)) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersUIAbilityBackground()
{
    return true;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIAbilityBackground(
    const std::string& name,
    const AAFwk::WantParams& param)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIABILITY_BACKGROUND);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs };
    napi_value result = nullptr;

    if (!CallJsFunctionWithResultInner(funcName, argc, argv, result)) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersUIExtension(
    const std::shared_ptr<NativeReference>& UIExtensionContentSession)
{
    return UIExtensionContentSession != nullptr;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentUIExtension(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& UIExtensionContentSession)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::UIEXTENSION_ABILITY);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs, UIExtensionContentSession->GetNapiValue() };
    napi_value result = nullptr;

    if (!CallJsFunctionWithResultInner(funcName, argc, argv, result)) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentExecutor::CheckParametersServiceExtension()
{
    return true;
}

bool JsInsightIntentExecutor::ExecuteInsightIntentServiceExtension(
    const std::string& name,
    const AAFwk::WantParams& param)
{
    TAG_LOGD(AAFwkTag::INTENT, "enter");
    HandleScope handleScope(runtime_);

    constexpr auto intentMode = static_cast<size_t>(InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY);
    constexpr auto funcName = JsInsightIntentExecutor::JS_FUNC_NAME_FOR_MODE[intentMode];
    constexpr auto argc = JsInsightIntentExecutor::JS_ARGC_FOR_MODE[intentMode];

    auto* env = runtime_.GetNapiEnv();
    napi_value nameJs = AppExecFwk::WrapStringToJS(env, name);
    napi_value paramJs = AppExecFwk::WrapWantParams(env, param);
    napi_value argv[argc] = { nameJs, paramJs };
    napi_value result = nullptr;

    if (!CallJsFunctionWithResultInner(funcName, argc, argv, result)) {
        ReplyFailedInner();
        return false;
    }

    return HandleResultReturnedFromJsFunc(result);
}
} // namespace OHOS::AbilityRuntime
