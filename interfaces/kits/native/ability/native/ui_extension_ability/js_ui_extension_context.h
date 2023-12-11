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
struct NapiCallbackInfo;
class JsEmbeddableUIAbilityContext;

class JsUIExtensionContext {
public:
    explicit JsUIExtensionContext(const std::shared_ptr<UIExtensionContext>& context) : context_(context) {}
    virtual ~JsUIExtensionContext() = default;
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value CreateJsUIExtensionContext(napi_env env, std::shared_ptr<UIExtensionContext> context);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);

protected:
    virtual napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnTerminateSelf(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnTerminateSelfWithResult(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnStartAbilityForResult(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    virtual napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);

private:
    std::weak_ptr<UIExtensionContext> context_;
    friend class JsEmbeddableUIAbilityContext;

    bool CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want,
        AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
};

class JSUIExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSUIExtensionConnection(napi_env env);
    ~JSUIExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId();
private:
    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct UIExtensionConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const UIExtensionConnectionKey &key1, const UIExtensionConnectionKey &key2) const
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
