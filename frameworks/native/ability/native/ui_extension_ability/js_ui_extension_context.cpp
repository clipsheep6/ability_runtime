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

#include "js_ui_extension_context.h"

#include <cstdint>

#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_error_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_ability.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "napi_common_start_options.h"
#include "start_options.h"
#include "hitrace_meter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t ERROR_CODE_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
} // namespace

static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;

void JsUIExtensionContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContext Finalizer is called");
    std::unique_ptr<JsUIExtensionContext>(static_cast<JsUIExtensionContext*>(data));
}

NativeValue *JsUIExtensionContext::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::StartAbilityForResult(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsUIExtensionContext *me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue* JsUIExtensionContext::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContext* me = CheckParamsAndGetThis<JsUIExtensionContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContext::OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HILOG_DEBUG("OnStartAbility is called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Start ability failed, not enough params.");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    size_t unwrapArgc = 0;
    AAFwk::Want want;
    AAFwk::StartOptions startOptions;
    if (!CheckStartAbilityInputParam(engine, info, want, startOptions, unwrapArgc)) {
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, startOptions, unwrapArgc](NativeEngine& engine, AsyncTask& task, int32_t status) {
            HILOG_DEBUG("startAbility begin");
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            ErrCode innerErrorCode = ERR_OK;
            (unwrapArgc == 1) ? innerErrorCode = context->StartAbility(want) :
                innerErrorCode = context->StartAbility(want, startOptions);
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc == unwrapArgc) ? nullptr : info.argv[unwrapArgc];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionContext OnStartAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::OnTerminateSelf(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_DEBUG("OnTerminateSelf is called");
    AsyncTask::CompleteCallback complete =
        [weak = context_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_ERROR("context is released");
                task.Reject(engine, CreateJsError(engine, ERROR_CODE_ONE, "Context is released"));
                return;
            }

            ErrCode innerErrorCode = context->TerminateSelf();
            if (innerErrorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, innerErrorCode));
            }
        };

    NativeValue* lastParam = (info.argc == ARGC_ZERO) ? nullptr : info.argv[INDEX_ZERO];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JSUIExtensionContext OnTerminateSelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContext::OnStartAbilityForResult(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    if (info.argc == ARGC_ZERO) {
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    if (!UnWrapWant(engine, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("failed to parse want!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    decltype(info.argc) unwrapArgc = 1;
    AAFwk::StartOptions startOptions;
    if (info.argc > ARGC_ONE && info.argv[INDEX_ONE]->TypeOf() == NATIVE_OBJECT) {
        HILOG_INFO("start options is used.");
        AppExecFwk::UnwrapStartOptions(
            reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ONE]), startOptions);
        unwrapArgc++;
    }
    NativeValue *lastParam = info.argc > unwrapArgc ? info.argv[unwrapArgc] : nullptr;
    NativeValue *result = nullptr;
    std::unique_ptr<AsyncTask> uasyncTask = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int resultCode, const AAFwk::Want &want, bool isInner) {
        HILOG_INFO("async callback is called.");
        NativeValue *abilityResult = WrapAbilityResult(engine, resultCode, want);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed.");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            return;
        }
        if (isInner) {
            asyncTask->Reject(engine, CreateJsErrorByNativeErr(engine, resultCode));
            return;
        }
        asyncTask->Resolve(engine, abilityResult);
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released.");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
        return result;
    }
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
    (unwrapArgc == INDEX_ONE) ? context->StartAbilityForResult(want, curRequestCode_, std::move(task))
                              : context->StartAbilityForResult(want, startOptions, curRequestCode_, std::move(task));
    HILOG_DEBUG("end.");
    return result;
}

