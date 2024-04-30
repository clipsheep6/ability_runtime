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

#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "ui_service_host_interface.h"

#ifndef OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_PROXY_H
#define OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_PROXY_H

namespace OHOS {
namespace AAFwk {

class UIServiceHostProxy : public IRemoteProxy<IUiServiceHost> {
public:
    explicit UIServiceHostProxy(const sptr<IRemoteObject>& impl)
        :IRemoteProxy<IUiServiceHost>(impl) {}
    virtual ~UIServiceHostProxy() {}

    virtual void SendData(OHOS::AAFwk::WantParams &data) override;
};
} // namespace AAFwk
} // namespace OHOS
#endif //OHOS_ABILITY_RUNTIME_UI_SERVICE_HOST_PROXY_H