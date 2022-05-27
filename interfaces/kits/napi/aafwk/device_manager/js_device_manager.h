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

#ifndef OHOS_ABILITY_RUNTIME_DEVICE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_DEVICE_MANAGER_H

#include "native_engine/native_engine.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {

class JsDeviceManager final {
public:
    JsDeviceManager() = default;
    ~JsDeviceManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* Register(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* Unregister(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* RegisterDeviceStatusCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* UnregisterDeviceStatusCallback(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* UpdateConnectStatus(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* StartDeviceManager(NativeEngine* engine, NativeCallbackInfo* info);
};

NativeValue* JsDeviceManagerInit(NativeEngine* engine, NativeValue* exportObj);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_DEVICE_MANAGER_H