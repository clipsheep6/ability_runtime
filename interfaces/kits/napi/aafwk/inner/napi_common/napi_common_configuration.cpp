/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "napi_common_configuration.h"

#include "hilog_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AppExecFwk {

EXTERN_C_START
bool UnwrapConfiguration(napi_env env, napi_value param, Configuration &config)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return false;
    }

    std::string language {""};
    if (UnwrapStringByPropertyName(env, param, "language", language)) {
        HILOG_DEBUG("The parsed language part %{public}s", language.c_str());
        if (!config.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, language)) {
            HILOG_ERROR("language Parsing failed");
            return false;
        }
    }

    return true;
}
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
