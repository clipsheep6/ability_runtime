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

#include "auto_fill_extension.h"

#include "hilog_wrapper.h"
#include "js_auto_fill_extension.h"
#include "runtime.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
AutoFillExtension *AutoFillExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_DEBUG("Called.");
    if (!runtime) {
        return new AutoFillExtension();
    }
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsAutoFillExtension::Create(runtime);
        default:
            return new AutoFillExtension();
    }
}

void AutoFillExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("Called.");
    ExtensionBase<UIExtensionContext>::Init(record, application, handler, token);
}
} // namespace AbilityRuntime
} // namespace OHOS
