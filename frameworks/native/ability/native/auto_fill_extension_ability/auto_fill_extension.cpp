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

#include "auto_fill_extension.h"

#include "auto_fill_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_auto_fill_extension.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
AutoFillExtension *AutoFillExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    if (runtime == nullptr) {
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
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Called.");
    ExtensionBase<AutoFillExtensionContext>::Init(record, application, handler, token);
}

std::shared_ptr<AutoFillExtensionContext> AutoFillExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<AutoFillExtensionContext> context =
        ExtensionBase<AutoFillExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "UIExtension CreateAndInitContext context is nullptr.");
        return context;
    }
    return context;
}
} // namespace AbilityRuntime
} // namespace OHOS
