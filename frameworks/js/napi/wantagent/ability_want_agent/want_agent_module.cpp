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

#include <pthread.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

#include "napi_want_agent.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "hilog_wrapper.h"
#include "js_runtime_utils.h"

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AbilityRuntime::WantAgent;

namespace OHOS {
EXTERN_C_START
/*
 * function for module exports
 */

napi_value JsNapiWantAgentInit(napi_env env, napi_value exportObj)
{
    HILOG_DEBUG("JsNapiWantAgentInit is called");

    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("env or exportObj nullptr");
        return nullptr;
    }
    if (!CheckTypeForNapiValue(env, exportObj, napi_object)) {
        HILOG_ERROR("object nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWantAgent> jsWantAgent = std::make_unique<JsWantAgent>();
    napi_wrap(env, exportObj, jsWantAgent.release(), JsWantAgent::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "WantAgentFlags", WantAgentFlagsInit(env));
    napi_set_named_property(env, exportObj, "OperationType", WantAgentOperationTypeInit(env));

    HILOG_DEBUG("JsNapiWantAgentInit BindNativeFunction called");
    const char *moduleName = "JsWantAgent";
    BindNativeFunction(env, exportObj, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(env, exportObj, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(env, exportObj, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(env, exportObj, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(env, exportObj, "trigger", moduleName, JsWantAgent::NapiTrigger);
    BindNativeFunction(env, exportObj, "getWant", moduleName, JsWantAgent::NapiGetWant);
    BindNativeFunction(env, exportObj, "getWantAgent", moduleName, JsWantAgent::NapiGetWantAgent);
    BindNativeFunction(env, exportObj, "getOperationType", moduleName, JsWantAgent::NapiGetOperationType);
    HILOG_DEBUG("JsNapiWantAgentInit end");
    return exportObj;
}
EXTERN_C_END

static napi_module _module = {
    .nm_filename = "app/ability/wantagent_napi.so/want_agent.js",
    .nm_modname = "app.ability.wantAgent",
    .nm_register_func = OHOS::JsNapiWantAgentInit,
};

extern "C" __attribute__((constructor))
void NAPI_app_ability_WantAgent_AutoRegister()
{
    napi_module_register(&_module);
}
}  // namespace OHOS
