/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_application.h"

#include "ability_runtime_error_util.h"
#include "hilog_tag_wrapper.h"
#include "js_application_context_utils.h"

namespace OHOS {
namespace AbilityRuntime {
void JsApplication::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called.");
    std::unique_ptr<JsApplication>(static_cast<JsApplication *>(data));
}

napi_value JsApplication::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsApplication, OnGetApplicationContext);
}

napi_value JsApplication::OnGetApplicationContext(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called.");
    napi_value value = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Mandatory parameters are left unspecified.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Mandatory parameters are left unspecified.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    return object;
}

napi_value ApplicationInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Called.");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Env or exportObj is nullptr.");
        return nullptr;
    }

    auto jsApplication = std::make_unique<JsApplication>();
    napi_wrap(env, exportObj, jsApplication.release(), JsApplication::Finalizer, nullptr, nullptr);

    const char *moduleName = "application";
    BindNativeFunction(env, exportObj, "getApplicationContext", moduleName,
        JsApplication::GetApplicationContext);
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS