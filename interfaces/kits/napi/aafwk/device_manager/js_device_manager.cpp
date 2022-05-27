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

#include "js_device_manager.h"

#include <memory>
#include "hilog_wrapper.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using namespace OHOS::AAFwk;

void JsDeviceManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsDeviceManager::Finalizer is called");
    std::unique_ptr<JsDeviceManager>(static_cast<JsDeviceManager*>(data));
}

NativeValue* JsDeviceManager::Register(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue* JsDeviceManager::Unregister(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue* JsDeviceManager::RegisterDeviceStatusCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue* JsDeviceManager::UnregisterDeviceStatusCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue *JsDeviceManager::UpdateConnectStatus(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue *JsDeviceManager::StartDeviceManager(NativeEngine *engine, NativeCallbackInfo *info)
{
    HILOG_INFO("%{public}s called.", __func__);
    return engine->CreateUndefined();
}

NativeValue* JsDeviceManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("%{public}s called.", __func__);
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("%{public}s Invalid input parameters", __func__);
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("%{public}s convertNativeValueTo result is nullptr.", __func__);
        return nullptr;
    }

    std::unique_ptr<JsDeviceManager> jsDeviceManager = std::make_unique<JsDeviceManager>();
    object->SetNativePointer(jsDeviceManager.release(), JsDeviceManager::Finalizer, nullptr);

    BindNativeFunction(*engine, *object, "register", JsDeviceManager::Register);
    BindNativeFunction(*engine, *object, "unregister", JsDeviceManager::Unregister);
    BindNativeFunction(*engine, *object, "on", JsDeviceManager::RegisterDeviceStatusCallback);
    BindNativeFunction(*engine, *object, "off", JsDeviceManager::UnregisterDeviceStatusCallback);
    BindNativeFunction(*engine, *object, "updateConnectStatus", JsDeviceManager::UpdateConnectStatus);
    BindNativeFunction(*engine, *object, "startDeviceManager", JsDeviceManager::StartDeviceManager);

    HILOG_INFO("%{public}s called end.", __func__);
    return engine->CreateUndefined();
}
}  // namespace AbilityRuntime
}  // namespace OHOS