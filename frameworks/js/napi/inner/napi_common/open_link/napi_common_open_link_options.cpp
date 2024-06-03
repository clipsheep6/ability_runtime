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

#include "napi_common_open_link_options.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "int_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AppExecFwk {

bool UnwrapOpenLinkOptions(napi_env env, napi_value param, AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want)
{
    TAG_LOGI(AAFwkTag::JSNAPI, "called");

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGI(AAFwkTag::JSNAPI, "Params is invalid.");
        return false;
    }

    napi_value jsValue = GetPropertyValueByPropertyName(env, param, "parameters", napi_object);
    if (jsValue != nullptr) {
        AAFwk::WantParams wantParams;
        if (UnwrapWantParams(env, jsValue, wantParams)) {
            want.SetParams(wantParams);
        }
    }

    bool appLinkingOnly = false;
    if (UnwrapBooleanByPropertyName(env, param, APP_LINKING_ONLY.c_str(), appLinkingOnly)) {
        openLinkOptions.SetAppLinkingOnly(appLinkingOnly);
        want.SetParam(APP_LINKING_ONLY, appLinkingOnly);
    }
    if (!want.HasParameter(APP_LINKING_ONLY)) {
        want.SetParam(APP_LINKING_ONLY, false);
    }

    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
