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

#ifndef OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H

#include "save_request_callback_interface.h"
#include "native_engine/native_value.h"

class NativeReference;
namespace OHOS {
namespace AbilityRuntime {
using AutoFillManagerFunc = std::function<void(int32_t)>;
class JsSaveRequestCallback : public ISaveRequestCallback {
public:
    JsSaveRequestCallback(napi_env env, int32_t instanceId, AutoFillManagerFunc autoFillManagerFunc);
    virtual ~JsSaveRequestCallback();

    void Register(napi_value value);
    void OnSaveRequestSuccess() override;
    void OnSaveRequestFailed() override;

private:
    void JSCallFunction(const std::string &methodName);
    void JSCallFunctionWorker(const std::string &methodName);
    bool IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value);

    napi_env env_;
    std::shared_ptr<NativeReference> callback_;
    std::mutex callbackMutex_;
    int32_t instanceId_;
    AutoFillManagerFunc autoFillManagerFunc_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_SAVE_REQUEST_CALLBACK_H
