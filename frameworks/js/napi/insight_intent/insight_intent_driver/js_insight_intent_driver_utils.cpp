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

#include "js_insight_intent_driver_utils.h"

#include <cstdint>

#include "ability_state.h"
#include "hilog_wrapper.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsExecuteResult(napi_env env, const AppExecFwk::InsightIntentExecuteResult &result)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);

    napi_set_named_property(env, objValue, "code", CreateJsValue(env, result.code));
    if (result.result != nullptr) {
        napi_set_named_property(env, objValue, "result",
            OHOS::AppExecFwk::CreateJsWantParams(env, *result.result));
    }
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS
