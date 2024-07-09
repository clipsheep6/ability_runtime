/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_REQUEST_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_AUTO_REQUEST_CALLBACK_INTERFACE_H

#include "auto_fill_custom_config.h"
#include "view_data.h"

namespace OHOS {
namespace AbilityRuntime {
class IFillRequestCallback {
public:
    virtual ~IFillRequestCallback() {}

    virtual void OnFillRequestSuccess(const AbilityBase::ViewData &viewData) = 0;
    virtual void OnFillRequestFailed(int32_t errCode, const std::string& fillContent = "", bool isPopup = false) = 0;
    virtual void onPopupConfigWillUpdate(AutoFill::AutoFillCustomConfig& config) {}
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_REQUEST_CALLBACK_INTERFACE_H