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

#ifndef OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CONTEXT_H

#include "extension_context.h"
#include "js_auto_fill_extension_util.h"
#include "session_info.h"

namespace OHOS {
namespace AbilityRuntime {
class IAutoFillExtensionCallback {
public:
    virtual int32_t OnReloadInModal(const sptr<AAFwk::SessionInfo> &sessionInfo, const CustomData &customData) = 0;
};

class AutoFillExtensionContext : public ExtensionContext {
public:
    AutoFillExtensionContext() = default;
    virtual ~AutoFillExtensionContext() = default;

    void SetAutoFillExtensionCallback(const std::weak_ptr<IAutoFillExtensionCallback> &autoFillExtensionCallback);
    void SetSessionInfo(const wptr<AAFwk::SessionInfo> &sessionInfo);
    int32_t ReloadInModal(const CustomData &customData);

private:
    std::weak_ptr<IAutoFillExtensionCallback> autoFillExtensionCallback_;
    wptr<AAFwk::SessionInfo> sessionInfo_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AUTO_FILL_EXTENSION_CONTEXT_H
