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

    object->SetProperty("WantAgentFlags", WantAgentFlagsInit(engine));
    object->SetProperty("OperationType", WantAgentOperationTypeInit(engine));

    HILOG_DEBUG("JsNapiWantAgentInit BindNativeFunction called");
    const char *moduleName = "JsWantAgent";
    BindNativeFunction(*engine, *object, "equal", moduleName, JsWantAgent::Equal);
    BindNativeFunction(*engine, *object, "getBundleName", moduleName, JsWantAgent::GetBundleName);
    BindNativeFunction(*engine, *object, "getUid", moduleName, JsWantAgent::GetUid);
    BindNativeFunction(*engine, *object, "cancel", moduleName, JsWantAgent::Cancel);
    BindNativeFunction(*engine, *object, "trigger", moduleName, JsWantAgent::NapiTrigger);
    BindNativeFunction(*engine, *object, "getWant", moduleName, JsWantAgent::NapiGetWant);
    BindNativeFunction(*engine, *object, "getWantAgent", moduleName, JsWantAgent::NapiGetWantAgent);
    BindNativeFunction(*engine, *object, "getOperationType", moduleName, JsWantAgent::NapiGetOperationType);
    HILOG_DEBUG("JsNapiWantAgentInit end");
    return exportObj;
}
EXTERN_C_END

extern "C" __attribute__((constructor))
void NAPI_app_ability_WantAgent_AutoRegister()
{
    auto moduleManager = NativeModuleManager::GetInstance();
    NativeModule newModuleInfo = {
        .name = "app.ability.wantAgent",
        .fileName = "app/ability/wantagent_napi.so/want_agent.js",
        .registerCallback = OHOS::JsNapiWantAgentInit,
    };

    moduleManager->Register(&newModuleInfo);
}
}  // namespace OHOS
