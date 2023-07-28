/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_CONNECTION_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_CONNECTION_H


#include <algorithm>
#include <memory>
#include <native_engine/native_value.h>

#include "ability_connect_callback.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/ability/ability_runtime/ability_context.h"
#include "js_free_install_observer.h"
#include "js_runtime.h"
#include "event_handler.h"

namespace OHOS {
namespace AbilityRuntime {
struct ConnectCallback {
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

class JSAbilityConnection : public AbilityConnectCallback {
public:
    explicit JSAbilityConnection(NativeEngine& engine);
    ~JSAbilityConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(NativeValue* jsConnectionObject);
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    void SetEventHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
    {
        handler_ = handler;
    }
private:
    NativeValue* ConvertElement(const AppExecFwk::ElementName &element);
    NativeEngine& engine_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
    std::shared_ptr<AppExecFwk::EventHandler> handler_;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct KeyCompare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

static std::map<ConnectionKey, sptr<JSAbilityConnection>, KeyCompare> abilityConnects_;
static int64_t g_serialNumber = 0;

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_CONNECTION_H