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

#include "js_ability_stage_context.h"

#include "ability_runtime/context/context.h"
#include "hilog_wrapper.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
void JsAbilityStageContext::ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference> &jsContext,
    const std::shared_ptr<AppExecFwk::Configuration> &config)
{
    HILOG_DEBUG("called");
    if (!jsContext || !config) {
        HILOG_ERROR("jsContext or config is nullptr.");
        return;
    }

    napi_value object = jsContext->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, object, "onUpdateConfiguration", &method);
    if (!method) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_DEBUG("call onUpdateConfiguration");
    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_call_function(env, object, method, 1, argv, nullptr);
}

napi_value CreateJsAbilityStageContext(napi_env env,
    std::shared_ptr<AbilityRuntime::Context> context, DetachCallback detach, NapiAttachCallback attach)
{
    HILOG_DEBUG("called.");
    napi_value objValue = CreateJsBaseContext(env, context);
    if (context == nullptr) {
        return objValue;
    }

    auto configuration = context->GetConfiguration();
    if (configuration != nullptr && CheckTypeForNapiValue(env, objValue, napi_object)) {
        napi_set_named_property(env, objValue, "config", CreateJsConfiguration(env, *configuration));
    }
    return objValue;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
