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
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote is nullptr");
        return;
    }

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("data.WriteInterfaceToken(GetDescriptor()) return false");
        return;
    }

    int result = remote->SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("SendRequest error, result=%{public}d", result);
    }
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @param changeInfo Indicates the info of the data to operate.
 */
void DataAbilityObserverProxy::OnChangeExt(const ChangeInfo &changeInfo)
{
    auto remote = Remote();
    if (remote == nullptr) {
        HILOG_ERROR("remote is nullptr");
        return;
    }

    OHOS::MessageParcel data;
    OHOS::MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!data.WriteInterfaceToken(DataAbilityObserverProxy::GetDescriptor())) {
        HILOG_ERROR("data.WriteInterfaceToken(GetDescriptor()) return false");
        return;
    }

    if (ChangeInfo::Marshalling(changeInfo, data)) {
        HILOG_ERROR("changeInfo marshalling failed");
        return;
    }

    int result = remote->SendRequest(IDataAbilityObserver::DATA_ABILITY_OBSERVER_CHANGE_EXT, data, reply, option);
    if (result != ERR_NONE) {
        HILOG_ERROR("SendRequest error, result=%{public}d", result);
    }
}

}  // namespace AAFwk
}  // namespace OHOS
