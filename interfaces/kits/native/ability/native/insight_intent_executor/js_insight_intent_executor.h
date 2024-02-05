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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_EXECUTOR_H

#include "insight_intent_executor.h"

#include "native_reference.h"

class NativeReference;

namespace OHOS {
namespace AppExecFwk {
    struct InsightIntentExecuteResult;
} // namespace AAFwk
namespace AbilityRuntime {
class JsRuntime;

class JsInsightIntentExecutor final : public InsightIntentExecutor {
public:
    static std::shared_ptr<JsInsightIntentExecutor> Create(JsRuntime& runtime);
    enum class State {
        INVALID,
        CREATED,
        INITIALIZED,
        EXECUTING,
        EXECUTATION_DONE,
        DESTROYED
    };
private:
    static constexpr std::array<const char*,
        static_cast<size_t>(InsightIntentExecuteMode::Count)> JS_FUNC_NAME_FOR_MODE {
            "onExecuteInUIAbilityForegroundMode",
            "onExecuteInUIAbilityBackgroundMode",
            "onExecuteInUIExtensionAbility",
            "onExecuteInServiceExtensionAbility"
        };
    static constexpr std::array<size_t,
        static_cast<size_t>(InsightIntentExecuteMode::Count)> JS_ARGC_FOR_MODE {
            3, 2, 3, 2 };

    explicit JsInsightIntentExecutor(JsRuntime& runtime);
public:
    JsInsightIntentExecutor(const JsInsightIntentExecutor&) = delete;
    JsInsightIntentExecutor(const JsInsightIntentExecutor&&) = delete;
    JsInsightIntentExecutor& operator=(const JsInsightIntentExecutor&) = delete;
    JsInsightIntentExecutor& operator=(const JsInsightIntentExecutor&&) = delete;
    ~JsInsightIntentExecutor() override;

    /**
     * @brief Init the intent executor and intent context.
     *
     * @param
     */
    bool Init(const InsightIntentExecutorInfo& insightIntentInfo) override;

    /**
     * @brief Handling the life cycle execute intent.
     *
     * @param
     *
     */
    bool HandleExecuteIntent(
        InsightIntentExecuteMode mode,
        const std::string& name,
        const AAFwk::WantParams& param,
        const std::shared_ptr<NativeReference>& pageLoader,
        std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
        bool& isAsync) override;

    inline State GetState() const
    {
        return state_;
    }

private:
    static std::unique_ptr<NativeReference> LoadJsCode(
        const InsightIntentExecutorInfo& insightIntentInfo,
        JsRuntime& runtime);

    static bool CallJsFunctionWithResult(
        napi_env env,
        napi_value obj,
        const char* funcName,
        size_t argc,
        const napi_value* argv,
        napi_value& result
    );

    bool CallJsFunctionWithResultInner(
        const char* funcName,
        size_t argc,
        const napi_value* argv,
        napi_value& result
    );

    static std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> GetResultFromJs(napi_env env,
        napi_value resultJs);

    static napi_value ResolveCbCpp(napi_env env, napi_callback_info info);
    static napi_value RejectCbCpp(napi_env env, napi_callback_info info);

    static void ReplyFailed(InsightIntentExecutorAsyncCallback* callback,
        InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_INTERNAL_ERROR);
    static void ReplySucceeded(InsightIntentExecutorAsyncCallback* callback,
        std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    void ReplyFailedInner(InsightIntentInnerErr innerErr = InsightIntentInnerErr::INSIGHT_INTENT_INTERNAL_ERROR);
    void ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp);
    bool ExecuteIntentCheckError();

    bool HandleResultReturnedFromJsFunc(napi_value resultJs);

    static bool CheckParametersUIAbilityForeground(const std::shared_ptr<NativeReference>& windowStage);
    bool ExecuteInsightIntentUIAbilityForeground(
        const std::string& name,
        const AAFwk::WantParams& param,
        const std::shared_ptr<NativeReference>& windowStage);

    static bool CheckParametersUIAbilityBackground();
    bool ExecuteInsightIntentUIAbilityBackground(
        const std::string& name,
        const AAFwk::WantParams& param);

    static bool CheckParametersUIExtension(const std::shared_ptr<NativeReference>& UIExtensionContentSession);
    bool ExecuteInsightIntentUIExtension(
        const std::string& name,
        const AAFwk::WantParams& param,
        const std::shared_ptr<NativeReference>& UIExtensionContentSession);

    static bool CheckParametersServiceExtension();
    bool ExecuteInsightIntentServiceExtension(
        const std::string& name,
        const AAFwk::WantParams& param);

    JsRuntime& runtime_;
    State state_ = State::CREATED;
    std::unique_ptr<NativeReference> jsObj_ = nullptr;
    std::unique_ptr<NativeReference> contextObj_ = nullptr;
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback_;
    bool isAsync_ = false;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_EXECUTOR_H
