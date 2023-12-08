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

#ifndef OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H
#define OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H

#include <mutex>
#include <unordered_map>

#include "iremote_object.h"

namespace OHOS::AbilityRuntime {
class FreezeUtil {
public:
    enum class TimeoutState {
        UNKNOWN = 0,
        LOAD,
        FOREGROUND,
        BACKGROUND
    };

    struct LifecycleFlow {
        sptr<IRemoteObject> token;
        TimeoutState state = TimeoutState::UNKNOWN;

        bool operator==(const LifecycleFlow &other) const
        {
            return token == other.token && state == other.state;
        }
    };

    FreezeUtil& operator=(const FreezeUtil&) = delete;
    FreezeUtil(const FreezeUtil&) = delete;
    virtual ~FreezeUtil() = default;
    static FreezeUtil& GetInstance();

    void AddLifecycleEvent(const LifecycleFlow &flow, const std::string &entry);
    std::string GetLifecycleEvent(const LifecycleFlow &flow);
    void DeleteLifecycleEvent(const LifecycleFlow &flow);
    void DeleteLifecycleEvent(sptr<IRemoteObject> token);

private:
    FreezeUtil() = default;
    void DeleteLifecycleEventInner(const LifecycleFlow &flow);

    class LifecycleFlowObjHash {
    public:
        size_t operator() (const LifecycleFlow &flow) const
        {
            return reinterpret_cast<size_t>(flow.token.GetRefPtr()) + std::hash<int>()(static_cast<int>(flow.state));
        }
    };

    std::mutex mutex_;
    std::unordered_map<LifecycleFlow, std::string, LifecycleFlowObjHash> lifecycleFlow_;
};
}  // namespace OHOS::AbilityRuntime
#endif  // OHOS_ABILITY_RUNTIME_FREEZE_UTIL_H