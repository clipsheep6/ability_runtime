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

#ifndef OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_MANAGER_H

#include "cpp/mutex.h"

#include "ability_record.h"
#include "status_bar_delegate_interface.h"

namespace OHOS {
namespace AAFwk {
class StatusBarDelegateManager {
public:
    StatusBarDelegateManager() = default;
    virtual ~StatusBarDelegateManager() = default;

    int32_t RegisterStatusBarDelegate(sptr<AbilityRuntime::IStatusBarDelegate> delegate);
    bool IsCallerInStatusBar();
    int32_t DoProcessAttachment(std::shared_ptr<AbilityRecord> abilityRecord);

private:
    DISALLOW_COPY_AND_MOVE(StatusBarDelegateManager);

    sptr<AbilityRuntime::IStatusBarDelegate> GetStatusBarDelegate();

    mutable ffrt::mutex statusBarDelegateMutex_;
    sptr<AbilityRuntime::IStatusBarDelegate> statusBarDelegate_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_STATUS_BAR_DELEGATE_MANAGER_H