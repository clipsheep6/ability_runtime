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

#ifndef OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_CONTEXT_H

#include <memory>

#include "ui_extension_context.h"
#include "js_free_install_observer.h"
#include "native_engine/native_engine.h"
#include "js_ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;

class JsDemoUIExtensionContext : public JsUIExtensionContext {
public:
    explicit JsDemoUIExtensionContext(const std::shared_ptr<UIExtensionContext>& context)
        : JsUIExtensionContext(context) {}
    virtual ~JsDemoUIExtensionContext() = default;
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value TestMethod(napi_env env, napi_callback_info info);
    static napi_value CreateJsDemoUIExtensionContext(napi_env env, std::shared_ptr<UIExtensionContext> context);

protected:
    virtual napi_value OnTestMethod(napi_env env, NapiCallbackInfo& info);
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_DEMO_UI_EXTENSION_CONTEXT_H