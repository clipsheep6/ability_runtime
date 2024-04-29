/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "session_handler_proxy.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
void SessionHandlerProxy::OnSessionMovedToFront(int32_t sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    if (!data.WriteInterfaceToken(ISessionHandler::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::DEFAULT, "write interface token failed.");
        return;
    }
    if (!data.WriteInt32(sessionId)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "sessionId write failed.");
        return;
    }
    auto remote = Remote();
    if (!remote) {
        TAG_LOGE(AAFwkTag::DEFAULT, "remote object is nullptr.");
        return;
    }
    int32_t ret = remote->SendRequest(ISessionHandler::ON_SESSION_MOVED_TO_FRONT, data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::DEFAULT, "OnSessionMovedToFront fail to Send request, err: %{public}d.", ret);
    }
}
}
}