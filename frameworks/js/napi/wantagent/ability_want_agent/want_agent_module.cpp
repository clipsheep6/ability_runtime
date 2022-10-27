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

NativeValue* JsNapiWantAgentInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_DEBUG("JsNapiWantAgentInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("engine or exportObj null");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("object null");
        return nullptr;
    }

    std::unique_ptr<JsWantAgent> jsWantAgent = std::make_unique<JsWantAgent>();
    object->SetNativePointer(jsWantAgent.release(), JsWantAgent::Finalizer, nullptr);

    HILOG_DEBUG("JsNapiWantAgentInit BindNativeFunction called");
    const char *moduleName = "JsWantAgent";
    BindNativeFunction(*engine, *object, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(*engine, *object, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(*engine, *object, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(*engine, *object, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(*engine, *object, "trigger", moduleName, JsWantAgent::NapiTrigger);
    BindNativeFunction(*engine, *object, "getWant", moduleName, JsWantAgent::NapiGetWant);
    HILOG_DEBUG("JsNapiWantAgentInit end");
    return exportObj;
}

napi_value NapiWantAgentInit(napi_env env, napi_value exports)
{
    HILOG_INFO("napi_moudule Init start...");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getWantAgent", NAPI_GetWantAgent),
        DECLARE_NAPI_FUNCTION("getOperationType", GetOperationType),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    HILOG_INFO("napi_moudule Init end...");
    return reinterpret_cast<napi_value>(JsNapiWantAgentInit(reinterpret_cast<NativeEngine*>(env),
        reinterpret_cast<NativeValue*>(exports)));
}

static napi_value Init(napi_env env, napi_value exports)
{
    NapiWantAgentInit(env, exports);
    WantAgentFlagsInit(env, exports);
    WantAgentOperationTypeInit(env, exports);
    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "app.ability.wantAgent",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
}  // namespace OHOS
