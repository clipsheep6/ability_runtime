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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_OVERLAY_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_OVERLAY_MANAGER_H

#include <vector>
#include "ability_info.h"
#include "application_info.h"
#include "want.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "foundation/bundlemanager/bundle_framework/interfaces/inner_api/appexecfwk_core/include/overlay/overlay_manager_interface.h"

namespace OHOS {
namespace AppExecFwk {
class OverlayManagerProxy : public IRemoteProxy<IOverlayManager> {
public:
    explicit OverlayManagerProxy(const sptr<IRemoteObject> &object);
    virtual ~OverlayManagerProxy()
    {}

    virtual ErrCode GetTargetOverlayModuleInfo(const std::string &targetModuleName,
        std::vector<OverlayModuleInfo> &overlayModuleInfos, int32_t userId = Constants::UNSPECIFIED_USERID) override;
};

class OverlayManagerHost : public IRemoteStub<IOverlayManager> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_OVERLAY_MANAGER_H
