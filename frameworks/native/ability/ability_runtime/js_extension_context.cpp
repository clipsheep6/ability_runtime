/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_extension_context.h"

#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
void JsExtensionContext::ConfigurationUpdated(napi_env env, napi_ref jsContext,
        const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    if (env == nullptr || jsContext == nullptr || config == nullptr) {
        HILOG_ERROR("engine or jsContext or config is nullptr.");
        return;
    }

    napi_value object = nullptr;
    napi_get_reference_value(env, jsContext, &object);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, object, "onUpdateConfiguration", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsExtensionContext call onUpdateConfiguration.");
    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_call_function(env, object, method, 1, argv, nullptr);
}

// to do
void JsExtensionContext::ConfigurationUpdated(NativeEngine* engine, const std::shared_ptr<NativeReference>& jsContext,
    const std::shared_ptr<AppExecFwk::Configuration>& config)
{
    ConfigurationUpdated(reinterpret_cast<napi_env>(engine), reinterpret_cast<napi_ref>(jsContext.get()), config);
}

napi_value CreateJsExtensionContext(napi_env env, const std::shared_ptr<ExtensionContext>& context,
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
{
    if (context == nullptr) {
        HILOG_ERROR("Failed to CreateJsExtensionContext, context is nullptr.");
        return nullptr;
    }
    // to do
    napi_value object = reinterpret_cast<napi_value>(
        CreateJsBaseContext(*reinterpret_cast<NativeEngine*>(env), context));
    if (object == nullptr) {
        HILOG_ERROR("Failed to CreateJsExtensionContext, object is nullptr.");
        return nullptr;
    }
    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        napi_set_named_property(env, object, "config", CreateJsConfiguration(env, *configuration));
    }

    auto hapModuleInfo = context->GetHapModuleInfo();
    if (abilityInfo && hapModuleInfo) {
        auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo& info) {
            HILOG_DEBUG("%{public}s, %{public}s", info.bundleName.c_str(), info.name.c_str());
            return info.bundleName == abilityInfo->bundleName && info.name == abilityInfo->name;
        };
        auto infoIter = std::find_if(
            hapModuleInfo->extensionInfos.begin(), hapModuleInfo->extensionInfos.end(), isExist);
        if (infoIter == hapModuleInfo->extensionInfos.end()) {
            HILOG_ERROR("Set extensionAbilityInfo fail.");
        } else {
            napi_set_named_property(env, object, "extensionAbilityInfo", CreateJsExtensionAbilityInfo(env, *infoIter));
        }
    }

    return object;
}

// to do
NativeValue* CreateJsExtensionContext(NativeEngine& engine, const std::shared_ptr<ExtensionContext>& context,
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
{
    return reinterpret_cast<NativeValue*>(
        CreateJsExtensionContext(reinterpret_cast<napi_env>(&engine), context, abilityInfo));
}
} // namespace AbilityRuntime
} // namespace OHOS
