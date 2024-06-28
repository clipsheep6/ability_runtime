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

#ifndef OHOS_ABILITY_RUNTIME_DIALOG_UI_EXTENSION_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_DIALOG_UI_EXTENSION_CALLBACK_H

#include "ability_context.h"
#ifdef SUPPORT_SCREEN
#include "ui_content.h"
#endif // SUPPORT_SCREEN
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class DialogUIExtensionCallback {
public:
    DialogUIExtensionCallback(const std::weak_ptr<AppExecFwk::IAbilityCallback> &abilityCallback);
    ~DialogUIExtensionCallback() = default;
    void OnRelease();
    void OnError();
    void OnDestroy();
    void SetSessionId(int32_t sessionId);
#ifdef SUPPORT_SCREEN
    void SetUIContent(Ace::UIContent *uiContent);
#endif // SUPPORT_SCREEN
private:
    int32_t sessionId_ = 0;
    #ifdef SUPPORT_SCREEN
    Ace::UIContent *uiContent_ = nullptr;
    #endif // SUPPORT_SCREEN
    std::weak_ptr<AppExecFwk::IAbilityCallback> abilityCallback_;
};
} // AbilityRuntime
} // OHOS
#endif // OHOS_ABILITY_RUNTIME_DIALOG_UI_EXTENSION_CALLBACK_H