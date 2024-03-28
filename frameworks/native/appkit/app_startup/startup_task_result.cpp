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

#include "startup_task_result.h"

namespace OHOS {
namespace AbilityRuntime {
StartupTaskResult::StartupTaskResult() = default;

StartupTaskResult::~StartupTaskResult() = default;

StartupTaskResult::StartupTaskResult(int32_t resultCode, const std::string &resultMessage)
    : resultCode_(resultCode), resultMessage_(resultMessage)
{}

int32_t StartupTaskResult::GetResultCode() const
{
    return resultCode_;
}

std::string StartupTaskResult::GetResultMessage() const
{
    return resultMessage_;
}

StartupTaskResult::ResultType StartupTaskResult::GetResultType() const
{
    return ResultType::INVALID;
}
} // namespace AbilityRuntime
} // namespace OHOS
