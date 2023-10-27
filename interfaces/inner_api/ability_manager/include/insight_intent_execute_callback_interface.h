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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_INTERFACE_H

#include "insight_intent_execute_result.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class IInsightIntentExecuteCallback
 * IInsightIntentExecuteCallback is used to notify caller ability that intent execute is complete.
 */
class IInsightIntentExecuteCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AAFwk.IntentExecuteCallback");

    /**
     * OnExecuteDone, AbilityMs notify caller ability the result of intent execute.
     *
     * @param key, the execute callback key.
     * @param resultCode, ERR_OK on success, others on failure.
     * @param executeResult, the execute result.
     */
    virtual void OnExecuteDone(uint64_t key, int32_t resultCode,
        const AppExecFwk::InsightIntentExecuteResult &executeResult) = 0;

    enum {
        // ipc id for OnExecuteDone
        ON_INSIGHT_INTENT_EXECUTE_DONE = 1,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_EXECUTE_CALLBACK_INTERFACE_H
