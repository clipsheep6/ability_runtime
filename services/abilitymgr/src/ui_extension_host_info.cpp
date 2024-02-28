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

#include "ui_extension_host_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
bool UIExtensionHostInfo::ReadFromParcel(Parcel &parcel)
{
    std::unique_ptr<AppExecFwk::ElementName> abilityInfo(parcel.ReadParcelable<AppExecFwk::ElementName>());
    if (abilityInfo == nullptr) {
        HILOG_ERROR("Read host info failed.");
        return false;
    }

    elementName_ = *abilityInfo;
    return true;
}

UIExtensionHostInfo *UIExtensionHostInfo::Unmarshalling(Parcel &parcel)
{
    UIExtensionHostInfo *hostInfo = new (std::nothrow) UIExtensionHostInfo();
    if (hostInfo == nullptr) {
        HILOG_ERROR("Create host info failed.");
        return nullptr;
    }

    if (!hostInfo->ReadFromParcel(parcel)) {
        delete hostInfo;
        hostInfo = nullptr;
    }

    return hostInfo;
}

bool UIExtensionHostInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(&elementName_)) {
        HILOG_ERROR("Write element name failed.");
        return false;
    }

    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
