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

#ifndef OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "ui_extension_context.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
class JSUIExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSUIExtensionConnection(NativeEngine& engine) : engine_(engine) {}
    ~JSUIExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId();
private:
    NativeEngine& engine_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

class JsUIExtensionContext final {
public:
    explicit JsUIExtensionContext(const std::shared_ptr<UIExtensionContext>& context) : context_(context) {}
    ~JsUIExtensionContext() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* StartAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* TerminateAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* ConnectExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* DisconnectExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartUIExtensionAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateJsUIExtensionContext(NativeEngine& engine, std::shared_ptr<UIExtensionContext> context);

private:
    std::weak_ptr<UIExtensionContext> context_;
    NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnTerminateAbility(NativeEngine& engine, const NativeCallbackInfo& info);
    NativeValue* OnConnectExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnDisconnectExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnStartUIExtensionAbility(NativeEngine& engine, NativeCallbackInfo& info);

    bool CheckStartAbilityInputParam(NativeEngine& engine, NativeCallbackInfo& info, AAFwk::Want& want,
        AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
    bool CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const;
    bool CheckConnectionParam(NativeEngine& engine, NativeValue* value, sptr<JSUIExtensionConnection>& connection,
        AAFwk::Want& want) const;
    bool CheckOnDisconnectExtensionAbilityParam(NativeEngine& engine, NativeCallbackInfo& info,
        int64_t& connectId) const;
    void FindConnection(NativeEngine& engine, NativeCallbackInfo& info, AAFwk::Want& want,
        sptr<JSUIExtensionConnection>& connection, int64_t& connectId) const;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
