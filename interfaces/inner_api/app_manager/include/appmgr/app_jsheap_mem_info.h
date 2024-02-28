/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2024. All rights reserved.
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

#ifndef OHOS_ABILITY_RUNTIME_APP_JSHEAP_MEM_INFO_H
#define OHOS_ABILITY_RUNTIME_APP_JSHEAP_MEM_INFO_H

#include "parcel.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
struct JsHeapDumpInfo : public Parcelable {
    uint32_t pid;
    uint32_t tid;
    bool needGc;
    bool needSnapshot;
    virtual bool Marshalling(Parcel &parcel) const override;
    static JsHeapDumpInfo *Unmarshalling(Parcel &parcel);
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_APP_JSHEAP_MEM_INFO_H