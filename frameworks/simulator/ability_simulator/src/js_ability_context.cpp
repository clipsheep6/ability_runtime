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

#include "js_ability_context.h"

#include "ability_business_error.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_converter.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
}
void JsAbilityContext::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_DEBUG("called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

NativeValue *JsAbilityContext::StartAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartAbilityAsCaller(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartRecentAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartAbilityByCall(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartAbilityForResult(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartAbilityForResultWithAccount(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartServiceExtensionAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StartServiceExtensionAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StopServiceExtensionAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::StopServiceExtensionAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::ConnectAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::ConnectAbilityWithAccount(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::DisconnectAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::TerminateSelf(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityContext *me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue *JsAbilityContext::OnTerminateSelf(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("TerminateSelf");
    auto abilityContext = context_.lock();
    if (abilityContext == nullptr) {
        return nullptr;
    }
    abilityContext->SetTerminating(true);

    NativeValue *lastParam = (info.argc > ARGC_ZERO) ? info.argv[ARGC_ZERO] : nullptr;
    NativeValue *result = nullptr;
    auto task = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    if (task == nullptr) {
        return nullptr;
    }

    auto errcode = abilityContext->TerminateSelf();
    if (errcode == 0) {
        task->Resolve(engine, engine.CreateUndefined());
    } else {
        task->Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
    }

    return result;
}

NativeValue *JsAbilityContext::TerminateSelfWithResult(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityContext *me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue *JsAbilityContext::OnTerminateSelfWithResult(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("called.");
    auto abilityContext = context_.lock();
    if (abilityContext == nullptr) {
        return nullptr;
    }
    abilityContext->SetTerminating(true);

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    NativeValue *result = nullptr;
    auto task = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    if (task == nullptr) {
        return nullptr;
    }

    auto errcode = abilityContext->TerminateSelf();
    if (errcode == 0) {
        task->Resolve(engine, engine.CreateUndefined());
    } else {
        task->Reject(engine, CreateJsErrorByNativeErr(engine, errcode));
    }

    return result;
}

NativeValue *JsAbilityContext::RestoreWindowStage(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::RequestDialogService(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
}

NativeValue *JsAbilityContext::IsTerminating(NativeEngine *engine, NativeCallbackInfo *info)
{
    JsAbilityContext *me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnIsTerminating(*engine, *info) : nullptr;
}

NativeValue *JsAbilityContext::OnIsTerminating(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_DEBUG("IsTerminating");
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_ERROR("OnIsTerminating context is nullptr");
        return engine.CreateUndefined();
    }
    return engine.CreateBoolean(context->IsTerminating());
}

NativeValue *CreateJsErrorByNativeErr(NativeEngine &engine, int32_t err, const std::string &permission)
{
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty()) ?
        GetNoPermissionErrorMsg(permission) : GetErrorMsg(errCode);
    return CreateJsError(engine, static_cast<int32_t>(errCode), errMsg);
}

void JsAbilityContext::ConfigurationUpdated(NativeEngine *engine, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_DEBUG("called.");
    if (jsContext == nullptr || config == nullptr) {
        HILOG_ERROR("jsContext is nullptr.");
        return;
    }

    NativeValue *value = jsContext->Get();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    NativeValue *method = object->GetProperty("onUpdateConfiguration");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    NativeValue *argv[] = { CreateJsConfiguration(*engine, *config) };
    engine->CallFunction(value, method, argv, 1);
}

NativeValue *CreateJsAbilityContext(NativeEngine &engine, const std::shared_ptr<AbilityContext> &context)
{
    NativeValue *objValue = CreateJsBaseContext(engine, context);
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
    }

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        object->SetProperty("abilityInfo", CreateJsAbilityInfo(engine, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        object->SetProperty("config", CreateJsConfiguration(engine, *configuration));
    }

    const char *moduleName = "JsAbilityContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(engine, *object, "startAbilityAsCaller", moduleName, JsAbilityContext::StartAbilityAsCaller);
    BindNativeFunction(engine, *object, "startAbilityWithAccount", moduleName,
        JsAbilityContext::StartAbilityWithAccount);
    BindNativeFunction(engine, *object, "startAbilityByCall", moduleName, JsAbilityContext::StartAbilityByCall);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(engine, *object, "startAbilityForResultWithAccount", moduleName,
        JsAbilityContext::StartAbilityForResultWithAccount);
    BindNativeFunction(engine, *object, "startServiceExtensionAbility", moduleName,
        JsAbilityContext::StartServiceExtensionAbility);
    BindNativeFunction(engine, *object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbility", moduleName,
        JsAbilityContext::StopServiceExtensionAbility);
    BindNativeFunction(engine, *object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(engine, *object, "connectAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "connectServiceExtensionAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(engine, *object, "connectAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "connectServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(engine, *object, "disconnectAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(
        engine, *object, "disconnectServiceExtensionAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(engine, *object, "terminateSelfWithResult", moduleName,
        JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(engine, *object, "restoreWindowStage", moduleName, JsAbilityContext::RestoreWindowStage);
    BindNativeFunction(engine, *object, "isTerminating", moduleName, JsAbilityContext::IsTerminating);
    BindNativeFunction(engine, *object, "startRecentAbility", moduleName,
        JsAbilityContext::StartRecentAbility);
    BindNativeFunction(engine, *object, "requestDialogService", moduleName,
        JsAbilityContext::RequestDialogService);

    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
