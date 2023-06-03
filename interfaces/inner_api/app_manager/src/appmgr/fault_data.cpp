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

#include "fault_data.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool FaultData::ReadFromParcel(Parcel &parcel)
{
    std::u16string strValue;
    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.name = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.message = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.stack = Str16ToStr8(strValue);

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        return false;
    }
    faultType = static_cast<FaultDataType>(type);
    return true;
}

FaultData *FaultData::Unmarshalling(Parcel &parcel)
{
    FaultData *info = new FaultData();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool FaultData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(errorObject.name))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(errorObject.message))) {
        return false;
    }
    
    if (!parcel.WriteString16(Str8ToStr16(errorObject.stack))) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        return false;
    }
    return true;
}

bool AppFaultDataBySA::ReadFromParcel(Parcel &parcel)
{
    std::u16string strValue;
    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.name = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.message = Str16ToStr8(strValue);

    if (!parcel.ReadString16(strValue)) {
        return false;
    }
    errorObject.stack = Str16ToStr8(strValue);

    int type = 0;
    if (!parcel.ReadInt32(type)) {
        return false;
    }
    faultType = static_cast<FaultDataType>(type);

    if (!parcel.ReadInt32(pid)) {
        return false;
    }
    return true;
}

AppFaultDataBySA *AppFaultDataBySA::Unmarshalling(Parcel &parcel)
{
    AppFaultDataBySA *info = new AppFaultDataBySA();
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool AppFaultDataBySA::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(errorObject.name))) {
        return false;
    }

    if (!parcel.WriteString16(Str8ToStr16(errorObject.message))) {
        return false;
    }
    
    if (!parcel.WriteString16(Str8ToStr16(errorObject.stack))) {
        return false;
    }

    if (!parcel.WriteInt32(static_cast<int32_t>(faultType))) {
        return false;
    }

    if (!parcel.WriteInt32(pid)) {
        return false;
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS