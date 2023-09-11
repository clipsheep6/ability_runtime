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

#include "app_debug_info.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
bool AppDebugInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    pid = parcel.ReadInt32();
    uid = parcel.ReadInt32();
    return true;
}

AppDebugInfo *AppDebugInfo::Unmarshalling(Parcel &parcel)
{
    AppDebugInfo *info = new (std::nothrow) AppDebugInfo();
    if (info == nullptr) {
        HILOG_ERROR("AppDebugInfo is nullptr");
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        HILOG_ERROR("ReadFromParcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool AppDebugInfo::Marshalling(Parcel &parcel) const
{
    // write bundleName
    if (!parcel.WriteString(bundleName)) {
        HILOG_ERROR("Write bundleName failed");
        return false;
    }
    // write pid
    if (!parcel.WriteInt32(pid)) {
        HILOG_ERROR("Write pid failed");
        return false;
    }
    // write uid
    if (!parcel.WriteInt32(uid)) {
        HILOG_ERROR("Write uid failed");
        return false;
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS
