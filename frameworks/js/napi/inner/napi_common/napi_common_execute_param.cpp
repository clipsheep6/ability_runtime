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

#include "napi_common_execute_param.h"

#include "hilog_wrapper.h"
#include "napi_common_util.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapExecuteParam(napi_env env, napi_value param, InsightIntentExecuteParam &executeParam)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_ERROR("Params is invalid.");
        return false;
    }
    std::string bundleName, moduleName, abilityName, insightIntentName;
    if (!UnwrapStringByPropertyName(env, param, "bundleName", bundleName)) {
        HILOG_ERROR("Wrong argument type bundleName.");
        return false;
    }
    executeParam.bundleName_ = bundleName;

    if (!UnwrapStringByPropertyName(env, param, "moduleName", moduleName)) {
        HILOG_ERROR("Wrong argument type moduleName.");
        return false;
    }
    executeParam.moduleName_ = moduleName;

    if (!UnwrapStringByPropertyName(env, param, "abilityName", abilityName)) {
        HILOG_ERROR("Wrong argument type abilityName.");
        return false;
    }
    executeParam.abilityName_ = abilityName;

    if (!UnwrapStringByPropertyName(env, param, "insightIntentName", insightIntentName)) {
        HILOG_ERROR("Wrong argument type insightIntentName.");
        return false;
    }
    executeParam.insightIntentName_ = insightIntentName;

    napi_value napiIntentParam = nullptr;
    napi_get_named_property(env, param, "insightIntentParam", &napiIntentParam);
    if (napiIntentParam == nullptr) {
        HILOG_ERROR("Wrong argument type insightIntentParam.");
        return false;
    }

    if (!IsTypeForNapiValue(env, napiIntentParam, napi_object)) {
        HILOG_ERROR("Wrong argument type intentParam.");
        return false;
    }
    auto wp = std::make_shared<WantParams>();
    if (!AppExecFwk::UnwrapWantParams(env, napiIntentParam, *wp)) {
        HILOG_ERROR("Wrong argument type intentParam.");
        return false;
    }
    executeParam.insightIntentParam_ = wp;
    int32_t executeMode = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "executeMode", executeMode)) {
        HILOG_ERROR("Wrong argument type executeMode.");
        return false;
    }
    executeParam.executeMode_ = executeMode;
    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
