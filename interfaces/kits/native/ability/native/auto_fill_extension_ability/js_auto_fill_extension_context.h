/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H

#include <memory>

#include "auto_fill_extension_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
struct NapiCallbackInfo;
class JsAutoFillExtensionContext final {
public:
    explicit JsAutoFillExtensionContext(const std::shared_ptr<AutoFillExtensionContext> &context) : context_(context) {}
    virtual ~JsAutoFillExtensionContext() = default;

    static void Finalizer(napi_env env, void *data, void *hint);
    static napi_value ReloadInModal(napi_env env, napi_callback_info info);
    static napi_value CreateJsAutoFillExtensionContext(
        napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context);
    std::shared_ptr<AutoFillExtensionContext> GetAutoFillExtensionContext()
    {
        return context_.lock();
    }

private:
    napi_value OnReloadInModal(napi_env env, NapiCallbackInfo &info);
    std::weak_ptr<AutoFillExtensionContext> context_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_AUTO_FILL_EXTENSION_CONTEXT_H
