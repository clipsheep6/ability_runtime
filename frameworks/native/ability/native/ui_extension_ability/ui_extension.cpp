/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ui_extension.h"

#include "hilog_wrapper.h"
#include "js_ui_extension.h"
#include "runtime.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
UIExtension* UIExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new UIExtension();
    }
    HILOG_DEBUG("UIExtension Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsUIExtension::Create(runtime);

        default:
            return new UIExtension();
    }
}

void UIExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<UIExtensionContext>::Init(record, application, handler, token);
    HILOG_DEBUG("UIExtension begin init context");
}

std::shared_ptr<UIExtensionContext> UIExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<UIExtensionContext> context =
        ExtensionBase<UIExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        HILOG_ERROR("UIExtension CreateAndInitContext context is nullptr");
        return context;
    }
    return context;
}
}
}
