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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_OPTIONS_H
#define OHOS_ABILITY_RUNTIME_PROCESS_OPTIONS_H

#include "parcel.h"

namespace OHOS {
namespace AAFwk {
enum class ProcessMode {
    UNSPECIFIED = 0,
    NEW_PROCESS_ATTACH_TO_PARENT = 1,
    NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM = 2,
    END
};

enum class StartupVisibility {
    UNSPECIFIED = -1,
    STARTUP_HIDE = 0,
    STARTUP_SHOW = 1,
    END
};

class ProcessOptions final : public Parcelable {
public:
    ProcessOptions() = default;
    ~ProcessOptions() = default;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ProcessOptions *Unmarshalling(Parcel &parcel);

    static ProcessMode ConvertInt32ToProcessMode(int32_t value);
    static StartupVisibility ConvertInt32ToStartupVisibility(int32_t value);
    static bool IsNewProcessMode(ProcessMode value);

    ProcessMode processMode = ProcessMode::UNSPECIFIED;
    StartupVisibility startupVisibility = StartupVisibility::UNSPECIFIED;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_PROCESS_OPTIONS_H
