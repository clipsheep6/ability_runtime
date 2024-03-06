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

#include "process_options.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool ProcessOptions::ReadFromParcel(Parcel &parcel)
{
    processMode = static_cast<ProcessMode>(parcel.ReadInt32());
    startupVisibility = static_cast<StartupVisibility>(parcel.ReadInt32());
    return true;
}

ProcessOptions *ProcessOptions::Unmarshalling(Parcel &parcel)
{
    ProcessOptions *option = new (std::nothrow) ProcessOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool ProcessOptions::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(processMode))) {
        HILOG_ERROR("Failed to write processMode");
        return false;
    }
    if (!parcel.WriteInt32(static_cast<int32_t>(startupVisibility))) {
        HILOG_ERROR("Failed to write startupVisibility");
        return false;
    }
    return true;
}

ProcessMode ProcessOptions::ConvertInt32ToProcessMode(int32_t value)
{
    if (value <= static_cast<int32_t>(ProcessMode::UNSPECIFIED) ||
        value >= static_cast<int32_t>(ProcessMode::END)) {
        return ProcessMode::UNSPECIFIED;
    }
    return static_cast<ProcessMode>(value);
}

StartupVisibility ProcessOptions::ConvertInt32ToStartupVisibility(int32_t value)
{
    if (value <= static_cast<int32_t>(StartupVisibility::UNSPECIFIED) ||
        value >= static_cast<int32_t>(StartupVisibility::END)) {
        return StartupVisibility::UNSPECIFIED;
    }
    return static_cast<StartupVisibility>(value);
}

bool ProcessOptions::IsNewProcessMode(ProcessMode value)
{
    return (value == ProcessMode::NEW_PROCESS_ATTACH_TO_PARENT) ||
        (value == ProcessMode::NEW_PROCESS_ATTACH_TO_STATUS_BAR_ITEM);
}
}  // namespace AAFwk
}  // namespace OHOS
