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

#include "insight_intent_executor.h"
#include "js_insight_intent_executor.h"

#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "runtime.h"

namespace OHOS::AbilityRuntime {
std::shared_ptr<InsightIntentExecutor> InsightIntentExecutor::Create(Runtime& runtime)
{
    HILOG_DEBUG("InsightIntentExecutor Create runtime");
    switch (runtime.GetLanguage()) {
        case Runtime::Language::JS:
            return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentExecutor::Create(
                static_cast<JsRuntime&>(runtime)));
        default:
            return nullptr;
    }
}
} // namespace OHOS::AbilityRuntime
