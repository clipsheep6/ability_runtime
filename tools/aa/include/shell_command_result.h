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

#ifndef FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_RESULT_H
#define FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_RESULT_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
struct ShellCommandResult : public Parcelable {
    int32_t exitCode {-1};
    std::string stdResult {};

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ShellCommandResult *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_STANDARD_TOOLS_AA_INCLUDE_SHELL_COMMAND_RESULT_H
