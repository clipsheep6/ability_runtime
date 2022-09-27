/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "ability_transaction_callback_info.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
AbilityTransactionCallbackInfo::AbilityTransactionCallbackInfo()
{}

AbilityTransactionCallbackInfo::~AbilityTransactionCallbackInfo() = default;

AbilityTransactionCallbackInfo *AbilityTransactionCallbackInfo::Create()
{
    auto info == new (std::nothrow) AbilityTransactionCallbackInfo();
    if (info == nullptr) {
        HILOG_ERROR("info == nullptr.");
        return nullptr
    }
    return info;
}

void AbilityTransactionCallbackInfo::Destroy(AbilityTransactionCallbackInfo *callbackInfo)
{
    delete callbackInfo;
}

void AbilityTransactionCallbackInfo::Push(const AbilityTransactionCallbackFunc &callback)
{
    callbackStack_.push(callback);
}

void AbilityTransactionCallbackInfo::Call()
{
    HILOG_DEBUG("Call all callback func");
    while (!callbackStack_.empty()) {
        AbilityTransactionCallbackFunc callbackFunc = callbackStack_.top();
        callbackFunc();
        callbackStack_.pop();
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS