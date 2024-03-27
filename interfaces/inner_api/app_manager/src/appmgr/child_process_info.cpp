/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "child_process_info.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "nlohmann/json.hpp"
#include "parcel_macro_base.h"
#include "string_ex.h"

namespace OHOS {
namespace AppExecFwk {
bool ChildProcessInfo::ReadFromParcel(Parcel &parcel)
{
    int32_t pidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, pidData);
    pid = static_cast<int32_t>(pidData);

    int32_t hostPidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, hostPidData);
    hostPid = static_cast<int32_t>(hostPidData);

    int32_t uidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, uidData);
    uid = static_cast<int32_t>(uidData);

    bundleName = Str16ToStr8(parcel.ReadString16());
    processName = Str16ToStr8(parcel.ReadString16());
    srcEntry = Str16ToStr8(parcel.ReadString16());
    jitEnabled = parcel.ReadBool();

    return true;
}

ChildProcessInfo *ChildProcessInfo::Unmarshalling(Parcel &parcel)
{
    ChildProcessInfo *info = new (std::nothrow) ChildProcessInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool ChildProcessInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(pid));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(hostPid));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(uid));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(processName));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(srcEntry));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Bool, parcel, jitEnabled);
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
