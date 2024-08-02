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

#include "exit_reason.h"

#include "hilog_tag_wrapper.h"
#include "parcel_macro_base.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {
ExitReason::ExitReason(const Reason reason, const std::string &exitMsg)
{
    this->reason = reason;
    this->exitMsg = exitMsg;
}

bool ExitReason::ReadFromParcel(Parcel &parcel)
{
    int32_t reasonData;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, reasonData);
    reason = static_cast<Reason>(reasonData);

    exitMsg = Str16ToStr8(parcel.ReadString16());
    return true;
}

ExitReason *ExitReason::Unmarshalling(Parcel &parcel)
{
    ExitReason *data = new (std::nothrow) ExitReason();
    if (!data) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "data is nullptr.");
        return nullptr;
    }
    if (!data->ReadFromParcel(parcel)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "read from parcel failed");
        delete data;
        data = nullptr;
    }
    return data;
}

bool ExitReason::Marshalling(Parcel &parcel) const
{
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, static_cast<int32_t>(reason));
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(String16, parcel, Str8ToStr16(exitMsg));
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
