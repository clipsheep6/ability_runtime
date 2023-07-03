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

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_resource_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
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
    return nullptr;
}

NativeValue *JsAbilityContext::TerminateSelfWithResult(NativeEngine *engine, NativeCallbackInfo *info)
{
    return nullptr;
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
    return nullptr;
}

NativeValue *CreateJsAbilityContext(NativeEngine &engine, const std::shared_ptr<AbilityContext> &context)
{
    NativeValue *objValue = CreateJsBaseContext(engine, context);
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>();
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    auto resourceManager = context->GetResourceManager();
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
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
}  // namespace AbilityRuntime
}  // namespace OHOS
