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

#include "dataobs_mgr_proxy.h"

#include "errors.h"
#include "hilog_wrapper.h"
#include "dataobs_mgr_errors.h"
#include "common_utils.h"

namespace OHOS {
namespace AAFwk {
bool DataObsManagerProxy::WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(DataObsManagerProxy::GetDescriptor())) {
        HILOG_ERROR("failed, write interface token error");
        return false;
    }
    return true;
}

bool DataObsManagerProxy::WriteParam(MessageParcel &data, const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    if (!data.WriteString(uri.ToString())) {
        HILOG_ERROR("failed, write uri error");
        return false;
    }

    if (dataObserver == nullptr) {
        HILOG_ERROR("failed, dataObserver is nullptr");
        return false;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        HILOG_ERROR("failed, write dataObserver error");
        return false;
    }
    return true;
}

int32_t DataObsManagerProxy::RegisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::REGISTER_OBSERVER, data, reply, option);
    if (error != NO_ERROR) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, uri: %{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return error;
    }

    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::UnregisterObserver(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::UNREGISTER_OBSERVER, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, uri: %{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return error;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

int32_t DataObsManagerProxy::NotifyChange(const Uri &uri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!data.WriteString(uri.ToString())) {
        HILOG_ERROR("failed, write uri error, uri: %{public}s", CommonUtils::Anonymous(uri.ToString()).c_str());
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::NOTIFY_CHANGE, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, uri: %{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? res : IPC_ERROR;
}

Status DataObsManagerProxy::RegisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver,
    bool isDescendants)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }

    if (!data.WriteBool(isDescendants)) {
        HILOG_ERROR("failed, write isDescendants error, uri: %{public}s, isDescendants: %{public}d",
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::REGISTER_OBSERVER_EXT, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, uri: %{public}s, isDescendants: %{public}d", error,
            CommonUtils::Anonymous(uri.ToString()).c_str(), isDescendants);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::UnregisterObserverExt(const Uri &uri, sptr<IDataAbilityObserver> dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (!WriteParam(data, uri, dataObserver)) {
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::UNREGISTER_OBSERVER_EXT, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, uri: %{public}s", error,
            CommonUtils::Anonymous(uri.ToString()).c_str());
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::UnregisterObserverExt(sptr<IDataAbilityObserver> dataObserver)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (dataObserver == nullptr) {
        HILOG_ERROR("failed, dataObserver is nullptr");
        return INVALID_PARAM;
    }

    if (!data.WriteRemoteObject(dataObserver->AsObject())) {
        HILOG_ERROR("failed, write dataObserver error");
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::UNREGISTER_OBSERVER_ALL_EXT, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d", error);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}

Status DataObsManagerProxy::NotifyChangeExt(const ChangeInfo &changeInfo)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!WriteInterfaceToken(data)) {
        return IPC_PARCEL_ERROR;
    }

    if (ChangeInfo::Marshalling(changeInfo, data)) {
        HILOG_ERROR("failed, changeInfo marshalling error, changeType:%{public}ud, num of uris:%{public}ul, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return INVALID_PARAM;
    }

    auto error = Remote()->SendRequest(IDataObsMgr::NOTIFY_CHANGE_EXT, data, reply, option);
    if (error != 0) {
        HILOG_ERROR("failed, SendRequest error: %{public}d, changeType:%{public}ud, num of uris:%{public}ul, data is "
                    "nullptr:%{public}d, size:%{public}ud",
            error, changeInfo.changeType_, changeInfo.uris_.size(), changeInfo.data_ == nullptr, changeInfo.size_);
        return IPC_ERROR;
    }
    int32_t res = IPC_ERROR;
    return reply.ReadInt32(res) ? static_cast<Status>(res) : IPC_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
