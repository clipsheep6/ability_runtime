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
#include "dialog_ui_extension_callback.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
DialogUIExtensionCallback::DialogUIExtensionCallback(const std::weak_ptr<AppExecFwk::IAbilityCallback> &abilityCallback)
    : abilityCallback_(abilityCallback)
{}
void DialogUIExtensionCallback::OnRelease()
{
    TAG_LOGD(AAFwkTag::DIALOG, "Called");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "abilityCallback is nullptr");
        return;
    }
#ifdef SUPPORT_SCREEN
    abilityCallback->EraseUIExtension(sessionId_);

    if (uiContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "uiContent_ is nullptr.");
        return;
    }
    uiContent_->CloseModalUIExtension(sessionId_);
#endif // SUPPORT_SCREEN
}

void DialogUIExtensionCallback::OnError()
{
    TAG_LOGD(AAFwkTag::DIALOG, "Called");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "abilityCallback is nullptr");
        return;
    }
#ifdef SUPPORT_SCREEN
    abilityCallback->EraseUIExtension(sessionId_);
    if (uiContent_ == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "uiContent_ is nullptr.");
        return;
    }
    uiContent_->CloseModalUIExtension(sessionId_);
#endif // SUPPORT_SCREEN
}

void DialogUIExtensionCallback::OnDestroy()
{
    TAG_LOGD(AAFwkTag::DIALOG, "Called");
    auto abilityCallback = abilityCallback_.lock();
    if (abilityCallback == nullptr) {
        TAG_LOGE(AAFwkTag::DIALOG, "abilityCallback is nullptr");
        return;
    }
#ifdef SUPPORT_SCREEN
    abilityCallback->EraseUIExtension(sessionId_);
#endif // SUPPORT_SCREEN
}

void DialogUIExtensionCallback::SetSessionId(int32_t sessionId)
{
    sessionId_= sessionId;
}
#ifdef SUPPORT_SCREEN
void DialogUIExtensionCallback::SetUIContent(Ace::UIContent *uiContent)
{
    uiContent_ = uiContent;
}
#endif // SUPPORT_SCREEN
} // namespace AbilityRuntime
} // namespace OHOS