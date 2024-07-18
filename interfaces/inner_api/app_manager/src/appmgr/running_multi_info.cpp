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
#include "parcel_macro_base.h"

namespace OHOS {
namespace AppExecFwk {
bool RunningMultiAppInfo::ReadFromParcel(Parcel &parcel)
{
    bundleName = Str16ToStr8(parcel.ReadString16());
    mode = parcel.ReadInt32();
    int32_t runningAppClonesSize;
    READ_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, runningAppClonesSize);
    if (runningAppClonesSize > MAX_CLONE_APP_NUM) {
        return false;
    }
    for (auto i = 0; i < runningAppClonesSize; i++) {
        RunningAppClone clone;
        clone.appCloneIndex = parcel.ReadInt32();
        clone.uid = parcel.ReadInt32();
        parcel.ReadInt32Vector(&clone.pids);
        runningAppClones.emplace_back(clone);
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
    WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, runningAppClones.size());
    if (runningAppClones.size() > MAX_CLONE_APP_NUM) {
        return false;
    }
    for (auto &clone : runningAppClones) {
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, clone.appCloneIndex);
        WRITE_PARCEL_AND_RETURN_FALSE_IF_FAIL(Int32, parcel, clone.uid);
        if (!parcel.WriteInt32Vector(clone.pids)) {
            TAG_LOGE(AAFwkTag::APPMGR, "write runningAppClones failed.");
            return false;
        }
    }
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS