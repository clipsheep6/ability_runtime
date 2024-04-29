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

#include "demo_ui_extension.h"

#include "hilog_tag_wrapper.h"
#include "js_demo_ui_extension.h"

namespace OHOS {
namespace AbilityRuntime {
DemoUIExtension *DemoUIExtension::Create(const std::unique_ptr<Runtime> &runtime)
{
    HILOG_DEBUG("Create demo extension.");
    if (runtime == nullptr) {
        return new DemoUIExtension();
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsDemoUIExtension::Create(runtime);
        default:
            return new (std::nothrow) DemoUIExtension();
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
