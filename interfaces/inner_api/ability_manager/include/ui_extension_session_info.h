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

#ifndef OHOS_ABILITY_RUNTIME_UI_EXTENSION_SESSION_INFO_H
#define OHOS_ABILITY_RUNTIME_UI_EXTENSION_SESSION_INFO_H

#include "parcel.h"
#include "session_info_constants.h"

namespace OHOS {
namespace AbilityRuntime {
class UIExtensionSessionInfo : public Parcelable {
public:
    UIExtensionSessionInfo() = default;
    virtual ~UIExtensionSessionInfo() = default;

    bool Marshalling(Parcel &parcel) const override;
    static UIExtensionSessionInfo *Unmarshalling(Parcel &parcel);

    int32_t persistentId = 0;
    uint32_t hostWindowId = 0;
    AAFwk::UIExtensionUsage uiExtensionUsage = AAFwk::UIExtensionUsage::MODAL;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_EXTENSION_SESSION_INFO_H
