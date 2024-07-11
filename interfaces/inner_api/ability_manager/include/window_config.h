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

#ifndef OHOS_ABILITY_RUNTIME_WINDOW_CONFIG_H
#define OHOS_ABILITY_RUNTIME_WINDOW_CONFIG_H

#include <string>

#include "parcel.h"

namespace OHOS {
namespace AAFwk {
struct WindowConfig : public Parcelable {
    WindowConfig() = default;

    int32_t windowType = 0;
    int32_t posx = 0;
    int32_t posy = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    virtual bool Marshalling(Parcel &parcel) const override;
    static WindowConfig *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_WINDOW_CONFIG_H