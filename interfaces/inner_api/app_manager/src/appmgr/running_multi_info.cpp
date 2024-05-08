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

#include "running_multi_info.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool RunningMultiAppInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    mode = parcel.ReadInt32();
    if (!parcel.ReadStringVector(&instance)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read instance failed.");
        return false;
    }
    int32_t isolationSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, isolationSize);
    for (auto i = 0; i < isolationSize; i++) {
        RunningAppTwin twin;
        twin.appTwinIndex = parcel.ReadInt32();
        twin.uid = parcel.ReadInt32();
        parcel.ReadInt32Vector(&twin.pids);
        isolation.emplace_back(twin);
    }
    return true;
}

RunningMultiAppInfo *RunningMultiAppInfo::Unmarshalling(Parcel &parcel)
{
    RunningMultiAppInfo *info = new (std::nothrow) RunningMultiAppInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool RunningMultiAppInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, mode);
    if (!parcel.WriteStringVector(instance)) {
        TAG_LOGE(AAFwkTag::APPMGR, "write instance failed.");
        return false;
    }
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, isolation.size());
    for (auto &twin : isolation) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, twin.appTwinIndex);
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, twin.uid);
        if (!parcel.WriteInt32Vector(twin.pids)) {
        TAG_LOGE(AAFwkTag::APPMGR, "read instance failed.");
        return false;
        }   
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS