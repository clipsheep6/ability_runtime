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

#include "napi/native_api.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

#ifndef OHOS_ABILITY_RUNTIME_JS_APPLICATION_H
#define OHOS_ABILITY_RUNTIME_JS_APPLICATION_H
namespace OHOS {
namespace AbilityRuntime {
class JsApplication {
public:
    explicit JsApplication()
    {}
    virtual ~JsApplication() = default;

    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);

private:
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo &info);
};

    napi_value ApplicationInit(napi_env env, napi_value exportObj);
} // namespace AbilityRuntime
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_JS_APPLICATION_H