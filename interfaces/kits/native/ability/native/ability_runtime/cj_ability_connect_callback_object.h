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

#ifndef OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONNECT_CALLBACK_OBJECT_H
#define OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONNECT_CALLBACK_OBJECT_H

#include <cstdint>

#include "ability_connect_callback.h"
#include "cj_element_name_ffi.h"

extern "C" {
struct CJAbilityConnectCallbackFuncs {
    void (*onConnect)(int64_t id, ElementNameHandle elementNameHandle, int64_t remoteObjectId, int32_t resultCode);
    void (*onDisconnect)(int64_t id, ElementNameHandle elementNameHandle, int32_t resultCode);
    void (*release)(int64_t id);
};

#define EXPORT __attribute__((visibility("default")))
EXPORT void RegisterCJAbilityConnectCallbackFuncs(void (*registerFunc)(CJAbilityConnectCallbackFuncs* result));
#undef EXPORT
};

namespace OHOS {
namespace AbilityRuntime {
class CJAbilityConnectCallback : public AbilityConnectCallback {
public:
    explicit CJAbilityConnectCallback(int64_t id) : callbackId_(id) {};
    ~CJAbilityConnectCallback() override;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName& element, const sptr<IRemoteObject>& remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName& element, int resultCode) override;

private:
    int64_t callbackId_ = 0;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CJ_ABILITY_CONNECT_CALLBACK_OBJECT_H
