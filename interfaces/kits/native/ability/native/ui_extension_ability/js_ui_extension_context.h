/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H

#include <memory>

#include "ui_extension_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class JsUIExtensionContext {
public:
    explicit JsUIExtensionContext(const std::shared_ptr<UIExtensionContext>& context) : context_(context) {}
    virtual ~JsUIExtensionContext() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateJsUIExtensionContext(NativeEngine& engine, std::shared_ptr<UIExtensionContext> context);

protected:
    virtual NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info);
    virtual NativeValue* OnTerminateSelf(NativeEngine& engine, const NativeCallbackInfo& info);
    virtual NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, const NativeCallbackInfo& info);

private:
    std::weak_ptr<UIExtensionContext> context_;

    bool CheckStartAbilityInputParam(NativeEngine& engine, NativeCallbackInfo& info, AAFwk::Want& want,
        AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
    bool CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const;

    static bool UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want);
    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
