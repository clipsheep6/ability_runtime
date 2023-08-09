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
    static NativeValue* StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* ConnectAbility(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* DisconnectAbility(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    virtual NativeValue* OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info);
    virtual NativeValue* OnTerminateSelf(NativeEngine& engine, const NativeCallbackInfo& info);
    virtual NativeValue* OnTerminateSelfWithResult(NativeEngine& engine, const NativeCallbackInfo& info);
    virtual NativeValue* OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info);
    virtual NativeValue* OnConnectAbility(NativeEngine& engine, NativeCallbackInfo& info);
    virtual NativeValue* OnDisconnectAbility(NativeEngine& engine, NativeCallbackInfo& info);

private:
    std::weak_ptr<UIExtensionContext> context_;
    int curRequestCode_ = 0;

    bool CheckStartAbilityInputParam(NativeEngine& engine, NativeCallbackInfo& info, AAFwk::Want& want,
        AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
    bool CheckWantParam(NativeEngine& engine, NativeValue* value, AAFwk::Want& want) const;
    static bool UnWrapWant(NativeEngine& engine, NativeValue* argv, AAFwk::Want& want);
    static bool UnWrapAbilityResult(NativeEngine& engine, NativeValue* argv, int& resultCode, AAFwk::Want& want);
    static NativeValue* WrapAbilityResult(NativeEngine& engine, const int& resultCode, const AAFwk::Want& want);
    static NativeValue* WrapWant(NativeEngine& engine, const AAFwk::Want& want);
};

class JSUIExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSUIExtensionConnection(NativeEngine& engine);
    ~JSUIExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId();
private:
    NativeEngine& engine_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_UI_EXTENSION_CONTEXT_H
