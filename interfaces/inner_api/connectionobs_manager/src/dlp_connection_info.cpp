/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dlp_connection_info.h"

namespace OHOS {
namespace AbilityRuntime {
bool DlpConnectionInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(dlpUid)) {
        return false;
    }

    if (!parcel.WriteInt32(openedAbilityCount)) {
        return false;
    }

    return true;
}

bool DlpConnectionInfo::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(dlpUid)) {
        return false;
    }

    if (!parcel.ReadInt32(openedAbilityCount)) {
        return false;
    }

    return true;
}

DlpConnectionInfo *DlpConnectionInfo::Unmarshalling(Parcel &parcel)
{
    DlpConnectionInfo *data = new (std::nothrow) DlpConnectionInfo();
    if (!data->ReadFromParcel(parcel)) {
        delete data;
        data = nullptr;
    }
    return data;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
