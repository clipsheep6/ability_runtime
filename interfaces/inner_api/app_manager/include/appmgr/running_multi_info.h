/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
// enum class MultiAppMode {
//     NOT_SUPPORT = 0,
//     MULTI_INSTANCE = 1,
//     MULTI_ISOLATION_APP = 2,
// };


// struct RunningAppInstance : public Parcelable {
//     std::string appInstanceKey;
//     std::int32_t uid;
//     std::vector<int32_t> pids;

//     bool ReadFromParcel(Parcel &parcel);
//     virtual bool Marshalling(Parcel &parcel) const override;
//     static RunningProcessInfo *Unmarshalling(Parcel &parcel);
// };

// struct RunningIsolationApp : public Parcelable {
//     std::uint32_t appIndex;
//     std::int32_t uid;
//     std::vector<int32_t> pids;

//     bool ReadFromParcel(Parcel &parcel);
//     virtual bool Marshalling(Parcel &parcel) const override;
//     static RunningProcessInfo *Unmarshalling(Parcel &parcel);
// };

struct RunningMultiAppInfo : public Parcelable {
    std::string bundleName_;
    int32_t mode_;
    std::vector<std::string> instance_;
    std::vector<std::string> isolation_;
    // RunningAppInstance instance;0000000002222
    // RunningIsolationApp isolation;
    RunningMultiAppInfo(std::string bundleName, int32_t mode, std::vector<std::string> instance,
        std::vector<std::string> isolation): bundleName_(bundleName), mode_(mode), instance_(instance), isolation_(isolation) {}
    RunningMultiAppInfo() {}
    
    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static RunningMultiAppInfo *Unmarshalling(Parcel &parcel);
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RUNNING_MULTI_INFO_H