NativeValue* JsUIExtensionContext::OnTerminateSelfWithResult(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_INFO("OnTerminateSelfWithResult is called");

    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    int resultCode = 0;
    AAFwk::Want want;
    if (!JsUIExtensionContext::UnWrapAbilityResult(engine, info.argv[0], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [weak = context_, want, resultCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto context = weak.lock();
            if (!context) {
                HILOG_WARN("context is released");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
                return;
            }

            auto errorCode = context->TerminateSelf();
            if (errorCode == 0) {
                task.Resolve(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsUIExtensionContext::OnTerminateSelfWithResult",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    HILOG_INFO("OnTerminateSelfWithResult is called end");
    return result;
}

NativeValue *JsUIExtensionContext::CreateJsUIExtensionContext(NativeEngine& engine,
    std::shared_ptr<UIExtensionContext> context)
{
    HILOG_DEBUG("CreateJsUIExtensionContext begin");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context) {
        abilityInfo = context->GetAbilityInfo();
    }
    NativeValue* objValue = CreateJsExtensionContext(engine, context, abilityInfo);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsUIExtensionContext> jsContext = std::make_unique<JsUIExtensionContext>(context);
    object->SetNativePointer(jsContext.release(), Finalizer, nullptr);

    // make handler
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());

    const char *moduleName = "JsUIExtensionContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, StartAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, StartAbilityForResult);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);

    return objValue;
}

bool JsUIExtensionContext::CheckStartAbilityInputParam(NativeEngine& engine, NativeCallbackInfo& info,
    AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const
{
    if (info.argc < ARGC_ONE) {
        return false;
    }
    unwrapArgc = ARGC_ZERO;
    // Check input want
    if (!CheckWantParam(engine, info.argv[INDEX_ZERO], want)) {
        return false;
    }
    ++unwrapArgc;
    if (info.argc > ARGC_ONE && info.argv[1]->TypeOf() == NATIVE_OBJECT) {
        AppExecFwk::UnwrapStartOptions(reinterpret_cast<napi_env>(&engine),
            reinterpret_cast<napi_value>(info.argv[1]), startOptions);
        unwrapArgc++;
    }
    return true;
}

bool JsUIExtensionContext::CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const
{
    if (!OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(value), want)) {
        HILOG_ERROR("The input want is invalid.");
        return false;
    }
    HILOG_DEBUG("UnwrapWant, BundleName: %{public}s, AbilityName: %{public}s.", want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    return true;
}

bool JsUIExtensionContext::UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("UnWrapWant argv == nullptr!");
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(argv), want);
}

bool JsUIExtensionContext::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode,
    AAFwk::Want& want)
{
    if (argv == nullptr) {
        HILOG_WARN("UnWrapAbilityResult argv == nullptr!");
        return false;
    }
    if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of abilityResult!");
        return false;
    }
    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    NativeValue* jResultCode = jObj->GetProperty("resultCode");
    if (jResultCode == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jResultCode == nullptr!");
        return false;
    }
    if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_WARN("UnWrapAbilityResult invalid type of resultCode!");
        return false;
    }
    resultCode = int64_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
    NativeValue* jWant = jObj->GetProperty("want");
    if (jWant == nullptr) {
        HILOG_WARN("UnWrapAbilityResult jWant == nullptr!");
        return false;
    }
    if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_WARN("UnWrapAbilityResult invalid type of want!");
        return false;
    }
    return JsUIExtensionContext::UnWrapWant(engine, jWant, want);
}

NativeValue *JsUIExtensionContext::WrapAbilityResult(
    NativeEngine &engine, const int &resultCode, const AAFwk::Want &want)
{
    NativeValue *jAbilityResult = engine.CreateObject();
    NativeObject *abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", JsUIExtensionContext::WrapWant(engine, want));
    return jAbilityResult;
}

NativeValue *JsUIExtensionContext::WrapWant(NativeEngine &engine, const AAFwk::Want &want)
{
    return reinterpret_cast<NativeValue *>(AppExecFwk::WrapWant(reinterpret_cast<napi_env>(&engine), want));
}
}  // namespace AbilityRuntime
}  // namespace OHOS
