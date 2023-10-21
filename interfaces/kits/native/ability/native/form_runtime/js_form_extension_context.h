/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_FORM_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_FORM_EXTENSION_CONTEXT_H

#include <memory>

#include "form_extension_context.h"
#include "ability_connect_callback.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsFormExtensionContext(napi_env env, std::shared_ptr<FormExtensionContext> context);

class JSFormExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSFormExtensionConnection(napi_env env);
    ~JSFormExtensionConnection();
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
    napi_env env_;
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
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_FORM_EXTENSION_CONTEXT_H
