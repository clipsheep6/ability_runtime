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

#include "data_ability_observer_proxy.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
DataAbilityObserverProxy::DataAbilityObserverProxy(const sptr<IRemoteObject> &remote)
    : IRemoteProxy<IDataAbilityObserver>(remote)
{}
DataAbilityObserverProxy::~DataAbilityObserverProxy()
{}
/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 */
void DataAbilityObserverProxy::OnChange()
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("data.WriteInterfaceToken(GetDescriptor()) return false");
        return;
    }

    if (!SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option)) {
        HILOG_ERROR("SendRequest error");
        return;
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param changeInfo Indicates the info of the data to operate.
 */
void DataAbilityObserverProxy::OnChangeExt(const ChangeInfo &changeInfo)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("data.WriteInterfaceToken(GetDescriptor()) return false");
        return;
    }

    if (!ChangeInfo::Marshalling(changeInfo, data)) {
        HILOG_ERROR("changeInfo marshalling failed");
        return;
    }

    if (!SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT, data, reply, option)) {
        HILOG_ERROR("SendRequest error");
        return;
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param uri Indicates the path of the data to operate.
 */
void DataAbilityObserverProxy::OnChangePreferences(const std::string &key)
{
    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("data.WriteInterfaceToken(GetDescriptor()) return false");
        return;
    }

    if (!data.WriteString(key)) {
        HILOG_ERROR("data.WriteString(key) return false");
        return;
    }

    if (!SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_PREFERENCES, data, reply, option)) {
        HILOG_ERROR("SendRequest error");
        return;
    }
}

bool DataAbilityObserverProxy::SendRequest(uint32_t code, MessageParcel &data,
                                           MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote object is nullptr.");
        return false;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != NO_ERROR) {
        HILOG_ERROR("SendRequest failed. code is %{public}d, ret is %{public}d.", code, ret);
        return false;
    }
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
