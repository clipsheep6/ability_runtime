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
void JsExtensionContext::ConfigurationUpdated(NativeEngine* engine, const std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (engine == nullptr || jsContext == nullptr || config == nullptr) {
        HILOG_ERROR("engine or jsContext or config is nullptr.");
        return;
    }

    NativeValue* value = jsContext->Get();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    NativeValue* method = object->GetProperty("onUpdateConfiguration");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsExtensionContext call onUpdateConfiguration.");
    NativeValue* argv[] = {CreateJsConfiguration(*engine, *config)};
    engine->CallFunction(value, method, argv, 1);
}

NativeValue* CreateJsExtensionContext(NativeEngine& engine, const std::shared_ptr<ExtensionContext> &context,
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo)
{
    HILOG_INFO("CreateJsExtensionContext begin");
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    if (context == nullptr) {
        HILOG_ERROR("Failed to CreateJsExtensionContext, context is nullptr.");
        return objValue;
    }
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to CreateJsExtensionContext, object is nullptr.");
        return objValue;
    }
    auto configuration = context->GetConfiguration();
    if (configuration != nullptr) {
        object->SetProperty("config", CreateJsConfiguration(engine, *configuration));
    }

    auto hapModuleInfo = context->GetHapModuleInfo();
    if (abilityInfo && hapModuleInfo) {
        auto isExist = [&abilityInfo](const AppExecFwk::ExtensionAbilityInfo &info) {
            HILOG_INFO("%{public}s, %{public}s", info.bundleName.c_str(), info.name.c_str());
            return info.bundleName == abilityInfo->bundleName && info.name == abilityInfo->name;
        };
        auto infoIter = std::find_if(
            hapModuleInfo->extensionInfos.begin(), hapModuleInfo->extensionInfos.end(), isExist);
        if (infoIter == hapModuleInfo->extensionInfos.end()) {
            HILOG_ERROR("Set extensionAbilityInfo fail.");
        } else {
            object->SetProperty("extensionAbilityInfo", CreateJsExtensionAbilityInfo(engine, *infoIter));
        }
    }

    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
