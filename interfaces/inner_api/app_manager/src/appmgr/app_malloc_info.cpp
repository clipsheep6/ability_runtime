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

#include "app_malloc_info.h"

namespace OHOS {
namespace AppExecFwk {
bool MallocInfo::Marshalling(Parcel &parcel) const
{
    return (parcel.WriteInt32(usmblks) && parcel.WriteInt32(uordblks)
        && parcel.WriteInt32(fordblks) && parcel.WriteInt32(hblkhd));
}

MallocInfo *MallocInfo::Unmarshalling(Parcel &parcel)
{
    MallocInfo *mallocInfo = new (std::nothrow) MallocInfo();
    if (mallocInfo == nullptr) {
        return nullptr;
    }
    mallocInfo->usmblks = parcel.ReadInt32();
    mallocInfo->uordblks = parcel.ReadInt32();
    mallocInfo->fordblks = parcel.ReadInt32();
    mallocInfo->hblkhd = parcel.ReadInt32();
    return mallocInfo;
}
}  // namespace AppExecFwk
}  // namespace OHOS
