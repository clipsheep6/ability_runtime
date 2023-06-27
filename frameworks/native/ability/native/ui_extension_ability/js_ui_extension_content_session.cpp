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
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
    : sessionInfo_(sessionInfo), uiWindow_(uiWindow) {}

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
    return nullptr;
}

NativeValue *JsUIExtensionContentSession::OnSendData(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    return nullptr;
}

NativeValue *JsUIExtensionContentSession::OnSetReceiveDataCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("called");
    return nullptr;
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
        return engine.CreateNumber(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    }
    Rosen::WMError ret = uiWindow_->SetUIContent(contextPath, &engine, storage);
    if (ret == Rosen::WMError::WM_OK) {
        return engine.CreateNumber(static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
    } else {
        return engine.CreateNumber(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER));
    }
}

NativeValue *JsUIExtensionContentSession::CreateJsUIExtensionContentSession(NativeEngine& engine,
    sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow)
{
    HILOG_DEBUG("begin");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsUIExtensionContentSession> jsSession =
        std::make_unique<JsUIExtensionContentSession>(sessionInfo, uiWindow);
    object->SetNativePointer(jsSession.release(), Finalizer, nullptr);

    const char *moduleName = "JsUIExtensionContentSession";
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName, TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "sendData", moduleName, SendData);
    BindNativeFunction(engine, *object, "setReceiveDataCallback", moduleName, SetReceiveDataCallback);
    BindNativeFunction(engine, *object, "loadContent", moduleName, LoadContent);
    return objValue;
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