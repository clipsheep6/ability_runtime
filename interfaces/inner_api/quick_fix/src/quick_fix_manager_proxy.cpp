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

#include "quick_fix_manager_proxy.h"

#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
QuickFixManagerProxy::QuickFixManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IQuickFixManager>(impl) {}

void QuickFixManagerProxy::ApplyQuickFix(const std::vector<std::string> &quickFixFiles)
{
    HILOG_DEBUG("ApplyQuickFix is called.");

    MessageParcel data;
    if (!data.WriteInterfaceToken(IQuickFixManager::GetDescriptor())) {
        HILOG_ERROR("Write interface token failed.");
        return;
    }

    if (!data.WriteParcelable(&quickFixFiles)) {
        HILOG_ERROR("Write quick fix files failed.");
        return;
    }

    MessageParcel reply;
    MessageOption option;
    int error = Remote()->SendRequest(QuickFixMgrCmd::ON_APPLY_QUICK_FIX, data, reply, option);
    if (error != ERR_OK) {
        HILOG_ERROR("SendRequest fail, error: %{public}d", error);
        return;
    }
}
}  // namespace AAFwk
}  // namespace OHOS
