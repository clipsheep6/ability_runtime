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

#include "dlp_state_data.h"

#include "hilog_wrapper.h"
#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
bool DlpStateData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(targetPid)) {
        return false;
    }

    if (!parcel.WriteInt32(targetUid)) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(targetBundleName))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(targetModuleName))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(targetAbilityName))) {
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

bool DlpStateData::ReadFromParcel(Parcel &parcel)
{
    if (!parcel.ReadInt32(targetPid)) {
        return false;
    }

    if (!parcel.ReadInt32(targetUid)) {
        return false;
    }

    std::u16string strValue;
    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    targetBundleName = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    targetModuleName = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    targetAbilityName = Str16ToStr8(strValue);

    if (!parcel.ReadInt32(callerUid)) {
        HILOG_WARN("DlpStateData::ReadFromParcel read callerUid failed");
        return false;
    }

    if (!parcel.ReadInt32(callerPid)) {
        HILOG_WARN("DlpStateData::ReadFromParcel read callerPid failed");
        return false;
    }

    if (!parcel.ReadString16(strValue)) {
        HILOG_WARN("DlpStateData::ReadFromParcel read strValue failed");
        return false;
    }
    callerName = Str16ToStr8(strValue);

    return true;
}

DlpStateData *DlpStateData::Unmarshalling(Parcel &parcel)
{
    DlpStateData *data = new (std::nothrow) DlpStateData();
    if (data && !data->ReadFromParcel(parcel)) {
        delete data;
        data = nullptr;
    }
    return data;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
