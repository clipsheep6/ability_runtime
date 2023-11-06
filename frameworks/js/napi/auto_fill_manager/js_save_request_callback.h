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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_SAVE_REQUEST_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_SAVE_REQUEST_CALLBACK_H

#include <chrono>
#include <iremote_object.h>

#include "view_data.h"
#include "save_request_callback_interface.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"

class NativeReference;
namespace OHOS {
namespace AbilityRuntime {
/**
 *
 * @class JsSaveRequestCallback
 */
class JsSaveRequestCallback : public ISaveRequestCallback {
public:
    explicit JsSaveRequestCallback(napi_env env);
    virtual ~JsSaveRequestCallback();
	void Register(napi_value value);
    void UnRegister(napi_value value);
    bool IsCallbacksEmpty();

    void OnSaveRequestSuccess() override;
    void OnSaveRequestFailed() override;

private:
    void JSCallFunction(const std::string &methodName);
    void JSCallFunctionWorker(const std::string &methodName);
    bool IsJsCallbackEquals(std::shared_ptr<NativeReference> callback, napi_value value);

    napi_env env_;
    std::vector<std::shared_ptr<NativeReference>> callbacks_;
    std::mutex mutexlock_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_SAVE_REQUEST_CALLBACK_H
