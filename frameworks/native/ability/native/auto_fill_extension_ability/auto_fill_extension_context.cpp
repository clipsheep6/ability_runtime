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

#include "auto_fill_extension_context.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
void AutoFillExtensionContext::SetAutoFillExtensionCallback(
    const std::weak_ptr<IAutoFillExtensionCallback> &autoFillExtensionCallback)
{
    autoFillExtensionCallback_ = autoFillExtensionCallback;
}

void AutoFillExtensionContext::SetSessionInfo(const wptr<AAFwk::SessionInfo> &sessionInfo)
{
    sessionInfo_ = sessionInfo;
}

int32_t AutoFillExtensionContext::ReloadInModal(const CustomData &customData)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    auto sessionInfo = sessionInfo_.promote();
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Session info is nullptr.");
        return ERR_NULL_OBJECT;
    }
    auto autoFillExtensionCallback = autoFillExtensionCallback_.lock();
    if (autoFillExtensionCallback == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Auto fill extension callback is nullptr.");
        return ERR_NULL_OBJECT;
    }
    return autoFillExtensionCallback->OnReloadInModal(sessionInfo, customData);
}
} // namespace AbilityRuntime
} // namespace OHOS
