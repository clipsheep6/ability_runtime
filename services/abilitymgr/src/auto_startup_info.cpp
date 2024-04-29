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
#include "auto_startup_info.h"

#include "string_ex.h"

namespace OHOS {
namespace AbilityRuntime {
bool AutoStartupInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    abilityName = Str16ToStr8(parcel.ReadString16());
    moduleName = Str16ToStr8(parcel.ReadString16());
    abilityTypeName = Str16ToStr8(parcel.ReadString16());
    return true;
}

AutoStartupInfo *AutoStartupInfo::Unmarshalling(Parcel &parcel)
{
    AutoStartupInfo *info = new (std::nothrow) AutoStartupInfo();
    if (info == nullptr) {
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

bool AutoStartupInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(bundleName))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(abilityName))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(moduleName))) {
        return false;
    }
    if (!parcel.WriteString16(Str8ToStr16(abilityTypeName))) {
        return false;
    }
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
