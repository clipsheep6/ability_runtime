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
#include "mock_overlay_manager.h"

#include "ability_info.h"
#include "application_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
OverlayManagerProxy::OverlayManagerProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IOverlayManager>(object)
{}

ErrCode OverlayManagerProxy::GetTargetOverlayModuleInfo(const std::string &targetModuleName,
    std::vector<OverlayModuleInfo> &overlayModuleInfos, int32_t userId)
{
    return ERR_OK;
}

int OverlayManagerHost::OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
{
    return 0;
}
}  // namespace AppExecFwk
}  // namespace OHOS
