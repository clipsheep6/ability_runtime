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

#include "render_process_info.h"

#include "nlohmann/json.hpp"
#include "string_ex.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool RenderProcessInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName_ = Str16ToStr8(parcel.ReadString16());
    processName_ = Str16ToStr8(parcel.ReadString16());
    int32_t pidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, pidData);
    pid_ = static_cast<int32_t>(pidData);
    int32_t uidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, uidData);
    uid_ = static_cast<int32_t>(uidData);
    int32_t hostUidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, hostUidData);
    hostUid_ = static_cast<int32_t>(hostUidData);
    int32_t hostPidData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, hostPidData);
    hostPid_ = static_cast<int32_t>(hostPidData);
    int32_t stateData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, stateData);
    state_ = static_cast<int32_t>(stateData);
    return true;
}

RenderProcessInfo *RenderProcessInfo::Unmarshalling(Parcel &parcel)
{
    RenderProcessInfo *info = new (std::nothrow) RenderProcessInfo();
    if (info && !info->ReadFromParcel(parcel)) {
        TAG_LOGW(AAFwkTag::APPMGR, "read from parcel failed");
        delete info;
        info = nullptr;
    }
    return info;
}

bool RenderProcessInfo::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(bundleName_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(processName_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(pid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(uid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(hostUid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(hostPid_));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(state_));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
