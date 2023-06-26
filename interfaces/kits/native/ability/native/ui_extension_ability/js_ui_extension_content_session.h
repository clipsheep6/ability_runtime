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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H

#include "native_engine/native_engine.h"
#include "session_info.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {
class JsUIExtensionContentSession {
public:
    JsUIExtensionContentSession(sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow);
    virtual ~JsUIExtensionContentSession() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateJsUIExtensionContentSession(NativeEngine& engine,
        sptr<AAFwk::SessionInfo> sessionInfo, sptr<Rosen::Window> uiWindow);

    static NativeValue* TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SendData(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetReceiveDataCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* LoadContent(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    NativeValue* OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSendData(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetReceiveDataCallback(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info);

    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want);
private:
    sptr<AAFwk::SessionInfo> sessionInfo_;
    sptr<Rosen::Window> uiWindow_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTENT_SESSION_H
