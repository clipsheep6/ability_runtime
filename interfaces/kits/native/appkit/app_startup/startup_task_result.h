/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STARTUP_TASK_RESULT_H
#define OHOS_ABILITY_RUNTIME_STARTUP_TASK_RESULT_H

#include <string>
#include "ability_manager_errors.h"
#include "ability_transaction_callback_info.h"

namespace OHOS {
namespace AbilityRuntime {
class StartupTaskResult {
public:
    enum class ResultType {
        INVALID,
        JS
    };

    StartupTaskResult();

    StartupTaskResult(int32_t resultCode, const std::string &resultMessage);

    virtual ~StartupTaskResult();

    int32_t GetResultCode() const;

    std::string GetResultMessage() const;

    virtual ResultType GetResultType() const;

private:
    int32_t resultCode_ = ERR_OK;
    std::string resultMessage_;
};
using StartupTaskResultCallback = AppExecFwk::AbilityTransactionCallbackInfo<const std::shared_ptr<StartupTaskResult>>;
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_STARTUP_TASK_RESULT_H

