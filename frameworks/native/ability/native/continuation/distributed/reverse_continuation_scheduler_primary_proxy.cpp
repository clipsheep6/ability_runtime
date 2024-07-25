
/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "reverse_continuation_scheduler_primary_proxy.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerPrimaryProxy::ReverseContinuationSchedulerPrimaryProxy(
    const sptr<IRemoteObject> &remoteObject)
    : IRemoteProxy<IReverseContinuationSchedulerPrimary>(remoteObject)
{}

/**
 * @brief Replica call this method when it terminated.
 */
void ReverseContinuationSchedulerPrimaryProxy::NotifyReplicaTerminated()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerPrimaryProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "write interface token failed");
        return;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "Remote is nullptr");
        return;
    }
    if (!remoteObject->SendRequest(
        IReverseContinuationSchedulerPrimary::NOTIFY_REPLICA_TERMINATED, data, reply, option)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "SendRequest failed");
        return;
    }
}

/**
 * @brief Replica call this method to notify primary go on.
 *
 * @param want Contains data to be restore.
 * @return True if success, otherwise false.
 */
bool ReverseContinuationSchedulerPrimaryProxy::ContinuationBack(const AAFwk::Want &want)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerPrimaryProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "write interface token failed");
        return false;
    }
    if (!data.WriteParcelable(&want)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "failed to write want");
        return false;
    }
    sptr<IRemoteObject> remoteObject = Remote();
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "Remote is nullptr");
        return false;
    }
    if (!remoteObject->SendRequest(IReverseContinuationSchedulerPrimary::CONTINUATION_BACK, data, reply, option)) {
        TAG_LOGE(AAFwkTag::CONTINUATION,
            "SendRequest failed");
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
