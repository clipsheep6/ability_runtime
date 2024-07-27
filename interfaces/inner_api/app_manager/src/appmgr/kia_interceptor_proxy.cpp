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

#include "kia_interceptor_proxy.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool KiaInterceptorProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(KiaInterceptorProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::APPMGR, "write interface token failed");
        return false;
    }
    return true;
}

int KiaInterceptorProxy::OnIntercept(AAFwk::Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!WriteInterfaceToken(data)) {
        return ERR_INVALID_VALUE;
    }
    data.WriteParcelable(&want);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "Remote() is NULL");
        return ERR_INVALID_VALUE;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(IKiaInterceptor::KIA_INTERCEPTOR_ON_INTERCEPT),
        data, reply, option);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "SendRequest is failed, error code: %{public}d.", ret);
        return ret;
    }
    int resultCode = reply.ReadInt32();
    if (resultCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "OnIntercept failed, resultCode=%{public}d.", resultCode);
        return resultCode;
    }
    sptr<AAFwk::Want> resultWant = reply.ReadParcelable<AAFwk::Want>();
    if (resultWant == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "resultWant is nullptr.");
        return ERR_INVALID_VALUE;
    }
    want = *resultWant;
    return resultCode;
}
}  // namespace AppExecFwk
}  // namespace OHOS
