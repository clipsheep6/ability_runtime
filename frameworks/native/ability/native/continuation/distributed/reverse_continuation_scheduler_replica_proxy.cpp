/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "reverse_continuation_scheduler_replica_proxy.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
ReverseContinuationSchedulerReplicaProxy::ReverseContinuationSchedulerReplicaProxy(
    const sptr<IRemoteObject> &remoteObject)
    : IRemoteProxy<IReverseContinuationSchedulerReplica>(remoteObject)
{}

sptr<IRemoteObject> ReverseContinuationSchedulerReplicaProxy::AsObject()
{
    sptr<IRemoteObject> remoteObject = Remote();

    return remoteObject;
}

void ReverseContinuationSchedulerReplicaProxy::PassPrimary(const sptr<IRemoteObject> &primary)
{
    HILOG_INFO("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::PassPrimary write interface token failed");
        return;
    }
    if (primary) {
        if (!data.WriteBool(true) || !data.WriteRemoteObject(primary)) {
            HILOG_ERROR("Failed to write flag and primary");
            return;
        }
    } else {
        HILOG_DEBUG("primary is nullptr");
        if (!data.WriteBool(false)) {
            HILOG_ERROR("Failed to write flag");
            return;
        }
    }

    int32_t ret = SendTransactCmd(
        static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::PASS_PRIMARY), data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::PassPrimary SendRequest return false");
    }
    HILOG_INFO("%{public}s called end", __func__);
}

bool ReverseContinuationSchedulerReplicaProxy::ReverseContinuation()
{
    HILOG_INFO("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::ReverseContinuation write interface token failed");
        return false;
    }

    if (SendTransactCmd(
        static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::REVERSE_CONTINUATION),
        data,
        reply,
        option) != NO_ERROR) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::ReverseContinuation SendRequest return false");
        return false;
    }
    HILOG_INFO("%{public}s called end", __func__);
    return true;
}

void ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult(int reverseResult)
{
    HILOG_INFO("%{public}s called begin", __func__);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(ReverseContinuationSchedulerReplicaProxy::GetDescriptor())) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult write interface token failed");
        return;
    }
    if (!data.WriteInt32(reverseResult)) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult write parcel flags failed");
        return;
    }
    if (SendTransactCmd(
        static_cast<uint32_t>(IReverseContinuationSchedulerReplica::Message::NOTIFY_REVERSE_RESULT),
        data,
        reply,
        option) != NO_ERROR) {
        HILOG_ERROR("ReverseContinuationSchedulerReplicaProxy::NotifyReverseResult SendRequest return false");
    }
    HILOG_INFO("%{public}s called end", __func__);
}

int32_t ReverseContinuationSchedulerReplicaProxy::SendTransactCmd(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("Remote is nullptr.");
        return ERR_NULL_OBJECT;
    }

    return remote->SendRequest(code, data, reply, option);
}
}  // namespace AppExecFwk
}  // namespace OHOS
