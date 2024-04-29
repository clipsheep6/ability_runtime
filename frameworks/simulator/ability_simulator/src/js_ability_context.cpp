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

#include "js_ability_context.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
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
void JsAbilityContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

napi_value JsAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartAbilityAsCaller(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartRecentAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartAbilityWithAccount(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartAbilityByCall(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartAbilityForResultWithAccount(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StopServiceExtensionAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::ConnectAbilityWithAccount(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnTerminateSelf);
}

napi_value JsAbilityContext::OnTerminateSelf(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "TerminateSelf");
    auto abilityContext = context_.lock();
    if (abilityContext == nullptr) {
        return nullptr;
    }
    abilityContext->SetTerminating(true);

    napi_value lastParam = (info.argc > ARGC_ZERO) ? info.argv[ARGC_ZERO] : nullptr;
    napi_value result = nullptr;
    auto task = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    if (task == nullptr) {
        return nullptr;
    }

    auto errcode = abilityContext->TerminateSelf();
    if (errcode == 0) {
        task->Resolve(env, CreateJsUndefined(env));
    } else {
        task->Reject(env, CreateJsErrorByNativeErr(env, errcode));
    }

    return result;
}

napi_value JsAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnTerminateSelfWithResult);
}

napi_value JsAbilityContext::OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called.");
    auto abilityContext = context_.lock();
    if (abilityContext == nullptr) {
        return nullptr;
    }
    abilityContext->SetTerminating(true);

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    auto task = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    if (task == nullptr) {
        return nullptr;
    }

    auto errcode = abilityContext->TerminateSelf();
    if (errcode == 0) {
        task->Resolve(env, CreateJsUndefined(env));
    } else {
        task->Reject(env, CreateJsErrorByNativeErr(env, errcode));
    }

    return result;
}

napi_value JsAbilityContext::RestoreWindowStage(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::RequestDialogService(napi_env env, napi_callback_info info)
{
    return nullptr;
}

napi_value JsAbilityContext::IsTerminating(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAbilityContext, OnIsTerminating);
}

napi_value JsAbilityContext::OnIsTerminating(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "IsTerminating");
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "OnIsTerminating context is nullptr");
        return CreateJsUndefined(env);
    }
    return CreateJsValue(env, context->IsTerminating());
}

napi_value CreateJsErrorByNativeErr(napi_env env, int32_t err, const std::string &permission)
{
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty()) ?
        GetNoPermissionErrorMsg(permission) : GetErrorMsg(errCode);
    return CreateJsError(env, static_cast<int32_t>(errCode), errMsg);
}

void JsAbilityContext::ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    TAG_LOGD(AAFwkTag::ABILITY_SIM, "called.");
    if (jsContext == nullptr || config == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "jsContext is nullptr.");
        return;
    }

    napi_value value = jsContext->GetNapiValue();
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "value is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, value, "onUpdateConfiguration", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY_SIM, "Failed to get onUpdateConfiguration from object");
        return;
    }

    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_value callResult = nullptr;
    napi_call_function(env, nullptr, method, 1, argv, &callResult);
}

napi_value CreateJsAbilityContext(napi_env env, const std::shared_ptr<AbilityContext> &context)
{
    napi_value object = CreateJsBaseContext(env, context);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    napi_wrap(env, object, jsContext.release(), JsAbilityContext::Finalizer, nullptr, nullptr);

    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        napi_set_named_property(env, object, "resourceManager",
            CreateJsResourceManager(env, resourceManager, context));
    }

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(
            env, object, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(
            env, object, "config", CreateJsConfiguration(env, *configuration));
    }

    const char *moduleName = "JsAbilityContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(env, object, "startAbilityAsCaller", moduleName, JsAbilityContext::StartAbilityAsCaller);
    BindNativeFunction(env, object, "startAbilityWithAccount", moduleName,
        JsAbilityContext::StartAbilityWithAccount);
    BindNativeFunction(env, object, "startAbilityByCall", moduleName, JsAbilityContext::StartAbilityByCall);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(env, object, "startAbilityForResultWithAccount", moduleName,
        JsAbilityContext::StartAbilityForResultWithAccount);
    BindNativeFunction(env, object, "startServiceExtensionAbility", moduleName,
        JsAbilityContext::StartServiceExtensionAbility);
    BindNativeFunction(env, object, "startServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StartServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "stopServiceExtensionAbility", moduleName,
        JsAbilityContext::StopServiceExtensionAbility);
    BindNativeFunction(env, object, "stopServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::StopServiceExtensionAbilityWithAccount);
    BindNativeFunction(env, object, "connectAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(env, object, "connectServiceExtensionAbility", moduleName, JsAbilityContext::ConnectAbility);
    BindNativeFunction(env, object, "connectAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(env, object, "connectServiceExtensionAbilityWithAccount", moduleName,
        JsAbilityContext::ConnectAbilityWithAccount);
    BindNativeFunction(env, object, "disconnectAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(
        env, object, "disconnectServiceExtensionAbility", moduleName, JsAbilityContext::DisconnectAbility);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(env, object, "terminateSelfWithResult", moduleName,
        JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(env, object, "restoreWindowStage", moduleName, JsAbilityContext::RestoreWindowStage);
    BindNativeFunction(env, object, "isTerminating", moduleName, JsAbilityContext::IsTerminating);
    BindNativeFunction(env, object, "startRecentAbility", moduleName,
        JsAbilityContext::StartRecentAbility);
    BindNativeFunction(env, object, "requestDialogService", moduleName,
        JsAbilityContext::RequestDialogService);

    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS
