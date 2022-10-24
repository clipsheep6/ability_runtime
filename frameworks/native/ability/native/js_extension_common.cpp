/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_extension_common.h"

#include "hilog_wrapper.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}

using namespace OHOS::AppExecFwk;

std::shared_ptr<JsExtensionCommon> JsExtensionCommon::Create(JsRuntime &jsRuntime, NativeReference &jsObj,
    const std::shared_ptr<NativeReference> &shellContextRef)
{
    return std::make_shared<JsExtensionCommon>(jsRuntime, jsObj, shellContextRef);
}

JsExtensionCommon::JsExtensionCommon(JsRuntime &jsRuntime, NativeReference &jsObj,
    const std::shared_ptr<NativeReference> &shellContextRef)
    : jsRuntime_(jsRuntime), jsObj_(jsObj), shellContextRef_(shellContextRef) {}

JsExtensionCommon::~JsExtensionCommon() = default;

void JsExtensionCommon::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (!fullConfig) {
        HILOG_ERROR("invalid configuration.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    JsExtensionContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(
        reinterpret_cast<napi_env>(&nativeEngine), *fullConfig);
    NativeValue* jsConfiguration = reinterpret_cast<NativeValue*>(napiConfiguration);
    CallObjectMethod("onConfigurationUpdated", &jsConfiguration, ARGC_ONE);
}

void JsExtensionCommon::OnMemoryLevel(int level)
{
    HILOG_DEBUG("%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto &nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue *value = jsObj_.Get();
    NativeObject *obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get js instance object");
        return;
    }

    NativeValue *jslevel = CreateJsValue(nativeEngine, level);
    NativeValue *argv[] = {
        jslevel,
    };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

NativeValue* JsExtensionCommon::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsExtensionCommon::CallObjectMethod(%{public}s), begin", name);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    NativeValue* value = jsObj_.Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get js instance object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr || method->TypeOf() != NATIVE_FUNCTION) {
        HILOG_ERROR("Failed to get '%{public}s' from js object", name);
        return nullptr;
    }
    HILOG_INFO("JsExtensionCommon::CallFunction(%{public}s), success", name);
    return nativeEngine.CallFunction(value, method, argv, argc);
}
}
}
