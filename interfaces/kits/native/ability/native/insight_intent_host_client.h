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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_HOST_CLIENT_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_HOST_CLIENT_H

#include <map>
#include <memory>
#include <mutex>
#include "insight_intent_callback_interface.h"
#include "insight_intent_execute_callback_stub.h"

namespace OHOS {
namespace AbilityRuntime {
class InsightIntentHostClient : public AAFwk::InsightIntentExecuteCallbackStub {
public:
    InsightIntentHostClient() = default;
    virtual ~InsightIntentHostClient() = default;

    /**
     * @brief Get InsightIntentHostClient instance.
     * @return InsightIntentHostClient instance.
     */
    static sptr<InsightIntentHostClient> GetInstance();

    /**
     * @brief Add InsightIntent host.
     * @param callback the host of the InsightIntent executing.
     * @return Returns the execute callback key.
     */
    uint64_t AddInsightIntentExecute(const std::shared_ptr<InsightIntentExecuteCallbackInterface> &callback);

    /**
     * @brief Remove InsightIntent host.
     * @param key, the execute callback key.
     */
    void RemoveInsightIntentExecute(uint64_t key);

    /**
     * @brief Indicates that the InsightIntent executing is complete
     * @param key, the execute callback key.
     * @param resultCode, ERR_OK on success, others on failure.
     * @param executeResult, the execute result.
     */
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override;

private:
    static std::mutex instanceMutex_;
    uint64_t key_ = 0;
    static sptr<InsightIntentHostClient> instance_;
    mutable std::mutex insightIntentExecutebackMutex_;
    std::map<uint64_t, std::shared_ptr<InsightIntentExecuteCallbackInterface>> callbackMap_;
    DISALLOW_COPY_AND_MOVE(InsightIntentHostClient);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_HOST_CLIENT_H