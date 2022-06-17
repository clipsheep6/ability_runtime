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

#include "form_share_info.h"
#include "hilog_wrapper.h"
#include "message_parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool FormShareInfo::ReadFromParcel(Parcel &parcel)
{
    formId = parcel.ReadInt64();
    formName = Str16ToStr8(parcel.ReadString16());
    bundleName = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    abilityName = Str16ToStr8(parcel.ReadString16());
    formTempFlag = parcel.ReadBool();
    dimensionId = parcel.ReadUint32();

    std::unique_ptr<OHOS::AAFwk::WantParams> wantParams(parcel.ReadParcelable<OHOS::AAFwk::WantParams>());
    if (wantParams == nullptr){
        return false;
    }
    providerData = *wantParams;

    deviceId = Str16ToStr8(parcel.ReadString16());
    deviceName = Str16ToStr8(parcel.ReadString16());
    return true;
}

bool FormShareInfo::Marshalling(Parcel &parcel) const
{
    // write formId
    if (!parcel.WriteInt64(formId)) {
        return false;
    }

    // write formName
    if (!parcel.WriteString16(Str8ToStr16(formName))) {
        return false;
    }

    // write bundleName
    if (!parcel.WriteString16(Str8ToStr16(bundleName))) {
        return false;
    }

    // write moduleName
    if (!parcel.WriteString16(Str8ToStr16(moduleName))) {
        return false;
    }

    // write abilityName
    if (!parcel.WriteString16(Str8ToStr16(abilityName))) {
        return false;
    }

    // write tempFlag
    if (!parcel.WriteBool(formTempFlag)) {
        return false;
    }

    // write dimensionId
    if (!parcel.WriteInt32(dimensionId)) {
        return false;
    }

    // write providerData
    if (!parcel.WriteParcelable(&providerData)) {
        return false;
    }

    // write deviceId
    if (!parcel.WriteString16(Str8ToStr16(deviceId))) {
        return false;
    }

    // write deviceName
    if (!parcel.WriteString16(Str8ToStr16(deviceName))) {
        return false;
    }

    return true;
}

FormShareInfo* FormShareInfo::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<FormShareInfo> object = std::make_unique<FormShareInfo>();
    if (object && !object->ReadFromParcel(parcel)) {
        object = nullptr;
    }
    return object.release();
}
}  // namespace AppExecFwk
}  // namespace OHOS
