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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_PROXY_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_PROXY_H

#include "iremote_proxy.h"
#include "insight_intent_execute_callback_interface.h"

namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteCallbackProxy : public IRemoteProxy<IInsightIntentExecuteCallback> {
public:
    explicit InsightIntentExecuteCallbackProxy
        (const sptr<IRemoteObject> &impl) : IRemoteProxy<IInsightIntentExecuteCallback>(impl) {}

    virtual ~InsightIntentExecuteCallbackProxy() {}

    /**
     * OnExecuteDone, AbilityMs notify caller ability the result of intent execute.
     *
     * @param key, the execute callback key.
     * @param resultCode, ERR_OK on success, others on failure.
     * @param executeResult, the execute result.
     */
    void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) override;

private:
    static inline BrokerDelegator<InsightIntentExecuteCallbackProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_PROXY_H