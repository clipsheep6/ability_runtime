/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "process_data.h"

#include "hilog_wrapper.h"
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool ProcessData::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(state));
    return (parcel.WriteString(bundleName) && parcel.WriteInt32(pid) && parcel.WriteInt32(uid) &&
        parcel.WriteBool(isContinuounsTask) && parcel.WriteBool(isKeepAlive));
}

bool ProcessData::ReadFromParcel(Parcel &parcel)
{
    int32_t stateData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, stateData);
    state = static_cast<AppProcessState>(stateData);
    bundleName = parcel.ReadString();
    pid = parcel.ReadInt32();
    uid = parcel.ReadInt32();
    isContinuounsTask = parcel.ReadBool();
    isKeepAlive = parcel.ReadBool();

    return true;
}

ProcessData *ProcessData::Unmarshalling(Parcel &parcel)
{
    ProcessData *processData = new (std::nothrow) ProcessData();
    if (processData && !processData->ReadFromParcel(parcel)) {
        HILOG_WARN("processData failed, because ReadFromParcel failed");
        delete processData;
        processData = nullptr;
    }
    return processData;
}
}  // namespace AppExecFwk
}  // namespace OHOS
