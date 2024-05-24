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

#ifndef OHOS_ABILITY_RUNTIME_RUNNING_MULTI_INFO_H
#define OHOS_ABILITY_RUNTIME_RUNNING_MULTI_INFO_H

#include <string>
#include <vector>

#include "ability_info.h"
#include "app_mgr_constants.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
const int32_t MAX_CLONE_APP_NUM = 128;
struct RunningAppClone {
    int32_t appCloneIndex;
    int32_t uid;
    std::vector<int32_t> pids;
};

struct RunningMultiAppInfo : public Parcelable {
    std::string bundleName;
    int32_t mode;
    std::vector<RunningAppClone> runningAppClones;
    
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static RunningMultiAppInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RUNNING_MULTI_INFO_H