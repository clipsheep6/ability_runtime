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

#include "js_ui_extension_content_session.h"

#include "ability_manager_client.h"
#include "event_handler.h"
#include "hilog_wrapper.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_context.h"
#include "napi_common_want.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
} // namespace

JsUIExtensionContentSession::JsUIExtensionContentSession(
    NativeEngine& engine, sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : engine_(engine), sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

void JsUIExtensionContentSession::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_DEBUG("JsUIExtensionContentSession Finalizer is called");
    std::unique_ptr<JsUIExtensionContentSession>(static_cast<JsUIExtensionContentSession*>(data));
}

NativeValue *JsUIExtensionContentSession::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SendData(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSendData(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetReceiveDataCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetReceiveDataCallback(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetWindowBackgroundColor(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColor(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::SetWindowPrivacyMode(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsUIExtensionContentSession* me = CheckParamsAndGetThis<JsUIExtensionContentSession>(engine, info);
    return (me != nullptr) ? me->OnSetWindowPrivacyMode(*engine, *info) : nullptr;
}

NativeValue *JsUIExtensionContentSession::OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    AsyncTask::CompleteCallback complete =
        [sessionInfo = sessionInfo_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (sessionInfo == nullptr) {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsUIExtensionContentSession::OnTerminateSelf",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    int resultCode = 0;
    AAFwk::Want want;
    if (!UnWrapAbilityResult(engine, info.argv[INDEX_ZERO], resultCode, want)) {
        HILOG_ERROR("OnTerminateSelfWithResult Failed to parse ability result!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, sessionInfo = sessionInfo_, want, resultCode](NativeEngine& engine,
            AsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->TransferAbilityResult(resultCode, want);
            if (ret != Rosen::WMError::WM_OK) {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto errorCode = AAFwk::AbilityManagerClient::GetInstance()->TerminateUIExtensionAbility(sessionInfo);
            if (errorCode == 0) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsErrorByNativeErr(engine, errorCode));
            }
        };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsUIExtensionContentSession::OnTerminateSelfWithResult",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::OnSendData(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    AAFwk::WantParams params;
    if (!AppExecFwk::UnwrapWantParams(reinterpret_cast<napi_env>(&engine),
        reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), params)) {
        HILOG_ERROR("OnSendData Failed to parse param!");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }

    Rosen::WMError ret = uiWindow_->TransferExtensionData(params);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("TransferExtensionData success");
    } else {
        HILOG_ERROR("TransferExtensionData failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetReceiveDataCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    if (info.argc < ARGC_ONE || info.argv[INDEX_ZERO]->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (!isRegistered) {
        if (uiWindow_ == nullptr) {
            HILOG_ERROR("uiWindow_ is nullptr");
            ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
            return engine.CreateUndefined();
        }
        receiveDataCallback_ = std::make_shared<CallbackWrapper>();
        std::weak_ptr<CallbackWrapper> weakCallback(receiveDataCallback_);
        auto handler = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::GetMainEventRunner());
        uiWindow_->RegisterTransferComponentDataListener([&engine = engine_, handler, weakCallback](
            const AAFwk::WantParams& wantParams) {
            if (handler) {
                handler->PostTask([&engine, weakCallback, wantParams]() {
                    JsUIExtensionContentSession::CallReceiveDataCallback(engine, weakCallback, wantParams);
                });
            }
        });
        isRegistered = true;
    }

    NativeValue* callback = info.argv[INDEX_ZERO];
    if (receiveDataCallback_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    receiveDataCallback_->ResetCallback(std::shared_ptr<NativeReference>(engine.CreateReference(callback, 1)));
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    std::string contextPath;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], contextPath)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }
    HILOG_DEBUG("contextPath: %{public}s", contextPath.c_str());
    NativeValue* storage = nullptr;
    if (info.argc > ARGC_ONE && info.argv[INDEX_ONE]->TypeOf() == NATIVE_OBJECT) {
        storage = info.argv[INDEX_ONE];
    }
    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    Rosen::WMError ret = uiWindow_->SetUIContent(contextPath, &engine, storage);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("SetUIContent success");
    } else {
        HILOG_ERROR("SetUIContent failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetWindowBackgroundColor(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    std::string color;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], color)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    if (uiWindow_ == nullptr) {
        HILOG_ERROR("uiWindow_ is nullptr");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
        return engine.CreateUndefined();
    }
    Rosen::WMError ret = uiWindow_->SetBackgroundColor(color);
    if (ret == Rosen::WMError::WM_OK) {
        HILOG_DEBUG("SetBackgroundColor success");
    } else {
        HILOG_ERROR("SetBackgroundColor failed, ret=%{public}d", ret);
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INNER);
    }
    return engine.CreateUndefined();
}

NativeValue *JsUIExtensionContentSession::OnSetWindowPrivacyMode(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    bool isPrivacyMode = false;
    if (info.argc < ARGC_ONE || !ConvertFromJsValue(engine, info.argv[INDEX_ZERO], isPrivacyMode)) {
        HILOG_ERROR("invalid param");
        ThrowError(engine, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return engine.CreateUndefined();
    }

    AsyncTask::CompleteCallback complete =
        [uiWindow = uiWindow_, isPrivacyMode](NativeEngine& engine, AsyncTask& task, int32_t status) {
            if (uiWindow == nullptr) {
                HILOG_ERROR("uiWindow is nullptr");
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
                return;
            }
            auto ret = uiWindow->SetPrivacyMode(isPrivacyMode);
            if (ret == Rosen::WMError::WM_OK) {
                task.ResolveWithNoError(engine, engine.CreateUndefined());
            } else {
                task.Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
            }
        };
    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsUIExtensionContentSession::OnSetWindowPrivacyMode",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JsUIExtensionContentSession::CreateJsUIExtensionContentSession(NativeEngine& engine,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    HILOG_DEBUG("begin");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(engine, sessionInfo, uiWindow);
    object->SetNativePointer(jsSession.release(), Finalizer, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "sendData", moduleName, SendData);
    BindNativeFunction(engine, *object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(engine, *object, "loadContent", moduleName, LoadContent);
    BindNativeFunction(engine, *object, "setWindowBackgroundColor", moduleName, SetWindowBackgroundColor);
    BindNativeFunction(engine, *object, "setWindowPrivacyMode", moduleName, SetWindowPrivacyMode);
    return objValue;
}

void JsUIExtensionContentSession::CallReceiveDataCallback(NativeEngine& engine,
    std::weak_ptr<CallbackWrapper> weakCallback, const AAFwk::WantParams& wantParams)
{
    auto cbWrapper = weakCallback.lock();
    if (cbWrapper == nullptr) {
        HILOG_WARN("cbWrapper is nullptr");
        return;
    }
    auto callback = cbWrapper->GetCallback();
    if (callback == nullptr) {
        HILOG_WARN("callback is nullptr");
        return;
    }
    NativeValue* method = callback->Get();
    if (method == nullptr) {
        HILOG_WARN("method is nullptr");
        return;
    }
    HandleScope handleScope(engine);
    NativeValue* nativeWantParams = AppExecFwk::CreateJsWantParams(engine, wantParams);
    if (nativeWantParams == nullptr) {
        HILOG_ERROR("nativeWantParams is nullptr");
        return;
    }
    NativeValue* argv[] = {nativeWantParams};
    engine.CallFunction(engine.GetGlobal(), method, argv, ARGC_ONE);
}

bool JsUIExtensionContentSession::UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode,
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
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jWant), want);
}
}  // namespace AbilityRuntime
}  // namespace OHOS