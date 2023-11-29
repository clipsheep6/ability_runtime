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

#include "ui_extension_ability_connect_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
bool UIExtensionAbilityConnectInfo::ReadFromParcel(Parcel &parcel)
{
    hostBundleName = parcel.ReadString();
    uiExtensionAbilityId = parcel.ReadInt32();
    return true;
}

UIExtensionAbilityConnectInfo *UIExtensionAbilityConnectInfo::Unmarshalling(Parcel &parcel)
{
    UIExtensionAbilityConnectInfo *connectInfo = new (std::nothrow) UIExtensionAbilityConnectInfo();
    if (connectInfo == nullptr) {
        HILOG_ERROR("New connect Info failed.");
        return nullptr;
    }

    if (!connectInfo->ReadFromParcel(parcel)) {
        delete connectInfo;
        connectInfo = nullptr;
    }
    return connectInfo;
}

bool UIExtensionAbilityConnectInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(hostBundleName)) {
        HILOG_ERROR("Write hostBundleName failed.");
        return false;
    }

    if (!parcel.WriteInt32(uiExtensionAbilityId)) {
        HILOG_ERROR("Write uiExtensionAbilityId failed.");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
