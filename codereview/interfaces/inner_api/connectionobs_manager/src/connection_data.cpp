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

#include "connection_data.h"

#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool ConnectionData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(extensionPid)) {
        return false;
    }

    if (!parcel.WriteInt32(extensionUid)) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(extensionBundleName))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(extensionModuleName))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(extensionName))) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(extensionType))) {
        return false;
    }

    if (!parcel.WriteInt32(callerUid)) {
        return false;
    }

    if (!parcel.WriteInt32(callerPid)) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(callerName))) {
        return false;
    }

    return true;
}

bool ConnectionData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(extensionPid)) {
        return false;
    }

    if (!parcel.ReadInt32(extensionUid)) {
        return false;
    }

    std::u16string strValue;
    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    extensionBundleName = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    extensionModuleName = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    extensionName = Str16ToStr8(strValue);

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        return false;
    }
    extensionType = static_cast<ExtensionAbilityType>(type);

    if (!parcel.ReadInt32(callerUid)) {
        HILOG_WARN("ConnectionData::ReadFromParcel read callerUid failed");
        return false;
    }

    if (!parcel.ReadInt32(callerPid)) {
        HILOG_WARN("ConnectionData::ReadFromParcel read callerPid failed");
        return false;
    }

    if (!parcel.ReadString16(strValue)) {
        HILOG_WARN("ConnectionData::ReadFromParcel read strValue failed");
        return false;
    }
    callerName = Str16ToStr8(strValue);

    return true;
}

ConnectionData *ConnectionData::Unmarshalling(Parcel &parcel)
{
    ConnectionData *data = new ConnectionData();
    if (!data->ReadFromParcel(parcel)) {
        delete data;
        data = nullptr;
    }
    return data;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
