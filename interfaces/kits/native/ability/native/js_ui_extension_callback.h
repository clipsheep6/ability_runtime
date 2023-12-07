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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H

#include <string>
#include "native_engine/native_reference.h"

namespace OHOS {
namespace Ace {
class UIContent;
}
namespace AbilityRuntime {
class JsUIExtensionCallback : public std::enable_shared_from_this<JsUIExtensionCallback> {
public:
    explicit JsUIExtensionCallback(napi_env env) : env_(env) {}
    ~JsUIExtensionCallback();
    void OnError(int32_t number);
    void OnRelease(int32_t code);
    void SetJsCallbackObject(napi_value jsCallbackObject);
    void CallJsError(int32_t number);
    void SetSessionId(int32_t sessionId);
    void SetUIContent(Ace::UIContent* uiContent);
private:
    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> jsCallbackObject_ = nullptr;
    int32_t sessionId_ = 0;
    Ace::UIContent* uiContent_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CALLBACK_H