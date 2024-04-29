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
#ifndef OHOS_ABILITY_RUNTIME_CONTINUATION_DEVICE_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_CONTINUATION_DEVICE_CALLBACK_PROXY_H
#include <memory>
#include "connect_callback_stub.h"
#include "continuation_device_callback_interface.h"

namespace OHOS {
namespace AppExecFwk {
class ContinuationDeviceCallbackProxy : public ConnectCallbackStub {
public:
    /**
     * A constructor used to create a {@link ohos.aafwk.ability.continuation.ContinuationDeviceCallbackProxy} instance.
     */
    explicit ContinuationDeviceCallbackProxy(std::shared_ptr<IContinuationDeviceCallback> &callback);
    /**
     * A destructor used to release a {@link ohos.aafwk.ability.continuation.ContinuationDeviceCallbackProxy} instance.
     */
    virtual ~ContinuationDeviceCallbackProxy();

    /**
     * @brief After the connection of continuationdevice is completed, call back the function.
     * @param deviceId indicates the continuation device ID.
     * @param deviceType indicators he continuation device deviceType.
     * @return none.
     */
    virtual void Connect(const std::string &deviceId, const std::string &deviceType) override;

    /**
     * @brief When the continuationdevice is disconnected, call back the function.
     * @param deviceId indicates the continuation device ID.
     * @param deviceType indicators he continuation device deviceType.
     * @return none.
     */
    virtual void Disconnect(const std::string &deviceId) override;

private:
    std::weak_ptr<IContinuationDeviceCallback> callback_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
