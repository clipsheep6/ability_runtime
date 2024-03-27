/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
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

JsExtensionCommon::~JsExtensionCommon()
{
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsExtensionCommon::OnConfigurationUpdated(const std::shared_ptr<AppExecFwk::Configuration> &fullConfig)
{
    TAG_LOGI(AAFwkTag::EXT, "%{public}s called.", __func__);
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::EXT, "invalid configuration.");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
}

void JsExtensionCommon::OnMemoryLevel(int level)
{
    TAG_LOGD(AAFwkTag::EXT, "%{public}s called.", __func__);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_.GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to get js instance object");
        return;
    }

    napi_value jslevel = CreateJsValue(env, level);
    napi_value argv[] = {
        jslevel,
    };
    CallObjectMethod("onMemoryLevel", argv, ArraySize(argv));
}

napi_value JsExtensionCommon::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::EXT, "name: %{public}s", name);

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_.GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to get js instance object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, obj, napi_function)) {
        TAG_LOGE(AAFwkTag::EXT, "Failed to get '%{public}s' from js object", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::EXT, "(%{public}s), success", name);
    napi_value result = nullptr;
    napi_call_function(env, obj, method, argc, argv, &result);
    return result;
}
}
}
