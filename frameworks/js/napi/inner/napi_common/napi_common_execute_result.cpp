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

#include "napi_common_execute_result.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "insight_intent_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "want_params.h"
#include <memory>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
bool UnwrapExecuteResult(napi_env env, napi_value param, InsightIntentExecuteResult &executeResult)
{
    TAG_LOGD(AAFwkTag::JSNAPI, "called.");

    if (!IsTypeForNapiValue(env, param, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "UnwrapExecuteResult not object");
        return false;
    }
    int32_t code = 0;
    if (!UnwrapInt32ByPropertyName(env, param, "code", code)) {
        TAG_LOGE(AAFwkTag::JSNAPI, "Intent result must contian a code.");
        return false;
    }
    executeResult.code = code;

    napi_value result = nullptr;
    napi_get_named_property(env, param, "result", &result);
    if (result != nullptr) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, result, &valueType);
        if (valueType != napi_object) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type result.");
            return false;
        }
        auto wp = std::make_shared<AAFwk::WantParams>();
        if (!AppExecFwk::UnwrapWantParams(env, result, *wp)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Wrong argument type result.");
            return false;
        }
        if (!executeResult.CheckResult(wp)) {
            TAG_LOGE(AAFwkTag::JSNAPI, "Invalid intent result.");
            return false;
        }
        executeResult.result = wp;
    }

    return true;
}
}  // namespace AbilityRuntime
}  // namespace OHOS
