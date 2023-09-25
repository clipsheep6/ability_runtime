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

#ifndef OHOS_ABILITY_RUNTIME_APP_DEBUG_INFO_H
#define OHOS_ABILITY_RUNTIME_APP_DEBUG_INFO_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @struct AppDebugInfo
 * Defines app debug struct info.
 */
struct AppDebugInfo : public Parcelable {
    std::string bundleName;
    int32_t pid;
    int32_t uid;
    bool isDebugStart = false; // Can only be used by app services, isAttachDebug is true when isDebugStart is false.

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static AppDebugInfo *Unmarshalling(Parcel &parcel);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_DEBUG_INFO_H
