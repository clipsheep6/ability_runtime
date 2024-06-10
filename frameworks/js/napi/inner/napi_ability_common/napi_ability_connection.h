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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_ABILITY_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_NAPI_ABILITY_CONNECTION_H

#include <map>
#include <stdint.h>

#include "ability_connect_callback_stub.h"
#include "ability_manager_client.h"
#include "js_napi_common_ability.h"
#include "js_runtime_utils.h"
#include "napi_common_error.h"

namespace OHOS {
namespace AppExecFwk {
class JsNapiCommon;
using ConnectRemoveKeyType = JsNapiCommon*;

enum {
    CONNECTION_STATE_DISCONNECTED = -1,

    CONNECTION_STATE_CONNECTED = 0,

    CONNECTION_STATE_CONNECTING = 1
};

struct ConnectionCallback {
    ConnectionCallback(napi_env env, napi_value cbInfo, ConnectRemoveKeyType key)
    {
        this->env = env;
        napi_value jsMethod = nullptr;
        napi_get_named_property(env, cbInfo, "onConnect", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &connectCallbackRef);
        napi_get_named_property(env, cbInfo, "onDisconnect", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &disconnectCallbackRef);
        napi_get_named_property(env, cbInfo, "onFailed", &jsMethod);
        napi_create_reference(env, jsMethod, 1, &failedCallbackRef);
        removeKey = key;
    }
    ConnectionCallback(ConnectionCallback &) = delete;
    ConnectionCallback(ConnectionCallback &&other)
        : env(other.env), connectCallbackRef(other.connectCallbackRef),
        disconnectCallbackRef(other.disconnectCallbackRef), failedCallbackRef(other.failedCallbackRef),
        removeKey(other.removeKey)
    {
        other.env = nullptr;
        other.connectCallbackRef = nullptr;
        other.disconnectCallbackRef = nullptr;
        other.failedCallbackRef = nullptr;
        other.removeKey = nullptr;
    }
    const ConnectionCallback &operator=(ConnectionCallback &) = delete;
    const ConnectionCallback &operator=(ConnectionCallback &&other)
    {
        Reset();
        env = other.env;
        connectCallbackRef = other.connectCallbackRef;
        disconnectCallbackRef = other.disconnectCallbackRef;
        failedCallbackRef = other.failedCallbackRef;
        other.env = nullptr;
        other.connectCallbackRef = nullptr;
        other.disconnectCallbackRef = nullptr;
        other.failedCallbackRef = nullptr;
        other.removeKey = nullptr;
        return *this;
    }
    ~ConnectionCallback()
    {
        Reset();
    }
    void Reset();

    napi_env env = nullptr;
    napi_ref connectCallbackRef = nullptr;
    napi_ref disconnectCallbackRef = nullptr;
    napi_ref failedCallbackRef = nullptr;
    ConnectRemoveKeyType removeKey = nullptr;
};

class NAPIAbilityConnection : public AAFwk::AbilityConnectionStub {
public:
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void AddConnectionCallback(std::shared_ptr<ConnectionCallback> callback);
    void HandleOnAbilityConnectDone(ConnectionCallback &callback, int resultCode);
    void HandleOnAbilityDisconnectDone(ConnectionCallback &callback, int resultCode);
    int GetConnectionState() const;
    void SetConnectionState(int connectionState);
    size_t GetCallbackSize();
    size_t RemoveAllCallbacks(ConnectRemoveKeyType key);

private:
    std::list<std::shared_ptr<ConnectionCallback>> callbacks_;
    AppExecFwk::ElementName element_;
    sptr<IRemoteObject> serviceRemoteObject_ = nullptr;
    int connectionState_ = CONNECTION_STATE_DISCONNECTED;
    mutable std::mutex lock_;
};

struct ConnectionKey {
    Want want;
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
static std::map<ConnectionKey, sptr<NAPIAbilityConnection>, key_compare> connects_;
static std::mutex g_connectionsLock_;
static int64_t serialNumber_ = 0;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_NAPI_ABILITY_CONNECTION_H