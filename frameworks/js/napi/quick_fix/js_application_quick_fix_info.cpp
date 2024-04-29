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

#include "js_application_quick_fix_info.h"

#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsApplicationQuickFixInfo(
    napi_env env, const AAFwk::ApplicationQuickFixInfo &appQuickFixInfo)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    napi_set_named_property(env, objValue, "bundleName", CreateJsValue(env, appQuickFixInfo.bundleName));
    napi_set_named_property(env, objValue, "bundleVersionCode",
        CreateJsValue(env, appQuickFixInfo.bundleVersionCode));
    napi_set_named_property(env, objValue, "bundleVersionName",
        CreateJsValue(env, appQuickFixInfo.bundleVersionName));
    napi_set_named_property(env, objValue, "quickFixVersionCode",
        CreateJsValue(env, appQuickFixInfo.appqfInfo.versionCode));
    napi_set_named_property(env, objValue, "quickFixVersionName",
        CreateJsValue(env, appQuickFixInfo.appqfInfo.versionName));
    napi_set_named_property(env, objValue, "hapModuleQuickFixInfo",
        CreateJsHapModuleQuickFixInfoArray(env, appQuickFixInfo.appqfInfo.hqfInfos));
    return objValue;
}

napi_value CreateJsHapModuleQuickFixInfoArray(napi_env env, const std::vector<AppExecFwk::HqfInfo> &hqfInfos)
{
    napi_value arrayValue = nullptr;
    napi_create_array_with_length(env, hqfInfos.size(), &arrayValue);
    uint32_t index = 0;
    for (const auto &hqfInfo : hqfInfos) {
        napi_value objValue = nullptr;
        napi_create_object(env, &objValue);
        napi_set_named_property(env, objValue, "moduleName", CreateJsValue(env, hqfInfo.moduleName));
        napi_set_named_property(env, objValue, "originHapHash", CreateJsValue(env, hqfInfo.hapSha256));
        napi_set_named_property(env, objValue, "quickFixFilePath", CreateJsValue(env, hqfInfo.hqfFilePath));
        napi_set_element(env, arrayValue, index++, objValue);
    }
    return arrayValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
