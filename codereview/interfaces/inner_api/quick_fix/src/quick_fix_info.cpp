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

#include "quick_fix_info.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool ApplicationQuickFixInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    bundleVersionCode = parcel.ReadUint32();
    bundleVersionName = parcel.ReadString();
    std::unique_ptr<AppExecFwk::AppqfInfo> qfInfo(parcel.ReadParcelable<AppExecFwk::AppqfInfo>());
    if (qfInfo == nullptr) {
        HILOG_ERROR("ReadParcelable<AppqfInfo> failed.");
        return false;
    }
    appqfInfo = *qfInfo;
    return true;
}

bool ApplicationQuickFixInfo::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(bundleName)) {
        HILOG_ERROR("Write bundleName failed.");
        return false;
    }
    if (!parcel.WriteUint32(bundleVersionCode)) {
        HILOG_ERROR("Write bundleVersionCode failed.");
        return false;
    }
    if (!parcel.WriteString(bundleVersionName)) {
        HILOG_ERROR("Write bundleVersionName failed.");
        return false;
    }
    if (!parcel.WriteParcelable(&appqfInfo)) {
        HILOG_ERROR("Write appQfInfo failed.");
        return false;
    }
    return true;
}

ApplicationQuickFixInfo *ApplicationQuickFixInfo::Unmarshalling(Parcel &parcel)
{
    ApplicationQuickFixInfo *info = new (std::nothrow) ApplicationQuickFixInfo();
    if (info == nullptr) {
        HILOG_ERROR("Create failed.");
        return nullptr;
    }

    if (!info->ReadFromParcel(parcel)) {
        HILOG_ERROR("Read from parcel failed.");
        delete info;
        return nullptr;
    }

    return info;
}
} // namespace AAFwk
} // namespace OHOS