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

#ifndef OHOS_ABILITY_RUNTIME_CHILD_PROCESS_REQUEST_H
#define OHOS_ABILITY_RUNTIME_CHILD_PROCESS_REQUEST_H

#include <string>

#include "child_process_args.h"
#include "child_process_info.h"
#include "child_process_options.h"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
struct ChildProcessRequest : public Parcelable {
    std::string srcEntry;
    int32_t childProcessType = CHILD_PROCESS_TYPE_JS;
    int32_t childProcessCount = 0;
    bool isStartWithDebug = false;
    ChildProcessArgs args;
    ChildProcessOptions options;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static ChildProcessRequest *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CHILD_PROCESS_REQUEST_H
