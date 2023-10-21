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

#include "freeze_util.h"

#include "hilog_wrapper.h"

namespace OHOS::AbilityRuntime {
FreezeUtil& FreezeUtil::GetInstance()
{
    static FreezeUtil instance;
    return instance;
}

void FreezeUtil::AddLifecycleEvent(const LifecycleFlow &flow, const std::string &entry)
{
    std::lock_guard lock(mutex_);
    if (lifecycleFolw_.count(flow)) {
        lifecycleFolw_[flow] = lifecycleFolw_[flow] + "\n" + entry;
    } else {
        lifecycleFolw_[flow] = entry;
    }
}

std::string FreezeUtil::GetLifecycleEvent(const LifecycleFlow &flow)
{
    std::lock_guard lock(mutex_);
    auto search = lifecycleFolw_.find(flow);
    if (search != lifecycleFolw_.end()) {
        return search->second;
    }
    return "";
}

void FreezeUtil::DeleteLifecycleEvent(const LifecycleFlow &flow)
{
    std::lock_guard lock(mutex_);
    DeleteLifecycleEventInner(flow);
}

void FreezeUtil::DeleteLifecycleEvent(sptr<IRemoteObject> token)
{
    std::lock_guard lock(mutex_);
    if (lifecycleFolw_.empty()) {
        return;
    }
    LifecycleFlow foregroundFlow = { token, TimeoutState::FOREGROUND };
    DeleteLifecycleEventInner(foregroundFlow);

    LifecycleFlow backgroundFlow = { token, TimeoutState::BACKGROUND };
    DeleteLifecycleEventInner(backgroundFlow);
}

void FreezeUtil::DeleteLifecycleEventInner(const LifecycleFlow &flow)
{
    if (lifecycleFolw_.count(flow)) {
        lifecycleFolw_.erase(flow);
    }
    HILOG_INFO("lifecycleFolw_ size: %{public}d", lifecycleFolw_.size());
}
}  // namespace OHOS::AbilityRuntime
