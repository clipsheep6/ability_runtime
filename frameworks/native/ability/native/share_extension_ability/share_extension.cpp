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

#include "share_extension.h"

#include "hilog_wrapper.h"
#include "js_share_extension.h"
#include "runtime.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
ShareExtension *ShareExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_DEBUG("called.");
    if (!runtime) {
        return new ShareExtension();
    }
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsShareExtension::Create(runtime);
        default:
            return new ShareExtension();
    }
}

void ShareExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HILOG_DEBUG("called.");
    ExtensionBase<UIExtensionContext>::Init(record, application, handler, token);
}
} // namespace AbilityRuntime
} // namespace OHOS