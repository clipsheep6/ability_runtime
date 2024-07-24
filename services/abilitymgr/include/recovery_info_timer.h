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

#ifndef OHOS_ABILITY_RUNTIME_RECOVERY_INFO_TIMER_H
#define OHOS_ABILITY_RUNTIME_RECOVERY_INFO_TIMER_H

#include <list>
#include <string>
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {

struct RecoveryInfo {
    uint32_t tokenId = 0;
    int64_t time = 0;
    std::string bundleName = "";
    std::string moduleName = "";
    std::string abilityName = "";
};

class RecoveryInfoTimer {
    RecoveryInfoTimer() = default;
    ~RecoveryInfoTimer() = default;
public:
    static RecoveryInfoTimer& GetInstance();

    virtual void SubmitSaveRecoveryInfo(RecoveryInfo recoveryInfo);

private:
    std::mutex recoveryInfoQueueLock_;
    std::list<RecoveryInfo> recoveryInfoQueue_;
};
}
}
#endif