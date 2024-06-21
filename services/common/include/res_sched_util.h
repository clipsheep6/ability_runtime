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

#ifndef OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H
#define OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H

#include <unordered_set>

#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
struct AbilityInfo;
}
namespace AAFwk {
using AbilityInfo = AppExecFwk::AbilityInfo;
constexpr int64_t RES_TYPE_SCB_START_ABILITY = 0;
constexpr int64_t RES_TYPE_EXTENSION_START_ABILITY = 1;
constexpr int64_t RES_TYPE_MISSION_LIST_START_ABILITY = 2;
class ResSchedUtil final {
public:
    static ResSchedUtil &GetInstance();
    void ReportAbilitStartInfoToRSS(const AbilityInfo &abilityInfo, int32_t pid, bool isColdStart);
    void ReportAbilitAssociatedStartInfoToRSS(
        const AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid);
    void ReportEventToRSS(int32_t uid, std::string bundleName, std::string name);
    void GetAllFrozenPidsFromRSS(std::unordered_set<int32_t> &frozenPids);
private:
    ResSchedUtil() = default;
    ~ResSchedUtil() = default;
    DISALLOW_COPY_AND_MOVE(ResSchedUtil);

    int64_t convertType(int64_t resSchedType);
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H