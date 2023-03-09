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

#include "dataobs_mgr_stub.h"

#include "string_ex.h"

#include "data_ability_observer_proxy.h"
#include "dataobs_mgr_errors.h"
#include "ipc_skeleton.h"
#include "memory_guard.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {
using Uri = OHOS::Uri;

const DataObsManagerStub::RequestFuncType DataObsManagerStub::HANDLES[TRANS_BUTT] = {
    &DataObsManagerStub::RegisterObserverInner,
    &DataObsManagerStub::UnregisterObserverInner,
    &DataObsManagerStub::NotifyChangeInner,
    &DataObsManagerStub::RegisterObserverExtInner,
    &DataObsManagerStub::UnregisterObserverExtInner,
    &DataObsManagerStub::UnregisterObserverExtALLInner,
    &DataObsManagerStub::NotifyChangeExtInner
};

DataObsManagerStub::DataObsManagerStub() {}

DataObsManagerStub::~DataObsManagerStub() {}

int DataObsManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    MemoryGuard cacheGuard;
    HILOG_INFO("code: %{public}d, flags: %{public}d, callingPid:%{public}d", code, option.GetFlags(),
        IPCSkeleton::GetCallingPid());
    std::u16string descriptor = DataObsManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        HILOG_ERROR("local descriptor is not equal to remote, descriptor: %{public}s, remoteDescriptor: %{public}s",
            CommonUtils::Anonymous(Str16ToStr8(descriptor)).c_str(),
            CommonUtils::Anonymous(Str16ToStr8(remoteDescriptor)).c_str());
        return ERR_INVALID_STATE;
    }

    if (code < TRANS_HEAD || code >= TRANS_BUTT || HANDLES[code] == nullptr) {
        HILOG_ERROR("not support code:%{public}u, BUTT:%{public}d", code, TRANS_BUTT);
        return -1;
    }
    return (this->*HANDLES[code])(data, reply);
}

int DataObsManagerStub::RegisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        HILOG_ERROR("uri is invalid");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    int32_t result = RegisterObserver(uri, observer);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::UnregisterObserverInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        HILOG_ERROR("uri is invalid");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    int32_t result = UnregisterObserver(uri, observer);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int DataObsManagerStub::NotifyChangeInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        HILOG_ERROR("uri is invalid");
        return IPC_STUB_INVALID_DATA_ERR;
    }

    int32_t result = NotifyChange(uri);
    reply.WriteInt32(result);
    return NO_ERROR;
}

int32_t DataObsManagerStub::RegisterObserverExtInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        HILOG_ERROR("uri is invalid");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    bool isDescendants = data.ReadBool();
    reply.WriteInt32(RegisterObserverExt(uri, observer, isDescendants));
    return SUCCESS;
}

int32_t DataObsManagerStub::UnregisterObserverExtInner(MessageParcel &data, MessageParcel &reply)
{
    Uri uri(data.ReadString());
    if (uri.ToString().empty()) {
        HILOG_ERROR("uri is invalid");
        return IPC_STUB_INVALID_DATA_ERR;
    }
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);

    reply.WriteInt32(UnregisterObserverExt(uri, observer));
    return SUCCESS;
}

int32_t DataObsManagerStub::UnregisterObserverExtALLInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    auto observer = remote == nullptr ? nullptr : iface_cast<IDataAbilityObserver>(remote);
    reply.WriteInt32(UnregisterObserverExt(observer));
    return SUCCESS;
}

int32_t DataObsManagerStub::NotifyChangeExtInner(MessageParcel &data, MessageParcel &reply)
{
    ChangeInfo changeInfo;
    if (!ChangeInfo::Unmarshalling(changeInfo, data)) {
        return IPC_STUB_INVALID_DATA_ERR;
    }

    reply.WriteInt32(NotifyChangeExt(changeInfo));
    return SUCCESS;
}
}  // namespace AAFwk
}  // namespace OHOS
