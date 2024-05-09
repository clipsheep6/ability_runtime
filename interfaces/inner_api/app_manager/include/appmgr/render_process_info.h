/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_RENDER_PROCESS_INFO_H
#define OHOS_ABILITY_RUNTIME_RENDER_PROCESS_INFO_H

#include <string>
#include <vector>

#include "ability_info.h"
#include "app_mgr_constants.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct RenderProcessInfo : public Parcelable {
    std::string bundleName_;
    std::string processName_;
    std::int32_t pid_;
    std::int32_t uid_;
    std::int32_t hostUid_;
    std::int32_t hostPid_;
    std::int32_t state_;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static RenderProcessInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_RENDER_PROCESS_INFO_H
