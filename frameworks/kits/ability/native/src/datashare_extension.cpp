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

#include "datashare_extension.h"

#include "ability_loader.h"
#include "connection_manager.h"
#include "extension_base.cpp"
#include "hilog_wrapper.h"
#include "js_datashare_extension.h"
#include "runtime.h"
#include "datashare_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
DataShareExtension* DataShareExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new DataShareExtension();
    }
    HILOG_INFO("DataShareExtension::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsDataShareExtension::Create(runtime);

        default:
            return new DataShareExtension();
    }
}

void DataShareExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<DataShareExtensionContext>::Init(record, application, handler, token);
    HILOG_INFO("DataShareExtension begin init context");
}

std::shared_ptr<DataShareExtensionContext> DataShareExtension::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<DataShareExtensionContext> context =
        ExtensionBase<DataShareExtensionContext>::CreateAndInitContext(record, application, handler, token);
    if (record == nullptr) {
        HILOG_ERROR("DataShareExtension::CreateAndInitContext record is nullptr");
        return context;
    }
    context->SetAbilityInfo(record->GetAbilityInfo());
    return context;
}
}
}