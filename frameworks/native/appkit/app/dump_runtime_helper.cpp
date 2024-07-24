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

#include "dump_runtime_helper.h"

#include "app_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "singleton.h"
#include "dfx_jsnapi.h"

namespace OHOS {
namespace AppExecFwk {
DumpRuntimeHelper::DumpRuntimeHelper(const std::shared_ptr<OHOSApplication> &application)
    : application_(application)
{}

void DumpRuntimeHelper::SetAppFreezeFilterCallback()
{
    if (application_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetAppFreezeFilterCallback OHOSApplication is nullptr");
        return;
    }
    auto& runtime = application_->GetRuntime();
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetAppFreezeFilterCallback GetRuntime is nullptr");
        return;
    }
    auto appfreezeFilterCallback = [] (const int32_t pid) -> bool {
        auto client = DelayedSingleton<AppExecFwk::AppMgrClient>::GetInstance();
        if (client == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "SetAppFreezeFilterCallback client is nullptr");
            return false;
        }
        return client->SetAppFreezeFilter(pid);
    };
    auto vm = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetEcmaVm();
    panda::DFXJSNApi::SetAppFreezeFilterCallback(vm, appfreezeFilterCallback);
}
} // namespace AppExecFwk
} // namespace OHOS
