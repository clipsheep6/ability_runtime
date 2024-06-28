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

#include "insight_intent_executor.h"
#include "js_insight_intent_executor.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "runtime.h"

namespace OHOS::AbilityRuntime {
std::shared_ptr<InsightIntentExecutor> InsightIntentExecutor::Create(Runtime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "InsightIntentExecutor Create runtime");
    switch (runtime.GetLanguage()) {
        case Runtime::Language::JS:
            return static_cast<std::shared_ptr<InsightIntentExecutor>>(JsInsightIntentExecutor::Create(
                static_cast<JsRuntime&>(runtime)));
        default:
            return nullptr;
    }
}

bool InsightIntentExecutor::Init(const InsightIntentExecutorInfo& intentInfo)
{
    auto executeParam = intentInfo.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "Execute param invalid.");
        return false;
    }

    context_ = std::make_shared<InsightIntentContext>(intentInfo.token, executeParam->bundleName_,
        intentInfo.windowMode, executeParam->insightIntentId_);
    return true;
}

std::shared_ptr<InsightIntentContext> InsightIntentExecutor::GetContext()
{
    return context_;
}
} // namespace OHOS::AbilityRuntime
