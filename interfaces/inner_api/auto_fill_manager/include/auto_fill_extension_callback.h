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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H

#include <memory>
#include <string>

#include "fill_request_callback_interface.h"
#include "save_request_callback_interface.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class AutoFillExtensionCallback {
public:
    AutoFillExtensionCallback() = default;
    ~AutoFillExtensionCallback() = default;

    void OnResult(int32_t number, const AAFwk::Want &want);
    void OnRelease(int32_t number);

    void SetFillRequestCallback(const std::shared_ptr<IFillRequestCallback> &callback);
    void SetSaveRequestCallback(const std::shared_ptr<ISaveRequestCallback> &callback);

private:
    std::weak_ptr<IFillRequestCallback> fillCallback_;
    std::weak_ptr<ISaveRequestCallback> saveCallback_;
    bool isFillCallback_ = false;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CALLBACK_H